/**
 * @file ast.cpp
 * @author liuzijie (916093580@qq.com)
 * @brief 抽象语法树管理
 * @version 0.1
 * @date 2024-04-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "AST.h"

//抽象语法树的根节点
ast_node* ast_root = nullptr;

/// @brief 创建某种类型的节点并且设置子节点
ast_node * new_ast_node(ast_operator_type type, 
	ast_node* first_son,
	ast_node* second_son,
	ast_node* third_son,
	ast_node* forth_son
)
{
	ast_node * Node = new ast_node();
	Node->type = type;
	Node->parent = nullptr;
	//Node->val = nullptr;
	if(first_son != nullptr)
	{
		Node->sons.push_back(first_son);
		first_son->parent = Node;	
	}

	if(second_son != nullptr)
	{
		Node->sons.push_back(second_son);
		second_son->parent = Node;
	}

	if(third_son != nullptr)
	{
		Node->sons.push_back(third_son);
		third_son->parent = Node;
	}

	if(forth_son != nullptr)
	{
		Node->sons.push_back(forth_son);
		forth_son->parent = Node;
	}

	return Node;
}

/// @brief 创建AST的终止节点
/// @param attr
ast_node * new_ast_leaf_node(struct ast_node_attr &attr)
{
	ast_node * Node = new ast_node();
	Node->type = ast_operator_type::AST_OP_END;
	Node->parent = nullptr;
	//Node->val = nullptr;
	Node->attr = attr;

	return Node;
}

/// @brief 递归清理抽象语法树
/// @param node 清理的当前节点
void free_ast_node(ast_node * node)
{
	if(!node)
	{
		return;
	}

	for(auto child : node->sons){
		free_ast_node(child);
	}

	//清理孩子元素
	node->sons.clear();
	
	//清理node资源
	delete node;
}

/// @brief 资源清理程序的最外层
void free_ast()
{
	free_ast_node(ast_root);
	ast_root = nullptr;
}
