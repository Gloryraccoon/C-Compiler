#pragma once
#include <vector>
#include <string>
#include "SymbolTable.h"

typedef enum IROperator
{
    IRINST_OP_RETURN,   //函数返回指令
    IRINST_OP_FUNC_DEF,   //函数定义指令
    IRINST_OP_FUNC_CALL,   //函数调用指令
    IRINST_VAR_DEF,      //变量定义指令

    IRINST_BRJUMP,    //无条件跳转
    IRINST_BCJUMP,    //有条件跳转

    IRINST_STRING,   //无效指令
    IRINST_NONE,    //未知指令

    IRINST_OP_ASSIGN, //赋值指令

    IRINST_OP_NEG,   //取负指令

    /*二元运算指令*/
    IRINST_ADD,
    IRINST_SUB,
    IRINST_MUL,
    IRINST_DIV,
    IRINST_MOD,

    IRINST_CMP,

}IROperator;


/// @brief IR基类
class IRInst
{
public:
    /// @brief 构造函数
    IRInst();

    /// @brief 构造函数
    /// @param op 
    /// @param val 
    IRInst(IROperator op, Value* val = nullptr);

    /// @brief 构造函数
    /// @param op 
    /// @param func 
    IRInst(IROperator op, Function* func = nullptr);

    /// @brief 析构函数
    virtual ~IRInst();

    /// @brief 获得指令操作码
    /// @return 
    IROperator getOP();

    /// @brief 获得跳转label
    /// @return 
    virtual std::string getLabel();

    /// @brief 获取源操作数
    /// @return 
    std::vector<Value*>& getSrc();

    /// @brief 获得结果操作数
    /// @return 
    Value* getDst();

    virtual void toString(std::string& str);

public:
    std::string labelA;
    std::string labelB;

protected:
    ///@brief 指令op
    IROperator op;

    /// @brief  源操作数列表
    std::vector<Value*> srcValues;

    /// @brief 目的操作数/跳转指令指向目标
    Value* dstValue;

    /// @brief 指令对应的函数对象
    Function* func;
};

/// @brief 变量声明指令
class DeclareIRInst : public IRInst
{
public:
    DeclareIRInst(Value* result, bool IsGlobal = true, bool IsArray = false, bool IsTemp = false, bool IsGlobalAssign = false);

    virtual ~DeclareIRInst() override;

    /// @brief 转换出字符串
    /// @param str 
    void toString(std::string& str) override;

public:
    bool IsGlobal = true;
    bool IsArray = false;
    bool IsTemp = false;
    bool IsGlobalAssign = false;
};

/// @brief 跳转指令
class GotoIRInst : public IRInst {
public:
    /// @brief 有条件跳转指令
    /// @param op 操作码
    /// @param src1 源操作数
    /// @param label1 跳转label1
    /// @param label2 跳转label2
    GotoIRInst(IROperator op, Value *src1, std::string label1, std::string label2);

    /// @brief 无条件跳转指令
    /// @param op 操作码
    /// @param label 跳转label
    GotoIRInst(IROperator op, std::string label);
    /// @brief 析构函数
    virtual ~GotoIRInst() override;

    /// @brief 转换成字符串
    void toString(std::string &str) override;
};

/// @brief 赋值指令或者说复制指令
class AssignIRInst : public IRInst {

public:
    bool check_ptr = false;
    /// @brief 构造函数
    /// @param result 
    /// @param srcVal1 
    AssignIRInst(Value *result, Value *srcVal1, bool check_ptr = false);

    /// @brief 析构函数
    virtual ~AssignIRInst() override;

    /// @brief 转换成字符串
    void toString(std::string &str) override;

};

/// @brief 函数定义指令
class DefineFuncIRInst : public IRInst {
protected:

    /// @brief 函数名
    std::string name;

public:
    bool IsGlobal = true;
    bool IsArray = false;
    // 返回值类型
    ValueType return_type;

    /// @brief 构造函数
    /// @param func_name 函数符号
    /// @param ret_type 返回类型
    DefineFuncIRInst(Function *func_name, ValueType return_type = ValueType::TYPE_INT);

    /// @brief 析构函数
    virtual ~DefineFuncIRInst() override;

    /// @brief 转换成字符串
    void toString(std::string &str) override;
};

/// @brief 无用指令
class StringIRInst : public IRInst {
private:
    std::string useless_str;
public:
    /// @brief 构造函数
    /// @param str 没啥用字符串 
    StringIRInst(std::string str);
    std::string getLabel() override;
    /// @brief 析构函数
    virtual ~StringIRInst() override;

    /// @brief 转换成字符串
    void toString(std::string &str) override;

};

/// @brief 一元运算指令
class SingleIRInst : public IRInst {

public:

    /// @brief 一元运算，比如取负运算 a = neg %t0
    /// @param op 指令码
    /// @param result 目的操作数
    /// @param src 源操作数
    SingleIRInst(IROperator op, Value *result, Value *src);

    /// @brief void函数的退出语句 exit
    /// @param op 
    SingleIRInst(IROperator op);
    /// @brief 析构函数
    virtual ~SingleIRInst() override;

    /// @brief 转换成字符串
    void toString(std::string &str) override;

};

class BinaryIRInst : public IRInst
{
public:
    std::string cmp;

    /// @brief 构造函数
    BinaryIRInst(IROperator op, Value *result, Value *srcVal1, Value *srcVal2);

    /// @brief 跳转指令
    /// @param op 操作码
    /// @param cmp 比较运算符 lt,gt,ne,eq等
    /// @param result 目的操作数
    /// @param srcVal1 源操作数1
    /// @param srcVal2 源操作数2
    BinaryIRInst(IROperator op, std::string cmp, Value *result, Value *srcVal1, Value *srcVal2);
    /// @brief 析构函数
    virtual ~BinaryIRInst() override;

    /// @brief 转换成字符串
    void toString(std::string &str) override;

};

class CallFuncIRInst : public IRInst
{
protected:

    /// @brief 函数名
    std::string name;

public:

    /// @brief 无参数并且没有要设置的返回值的函数调用
    /// @param name 函数名
    CallFuncIRInst(std::string _name);
    /// @brief 无参数有返回值的函数调用
    /// @param name 函数名
    CallFuncIRInst(std::string _name, Value *_result);


    /// @brief 含参函数调用
    /// @param _name 函数名
    /// @param _srcVal1 参数
    /// @param _paras 含参
    /// @param _result 返回值
    CallFuncIRInst(std::string _name, Value *_srcVal1, bool _paras, Value *_result = nullptr);


    /// @brief 含参函数调用
    /// @param _name 函数名
    /// @param _srcVal 参数列表
    /// @param _paras 含参
    /// @param _result 返回值
    CallFuncIRInst(std::string _name, std::vector<Value *> &_srcVal, bool _paras, Value *_result = nullptr);

    /// @brief 析构函数
    virtual ~CallFuncIRInst() override;

    /// @brief 转换成字符串
    void toString(std::string &str) override;
};