/**
 * @file FrontEndExecutor.h
 * @author liuzijie (916093580@qq.com)
 * @brief 前端分析执行器的类原型
 * @version 0.1
 * @date 2024-05-03
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include <string>

class FrontEndExecutor {
public:
    /// @brief 构造函数
    /// @param[in] 源文件路径
    FrontEndExecutor(std::string _filename) : filename(_filename)
    {}

    /// @brief 析构函数
    virtual ~FrontEndExecutor()
    {}

    /// @brief 前端执行器的运行函数
    /// @return true: 成功 false: 失败
    virtual bool run() = 0;

protected:
    /// @brief 要解析的文件路径
    std::string filename;
};
