#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexeme.h"
#include "environment.h"
#include "evaluator.h"
#include "builtin.h"

//SHOULD NOT BE CHANGING ANY POINTERS
Lexeme *eval(Lexeme *tree, Lexeme *env){
	if (tree == NULL) return NULL;
	int t = tree->type;
	//printf("%s\n", strType(t));
	if (t == INTEGER || t == STRING || t == ENVIRONMENT || t == LEXEME_NULL) return tree;
	else if (t == THIS) return env;
	else if (t == VARIABLE) return lookupEnv(tree, env);
	else if (t == OBJECT) return extendEnv(NULL, NULL, NULL);
	else if (t == DOT) return evalDot(tree, env);
	else if (t == ARRAY) return evalArray(tree, env);
	else if (t == LIST) return evalList(tree, env);
	else if (t == ITEM) return evalListItem(tree, env);
	
	else if (t == NOT) return evalNot(tree, env);
	else if (t == EXISTS) return evalExists(tree, env);
	else if (t == BINARY) return evalBinary(tree, env);
	else if (t == O_PAREN) return eval(tree->left, env);

	else if (t == PROCEDURE || t == LAMBDA) return evalFn(tree, env);
	else if (t == DEF) return evalDef(tree, env);
	else if (t == SET) return evalSet(tree, env);
	else if (t == COPY) return evalCopy(tree, env);
	else if (t == SWAP) return evalSwap(tree, env);
	else if (t == FN_CALL) return evalFnCall(tree, env);
	
	else if (t == IF) return evalIf(tree, env);
	else if (t == FOR) return evalFor(tree, env);
	else if (t == WHILE) return evalWhile(tree, env);
	else if (t == RETURN || t == DONE) return evalShortCircuit(tree, env);
	else if (t == UMINUS) return evalUMinus(tree, env);
	
	else if (t == STATEMENTS){
		Lexeme *lLex = eval(tree->left, env);
		if (lLex && lLex->shortCircuit){
			//lLex->shortCircuit = 0;
			//printf("SHORTING\n");
			return lLex;
		}
		else return eval(tree->right, env);
	}
	
	else if (t == HALT){
		printf("[HALTED]\n");
		exit(2);
	}
	else return NULL;	
	//else return newLexeme(LEXEME_NULL);
}
Lexeme *evalListItem(Lexeme *tree, Lexeme *env){
	Lexeme *i = newLexeme(ITEM);
	i->left = eval(tree->left, env);
	i->right = eval(tree->right, env);
	return i;
}

Lexeme *evalList(Lexeme *tree, Lexeme *env){
	Lexeme *t = newLexeme(LIST);
	t->right = eval(tree->right, env);
	return t;
}

Lexeme *evalArray(Lexeme *tree, Lexeme *env){
	//explore(tree);
	Lexeme *temp = newLexeme(LIST);
	temp->right = tree->right;
	Lexeme *ultraTemp = newLexeme(LIST);
	ultraTemp->right = newLexeme(ITEM);
	ultraTemp->right->left = eval(temp, env);
	return toArray_builtin(ultraTemp, env);
}

Lexeme *evalIndex(Lexeme *lhs, Lexeme *rhs){
	if(lhs->type != ARRAY){
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible LHS for INDEX operator #: %s", lhs->lineNo, strType(lhs->type));
		exit(2);
	}
	if(rhs->type != INTEGER){
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible RHS for INDEX operator #: %s", rhs->lineNo, strType(lhs->type));
		exit(2);
	}
	return lhs->arr[rhs->intVal];
}
		

Lexeme *evalNot(Lexeme *tree, Lexeme *env){
	Lexeme *result = eval(tree->left, env);
	if(result->type == INTEGER){
		result->intVal = !result->intVal;
	}
	else{
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible type for NOT operator: %s", tree->lineNo, strType(result->type));
		exit(2);
	}
	return result;
}

Lexeme *evalExists(Lexeme *tree, Lexeme *env){
	Lexeme *variable = tree->left;
	if(variable->type == DOT){
		Lexeme *dotEnv = eval(variable->left, env);
		Lexeme *tempTree = newLexeme(EXISTS);
		tempTree->left = variable->right;
		return evalExists(tempTree, dotEnv);
	}
	Lexeme *result = newLexeme(INTEGER);
	if(varExists(variable, env)){
		Lexeme *n = eval(variable, env);
		if(n->type == LEXEME_NULL)
			return result; //0
		result->intVal = 1;
	}
	return result;
	
}
	
Lexeme *evalDef(Lexeme *tree, Lexeme *env){
	//todo: switch order of error checking and dot checking (without fucking it up)
	//explore(env);
	Lexeme *variable = tree->left;
	if (inEnvironment(tree->left, env)){
		fprintf(stderr, "%d:[DEFINE ERROR] Variable %s is already defined!\n", tree->left->lineNo, tree->left->strVal);
		//explore(env);
		exit(2);
	}
	Lexeme *value = eval(tree->right, env);
	if(variable->type == DOT){
		Lexeme *dotEnv = eval(variable->left, env);
		//explore(dotEnv);
		Lexeme *tempTree = newLexeme(DEF);
		tempTree->left = variable->right;
		tempTree->right = value;
		return evalDef(tempTree, dotEnv);
	}
	//printf("Inserting..."); displayLex(variable);
	//printf("\nExploring environment BEFORE insertion...");
	//explore(env);
	insertEnv(env, variable, value);
	//printf("\nExploring environment AFTER insertion...");
	//explore(env);
	return value;
}

Lexeme *evalSet(Lexeme *tree, Lexeme *env){
	Lexeme *variable = tree->left;
	Lexeme *value = eval(tree->right, env);
	if (variable->type == DOT){
		Lexeme *dotEnv = eval(variable->left, env);
		Lexeme *tempTree = newLexeme(SET);
		tempTree->right = value;
		tempTree->left = variable->right;
		return eval(tempTree, dotEnv);
	}
	updateEnv(env, variable, value);
	return value;
}

Lexeme *evalCopy(Lexeme *tree, Lexeme *env){
	void copyHelper(Lexeme *orig, Lexeme *dest, Lexeme *origEnv, Lexeme *destEnv){
		if(orig->type == DOT)
			copyHelper(orig->right, dest, eval(orig->left, origEnv), destEnv);
		else if(dest->type == DOT)
			copyHelper(orig, dest->right, orig, eval(dest->left, destEnv));
		else varCopy(orig, dest, origEnv, destEnv);
	}
	copyHelper(tree->left, tree->right, env, env);
	return newLexeme(LEXEME_NULL);
}

Lexeme *evalSwap(Lexeme *tree, Lexeme *env){
	void swapHelper(Lexeme *orig, Lexeme *dest, Lexeme *origEnv, Lexeme *destEnv){
		if(orig->type == DOT)
			swapHelper(orig->right, dest, eval(orig->left, origEnv), destEnv);
		else if(dest->type == DOT)
			swapHelper(orig, dest->right, origEnv, eval(dest->left, destEnv));
		else
			varSwap(orig, dest, origEnv, destEnv);
	}
	swapHelper(tree->left, tree->right, env, env);
	return newLexeme(LEXEME_NULL);
}

Lexeme *evalFn(Lexeme *tree, Lexeme *env){
	Lexeme *closure = newLexeme(CLOSURE);
	closure->left = env;
	closure->right = tree; //unchanged procedure
	return closure;
}

Lexeme *evalFnCall(Lexeme *tree, Lexeme *env){
	//printf("Extending from function call: %s!\n", tree->left->strVal);
	Lexeme *closure = eval(tree->left, env); //variable name will be looked up and closure is returned
	//explore(tree);
	Lexeme *args = eval(tree->right, env); //evaluates args list
	if (closure->type == BUILTIN){
		Lexeme* (*f)(Lexeme *, Lexeme *) = closure->fVal;
		return f(args, env);
	}
	Lexeme *fnParams = closure->right->left;
	Lexeme *fnBody = closure->right->right;
	Lexeme *fnEnv = closure->left;

	Lexeme *lenP = newLexeme(LIST);
	lenP->right = newLexeme(ITEM);
	lenP->right->left = fnParams;
	Lexeme *lenPi = length_builtin(lenP, NULL);
	Lexeme *lenA = newLexeme(LIST);
	lenA->right = newLexeme(ITEM);
	lenA->right->left = args;
	Lexeme *lenAi = length_builtin(lenA, NULL);
	
	//make sure arguments match up and eval them
	int lengthDif = lenPi->intVal - lenAi->intVal;
	if(lengthDif < 0){
		fprintf(stderr, "%d:[ARGS ERROR] Too many arguments supplied for function %s!\n", tree->right->lineNo, tree->left->strVal);
		exit(2);
	}if(lengthDif > 0){
		fprintf(stderr, "%d:[ARGS ERROR] Too few arguments supplied for function %s!\n", tree->right->lineNo, tree->left->strVal);
		exit(2);
	}
	Lexeme *xenv = extendEnv(fnEnv, fnParams, args);
	//explore(xenv); this is fine but fn changed
	explore(fnBody);
	Lexeme *b =  eval(fnBody, xenv);
	b->shortCircuit = 0;
	return b;
}

Lexeme *evalBinary(Lexeme *tree, Lexeme *env){
	int precedence(int op){
		if(op == PLUS || op == MINUS) return 1;
		if(op == MULTIPLY || op == DIVIDE) return 2;
		if(op == EXPONENT || op == MODULO) return 3;
		return 0;
	}
	//left associative!
	Lexeme *result;
	int op = tree->left->right->type;
	Lexeme *lhs = eval(tree->left->left, env);
	Lexeme *rhs;

	//now if op is short-circuiting, we need to send unevaled rhs to op function
	if (op == AND) return evalAnd(lhs, tree->right, env);
	else if (op == OR) return evalOr(lhs, tree->right, env);
	if(tree->right->type == BINARY){
		int lOpPrec = precedence(op);
		int rOpPrec = precedence(tree->right->left->right->type);
		//printf("lOp is %d, rOp is %d", lOpPrec, rOpPrec);
		if (lOpPrec >= rOpPrec){
			Lexeme *tempTree = newLexeme(BINARY);
			tempTree->left = tree->left;
			tempTree->right = tree->right->left->left;
			tree = tree->right;
			tree->left->left = eval(tempTree, env);
			return eval(tree, env);
		}
		else rhs = eval(tree->right, env);
	}
	else rhs = eval(tree->right, env);

	if (op == EQUALS) result = evalEquals(lhs, rhs);
	else if (op == NOT_EQUALS) result = evalNotEquals(lhs, rhs);
	else if (op == PLUS) result = evalPlus(lhs, rhs);
	else if (op == MINUS) result = evalMinus(lhs, rhs);
	else if (op == MULTIPLY) result = evalMultiply(lhs, rhs);
	else if (op == DIVIDE) result = evalDivide(lhs, rhs);
	else if (op == LESS_THAN) result =  evalLessThan(lhs, rhs);
	else if (op == GREATER_THAN) result = evalGreaterThan(lhs, rhs);
	else if (op == LESS_EQ_THAN) result =  evalLessEqThan(lhs, rhs);
	else if (op == GREATER_EQ_THAN) result = evalGreaterEqThan(lhs, rhs);
	else if (op == EXPONENT) result = evalExponent(lhs, rhs);
	else if (op == MODULO) result = evalModulo(lhs, rhs);
	else if (op == INDEX) result = evalIndex(lhs, rhs);
	return result;
}

Lexeme *evalWhile(Lexeme *tree, Lexeme *env){
	Lexeme *condition = eval(tree->left, env);
	if (condition->type != INTEGER){
		fprintf(stderr, "[TYPE ERROR] Incompatible condition type for WHILE statement : %s", strType(condition->type));
		exit(2);
	}
	if (condition->intVal){
		Lexeme *result = eval(tree->right, env);
		if (result->shortCircuit) return result;
		else return evalWhile(tree, env);
	}
	else return newLexeme(LEXEME_NULL);
}	

Lexeme *evalFor(Lexeme *tree, Lexeme *env){
	Lexeme *forVar = tree->left->left->left;
	if(!inEnvironment(forVar, env)){
		Lexeme *forVarList = newLexeme(LIST);
		Lexeme *forValList = newLexeme(LIST);
		forVarList->right = newLexeme(ITEM);
		forValList->right = newLexeme(ITEM);
		forVarList->right->left = tree->left->left->left;
		forValList->right->left = eval(tree->left->left->right, env);
		return evalFor(tree, extendEnv(env, forVarList, forValList));
	}
	Lexeme *condition = eval(tree->left->right->left, env);
	if (condition->type != INTEGER){
		fprintf(stderr, "[TYPE ERROR] Incompatible condition type for FOR statement : %s", strType(condition->type));
		exit(2);
	}
	if (condition->intVal){
		Lexeme *result = eval(tree->right, env);
		if (result->shortCircuit) return result;
		else{
			updateEnv(env, forVar, eval(tree->left->right->right, env)); //increment
			return evalFor(tree, env);
		}
	}
	else return newLexeme(LEXEME_NULL);
}

Lexeme *evalShortCircuit(Lexeme *tree, Lexeme *env){
	Lexeme *result;
	if(tree->left) result = eval(tree->left, env);
	else result = newLexeme(LEXEME_NULL);
	result->shortCircuit = 1;
	return result;
}

Lexeme * evalAnd(Lexeme *lhs, Lexeme *rhsUneval, Lexeme *env){
	if(lhs->type == INTEGER){
		if(lhs->intVal) return eval(rhsUneval, env);
		else return lhs; //false
	}
	else{//TODO CHANGE TYPE ERRORS
		fprintf(stderr, "[TYPE ERROR] Incompatible LHS for AND : "); displayLex(lhs);
		exit(2);
	}
}

Lexeme * evalOr(Lexeme *lhs, Lexeme *rhsUneval, Lexeme *env){
	if(lhs->type == INTEGER){
		if(lhs->intVal) return lhs; //true
		else return eval(rhsUneval, env);
	}
	else{
		fprintf(stderr, "[TYPE ERROR] Incompatible LHS for OR : "); displayLex(lhs);
		exit(2);
	}
}

Lexeme * evalEquals(Lexeme *lhs, Lexeme *rhs){
	Lexeme *result = newLexeme(INTEGER);
	if(lhs->type == STRING && rhs->type == STRING){
		if(!strcmp(lhs->strVal, rhs->strVal)) result->intVal = 1;
		else result->intVal = 0;
	}
	else if ((lhs->type == INTEGER) && (rhs->type == INTEGER))
		if (lhs->intVal == rhs->intVal)
			result->intVal = 1;
	if(lhs == rhs){
		result->intVal = 1;
	}
	return result;
}

Lexeme * evalNotEquals(Lexeme *lhs, Lexeme *rhs){
	Lexeme *result = newLexeme(INTEGER);
	result->intVal = 1;
	if(lhs->type == STRING && rhs->type == STRING){
		if(!strcmp(lhs->strVal, rhs->strVal)) result->intVal = 0;
	}
	else if ((lhs->type == INTEGER) && (rhs->type == INTEGER)){
		if (lhs->intVal == rhs->intVal) result->intVal = 0;
	}
	if(lhs == rhs){
		result->intVal = 0;
	}
	return result;
}

Lexeme * evalPlus(Lexeme *lhs, Lexeme *rhs){
	int isNumber(char *s){
		int i=0;
		while(s[i] != '\0'){
			if(!isdigit(s[i])) return 0;
			i++;
		}
		return 1;
	}
	Lexeme *sum = newLexeme(INTEGER);
	if(lhs->type == STRING){
		if(isNumber(lhs->strVal)){
			Lexeme *temp = newLexeme(INTEGER);
			temp->intVal = atoi(lhs->strVal);
			lhs = temp;
		}
	}
	if(rhs->type == STRING){
		if(isNumber(rhs->strVal)){
			Lexeme *temp = newLexeme(INTEGER);
			temp->intVal = atoi(rhs->strVal);
			rhs = temp;
		}
	}
	
	if((lhs->type == INTEGER) && (rhs->type == INTEGER))
		sum->intVal = lhs->intVal+rhs->intVal;
	else if((lhs->type == INTEGER) && (rhs->type == STRING)){
		sum->type = STRING;
		char tempC[12];
		sprintf(tempC, "%d", lhs->intVal);
		strcpy(sum->strVal, strcat(tempC, rhs->strVal));
	}
	else if((lhs->type == STRING) && (rhs->type == STRING)){
		sum->type = STRING;
		strcpy(sum->strVal, strcat(lhs->strVal, rhs->strVal));
	}
	else if((lhs->type == STRING) && (rhs->type == INTEGER)){
		sum->type = STRING;
		char tempC[12];
		sprintf(tempC, "%d", rhs->intVal);
		strcpy(sum->strVal, strcat(lhs->strVal, tempC));
	}
	else{
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible types for function +: %s and %s\n", lhs->lineNo, strType(lhs->type), strType(rhs->type));
		exit(2);
	}	
	return sum;
}

Lexeme *evalMinus(Lexeme *lhs, Lexeme *rhs){
	//todo: add string minus?
	Lexeme *difference = newLexeme(INTEGER);
	if((lhs->type == INTEGER) && (rhs->type == INTEGER))
		difference->intVal = lhs->intVal-rhs->intVal;
	else{
		exit(2);
	}
	return difference;
}

Lexeme *evalMultiply(Lexeme *lhs, Lexeme *rhs){
	Lexeme *product = newLexeme(INTEGER);
	if((lhs->type == INTEGER) && (rhs->type == INTEGER))
		product->intVal = lhs->intVal*rhs->intVal;
	else{
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible types for function *: %s and %s\n", lhs->lineNo, strType(lhs->type), strType(rhs->type));
		exit(2);
	}
	return product;
}

Lexeme *evalDivide(Lexeme *lhs, Lexeme *rhs){
	Lexeme *quotient = newLexeme(INTEGER);
	if(lhs->type == INTEGER && rhs->type == INTEGER){
		if(rhs->intVal == 0){
			fprintf(stderr, "[MATH ERROR] Divide by zero!\n");
			exit(2);
		}
		quotient->intVal = lhs->intVal/rhs->intVal;
	}
	else{
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible types for function /: %s and %s\n", lhs->lineNo, strType(lhs->type), strType(rhs->type));
		exit(2);
	}
	return quotient;
}

Lexeme *evalLessThan(Lexeme *lhs, Lexeme *rhs){
	Lexeme *result = newLexeme(INTEGER);
	if(lhs->type == INTEGER && rhs->type == INTEGER)
		result->intVal = lhs->intVal<rhs->intVal;
	else{
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible types for function <: %s and %s\n", lhs->lineNo, strType(lhs->type), strType(rhs->type));
		exit(2);
	}
	return result;
}

Lexeme *evalGreaterThan(Lexeme *lhs, Lexeme *rhs){
	Lexeme *result = newLexeme(INTEGER);
	if(lhs->type == INTEGER && rhs->type == INTEGER)
		result->intVal = lhs->intVal>rhs->intVal;
	else{
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible types for function >: %s and %s\n", rhs->lineNo, strType(lhs->type), strType(rhs->type));
		exit(2);
	}
	return result;
}

Lexeme *evalLessEqThan(Lexeme *lhs, Lexeme *rhs){
	Lexeme *result = newLexeme(INTEGER);
	if(lhs->type == INTEGER && rhs->type == INTEGER)
		result->intVal = lhs->intVal<=rhs->intVal;
	else{
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible types for function <=: %s and %s\n", lhs->lineNo, strType(lhs->type), strType(rhs->type));
		exit(2);
	}
	return result;
}

Lexeme *evalGreaterEqThan(Lexeme *lhs, Lexeme *rhs){
	Lexeme *result = newLexeme(INTEGER);
	if(lhs->type == INTEGER && rhs->type == INTEGER)
		result->intVal = lhs->intVal>=rhs->intVal;
	else{
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible types for function >=: %s and %s\n", lhs->lineNo, strType(lhs->type), strType(rhs->type));
		exit(2);
	}
	return result;
}

Lexeme *evalExponent(Lexeme *lhs, Lexeme *rhs){
	int power(int base, int exp) {
		if (exp == 0)
			return 1;
		else if (exp % 2)
			return base * power(base, exp - 1);
		else {
			int temp = power(base, exp / 2);
			return temp * temp;
		}
	}
	Lexeme *result = newLexeme(INTEGER);
	if(lhs->type == INTEGER && rhs->type == INTEGER)
		result->intVal = power(lhs->intVal, rhs->intVal);
	else{
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible types for function >: %s and %s\n", lhs->lineNo, strType(lhs->type), strType(rhs->type));
		exit(2);
	}
	return result;
}

Lexeme *evalModulo(Lexeme *lhs, Lexeme *rhs){
	Lexeme *result = newLexeme(INTEGER);
	if(lhs->type == INTEGER && rhs->type == INTEGER)
		result->intVal = lhs->intVal % rhs->intVal;
	else{
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible types for function %%: %s and %s\n", lhs->lineNo, strType(lhs->type), strType(rhs->type));
		exit(2);
	}
	return result;
}

Lexeme *evalUMinus(Lexeme *tree, Lexeme *env){
	Lexeme *l = eval(tree->left, env);
	if(l->type != INTEGER){
		fprintf(stderr, "%d:[TYPE ERROR] Incompatible type for UNARY MINUS: %s\n", tree->lineNo, strType(l->type));
		exit(2);
	}
	l->intVal = 0-l->intVal;
	return l;
}

Lexeme *evalIf(Lexeme *tree, Lexeme* env){
	Lexeme *ifResult = eval(tree->left->left, env);
	if(ifResult->type != INTEGER){
		return eval(tree->left->right, env);
	}
	else{
		if(ifResult->intVal){ //true
			return eval(tree->left->right, env);
		}
		else if(tree->right){ //if else
			return eval(tree->right, env);
		}
		else{
			return newLexeme(LEXEME_NULL);
		}
	}
}

Lexeme *evalDot(Lexeme *tree, Lexeme *env){
	Lexeme *dotEnv = eval(tree->left, env);
	return eval(tree->right, dotEnv);
}