#ifndef __ENVIRONMENT_INCLUDED__
#define __ENVIRONMENT_INCLUDED__

#include <stdio.h>
#include <stdlib.h>

extern void printEnv(Lexeme *);
extern Lexeme *extendEnv(Lexeme *, Lexeme *, Lexeme *);
extern void insertEnv(Lexeme *, Lexeme *, Lexeme *);
extern int inEnvironment(Lexeme *, Lexeme *);
extern int varExists(Lexeme *, Lexeme *);
extern Lexeme *lookupEnv(Lexeme *, Lexeme *);
extern void updateEnv(Lexeme *, Lexeme *, Lexeme *);
extern void varCopy(Lexeme *, Lexeme *, Lexeme *, Lexeme *);
extern void varSwap(Lexeme *, Lexeme *, Lexeme *, Lexeme *);

#endif