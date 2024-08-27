#pragma once
#include "FrontEndExecutor.h"

class FlexBisonExecutor : public FrontEndExecutor {
public:
    FlexBisonExecutor(std::string filename) : FrontEndExecutor(filename) {}
    virtual ~FlexBisonExecutor() {}

    /// @brief 前端词法与语法解析生成AST
    /// @return true: 成功 false：错误
    bool run() override;
};
