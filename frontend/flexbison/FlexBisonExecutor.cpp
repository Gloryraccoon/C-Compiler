#include "FlexBisonExecutor.h"
#include "BisonParser.h"
#include "FlexLexer.h"

/// @brief 前端词法与语法解析生成AST
/// @return true: 成功 false：错误
bool FlexBisonExecutor::run()
{
    // 若指定有参数，则作为词法分析的输入文件
    yyin = fopen(filename.c_str(), "r");
    if (yyin == nullptr) {
        printf("Can't open file %s\n", filename.c_str());
        return false;
    }

    // 如果要查看LALR的移进与归约过程，请设置yydebug为1
    // yydebug = 1;

    // 词法、语法分析生成抽象语法树AST
    bool result = yyparse();
    if (0 != result) {
        printf("yyparse failed\n");

        // 关闭文件
        fclose(yyin);

        return false;
    }

    // 关闭文件
    fclose(yyin);

    return true;
}