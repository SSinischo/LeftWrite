#ifndef __RECOGNIZER_INCLUDED__
#define __RECOGNIZER_INCLUDED__

#include <stdio.h>
#include <stdlib.h>
#include "lexeme.h"

typedef struct LexList{
	Lexeme *lexx;
	struct LexList *next;
	struct LexList *prev;
} LexList;

extern LexList* buildList(FILE *);
extern Lexeme* recognize(FILE *);
extern void printPreorder(Lexeme *);
extern int explore(Lexeme *);
extern int check(int);
extern Lexeme* match(int);
extern Lexeme* body();
extern Lexeme* statements();
extern Lexeme* statement();
extern Lexeme* include();
extern int statementPending();
extern Lexeme* expression();
extern Lexeme * object();
extern Lexeme * unary();
extern int unaryPending();
extern Lexeme * binary();
extern Lexeme * operator();
extern int operatorPending();
extern Lexeme * definition();
extern Lexeme * assignment();
extern Lexeme * declaration();
extern Lexeme * copySwap();
extern Lexeme * ifStatement();
extern Lexeme * whileLoop();
extern Lexeme * forLoop();
extern Lexeme * list();
extern Lexeme * array();
extern Lexeme * listItem();
#endif