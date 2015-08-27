#include "lispy.h"
#include <editline/readline.h>
#include <editline/history.h>
#include <stdio.h>
#include <malloc.h>

int main(int argc, char **argv)
{
	puts("Welcome To My Tiny_lisp!");
	puts("version 0.0!");
	
	token_result *result;
	ast_tree *t;
	lval *v;
	lenv *e;
	e = lenv_init();
	while(1){
		char *input = readline("lispy> ");
		add_history(input);
		if(!strcmp(input, "exit")){
			puts("have fun!");
			return 0;
		}
		result = tokenizer(input);	
		if(parser(result, &t)){
			v = lval_eval(e, lval_read(t));
			
			lval_print(v);
			lval_del(v);
			free_ast_tree(t);
			free_token_result(result);
		}
		else{
			ast_print_err(t);
			free_token_result(result);
			free_ast_tree(t);
		}
		free(input);
	}
	lenv_del(e);
	return 0;
}
