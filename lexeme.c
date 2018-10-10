#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include "lexeme.h"

FILE *f;
int lineNumber;
char pbChar[20];
int pbIndex;

char *strType(int i){
	char *array[]={"UNKNOWN", "INTEGER", "VARIABLE", "STRING", "LEXEME_NULL",
		"DEF", "SET", "DECL", "RETURN", "DONE", "PROCEDURE", "FN_CALL", "LAMBDA", "CLOSURE", "BUILTIN",
		"IF", "ELSE", "FOR", "WHILE", "AND", "OR", "NOT", "EXISTS", "COPY", "SWAP",
		"EQUALS", "NOT_EQUALS", "PLUS", "MINUS", "MULTIPLY", "DIVIDE", "LESS_THAN", "GREATER_THAN", "LESS_EQ_THAN", "GREATER_EQ_THAN", "EXPONENT", "MODULO", "UMINUS",
		"O_BRACE", "C_BRACE", "O_PAREN", "C_PAREN", "O_BRACKET", "C_BRACKET", "COMMA", "DOT", "LIST", "ARRAY", "INDEX", 
		"STATEMENTS", "ITEM", "TABLE", "ENVIRONMENT", "THIS", "OBJECT", "BINARY", "GLUE", "HALT", "INCLUDE", "ENDofINPUT"};
	return array[i];
}

void displayLex(Lexeme *l){
	if (l->type == 1){
		printf("int %d", l->intVal);
	}
	else if (l->type == 2){
		printf("var %s", l->strVal);
	}
	else if (l->type == 3){
		printf("string %s", l->strVal);
	}
	else{
		printf("%s", strType(l->type));
	}
}

void printPreorder(Lexeme *tree){
	void loop(Lexeme *t, int spaces, int side){
		for(int i=0; i<spaces; i++){
			printf("| ");
		}
		if(side == 0) printf("[L] ");
		if(side == 1) printf("[R] ");
		printf("%s", strType(t->type));
		if (t->type == INTEGER|| t->type == ENVIRONMENT) printf(": %d", t->intVal);
		else if (t->type == VARIABLE || t->type == STRING) printf(" : %s", t->strVal);
		
		if(t->type != ENVIRONMENT){
			if(t->left != NULL){
				printf("\n");
				loop(t->left, spaces+1, 0);
			}
			if(t->right != NULL){
				printf("\n");
				loop(t->right, spaces+1, 1);
			}
		}
	}
	printf("\n- - Preorder  Traversal - -\n*");
	loop(tree, 0, -1);
}

int explore(Lexeme *t){
	printf("\e[1;1H\e[2J");
	
		printf("(%p)", t);
		printf("\033[1;19H[*]");
	displayLex(t);
	if(t->left){
		printf("\033[2;20H║\033[3;20H║\033[4;8H╔═══════════╩\033[5;7H[L]");
		displayLex(t->left);
		if(t->left->left){
			printf("\033[6;8H║\033[7;2H╔═════╩\033[8;1H[LL]");
			displayLex(t->left->left);
		}
		if(t->left->right){
			printf("\033[6;8H║\033[7;8H╩═══════╗\033[8;16H║\033[9;15H[LR]");
			displayLex(t->left->right);
		}
	}
	if(t->right){
		printf("\033[2;20H║\033[3;20H║\033[4;20H╩════════════╗\033[5;32H[R]");
		displayLex(t->right);
		if(t->right->left){
			printf("\033[6;33H║\033[7;25H╔═══════╩\033[8;24H[RL]");
			displayLex(t->right->left);
		}
		if(t->right->right){
			printf("\033[6;33H║\033[7;33H╩═══════╗\033[8;41H║\033[9;40H[RR]");
			displayLex(t->right->right);
		}
	}	
	printf("\n");
	char ch = getchar();
	getchar();
	if(ch == 'a'){
		if(!t->left)
			return explore(t);
		if(explore(t->left))
			return explore(t);
	}
	if(ch == 'd'){
		if(!t->right)
			return explore(t);
		if(explore(t->right))
			return explore(t);
	}
	else if (ch == 'w'){
		return 1;
	}
	return 0;
}

Lexeme * newLexeme(int t){
	Lexeme *l = malloc(sizeof(Lexeme));
	l->type = t;
	l->intVal = 0;
	l->shortCircuit = 0;
	l->left = 0;
	l->right = 0;
	l->lineNo = lineNumber;
	return l;
}

Lexeme * cloneLex(Lexeme *orig){
	Lexeme *dest = newLexeme(orig->type);
	strcpy(dest->strVal, orig->strVal);
	dest->intVal = orig->intVal;
	dest->fVal = orig->fVal;
	dest->left = orig->left;
	dest->right = orig->right;
	dest->shortCircuit = orig->shortCircuit;
	return dest;
}

void initLexer(FILE *ff){
	f = ff;
	pbIndex = 0;
	lineNumber = 1;
}

char doRead(){
	if (pbIndex){
		char t = pbChar[pbIndex];
		pbIndex--;
		return t;
	}
	char ch;
	fscanf(f, "%c", &ch);
	if(ch == '\n') lineNumber = lineNumber+1;
	return ch;
}

void pushback(char c){
	pbIndex++; //woah wait waht
	pbChar[pbIndex] = c;
}

void skipToScissors(){
	char c = doRead();
	while(c != '\%'){
		c = doRead();
		if (feof(f)) return;
	}
	c = doRead();
	if(c != '<')
		skipToScissors();
}

void skipWhiteSpace(){
	if (feof(f)) return; //gets called on imports
	char c = doRead();
	while (isspace (c) || c == '~'){
		if (feof(f)) break;
		if (c == '~')
			while(c != '\n') c = doRead();
		else c = doRead();
	}
	pushback(c);
}

Lexeme * lexNumber(){
	char c = doRead();
	char token[512] = "";
	int i = 0;
	while(isdigit(c)){
		token[i] = c;
		c = doRead();
		i++;
	}
	token[i] = '\0';
	pushback(c);
	
	Lexeme *newLex = newLexeme(INTEGER);
	newLex->intVal = atoi(token);
	return newLex;
}

Lexeme * lexVariableOrKeyword(){
	char c = doRead();
	char token[512] = "";
	int i = 0;
	
	while(isdigit(c) || isalpha(c) || c == '_'){
		token[i] = c;
		c = doRead();
		i++;
	}
	token[i] = '\0';
	pushback(c);
	
	if(!strcasecmp(token, "def")) return newLexeme(DEF);
	else if(!strcasecmp(token, "set")) return newLexeme(SET);
	else if(!strcasecmp(token, "decl")) return newLexeme(DECL);
	else if(!strcasecmp(token, "if")) return newLexeme(IF);
	else if(!strcasecmp(token, "else")) return newLexeme(ELSE);
	else if(!strcasecmp(token, "for")) return newLexeme(FOR);
	else if(!strcasecmp(token, "while")) return newLexeme(WHILE);
	else if(!strcasecmp(token, "return")) return newLexeme(RETURN);
	else if(!strcasecmp(token, "done")) return newLexeme(DONE);
	else if(!strcasecmp(token, "lambda")) return newLexeme(LAMBDA);
	else if(!strcasecmp(token, "OBJECT")) return newLexeme(OBJECT);
	else if(!strcasecmp(token, "HALT")) return newLexeme(HALT);
	else if(!strcasecmp(token, "NULL")) return newLexeme(LEXEME_NULL);
	else if(!strcasecmp(token, "this")) return newLexeme(THIS);
	else if(!strcasecmp(token, "copy")) return newLexeme(COPY);
	else if(!strcasecmp(token, "swap")) return newLexeme(SWAP);
	else if(!strcasecmp(token, "_include")) return newLexeme(INCLUDE);
	else if(!strcasecmp(token, "FALSE")) return newLexeme(INTEGER);
	else if(!strcasecmp(token, "TRUE")){
		Lexeme *newLex = newLexeme(INTEGER);
		newLex->intVal = 1;
		return newLex;
	}
	else{
		Lexeme *newLex = newLexeme(VARIABLE);
		strcpy(newLex->strVal, token);
		return newLex;
	}
}

Lexeme * lexString(){
	char c = doRead();
	char token[512] = "";
	int i = 0;
	
	while(c != '\"'){
		token[i] = c;
		c = doRead();
		i++;
	}
	token[i] = '\0';
	Lexeme *newLex = newLexeme(STRING);
	strcpy(newLex->strVal, token);
	return newLex;
}
	
Lexeme * lex(){
	char ch;
	char d; //second pushback if needed
	skipWhiteSpace();
	ch = doRead();

	switch(ch) 
	{ 
		// single character tokens 
		case '{': return newLexeme(O_BRACE); 
		case '}': return newLexeme(C_BRACE); 
		case '(': return newLexeme(O_PAREN); 
		case ')': return newLexeme(C_PAREN); 
		case '[': return newLexeme(O_BRACKET); 
		case ']': return newLexeme(C_BRACKET); 
		case ',': return newLexeme(COMMA);
		case '.': return newLexeme(DOT);
		case '+': return newLexeme(PLUS); //what about ++ and += ?
		case '*': return newLexeme(MULTIPLY); 
		case '-': return newLexeme(MINUS); 
		case '/': return newLexeme(DIVIDE); 
		case '<': 
			d = doRead();
			if (d == '=') return newLexeme(LESS_EQ_THAN); 
			else{
				pushback(d);
				return newLexeme(LESS_THAN);
			}
		case '>': 
			d = doRead();
			if (d == '=') return newLexeme(GREATER_EQ_THAN); 
			else{
				pushback(d);
				return newLexeme(GREATER_THAN);
			}
		case '=': return newLexeme(EQUALS); 
		case '#': return newLexeme(INDEX);
		case '&': return newLexeme(AND);
		case '|': return newLexeme(OR);
		case '?': return newLexeme(EXISTS);
		case '!': 
			d = doRead();
			if (d == '=') return newLexeme(NOT_EQUALS);
			else{
				pushback(d);
				return newLexeme(NOT);
			}
		case '^': return newLexeme(EXPONENT);
		case '\%': 
			d = doRead();
			if (d == '<'){
				skipToScissors();
				return lex();
			}
			else{
				pushback(d);
				return newLexeme(MODULO);
			}
		default:
			if (isdigit(ch)){ 
				pushback(ch);
				return lexNumber(); 
			} 
			else if (isalpha(ch) || ch == '_'){ 
				pushback(ch);
				return lexVariableOrKeyword();
			} 
			else if (ch == '\"'){ 
				return lexString(); 
			} 
			else if (feof(f)){
				return newLexeme(ENDofINPUT);
			}
			else{
				fprintf(stderr, "%d:[LEX ERROR] Unknown input character: %c!\n", lineNumber, ch);
				exit(2);
			}
	}
}

