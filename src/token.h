#ifndef __MEEP_TOKEN_HEADER
#define __MEEP_TOKEN_HEADER

#include <stdio.h>
#include <stdlib.h>

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
        Token_SYMBOL
    } kind;
} Token;

typedef struct {
    FILE *stream;
    Token *tokens;
    int numTokens;
    int lineno;
} TokenScanner;

void storeToken(FILE *stream, long tokenStart, Token *token) {
    long length = ftell(stream) - tokenStart;
    token->string = (char *)malloc((length + 1) * sizeof(char));
    fseek(stream, tokenStart, SEEK_SET);
    fread(token->string, sizeof(char), length, stream);
    token->string[length] = '\0';
}

int isBinDigit(int ch) {
    return ch == '0' || ch == '1';
}

int isOctDigit(int ch) {
    return ch >= '0' && ch <= '7';
}

int isDecDigit(int ch) {
    return ch >= '0' && ch <= '9';
}

int isHexDigit(int ch) {
    return (ch >= '0' && ch <= '9') ||
           (ch >= 'a' && ch <= 'f') ||
           (ch >= 'A' && ch <= 'F');
}

// /#.*/
int scanToken_COMMENT(FILE *stream, Token *token) {
    int ch = fgetc(stream);
    if (ch != '#') {
        if (ch != EOF) fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    long tokenStart = ftell(stream);
    while (1) {
        ch = fgetc(stream);
        if (ch == '\n') fseek(stream, -1, SEEK_CUR);
        if (ch == '\n' || ch == EOF) break;
    }

    storeToken(stream, tokenStart, token);
    token->kind = Token_COMMENT;
    return 1;
}

// /\n/
int scanToken_NEWLINE(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int ch = fgetc(stream);
    if (ch != '\n') {
        if (ch != EOF) fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    storeToken(stream, tokenStart, token);
    token->kind = Token_NEWLINE;
    return 1;
}

// /[ \t]+/
int scanToken_WHITESPACE(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int ch = fgetc(stream);
    if (ch != ' ' && ch != '\t') {
        if (ch != EOF) fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    while (1) {
        ch = fgetc(stream);
        if (ch != ' ' && ch != '\t') {
            if (ch != EOF) fseek(stream, -1, SEEK_CUR);
            break;
        }
    }

    storeToken(stream, tokenStart, token);
    token->kind = Token_WHITESPACE;
    return 1;
}

// /"([\s\S]|[^"])*"/
int scanToken_STRING(FILE *stream, Token *token) {
    int ch = fgetc(stream);
    if (ch != '"') {
        if (ch != EOF) fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    long tokenStart = ftell(stream);
    while (1) {
        ch = fgetc(stream);
        if (ch == '\\') {
            ch = fgetc(stream);
        } else if (ch == '"') {
            fseek(stream, -1, SEEK_CUR);
            break;
        } if (ch == EOF) {
            fseek(stream, tokenStart - 1, SEEK_SET);
            return 0;
        }
    }

    storeToken(stream, tokenStart, token);
    token->kind = Token_STRING;
    fseek(stream, 1, SEEK_CUR);
    return 1;
}

// /(0b[01]+)|(0o[0-7]+)|(0x[0-9a-fA-F]+)|[0-9]+/
int scanToken_NUMBER(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int (*isValidDigit)(int) = &isDecDigit;
    int digitsRead = 1;

    int ch = fgetc(stream);
    if (!isdigit(ch)) {
        if (ch != EOF) fseek(stream, -1, SEEK_CUR);
        return 0;
    } if (ch == '0') {
        int format = fgetc(stream);
        digitsRead = 0;
        if (format == 'b') {
            isValidDigit = &isBinDigit;
        } else if (format == 'o') {
            isValidDigit = &isOctDigit;
        } else if (format == 'x') {
            isValidDigit = &isHexDigit;
        } else {
            if (format != EOF) fseek(stream, -2, SEEK_CUR);
            if (isalnum(format)) return 0;
        }
    }

    while (1) {
        ch = fgetc(stream);
        if (!isValidDigit(ch)) {
            if (isalnum(ch)) {
                fseek(stream, tokenStart, SEEK_SET);
                return 0;
            }

            if (ch != EOF) fseek(stream, -1, SEEK_CUR);
            break;
        }

        digitsRead++;
    }

    if (digitsRead == 0) {
        fseek(stream, tokenStart, SEEK_SET);
        return 0;
    }

    storeToken(stream, tokenStart, token);
    token->kind = Token_NUMBER;
    return 1;
}

// /[_a-zA-Z][_a-zA-Z0-9]*/
int scanToken_ID(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int ch = fgetc(stream);
    if (ch != '_' && !isalpha(ch)) {
        if (ch != EOF) fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    while (1) {
        ch = fgetc(stream);
        if (ch != '_' && !isalnum(ch)) {
            if (ch != EOF) fseek(stream, -1, SEEK_CUR);
            break;
        }
    }

    storeToken(stream, tokenStart, token);
    token->kind = Token_ID;
    return 1;
}

int scanToken_SYMBOL(FILE *stream, Token *token) {
    return 0;
}

int initTokenScanner(TokenScanner *scanner, FILE *stream) {
    if (stream == NULL) return 0;
    scanner->stream = stream;
    scanner->tokens = NULL;
    scanner->numTokens = 0;
    scanner->lineno = 1;
    return 1;
}

int deinitTokenScanner(TokenScanner *scanner) {
    for (int i = 0; i < scanner->numTokens; i++) free(scanner->tokens[i].string);
    free(scanner->tokens);
    fclose(scanner->stream);
    return 1;
}

int scanToken(TokenScanner *scanner, Token *token) {
    if (
        scanToken_COMMENT(scanner->stream, token) ||
        scanToken_NEWLINE(scanner->stream, token) ||
        scanToken_WHITESPACE(scanner->stream, token) ||
        scanToken_STRING(scanner->stream, token) ||
        scanToken_NUMBER(scanner->stream, token) ||
        scanToken_ID(scanner->stream, token) ||
        scanToken_SYMBOL(scanner->stream, token)
    ) {
        token->lineno = scanner->lineno;
        if (token->kind == Token_NEWLINE) scanner->lineno++;
        scanner->tokens = (Token *)realloc(scanner->tokens, ++(scanner->numTokens) * sizeof(Token));
        scanner->tokens[scanner->numTokens - 1] = *token;
        return 1;
    } else if (feof(scanner->stream)) {
        return 0;
    } else {
        return -1;
    }
}

#endif