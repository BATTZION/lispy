/*
 *语法分析器,适用于lisp
 *
 * GOAL-> EXPRSSION*
 *
 * SEXPRESSION -> ( EXPR* )
 * QEXPRESSION ->  '( EXPR* )
 * 
 * EXPRESSION  -> number | symbol | SEXPRESSION | QEXPRESSION
 *
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "lispy.h"

struct ast_tree{
	//节点类型
	char *tag;
    //节点内容
	char *contents;
	double num;
	//子节点数目
	int children_num;
	//子节点
	ast_tree **children;
};
static int sp = 0;

void ast_add_tag(ast_tree *a, const char *tag)
{
	a->tag = realloc(a->tag, strlen(tag) + 1 + strlen(a->tag) + 1);
	memmove(a->tag + strlen(tag) + 1, a->tag, strlen(a->tag)+1);
	memmove(a->tag, tag, strlen(tag));
	memmove(a->tag + strlen(tag), "|", 1);
}

void ast_tag(ast_tree *tree, const char *tag)
{
	tree->tag = realloc(tree->tag, strlen(tag) + 1);
	memset(tree->tag, 0, strlen(tag) + 1);
	strcpy(tree->tag, tag);
}

//从词法分析流中获取下一个词
token *next_token(token_result *result)
{
	if(sp < result->count)
	  return result->cell[sp++];
	else
	  return NULL;
}
//将词重新放回词法分析流中
void putback_token()
{
	if(sp > 0)
	  sp--;
}
//创建语法树
ast_tree *new_tree()
{
	ast_tree *tree = malloc(sizeof(ast_tree));
	tree->tag = NULL;
	tree->contents = NULL;
	tree->children_num = 0;
	tree->children = NULL;
	return tree;
}
ast_tree *parser_expr(token_result *result);

//分析数字
ast_tree *parser_num(ast_tree *tree, double number)
{
	ast_add_tag(tree, "number");
	tree->num = number;
	return tree;
}
//分析符号
ast_tree *parser_sym(ast_tree *tree, char *sym)
{
	ast_add_tag(tree, "symbol");
	tree->contents = malloc(strlen(sym) + 1);
	memset(tree->contents, 0, strlen(sym) + 1);
	strcpy(tree->contents,sym);
	return tree;
}
ast_tree *parser_char(char *s)
{
	ast_tree *tree = new_tree();
	ast_tag(tree, "char");
	tree->contents = malloc(strlen(s) + 1);
	strcpy(tree->contents, s);
	return tree;
}
ast_tree *ast_tree_add(ast_tree *tree, ast_tree *add)
{
	tree->children_num++;
	tree->children = realloc(tree->children, sizeof(ast_tree *) * tree->children_num);
	tree->children[tree->children_num - 1] = add;
	return tree;
}
//语法分析错误
ast_tree *parser_err(char *err)
{
	ast_tree *tree = new_tree();
	ast_tag(tree, "error");
	tree->contents = malloc(strlen(err) + 1);
	strcpy(tree->contents, err);
	return tree;
}
//分析 sexpr
ast_tree *parser_sexpr(token_result *result)
{
	token *t;
	ast_tree *tree = new_tree(), *temp;
	ast_tag(tree, "sexpr");
    //读取前括号
	t = next_token(result);
	ast_tree_add(tree, parser_char(t->sym));
	
	while((t = next_token(result))){
		if(t->type == TOKEN_SYM && strcmp(t->sym, ")") == 0){
			ast_tree_add(tree, parser_char(t->sym));
			  return tree;
		}
		putback_token();
		temp = parser_expr(result);
		//出错则直接返回
		if(strstr(temp->tag, "error"))
		  return temp;
		ast_tree_add(tree, temp);
	}
	return parser_err("error: miss the back quote!");

}
// 分析 qexpr
ast_tree *parser_qexpr(token_result *result)
{
	token *t;
	ast_tree *tree = new_tree(), *temp;
	ast_tag(tree, "qexpr");
	//读取引号
	t = next_token(result);
	ast_tree_add(tree, parser_char(t->sym));
	//检查是否为Q_EXPR
	t = next_token(result);
	if( t->type != TOKEN_SYM || strcmp(t->sym, "(") != 0)
	  return parser_err("error : QEXPRSSION miss left brace");
	else
	  ast_tree_add(tree, parser_char(t->sym));
	while((t = next_token(result))){
		if(t->type == TOKEN_SYM && strcmp(t->sym, ")") == 0){
			ast_tree_add(tree, parser_char(t->sym));
			  return tree;
		}
		putback_token();
		temp = parser_expr(result);
		if(strstr(temp->tag, "error"))
		  return temp;
		ast_tree_add(tree, temp);
	}
	return parser_err("error: miss the back quote!");
}

//分析 expr
ast_tree *parser_expr(token_result *result)
{
	ast_tree *tree;
	token *t;
	if((t = next_token(result)) == NULL)
	  return NULL;

	if(t->type == TOKEN_NUM){
		tree = new_tree();
		ast_tag(tree,"expr");
		return parser_num(tree, t->num);
	}

	if(t->type == TOKEN_SYM){
		if(strcmp(t->sym, "(") == 0){
			putback_token();
		    return  parser_sexpr(result);
		}
		if(strcmp(t->sym, "'") == 0){
			putback_token();
			return parser_qexpr(result);
		}
		else{
			tree = new_tree();
			ast_tag(tree,"expr");
			return parser_sym(tree, t->sym);
		}
	}
}

ast_tree *parser(token_result *result)
{
	ast_tree *tree = new_tree();
	ast_tag(tree, "root");
	ast_tree *children;

	while((children = parser_expr(result)))
	  ast_tree_add(tree, children);
	return tree;
}
void ast_print(ast_tree * t, int len)
{
	int i = 0;
	ast_tree *child;
	
	if(len == 0 && t->children_num != 0)
	  printf("root|>:\n");
    for(i = 0; i < t->children_num; i++){
		child = t->children[i];
		if(strstr(child->tag, "error")){
		  printf("error: %s\n", child->contents);
		  return;
		}
	}
	for(i = 0; i < t->children_num; i++){
		child = t->children[i];
		if(strstr(child->tag, "symbol") || strstr(child->tag, "char"))
		  printf("%*s%s: \'%s\'\n", len+2, " ", child->tag, child->contents);
		if(strstr(child->tag, "num"))
		  printf("%*s%s: \'%G\'\n", len+2, " ", child->tag, child->num);
		if(strstr(child->tag, "sexpr")){
			printf("%*ssexpr|>:\n",len+2, " ");
			ast_print(child, len + 2);
		}
		if(strstr(child->tag, "qexpr")){
			printf("%*sqexpr|>\n",len + 2, " ");
			ast_print(child, len + 2);
		}
	}
}
void free_ast_tree(ast_tree *t)
{
	int i = 0;
	ast_tree *child;
	for(i = 0; i < t->children_num; i++){
	  child = t->children[i];
		if(strstr(child->tag,"num"))
		  free(child);
		else if(strstr(child->tag, "symbol") || strstr(child->tag, "char") || strstr(child->tag, "error")){
			free(child->contents);
			free(child);
		}
		else
		  free_ast_tree(child);
	}
	sp = 0;
	free(t);
}
