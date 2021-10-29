/**
 * @file syntaxstack.c
 *
 * @author Hnatovskyj Vítek xhnato00
 * @author Lán Rostislav xlanro00
 * @author Mach Ondřej xmacho12
 * @author Slivka Matej xslivk03
 *
 * IFJ projekt 2021
 */

#include "syntaxstack.h"

void syntaxstack_init(SyntaxStack* s) {
    s->data = NULL;
    s->top = -1;
    s->allocatedElements = 0;
}

Status syntaxstack_push(SyntaxStack* s, TokenType item) {
    // reallocate if not enough space
    if ((s->top + 1) >= s->allocatedElements) {
        size_t numElements = s->allocatedElements + ALLOCATION_CHUNK;

        TokenType *newPtr = realloc(s->data, newSize * sizeof(TokenType));
        if (newPtr == NULL) {
            return ERR_INTERNAL;
        }
        s->allocatedElements = numElements;
        s->data = newPtr;
    }

    s->top++;
    s->data[s->top] = item;
}

TokenType syntaxstack_top(SyntaxStack* s) {
    if (!syntaxstack_isempty(s)) {
        return s->data[s->top];
    }
    return TOKEN_ERROR; // TODO very poor solution, but this should never happen anyway
}

TokenType syntaxstack_pop(SyntaxStack* s) {
    if (!syntaxstack_isempty(s)) {
        return s->data[s->top--];
    }
    return TOKEN_ERROR; // TODO very poor solution, but this should never happen anyway
}

bool syntaxstack_isempty(SyntaxStack* s) {
    return s->top == -1;
}

void syntaxstack_destroy(SyntaxStack* s) {
    free(s->data);
    s->data = NULL;
    s->top = -1;
    s->allocatedElements = 0;
}