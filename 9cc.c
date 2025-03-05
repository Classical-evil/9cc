#include "9cc.h"

Token *token;
char *user_input;
Node* code[100];

void print_token() {
	Token* n = token;
	while(n->kind != TK_EOF) {
		printf("val:%d str:%c\n", n->val, n->str[0]);
		n = n->next;
	}
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "input number wrong");
		return 1;
	}
	user_input = argv[1];
	token = tokenize(argv[1]);
//	print_token();
	program();

	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");
	
	printf("	push rbp\n");
	printf("	mov rbp, rsp\n");
	printf("	sub rsp, 208\n");
	for(int i = 0;code[i];i++) {
		gen(code[i]);
		printf("	pop rax\n");
	}
	
	printf("	mov rsp, rbp\n");
	printf("	pop rbp\n");
	printf("    ret\n");

	return 0;
}
