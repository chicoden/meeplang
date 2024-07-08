#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct {
    char *string;
    int lineno;
    enum {
        Token_COMMENT,
        Token_NEWLINE,
        Token_WHITESPACE,
        Token_STRING,
        Token_NUMBER,
        Token_ID,
        Token_KEYWORD,
        Token_SYMBOL
    } kind;
} Token;

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

void storeToken(FILE *stream, long tokenStart, Token *token) {
    long length = ftell(stream) - tokenStart;
    token->string = (char *)malloc((length + 1) * sizeof(char));
    fseek(stream, tokenStart, SEEK_SET);
    fread(token->string, sizeof(char), length, stream);
    token->string[length] = '\0';
}

int scanToken_COMMENT(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int ch = fgetc(stream);
    if (ch != '#') {
        fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    while (1) {
        ch = fgetc(stream);
        if (ch == '\n') fseek(stream, -1, SEEK_CUR);
        if (ch == '\n' || ch == EOF) break;
    }

    storeToken(stream, tokenStart, token);
    token->kind = Token_COMMENT;
    return 1;
}

int scanToken_NEWLINE(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int ch = fgetc(stream);
    if (ch != '\n') {
        fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    storeToken(stream, tokenStart, token);
    token->kind = Token_NEWLINE;
    return 1;
}

int scanToken_WHITESPACE(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int ch = fgetc(stream);
    if (ch != ' ' && ch != '\t') {
        fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    while (1) {
        ch = fgetc(stream);
        if (ch != ' ' && ch != '\t') {
            fseek(stream, -1, SEEK_CUR);
            break;
        }
    }

    storeToken(stream, tokenStart, token);
    token->kind = Token_WHITESPACE;
    return 1;
}

int scanToken_STRING(FILE *stream, Token *token) {
    return 0;
}

int scanToken_NUMBER(FILE *stream, Token *token) {
    return 0;
}

int scanToken_ID(FILE *stream, Token *token) {
    return 0;
}

int scanToken_SYMBOL(FILE *stream, Token *token) {
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Expected 2 arguments (input file, output file), got %i instead\n", argc - 1);
        return 0;
    }

    FILE *infile = fopen(argv[1], "r");
    if (infile == NULL) {
        printf("%s is not a file\n", argv[1]);
        return 0;
    }

    Token *tokens = NULL;
    int numTokens = 0;
    int lineno = 1;
    while (1) {
        Token curToken;
        if (
            scanToken_COMMENT(infile, &curToken) ||
            scanToken_NEWLINE(infile, &curToken) ||
            scanToken_WHITESPACE(infile, &curToken) ||
            scanToken_STRING(infile, &curToken) ||
            scanToken_NUMBER(infile, &curToken) ||
            scanToken_ID(infile, &curToken) ||
            scanToken_SYMBOL(infile, &curToken)
        ) {
            curToken.lineno = lineno;
            if (curToken.kind == Token_NEWLINE) lineno++;

            switch (curToken.kind) {
                case Token_COMMENT:
                    printf("COMMENT on line %i\n", curToken.lineno);
                    printQuote(curToken.string);
                    putchar('\n');
                    break;

                case Token_NEWLINE:
                    printf("NEWLINE on line %i\n", curToken.lineno);
                    printQuote(curToken.string);
                    putchar('\n');
                    break;

                case Token_WHITESPACE:
                    printf("WHITESPACE on line %i\n", curToken.lineno);
                    printQuote(curToken.string);
                    putchar('\n');
                    break;

                case Token_STRING:
                    printf("STRING on line %i\n", curToken.lineno);
                    printf("%s\n", curToken.string);
                    break;

                case Token_NUMBER:
                    printf("NUMBER on line %i\n", curToken.lineno);
                    printf("%s\n", curToken.string);
                    break;

                case Token_ID:
                    printf("ID on line %i\n", curToken.lineno);
                    printf("%s\n", curToken.string);
                    break;

                case Token_SYMBOL:
                    printf("SYMBOL on line %i\n", curToken.lineno);
                    printf("%s\n", curToken.string);
                    break;

                default:
                    break;
            }

            tokens = (Token *)realloc(tokens, numTokens++ * sizeof(Token));
            tokens[numTokens - 1] = curToken;
        } else if (feof(infile)) {
            break;
        } else {
            printf("error: unexpected token on line %i\n", lineno);
            fclose(infile);
            return 0;
        }
    }

    fclose(infile);

    // Do something with token array

    for (int i = 0; i < numTokens; i++) free(tokens[i].string);
    free(tokens);
    return 0;
}