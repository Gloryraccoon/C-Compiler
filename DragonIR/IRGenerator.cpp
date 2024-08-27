#include "IRGenerator.h"
#include "SymbolGenerator.h"

#include <string.h>

std::stack<std::string> TrueLabels;
std::stack<std::string> FalseLabels;
std::stack<std::string> Breaklabels;
std::stack<std::string> ContinueLabels;
static int ifflag = 0;
extern bool ExitLabel;
extern Value* findValue(std::string name, std::string funcName, bool Temp);
extern Function* findFuncValue(std::string name);
std::string ArrName;

IRGenerator::IRGenerator(ast_node* _root) : root(_root)
{
    /*只处理单参数IR函数*/
    ast2ir_handlers[ast_operator_type::AST_OP_TOP] = &IRGenerator::ir_compile_unit_top;
    ast2ir_handlers[ast_operator_type::AST_SENTENCE_BLOCK] = &IRGenerator::ir_block;
    ast2ir_handlers[ast_operator_type::AST_FUNC_DEF] = &IRGenerator::ir_function_define;
    ast2ir_handlers[ast_operator_type::AST_OP_RETURN] = &IRGenerator::ir_return;
    ast2ir_handlers[ast_operator_type::AST_OP_ASSIGN] = &IRGenerator::ir_assign;
    ast2ir_handlers[ast_operator_type::AST_OP_WHILE] = &IRGenerator::ir_while;
    ast2ir_handlers[ast_operator_type::AST_OP_IF] = &IRGenerator::ir_if;
    ast2ir_handlers[ast_operator_type::AST_OP_CMP] = &IRGenerator::ir_compare;
    ast2ir_handlers[ast_operator_type::AST_OP_FOR] = &IRGenerator::ir_for;
    ast2ir_handlers[ast_operator_type::AST_OP_CONTINUE] = &IRGenerator::ir_continue;
    ast2ir_handlers[ast_operator_type::AST_OP_BREAK] = &IRGenerator::ir_break;
    ast2ir_handlers[ast_operator_type::AST_OP_NEG] = &IRGenerator::ir_neg;
    ast2ir_handlers[ast_operator_type::AST_OP_LINC] = &IRGenerator::ir_linc;
    ast2ir_handlers[ast_operator_type::AST_OP_RINC] = &IRGenerator::ir_rinc;
    ast2ir_handlers[ast_operator_type::AST_OP_LDEC] = &IRGenerator::ir_ldec;
    ast2ir_handlers[ast_operator_type::AST_OP_RDEC] = &IRGenerator::ir_rdec;
    ast2ir_handlers[ast_operator_type::AST_OP_AND] = &IRGenerator::ir_and;
    ast2ir_handlers[ast_operator_type::AST_OP_OR] = &IRGenerator::ir_or;  

}

bool IRGenerator::run()
{
    ast_node* result = nullptr;
    result = ir_visit_ast_node(root);
    return result != nullptr;
}

bool IRGenerator::ir_compile_unit_top(ast_node* node)
{
    /*遍历全局变量符号表*/
    for(int i = 0;i < symbolTable->valueName.size();++i)
    {
        Value* value;
        value = symbolTable->globalValueList[symbolTable->valueName[i]];
        
        if(value->IsGlobalAssigned)
        {
            node->blockInsts.addInst(new DeclareIRInst(value, true, true, false, true));
        }
        else
        {
            node->blockInsts.addInst(new DeclareIRInst(value, true, true));
        }
    }

    for(auto pit = node->sons.begin();pit != node->sons.end();++pit)
    {
        if((*pit)->type == AST_FUNC_DEF)
        {
            ast_node* temp = ir_visit_ast_node(*pit);
            node->blockInsts.addInst(temp->blockInsts);
        }
    }
    
    return true;
}

bool IRGenerator::ir_block(ast_node* node)
{
    std::vector<ast_node*>::iterator pit;
    LocalBlock* lcalBlock = new LocalBlock();

    Function* function = symbolTable->findFuncValue(Function::currentFunc);
    if(function->currentScope != -1)
    {
        function->tempstack.push(lcalBlock);
    }
    ++function->currentScope;

    for(pit = node->sons.begin();pit != node->sons.end();++pit)
    {
        ast_node* temp;
        temp = ir_visit_ast_node(*pit, true);
        if(!temp)
        {
            continue;
        }
        node->blockInsts.addInst(temp->blockInsts);
        if(temp->type == AST_OP_RETURN)
        {
            break;
        }
    }
    function->tempstack.pop(function->currentScope);
    --function->currentScope;
    return true;
}

bool IRGenerator::ir_return(ast_node* node)
{
    if(node->sons.size() == 0)
    {
        if(ifflag)
        {
            node->blockInsts.addInst(new GotoIRInst(IRINST_BRJUMP, ".L2"));
            ExitLabel = true;
        }
        return true;
    }

    ast_node* temp = ir_visit_ast_node(node->sons[0]);
    if(!temp)
    {
        return false;
    }

    node->blockInsts.addInst(temp->blockInsts);
    Value* Val_return = symbolTable->findLocalValue("return", Function::currentFunc, true);

    node->blockInsts.addInst(new AssignIRInst(Val_return, temp->val));

    if(ifflag)
    {
        node->blockInsts.addInst(new GotoIRInst(IRINST_BRJUMP, ".L2"));
        ExitLabel = true;
    }

    return true;
}

bool IRGenerator::ir_function_define(ast_node* node)
{
    ast_node* func_return_type = node->sons[0];

    ast_node* func_name = node->sons[1];
    ast_node* func_args = node->sons[2];

    Function::currentFunc = func_name->attr.id;

    Function* function = symbolTable->funcsList[Function::currentFunc];

    func_name->func = function;
    
    if(Function::is_build_func(Function::currentFunc))
    {
        return true;
    }

    LocalBlock* lcalBlock = new LocalBlock();
    function->tempstack.push(lcalBlock);
    function->currentLocal = 0;
    function->currentScope = 0;

    ValueType type;
    Value* returnValue;

    if(strcmp(func_return_type->attr.id, "int") == 0)
    {
        type = ValueType::TYPE_INT;
        returnValue = function->stack.findALLStack("return", 0);  //从最外部作用域开始找返回语句
        int return_index = func_args->sons.size();
        returnValue = function->localValueList[function->localValueNames[return_index]];
        function->addValue(returnValue, "return", true);  //将该值加入临时栈
        ++function->currentLocal;
    }
    else if(strcmp(func_return_type->attr.id, "void") == 0)
    {
        type = ValueType::TYPE_VOID;
    }

    if(node->sons[3]->type == AST_OP_EMPTY)
    {
        return true;
    }

    node->blockInsts.addInst(new DefineFuncIRInst(function, type));

    //int i = 0;
    for(auto pit = func_args->sons.end() - 1;pit != func_args->sons.begin() - 1;--pit)
    {
        ast_node* arg_name = (*pit)->sons[1];
        if(arg_name->type != AST_ARRAY)
        {
            symbolTable->addValue(arg_name->attr.id, arg_name->val, Function::currentFunc, true);
        }
        else
        {
            symbolTable->addValue(arg_name->sons[0]->attr.id, arg_name->val, Function::currentFunc, true);
        }

        ++function->currentLocal;
    }

    function->currentScope = -1;

    ast_node* func_block = ir_visit_ast_node(node->sons[3]);

    InterCode* blockInsts = new InterCode();

    for(int i = 0;i < function->argsList.size();++i)
    {
        Value* srcValue = function->tempValueList[function->tempValueNames[i]];
        Value* resultValue = function->localValueList[function->localValueNames[i]];

        blockInsts->addInst(new AssignIRInst(resultValue, srcValue));
    }

    blockInsts->addInst(func_block->blockInsts);

    if(ExitLabel)
    {
        blockInsts->addInst(new GotoIRInst(IRINST_BRJUMP, ".L2"));

        blockInsts->addInst(new StringIRInst(".L2:"));
    }

    ExitLabel = false;

    function->currentScope = 0;
    
    if (strcmp(func_return_type->attr.id, "int") == 0) 
    {
        Value *srcValue = nullptr;
        srcValue = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
        blockInsts->addInst(new AssignIRInst(srcValue, returnValue));
        blockInsts->addInst(new SingleIRInst(IRINST_OP_RETURN, srcValue, srcValue));
    } 
    else 
    {
        blockInsts->addInst(new SingleIRInst(IRINST_OP_RETURN));
    }

    for(int i = 0;i < function->localValueNames.size();++i)
    {
        Value* value = function->localValueList[function->localValueNames[i]];
        node->blockInsts.addInst(new DeclareIRInst(value, false, true));
    }

    for(int i = function->argsList.size();i < function->tempValueNames.size();++i)
    {
        Value* value = function->tempValueList[function->tempValueNames[i]];
        node->blockInsts.addInst(new DeclareIRInst(value, false, true, true));
    }

    node->blockInsts.addInst(new StringIRInst("    entry"));

    node->blockInsts.addInst(*blockInsts);
    node->blockInsts.addInst(new StringIRInst("}"));

    return true;   
}

ast_node* IRGenerator::ir_visit_ast_node(ast_node* node, bool isLeft)
{
    if(node == nullptr)
    {
        return nullptr;
    }

    std::unordered_map<ast_operator_type, ast2ir_handler_t>::const_iterator pit;
    pit = ast2ir_handlers.find(node->type);
    if(pit != ast2ir_handlers.end())
    {
        ast2ir_handler_t function = pit->second;
        bool result = (this->*function)(node);
        if(!result)
        {
            return nullptr;
        }
    }
    else
    {
        bool result = true;
        /*处理多参数IR函数*/
        switch(node->type)
        {
            /*处理叶子节点*/
            case AST_OP_END:
                result = ir_leaf_node(node, isLeft);
                break;
            case AST_DEF_LIST:
                result = ir_def_list(node, true);
                break;            
            case AST_OP_MUL:
            case AST_OP_ADD:
            case AST_OP_SUB:
            case AST_OP_DIV:
            case AST_OP_MOD:
                result = ir_op_binary(node, node->type);
                break;
            case AST_OP_NOT:
                result = ir_not(node, isLeft);
                break;
            case AST_FUNC_CALL:
                result = ir_call_func(node, isLeft);
                break;
            case AST_ARRAY_INDEX:
                result = ir_array_index(node, isLeft);
                break;
            case AST_OP_EMPTY:
                result = true;
                break;
            default:
                result = false;
                break;
        }

        if(!result)
        {
            return nullptr;
        }
    }

    return node;
}

bool IRGenerator::ir_leaf_node(ast_node* node, bool isLeft)
{
    Function* function = nullptr;
    Value* value = nullptr;
    if(node->attr.kind == DIGIT_ID)
    {
        value = findValue(node->attr.id, Function::currentFunc, true);
        function = findFuncValue(node->attr.id);

        if((!value) && (!function))
        {
            //printf("Line(%d) Variable(%s) not defined\n",
            //node->attr.lineno,
            //node->attr.id);
            return false;
        }
        node->val = value;

        if(!isLeft)
        {
            if(value->dimensions.size() > 0)
            {
                Value *dstVal = newTempValue(ValueType::TYPE_INT_PTR, Function::currentFunc);
                node->blockInsts.addInst(new AssignIRInst(dstVal, value, true));
                node->val = dstVal;
            }
            else
            {
                Value* dstValue = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
                node->blockInsts.addInst(new AssignIRInst(dstValue, value));
                node->val = dstValue;
            }
        }
    }
    else if(node->attr.kind == DIGIT_INT)
    {
        value = newConstValue(node->attr.integer_val);
        node->val = value;
    }
    else
    {
        value = newConstValue(node->attr.real_val);
        node->val = value;
    }

    return true;
}

bool IRGenerator::ir_def_list(ast_node* node, bool isLocal)
{
    for (auto pit = node->sons.begin() + 1; pit != node->sons.end(); ++pit) {
        Value* value = nullptr;
        ast_node* tempNode = *pit;
        if(tempNode->type == AST_ARRAY) //处理数组
        {
            bool result = ir_array(tempNode, isLocal);
            if (!result)
                return false;
        }
        else if(tempNode->type == AST_OP_ASSIGN)
        {
            ast_node* temp_tempNode = tempNode->sons[0];
            Function* function = symbolTable->findFuncValue(Function::currentFunc);
            value = function->localValueList[function->localValueNames[function->currentLocal++]];
            symbolTable->addValue(temp_tempNode->attr.id, value, Function::currentFunc, true);
            bool result = ir_assign(tempNode);
            if(!result)
            {
                return false;
            }
            node->blockInsts.addInst(tempNode->blockInsts);
        }
        else  
        {
            Function* function = symbolTable->findFuncValue(Function::currentFunc);
            value = function->localValueList[function->localValueNames[function->currentLocal++]];
            symbolTable->addValue(tempNode->attr.id, value, Function::currentFunc, true);
        }
    }
    return true;
}

bool IRGenerator::ir_assign(ast_node* node)
{
    ast_node* left_node = node->sons[0];
    ast_node* right_node = node->sons[1];

    // 赋值运算符的左侧操作数
    ast_node* left = ir_visit_ast_node(left_node, true);
    if (!left) 
    {
        return false;
    }

    ast_node* right = ir_visit_ast_node(right_node, false);
    if (!right) {
        return false;
    }

    //创建IR
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(left->blockInsts);

    Value *leftValue = left->val, *rightValue = right->val;
    node->blockInsts.addInst(new AssignIRInst(leftValue, rightValue));
    
    return true;
}

bool IRGenerator::ir_op_binary(ast_node* node, ast_operator_type type)
{
    ast_node* left_node = node->sons[0];
    ast_node* right_node = node->sons[1];

    // 赋值运算符的左侧操作数
    ast_node *left = ir_visit_ast_node(left_node);
    if (!left) {
        return false;
    }

    // 赋值运算符的右侧操作数
    ast_node *right = ir_visit_ast_node(right_node);
    if (!right) {
        return false;
    }

    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    Value *leftValue = left->val;
    Value *rightValue = right->val;
    Value *resultValue = nullptr;
    if ((leftValue->is_const) && (rightValue->is_const)) {
        int result = 0;
        switch (type) {
        case AST_OP_ADD:
            result = rightValue->intVal + leftValue->intVal;
            break;
        case AST_OP_SUB:
            result = rightValue->intVal - leftValue->intVal;
            break;
        case AST_OP_MUL:
            result = rightValue->intVal * leftValue->intVal;
            break;
        case AST_OP_DIV:
            result = rightValue->intVal / leftValue->intVal;
            break;
        case AST_OP_MOD:
            result = rightValue->intVal % leftValue->intVal;
            break;
        default:
            break;
        }

        resultValue = newConstValue(result);
        node->val = resultValue;
    } else {
        resultValue = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
        node->val = resultValue;
        switch (type) {
        case AST_OP_ADD:
            node->blockInsts.addInst(
                new BinaryIRInst(IRINST_ADD, node->val, left->val, right->val)
            );
            break;
        case AST_OP_SUB:
            node->blockInsts.addInst(
                new BinaryIRInst(IRINST_SUB, node->val, left->val, right->val)
            );
            break;
        case AST_OP_MUL:
            node->blockInsts.addInst(
                new BinaryIRInst(IRINST_MUL, node->val, left->val, right->val)
            );
            break;
        case AST_OP_DIV:
            node->blockInsts.addInst(
                new BinaryIRInst(IRINST_DIV, node->val, left->val, right->val)
            );
            break;
        case AST_OP_MOD:
            node->blockInsts.addInst(
                new BinaryIRInst(IRINST_MOD, node->val, left->val, right->val)
            );
            break;
        default:
            break;
        }

    }
    return true;
}

bool IRGenerator::ir_while(ast_node* node)
{
    ast_node *first_node = node->sons[0];
    ast_node *second_node = node->sons[1];
    ifflag++;
    std::string label1 = newLabel(Function::currentFunc);  // 条件语句
    std::string label2 = newLabel(Function::currentFunc);  // true语句
    std::string label3 = newLabel(Function::currentFunc);  // 下一条语句
    TrueLabels.push(label2);
    FalseLabels.push(label3);
    Breaklabels.push(label3);
    ContinueLabels.push(label1);

    ast_node *left = ir_visit_ast_node(first_node);
    if (!left) 
    {
        return false;
    }

    ast_node *right = ir_visit_ast_node(second_node);
    if (!right) 
    {
        return false;
    }    

    node->blockInsts.addInst(new GotoIRInst(IRINST_BRJUMP, label1));
    node->blockInsts.addInst(new StringIRInst(label1 + ":"));
    node->blockInsts.addInst(left->blockInsts);
    if (first_node->type == AST_OP_AND || first_node->type == AST_OP_OR || first_node->type == AST_OP_CMP || first_node->type == AST_OP_NOT) 
    {} 
    else 
    {
        if (first_node->val->is_var) {
            Value *val = findValue(first_node->attr.id, Function::currentFunc, true);
            Value *dstVal = first_node->val;
            node->blockInsts.addInst(new AssignIRInst(dstVal, val));
        }

        if (first_node->val->type != ValueType::TYPE_BOOLEAN) {
            Value *val = newTempValue(ValueType::TYPE_BOOLEAN, Function::currentFunc);
            Value *constVal = newConstValue(0);
            node->blockInsts.addInst(new BinaryIRInst(IRINST_CMP, "ne", val, left->val, constVal));
            node->blockInsts.addInst(new GotoIRInst(IRINST_BCJUMP, val, TrueLabels.top(), FalseLabels.top()));
            node->val = val;
        }

    }

    node->blockInsts.addInst(new StringIRInst(label2 + ":"));
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(new GotoIRInst(IRINST_BRJUMP, label1));
    node->blockInsts.addInst(new StringIRInst(label3 + ":"));

    TrueLabels.pop();
    FalseLabels.pop();
    Breaklabels.pop();
    ContinueLabels.pop();
    ifflag--;

    return true;
}

bool IRGenerator::ir_if(ast_node* node)
{
    ifflag++;
    std::string label1 = newLabel(Function::currentFunc);  // true语句
    std::string label2 = newLabel(Function::currentFunc);  // false语句
    std::string label3 = newLabel(Function::currentFunc);  // 下一条语句
    if (node->sons.size() == 3) {
        TrueLabels.push(label1);
        FalseLabels.push(label2);
    } else {
        TrueLabels.push(label1);
        FalseLabels.push(label3);
    }

    ast_node *first_node = node->sons[0];
    ast_node *second_node = node->sons[1];
    ast_node *condition = ir_visit_ast_node(first_node);
    if (!condition) 
    {
        return false;
    }
    node->blockInsts.addInst(condition->blockInsts);
    if (first_node->type == AST_OP_AND || first_node->type == AST_OP_OR || first_node->type == AST_OP_CMP || first_node->type == AST_OP_NOT) {
    } else {
        if (first_node->val->is_var) {
            Value *val = findValue(first_node->attr.id, Function::currentFunc, true);
            Value *dstVal = first_node->val;
            node->blockInsts.addInst(
                new AssignIRInst(dstVal, val)
            );
        }
        if (first_node->val->type == ValueType::TYPE_BOOLEAN) {

        } else {
            Value *val = newTempValue(ValueType::TYPE_BOOLEAN, Function::currentFunc);

            Value *constVal = newConstValue(0);
            node->blockInsts.addInst(
            new BinaryIRInst(IRINST_CMP, "ne", val, condition->val, constVal)
            );

            node->blockInsts.addInst(
                    new GotoIRInst(IRINST_BCJUMP, val, TrueLabels.top(), FalseLabels.top())
            );
            node->val = val;
        }

    }

    if (node->sons.size() == 3) {
        ast_node *third_node = node->sons[2];
        ast_node *true_node = ir_visit_ast_node(second_node, true);
        ast_node *false_node = ir_visit_ast_node(third_node, true);

        if (true_node->type != AST_OP_EMPTY && false_node->type != AST_OP_EMPTY) {
            if (true_node->newLabel) {
                node->blockInsts.addInst(
                        new StringIRInst(label1 + ":")
                );
            }

            node->blockInsts.addInst(true_node->blockInsts);
            if (true_node->jump) {
                node->blockInsts.addInst(
                        new GotoIRInst(IRINST_BRJUMP, label3)
                );
            }

            if (false_node->newLabel) {
                node->blockInsts.addInst(
                new StringIRInst(label2 + ":")
                );
            }
            node->blockInsts.addInst(false_node->blockInsts);
            if (false_node->jump) {
                node->blockInsts.addInst(
                        new GotoIRInst(IRINST_BRJUMP, label3)
                );
            }

        } else if (true_node->type != AST_OP_EMPTY and false_node->type == AST_OP_EMPTY) {

        }
    } else {
        ast_node *true_node = ir_visit_ast_node(second_node, true);
        node->blockInsts.addInst(
                new StringIRInst(label1 + ":")
        );
        node->blockInsts.addInst(true_node->blockInsts);
        if (true_node->jump) {
            node->blockInsts.addInst(
                    new GotoIRInst(IRINST_BRJUMP, label3)
            );
        }
    }
    node->blockInsts.addInst(
        new StringIRInst(label3 + ":")
    );
    TrueLabels.pop();
    FalseLabels.pop();
    ifflag--;
    return true;
}

bool IRGenerator::ir_compare(ast_node* node)
{
    ast_node *left = ir_visit_ast_node(node->sons[0]);
    if (!left)
    {
        return false;
    }

    ast_node *right = ir_visit_ast_node(node->sons[2]);
    if (!right) 
    {
        return false;
    }

    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);

    Value *val = newTempValue(ValueType::TYPE_BOOLEAN, Function::currentFunc);
    node->val = val;

    // 比较运算符
    ast_node *cmp_node = node->sons[1];
    node->blockInsts.addInst(
        new BinaryIRInst(IRINST_CMP, cmp_node->attr.id, node->val, left->val, right->val)
    );
    // printf("cmp指令0\n");
    node->blockInsts.addInst(
            new GotoIRInst(IRINST_BCJUMP, node->val, TrueLabels.top(), FalseLabels.top())
    );

    return true;
}

bool IRGenerator::ir_for(ast_node* node)
{
    ast_node *src1_node = node->sons[0];
    ast_node *src2_node = node->sons[1];
    ast_node *src3_node = node->sons[2];
    ast_node *src4_node = node->sons[3];
    std::string label1 = newLabel(Function::currentFunc);  // 条件语句
    std::string label2 = newLabel(Function::currentFunc);  // true语句
    std::string label3 = newLabel(Function::currentFunc);  // 下一条语句
    TrueLabels.push(label2);
    FalseLabels.push(label3);
    Breaklabels.push(label3);
    ContinueLabels.push(label1);
    ast_node *init_node = ir_visit_ast_node(src1_node);
    if (!init_node) {
        return false;
    }
    node->blockInsts.addInst(init_node->blockInsts);

    ast_node *cond = ir_visit_ast_node(src2_node);
    if (!cond) {
        return false;
    }
    ast_node *expr3 = ir_visit_ast_node(src3_node);
    ast_node *block_node = ir_visit_ast_node(src4_node);
    if (!block_node) {
        return false;
    }
    node->blockInsts.addInst(
        new GotoIRInst(IRINST_BRJUMP, label1)
    );
    node->blockInsts.addInst(
        new StringIRInst(label1 + ":")
    );
    node->blockInsts.addInst(cond->blockInsts);
    if (!(cond->val->type == ValueType::TYPE_BOOLEAN)) {
        Value *val = nullptr;
        val = newTempValue(ValueType::TYPE_BOOLEAN, Function::currentFunc);
        Value *constVal = newConstValue(0);
        node->blockInsts.addInst(
        new BinaryIRInst(IRINST_CMP, "ne", val, cond->val, constVal)
        );
        node->blockInsts.addInst(
            new GotoIRInst(IRINST_BCJUMP, val, TrueLabels.top(), FalseLabels.top())
        );
        node->val = val;
    }
    node->blockInsts.addInst(
        new StringIRInst(label2 + ":")
    );
    node->blockInsts.addInst(block_node->blockInsts);
    node->blockInsts.addInst(expr3->blockInsts);
    node->blockInsts.addInst(
        new GotoIRInst(IRINST_BRJUMP, label1)
    );
    node->blockInsts.addInst(
        new StringIRInst(label3 + ":")
    );
    TrueLabels.pop();
    FalseLabels.pop();
    Breaklabels.pop();
    ContinueLabels.pop();
    return true;
}

bool IRGenerator::ir_continue(ast_node* node)
{
    node->blockInsts.addInst(new GotoIRInst(IRINST_BRJUMP, ContinueLabels.top()));
    return true;
}

bool IRGenerator::ir_break(ast_node* node)
{
    node->blockInsts.addInst(new GotoIRInst(IRINST_BRJUMP, Breaklabels.top()));
    return true;
}

bool IRGenerator::ir_neg(ast_node* node)
{
    ast_node* son = node->sons[0];
    ast_node* result = ir_visit_ast_node(son);
    if (!result) {
        return false;
    }

    node->blockInsts.addInst(result->blockInsts);
    if (!(result->val->is_const or result->val->type == ValueType::TYPE_BOOLEAN)) 
    {
        Value *val = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
        node->blockInsts.addInst(
        new SingleIRInst(IRINST_OP_NEG, val, result->val));
        node->val = val;
    } 
    else 
    {
        node->val = result->val;
        node->val->intVal = -node->val->intVal;
    }
    return true;
}

bool IRGenerator::ir_not(ast_node* node, bool isleft)
{
    ast_node* temp_node = node->sons[0];
    std::string label1 = TrueLabels.top();
    TrueLabels.pop();
    std::string label2 = FalseLabels.top();
    FalseLabels.pop();
    TrueLabels.push(label2);
    FalseLabels.push(label1);
    ast_node *left = ir_visit_ast_node(temp_node);
    if (!left) {
        return false;
    }
    node->blockInsts.addInst(left->blockInsts);

    if(!isleft)
    {
        if (!(left->val->is_const)) 
        {
            if (left->val->type == ValueType::TYPE_BOOLEAN) 
            {
                node->val = left->val;
            } 
            else 
            {
                Value *val = newTempValue(ValueType::TYPE_BOOLEAN, Function::currentFunc);
                node->val = val;
                Value *constVal = newConstValue(0);
                node->blockInsts.addInst(
                new BinaryIRInst(IRINST_CMP, "ne", node->val, left->val, constVal)
                );

                node->blockInsts.addInst(
                new GotoIRInst(IRINST_BCJUMP, node->val, TrueLabels.top(), FalseLabels.top())
                );
            }
        }
    }
    label1 = TrueLabels.top();
    TrueLabels.pop();
    label2 = FalseLabels.top();
    FalseLabels.pop();
    TrueLabels.push(label2);
    FalseLabels.push(label1);
    return true;
}

bool IRGenerator::ir_linc(ast_node* node)
{
    ast_node* son = node->sons[0];
    ast_node* result = ir_visit_ast_node(son, true);
    if (!result) {
        return false;
    }

    node->blockInsts.addInst(result->blockInsts);
    Value* ConstVal = newConstValue(1);
    Value* val = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
    node->blockInsts.addInst(new BinaryIRInst(IRINST_ADD, val, result->val, ConstVal));
    node->blockInsts.addInst(new AssignIRInst(result->val, val));
    node->val = result->val;
    return true;
}

bool IRGenerator::ir_rinc(ast_node* node)
{
    ast_node* son = node->sons[0];
    ast_node* result = ir_visit_ast_node(son, true);
    if (!result) {
        return false;
    }

    node->blockInsts.addInst(result->blockInsts);
    Value* ConstVal = newConstValue(1);
    Value* val = newTempValue(ValueType::TYPE_INT, Function::currentFunc);

    node->blockInsts.addInst(new AssignIRInst(val, result->val));
    Value *tempval = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
    node->blockInsts.addInst(new BinaryIRInst(IRINST_ADD, tempval, result->val, ConstVal));
    node->blockInsts.addInst(new AssignIRInst(result->val, tempval));
    node->val = val;
    
    return true;
}

bool IRGenerator::ir_ldec(ast_node* node)
{
    ast_node* son = node->sons[0];
    ast_node* result = ir_visit_ast_node(son, true);
    if (!result) {
        return false;
    }

    node->blockInsts.addInst(result->blockInsts);
    Value* ConstVal = newConstValue(1);
    Value* val = newTempValue(ValueType::TYPE_INT, Function::currentFunc);

    node->blockInsts.addInst(new BinaryIRInst(IRINST_SUB, val, result->val, ConstVal));
    node->blockInsts.addInst(new AssignIRInst(result->val, val));
    node->val = result->val;
    
    return true;
}

bool IRGenerator::ir_rdec(ast_node* node)
{
    ast_node* son = node->sons[0];
    ast_node* result = ir_visit_ast_node(son, true);
    if (!result) {
        return false;
    }

    node->blockInsts.addInst(result->blockInsts);
    Value* ConstVal = newConstValue(1);
    Value* val = newTempValue(ValueType::TYPE_INT, Function::currentFunc);

    node->blockInsts.addInst(new AssignIRInst(val, result->val));
    Value *tempval = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
    node->blockInsts.addInst(new BinaryIRInst(IRINST_SUB, tempval, result->val, ConstVal));
    node->blockInsts.addInst(new AssignIRInst(result->val, tempval));
    node->val = val;
    
    return true;
}

bool IRGenerator::ir_and(ast_node* node)
{
    ast_node* first_node = node->sons[0];
    ast_node* second_node = node->sons[1];
    std::string label1 = newLabel(Function::currentFunc);  // true语句
    TrueLabels.push(label1);

    ast_node *left = ir_visit_ast_node(first_node);
    if (!left) {
        return false;
    }
    node->val = left->val;
    TrueLabels.pop();

    ast_node *right = ir_visit_ast_node(second_node);
    if (!right) {
        return false;
    }

    node->blockInsts.addInst(left->blockInsts);
    if (!(left->val->type == ValueType::TYPE_BOOLEAN)) {
        Value *val = nullptr;
        val = newTempValue(ValueType::TYPE_BOOLEAN, Function::currentFunc);
        Value *constVal = newConstValue(0);
        node->blockInsts.addInst(
        new BinaryIRInst(IRINST_CMP, "ne", val, left->val, constVal)
        );
        node->blockInsts.addInst(
            new GotoIRInst(IRINST_BCJUMP, val, TrueLabels.top(), FalseLabels.top())
        );
        node->val = val;
    }
    node->blockInsts.addInst(
            new StringIRInst(label1 + ":")
    );
    node->blockInsts.addInst(right->blockInsts);
    if (!(right->val->type == ValueType::TYPE_BOOLEAN)) {
        Value *val = nullptr;
        val = newTempValue(ValueType::TYPE_BOOLEAN, Function::currentFunc);
        Value *constVal = newConstValue(0);
        node->blockInsts.addInst(
        new BinaryIRInst(IRINST_CMP, "ne", val, right->val, constVal)
        );
        node->blockInsts.addInst(
            new GotoIRInst(IRINST_BCJUMP, val, TrueLabels.top(), FalseLabels.top())
        );
        node->val = val;
    }
    return true;
}

bool IRGenerator::ir_or(ast_node* node)
{
    ast_node *src1_node = node->sons[0];
    ast_node *src2_node = node->sons[1];
    std::string label1 = newLabel(Function::currentFunc);  // true语句
    FalseLabels.push(label1);
    ast_node *left = ir_visit_ast_node(src1_node);
    if (!left) {
        return false;
    }
    node->val = left->val;
    FalseLabels.pop();
    ast_node *right = ir_visit_ast_node(src2_node);
    if (!right) {
        return false;
    }
    node->blockInsts.addInst(left->blockInsts);
    if (!(left->val->type == ValueType::TYPE_BOOLEAN)) {
        Value *val = nullptr;
        val = newTempValue(ValueType::TYPE_BOOLEAN, Function::currentFunc);
        Value *constVal = newConstValue(0);
        node->blockInsts.addInst(
        new BinaryIRInst(IRINST_CMP, "ne", val, left->val, constVal)
        );
        node->blockInsts.addInst(
            new GotoIRInst(IRINST_BCJUMP, val, TrueLabels.top(), FalseLabels.top())
        );
        node->val = val;
    }
    node->blockInsts.addInst(
            new StringIRInst(label1 + ":")
    );
    node->blockInsts.addInst(right->blockInsts);
    if (!(right->val->type == ValueType::TYPE_BOOLEAN)) {
        Value *val = nullptr;
        val = newTempValue(ValueType::TYPE_BOOLEAN, Function::currentFunc);
        Value *constVal = newConstValue(0);
        node->blockInsts.addInst(
        new BinaryIRInst(IRINST_CMP, "ne", val, right->val, constVal)
        );
        node->blockInsts.addInst(
            new GotoIRInst(IRINST_BCJUMP, val, TrueLabels.top(), FalseLabels.top())
        );
        node->val = val;
    }
    return true;
}

bool IRGenerator::ir_call_func(ast_node* node, bool isleft)
{
    // 第一个节点是函数名
    ast_node* temp_node = node->sons[0];
    ast_node* result = ir_visit_ast_node(temp_node, true);
    if (!result) {
        return false;
    }

    // 这里应该先判断一下参数个数是否匹配
    if (node->sons.size() == 2) {
        // 第一步首先将变量实参复制给临时变量
        for (auto pit = node->sons[1]->sons.begin(); pit != node->sons[1]->sons.end(); ++pit) {
            ast_node *temp = ir_visit_ast_node(*pit);
            node->blockInsts.addInst(temp->blockInsts);
        }
        std::vector<Value *> _srcVal;
        // 第二步将实参加入参数列表
        for (auto pit = node->sons[1]->sons.begin(); pit != node->sons[1]->sons.end(); ++pit) {
            _srcVal.push_back((*pit)->val);
        }
        if (isleft) {
            // 无返回值函数调用
            node->blockInsts.addInst(
            new CallFuncIRInst(result->attr.id, _srcVal, true)
            );
        } else {
            // 有返回值函数调用
            Value *val = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
            node->blockInsts.addInst(
            new CallFuncIRInst(result->attr.id, _srcVal, true, val)
            );
            node->val = val;
        }
    } else {
        // 无参函数调用
        if (isleft) {
            node->blockInsts.addInst(
            new CallFuncIRInst(result->attr.id)
            );
        } else {
            Value *val = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
            node->blockInsts.addInst(
            new CallFuncIRInst(result->attr.id, val)
            );
            node->val = val;
        }
    }
    return true;
}

bool IRGenerator::ir_array(ast_node* node, bool isLocal)
{
    Value *value = nullptr;
    // 标识符子节点
    ast_node* array_id = node->sons[0];
    if (isLocal) {
        // 局部变量
        Function* function = symbolTable->findFuncValue(Function::currentFunc);
        value = function->localValueList[function->localValueNames[function->currentLocal++]];
        symbolTable->addValue(array_id->attr.id, value, Function::currentFunc, true);
    }
    return true;
}

bool IRGenerator::ir_array_index(ast_node* node, bool isLeft)
{
    // 数组索引第一个孩子节点是变量名
    ast_node *src1_node = node->sons[0];
    ast_node *array_name = src1_node;
    ArrName = array_name->attr.id;
    Value *ArrVal = findValue(ArrName, Function::currentFunc, true);
    std::vector<ast_node *>::iterator pIter;
    for (pIter = node->sons.begin() + 1; pIter != node->sons.end(); ++pIter) {
        ast_node *temp;
        temp = ir_visit_ast_node(*pIter, false);
        if (!temp) {
            continue;
        }
        node->blockInsts.addInst(temp->blockInsts);
    }
    Value *Resultval1 = node->sons[1]->val;

    if (node->sons.size() > 2) {
        int i = 0;
        for (pIter = node->sons.begin() + 1; pIter != node->sons.end() - 1; ++pIter, ++i) {
            ast_node *temp1 = *(pIter);
            ast_node *temp2 = *(pIter + 1);
            int dim = ArrVal->dimensions[i + 1];

            if (temp1->val->is_const and temp2->val->is_const) {
                temp2->val->intVal = (temp1->val->intVal * dim + temp2->val->intVal);
                Resultval1 = temp2->val;
            } else {
                Value *val = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
                Value *offsetVal = newConstValue(dim);
                node->blockInsts.addInst(
                    new BinaryIRInst(IRINST_MUL, val, temp1->val, offsetVal)
                );
                Resultval1 = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
                node->blockInsts.addInst(
                    new BinaryIRInst(IRINST_ADD, Resultval1, val, temp2->val)
                );
                temp2->val = Resultval1;
            }
        }
    } else {
    }

    Value *Resultval3;
    int dim = 4;
    if (Resultval1->is_const) {
        Resultval3 = newConstValue(Resultval1->intVal * dim);
    } else {
        Resultval3 = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
        Value *offsetVal = newConstValue(dim);
        node->blockInsts.addInst(
            new BinaryIRInst(IRINST_MUL, Resultval3, Resultval1, offsetVal)
        );
    }

    Value *Resultval2 = newTempValue(ValueType::TYPE_INT_PTR, Function::currentFunc);
    node->blockInsts.addInst(
        new BinaryIRInst(IRINST_ADD, Resultval2, ArrVal, Resultval3)
    );
    node->val = Resultval2;

    if (!isLeft) {
        if (node->sons.size() - 1 >= ArrVal->dimIndex) 
        {
            Value *val = newTempValue(ValueType::TYPE_INT, Function::currentFunc);
            node->blockInsts.addInst(
            new AssignIRInst(val, Resultval2)
            );
            node->val = val;
        }

    }

    return true;
}


