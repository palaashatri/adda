// This file tokenizes the input source code.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOKEN_VAR, TOKEN_PRINT, TOKEN_NUMBER, TOKEN_IDENTIFIER,
    TOKEN_OPERATOR, TOKEN_IF, TOKEN_ELSE, TOKEN_LPAREN, TOKEN_RPAREN,
    TOKEN_SEMICOLON, TOKEN_ASSIGN, TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char* value;
} Token;

// Function to tokenize input source code
Token get_next_token(char** input) {
    while (**input == ' ') (*input)++;  // Skip whitespace

    Token token;
    if (isdigit(**input)) {  // Numbers
        token.type = TOKEN_NUMBER;
        token.value = strndup(*input, 1);
        (*input)++;
    } else if (isalpha(**input)) {  // Identifiers (Variables, Keywords)
        token.type = TOKEN_IDENTIFIER;
        token.value = strndup(*input, 1);
        (*input)++;
    } else if (**input == '+') {  // Operators
        token.type = TOKEN_OPERATOR;
        token.value = "+";
        (*input)++;
    } else if (**input == ';') {  // Semicolons
        token.type = TOKEN_SEMICOLON;
        token.value = ";";
        (*input)++;
    } else {
        token.type = TOKEN_EOF;
        token.value = NULL;
    }

    return token;
}

Token get_next_token(char** input);