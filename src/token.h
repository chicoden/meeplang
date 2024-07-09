#ifndef __MEEP_TOKEN_HEADER
#define __MEEP_TOKEN_HEADER

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
        Token_FLOAT,
        Token_INTEGER,
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

    // Comments must start with #
    if (ch != '#') {
        if (ch != EOF) fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    // Scan until a newline
    long tokenStart = ftell(stream);
    do {
        ch = fgetc(stream);
    } while (ch != '\n' && ch != EOF);

    if (ch != EOF) fseek(stream, -1, SEEK_CUR);
    storeToken(stream, tokenStart, token);
    token->kind = Token_COMMENT;
    return 1;
}

// /\n/
int scanToken_NEWLINE(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int ch = fgetc(stream);

    // Check if there is a newline
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

    // Check that there is at least one whitespace
    if (ch != ' ' && ch != '\t') {
        if (ch != EOF) fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    // Consume any following whitespace
    do {
        ch = fgetc(stream);
    } while (ch == ' ' || ch == '\t');

    if (ch != EOF) fseek(stream, -1, SEEK_CUR);
    storeToken(stream, tokenStart, token);
    token->kind = Token_WHITESPACE;
    return 1;
}

// /"([\s\S]|[^"])*"/
int scanToken_STRING(FILE *stream, Token *token) {
    int ch = fgetc(stream);

    // Strings must start with "
    if (ch != '"') {
        if (ch != EOF) fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    long tokenStart = ftell(stream);
    while (1) {
        ch = fgetc(stream);
        if (ch == '\\') {
            // Consume an extra character after escapes
            ch = fgetc(stream);
        } else if (ch == '"') {
            // Stop when we reach a closing "
            fseek(stream, -1, SEEK_CUR);
            break;
        } if (ch == '\n' || ch == EOF) {
            // Reached newline or EOF without terminating the string
            fseek(stream, tokenStart - 1, SEEK_SET);
            return 0;
        }
    }

    storeToken(stream, tokenStart, token);
    token->kind = Token_STRING;
    fseek(stream, 1, SEEK_CUR);
    return 1;
}

// /[\+\-]?([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+)/
int scanToken_FLOAT(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int ch = fgetc(stream);

    // + or - are ok
    if (ch == '+' || ch == '-') ch = fgetc(stream);

    // Check whether the first character is a digit
    // If so, be sure to consume any further digits
    int hasWholePart = isDecDigit(ch);
    if (hasWholePart) {
        while (isDecDigit(ch)) ch = fgetc(stream);
    }

    // Now we expect a decimal point if this is a float
    if (ch != '.') {
        fseek(stream, tokenStart, SEEK_SET);
        return 0;
    }

    // If there is no whole part (digits to the left of the decimal point),
    // There must be at least one digit after the decimal point
    if (!hasWholePart) {
        ch = fgetc(stream);
        if (!isDecDigit(ch)) {
            fseek(stream, tokenStart, SEEK_SET);
            return 0;
        }
    }

    // Consume any further digits
    do {
        ch = fgetc(stream);
    } while (isDecDigit(ch));

    // This is not a float if it contains alphabet characters
    if (isalpha(ch)) {
        fseek(stream, tokenStart, SEEK_SET);
        return 0;
    }

    if (ch != EOF) fseek(stream, -1, SEEK_CUR);
    storeToken(stream, tokenStart, token);
    token->kind = Token_FLOAT;
    return 1;
}

// /(0b[01]+)|(0o[0-7]+)|(0x[0-9a-fA-F]+)|([1-9][0-9]*)/
int scanToken_INTEGER(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int ch = fgetc(stream);

    // Check that the first character is a digit
    if (!isDecDigit(ch)) {
        if (ch != EOF) fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    int anyDigits = 1;
    int (*isValidDigit)(int) = &isDecDigit;
    if (ch == '0') {
        // If the first digit is 0, check the next character for a literal format
        int format = fgetc(stream);
        anyDigits = 0; // First digit is no longer part of the actual value
        if (format == 'b') { // Binary literal
            isValidDigit = &isBinDigit;
        } else if (format == 'o') { // Octal literal
            isValidDigit = &isOctDigit;
        } else if (format == 'x') { // Hex literal
            isValidDigit = &isHexDigit;
        } else {
            if (format != EOF) fseek(stream, -2, SEEK_CUR); // Restart scanning
            if (isalnum(format)) return 0; // Since the literal started with 0, only b|o|x are allowed
        }
    }

    if (!anyDigits) {
        // Verify that there actually digits
        ch = fgetc(stream);
        if (!isValidDigit(ch)) {
            fseek(stream, tokenStart, SEEK_SET);
            return 0;
        }
    }

    // Consume all remaining digits
    do {
        ch = fgetc(stream);
    } while (isValidDigit(ch));

    // ch is not a valid digit and should not be alphanumeric either
    if (isalnum(ch)) {
        fseek(stream, tokenStart, SEEK_SET);
        return 0;
    }

    if (ch != EOF) fseek(stream, -1, SEEK_CUR);
    storeToken(stream, tokenStart, token);
    token->kind = Token_INTEGER;
    return 1;
}

// /[_a-zA-Z][_a-zA-Z0-9]*/
int scanToken_ID(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int ch = fgetc(stream);

    // First character must be _ or alphabet character
    if (ch != '_' && !isalpha(ch)) {
        if (ch != EOF) fseek(stream, -1, SEEK_CUR);
        return 0;
    }

    // Consume any following _ or alphanumeric characters
    do {
        ch = fgetc(stream);
    } while (ch == '_' || isalnum(ch));

    if (ch != EOF) fseek(stream, -1, SEEK_CUR);
    storeToken(stream, tokenStart, token);
    token->kind = Token_ID;
    return 1;
}

// /[\(\)\[\]{}:;,\?\$~]|(\.(\.\.)?)|([<>=!+*\/%]=?)|(-[=>]?)|([&|\^][&|\^]?=?)/
int scanToken_SYMBOL(FILE *stream, Token *token) {
    long tokenStart = ftell(stream);
    int ch = fgetc(stream);

    // Compare against single character symbols
    if (
        ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}' ||
        ch == ':' || ch == ';' || ch == ',' || ch == '?' || ch == '$' || ch == '~'
    ) goto symbolMatch;

    if (ch == '.') {
        // Attempt to consume two more '.', return to original location on failure
        long returnPoint = ftell(stream);
        if (fgetc(stream) != '.' || fgetc(stream) != '.') fseek(stream, returnPoint, SEEK_SET);
        goto symbolMatch;
    }

    if (ch == '<' || ch == '>' || ch == '=' || ch == '!' || ch == '+' || ch == '*' || ch == '/' || ch == '%') {
        // Attempt to consume an additional =
        if (fgetc(stream) != '=') fseek(stream, -1, SEEK_CUR);
        goto symbolMatch;
    }

    // Handle special cases -, ->, -=
    if (ch == '-') {
        // Attempt to consume > or =
        ch = fgetc(stream);
        if (ch != '>' && ch != '=') fseek(stream, -1, SEEK_CUR);
        goto symbolMatch;
    }

    if (ch == '&' || ch == '|' || ch == '^') {
        if (fgetc(stream) != ch) fseek(stream, -1, SEEK_CUR); // Try consuming the same character
        if (fgetc(stream) != '=') fseek(stream, -1, SEEK_CUR); // Try consuming an =
        goto symbolMatch;
    }

    // I think I'm justified in using goto here
    noMatch: // Default
        return 0;

    symbolMatch:
        storeToken(stream, tokenStart, token);
        token->kind = Token_SYMBOL;
        return 1;
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
        scanToken_FLOAT(scanner->stream, token) ||
        scanToken_INTEGER(scanner->stream, token) ||
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