#include "SymbolGenerator.h"
#include <string.h>
bool ExitLabel = false;
int LoopCount = 0;
bool ifcmp = false;

SymbolGenerator::SymbolGenerator(ast_node* _root):root(_root)
{
    sym_handlers[ast_operator_type::AST_OP_TOP] = &SymbolGenerator::sym_top_compile_unit;
    sym_handlers[ast_operator_type::AST_FUNC_DEF] = &SymbolGenerator::sym_func_def;
    sym_handlers[ast_operator_type::AST_SENTENCE_BLOCK] = &SymbolGenerator::sym_sentence_block;
    sym_handlers[ast_operator_type::AST_OP_RETURN] = &SymbolGenerator::sym_return;
    sym_handlers[ast_operator_type::AST_OP_END] = &SymbolGenerator::sym_leaf;
    sym_handlers[ast_operator_type::AST_OP_ASSIGN] = &SymbolGenerator::sym_assign;

    /*二元运算符*/
    sym_handlers[ast_operator_type::AST_OP_ADD] = &SymbolGenerator::sym_op_binary;
    sym_handlers[ast_operator_type::AST_OP_SUB] = &SymbolGenerator::sym_op_binary;
    sym_handlers[ast_operator_type::AST_OP_MUL] = &SymbolGenerator::sym_op_binary;
    sym_handlers[ast_operator_type::AST_OP_DIV] = &SymbolGenerator::sym_op_binary;
    sym_handlers[ast_operator_type::AST_OP_MOD] = &SymbolGenerator::sym_op_binary;

    //while语句
    sym_handlers[ast_operator_type::AST_OP_WHILE] = &SymbolGenerator::sym_while;

    //if语句
    sym_handlers[ast_operator_type::AST_OP_IF] = &SymbolGenerator::sym_if;

    //比较
    sym_handlers[ast_operator_type::AST_OP_CMP] = &SymbolGenerator::sym_compare;

    //for语句
    sym_handlers[ast_operator_type::AST_OP_FOR] = &SymbolGenerator::sym_for;

    //continue语句
    sym_handlers[ast_operator_type::AST_OP_CONTINUE] = &SymbolGenerator::sym_continue;

    //break语句
    sym_handlers[ast_operator_type::AST_OP_BREAK] = &SymbolGenerator::sym_break;

    //取负指令
    sym_handlers[ast_operator_type::AST_OP_NEG] = &SymbolGenerator::sym_neg;

    //逻辑非运算
    sym_handlers[ast_operator_type::AST_OP_NOT] = &SymbolGenerator::sym_not;

    //自增自减运算(左右)
    sym_handlers[ast_operator_type::AST_OP_LINC] = &SymbolGenerator::sym_inc_dec;
    sym_handlers[ast_operator_type::AST_OP_RINC] = &SymbolGenerator::sym_inc_dec;
    sym_handlers[ast_operator_type::AST_OP_LDEC] = &SymbolGenerator::sym_inc_dec;
    sym_handlers[ast_operator_type::AST_OP_RDEC] = &SymbolGenerator::sym_inc_dec;

    //逻辑与-逻辑或
    sym_handlers[ast_operator_type::AST_OP_AND] = &SymbolGenerator::sym_logic;
    sym_handlers[ast_operator_type::AST_OP_OR] = &SymbolGenerator::sym_logic;

    //函数调用
    sym_handlers[ast_operator_type::AST_FUNC_CALL] = &SymbolGenerator::sym_call_func;

    //数组下标索引
    sym_handlers[ast_operator_type::AST_ARRAY_INDEX] = &SymbolGenerator::sym_array_index;
}

/// @brief 添加内置函数
void add_func_build_in()
{
    Function* function = newFunction("putint", ValueType::TYPE_VOID);
    Value* value = new CustomValue("k", ValueType::TYPE_INT); 
    function->argsList.push_back(value);

    function = newFunction("getint", ValueType::TYPE_INT);
    function = newFunction("putch", ValueType::TYPE_VOID);
    function->argsList.push_back(value);
    function = newFunction("getch", ValueType::TYPE_INT);
    function = newFunction("putarray", ValueType::TYPE_VOID);
    function->argsList.push_back(value);
    function->argsList.push_back(value);
    function = newFunction("getarray", ValueType::TYPE_VOID);
    function->argsList.push_back(value);
}

/// @brief 遍历抽象语法树形成符号表，进行语义分析
/// @return 
bool SymbolGenerator::run()
{
    ast_node* node;

    //为符号表添加内置函数
    add_func_build_in();

    // 从根节点进行遍历
    node = sym_visit_ast_node(root);

    return node != nullptr; 
}

ast_node* SymbolGenerator::sym_visit_ast_node(ast_node* node, bool isLeft, bool isLast)
{
    if(node == nullptr)
    {
        return nullptr;
    }

    bool result;

    std::unordered_map<ast_operator_type, sym_handler_t>::const_iterator pit;
    pit = sym_handlers.find(node->type);
    if(pit != sym_handlers.end())
    {
        sym_handler_t function = pit->second;
        result = (this->*function)(node);
    }
    else
    {
        switch(node->type)
        {
            case AST_DEF_LIST:
                result = sym_def_list(node, isLeft);
                break;
            default:
                result = false;
                break;
        }
    }
    
    if(!result)
    {
        node = nullptr;
    }
    
    return node;
}

bool SymbolGenerator::sym_top_compile_unit(ast_node* node)
{
    std::vector<ast_node*>::iterator pit;
    
    for(pit = node->sons.begin();pit != node->sons.end();++pit)
    {
        ast_node* pit_node = *pit;
        ast_node* temp = sym_visit_ast_node(pit_node);
        if(!temp)
        {
            return false;
        }
    }

    return true;
}

bool SymbolGenerator::sym_func_def(ast_node* node)
{
    ast_node* return_type = node->sons[0];

    ast_node* funcName = node->sons[1];
    Function::currentFunc = funcName->attr.id;

    if(Function::is_build_func(Function::currentFunc))
    {
        return true;
    }

    Function* function = nullptr;
    if(!GlobalIsExist(Function::currentFunc))
    {
        function = newFunction(Function::currentFunc);
    }
    else
    {
        std::string ERROR = std::string("错误:重定义") + std::string(Function::currentFunc);
        std::cout << funcName->attr.lineno << " " << ERROR << std::endl;
        return false;
    }

    funcName->func = function;
    LocalBlock* lcalBlock = new LocalBlock();
    function->stack.push(lcalBlock);
    function->currentScope = 0;

    ast_node* funcParasList = node->sons[2];

    std::vector<ast_node*>::iterator pit = funcParasList->sons.end() - 1;
    for(;pit != funcParasList->sons.begin() - 1;--pit)
    {
        ast_node *arg_ID = (*pit)->sons[1];
        if(arg_ID->type != AST_ARRAY)
        {
            newTempValue(ValueType::TYPE_INT, Function::currentFunc, true);
        }
        else
        {
            sym_array_arglist(arg_ID, true);
        }
    }

    for(pit = funcParasList->sons.end() - 1;pit != funcParasList->sons.begin() - 1;--pit)
    {
        ast_node* arg_ID = (*pit)->sons[1];
        if(arg_ID->type != AST_ARRAY)
        {
            Value* localCustomValue = nullptr;
            localCustomValue = newLocalCustomValue(arg_ID->attr.id, ValueType::TYPE_INT, Function::currentFunc);
            arg_ID->val = localCustomValue;
        }
        else
        {
            sym_array_arglist(arg_ID);
        }
    }   
    
    /*返回值问题*/
    if(strcmp(return_type->attr.id, "int") == 0)
    {
        newLocalCustomValue("return", ValueType::TYPE_INT, Function::currentFunc);
    }
    else if(strcmp(return_type->attr.id, "void") == 0)
    {
        funcName->func->return_type = ValueType::TYPE_VOID;
    }

    function->currentScope = -1;
    ast_node* funcBlock = sym_visit_ast_node(node->sons[3]);

    if(!funcBlock)
    {
        return false;
    }
    return true;
}

bool SymbolGenerator::sym_sentence_block(ast_node* node)
{
    LocalBlock* lcalBlock = new LocalBlock();
    Function* function = symbolTable->findFuncValue(Function::currentFunc);
    if(function->currentScope != -1)
    {
        function->stack.push(lcalBlock);
    }

    ++function->currentScope;
    std::vector<ast_node*>::iterator it;
    for(it = node->sons.begin();it != node->sons.end();++it)
    {
        ast_node* temp = sym_visit_ast_node(*it, true, true);
        if(!temp)
        {
            return false;
        }
    }

    function->stack.pop(function->currentScope);
    --function->currentScope;
    return true;
}

bool SymbolGenerator::sym_return(ast_node* node)
{
    if(node->sons.size() > 0)
    {
        ast_node* tempNode = node->sons[0];
        ast_node* result = sym_visit_ast_node(tempNode);
        if(!result)
        {
            return false;
        }
    }
    else
    {
        Function* function = symbolTable->findFuncValue(Function::currentFunc);
        if(function->return_type != ValueType::TYPE_VOID)
        {
            std::cout << node->attr.lineno << " " << "int型函数必须有返回值";
            return false; 
        }
    }
    return true;
}

bool SymbolGenerator::sym_leaf(ast_node* node)
{
    Value* value = nullptr;
    Function* function = nullptr;

    if(node->attr.kind == DIGIT_ID)
    {
        value = symbolTable->findLocalValue(node->attr.id, Function::currentFunc, false);
        function = symbolTable->findFuncValue(node->attr.id);
        if((!value) && (!function))
        {
            std::cout << "Line " << node->attr.lineno << " Value " << node->attr.id << " isn't defined" << std::endl;
            return false;
        }
    }

    return true;
}

bool SymbolGenerator::sym_def_list(ast_node* node, bool isLocal)
{
    ast_node* type_node = node->sons[0];  //类型节点
    
    /*接下来的定义变量的类型*/
    numdigit_variety EveryKind;
    ValueType EveryType;

    
    if(strcmp(type_node->attr.id, "int") == 0)
    {
        EveryKind = DIGIT_ID;
        EveryType = ValueType::TYPE_INT;
    }

    for(auto pit = node->sons.begin() + 1;pit != node->sons.end();++pit)
    {
        ast_node* tempNode = *pit;
        Value* value = nullptr;


        /*首先处理带有变量初始化的定义情况*/
        if(tempNode->type == AST_OP_ASSIGN)
        {
            ast_node* temp_tempNode = tempNode->sons[0];
            if(isLocal)
            {
                if(!LocalIsExist(Function::currentFunc, temp_tempNode->attr.id))
                {
                    value = newLocalCustomValue(temp_tempNode->attr.id, EveryType, Function::currentFunc);
                    if(!sym_assign(tempNode))
                    {
                        return false;
                    }
                }
                else
                {
                    std::string error = std::string("error: redefinition of ") + std::string(temp_tempNode->attr.id);
                    printError(temp_tempNode->attr.lineno, error);
                    return false;
                }
            }
            else
            {
                if (!GlobalIsExist(temp_tempNode->attr.id)) 
                {
                    value = newCustomValue(temp_tempNode->attr.id);
                    value->IsGlobalAssigned = true;
                    if(tempNode->sons[1]->type != AST_OP_NEG)
                    {
                        value->GlobalAssignVal = tempNode->sons[1]->attr.integer_val;
                    }
                    else
                    {
                        int tmp_num = tempNode->sons[1]->sons[0]->attr.integer_val;
                        value->GlobalAssignVal = -tmp_num;
                    }
                } 
                else
                {
                    std::string error = std::string("error: redefinition of ") + std::string(temp_tempNode->attr.id);
                    printError(temp_tempNode->attr.lineno, error);
                    return false;
                }
            }
        }

        if(tempNode->type == AST_ARRAY)  //处理数组情况
        {
            bool result = sym_array(tempNode, EveryType, isLocal);
            if(!result)
            {
                return false;
            }
        }
        else if(tempNode->type != AST_OP_ASSIGN)  //非数组情况以及定义初始化情况
        {
            if(isLocal)  //局部
            {
                if(!LocalIsExist(Function::currentFunc, tempNode->attr.id))
                {
                    value = newLocalCustomValue(tempNode->attr.id, EveryType, Function::currentFunc);
                }
                else
                {
                    std::string error = std::string("error: redefinition of ") + std::string(tempNode->attr.id);
                    printError(tempNode->attr.lineno, error);
                    return false;
                }
            }
            else   //全局
            {
                if (!GlobalIsExist(tempNode->attr.id)) 
                {
                    value = newCustomValue(tempNode->attr.id);
                } 
                else
                {
                    std::string error = std::string("error: redefinition of ") + std::string(tempNode->attr.id);
                    printError(tempNode->attr.lineno, error);
                    return false;
                }
            }

            tempNode->attr.kind = EveryKind;
            tempNode->val = value;
        }

        if(tempNode->type != AST_OP_ASSIGN)
        {
            tempNode->val->type = EveryType;
        }
    }

    return true;
}

bool SymbolGenerator::sym_assign(ast_node* node)
{
    ast_node* left_node = node->sons[0];
    ast_node* right_node = node->sons[1];

    // 赋值运算符的左侧操作数
    ast_node *left = sym_visit_ast_node(left_node, true);
    if (!left) {
        return false;
    }

    // 赋值运算符的右侧操作数
    ast_node *right = sym_visit_ast_node(right_node);
    if (!right) {
        return false;
    }
    return true;
}

bool SymbolGenerator::sym_op_binary(ast_node* node)
{
    ast_node* left_node = node->sons[0];
    ast_node* right_node = node->sons[1];

    // 赋值运算符的左侧操作数
    ast_node *left = sym_visit_ast_node(left_node);
    if (!left) {
        return false;
    }

    // 赋值运算符的右侧操作数
    ast_node *right = sym_visit_ast_node(right_node);
    if (!right) {
        return false;
    }
    return true;
}

bool SymbolGenerator::sym_while(ast_node* node)
{
    ast_node* first_node = node->sons[0];
    ast_node* second_node = node->sons[1];

    ++LoopCount;

    ast_node *left = sym_visit_ast_node(first_node);
    if (!left) {
        return false;
    }

    // 赋值运算符的右侧操作数
    ast_node *right = sym_visit_ast_node(second_node);
    if (!right) {
        return false;
    }

    --LoopCount;

    return true;
}

bool SymbolGenerator::sym_if(ast_node* node)
{
    ifcmp = true;
    ast_node *first_node = node->sons[0];
    ast_node *second_node = node->sons[1];
    ast_node *condition = sym_visit_ast_node(first_node);
    if (!condition) 
    {
        return false;
    }
    ast_node *true_node = sym_visit_ast_node(second_node, true);
    if (!true_node) 
    {
        return false;
    }
    if (node->sons.size() == 3) {
        ast_node *third_node = node->sons[2];
        ast_node *false_node = sym_visit_ast_node(third_node, true);
        if (!false_node) {
            return false;
        }
    }

    return true;
}

bool SymbolGenerator::sym_compare(ast_node* node)
{
    ast_node *left = sym_visit_ast_node(node->sons[0]);
    if (!left) 
    {
        return false;
    }

    ast_node *right = sym_visit_ast_node(node->sons[2]);
    if (!right) 
    {
        return false;
    }

    return true;
}

bool SymbolGenerator::sym_for(ast_node* node)
{
    LoopCount++;
    for (auto pit = node->sons.begin(); pit != node->sons.end(); ++pit) {
        ast_node *temp;
        temp = sym_visit_ast_node(*pit, true, true);
        if (!temp) {
            return false;
        }
    }
    LoopCount--;
    return true;
}

bool SymbolGenerator::sym_continue(ast_node* node)
{
    if(LoopCount != 0)
    {
        return true;
    }
    printError(node->attr.lineno, std::string("语义错误:>continue语句必须在循环中"));
    return false;
}

bool SymbolGenerator::sym_break(ast_node* node)
{
    if(LoopCount != 0)
    {
        return true;
    }
    printError(node->attr.lineno, std::string("语义错误:>break语句必须在循环中"));
    return false;
}

bool SymbolGenerator::sym_neg(ast_node* node)
{
    ast_node* son = node->sons[0];
    ast_node* result = sym_visit_ast_node(son);
    if (!result) 
    {
        return false;
    }
    return true;
}

bool SymbolGenerator::sym_not(ast_node* node)
{
    ast_node* son = node->sons[0];
    ast_node* result = sym_visit_ast_node(son);
    if (!result) {
        return false;
    }
    return true;
}

bool SymbolGenerator::sym_inc_dec(ast_node* node)
{
    ast_node* son = node->sons[0];
    ast_node* result = sym_visit_ast_node(son, true);
    if (!result) {
        return false;
    }
    return true;
}

bool SymbolGenerator::sym_logic(ast_node* node)
{
    ast_node* first_node = node->sons[0];
    ast_node* second_node = node->sons[1];
    ast_node *result = sym_visit_ast_node(first_node, true);
    if (!result) 
    {
        return false;
    }

    result = sym_visit_ast_node(second_node, true);
    if (!result) 
    {
        return false;
    }

    return true;
}

bool SymbolGenerator::sym_call_func(ast_node* node)
{
    // 第一个节点是函数名
    ast_node* temp_node = node->sons[0];
    ast_node* result = sym_visit_ast_node(temp_node, true);
    if (!result) 
    {
        return false;
    }

    if (node->sons.size() == 2) {
        // 参数个数检查
        ast_node *fargs_node = node->sons[1];

        for (auto pit = fargs_node->sons.begin(); pit != fargs_node->sons.end(); ++pit) {
            ast_node *temp;
            temp = sym_visit_ast_node(*pit, true, true);
            if (!temp) {
                return false;
            }
        }
        Function* function = symbolTable->findFuncValue(result->attr.id);
        if (fargs_node->sons.size() != function->argsList.size()) {
            std::string error = "<语义错误> " + std::string(result->attr.id) + " 形参实参不匹配";
            printError(temp_node->attr.lineno, error);
            return false;
        }
    } else {
        Function* function = symbolTable->findFuncValue(result->attr.id);
        if (function->argsList.size() > 0) {
            std::string error = "<语义错误> " + std::string(result->attr.id) + " 形参实参不匹配";
            printError(temp_node->attr.lineno, error);
            return false;
        }
    }
    return true;
}

bool SymbolGenerator::sym_array(ast_node* node, ValueType type, bool isLocal)
{
    Value *value = nullptr;
    // 第一个孩子是标识符
    ast_node* array_id = node->sons[0];
    if (isLocal) {
        // 局部变量
        if (!LocalIsExist(Function::currentFunc, array_id->attr.id)) 
        {
            value = newLocalCustomValue(array_id->attr.id, type, Function::currentFunc);

        } 
        else 
        {
            // 若变量名已存在，则报错重定义
            std::string error = std::string("error: redefinition of ") + std::string(array_id->attr.id);
            printError(array_id->attr.lineno, error);
            return false;
        }
    } 
    else 
    {
        // 全局变量
        if (!GlobalIsExist(array_id->attr.id)) {
            value = newCustomValue(array_id->attr.id);
        } 
        else 
        {
            // 若变量名已存在，则报错重定义
            std::string error = std::string("error: redefinition of ") + std::string(array_id->attr.id);
            printError(array_id->attr.lineno, error);
            return false;
        }
    }

    value->type = type;

    ast_node* array_dimensions = node->sons[1];
    // 第二个孩子是数组维度节点
    while (array_dimensions) {
        value->dimensions.push_back(array_dimensions->sons[0]->attr.integer_val);
        value->dimIndex++;
        if (array_dimensions->sons.size() > 1) {
            array_dimensions = array_dimensions->sons[1];
        } 
        else 
        {
            array_dimensions = nullptr;
        }
    }
    node->val = value;
    return true;
}

bool SymbolGenerator::sym_array_index(ast_node* node)
{
    // 第一个孩子节点是变量名
    ast_node* first_node = node->sons[0];
    ast_node* array_name = sym_visit_ast_node(first_node);
    if (!array_name) 
    {
        return false;
    }

    ast_node* dimensions = node->sons[1];
    // 第二个孩子是数组维度节点
    ast_node *right = sym_visit_ast_node(dimensions);
    if (!right) 
    {
        return false;
    }
    return true;
}

bool SymbolGenerator::sym_array_arglist(ast_node* node, bool istemp)
{
    ast_node *arr_name = node->sons[0];
    Value* localCustomValue = nullptr;
    if (istemp) 
    {
        localCustomValue = newTempValue(ValueType::TYPE_INT, Function::currentFunc, true);
    } 
    else 
    {
        localCustomValue = newLocalCustomValue(arr_name->attr.id, ValueType::TYPE_INT, Function::currentFunc);
    }

    ast_node* dim_node = node->sons[1];

    while (true) 
    {
        localCustomValue->dimensions.push_back(dim_node->attr.integer_val);
        if(dim_node->sons.size() == 0)
        {
            break;
        }
        dim_node = dim_node->sons[1];
    }

    node->val = localCustomValue;
    return true;
}
