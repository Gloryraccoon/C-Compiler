#pragma once

#include "Value.h"

/// @brief 函数作用域栈
class funcBlockStack
{
public:
    int scope = -1;
    std::vector<LocalBlock*> Stack;
    
    ///@brief 新增一个作用域
    void push(LocalBlock* newBlock);

    /// @brief 离开一个作用域后进行删除
    /// @param scope 作用域index
    void pop(int scope);

    /// @brief 在整个栈里查找某个变量
    /// @param varID 变量名
    /// @return 变量Value
    Value* findALLStack(std::string varID, int currentScope);

    /// @brief 在当前作用域查找变量
    /// @param varID 变量名
    /// @return 变量Value
    Value* findOneBlock(std::string varID, int currentScope);

    /// @brief 在当前作用域增加变量
    /// @param val 变量Value
    /// @param varID 变量名
    void addValue(Value *val, std::string varID, int currentScope);
};

/// @brief 函数管理类
class Function
{
public:
    std::string funcName;

    static std::string currentFunc;

    std::string getName() const
    {
        return funcName;
    }

    /// @brief 返回值类型
    ValueType return_type;

    /// @brief 参数列表
    std::vector<Value*> argsList;
    
    /// @brief 局部变量表
    std::unordered_map<std::string, Value *> localValueList;
    std::vector<std::string > localValueNames;

    /// @brief 临时变量表
    std::unordered_map<std::string, Value *> tempValueList;
    std::vector<std::string > tempValueNames;

    /// @brief 函数作用域符号栈
    funcBlockStack stack;  

    /// @brief 临时符号栈(生成中间IR)
    funcBlockStack tempstack;

    int currentScope = 0;
    int currentLocal = 0;

    /// @brief 从符号栈中查找某个变量
    /// @param valName 变量名
    /// @param Temp 临时栈还是符号栈
    /// @return 变量
    Value* findValue(std::string valName, bool Temp);

    /// @brief 添加符号到符号栈里
    /// @param value 变量
    /// @param valName 变量名
    /// @param Temp 添加到临时栈还是符号栈
    void addValue(Value *value, std::string valName, bool Temp = false);

    /// @brief 构造函数
    /// @param _name 函数名字
    Function(std::string _name, ValueType _return_type):funcName(_name), return_type(_return_type){};

    /// @brief 析构函数
    virtual ~Function()
    {}

    static bool is_build_func(std::string funcname)
    {
        if(funcname != "putint" 
            && funcname != "getint" 
            && funcname != "putch" 
            && funcname != "getch" 
            && funcname != "getarray" 
            && funcname != "putarray")
        {
            return false;
        }
        return true;
    }
};

/// @brief 新建一个函数Value，并加入到函数表，用于后续释放空间
/// @param name 函数名
/// @return 函数对象
Function* newFunction(std::string name, ValueType type = ValueType::TYPE_INT);

/// @brief 清理注册的所有Value资源
void freeValues();

// 用来保存所有的变量信息
// 存储函数里的label编号
static std::unordered_map<std::string, int> funcLabelCount;

/// @brief 新建一个label
/// @param func_name 函数名
/// @return label
static std::string newLabel(std::string funcName)
{
    std::string name;
    auto pIter = funcLabelCount.find(funcName);
    if (pIter == funcLabelCount.end()) {
        funcLabelCount[funcName] = 3;
        name = ".L" + int2str(funcLabelCount[funcName]);
    } else {
        name = ".L" + int2str(++funcLabelCount[funcName]);
    }
    return name;
}