#include <stdio.h>

struct token;
struct token_result;
typedef struct token token;
typedef struct token_result token_result;

token_result *tokenizer(char *s);
void print_token_result(token_result *out);
void free_token_result(token_result *s);
