/**
 * @file scanner.h
 *
 * @author Hnatovskyj Vítek xhnato00
 * @author Lán Rostislav xlanro00
 * @author Mach Ondřej xmacho12
 * @author Slivka Matej xslivk03
 *
 * IFJ projekt 2021
 */


#ifndef SCANNER_H
#define SCANNER_H

#include "common.h"
#include <stdio.h>

typedef enum {
    TOKEN_IDENTIFIER,
    // TODO add more types
} TokenType;

typedef struct {
    char *str;
    TokenType type;
} Token;

#endif // SCANNER_H
