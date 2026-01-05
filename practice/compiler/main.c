#include <stdio.h>
#include "lexer.c"
#include "parser.c"
#include "codegen.c"

int main() {
    char source[] = "var x = 42; print x + 5;";
    char* input = source;

    // Step 1: Tokenization
    Token token = get_next_token(&input);

    // Step 2: Parsing
    Token tokens[] = { token, get_next_token(&input), get_next_token(&input) };
    ASTNode* ast = parse_expression(tokens);

    // Step 3: Code Generation
    generate_code(ast);

    return 0;
}
