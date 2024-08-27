#include "SymbolTable.h"

SymbolTable *symbolTable = new SymbolTable();

Function* SymbolTable::findFuncValue(std::string funcName)
{
    Function *val = nullptr;
    auto it = funcsList.find(funcName);
    if (it != funcsList.end()) {
        val = it->second;
        return val;
    }
    return val;
}

bool SymbolTable::addValue(std::string varName, Value* value)
{
    // 如果变量已经存在，则返回false
    if (findGlobalValue(varName)) {
        return false;
    }
    globalValueList[varName] = value;
    valueName.push_back(varName);
    return true;
}

void SymbolTable::addValue(std::string varName, Value *value, std::string funcName, bool Temp)
{
    auto it = funcsList.find(funcName);
    if (it != funcsList.end()) {
        Function * function = it->second;
        function->addValue(value, varName, Temp);
    } else {
        printf("error: Function:%s can not found\n", funcName.c_str());
    }
}

void SymbolTable::addFunction(std::string funcName, Function* function)
{
    funcsList[funcName] = function;
    funcsName.push_back(funcName);
}

Value* SymbolTable::findGlobalValue(std::string varName)
{
    auto it = globalValueList.find(varName);
    if(it != globalValueList.end())
    {
        return it->second;
    }
    return nullptr;
}

Value* SymbolTable::findLocalValue(std::string varName, std::string funcName, bool tempStack)
{
    auto it = funcsList.find(funcName);
    Value* result = nullptr;
    if(it != funcsList.end())
    {
        Function* function = it->second;
        result = function->findValue(varName, tempStack);
        if(result == nullptr)
        {
            auto it0 = globalValueList.find(varName);
            if(it0 != globalValueList.end())
            {
                result = it0->second;
            }
            /*
            else
            {
                std::cout << "error:none symbol" << std::endl;
            }*/
        }
    }
    return result;
}
