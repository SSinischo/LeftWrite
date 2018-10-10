#ifndef __EVALUATOR_INCLUDED__
#define __EVALUATOR_INCLUDED__

#include <stdio.h>
#include <stdlib.h>

extern Lexeme * eval(Lexeme *, Lexeme *);
extern Lexeme * evalList(Lexeme *, Lexeme *);
extern Lexeme * evalListItem(Lexeme *, Lexeme *);
extern Lexeme * evalArray(Lexeme *, Lexeme *);
extern Lexeme * evalIndex(Lexeme *, Lexeme *);
extern Lexeme * evalDef(Lexeme *, Lexeme *);
extern Lexeme * evalSet(Lexeme *, Lexeme *);
extern Lexeme * evalCopy(Lexeme *, Lexeme *);
extern Lexeme * evalSwap(Lexeme *, Lexeme *);
extern Lexeme * evalFn(Lexeme *, Lexeme *);
extern Lexeme * evalFnCall(Lexeme *, Lexeme *);
extern Lexeme * evalNot(Lexeme *, Lexeme *);
extern Lexeme * evalExists(Lexeme *, Lexeme *);
extern Lexeme * evalBinary(Lexeme *, Lexeme *);
extern Lexeme * evalWhile(Lexeme *, Lexeme *);
extern Lexeme * evalFor(Lexeme *, Lexeme *);
extern Lexeme * evalShortCircuit(Lexeme *, Lexeme *);
extern Lexeme * evalAnd(Lexeme *, Lexeme *, Lexeme *);
extern Lexeme * evalOr(Lexeme *, Lexeme *, Lexeme *);
extern Lexeme * evalEquals(Lexeme *, Lexeme *);
extern Lexeme * evalNotEquals(Lexeme *, Lexeme *);
extern Lexeme * evalPlus(Lexeme *, Lexeme *);
extern Lexeme * evalMinus(Lexeme *, Lexeme *);
extern Lexeme * evalMultiply(Lexeme *, Lexeme *);
extern Lexeme * evalDivide(Lexeme *, Lexeme *);
extern Lexeme * evalLessThan(Lexeme *, Lexeme *);
extern Lexeme * evalGreaterThan(Lexeme *, Lexeme *);
extern Lexeme * evalLessEqThan(Lexeme *, Lexeme *);
extern Lexeme * evalGreaterEqThan(Lexeme *, Lexeme *);
extern Lexeme * evalExponent(Lexeme *, Lexeme *);
extern Lexeme * evalModulo(Lexeme *, Lexeme *);
extern Lexeme * evalUMinus(Lexeme *, Lexeme *);
extern Lexeme * evalIf(Lexeme *, Lexeme *);
extern Lexeme * evalDot(Lexeme *, Lexeme *);

#endif