// This file converts AST into assembly or executable code.
#include <stdio.h>
#include <stdlib.h>
#include "parser.c"  // Include the parser

void generate_code(ASTNode* node) {
    if (node->type == TOKEN_NUMBER) {
        printf("mov eax, %s\n", node->value);
    } else if (node->type == TOKEN_OPERATOR) {
        generate_code(node->left);
        generate_code(node->right);
        printf("add eax, ebx\n");
    }
}

void generate_code(ASTNode* node);
