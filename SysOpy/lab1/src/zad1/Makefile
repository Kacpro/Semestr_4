all: libraryStatic libraryShared

libraryStatic: blocklibrary.c
	gcc -Wall -c blocklibrary.c
	ar rcs libblocklibrary.a blocklibrary.o

libraryShared: blocklibrary.c
	gcc -Wall -fPIC -c blocklibrary.c
	gcc -Wall -o libblocklibrary.so -shared blocklibrary.o

clean: 
	rm -f *.o *.a *.so
