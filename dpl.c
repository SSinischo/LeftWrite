#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexeme.h"
#include "recognizer.h"
#include "evaluator.h"
#include "builtin.h"

int main(int argc,char **argv){
    (void)argc;
	FILE *f;
	if(argc == 1){
		fprintf(stderr, "No file specified!\n");
		exit(1);
	}
	if(argc == 2){
		f = fopen(argv[1],"r");
		if(!f){
			fprintf(stderr, "Could not open file!\n");
			exit(1);
		}
		eval(recognize(f), createDefaultEnv());
	}
	else if(argc == 3){
		f = fopen(argv[2],"r");
		if(!f){
			fprintf(stderr, "Could not open file!\n");
			exit(1);
		}
		if (!strcmp(argv[1], "-l")){
			LexList *lxx = buildList(f);
			lxx = lxx->next;
			while(lxx){
				displayLex(lxx->lexx);
				printf("\n");
				lxx = lxx->next;
			}
		}
		else if (!strcmp(argv[1], "-t")){
			Lexeme *t = recognize(f);
			printPreorder(t);
		}
		else if (!strcmp(argv[1], "-e")){
			Lexeme *t = recognize(f);
			explore(t);
		}
	}
}
