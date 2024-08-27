#pragma once

#include "IRInst.h"
#include <vector>
#include <string>

/// @brief 中间代码管理类
class InterCode
{
public:
    InterCode();

    ~InterCode();

    /// @brief 添加一个指令块
    /// @param block 
    void addInst(InterCode& block);

    /// @brief 添加一条中间指令
    /// @param inst 
    void addInst(IRInst* inst);

    int getCodeSize();

    //获取指令序列
    std::vector<IRInst*>& getInsts();

    /// @brief 文本输出线性IR指令
    void outputIR(const std::string &filePath);

protected:
    std::vector<IRInst*> code;
};