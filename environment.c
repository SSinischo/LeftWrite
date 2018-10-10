#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexeme.h"
#include "environment.h"
#include "evaluator.h"

Lexeme * cloneList(Lexeme *ogList){
	Lexeme * helper(Lexeme *ogItem){
		if(!ogItem->right){
			return cloneLex(ogItem);
		}
		Lexeme *newItem = cloneLex(ogItem);
		newItem->right = helper(ogItem->right);
		return newItem;
	}
	Lexeme *newList = newLexeme(LIST);
	if(ogList->right) newList->right = helper(ogList->right);
	return newList;
}

Lexeme * extendEnv(Lexeme *env, Lexeme *vars, Lexeme *vals){
	//if(vars){
		//printf("exploring vars from extendEnv\n");
		//explore(vars);
	//}
	Lexeme *e = newLexeme(ENVIRONMENT);
	Lexeme *t = newLexeme(TABLE);
	if(!vars){
		t->left = newLexeme(LIST); //vars
		t->right = newLexeme(LIST); //vals
	}
	else{
		t->left = cloneList(vars);
		t->right = vals;
		// THIS FILLS VALS WITH NULL IF NONE ARE SUPPLIED
		//Lexeme *lCur = t->left;
		//Lexeme *rCur = t->right;
		//while(lCur->right){
		//	if(!rCur->right){
		//		Lexeme *nullLex = newLexeme(ITEM);
		//		nullLex->left = newLexeme(LEXEME_NULL);
		//		rCur->right = nullLex;
		//	}
		//	lCur = lCur->right;
		//	rCur = rCur->right;
		//}
	}
	e->left = t;
	e->right = env;
	//printf("Environment extended.  New = %p\n", e);
	//explore(e);
	return e;
}

void insertEnv(Lexeme *env, Lexeme *var, Lexeme *val){
	//printf("Inserting '%s' in env: %p\n", var->strVal, env);
	Lexeme* varItem = newLexeme(ITEM);
	Lexeme* valItem = newLexeme(ITEM);
	varItem->left = var;
	valItem->left = val;
	varItem->right = env->left->left->right;
	valItem->right = env->left->right->right;
	env->left->left->right = varItem;
	env->left->right->right = valItem;
}

int inEnvironment(Lexeme *var, Lexeme *env){
	if(env->type == LEXEME_NULL){
		fprintf(stderr, "%d:[EXISTS ERROR] Attempted to access value '%s' from an undefined environment!\n", var->lineNo, var->strVal);
		exit(3);
	}
	else if(env->type != ENVIRONMENT){
		fprintf(stderr, "%d:[EXISTS ERROR] Attempted to access value '%s' from an object that is not a structure!\n", var->lineNo, var->strVal);
		exit(3);
	}
	//explore(var);
	//explore(env);
	Lexeme *varCursor = env->left->left;
	while(varCursor->right){
		varCursor = varCursor->right;
		if(strcmp(var->strVal, varCursor->left->strVal) == 0) return 1;
	}
	return 0;
}

int varExists(Lexeme *var, Lexeme *env){
	if(env->type == LEXEME_NULL){
		fprintf(stderr, "%d:[EXISTS ERROR] Attempted to access value '%s' from an undefined environment!\n", var->lineNo, var->strVal);
		exit(3);
	}
	else if(env->type != ENVIRONMENT){
			fprintf(stderr, "%d:[EXISTS ERROR] Attempted to access value '%s' from an object that is not a structure!\n", var->lineNo, var->strVal);
		exit(3);
	}
	if(inEnvironment(var, env)) return 1;
	else if(!env->right) return 0;
	else return varExists(var, env->right);
}
	
Lexeme * lookupEnv(Lexeme *var, Lexeme *env){
	//printf("Looking up '%s' in env: %p...\n", var->strVal, env);
	//if(!strcmp(var->strVal, "left"))explore(env);
	if(!env->left && !env->right){
		if(env->type == LEXEME_NULL)
			fprintf(stderr, "%d:[LOOKUP ERROR] Attempted to access value '%s' from an undefined environment!\n", var->lineNo, var->strVal);
		else if(env->type != ENVIRONMENT)
			fprintf(stderr, "%d:[LOOKUP ERROR] Attempted to access value '%s' from an object that is not a structure!\n", var->lineNo, var->strVal);
		else fprintf(stderr, "%d:[LOOKUP ERROR] Variable '%s' not defined!\n", var->lineNo, var->strVal);
		exit(3);
	}
	Lexeme *varCursor = env->left->left;
	Lexeme *valCursor = env->left->right;
	while(varCursor->right){
		varCursor = varCursor->right;
		valCursor = valCursor->right;
		if(strcmp(var->strVal, varCursor->left->strVal) == 0){
			//printf(" found - %p - '%s'\n", valCursor->left, strType(valCursor->left->type));
			return valCursor->left;
		}
	}
	if(!env->right){
		fprintf(stderr, "%d:[LOOKUP ERROR] Variable '%s' not defined!\n", var->lineNo, var->strVal);
		exit(3);
	}
	else{
		return lookupEnv(var, env->right); //zoom out one scope
	}
}
	
void updateEnv(Lexeme *env, Lexeme *var, Lexeme *val){
	//printf("Updating '%s' in env: %p to %p\n", var->strVal, env, val);
	//explore(env);
	if(!env->left && !env->right){
		if(env->type == LEXEME_NULL)
			fprintf(stderr, "%d:[SET ERROR] Attempted to access value '%s' from an undefined environment!\n", var->lineNo, var->strVal);
		else if(env->type != ENVIRONMENT)
			fprintf(stderr, "%d:[SET ERROR] Attempted to access value '%s' from an object that is not a structure!\n", var->lineNo, var->strVal);
		else fprintf(stderr, "%d:[SET ERROR] Variable '%s' not defined!\n", var->lineNo, var->strVal);
		exit(3);
	}
	Lexeme *varCursor = env->left->left;
	Lexeme *valCursor = env->left->right;
	while(varCursor->right){
		varCursor = varCursor->right;
		valCursor = valCursor->right;
		if(strcmp(var->strVal, varCursor->left->strVal) == 0){
			valCursor->left = val;
			return;
		}
	}
	if(!env->right){
		fprintf(stderr, "%d:[SET ERROR] Variable '%s' not defined!\n", var->lineNo, var->strVal);
		exit(3);
	}
	else{
		updateEnv(env->right, var, val);
	}
}

void varCopy(Lexeme *orig, Lexeme *dest, Lexeme *oEnv, Lexeme *dEnv){
	Lexeme *varCursor = oEnv->left->left;
	Lexeme *valCursor = oEnv->left->right;
	Lexeme *oVal;
	Lexeme *dVal;
	//find original value
	while(varCursor->right){
		varCursor = varCursor->right;
		valCursor = valCursor->right;
		if(strcmp(orig->strVal, varCursor->left->strVal) == 0){
			oVal = valCursor->left;
			break;
		}
	}
	//recursively copy lexemes unless it is an environment
	Lexeme *copyHelper(Lexeme *o){
		if(!o) return NULL;
		if(o->type == ENVIRONMENT) return o;
		Lexeme *n = cloneLex(o);
		n->right = copyHelper(o->right);
		n->left = copyHelper(o->left);
		return n;
	}
	//if the original value is an evnrionment, copy must also be an environment with the same enclosing scope
	if(oVal->type == ENVIRONMENT){
		dVal = newLexeme(ENVIRONMENT);
		dVal->right = oVal->right;
		dVal->left = copyHelper(oVal->left);
	}
	else
		dVal = copyHelper(oVal);
	//now dVal should be a copy
	insertEnv(dEnv, dest, dVal);
}

void varSwap(Lexeme *aVar, Lexeme *bVar, Lexeme *aEnv, Lexeme *bEnv){
	Lexeme *aVal = lookupEnv(aVar, aEnv);
	Lexeme *bVal = lookupEnv(bVar, bEnv);
	updateEnv(bEnv, bVar, aVal);
	updateEnv(aEnv, aVar, bVal);
}