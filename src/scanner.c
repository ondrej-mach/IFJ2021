/**
 * @file scanner.c
 *
 * @author Hnatovskyj Vítek xhnato00
 * @author Lán Rostislav xlanro00
 * @author Mach Ondřej xmacho12
 * @author Slivka Matej xslivk03
 *
 * IFJ projekt 2021
 */

#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "scanner.h"

#define ALLOC_SIZE 64

// The file, that is being read
static FILE *inFile;

// Count lines from the start of file and store it in tokens
// this is for easier debugging
static unsigned lineCount;
// characters form the start of the line
static unsigned characterCount;

// if a letter is read after the token has ended, it is kept here
// until the next reading
static int nextLetter;
static Token nextToken;

#define SCAN_END() nextLetter = input; \
    pToken->str[str_index] = '\0'; \
    return SUCCESS


Status scanner_init(FILE *in) {
    inFile = in;
    lineCount = 1;
    characterCount = 0;
    nextLetter = EOF;
    nextToken.type = NONE;
    return SUCCESS;
}

Status scanner_get_token(Token *pToken) {
    unsigned str_index;
    unsigned allocated_length = 0;

    // check if there is token returned with scanner_unget_token()
    if (nextToken.type != NONE) {
        *pToken = nextToken;
        nextToken.type = NONE;
        return SUCCESS;
    }

    // initialize everything
    ScannerState state = SCANNER_START;
    int input;
    pToken->str = NULL;

    // start the reading cycle
    while (true) {
        if (nextLetter != EOF) {
            input = nextLetter;
            nextLetter = EOF;
        } else {
            input = getc(inFile);
            characterCount++;
            if (input == '\n') {
                characterCount = 0;
                lineCount++;
            }
        }

        switch (state) {
            case SCANNER_START:
                str_index = 0;
                pToken->characterNumber = characterCount;
                pToken->lineNumber = lineCount;

                switch (input) {
                    case ':':
                        state = SCANNER_COLON;
                        pToken->type = TOKEN_COLON;
                        break;
                    case '+':
                        state = SCANNER_PLUS;
                        pToken->type = TOKEN_PLUS;
                        break;
                    case '-':
                        state = SCANNER_MINUS;
                        pToken->type = TOKEN_MINUS;
                        break;
                    case '*':
                        state = SCANNER_MULTIPLY;
                        pToken->type = TOKEN_MULTIPLY;
                        break;
                    case '/':
                        state = SCANNER_DIVIDE;
                        pToken->type = TOKEN_DIVIDE;
                        break;
                    case '.':
                        state = SCANNER_DOT;
                        break;
                    case '>':
                        state = SCANNER_GT;
                        pToken->type = TOKEN_GT;
                        break;
                    case '<':
                        state = SCANNER_LT;
                        pToken->type = TOKEN_LT;
                        break;
                    case '~':
                        state = SCANNER_TILDA;
                        break;
                    case '=':
                        state = SCANNER_ONE_EQUALS;
                        break;
                    case '#':
                        state = SCANNER_GET_LENGTH;
                        pToken->type = TOKEN_GET_LENGTH;
                        break;
                    case '\"':
                        state = SCANNER_STRING_START;
                        pToken->type = TOKEN_STRING_LIT;
                        break;
                    case ',':
                        state = SCANNER_COMMA;
                        pToken->type = TOKEN_COMMA;
                        break;
                    case '(':
                        state = SCANNER_PAR_L;
                        pToken->type = TOKEN_PAR_L;
                        break;
                    case ')':
                        state = SCANNER_PAR_R;
                        pToken->type = TOKEN_PAR_R;
                        break;
                    case EOF:
                        pToken->type = TOKEN_EOF;
                        state = SCANNER_EOF;
                        break;

                    default:
                        if (isspace(input)) {
                            continue;
                        } else if (input >= '0' && input <= '9') {
                            state = SCANNER_INT;
                        } else if ((input >= 'A' && input <= 'Z')
                                   || (input >= 'a' && input <= 'z')
                                   || input == '_') {
                            state = SCANNER_IDENTIFIER;
                        } else {
                            return ERR_LEXICAL;
                        }
                }
                break;

            case SCANNER_CONCATENATE:
            case SCANNER_COLON:
            case SCANNER_NEQ:
            case SCANNER_GET_LENGTH:
            case SCANNER_EOF:
            case SCANNER_PAR_L:
            case SCANNER_PAR_R:
            case SCANNER_COMMA:
            SCAN_END();

            case SCANNER_STRING_START:
                if (input == '"') {
                    state = SCANNER_STRING_FINAL;
                } else if (input == '\\') {
                    state = SCANNER_ESC_SEQ;
                } else if (input >= ' ') {
                    state = SCANNER_STRING_VALUE;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_STRING_VALUE:
                if (input == '"') {
                    state = SCANNER_STRING_FINAL;
                } else if (input == '\\') {
                    state = SCANNER_ESC_SEQ;
                } else if (input < ' ') {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_STRING_FINAL:
                pToken->type = TOKEN_STRING_LIT;
                SCAN_END();

            case SCANNER_ESC_SEQ:
                if (input == '0') {
                    state = SCANNER_SEQ_0;
                } else if (input == '1') {
                    state = SCANNER_SEQ_1;
                } else if (input == '2') {
                    state = SCANNER_SEQ_2;
                } else if (input == 't' || input == 'n' || input == '"' || input == '\\') {
                    state = SCANNER_STRING_VALUE;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_SEQ_0:
                if (input == '0') {
                    state = SCANNER_SEQ_00;
                } else if (input > '0' && input <= '9') {
                    state = SCANNER_SEQ_DEF;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_SEQ_1:
                if (input >= '0' && input <= '9') {
                    state = SCANNER_SEQ_DEF;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_SEQ_2:
                if (input == '5') {
                    state = SCANNER_SEQ_25;
                } else if (input >= '0' && input <= '4') {
                    state = SCANNER_SEQ_DEF;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_SEQ_00:
                if (input >= '1' && input <= '9') {
                    state = SCANNER_STRING_VALUE;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_SEQ_DEF:
                if (input >= '0' && input <= '9') {
                    state = SCANNER_STRING_VALUE;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_SEQ_25:
                if (input >= '0' && input <= '5') {
                    state = SCANNER_STRING_VALUE;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_MINUS:
                if (input == '-') {
                    state = SCANNER_COMMENT_START;
                } else {
                    SCAN_END();
                }
                break;

            case SCANNER_PLUS:
                SCAN_END();

            case SCANNER_DIVIDE:
                if (input == '/') {
                    state = SCANNER_INT_DIVIDE;
                    break;
                } else {
                    SCAN_END();
                }

            case SCANNER_INT_DIVIDE:
                pToken->type = TOKEN_INT_DIVIDE;
                SCAN_END();

            case SCANNER_MULTIPLY:
                SCAN_END();

            case SCANNER_DOT:
                if (input == '.') {
                    state = SCANNER_CONCATENATE;
                    pToken->type = TOKEN_CONCATENATE;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_ONE_EQUALS:
                if (input == '=') {
                    state = SCANNER_EQ;
                    pToken->type = TOKEN_EQUALS;

                } else {
                    pToken->type = TOKEN_ASSIGN;
                    SCAN_END();
                }
                break;

            case SCANNER_EQ:
                SCAN_END();

            case SCANNER_LT:
                if (input == '=') {

                    state = SCANNER_LEQ;
                    break;
                } else {
                    SCAN_END();
                }

            case SCANNER_GT:
                if (input == '=') {
                    state = SCANNER_GTE;

                    break;
                } else {
                    SCAN_END();
                }

            case SCANNER_LEQ:
                pToken->type = TOKEN_LEQ;
                SCAN_END();

            case SCANNER_GTE:
                pToken->type = TOKEN_GEQ;
                SCAN_END();

            case SCANNER_TILDA:
                if (input == '=') {
                    pToken->type = TOKEN_NEQ;
                    state = SCANNER_NEQ;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_COMMENT_START:
                if (input == '\n' || input == EOF) {
                    state = SCANNER_START;
                } else if (input == '[') {
                    state = SCANNER_ALMOST_BLOCK;
                } else {
                    state = SCANNER_COMMENT;
                }
                break;

            case SCANNER_ALMOST_BLOCK:
                if (input == '[') {
                    state = SCANNER_COMMENT_BLOCK;
                } else {
                    state = SCANNER_COMMENT;
                }
                break;

            case SCANNER_COMMENT:
                if (input == '\n' || input == EOF) {
                    state = SCANNER_START;
                }
                break;

            case SCANNER_COMMENT_BLOCK:
                if (input == ']') {
                    state = SCANNER_ALMOST_END;
                } else if (input == EOF) {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_ALMOST_END:
                if (input == ']') {
                    state = SCANNER_START;
                } else {
                    state = SCANNER_COMMENT_BLOCK;
                }
                break;

            case SCANNER_INT:
                if (input >= '0' && input <= '9') {

                } else if (input == '.') {
                    state = SCANNER_NUMBER_POINT;
                } else if (input == 'e' || input == 'E') {
                    state = SCANNER_EXP_BASE;
                }else {
                    pToken->type = TOKEN_INTEGER_LIT;
                    SCAN_END();
                }
                break;

            case SCANNER_NUMBER_POINT:
                if (input >= '0' && input <= '9') {
                    state = SCANNER_DOUBLE;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_DOUBLE:
                if (input >= '0' && input <= '9') {
                    
                } else if (input == 'e' || input == 'E') {
                    state = SCANNER_EXP_BASE;
                } else {
                    pToken->type = TOKEN_DOUBLE_LIT;
                    SCAN_END();
                }
                break;

            case SCANNER_EXP_BASE:
                if (input >= '0' && input <= '9') {
                    state = SCANNER_EXP;
                } else if (input == '+' || input == '-') {
                    state = SCANNER_EXP_SIGN;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_EXP_SIGN:
                if (input >= '0' && input <= '9') {
                    state = SCANNER_EXP;
                } else {
                    return ERR_LEXICAL;
                }
                break;

            case SCANNER_EXP:
                if (input >= '0' && input <= '9') {
                    
                } else {
                    pToken->type = TOKEN_DOUBLE_LIT;
                    SCAN_END();
                }
                break;

            case SCANNER_IDENTIFIER:
                if ((input >= 'A' && input <= 'Z') || (input >= 'a' && input <= 'z') || input == '_' ||
                    (input >= '0' && input <= '9')) {
                    // continue reading
                } else {
                    pToken->str[str_index] = '\0';
                    nextLetter = input;
                    if (!strcmp(pToken->str, "do")) {
                        pToken->type = TOKEN_DO;
                    } else if (!strcmp(pToken->str, "else")) {
                        pToken->type = TOKEN_ELSE;
                    } else if (!strcmp(pToken->str, "end")) {
                        pToken->type = TOKEN_END;
                    } else if (!strcmp(pToken->str, "function")) {
                        pToken->type = TOKEN_FUNCTION;
                    } else if (!strcmp(pToken->str, "global")) {
                        pToken->type = TOKEN_GLOBAL;
                    } else if (!strcmp(pToken->str, "if")) {
                        pToken->type = TOKEN_IF;
                    } else if (!strcmp(pToken->str, "integer")) {
                        pToken->type = TOKEN_INTEGER_KW;
                    } else if (!strcmp(pToken->str, "local")) {
                        pToken->type = TOKEN_LOCAL;
                    } else if (!strcmp(pToken->str, "nil")) {
                        pToken->type = TOKEN_NIL;
                    } else if (!strcmp(pToken->str, "number")) {
                        pToken->type = TOKEN_NUMBER_KW;
                    } else if (!strcmp(pToken->str, "require")) {
                        pToken->type = TOKEN_REQUIRE;
                    } else if (!strcmp(pToken->str, "return")) {
                        pToken->type = TOKEN_RETURN;
                    } else if (!strcmp(pToken->str, "string")) {
                        pToken->type = TOKEN_STRING_KW;
                    } else if (!strcmp(pToken->str, "then")) {
                        pToken->type = TOKEN_THEN;
                    } else if (!strcmp(pToken->str, "while")) {
                        pToken->type = TOKEN_WHILE;
                    } else {
                        pToken->type = TOKEN_IDENTIFIER;
                    }
                    return SUCCESS;
                }
                break;

            default:
                return ERR_LEXICAL;
        }
        if (str_index + 2 >= allocated_length) {
            allocated_length += ALLOC_SIZE;
            char *tmp = realloc(pToken->str, allocated_length);
            if (tmp == NULL) {
                return ERR_INTERNAL;
            }
            pToken->str = tmp;
        }
        pToken->str[str_index++] = (char)input;
    }
}

void scanner_unget_token(Token token) {
    nextToken = token;
}

void scanner_destroy_token(Token *pToken) {
    free(pToken->str);
}

Status scanner_destroy() {
    // deallocate all the resources
    fclose(inFile);
    return SUCCESS;
}

