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
	NK_EQ,
	NK_NEQ,
	NK_L,
	NK_LE,
} NodeKind;

typedef struct Node Node;

struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

Node* expr();
Node* relational();
Node* add();
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
	int len;
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

bool consume(char *op) {
	if (token->kind != TK_RESERVED || 
		token->len != strlen(op) ||
		memcmp(token->str, op, token->len)) 
		return false;
	token = token->next;
	return true;
}

void expect(char* op) {
	if (token->kind != TK_RESERVED || 
		token->len != strlen(op) ||
		memcmp(token->str, op, token->len)) 
		error_at(token->str, "expect %s", op);
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

Token* new_token(TokenKind kind, Token *cur, char* str, int len) {
	Token* tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
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
			cur = new_token(TK_RESERVED , cur, p, 1);
			p++;
			continue;
		}
		if (strchr("<>!=", *p)) {
			if (p[1] == '=') {
				cur = new_token(TK_RESERVED, cur, p, 2);
				p += 2;
				continue;
			}
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 1);
			cur->val = strtol(p, &p, 10);
			continue;
		}
		error_at(token->str, "can't tokenize");
	}
	new_token(TK_EOF, cur, p, 1);
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

//expr = relational( "==" relational | "!=" relational)*
Node* expr() {
	Node *node = relational();
	for(;;) {
		if(consume("==")) {
			node = new_node(NK_EQ, node, relational());
		}
		else if(consume("!=")) {
			node = new_node(NK_NEQ, node, relational());
		}
		else return node;
	}
}

//relational = add("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
	Node* node = add();
	for(;;) {
		if(consume(">")) {
			node = new_node(NK_L, add(), node);
		}
		else if(consume(">=")) {
			node = new_node(NK_LE, add(), node);
		}
		else if(consume("<")) {
			node = new_node(NK_L, node, add());
		}
		else if(consume("<=")) {
			node = new_node(NK_LE, node, add());
		}
		else return node;
	}
}

//expr = mul("+" mul | "-" mul)*
Node* add() {
	Node* node = mul();
	for(;;) {
		if (consume("+")) {
			node = new_node(NK_ADD, node, mul());
		}
		else if(consume("-")) {
			node = new_node(NK_SUB, node, mul());
		}
		else return node;
	}
}

//mul = unary("*" unary | "/" unary)* 
Node* mul() {
	Node* node = unary();
	for(;;) {
		if (consume("*")) {
			node = new_node(NK_MUL, node, unary());
		}
		else if(consume("/")) {
			node = new_node(NK_DIV, node, unary());
		}
		else return node;
	}
}

//unary = ("+" | "-")? primary
Node* unary() {
	if(consume("+")) {
		return primary();
	}
	if(consume("-")) {
		return new_node(NK_SUB, new_node_number(NK_NUM, 0), primary());
	}
	
	return	primary();
}

//primary = num | "(" expr ")"
Node* primary() {
	
	if(consume("(")) {
		Node* node = expr();
		expect(")");
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
		case NK_EQ: 
			printf("	cmp rax, rdi\n");
			printf("	sete al\n");
			printf("	movzb rax, al\n");
			break;
		case NK_NEQ: 
			printf("	cmp rax, rdi\n");
			printf("	setne al\n");
			printf("	movzb rax, al\n");
			break;
		case NK_L: 
			printf("	cmp rax, rdi\n");
			printf("	setl al\n");
			printf("	movzb rax, al\n");
			break;
		case NK_LE: 
			printf("	cmp rax, rdi\n");
			printf("	setle al\n");
			printf("	movzb rax, al\n");
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
