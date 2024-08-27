#include "Function.h"

std::string Function::currentFunc = "";

void funcBlockStack::push(LocalBlock* newBlock)
{
    ++scope;
    Stack.push_back(newBlock);
}

void funcBlockStack::pop(int target_scope)
{
    --scope;
    Stack[target_scope]->clear();
    Stack.pop_back();
}

Value* funcBlockStack::findALLStack(std::string varID, int currentScope)
{
    for (int i = currentScope;i >= 0;i--) {
        LocalBlock* LBlock = Stack[i];
        Value *value = LBlock->find(varID);
        if (value != nullptr) {
            return value;
        }
    }
    return nullptr;
}

Value* funcBlockStack::findOneBlock(std::string varID, int currentScope)
{
    return Stack[currentScope]->find(varID);
}

void funcBlockStack::addValue(Value* val, std::string varID, int currentScope)
{
    Stack[currentScope]->ValuesInBlock[varID] = val;
}

Value *Function::findValue(std::string valID, bool Temp)
{
    if (Temp) {
        return tempstack.findALLStack(valID, currentScope);
    } else {
        return stack.findALLStack(valID, currentScope);
    }
}

void Function::addValue(Value *value, std::string valID, bool Temp)
{
    if (Temp) {
        tempstack.addValue(value, valID, currentScope);
    } else {
        localValueList[value->getName()] = value;
        localValueNames.push_back(value->getName());
        stack.addValue(value, valID, currentScope);
    }
}
