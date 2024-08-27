%{
/* 这里声明语义动作符程序所需要的函数原型或者变量原型或定义等 */
/* 主要包含头文件，extern的全局变量，定义的全局变量等 */

#include <cstdio>
#include <cstdint>
#include <cstring>

// 此文件定义了文法中终结符的类别
#include "BisonParser.h"

// 对于整数或浮点数，词法识别无符号数，对于负数，识别为求负运算符与无符号数，请注意。
%}

/* 使它不要添加默认的规则,这样输入无法被给定的规则完全匹配时，词法分析器可以报告一个错误 */
%option nodefault

/* 产生yywrap函数 */
%option noyywrap

/* flex 生成的扫描器用全局变量yylineno 维护着输入文件的当前行编号 */
%option yylineno

/* 区分大小写 */
%option case-sensitive

/* yytext的类型为指针类型，即char * */
%option pointer

/* 生成可重用的扫描器API，这些API用于多线程环境 */
/* %option reentrant */

/* 不进行命令行交互，只能分析文件 */
%option never-interactive

%x COMMENT
/* 辅助定义式或者宏，后面使用时带上大括号 */

d			[0-9]
l           [a-zA-Z]|_
white		[\t\040]
white_line  [\r\n]

/* 正规式定义 */
%%
"/*"        {
                // 进入注释子有限自动机
                BEGIN(COMMENT);
            }

<COMMENT>"*/" {
                // 结束注释子有限自动机
                BEGIN(INITIAL);
            }

<COMMENT>([^*]|\n)+|.  {
                ; // 注释子有限自动机中，忽略所有字符，含换行符等
            }

<COMMENT><<EOF>> {
                // 注释子有限自动机，没有遇到*/但是文件结束
                printf("Line %d: Unterminated comment\n", yylineno);

                // 词法识别错误
                return 257;
            }

"//".*\n    {
                ; // 忽略行注释
            }

0[0-7]+     {
	
                // 词法识别八进制整数
                yylval.integer_num.val = (uint32_t)strtol(yytext, (char **)NULL, 8);
                yylval.integer_num.lineno = yylineno;
                return T_DIGIT;
            }

{d}+		{
                // 词法识别无符号整数，注意对于负数，则需要识别为负号和无符号数两个Token
                yylval.integer_num.val = (uint32_t)strtol(yytext, (char **)NULL, 10);
                yylval.integer_num.lineno = yylineno;
                return T_DIGIT;
            }

"+=" |
"-=" |
"*=" |
"/=" |
"%="        {
                yylval.var_id.lineno = yylineno;
                strncpy(yylval.var_id.id, yytext, sizeof(yylval.var_id.id));
                return T_ASSIGN_DIGITOP;
            }

"if"        {
                yylval.var_id.lineno = yylineno;
				return T_IF;                
            }

"else"      {
                yylval.var_id.lineno = yylineno;
				return T_ELSE;                
            }

"while"     {
                yylval.var_id.lineno = yylineno;
                return T_WHILE;
            }

"for"       {
                yylval.var_id.lineno = yylineno;
                return T_FOR;
            }

"continue"       {
                yylval.var_id.lineno = yylineno;
                return T_CONTINUE;
            }

"break"       {
                yylval.var_id.lineno = yylineno;
                return T_BREAK;
            }

"int"       {
				strncpy(yylval.var_id.id, yytext, sizeof(yylval.var_id.id));
				return T_INT;
			}

"void"      {
				strncpy(yylval.var_id.id, yytext, sizeof(yylval.var_id.id));
				return T_VOID;         
            }

"return"    {
                yylval.var_id.lineno = yylineno;
                return T_RETURN;
            }

0[Xx][0-9a-fA-F]+ {
	
                // 词法识别十六进制整数
                yylval.integer_num.val = (uint32_t)strtol(yytext, (char **)NULL, 16);
                yylval.integer_num.lineno = yylineno;
                return T_DIGIT;
            }

{l}+({d}|{l})* {
                strncpy(yylval.var_id.id, yytext, sizeof(yylval.var_id.id));
                yylval.var_id.lineno = yylineno;
                return T_ID;
            }

">"         {
    strncpy(yylval.cmp_attr.id, "gt", sizeof(yylval.cmp_attr.id));
    yylval.cmp_attr.lineno = yylineno;
    yylval.cmp_attr.cmp_kind = GT;
    return T_CMP;
}
"<"         {
    strncpy(yylval.cmp_attr.id, "lt", sizeof(yylval.cmp_attr.id));
    yylval.cmp_attr.lineno = yylineno;
    yylval.cmp_attr.cmp_kind = LT;
    return T_CMP;
}
"<="        {
    strncpy(yylval.cmp_attr.id, "le", sizeof(yylval.cmp_attr.id));
    yylval.cmp_attr.lineno = yylineno;
    yylval.cmp_attr.cmp_kind = LE;
    return T_CMP;
}
">="        {
    strncpy(yylval.cmp_attr.id, "ge", sizeof(yylval.cmp_attr.id));
    yylval.cmp_attr.lineno = yylineno;
    yylval.cmp_attr.cmp_kind = GE;
    return T_CMP;
}
"=="        {
    strncpy(yylval.cmp_attr.id, "eq", sizeof(yylval.cmp_attr.id));
    yylval.cmp_attr.lineno = yylineno;
    yylval.cmp_attr.cmp_kind = EQ;
    return T_CMP;
}
"!="        {
    strncpy(yylval.cmp_attr.id, "ne", sizeof(yylval.cmp_attr.id));
    yylval.cmp_attr.lineno = yylineno;
    yylval.cmp_attr.cmp_kind = NE;
    return T_CMP;
}

"!"          {return '!';}

"&&"         {return T_AND;}
"||"         {return T_OR;}

"++"         {yylval.var_id.lineno = yylineno;return T_INC;}
"--"         {yylval.var_id.lineno = yylineno;return T_DEC;}

"("			{return '(';}
")"			{return ')';}

"="         {return '=';}
";"         {return ';';}

"{"         {return '{';}
"}"         {return '}';}

","         {return ','; }

"["         {return '[';}
"]"         {return ']';}

"+"         {return '+';}
"-"         {return '-';}

"*"         {return '*';}
"/"         {return '/';}
"%"         {return '%';}

{white}+	 ;

{white_line}+ ;

.			{
                printf("Line %d: Invalid char %s\n", yylineno, yytext);
                // 词法识别错误
                return 257;
            }
%%
