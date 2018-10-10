#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexeme.h"
#include "environment.h"
#include "evaluator.h"
#include "recognizer.h"

Lexeme * printx(Lexeme *cursor, Lexeme *env){
	(void)env;
	//explore(cursor);
	if (!cursor->left && cursor->right && cursor->right->type == ITEM) return printx(cursor->right, env);
	if (cursor->left->type == LIST){
		printf("{");
		Lexeme *lcursor = cursor->left->right;
		while(lcursor){
			if(lcursor->left->type == STRING){
				printf("\"");
				Lexeme *temp = newLexeme(ITEM);
				temp->left = cloneLex(lcursor->left);
				temp->left->right = NULL;
				printx(temp, env);
				printf("\"");
			}
			else{
				Lexeme *temp = newLexeme(ITEM);
				temp->left = cloneLex(lcursor->left);
				temp->left->right = NULL;
				printx(temp, env);
			}
			if(lcursor->right) printf(" ");
			lcursor = lcursor->right;
		}
		printf("}");
	}
	else{
		if (cursor->left->type == ARRAY){
			printf("[");
			Lexeme **arr = cursor->left->arr;
			Lexeme *temp;
			for(int i=0; i<cursor->left->arrSize; i++){
				temp = newLexeme(ITEM);
				temp->left = arr[i];
				printx(temp, env);
				if(i<cursor->left->arrSize-1)printf(" ");
			}
			printf("]");
		}
		
		else if (cursor->left->type == STRING) printf("%s", cursor->left->strVal);
		else if (cursor->left->type == INTEGER) printf("%d", cursor->left->intVal);
		else if (cursor->left->type == LEXEME_NULL) printf("<null>");
		else{
			printf("%d:[PRINT ERROR] Tried to display lexeme with type ", cursor->left->lineNo);
			displayLex(cursor->left);
			printf("\n");
			exit(2);
		}
		if(cursor->right){
			printx(cursor->right, env);
		}
	}
	return newLexeme(LEXEME_NULL);
}

Lexeme * println(Lexeme *cursor, Lexeme *env){
	(void)env;
	printx(cursor, env);
	printf("\n");
	return newLexeme(LEXEME_NULL);
}

Lexeme * invokePrintEnv(Lexeme *args, Lexeme *env){
	(void)args;
	(void)env;
	//TODO
	//printEnv(env);
	return newLexeme(LEXEME_NULL);
}

Lexeme * pptr(Lexeme *args, Lexeme *env){
	(void)env;
	printf("%s - %p\n", strType(args->right->left->type), args->right->left);
	return newLexeme(LEXEME_NULL);
}

Lexeme * invokeExplore(Lexeme *args, Lexeme *env){
	(void)env;
	explore(args->right->left);
	return newLexeme(LEXEME_NULL);
}

Lexeme * length_builtin(Lexeme *args, Lexeme *env){
	(void)env;
	if(args->right && args->right->right){
		fprintf(stderr, "%d:[ARGS ERROR] More than one argument supplied for function LENGTH!\n", args->right->lineNo);
		exit(2);
	}
	if(!args->right)
		return newLexeme(INTEGER); //0
	Lexeme *l = args->right->left;
	int c = 0;
	if(l->type == ARRAY){
		c = l->arrSize;
	}
	else{
		while(l->right){
			l = l->right;
			c = c+1;
		}
	}
	Lexeme *i = newLexeme(INTEGER);
	i->intVal = c;
	return i;
}

Lexeme * toArray_builtin(Lexeme *args, Lexeme *env){
	if(args->right->right){
		fprintf(stderr, "%d:[ARGS ERROR] More than one argument supplied for function toArray_builtin!\n", args->right->lineNo);
		exit(2);
	}
	Lexeme *eArr = newLexeme(ARRAY);
	Lexeme *listCur = eval(args->right->left, env);
	Lexeme *iLex = length_builtin(args, env);
	int iSize = iLex->intVal;
	eArr->arr = malloc(sizeof(Lexeme)*iSize);
	listCur = listCur->right;
	int i=0;
	while(listCur){
		eArr->arr[i] = listCur->left;
		listCur = listCur->right;
		i++;
	}
	eArr->arrSize = iSize;
	return eArr;
}

Lexeme *toList(Lexeme *args, Lexeme *env){
	(void) env;
	if(args->right->right || args->right->left->type != ARRAY){
		fprintf(stderr, "%d:[ARGS ERROR] Incorrect arguments supplied for function TOLIST!\n", args->right->lineNo);
		exit(2);
	}
	Lexeme *nList = newLexeme(LIST);
	Lexeme *lCur = nList;
	for(int i=0; i<args->right->left->arrSize; i++){
		Lexeme *newItem = newLexeme(ITEM);
		newItem->left = args->right->left->arr[i];
		lCur->right = newItem;
		lCur = newItem;
	}
	return nList;
}

Lexeme *listGet(Lexeme *args, Lexeme *env){
	(void) env;
	if(args->right->right->right || args->right->left->type != LIST || args->right->right->left->type != INTEGER){
		fprintf(stderr, "%d:[ARGS ERROR] Incorrect arguments supplied for function LGET!\n", args->right->lineNo);
		exit(2);
	}
	Lexeme *cur = args->right->left->right;
	int index = args->right->right->left->intVal;
	for(int i=0; i<index; i++){
		if(!cur->right){
			fprintf(stderr, "%d:[INDEX ERROR] List out of index!\n", args->right->lineNo);
			exit(2);
		}
		cur = cur->right;
	}
	return cur->left;
}

Lexeme *listPrepend(Lexeme *args, Lexeme *env){
	(void) env;
	if(args->right->right->right || args->right->left->type != LIST){
		fprintf(stderr, "%d:[ARGS ERROR] Incorrect arguments supplied for function LPREPEND!\n", args->right->lineNo);
		exit(2);
	}
	Lexeme *obj = args->right->right->left;
	Lexeme *nItem = newLexeme(ITEM);
	nItem->left = obj;
	nItem->right = args->right->left->right;
	args->right->left->right = nItem;
	return newLexeme(LEXEME_NULL);
}

Lexeme *listAppend(Lexeme *args, Lexeme *env){
	(void) env;
	if(args->right->right->right || args->right->left->type != LIST){
		fprintf(stderr, "%d:[ARGS ERROR] Incorrect arguments supplied for function LAPPEND!\n", args->right->lineNo);
		exit(2);
	}
	Lexeme *obj = args->right->right->left;
	Lexeme *nItem = newLexeme(ITEM);
	nItem->left = obj;
	Lexeme *cur = args->right->left;
	while(cur->right)
		cur = cur->right;
	cur->right = nItem;
	return newLexeme(LEXEME_NULL);
}

Lexeme *listInsert(Lexeme *args, Lexeme *env){
	(void) env;
	if(args->right->right->right->right || args->right->left->type != LIST || args->right->right->left->type != INTEGER){
		fprintf(stderr, "%d:[ARGS ERROR] Incorrect arguments supplied for function LINS!\n", args->right->lineNo);
		exit(2);
	}
	Lexeme *cur = args->right->left;
	int index = args->right->right->left->intVal;
	Lexeme *obj = args->right->right->right->left;
	for(int i=0; i<index; i++){
		if(!cur->right){
			fprintf(stderr, "%d:[INDEX ERROR] List out of index!\n", args->right->lineNo);
			exit(2);
		}
		cur = cur->right;
	}
	Lexeme *nItem = newLexeme(ITEM);
	nItem->left = obj;
	nItem->right = cur->right;
	cur->right = nItem;
	return newLexeme(LEXEME_NULL);
}

Lexeme *listDelete(Lexeme *args, Lexeme *env){
	(void) env;
	if(args->right->right->right || args->right->left->type != LIST || args->right->right->left->type != INTEGER){
		fprintf(stderr, "%d:[ARGS ERROR] Incorrect arguments supplied for function LDEL!\n", args->right->lineNo);
		exit(2);
	}
	Lexeme *cur = args->right->left;
	int index = args->right->right->left->intVal;
	for(int i=0; i<index; i++){
		if(!cur->right){
			fprintf(stderr, "%d:[INDEX ERROR] List out of index!\n", args->right->lineNo);
			exit(2);
		}
		cur = cur->right;
	}
	Lexeme *r = cur->right;
	cur->right = cur->right->right;
	return r;
}

Lexeme * exists(Lexeme *args, Lexeme *env){
	(void)env;
	if (!args->right->left) return newLexeme(INTEGER);
	Lexeme *r = newLexeme(INTEGER);
	r->intVal = 1;
	return r;
}

Lexeme * createDefaultEnv(){
	Lexeme *globalEnv = extendEnv(NULL, NULL, NULL);
	
	Lexeme *printVar = newLexeme(VARIABLE);
	strcpy(printVar->strVal, "pt");
	Lexeme *printVal = newLexeme(BUILTIN);
	printVal->fVal = printx;
	insertEnv(globalEnv, printVar, printVal);
	
	Lexeme *printLnVar = newLexeme(VARIABLE);
	strcpy(printLnVar->strVal, "ptln");
	Lexeme *printLnVal = newLexeme(BUILTIN);
	printLnVal->fVal = println;
	insertEnv(globalEnv, printLnVar, printLnVal);
	
	Lexeme *invokePrintEnvVar = newLexeme(VARIABLE);
	strcpy(invokePrintEnvVar->strVal, "ptenv");
	Lexeme *invokePrintEnvVal = newLexeme(BUILTIN);
	invokePrintEnvVal->fVal = invokePrintEnv;
	insertEnv(globalEnv, invokePrintEnvVar, invokePrintEnvVal);
	
	Lexeme *pptrVar = newLexeme(VARIABLE);
	strcpy(pptrVar->strVal, "pptr");
	Lexeme *pptrVal = newLexeme(BUILTIN);
	pptrVal->fVal = pptr;
	insertEnv(globalEnv, pptrVar, pptrVal);
	
	Lexeme *invokeExploreVar = newLexeme(VARIABLE);
	strcpy(invokeExploreVar->strVal, "explore");
	Lexeme *invokeExploreVal = newLexeme(BUILTIN);
	invokeExploreVal->fVal = invokeExplore;
	insertEnv(globalEnv, invokeExploreVar, invokeExploreVal);
	
	Lexeme *toArray_builtinVar = newLexeme(VARIABLE);
	strcpy(toArray_builtinVar->strVal, "toArray");
	Lexeme *toArray_builtinVal = newLexeme(BUILTIN);
	toArray_builtinVal->fVal = toArray_builtin;
	insertEnv(globalEnv, toArray_builtinVar, toArray_builtinVal);
	
	Lexeme *toListVar = newLexeme(VARIABLE);
	strcpy(toListVar->strVal, "toList");
	Lexeme *toListVal = newLexeme(BUILTIN);
	toListVal->fVal = toList;
	insertEnv(globalEnv, toListVar, toListVal);
	
	Lexeme *listGetVar = newLexeme(VARIABLE);
	strcpy(listGetVar->strVal, "lGet");
	Lexeme *listGetVal = newLexeme(BUILTIN);
	listGetVal->fVal = listGet;
	insertEnv(globalEnv, listGetVar, listGetVal);
	
	Lexeme *listPrependVar = newLexeme(VARIABLE);
	strcpy(listPrependVar->strVal, "lPrep");
	Lexeme *listPrependVal = newLexeme(BUILTIN);
	listPrependVal->fVal = listPrepend;
	insertEnv(globalEnv, listPrependVar, listPrependVal);
	
	Lexeme *listAppendVar = newLexeme(VARIABLE);
	strcpy(listAppendVar->strVal, "lApp");
	Lexeme *listAppendVal = newLexeme(BUILTIN);
	listAppendVal->fVal = listAppend;
	insertEnv(globalEnv, listAppendVar, listAppendVal);
	
	Lexeme *listInsertVar = newLexeme(VARIABLE);
	strcpy(listInsertVar->strVal, "lIns");
	Lexeme *listInsertVal = newLexeme(BUILTIN);
	listInsertVal->fVal = listInsert;
	insertEnv(globalEnv, listInsertVar, listInsertVal);
	
	Lexeme *listDeleteVar = newLexeme(VARIABLE);
	strcpy(listDeleteVar->strVal, "lDel");
	Lexeme *listDeleteVal = newLexeme(BUILTIN);
	listDeleteVal->fVal = listDelete;
	insertEnv(globalEnv, listDeleteVar, listDeleteVal);
	
	Lexeme *lengthVar = newLexeme(VARIABLE);
	strcpy(lengthVar->strVal, "length");
	Lexeme *lengthVal = newLexeme(BUILTIN);
	lengthVal->fVal = length_builtin;
	insertEnv(globalEnv, lengthVar, lengthVal);
	
	Lexeme *existsVar = newLexeme(VARIABLE);
	strcpy(existsVar->strVal, "exists");
	Lexeme *existsVal = newLexeme(BUILTIN);
	existsVal->fVal = exists;
	insertEnv(globalEnv, existsVar, existsVal);
	
	return globalEnv;
}
	