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
	while(1){
		char *input = readline("lispy> ");
		add_history(input);
		if(!strcmp(input, "exit")){
			puts("have fun!");
			return 0;
		}
		result = tokenizer(input);
	   //print_token_result(result);
        
		
		t =parser(result);
		ast_print(t, 0);
		free_ast_tree(t);
		free_token_result(result);
		free(input);
	}
	return 0;
}
