#include "9cc.h"

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

