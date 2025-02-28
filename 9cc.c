#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
	NK_ADD,
	NK_SUB,
	NK_MUL,
	NK_DIV,
	NK_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

Node* expr();
Node* mul();
Node* primary();
Node* unary();

typedef enum {
	TK_RESERVED,
	TK_NUM,
	TK_EOF,
}TokenKind;

typedef struct Token Token;

struct Token{
	TokenKind kind;
	int val;
	char *str;
	Token *next;
};

Token *token;
char* user_input;

void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) 
		return false;
	token = token->next;
	return true;
}

void expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) 
		error_at(token->str, "expect %c", op);
	token = token->next;
}

int expect_number() {
	if (token->kind != TK_NUM) 
		error_at(token->str, "expect number");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

Token* new_token(TokenKind kind, Token *cur, char* str) {
	Token* tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

Token* tokenize(char* p) {
	Token head;
	head.next = NULL;
	Token* cur = &head;
	while(*p) {
		if (*p == ' ') {
			p++;
			continue;
		}
		if (strchr("+-*/()", *p)) {
			cur = new_token(TK_RESERVED , cur, p);
			p++;
			continue;
		}
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}
		error_at(token->str, "can't tokenize");
	}
	new_token(TK_EOF, cur, p);
	return head.next;
}

Node* new_node(NodeKind kind, Node* lhs, Node* rhs) {
	Node* node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node* new_node_number(NodeKind kind, int val) {
	Node* node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->val = val;
	return node;
}


Node* expr() {
	Node* node = mul();
	for(;;) {
		if (consume('+')) {
			node = new_node(NK_ADD, node, mul());
		}
		else if(consume('-')) {
			node = new_node(NK_SUB, node, mul());
		}
		else return node;
	}
}

Node* mul() {
	Node* node = unary();
	for(;;) {
		if (consume('*')) {
			node = new_node(NK_MUL, node, unary());
		}
		else if(consume('/')) {
			node = new_node(NK_DIV, node, unary());
		}
		else return node;
	}
}

Node* unary() {
	if(consume('+')) {
		return primary();
	}
	if(consume('-')) {
		return new_node(NK_SUB, new_node_number(NK_NUM, 0), primary());
	}
	
	return	primary();
}

Node* primary() {
	
	if(consume('(')) {
		Node* node = expr();
		expect(')');
		return node;
	}

	return new_node_number(NK_NUM, expect_number());

}

void gen(Node* node) {
	if(node->kind == NK_NUM) {
		printf("	push %d\n", node->val);
		return;
	}
	
	gen(node->lhs);
	gen(node->rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");
	switch(node->kind) {
	    case NK_ADD:
			printf("	add rax, rdi\n");
			break;
		case NK_SUB:
			printf("	sub rax, rdi\n");
			break;
		case NK_MUL:
			printf("	imul rax, rdi\n");
			break;
		case NK_DIV:
			printf("	cqo\n");
			printf("	idiv rdi\n");
			break;
		}
	printf("	push rax\n");
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "input number wrong");
		return 1;
	}
	user_input = argv[1];
	token = tokenize(argv[1]);
	Node* node = expr();

	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");
	
	gen(node);

	printf("	pop rax\n");
	printf("    ret\n");

	return 0;
}
