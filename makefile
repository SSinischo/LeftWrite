OBJS = lexeme.o recognizer.o environment.o evaluator.o builtin.o dpl.o
OOPTS = -Wall -std=c99 -Wextra -g -c
LOPTS = -Wall -std=c99 -Wextra -g

dpl : $(OBJS)
	gcc $(LOPTS) -o dpl $(OBJS)

dpl.o: dpl.c lexeme.h recognizer.o evaluator.o builtin.o
	gcc $(OOPTS) dpl.c

builtin.o : builtin.c builtin.h lexeme.o evaluator.o environment.o
	gcc $(OOPTS) builtin.c
	
evaluator.o : evaluator.c evaluator.h environment.o lexeme.o
	gcc $(OOPTS) evaluator.c
	
environment.o : environment.c environment.h lexeme.o
	gcc $(OOPTS) environment.c
	
recognizer.o : recognizer.c recognizer.h lexeme.o
	gcc $(OOPTS) recognizer.c

lexeme.o : lexeme.c lexeme.h
	gcc $(OOPTS) lexeme.c
	
clean:
	rm -rf $(OBJS) dpl

error1:
	cat tests/error1.lr
error1x:
	dpl tests/error1.lr
error2:
	cat tests/error2.lr
error2x:
	dpl tests/error2.lr
error3:
	cat tests/error3.lr
error3x:
	dpl tests/error3.lr
error4:
	cat tests/error4.lr
error4x:
	dpl tests/error4.lr
error5:
	cat tests/error5.lr
error5x:
	dpl tests/error5.lr
arrays:
	cat tests/arrays.lr
arraysx:
	dpl tests/arrays.lr
conditionals:
	cat tests/conditionals.lr
conditionalsx:
	dpl tests/conditionals.lr
recursion:
	cat tests/recursion.lr
recursionx:
	dpl tests/recursion.lr
iteration:
	cat tests/iteration.lr
iterationx:
	dpl tests/iteration.lr
functions:
	cat tests/functions.lr
functionsx:
	dpl tests/functions.lr
lambda:
	cat tests/lambda.lr
lambdax:
	dpl tests/lambda.lr
dictionary:
	cat tests/dictionary.lr
dictionaryx:
	dpl tests/dictionary.lr
	