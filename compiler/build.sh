gcc -arch arm64 -o toylang main.c lexer.c parser.c codegen.c
# gcc -o toylang lexer.o parser.o codegen.o main.o
./toylang hello.toy