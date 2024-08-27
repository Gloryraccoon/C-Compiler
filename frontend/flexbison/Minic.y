%{
#include <cstdio>
#include <cstring>

// 词法分析头文件
#include "FlexLexer.h"

// bison生成的头文件
#include "BisonParser.h"

// 抽象语法树函数定义原型头文件
#include "AST.h"

// LR分析失败时所调用函数的原型声明
void yyerror(char * msg);

%}

// 联合体声明，用于后续终结符和非终结符号属性指定使用
%union {
    class ast_node * node;

    struct digit_int_attr integer_num;
    struct digit_real_attr float_num;
    struct var_id_attr var_id;
	struct cmp_attr cmp_attr;
};

// 文法的开始符号
%start  CompileUnit

// 指定文法的终结符号，<>可指定文法属性
// 对于单个字符的算符或者分隔符，在词法分析时可直返返回对应的字符即可
%token <integer_num> T_DIGIT
%token <var_id> T_ID
%token <var_id> T_INT  //int类型关键字
%token <var_id> T_VOID   //void类型关键字
%token <var_id> T_RETURN   //return关键字
%token <var_id> T_CMP    //比较符号
%token <var_id> T_IF      //IF关键字
%token <var_id> T_ELSE    //ELSE关键字
%token <var_id> T_WHILE    //WHILE关键字
%token <var_id> T_FOR      //for关键字
%token <var_id> T_CONTINUE      //continue关键字
%token <var_id> T_BREAK      //break关键字

%token <var_id> T_ASSIGN_DIGITOP   //+= -=这些操作

%token T_AND T_OR //与  或
%token T_INC T_DEC  


%type <node> CompileUnit

// 指定文法的非终结符号，<>可指定文法属性
%type <node> program segment type def idtail deflist paras functail paradata onepara defdata arraydef 
%type <node> statblock subprogram sentenceblock sentence localvardef expression paradatatail leftValtail
%type <node> realargs
%type <node> unit
%type <node> ident num
%type <node> leftVal rightVal
%type <node> cmp

%right '='
%right T_ASSIGN_DIGITOP

%left T_OR
%left T_AND
%left PREC
%left T_CMP
%left '+' '-'
%left '*' '/' '%'

%right '!' "++" "--"

%nonassoc '(' ')'

%%
CompileUnit : program{
// Statements归约到CompileUnit时要执行的语义动作程序，C语言编写
    ast_root = $1;
};

program : segment{
	// 创建一个AST_OP_TOP类型的中间节点，孩子为Statement($1)
    $$ = new_ast_node(AST_OP_TOP, $1);
}
|  program segment
{
    // Statement($2)作为Block($1)的孩子 
    $2->parent = $1;
    $1->sons.push_back($2);
    $$ = $1;
}

/*全局变量，函数声明，函数定义*/
segment : type def
{
	if($2->type == AST_DEF_LIST)
	{
		ast_node* tempNode;
		tempNode = new_ast_node(AST_DEF_LIST, $1, $2->sons[0]);
		std::vector<ast_node*>::iterator it = $2->sons[1]->sons.end() - 1;
		for(;it != $2->sons[1]->sons.begin() - 1;--it)
		{
			tempNode->sons.push_back(*it);
			(*it)->parent = tempNode;
		}
		$2->sons[1]->sons.clear();
		free_ast_node($2->sons[1]);
		$2->sons.clear();
		free_ast_node($2);

		$$ = tempNode;
	}
	else if ($2->type == AST_OP_END){
    	// 单变量定义，$2为一个叶子节点
    	// int a;
    	$$ = new_ast_node(AST_DEF_LIST, $1, $2);
	}
	else
	{
		$$ = new_ast_node($2->type, $1, $2->sons[0], $2->sons[1], $2->sons[2]);
	}
};

/*变量类型*/
type : T_INT
{
	struct ast_node_attr tempVal;
	tempVal.kind = KEYWORD;
	tempVal.lineno = $1.lineno;
	strncpy(tempVal.id, $1.id, sizeof(tempVal.id));

    $$ = new_ast_leaf_node(tempVal);
}
| T_VOID
{
	struct ast_node_attr tempVal;
	tempVal.kind = KEYWORD;
	tempVal.lineno = $1.lineno;
	strncpy(tempVal.id, $1.id, sizeof(tempVal.id));

    $$ = new_ast_leaf_node(tempVal);
}
;
ident : T_ID
{
    struct ast_node_attr tempVal;
    tempVal.kind = DIGIT_ID;
    tempVal.lineno = $1.lineno;
    strncpy(tempVal.id, $1.id, sizeof(tempVal.id));
    $$ = new_ast_leaf_node(tempVal);
};

/* 变量定义或函数定义 */
def : ident idtail
{
	if ($2==NULL)$$ = $1;
	else if ($2->type == AST_DEF_LIST )
	{
		if($2->sons.size()==0){
    	// 单个变量定义 int a;
        	$$ = $1;  
        }
		else
		{
			// 多个变量
            $$ = new_ast_node(AST_DEF_LIST, $1,$2);
		}
	}
	else if($2->type == AST_VAR_INIT)
	{
		ast_node* tempNode;
		tempNode = new_ast_node(AST_OP_ASSIGN, $1, $2->sons[0]);
		
		$$ = new_ast_node(AST_DEF_LIST, tempNode, $2->sons[1]);
		$2->sons.clear();
		free_ast_node($2);
	}
	else if($2->type == AST_ARRAY_LIST)
	{
		ast_node* tempNode = new_ast_node(AST_ARRAY, $1, $2->sons[0]);
		$$ = new_ast_node(AST_DEF_LIST, tempNode, $2->sons[1]);
		$2->sons.clear();
		free_ast_node($2);
	}
	else if($2->type == AST_FUNC_DEF)
	{
		ast_node* tempNode;
		tempNode = new_ast_node(AST_FUNC_DEF, $1, $2->sons[0], $2->sons[1]);
		$$ = tempNode;
	}
};

idtail : deflist            
{
	$$ = $1;
}
| '(' paras ')' functail
{
	$$ = new_ast_node(AST_FUNC_DEF, $2, $4);
}
| '(' ')' functail
{
	ast_node* Node = new ast_node();
	Node->type = AST_OP_FARGS;
	$$ = new_ast_node(AST_FUNC_DEF, Node, $3);
}
| '=' expression deflist
{
	$$ = new_ast_node(AST_VAR_INIT, $2, $3);
}
| arraydef deflist
{
	$$ = new_ast_node(AST_ARRAY_LIST, $1, $2);
}
;

arraydef : '[' num ']'
{
	$$ = new_ast_node(AST_DIMENSION, $2);
}
| '[' num ']' arraydef
{
	$$ = new_ast_node(AST_DIMENSION, $2, $4);
}

paras : onepara
{
    $$ = new_ast_node(AST_OP_FARGS,$1);
}
| onepara ',' paras
{
    $1->parent = $3;
    $3->sons.push_back($1);
    $$ = $3;
}

onepara : type paradata 
{
	$$ = new_ast_node(AST_VAR_DECL, $1,$2);
};
paradata : ident 
{
	$$ = $1;
}
| ident paradatatail
{
	$$ = new_ast_node(AST_ARRAY, $1, $2);
};

paradatatail : '[' ']'
{
	$$ = new_ast_node(AST_DIMENSION);
}
| '[' num ']'
{
	$$ = new_ast_node(AST_DIMENSION, $2);
}
| paradatatail '[' num ']'
{
	$$ = new_ast_node(AST_DIMENSION, $3, $1);
};

/*关系运算*/
cmp : T_CMP
{
    struct ast_node_attr tempVal;
    tempVal.kind = CMP_KIND;
    tempVal.lineno = $1.lineno;
    strncpy(tempVal.id, $1.id, sizeof(tempVal.id));
    $$ = new_ast_leaf_node(tempVal);
}

/*函数声明或者是定义*/
functail : ';' 
{
	$$ = new_ast_node(AST_OP_EMPTY);
}
| statblock  //语句块，这里表示函数体
{
	$$ = $1;
}

statblock : '{' '}'
{
	$$ = new_ast_node(AST_OP_EMPTY);
}
| '{' subprogram '}'
{
	$$ = $2;
};

subprogram : sentenceblock
{
	$$ = new_ast_node(AST_SENTENCE_BLOCK, $1);
}
| subprogram sentenceblock
{
	$2->parent = $1;
	$1->sons.push_back($2);
	$$ = $1;
};

sentenceblock : sentence //非定义类语句
{
	$$ = $1;
}
| localvardef //局部变量定义语句
{
	$$ = $1;
}

/*非定义类语句*/
sentence : statblock //另一个语句块
{
	$$ = $1;
}
| expression ';'
{
	$$ = $1;
}
| T_RETURN expression ';'
{
	$$ = new_ast_node(AST_OP_RETURN, $2);
	$$->attr.lineno = $1.lineno;
}
| T_RETURN ';'
{
	$$ = new_ast_node(AST_OP_RETURN);
	$$->attr.lineno = $1.lineno;
}
| ';'   //空语句
{
	$$ = new_ast_node(AST_OP_EMPTY);
}
| T_IF '(' expression ')' sentence
{
	$$ = new_ast_node(AST_OP_IF, $3, $5);
}
| T_IF '(' expression ')' sentence T_ELSE sentence
{
	$$ = new_ast_node(AST_OP_IF, $3, $5, $7);
}
| T_WHILE '(' expression ')' sentence
{
	$$ = new_ast_node(AST_OP_WHILE, $3, $5);
}
| T_FOR '(' expression ';' expression ';' expression ')' sentence
{
	$$ = new_ast_node(AST_OP_FOR, $3, $5, $7, $9);
}
| T_BREAK ';'
{
	$$ = new_ast_node(AST_OP_BREAK);
	$$->attr.lineno=$1.lineno;
}
| T_CONTINUE ';'
{
	$$ = new_ast_node(AST_OP_CONTINUE);
	$$->attr.lineno=$1.lineno;
}


expression : unit
{
	$$ = $1;  //符号，数字
}
| leftVal '=' expression
{
	$$ = new_ast_node(AST_OP_ASSIGN, $1, $3);   //赋值语句
}
| expression '+' expression
{
	$$ = new_ast_node(AST_OP_ADD, $1, $3);
} 
| expression '-' expression
{
	$$ = new_ast_node(AST_OP_SUB, $1, $3);
}
| expression '*' expression
{
	$$ = new_ast_node(AST_OP_MUL, $1, $3);
}
| expression '/' expression
{
	$$ = new_ast_node(AST_OP_DIV, $1, $3);
}
| expression '%' expression
{
	$$ = new_ast_node(AST_OP_MOD, $1, $3);
}
| expression T_AND expression
{
	$$ = new_ast_node(AST_OP_AND, $1, $3);
}
| expression T_OR expression
{
	$$ = new_ast_node(AST_OP_OR, $1, $3); 
}
| expression cmp expression %prec PREC
{
	$$ = new_ast_node(AST_OP_CMP, $1,$2,$3);
}
| leftVal T_ASSIGN_DIGITOP expression
{
    ast_node *tempNode;
    if(!strcmp($2.id,"+=")){
        tempNode = new_ast_node(AST_OP_ADD,$1,$3);
    }else if(!strcmp($2.id,"-=")){
        tempNode = new_ast_node(AST_OP_SUB,$1,$3);
    }else if(!strcmp($2.id,"*=")){
        tempNode = new_ast_node(AST_OP_MUL,$1,$3);
    }else if(!strcmp($2.id,"/=")){
        tempNode = new_ast_node(AST_OP_DIV,$1,$3);
    }else if(!strcmp($2.id,"%=")){
        tempNode = new_ast_node(AST_OP_MOD,$1,$3);
    }else {
        yyerror("T_ASSIGN_DIGITOP ERROR!!!");
        return false;
    }
    $$ = new_ast_node(AST_OP_ASSIGN, $1, tempNode);
}

/*局部变量定义语句*/
localvardef : type defdata deflist
{
	ast_node* tempNode;
	tempNode = new_ast_node(AST_DEF_LIST, $1, $2);
	for(auto pit = $3->sons.end() - 1;pit != $3->sons.begin() - 1;--pit)
	{
		tempNode->sons.push_back(*pit);
		(*pit)->parent = tempNode;
	}
	$$ = tempNode;
}

deflist : ';' 
{ 
	$$ = new_ast_node(AST_DEF_LIST);
}
| ',' defdata deflist
{
	$2->parent = $3;
	$3->sons.push_back($2);
	$$= $3;
};

defdata : ident '=' expression
{
	$$ = new_ast_node(AST_OP_ASSIGN, $1, $3);
}
| ident
{
	$$ = $1;
}
| ident arraydef 
{
	$$ = new_ast_node(AST_ARRAY, $1, $2);
}
;

unit : rightVal
{
	$$ = $1;
}
| '-' unit
{
	$$ = new_ast_node(AST_OP_NEG, $2);
}
| '!' unit
{
	$$ = new_ast_node(AST_OP_NOT, $2);
}
| leftVal T_DEC
{
	$$ = new_ast_node(AST_OP_RDEC, $1);
}
| leftVal T_INC
{
	$$ = new_ast_node(AST_OP_RINC, $1);
}
| T_DEC leftVal
{
	$$ = new_ast_node(AST_OP_LDEC, $2);
}
| T_INC leftVal
{
	$$ = new_ast_node(AST_OP_LINC, $2);
}
;

rightVal : num
{
	$$ = $1;
}
| leftVal
{
	$$ = $1;
}
| '(' expression ')'
{
	$$ = $2;
}
| ident '(' realargs ')'   //有实参的函数调用
{
	$$ = new_ast_node(AST_FUNC_CALL, $1, $3);
}
| ident '(' ')'
{
	$$ = new_ast_node(AST_FUNC_CALL, $1);
}
;

realargs : expression
{
	$$ = new_ast_node(AST_REAL_ARGS, $1);
}
| realargs ',' expression
{
	$3->parent = $1;
	$1->sons.push_back($3);
	$$ = $1;
}

leftVal : ident
{
	$$ = $1;
}
| ident leftValtail
{
	ast_node* tempNode = new_ast_node(AST_ARRAY_INDEX, $1);
	for(auto pit = $2->sons.begin(); pit != $2->sons.end(); ++pit)
	{
		tempNode->sons.push_back(*pit);
		(*pit)->parent = tempNode;
	}
	$$ = tempNode;
}

leftValtail : '[' expression ']'
{
	$$ = new_ast_node(AST_DIMENSION, $2);
}
| leftValtail '[' expression ']'
{
	$3->parent = $1;
	$1->sons.push_back($3);
	$$ = $1;
}

num : T_DIGIT
{
	ast_node_attr Val;
	Val.kind = DIGIT_INT;
	Val.integer_val = $1.val;
	Val.lineno = $1.lineno;
	$$ = new_ast_leaf_node(Val);
};


%%

// 语法识别错误要调用函数的定义
void yyerror(char * msg)
{
    printf("Line %d: %s\n", yylineno, msg);
}
