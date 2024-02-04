CFLAGS=-std=c11 -g -static

9cc: 9cc.c

test: 9cc
		./test.sh

d-test:
		docker run --rm -v .:/9cc -w /9cc compilerbook make test

d-it:
		docker run --rm -it -v .:/9cc -w /9cc compilerbook

clean:
		rm -f 9cc *.o *~ tmp*

.PHONY: test clean d-test d-it
