#include "9cc.h"

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
		if (strchr("+-*/();", *p)) {
			cur = new_token(TK_RESERVED , cur, p, 1);
			p++;
	//		printf("+-/;*p:%c\n", *p);
			continue;
		}
		if (*p >= 'a' && *p <= 'z') {
			int len = 0;
			while(*p>='a' && *p<='z') {
	//			printf("a-z*p;%c\n", p[len]);
				len++;
				p++;
			}
			cur = new_token(TK_IDENT, cur, p, 3);
	//		printf("a-z*p;%c\n", *p);
	//		p += len;
			continue;
		}
 		if (strchr("<>!=", *p)) {
			if (p[1] == '=') {
				cur = new_token(TK_RESERVED, cur, p, 2);
				p += 2;
				continue;
			}
			cur = new_token(TK_RESERVED, cur, p, 1);
	//		printf("strchr*p:%c\n", *p);
			p++;
			continue;
		}
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 1);
	//		printf("isdigit*p:%c\n", *p);
			cur->val = strtol(p, &p, 10);
			continue;
		}
		error_at(token->str, "can't tokenize");
	}
	new_token(TK_EOF, cur, p, 1);
	return head.next;
}

