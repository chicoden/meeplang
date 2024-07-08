#include <stdio.h>
#include <ctype.h>
#include "../src/token.h"

void printQuote(char *string) {
    putchar('"');

    while (*string != '\0') {
        char ch = *(string++);
        if (isprint(ch)) putchar(ch);
        else if (ch == '\t') printf("\\t");
        else if (ch == '\n') printf("\\n");
        else printf("%02x", ch);
    }

    putchar('"');
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Expected 2 arguments (input file, output file), got %i instead\n", argc - 1);
        return 0;
    }

    TokenScanner scanner;
    if (!initTokenScanner(&scanner, fopen(argv[1], "r"))) {
        printf("%s is not a file\n", argv[1]);
        return 0;
    }

    while (1) {
        Token token;
        int code = scanToken(&scanner, &token);
        if (code == 1) {
            switch (token.kind) {
                case Token_COMMENT:
                    printf("COMMENT on line %i\n", token.lineno);
                    printQuote(token.string);
                    putchar('\n');
                    break;

                case Token_NEWLINE:
                    printf("NEWLINE on line %i\n", token.lineno);
                    printQuote(token.string);
                    putchar('\n');
                    break;

                case Token_WHITESPACE:
                    printf("WHITESPACE on line %i\n", token.lineno);
                    printQuote(token.string);
                    putchar('\n');
                    break;

                case Token_STRING:
                    printf("STRING on line %i\n", token.lineno);
                    printf("%s\n", token.string);
                    break;

                case Token_NUMBER:
                    printf("NUMBER on line %i\n", token.lineno);
                    printf("%s\n", token.string);
                    break;

                case Token_ID:
                    printf("ID on line %i\n", token.lineno);
                    printf("%s\n", token.string);
                    break;

                case Token_SYMBOL:
                    printf("SYMBOL on line %i\n", token.lineno);
                    printf("%s\n", token.string);
                    break;

                default:
                    break;
            }
        } else if (code == 0) {
            printf("EOF\n");
            break;
        } else {
            printf("error: unexpected token on line %i\n", scanner.lineno);
            break;
        }
    }

    deinitTokenScanner(&scanner);
    return 0;
}