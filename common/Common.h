﻿/**
 * @file common.h
 * @author zenglj (zenglj@nwpu.edu.cn)
 * @brief 共同函数的头文件
 * @version 0.1
 * @date 2023-09-24
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <string>

/// @brief 整数变字符串
/// @param num 无符号数
/// @return 字符串
std::string int2str(int64_t num);

/// @brief 浮点数变字符串
/// @param num 浮点数
/// @return 字符串
std::string double2str(double num);

/// @brief 检查字符是否是字母（大小写字母）
/// @param ch 字符
/// @return true：是，false：不是
bool isLetter(char ch);

/// @brief 检查字符是否为数字(0-9)
/// @param ch 字符
/// @return true：字符是数字字符(0-9) false:不是
bool isDigital(char ch);

/// @brief 检查字符是否为大小写字符或数字(0-9)
/// @param ch 字符
/// @return true：是，false：不是
bool isLetterDigital(char ch);

/// @brief 检查字符是否为大小写字符或数字(0-9)或下划线
/// @param ch 字符
/// @return true：是，false: 不是
bool isLetterDigitalUnderLine(char ch);

/// @brief 检查字符是否为大小写字符或下划线
/// @param ch 字符
/// @return true：是，false：不是
bool isLetterUnderLine(char ch);

/// @brief 删除前后空格
/// @param str 要处理的字符串
/// @return 处理后的字符串
std::string trim(const std::string & str);

void printError(int lineno, std::string msg);
