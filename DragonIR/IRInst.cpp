#include "IRInst.h"

/// @brief 构造函数
/// @param _op 指令类型
/// @param _result 结果操作数
IRInst::IRInst(IROperator _op, Value* _result): op(_op), dstValue(_result)
{}

IRInst::IRInst(IROperator _op, Function* _func):op(_op), func(_func)
{}

IRInst::IRInst()
{
    op = IRINST_NONE;
}

IRInst::~IRInst(){}

IROperator IRInst::getOP()
{
    return op;
}

std::string IRInst::getLabel()
{
    return ".L1";
}

/// @brief 获得源操作数列表
/// @return 
std::vector<Value*>& IRInst::getSrc()
{
    return srcValues;
}

/// @brief 获得目的操作数
/// @return 
Value* IRInst::getDst()
{
    return dstValue;
}

void IRInst::toString(std::string& str)
{
    str = "Unknown IR";
}

/// @brief 有条件跳转指令
/// @param op 操作码
/// @param src1 源操作数
/// @param label1 跳转label1
/// @param label2 跳转label2
GotoIRInst::GotoIRInst(IROperator _op, Value *_src1, std::string _label1, std::string _label2) :
    IRInst(_op, _src1)
{

    labelA = _label1;
    labelB = _label2;
}
/// @brief 无条件跳转指令
/// @param op 操作码
/// @param label 跳转label
GotoIRInst::GotoIRInst(IROperator _op, std::string _label) :
    IRInst(_op, (Value*)nullptr)
{
    dstValue = nullptr;
    labelA = _label;
}
/// @brief 析构函数
GotoIRInst::~GotoIRInst()
{

}
/// @brief 转换成字符串显示
/// @param str 转换后的字符串
void GotoIRInst::toString(std::string &str)
{

    str = "    ";
    switch (op) {
    case IRINST_BRJUMP:
        str += "br label " + labelA;
        break;
    case IRINST_BCJUMP: {
        Value *src1 = dstValue;
        str += "bc " + src1->getName() + ", label " + labelA + ", label " + labelB;
        break;
    }
    default:
        break;
    }
}

/// @brief 赋值IR指令
/// @param _result 
/// @param _srcVal1 
AssignIRInst::AssignIRInst(Value *_result, Value *_srcVal1, bool _check_ptr) :
    IRInst(IRINST_OP_ASSIGN, _result)
{
    check_ptr = _check_ptr;
    srcValues.push_back(_srcVal1);
}

/// @brief 析构函数
AssignIRInst::~AssignIRInst()
{

}

/// @brief 转换成字符串显示
/// @param str 转换后的字符串
void AssignIRInst::toString(std::string &str)
{
    Value *src1 = srcValues[0], *result = dstValue;
    str = "    ";
    std::string resultName, srcName;
    // 指针引用需要加 * 号
    if (result->type == ValueType::TYPE_INT_PTR) {
        if (check_ptr) {
            resultName = result->getName();
        } else {
            resultName = "*" + result->getName();
        }

    } else {
        resultName = result->getName();
    }
    if (src1->type == ValueType::TYPE_INT_PTR) {
        srcName = "*" + src1->getName();
    } else {
        srcName = src1->getName();
    }
    str += resultName + " = " + srcName;
}


// 函数定义指令
DefineFuncIRInst::DefineFuncIRInst(Function *_func_name, ValueType _return_type) :
    IRInst(IRINST_OP_FUNC_DEF, _func_name), return_type(_return_type)
{
    srcValues = _func_name->argsList;

}


/// @brief 析构函数
DefineFuncIRInst::~DefineFuncIRInst()
{

}
void DefineFuncIRInst::toString(std::string &str)
{
    //暂时只考虑int型
    Function *func_name = func;
    std::string type;
    if (return_type == ValueType::TYPE_INT) {
        type = "i32";
    } else if (return_type == ValueType::TYPE_VOID) {
        type = "void";
        printf("void 返回类型\n\n");
    }
    str = "define " + type + " @" + func_name->getName() + "(";
    for (size_t k = 0; k < srcValues.size(); ++k) {
        str += "i32 " + srcValues[k]->getName();

        if (srcValues[k]->dimensions.size() > 0) {
            str += "[0]";
            for (int i = 1; i < srcValues[k]->dimensions.size(); i++) {
                str += "[" + int2str(srcValues[k]->dimensions[i]) + "]";
            }
        }

        if (k != (srcValues.size() - 1)) {

            str += ", ";
        }
    }
    str += ") {";
}

/// @brief  无用指令
/// @param  
StringIRInst::StringIRInst(std::string str) :
    useless_str(str)
{
    op = IRINST_STRING;
}

/// @brief 析构函数
StringIRInst::~StringIRInst()
{

}
std::string StringIRInst::getLabel()
{
    return useless_str.substr(0, useless_str.size() - 1);
}

/// @brief 转换成字符串显示
/// @param str 转换后的字符串
void StringIRInst::toString(std::string &str)
{
    str = useless_str;
}

/// @brief 一元运算，包括函数退出指令 exit %t8 和取负运算 a = neg %t8
/// @param op 指令码
/// @param result 目的操作数
/// @param src 源操作数
SingleIRInst::SingleIRInst(IROperator _op, Value *_result, Value *_srcVal1) :
    IRInst(_op, _result)
{
    srcValues.push_back(_srcVal1);
}
/// @brief void函数的退出语句 exit
/// @param op 
SingleIRInst::SingleIRInst(IROperator _op) :
    IRInst(_op, (Value*)nullptr)
{}
/// @brief 析构函数
SingleIRInst::~SingleIRInst()
{

}


/// @brief 转换成字符串显示
/// @param str 转换后的字符串
void SingleIRInst::toString(std::string &str)
{
    Value *result = dstValue, *src;
    str = "    ";
    switch (op) {
    case IRINST_OP_NEG:
        src = srcValues[0];
        str += result->getName() + " = neg " + src->getName();
        break;
    case IRINST_OP_RETURN:
        if (srcValues.size() > 0) {
            src = srcValues[0];
            str += "exit " + src->getName();
        } else {
            str += "exit";
        }
        break;
    default:
        break;
    }
}

// 变量声明指令
DeclareIRInst::DeclareIRInst(Value *_result, bool _IsGlobal, bool _IsArray, bool _IsTemp, bool _IsGlobalAssign) :
    IRInst(IRINST_VAR_DEF, _result)
{
    IsGlobal = _IsGlobal;
    IsArray = _IsArray;
    IsTemp = _IsTemp;
    IsGlobalAssign = _IsGlobalAssign;
    srcValues.push_back(_result);
}

/// @brief 析构函数
DeclareIRInst::~DeclareIRInst()
{

}
void DeclareIRInst::toString(std::string &str)
{
    // todo 暂时只考虑int型
    Value *result = dstValue;
    std::string type = result->getType();
    // type = "i32";
    if (IsGlobal) {
        // 全局变量用@符号
        str = "declare " + type + " " + result->getName();
        if(IsGlobalAssign)
        {
            str += " = ";
            str += int2str(dstValue->GlobalAssignVal);
        }
    } else {
        // 局部变量用%l符号
        str = "    declare " + type + " " + result->getName();

    }
    if (IsArray) 
    {
        // 数组
        for (int i = 0; i < result->dimensions.size(); i++) {
            str += "[" + int2str(result->dimensions[i]) + "]";
        }
    }
    if (!IsGlobal && !IsTemp) {
        str += " ; variable: " + result->id_name;
        /*
        if (IsArray) 
        {
            // 数组
            for (int i = 0; i < result->dimensions.size(); i++) {
                str += "[" + int2str(result->dimensions[i]) + "]";
            }
        }
        */
    }
}

/// @brief 构造函数
BinaryIRInst::BinaryIRInst(IROperator _op, Value *_result, Value *_srcVal1, Value *_srcVal2) :
    IRInst(_op, _result)
{
    srcValues.push_back(_srcVal1);
    srcValues.push_back(_srcVal2);
}
BinaryIRInst::BinaryIRInst(IROperator _op, std::string _cmp, Value *_result, Value *_srcVal1, Value *_srcVal2) :
    IRInst(_op, _result)
{
    cmp = _cmp;
    srcValues.push_back(_srcVal1);
    srcValues.push_back(_srcVal2);
}

/// @brief 析构函数
BinaryIRInst::~BinaryIRInst(){}

/// @brief 转换成字符串
/// @param str 转换后的字符串
void BinaryIRInst::toString(std::string &str)
{

    Value *src1 = srcValues[0], *src2 = srcValues[1], *result = dstValue;
    /*   新增   */
    // 只需要判断结果类型即可判断是浮点运算还是整数运算
    str = "    ";
    switch (op) {
    case IRINST_ADD:
        // 加法指令，二元运算
        if (result->type == ValueType::TYPE_REAL) {
            // 浮点运算
            str += result->getName() + " = fadd " + src1->getName() + ", " + src2->getName();
        } else {
            str += result->getName() + " = add " + src1->getName() + ", " + src2->getName();
        }
        break;
    case IRINST_MOD:
        // 取余指令，二元运算
        str += result->getName() + " = mod " + src1->getName() + ", " + src2->getName();
        break;
    case IRINST_MUL:

        // 乘法指令，二元运算
        if (result->type == ValueType::TYPE_REAL) {
            // 浮点运算
            str += result->getName() + " = fmul " + src1->getName() + ", " + src2->getName();
        } else {
            str += result->getName() + " = mul " + src1->getName() + ", " + src2->getName();
        }
        break;
    case IRINST_SUB:

        // 减法指令，二元运算
        if (result->type == ValueType::TYPE_REAL) {
            // 浮点运算
            str += result->getName() + " = fsub " + src1->getName() + ", " + src2->getName();
        } else {
            str += result->getName() + " = sub " + src1->getName() + ", " + src2->getName();
        }
        break;
    case IRINST_DIV:

        // 除法指令，二元运算
        if (result->type == ValueType::TYPE_REAL) {
            // 浮点运算
            str += result->getName() + " = fdiv " + src1->getName() + ", " + src2->getName();
        } else {
            str += result->getName() + " = div " + src1->getName() + ", " + src2->getName();
        }
        break;
    case IRINST_CMP:
        str += result->getName() + " = icmp " + cmp + " " + src1->getName() + ", " + src2->getName();
        break;
    default:
        // 未知指令
        IRInst::toString(str);
        break;
    }
}

/// @brief 无参数的函数调用
/// @param name 函数名
/// @param result 保存返回值的Value
CallFuncIRInst::CallFuncIRInst(std::string _name) :
    IRInst(IRINST_OP_FUNC_CALL, (Value*)nullptr), name(_name)
{}
/// @brief 无参数的函数调用
/// @param name 函数名
/// @param result 保存返回值的Value
CallFuncIRInst::CallFuncIRInst(std::string _name, Value *_result) :
    IRInst(IRINST_OP_FUNC_CALL, _result), name(_name)
{}

/// @brief 含参函数调用
/// @param _name 函数名
/// @param _srcVal1 参数
/// @param _paras 含参
/// @param _result 返回值
CallFuncIRInst::CallFuncIRInst(std::string _name, Value *_srcVal1, bool _paras, Value *_result) :
    IRInst(IRINST_OP_FUNC_CALL, _result), name(_name)
{
    srcValues.push_back(_srcVal1);
}

/// @brief 含参函数调用
/// @param _name 函数名
/// @param _srcVal 参数列表
/// @param _paras 含参
/// @param _result 返回值
CallFuncIRInst::CallFuncIRInst(std::string _name, std::vector<Value *> &_srcVal, bool _paras, Value *_result) :
    IRInst(IRINST_OP_FUNC_CALL, _result), name(_name)
{
    // 实参拷贝
    srcValues = _srcVal;
}

/// @brief 析构函数
CallFuncIRInst::~CallFuncIRInst()
{

}


/// @brief 转换成字符串显示
/// @param str 转换后的字符串
void CallFuncIRInst::toString(std::string &str)
{
    Value *result = dstValue;

    // TODO 这里应该根据函数名查找函数定义或者声明获取函数的类型
    // 这里假定所有函数返回类型要么是i32，要么是void
    // 函数参数的类型是i32
    str = "    ";
    if (!dstValue) {

        // 函数没有返回值设置
        str += "call void @" + name + "(";
    } else {

        // 函数有返回值要设置到结果变量中
        str += result->getName() + " = call i32 @" + name + "(";
    }

    for (size_t k = 0; k < srcValues.size(); ++k) {
        std::string type = "i32 ";
        if (srcValues[k]->type == ValueType::TYPE_INT_PTR) {
            type = "i32* ";
        }
        str += type + srcValues[k]->getName();

        if (k != (srcValues.size() - 1)) {

            str += ", ";
        }
    }

    str += ")";
}