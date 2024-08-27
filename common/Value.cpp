#include "SymbolTable.h"

extern SymbolTable *symbolTable;

void newCustomValue(std::string name, ValueType type)
{
    auto pIter = symbolTable->globalValueList.find(name);
    if (pIter != symbolTable->globalValueList.end()) {
        // 符号表中存在，则只是更新值
        pIter->second->type = type;

        if (type == ValueType::TYPE_INT) {
            pIter->second->intVal = 0;
        } else {
            pIter->second->realVal = 0;
        }
    } else {
        Value *temp = new CustomValue(name, type);
        symbolTable->globalValueList.emplace(name, temp);
    }
}

Value* newCustomValue(std::string name)
{
    // 类型待定
    Value *temp = new CustomValue(name, ValueType::TYPE_INT);
    symbolTable->addValue(name, temp);
    return temp;
}

Function* newFunction(std::string name, ValueType type)
{
    // 类型待定
    Function *tempFunction = new Function(name, type);
    symbolTable->addFunction(name, tempFunction);
    return tempFunction;
}

Value *newLocalCustomValue(std::string name, ValueType type, std::string funcName)
{
    // 类型待定
    Value *tempValue = new LocalCustomValue(name, type, funcName);
    symbolTable->addValue(name, tempValue, funcName);
    return tempValue;
}

Value *newConstValue(int intVal)
{
    Value *tempValue = new ConstValue(intVal);
    symbolTable->globalValueList.emplace(tempValue->name, tempValue);

    return tempValue;
}

Value *newConstValue(double realVal)
{
    Value *tempValue = new ConstValue(realVal);
    symbolTable->globalValueList.emplace(tempValue->name, tempValue);

    return tempValue;
}

Value *newTempValue(ValueType type, std::string funcName, bool isFfargs)
{
    auto it = symbolTable->funcsList.find(funcName);
    Value *temp = new TempValue(type, funcName);
    it->second->tempValueNames.push_back(temp->name);
    it->second->tempValueList.emplace(temp->name, temp);
    if (isFfargs) {
        it->second->argsList.push_back(temp);
    }
    return temp;
}

Value *findValue(std::string name, std::string funcName, bool Temp)
{
    Function *temp = nullptr;
    Value *result = nullptr;
    // temp = findFuncValue(func_name);
    temp = symbolTable->findFuncValue(funcName);
    // 先在函数表的局部变量里找
    if (temp) {
        if (Temp) {
            result = temp->tempstack.findALLStack(name, temp->currentScope);
        } else {
            result = temp->stack.findALLStack(name, temp->currentScope);
        }
        if (result) { return result; }
    }
    // 局部变量找不到在全局变量里找
    auto pIter = symbolTable->globalValueList.find(name);
    if (pIter == symbolTable->globalValueList.end()) {
        // 变量名没有找到
        return nullptr;
    } else {
        result = pIter->second;
    }

    return result;
}

Function* findFuncValue(std::string name)
{
    Function *temp = nullptr;

    auto it = symbolTable->funcsList.find(name);
    if (it == symbolTable->funcsList.end()) {
        return nullptr;
    } else {
        temp = it->second;
    }
    return temp;
}

// 查看变量名是否存在
bool IsExist(std::string name, std::string funcName)
{
    auto it = symbolTable->funcsList.find(funcName);
    if (it == symbolTable->funcsList.end()) {
        auto it = symbolTable->globalValueList.find(name);
        if (it == symbolTable->globalValueList.end()) {
            return false;
        }
    } else {
        Function *sym = it->second;
        auto it = sym->stack.findALLStack(name, sym->currentScope);
        if (it) {
            return true;
        } else {
            return false;
        }
    }
    return true;
}

/// @brief 判断全局变量或者函数是否存在
bool GlobalIsExist(std::string name)
{
    auto it = symbolTable->globalValueList.find(name);
    if (it == symbolTable->globalValueList.end()) {
        auto it1 = symbolTable->funcsList.find(name);
        if (it1 == symbolTable->funcsList.end()) {
            return false;
        } else {
            return true;
        }
    } else {
        return true;
    }
    return true;
}

bool LocalIsExist(std::string funcName, std::string varName)
{
    auto it = symbolTable->funcsList.find(funcName);
    if (it == symbolTable->funcsList.end()) {
        printf("function %s doesn't exist\n", funcName.c_str());
        return false;
    }
    if (it->second->stack.findOneBlock(varName, it->second->currentScope) != nullptr) {
        return true;
    } else {
        return false;
    }
}

/// @brief 清理符号表
void freeValues()
{
    for (auto e : symbolTable->globalValueList) {
        delete e.second;
    }
    // Hash表清空
    symbolTable->globalValueList.clear();
}
