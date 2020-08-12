all: bbi.o bbi_test

bbi.o:
	gcc -c -o bbi.o bbi.c

bbi_test: bbi.o
	gcc -o bbi_test bbi_test.c bbi.o -lcriterion
	./bbi_test

clean:
	rm -f bbi.o bbi_test.o bbi_test

foo:
	echo "Hello"


