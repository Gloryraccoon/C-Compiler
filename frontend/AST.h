#pragma once

#include <cstdio>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include "Function.h"
#include "IRCode.h"

/// @brief 节点类型
typedef enum ast_operator_type
{	
	AST_OP_END,  //终结符节点
	AST_OP_TOP,  //全局定义与声明
	AST_DEF_LIST,     //定义变量
	AST_VAR_DECL,     // 变量声明
	AST_OP_EMPTY,  //空语句
	AST_OP_FARGS,   //参数列表
	AST_FUNC_DEF,  //函数定义
	AST_VAR_INIT,  //变量初始化
	AST_OP_ASSIGN, //赋值
	AST_ARRAY,        // 数组定义
    AST_ARRAY_LIST,   //数组定义的中间节点
	AST_DIMENSION,     //数组维度
	AST_SENTENCE_BLOCK,   //一条语句
	AST_OP_RETURN,      //return语句
	AST_ARRAY_INDEX,    //数组下标
	AST_FUNC_CALL,      //函数调用	
	AST_REAL_ARGS,       //函数实参列表

	/*单目运算*/
	AST_OP_NEG,         //取负运算
	AST_OP_NOT,         //非运算
	AST_OP_RDEC,     //右自减
	AST_OP_LDEC,     //左自减
	AST_OP_RINC,     //右自增
	AST_OP_LINC,     //左自增

	/*二元运算*/
	AST_OP_ADD,       //加法
	AST_OP_SUB,		  //减法
	AST_OP_MUL,		  //乘法
	AST_OP_DIV,       //除法
	AST_OP_MOD,       //取余
	AST_OP_AND,       //与
	AST_OP_OR,        //或

	/*关系运算*/
	AST_OP_CMP,

	AST_OP_IF,        //IF语句
	AST_OP_WHILE,		//while语句
	AST_OP_FOR,         //for语句
	AST_OP_CONTINUE,     //continue语句
	AST_OP_BREAK,         //break语句

}ast_operator_type;

typedef enum CMP_KIND {
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,
}_CMP_KIND;


/// @brief 字面量种类
typedef enum numdigit_variety
{
	KEYWORD,
	DIGIT_INT,
	DIGIT_REAL,
	DIGIT_ID,
	DEFAULT,
	CMP_KIND,
}numdigit_variety;

/// @brief 叶子节点的属性值
typedef struct ast_node_attr {
	numdigit_variety kind;
	_CMP_KIND cmp_kind;
	int lineno;   //行号信息

	union {
		int integer_val;
		double real_val;
		char id[256];
	};
}ast_node_attr;

/// @brief 节点类定义
class ast_node {
public:
	ast_node* parent;    //父节点
	std::vector<ast_node*> sons;  //孩子节点

	enum ast_operator_type type;  //节点类型
	ast_node *next;       // 同层的下一个节点
	ast_node *false_next; // 表达式为真跳转的节点
	ast_node *true_next;  // 表达式为假跳转的节点
	bool jump = true;            // 是否进行跳转
	bool newLabel = true;        // 是否新建一个新的label
	std::string label;           // 当前节点的label
	std::vector<std::string > labels; // 跳转的三个Label
    bool visited = false;        // 访问标记
    struct ast_node_attr attr;   // node的其它属性
	
    InterCode blockInsts;   //包含多条指令的线性IR指令块

	Function* func;     //如果节点是函数定义，则该项有实际意义
	Value* val;         //如果节点指代一个变量，则该项有实际意义
};

/// @brief 抽象语法树的根节点指针
extern ast_node* ast_root;

/// @brief 创建某种类型节点并且设置子结点
ast_node* new_ast_node(
	ast_operator_type type,
	ast_node* first_son = nullptr,
	ast_node* second_son = nullptr,
	ast_node* third_son = nullptr,
	ast_node* forth_son = nullptr
);

/// @brief 创建抽象语法树的叶子结点
ast_node* new_ast_leaf_node(ast_node_attr& val);

/// @brief 清理抽象语法树资源
void free_ast();
void free_ast_node(ast_node * node);
