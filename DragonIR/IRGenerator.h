#pragma once

#include "AST.h"
#include <unordered_map>
#include <stack>

/// @brief AST遍历产生线性IR类
class IRGenerator
{
public:
    IRGenerator(ast_node* root);

    ~IRGenerator() = default;

    bool run();

protected:
    bool ir_compile_unit_top(ast_node* node);

    bool ir_function_define(ast_node* node);

    bool ir_block(ast_node* node);

    bool ir_return(ast_node* node);

    bool ir_leaf_node(ast_node* node, bool isLeft);

    bool ir_def_list(ast_node* node, bool isLocal);
    
    bool ir_assign(ast_node* node);

    bool ir_op_binary(ast_node* node, ast_operator_type type);

    bool ir_while(ast_node* node);

    bool ir_if(ast_node* node);

    bool ir_compare(ast_node* node);

    bool ir_for(ast_node* node);

    bool ir_continue(ast_node* node);

    bool ir_break(ast_node* node);

    bool ir_neg(ast_node* node);

    bool ir_not(ast_node* node, bool isleft);

    bool ir_linc(ast_node* node);

    bool ir_rinc(ast_node* node);

    bool ir_ldec(ast_node* node);

    bool ir_rdec(ast_node* node);

    bool ir_and(ast_node* node);

    bool ir_or(ast_node* node);

    bool ir_call_func(ast_node* node, bool isleft);

    bool ir_array_index(ast_node* node, bool isleft);

    bool ir_array(ast_node* node, bool isLocal);

    ast_node* ir_visit_ast_node(ast_node* node, bool isLeft = false);

    typedef bool (IRGenerator::*ast2ir_handler_t)(ast_node*);

    std::unordered_map<ast_operator_type, ast2ir_handler_t> ast2ir_handlers;    
private:
    ast_node* root;
};