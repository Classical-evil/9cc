#include "9cc.h"

Token *token;
char *user_input;
Node* code[100];
LVal* locals;

void print_token() {	
	Token * tok = token;
	while(tok->kind != TK_EOF) {
		printf("%c -> ", tok->str[0]);
		tok = tok->next;
	} 
	printf("\n");
}

void init() {
	locals = calloc(1, sizeof(LVal));
	locals->next = NULL;
	locals->len = 0;
	locals->name = NULL;
	locals->offset = 0;
}

int calc_locals() {
	LVal* lval = locals;
	int count = 0;
	while(lval) {
		count++;
		lval = lval->next;
	}
	return count;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "input number wrong");
		return 1;
	}
	init();
	user_input = argv[1];
	token = tokenize(argv[1]);
//	print_token();
	program();

	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");
	printf("	push rbp\n");
	printf("	mov rbp, rsp\n");
	printf("	sub rsp, %d\n", (calc_locals() - 1) * 8);
	for(int i = 0;code[i];i++) {
		gen(code[i]);
		printf("	pop rax\n");
	}
	
	printf("	mov rsp, rbp\n");
	printf("	pop rbp\n");
	printf("    ret\n");

	return 0;
}
