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

extern Token *token;
extern char* user_input;

void error_at(char *loc, char *fmt, ...);  
bool consume(char *op);;  
void expect(char* op);  
int expect_number();  
bool at_eof();  
Token* new_token(TokenKind kind, Token *cur, char* str, int len);  
Token* tokenize(char* p);  
Node* new_node(NodeKind kind, Node* lhs, Node* rhs);  
Node* new_node_number(NodeKind kind, int val);  
Node* expr();  
Node *relational();  
Node* add();  
Node* mul();  
Node* unary();  
Node* primary();  
void gen(Node* node);  

