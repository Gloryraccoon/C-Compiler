/**
 * @file SymbolTable.h
 * @author liuzijie (916093580@qq.com)
 * @brief 符号表管理：变量、函数等管理的头文件
 * @version 0.1
 * @date 2024-5-11
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "Value.h"
#include "Function.h"

class SymbolTable;

extern SymbolTable* symbolTable;

class SymbolTable {
public:
    std::vector<std::string > valueName;

    // 保存函数名，以便顺序遍历
    std::vector<std::string > funcsName;

    // 用来保存所有的全局变量
    std::unordered_map<std::string, Value*> globalValueList;

    // 用来保存所有的函数信息
    std::unordered_map<std::string, Function*> funcsList;

    /// @brief 查找全局变量
    /// @param varName 变量名
    /// @return 变量
    Value *findGlobalValue(std::string varName);

    /// @brief 查找局部变量
    /// @param varName 变量名
    /// @param funcName 函数名
    /// @param tempStack 是否在临时栈里查找
    /// @return 变量
    Value *findLocalValue(std::string varName, std::string funcName, bool tempStack = false);
    
    /// @brief 查找函数
    /// @param funcName 函数名
    /// @return 函数符号
    Function *findFuncValue(std::string funcName);
    
    /// @brief 添加全局变量
    /// @param varName 变量名
    /// @param value 变量符号
    /// @return true:添加成功；false:变量已存在
    bool addValue(std::string varName, Value *value);

    /// @brief 添加局部变量
    /// @param varName 变量名
    /// @param value 变量value
    /// @param funcName 函数名
    /// @param Temp 是否添加到临时栈里
    void addValue(std::string varName, Value *value, std::string funcName, bool Temp = false);

    /// @brief 添加函数
    /// @param funcName 函数名
    /// @param symbol 函数对象指针
    void addFunction(std::string funcName, Function *function);
};
