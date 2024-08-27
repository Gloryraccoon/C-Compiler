/**
* @file main.cpp
* @author liuzijie (916093580@qq.com)
* @brief 主程序文件
* @version 0.1
* @date 2024-05-03
*
* @copyright Copyright (c) 2024
*
*/

#include <unistd.h>
#include <iostream>
#include <string>
#include "AST.h"
#include "FlexBisonExecutor.h"
#include "Graph.h"
#include "SymbolTable.h"
#include "SymbolGenerator.h"
#include "IRGenerator.h"
#include "IRCode.h"

//终端显示帮助
bool gShowHelp = false;

/// @brief 显示抽象语法树
int gShowAST = 0;

/// @brief 产生线性IR,默认输出
int gShowLineIR = 0;

/// @brief 输出中间IR，含汇编或者自定义IR等，默认输出线性IR
int gShowSymbol = 0;

std::string gInputFile;
std::string gOutputFile;

void showHelp(const std::string & exeName)
{
    std::cout << exeName + " -S [-a | -I/i] [-o output] source" << std::endl;
}

int ArgsAnalysis(int argc, char* argv[])
{
    int ch;
    const char options[] = "ho:SaIi";

    opterr = 1;

lb_check:
    while(((ch = getopt(argc, argv, options)) != -1)){
        switch (ch) {
            case 'S':
                gShowSymbol = 1;
                break;
            case 'o':
                gOutputFile = optarg;
                break;
            case 'h':
                gShowHelp = true;
                break;
            case 'a':
                gShowAST = 1;
                break;
            case 'i':
            case 'I':
                gShowLineIR = 1;
                break;
            default :
                return -1;
                break;
        }
    }

    argc -= optind;
    argv += optind;

    if ((argc >= 1)) {
    gInputFile = argv[0];

    }

    if (argc != 0) {
        optind = 0;
        goto lb_check;
    }

    // 必须指定输入文件和输出文件
    if (gInputFile.length() == 0) {
        return -1;
    }

    // 这几个只能指定一个
    int flag = gShowAST + gShowLineIR;
    if (flag != 1) {
        return -1;
    }

    if (gShowSymbol) 
    {
        if (flag != 1) 
        {
            // 线性中间IR、抽象语法树只能同时选择一个
            return -1;
        }
    } 
    else 
    {
        // 如果-S没有指定，但指定了-a等选项时，则失败
        if (flag != 0) {
            return -1;
        }
    }

    if (gOutputFile.empty()) {

        // 默认文件名
        if (gShowAST) 
        {
            gOutputFile = "ast.png";
        } 
        else if(gShowLineIR)
        {
            gOutputFile = "ir.txt";
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    //函数返回值，默认为-1
    int result = -1;

    //内部函数调用返回值保存变量
    int subResult;
    
    do{
        subResult = ArgsAnalysis(argc, argv);
        if(subResult < 0)
        {
            //在终端显示程序帮助信息
            showHelp(argv[0]);

            //错误不用设置，因此result默认值为-1
            break;
        }

        if(gShowHelp)
        {
            //在终端显示程序帮助信息
            showHelp(argv[0]);

            //这里必须设置返回值，默认值为-1
            result = 0;
            break;
        }

        FrontEndExecutor * frontEndExecutor;
        frontEndExecutor = new FlexBisonExecutor(gInputFile);

        // 前端执行：词法分析、语法分析后产生抽象语法树，其root为全局变量ast_root
        subResult = frontEndExecutor->run();
        if (!subResult) {

            std::cout << "FrontEnd's analysis failed\n";

            // 退出循环
            break;
        }

        // 清理前端资源
        delete frontEndExecutor;

        if (gShowAST) {

            // 遍历抽象语法树，生成抽象语法树图片
            OutputAST(ast_root, gOutputFile);

            // 清理抽象语法树
            free_ast();

            // 设置返回结果：正常
            result = 0;

            break;
        }

        if(gShowLineIR)
        {
            /*构建符号表并进行语义检查，后续就不需要语义检查了*/
            SymbolGenerator SymGenerator(ast_root);
            result = SymGenerator.run();
            if(!result)
            {
                std::cout << "symbol generated fail!!!" << std::endl;
                break;
            }
            
            InterCode IRCode;
            IRGenerator ast2ir(ast_root);
            result = ast2ir.run();
            if(!result)
            {
                std::cout << "IR generated fail!!!" << std::endl;
            }
            IRCode.addInst(ast_root->blockInsts);
            IRCode.outputIR(gOutputFile);
        }
        
    }while(false);

    return result;
}
