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
	while(1){
		char *input = readline("lispy> ");
		add_history(input);
		result = tokenizer(input);
		print_token_result(result);
		free_token_result(result);
		free(input);
	}
	return 0;
}
