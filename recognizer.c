#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "lexeme.h"
#include "recognizer.h"

Lexeme *curLex;
Lexeme *lexPB_a;
Lexeme *lexPB_b;
LexList *lxx;

LexList *buildList(FILE *f){
	FILE *newFile; //might not exist
	initLexer(f);
	LexList *head = malloc(sizeof(LexList));
	LexList *tail = head;
	Lexeme *myLex = lex();
	while(myLex->type != ENDofINPUT){
		LexList *newTail = malloc(sizeof(LexList));
		newTail->lexx = myLex;
		tail->next = newTail;
		tail = tail->next;
		myLex = lex();
	}
	head->prev = tail; //only used for concatination
	LexList *cur = head;
	while(cur->next){
		if(cur->next->lexx->type == INCLUDE){
			char *fileLocation = cur->next->next->lexx->strVal;
			newFile = fopen(fileLocation, "r");
			if(!newFile){
				fprintf(stderr, "[IMPORT ERROR] File %s not found!", fileLocation);
				exit(2);
			}
			LexList *import = buildList(newFile);
			//import = head of new list, should be the new cur
			//import->prev = tail of new list, connect it next statement (cur->next->next)
			import->prev->next = cur->next->next->next; //sets the tail of the import
			cur->next = import->next;
		}
		cur = cur->next;
	}
	return head;
	fclose(f);
}

Lexeme *recognize(FILE *fp){
	lxx = buildList(fp);
	lxx = lxx->next;
	return statements();
}
		
int check(int type){
	return lxx->lexx->type == type;
}

Lexeme *match(int type){
	Lexeme *temp = lxx->lexx;
	if(lxx->next) lxx = lxx->next;
	if(type){
		if (temp->type != type){
			if(lxx->next) lxx = lxx->next;
			fprintf(stderr, "[SYNTAX ERROR - LINE %d]\n", lxx->lexx->lineNo);
			fprintf(stderr, "    Was looking for %s but found %s!\n", strType(type), strType(temp->type));
			exit(2);
		}
	}
	return temp;
}

Lexeme * body(){
	match(O_BRACE);
	Lexeme *t = statements();
	match(C_BRACE);
	return t;
}

Lexeme * statements(){
	Lexeme *s = statement();
	if (statementPending()){
		Lexeme *t = newLexeme(STATEMENTS);
		t->left = s;
		t->right = statements();
		return t;
	}
	else return s;
}

Lexeme * statement(){
	if (unaryPending()){
		return expression();
	}
	else if (check(DEF)){
		return definition();
	}
	else if (check(SET)){
		return assignment();
	}
	else if (check(DECL)){
		return declaration();
	}
	else if (check(IF)){
		return ifStatement();
	}
	else if (check(FOR)){
		return forLoop();
	}
	else if (check(WHILE)){
		return whileLoop();
	}
	else if (check(COPY) || check(SWAP)){
		return copySwap();
	}
	else if (check(HALT)){
		return match(HALT);
	}
	else if (check(DONE)){
		return match(DONE);
	}
	else{
		Lexeme *t = match(RETURN);
		if(unaryPending()) t->left = expression();
		return t;
	}
}

int statementPending(){
	return (unaryPending() || check(DEF) || check(SET)|| check(DECL) || check(COPY) || check(SWAP)
			|| check(IF) || check(WHILE) || check(FOR) || check(HALT) || check(DONE) || check(RETURN));
}

Lexeme * expression(){
	Lexeme *u = unary();
	if(operatorPending()){
		Lexeme *op = operator();
		Lexeme *b = newLexeme(BINARY);
		b->left = newLexeme(GLUE);
		b->left->left = u;
		b->left->right = op;
		b->right = expression();
		return b;
	}
	return u;
}

Lexeme * unary(){
	if (check(INTEGER)) return match(INTEGER);
	else if (check(STRING)) return match(STRING);
	else if (check(OBJECT)) return match(OBJECT);
	else if (check(LEXEME_NULL)) return match(LEXEME_NULL);
	else if (check(THIS)) return match(THIS);
	else if (check(VARIABLE)){
		Lexeme *v = object();
		if (check(O_BRACE)){
			Lexeme *t = newLexeme(FN_CALL);
			t->left = v;
			t->right = list();
			return t;
		}
		else return v;
	}
	else if (check(LAMBDA)){
		Lexeme *t = match(LAMBDA);
		t->left = list();
		t->right = body();
		return t;
	}
	else if (check(O_BRACE)){
		return list();
	}
	else if (check(O_BRACKET)){
		return array();
	}
	else if (check(O_PAREN)){
		Lexeme *t = match(O_PAREN);
		t->left = expression();
		match(C_PAREN);
		return t;
	}
	else if (check(MINUS)){
		Lexeme *t = newLexeme(UMINUS);
		match(MINUS);
		t->left = match(INTEGER);
		return t;
	}
	else if (check(NOT)){
		Lexeme *t = match(NOT);
		t->left = unary();
		return t;
	}
	else{
		fprintf(stderr, "[SYNTAX ERROR - LINE %d]\n", lxx->lexx->lineNo);
		fprintf(stderr, "    Was looking for (UNARY) but found %s!\n", strType(lxx->lexx->type));
		exit(2);
	}
}

int unaryPending(){
	return (check(INTEGER) ||
			check(STRING) ||
			check(VARIABLE) ||
			check(MINUS) ||
			check(O_PAREN) ||
			check(LAMBDA) ||
			check(OBJECT) ||
			check(O_BRACE) || //list pending
			check(O_BRACKET) ||
			check(LEXEME_NULL) ||
			check(THIS) ||
			check(NOT));
}

Lexeme * object(){
	Lexeme *objectHelper(){
		Lexeme *u = match(VARIABLE);
		if(check(DOT)){
			Lexeme *d = match(DOT);
			d->left = u;
			d->right = objectHelper();
			return d;
		}
		return u;
	}
	Lexeme *obj = objectHelper();
	if(check(EXISTS)){
		Lexeme *e = match(EXISTS);
		e->left = obj;
		return e;
	}
	else return obj;
}
	
Lexeme * operator(){
	if (check(PLUS)) return match(PLUS);
	else if (check(MINUS)) return match(MINUS);
	else if (check(MULTIPLY)) return match(MULTIPLY);
	else if (check(DIVIDE)) return match(DIVIDE);
	else if (check(EQUALS)) return match(EQUALS);
	else if (check(NOT_EQUALS)) return match(NOT_EQUALS);
	else if (check(LESS_THAN)) return match(LESS_THAN);
	else if (check(GREATER_THAN)) return match(GREATER_THAN);
	else if (check(LESS_EQ_THAN)) return match(LESS_EQ_THAN);
	else if (check(GREATER_EQ_THAN)) return match(GREATER_EQ_THAN);
	else if (check(INDEX)) return match(INDEX);
	else if (check(AND)) return match(AND);
	else if (check(OR)) return match(OR);
	else if (check(MODULO)) return match(MODULO);
	else return match(EXPONENT);
}

int operatorPending(){
	return (check(PLUS) ||
			check(MINUS) ||
			check(MULTIPLY) ||
			check(DIVIDE) ||
			check(EQUALS) ||
			check(NOT_EQUALS) ||
			check(LESS_THAN) ||
			check(GREATER_THAN) ||
			check(LESS_EQ_THAN) ||
			check(GREATER_EQ_THAN) ||
			check(INDEX) ||
			check(AND) ||
			check(OR) ||
			check(MODULO) ||
			check(EXPONENT));
}

Lexeme * definition(){
	Lexeme *t = match(DEF);
	t->left = object();
	if (check(O_BRACE)){
		Lexeme *l = list();
		if (check(O_BRACE)){
			t->right = newLexeme(PROCEDURE); //function definition
			t->right->left = l;
			t->right->right = body();
		}
		else t->right = l; //list def
	}
	else t->right = unary();
	return t;
}

Lexeme * assignment(){
	Lexeme *t = match(SET);
	t->left = object();
	t->right = unary();
	return t;
}

Lexeme * declaration(){
	match(DECL);
	Lexeme *t = newLexeme(DEF);
	t->left = object();
	t->right = newLexeme(LEXEME_NULL); //null
	return t;
}
	
Lexeme * copySwap(){
	Lexeme *t;
	if (check(COPY)) t = match(COPY);
	else t = match(SWAP);
	t->left = object();
	t->right = object();
	return t;
}
	
Lexeme * ifStatement(){
	Lexeme *t = match(IF);
	t->left = newLexeme(GLUE);
	t->left->left = unary();
	if(check(O_BRACE)){
		t->left->right = body();
	}
	else t->left->right = statement();
	if (check(ELSE)){
		match(ELSE);
		if (check(IF)) t->right = ifStatement();
		else if(check(O_BRACE)) t->right = body();
		else t->right = statement();
	}
	return t;
}

Lexeme * whileLoop(){
	Lexeme *t = match(WHILE);
	t->left = unary();
	t->right = body();
	return t;
}

Lexeme * forLoop(){
	Lexeme *t = match(FOR);
	t->left = newLexeme(GLUE);
	t->left->left = newLexeme(GLUE);
	t->left->right = newLexeme(GLUE);
	match(O_PAREN);
	t->left->left->left = match(VARIABLE);
	t->left->left->right = unary();
	t->left->right->left = unary();
	t->left->right->right = expression();
	match(C_PAREN);
	t->right = body();
	return t;
}

Lexeme * list(){
	Lexeme *t = newLexeme(LIST);
	match(O_BRACE);
	t->right = listItem();
	match(C_BRACE);
	return t;
}

Lexeme * array(){
	Lexeme *t = newLexeme(ARRAY);
	match(O_BRACKET);
	t->right = listItem();
	match(C_BRACKET);
	return t;
}

Lexeme * listItem(){
	Lexeme *t = NULL;
	if (unaryPending()){
		t = newLexeme(ITEM);
		t->left = unary();
		t->right = listItem();
	}
	return t;
}