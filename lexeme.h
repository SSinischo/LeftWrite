#ifndef __LEXEME_INCLUDED__
#define __LEXEME_INCLUDED__

#include <stdio.h>
#include <stdlib.h>

enum lexType{
	UNKNOWN, INTEGER, VARIABLE, STRING, LEXEME_NULL,
	DEF, SET, DECL, RETURN, DONE, PROCEDURE, FN_CALL, LAMBDA, CLOSURE, BUILTIN,
	IF, ELSE, FOR, WHILE, AND, OR, NOT, EXISTS, COPY, SWAP,
	EQUALS, NOT_EQUALS, PLUS, MINUS, MULTIPLY, DIVIDE, LESS_THAN, GREATER_THAN, LESS_EQ_THAN, GREATER_EQ_THAN, EXPONENT, MODULO, UMINUS,
	O_BRACE, C_BRACE, O_PAREN, C_PAREN, O_BRACKET, C_BRACKET, COMMA, DOT, LIST, ARRAY, INDEX, 
	STATEMENTS, ITEM, TABLE, ENVIRONMENT, THIS, OBJECT,
	BINARY, GLUE, HALT, INCLUDE, ENDofINPUT
};

typedef struct Lexeme{
	int type;
	char strVal[512];
	int intVal;
	int shortCircuit;
	struct Lexeme* (*fVal)(struct Lexeme *, struct Lexeme *);
	struct Lexeme *left;
	struct Lexeme *right;
	struct Lexeme **arr;
	int arrSize;
	int lineNo;
} Lexeme;

extern char *strType(int);
extern void displayLex(Lexeme *);
extern void printPreorder(Lexeme *);
extern int explore(Lexeme *);
extern Lexeme *newLexeme(int);
extern Lexeme *cloneLex(Lexeme *);
extern void initLexer(FILE *);
extern void pushback(char);
extern char myRead();
extern void skipWhiteSpace();
extern void skipToScisors();
extern Lexeme *lex();
extern Lexeme *lexNumber();
extern Lexeme *lexString();
extern Lexeme *lexVariableOrKeyword();
#endif