#pragma once

#include "SymbolTable.h"
#include "AST.h"
#include <unordered_map>

/// @brief AST遍历产生符号表并且进行语义检查
class SymbolGenerator
{
public:
    /// @brief 构造函数
    /// @param root 
    /// @param symbolTable
    SymbolGenerator(ast_node* root);
    
    /// @brief 析构函数
    ~SymbolGenerator() = default;

    /// @brief 运行产生符号表
    /// @return 
    bool run();

protected:
    /// @brief 分析top节点
    /// @param node 
    /// @return 
    bool sym_top_compile_unit(ast_node* node);

    /// @brief 分析函数定义节点
    /// @param node 
    /// @return 
    bool sym_func_def(ast_node* node);

    /// @brief 分析语句块定义
    /// @param node 
    /// @return 
    bool sym_sentence_block(ast_node* node);

    /// @brief 分析return节点
    /// @param node 
    /// @return 
    bool sym_return(ast_node* node);

    /// @brief 分析叶子节点
    /// @param node 
    /// @return 
    bool sym_leaf(ast_node* node);

    /// @brief 变量定义
    /// @param node 
    /// @param isLocal 
    /// @return 
    bool sym_def_list(ast_node* node, bool isLocal);

    /// @brief 变量赋值
    /// @param node 
    /// @return 
    bool sym_assign(ast_node* node);

    /// @brief 二元运算
    /// @param node 
    /// @return 
    bool sym_op_binary(ast_node* node);

    /// @brief while语句块
    /// @param node 
    /// @return 
    bool sym_while(ast_node* node);

    /// @brief if语句块
    /// @param node 
    /// @return 
    bool sym_if(ast_node* node);

    /// @brief 比较功能
    /// @param node 
    /// @return 
    bool sym_compare(ast_node* node);

    bool sym_for(ast_node* node);

    bool sym_continue(ast_node* node);

    bool sym_break(ast_node* node);

    bool sym_neg(ast_node* node);

    bool sym_not(ast_node* node);

    bool sym_inc_dec(ast_node* node);

    bool sym_logic(ast_node* node);

    bool sym_call_func(ast_node* node);

    bool sym_array(ast_node* node, ValueType type, bool isLocal);

    bool sym_array_index(ast_node* node);

    bool sym_array_arglist(ast_node* node, bool istemp = false);

    ast_node* sym_visit_ast_node(ast_node* node, bool isLeft = false, bool isLast = false);

private:
    /// @brief 抽象语法树的根
    ast_node* root;

    typedef bool (SymbolGenerator::*sym_handler_t)(ast_node*);

    std::unordered_map<ast_operator_type, sym_handler_t> sym_handlers;
};
