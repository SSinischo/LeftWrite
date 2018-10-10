#ifndef __BUILTIN_INCLUDED__
#define __BUILTIN_INCLUDED__

#include <stdio.h>
#include <stdlib.h>

extern Lexeme * printx(Lexeme *, Lexeme *);
extern Lexeme * println(Lexeme *, Lexeme *);
extern Lexeme * invokePrintEnv(Lexeme *, Lexeme *);
extern Lexeme * invokeExplore(Lexeme *, Lexeme *);
extern Lexeme * pptr(Lexeme *, Lexeme *);
extern Lexeme * createDefaultEnv();
extern Lexeme * length_builtin(Lexeme *, Lexeme *);
extern Lexeme * toList(Lexeme *, Lexeme *);
extern Lexeme * toArray_builtin(Lexeme *, Lexeme *);
extern Lexeme * lIns(Lexeme *, Lexeme *);
extern Lexeme * lDel(Lexeme *, Lexeme *);
extern Lexeme * lApp(Lexeme *, Lexeme *);
extern Lexeme * lPrep(Lexeme *, Lexeme *);
extern Lexeme * exists(Lexeme *, Lexeme *);
#endif
