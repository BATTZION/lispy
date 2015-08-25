/*
 * 词法分析器,适用lisp
 *
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "lispy.h"
#include <stdlib.h>
#define MAX_LINE 256 

char *symbol = "(\\)\'+-*/" ;
token *token_num(double num)
{
	token *result = (token *)malloc(sizeof(token));
	result->type = TOKEN_NUM;
	result->num = num;
	return result;
}
token *token_sym(char *sym)
{
	char *str = (char *)malloc(strlen(sym) + 1 );
	strcpy(str, sym);
	token *result = (token *)malloc(sizeof(token));
	result->type = TOKEN_SYM;
	result->sym = str;
	return result;
}

void free_token(token *a)
{
	switch(a->type){
		case TOKEN_NUM:
			free(a);
			break;
		case TOKEN_SYM:
			free(a->sym);
			free(a);
			break;
		default:
			break;
	}
}
/* 
 *函数:判断字符是否为单符号
 */
int is_symbol(char c)
{
	return (strchr(symbol, c) == NULL) ? 0 : 1;
}

static char *next_token(char *s, char *d)
{
	int i = 0, len, j = 0;
	len = strlen(s);
	char *line = s;
	//去处空格
	while ((line[i] == ' ' || line[i] == '\t') && i < len)
	  i++;

	if(i == len || line[i] == '\n')
	  return NULL;

	if(is_symbol(line[i])){
		d[j++] = line[i];
		d[j] = '\0';
		return line + i + 1;
	}
	while(i < len && line[i] != '\n' && line[i] != ' '){
		if(is_symbol(line[i])){
			d[j] = 0;
			return line+i;
		}
		d[j++] = line[i++]; 
	}
	d[j] = 0;
	return line + i;
}
int is_double(char *s)
{
	int i = 0, len = strlen(s), num_dot = 0;
	while(i< len){
		if(s[i] >= '0' && s[i] <= '9')
		  i++;
		else{
			if(s[i] == '.' && num_dot == 0){
				num_dot++;
				i++;
			}
			else
			  return 0;
		}
	}
	return 1;
}
token_result* tokenizer(char *s)
{
	char Token[MAX_LINE], *line = s;
	memset(Token, 0, MAX_LINE);
	
	token_result *result = malloc(sizeof(result));
	result->count = 0;
	result->cell = NULL;

		
	while((line =next_token(line, Token))){
		result->count++;
		result->cell = realloc(result->cell, sizeof(token *) * result->count);
		if(is_double(Token)){
			//printf("%s\n",Token);
			result->cell[result->count - 1] = token_num(strtod(Token,NULL));
		}
		else
		  result->cell[result->count - 1] = token_sym(Token);
		memset(Token, 0, MAX_LINE);
	}
	return result;
}

void print_token_result(token_result *out)
{
	int i = 0;
	while( i++ < out->count)
	  switch(out->cell[i - 1]->type){
		  case TOKEN_NUM:
			  printf("TYPE : NUM, Content : %G\n",out->cell[i-1]->num);
			  break;
		  case TOKEN_SYM:
			  printf("TYPE : SYM, Content : %s\n",out->cell[i-1]->sym);
			  break;
	  }
}
void free_token_result(token_result *s)
{
	int i = 0;
	while(i++ < s->count)
	  free_token(s->cell[i-1]);
	free(s);
}
