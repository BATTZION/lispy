#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "lispy.h"

enum {LVAL_NUM, LVAL_SYM, LVAL_ERR, LVAL_SEXPR, LVAL_QEXPR};
lval *lval_number(double num)
{
	lval *value = malloc(sizeof(lval));
	value->type = LVAL_NUM;
	value->num = num;
	return value;
}
lval *lval_symbol(char *sym)
{
	lval *value = malloc(sizeof(lval));
	value->type = LVAL_SYM;
	value->sym = malloc(sizeof(sym) + 1);
	strcpy(value->sym, sym);
	return value;
}

lval *lval_sexpr()
{
	lval *value = malloc(sizeof(lval));
	value->type = LVAL_SEXPR;
	value->count = 0;
	value->cell = NULL;
	return value;
}
lval *lval_qexpr()
{
	lval *value = malloc(sizeof(lval));
	value->type = LVAL_QEXPR;
	value->count = 0;
	value->cell = NULL;
	return value;
}
lval *lval_err(char *str)
{
	lval *value = malloc(sizeof(lval));
	value->type = LVAL_ERR;
	value->err = malloc(strlen(str) + 1);
	strcpy(value->err, str);
	return value;
}
lval *lval_add(lval *dest, lval *src)
{
	dest->count++;
	dest->cell = realloc(dest->cell, sizeof(lval *) * dest->count);
	dest->cell[dest->count - 1] = src;
	return dest;
}

lval *lval_read(ast_tree *t)
{
	int i = 0;
	if(strstr(t->tag, "number"))
	  return lval_number(t->num);
	if(strstr(t->tag, "symbol"))
	  return lval_symbol(t->contents);
	lval *l = NULL;
	if(strstr(t->tag, "root") || strstr(t->tag, "sexpr"))
	  l = lval_sexpr();
	if(strstr(t->tag, "qexpr"))
	  l = lval_qexpr();
	for(i = 0; i < t->children_num; i++){
		if(strstr(t->children[i]->tag, "char"))
		  continue;
		else
		  lval_add(l, lval_read(t->children[i]));
	}
	return l;
}
void lval_del(lval *value)
{
	int i;
	switch(value->type){
		case LVAL_NUM:
			break;
		case LVAL_SYM:
			free(value->sym);
			break;
		case LVAL_ERR:
			free(value->err);
			break;
		case LVAL_SEXPR:
		case LVAL_QEXPR:
			for(i = 0; i < value->count; i++)
			  lval_del(value->cell[i]);
			free(value->cell);
			break;
	}
	free(value);
}
lval *lval_pop(lval *l, int i)
{
	lval *x =l->cell[i];
	memmove(&l->cell[i], &l->cell[i+1], sizeof(lval *) * (l->count - i - 1));
	l->count--;
	l->cell = realloc(l->cell,sizeof(lval*) *l->count);
	return x;
}
lval *lval_take(lval *l, int i)
{
	lval *x = lval_pop(l, i);
	lval_del(l);
	return x;
}
lval *builtin_arithmetic(lval *v, char *s)
{
	int i = 0;
	lval *x, *y;
	for( i = 0; i < v->count; i++)
	  if(v->cell[i]->type != LVAL_NUM)
		return lval_err("Can't operate on non-number");
	if( v->count == 0)
	  return v;
     x = lval_pop(v, 0);
	if (v->count == 0 && strcmp(s, "-") == 0)
	  x->num = -x->num;
	while(v->count > 0){
		y =lval_pop(v, 0);
		if(strcmp(s, "+") == 0)
		  x->num += y->num;
		if(strcmp(s, "-") == 0)
		  x->num -= y->num;
		if(strcmp(s, "*") == 0)
		  x->num *= y->num;
		if(strcmp(s, "/") == 0){
			if(y->num == 0){
				lval_del(x);
				lval_del(y);
				x = lval_err("Division by Zero!");
			}
			else
			  x->num /= y->num;
		}
	}
	lval_del(v);
	return x;
}
lval *builtin_head(lval *s)
{
	lval *x;
	//检查参数个数
	if(s->count != 1){
		lval_del(s);
		return lval_err("Function 'head' had passed too many arguements!");
	}
	x = lval_take(s, 0);
	//检查参数类型
	if(x->type != LVAL_QEXPR){
		lval_del(x);
	    return lval_err("Function 'head' passed incorrect types!");
	}
	if(x->count == 0){
		lval_del(x);
		return lval_err("Function 'head' passed an empty list!");
	}
	while(x->count > 1)
	  lval_del(lval_pop(x, 1));

	return x;
}
lval *builtin_tail(lval *s)
{
	lval *x;
	//检查参数个数
	if(s->count != 1){
		lval_del(s);
		return lval_err("Function 'tail' had passed too many arguements!");
	}
	x = lval_take(s, 0);
	//检查参数类型
	if(x->type != LVAL_QEXPR){
		lval_del(x);
	    return lval_err("Function 'tail' passed incorrect types!");
	}
	if(x->count == 0){
		lval_del(x);
		return lval_err("Function 'tail' passed an empty list!");
	}
	lval_del(lval_pop(x, 0));
	return x;
}
lval *builtin_list(lval *s)
{
	s->type = LVAL_QEXPR;
	return s;
}
lval *builtin_eval(lval *s)
{
	if(s->count != 1){
		lval_del(s);
		return lval_err("Function 'eval' passed too many arguments!");
	}
	if(s->cell[0]->type != LVAL_QEXPR){
		lval_del(s);
		return lval_err("Function 'eval' passed incorrect types!");
	}
	lval *x = lval_take(s, 0);
	x->type = LVAL_SEXPR;
	return lval_eval(x);
}
lval* lval_join(lval* x, lval* y){
	while (y->count)
	  x = lval_add(x, lval_pop(y, 0));
	lval_del(y);  
	return x;
}
lval *builtin_join(lval *a){
	int i;
	for(i = 0; i < a->count; i++){
		if(a->cell[i]->type != LVAL_QEXPR){
			lval_del(a);
			return lval_err("Fuction 'join' passed incorrect types!");
		}
	}
	lval* x = lval_pop(a, 0);
	while (a->count){
		x = lval_join(x, lval_pop(a, 0));
	}
	lval_del(a);
	return x;
}
lval *builtin_cons(lval *a)
{
	if(a->count != 2){
		lval_del(a);
		return lval_err("Function 'cons' with incorrect number of arguments!");
	}
	a->type = LVAL_QEXPR;
	return a;
}
lval *lval_builtin(lval *a, char *fun)
{
	if(!strcmp(fun, "list"))
	  return builtin_list(a);
	if(!strcmp(fun, "eval"))
	  return builtin_eval(a);
	if(!strcmp(fun, "head"))
	  return builtin_head(a);
	if(!strcmp(fun, "tail"))
	  return builtin_tail(a);
	if(!strcmp(fun, "join"))
	  return builtin_join(a);
	if(!strcmp(fun, "cons"))
	  return builtin_cons(a);
	if(strstr("+-*/", fun))
	  return builtin_arithmetic(a, fun);
	lval_del(a);
	return lval_err("Unkonw Function!");
}
lval *lval_eval(lval *s);
lval *lval_eval_sexp(lval *s)
{
	int i;
	for(i = 0; i < s->count; i++)
	  s->cell[i] = lval_eval(s->cell[i]);
	//检查错误
	for(i = 0; i < s->count; i++)
	  if(s->cell[i]->type == LVAL_ERR)
		return lval_take(s, i);
	if(s->count == 0)
	  return s;
	if(s->count == 1)
	  return s->cell[0];
	lval *function = lval_pop(s, 0);
	if(function->type != LVAL_SYM){
		lval_del(function);
		lval_del(s);
		return lval_err("S-EXPRESSION Does not start with symbol");

	}
	lval* result = lval_builtin(s, function->sym);
	lval_del(function);
	return result;
}

lval *lval_eval(lval *s)
{
	if(s->type == LVAL_SEXPR)
	  return lval_eval_sexp(s);
	return s;
}

void lval_single_print(lval *s);
void lval_expr_print(lval* v, char open, char close) {
	int i;
	putchar(open);
    for (i = 0; i < v->count; i++) { 
		lval_single_print(v->cell[i]);
		if (i != (v->count-1)) {
			putchar(' ');
		}
	}
	putchar(close);
}

void lval_single_print(lval *s)
{
	switch(s->type){
		case LVAL_NUM:
			printf("%G", s->num);
			break;
		case LVAL_ERR:
			printf("Error : %s", s->err);
			break;
		case LVAL_SYM:
			printf("%s", s->sym);
			break;
		case LVAL_QEXPR:
		case LVAL_SEXPR:
			lval_expr_print(s, '(', ')');
			break;
	}
}
void lval_print(lval *s)
{
	lval_single_print(s);
	putchar('\n');
}

