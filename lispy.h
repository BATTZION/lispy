#include <stdio.h>

//词法分析相关
struct token;
struct token_result;
typedef struct token token;
typedef struct token_result token_result;
enum {TOKEN_NUM, TOKEN_SYM};
struct token{
	int type;
	double num;
	char *sym;
};
/* 
 * 词法分析器,返回值
 */
struct token_result{
	int count;
	token **cell;
};

//语法分析相关
struct ast_tree;
typedef struct ast_tree ast_tree;
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


token_result *tokenizer(char *s);
void print_token_result(token_result *out);
void free_token_result(token_result *s);

int  parser(ast_tree **re, char *s);
void ast_print(ast_tree *, int);
void free_ast_tree(ast_tree *);
void ast_print_err(ast_tree*);


struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;
typedef lval *(*builtin_fun)(lenv *, lval *);
struct lval{
	int type;

	/* Basic */
	double num;
	char *err;
	char *sym;
    
	/* Function */
    builtin_fun builtin_fun;
	lenv *env;
	lval *formals;
	lval *body;

	/* Expression */
	int count;
	struct lval **cell;
	
};
lval *lval_eval(lenv *a, lval *b);
lval *lval_read(ast_tree *);
void  lval_del(lval *);
void  lval_print(lval *);

//lisp环境
struct lenv{
	int count;
	
	/* parents environment */
	
	lenv *parent;
	
	//符号表
	char **symbol;
	lval **value;
};

lenv *lenv_init();
void lenv_del();

