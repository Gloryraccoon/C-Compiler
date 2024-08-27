#pragma once

#include "ValueType.h"
#include "Common.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

static std::unordered_map<std::string, int> funcTempVarCount = {{"main", 0}};  //函数内部的临时变量数量（函数名匹配对应数量）

static uint64_t tempConstCount = 0; // 临时变量计数，默认从0开始

class Value
{
public:
    /// @brief 是否是常量
    bool is_const = false;

    /// @brief 是否编译器内部产生的临时变量
    bool is_temp = false;

    /// @brief 是否是用户定义的变量或标识符
    bool is_var = false;

    /// @brief 创建一个临时变量
    static std::string createTempVarName(std::string funcName, bool isLocal = false)
    {

        if (isLocal)
        {
            return "%l" + int2str(funcTempVarCount[funcName]++);
        }
        else
        {
            return "%t" + int2str(funcTempVarCount[funcName]++);
        }
    }

    /// @brief 创建一个临时常量
    static std::string createConstVarName()
    {


        return "%c" + int2str(tempConstCount++);
    }

public:
    /// @brief 变量名或内部标识的名字
    std::string name;
    /// @brief 变量名
    std::string id_name;

    /// @brief 类型
    ValueType type;

    /// @brief 整型常量的值
    int intVal = 0;

    /// @brief 实数常量的值
    double realVal = 0;
    int scope;

    /// @brief 数组维度级
    int dimIndex = 0;   

    std::vector<int> dimensions;

    /// @brief 用于区分是否定义即初始化的全局变量
    bool IsGlobalAssigned = false;

    /// @brief 全局变量初始化的值(如果定义即初始化)
    int GlobalAssignVal;
    

protected:
    /// @brief 默认实数类型的构造函数
    Value() : type(ValueType::TYPE_REAL) {}
    
    /// @brief 构造函数
    /// @param _name 
    /// @param _type 
    Value(std::string _name, ValueType _type) : name(_name), type(_type)
    {}

    /// @brief 构造函数
    /// @param _type 
    Value(ValueType _type) : type(_type) {}

public:
    /// @brief 析构函数
    virtual ~Value(){}

    /// @brief 获取名字
    /// @return 
    virtual std::string getName() const
    {
        return name;
    }
    /// @brief 获取类型
    virtual std::string getType() const
    {
        std::string typeID;
        switch (type) {
        case ValueType::TYPE_INT:
            typeID = "i32";
            break;
        case ValueType::TYPE_BOOLEAN:
            typeID = "i1";
            break;
        case ValueType::TYPE_VOID:
            typeID = "void";
            break;
        case ValueType::TYPE_INT_PTR:
            typeID = "i32*";
            break;
        default:
            typeID = "i32";
            break;
        }
        return typeID;
    }
};

/// @brief 临时变量
class TempValue : public Value {
public:
    /// @brief 创建临时Value，用于保存中间IR指令的值
    TempValue(ValueType type, std::string funcName) : Value(type)
    {
        is_temp = true;
        name = createTempVarName(funcName);

    }

    /// @brief 创建临时Value，用于保存中间IR指令的值
    TempValue(std::string funcName) : Value(ValueType::TYPE_REAL)
    {
        is_temp = true;
        name = createTempVarName(funcName);
    }

    /// @brief 析构函数
    virtual ~TempValue() override{}
};

/// @brief 常量
class ConstValue : public Value {
public:
    /// @brief 整数的临时变量值
    ConstValue(int val) : Value(ValueType::TYPE_INT)
    {
        is_const = true;
        name = createConstVarName();
        intVal = val;
    }

    /// @brief 实数的临时变量值
    ConstValue(double val) : Value(ValueType::TYPE_REAL)
    {
        is_const = true;
        name = createConstVarName();
        realVal = val;
    }

    /// @brief 析构函数
    virtual ~ConstValue() override {}

    /// @brief 获取名字
    /// @return 
    virtual std::string getName() const override
    {
        if (type == ValueType::TYPE_INT) {
            return int2str(this->intVal);
        } else {
            return double2str(this->realVal);
        }
    }
};

/// @brief 用户自定义的变量和标识符
class CustomValue : public Value {
public:
    /// @brief 创建变量Value，用于保存中间IR指令的值
    CustomValue(std::string _name, ValueType _type) : Value("@" + _name, _type)
    {
        is_var = true;
    }

    /// @brief 创建变量Value，用于保存中间IR指令的值
    CustomValue(std::string _name) : Value("@" + _name, ValueType::TYPE_REAL)
    {
        is_var = true;
    }

    /// @brief 析构函数
    virtual ~CustomValue() override {}
};

class LocalCustomValue : public Value
{
public:
    /// @brief 创建变量Value，用于保存中间IR指令的值
    LocalCustomValue(std::string _name, ValueType _type, std::string funcName) : Value(_type)
    {
        id_name = _name;
        is_var = true;
        name = createTempVarName(funcName, true);
    }

    /// @brief 创建变量Value，用于保存中间IR指令的值
    LocalCustomValue(std::string _name, std::string funcName) : Value(ValueType::TYPE_REAL)
    {
        id_name = _name;
        is_var = true;
        name = createTempVarName(funcName, true);
    }

    /// @brief 析构函数
    virtual ~LocalCustomValue() override{}
};

/// @brief 一个作用域
class LocalBlock
{
public:
    std::unordered_map<std::string, Value*> ValuesInBlock;
    int scope;

    void clear()
    {
        ValuesInBlock.clear();
    }
     
    Value* find(std::string varID)
    {
        auto it = ValuesInBlock.find(varID);
        if(it != ValuesInBlock.end())
        {
            return it->second;
        }
        return nullptr;
    }
};

bool IsExist(std::string name, std::string funcName);

bool LocalIsExist(std::string funcName, std::string varName);

bool GlobalIsExist(std::string name);

/// @brief 新建变量型Value
/// @param name 变量ID
/// @param type 变量类型
void newCustomValue(std::string name, ValueType type);

/// @brief 新建一个变量型的Value，并加入到符号表，用于后续释放空间
/// @param intVal 整数值
/// @return 常量Value
Value *newCustomValue(std::string name);

/// @brief 新建一个局部变量型的Value，并加入到符号表，用于后续释放空间
/// \param intVal 整数值
/// \return 常量Value
Value *newLocalCustomValue(std::string name, ValueType type, std::string funcName);

/// @brief 新建一个整型数值的Value，并加入到符号表，用于后续释放空间
/// @param intVal 整数值
/// @return 临时Value
Value *newConstValue(int intVal);

/// @brief 新建一个实数数值的Value，并加入到符号表，用于后续释放空间
/// @param intVal 整数值
/// @return 临时Value
Value *newConstValue(double realVal);

/// @brief 新建一个临时型的Value，并加入到符号表，用于后续释放空间
/// @param intVal 整数值
/// @return 常量Value
Value *newTempValue(ValueType type, std::string funcName, bool isFfargs = false);