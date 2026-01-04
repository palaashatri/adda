// This file parses tokens into an Abstract Syntax Tree (AST).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.c"  // Include the lexer

typedef struct ASTNode {
    TokenType type;
    char* value;
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

// Create a new AST node
ASTNode* create_ast_node(TokenType type, char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    node->value = strdup(value);
    node->left = node->right = NULL;
    return node;
}

// Parse Expressions
ASTNode* parse_expression(Token* tokens) {
    ASTNode* node = create_ast_node(tokens->type, tokens->value);

    if (tokens->type == TOKEN_OPERATOR) {
        node->left = parse_expression(tokens + 1);
        node->right = parse_expression(tokens + 2);
    }

    return node;
}

ASTNode* parse_expression(Token* tokens);
