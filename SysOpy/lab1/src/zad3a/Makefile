ARGS = 2048 4096 1 1 1000 1 1000
LIB_SRC = ../zad1/blocklibrary.c
MAIN_SRC = ../zad2/tester.c
MAIN_DYN_SRC = ../zad3a/tester_dynamic.c

all: static shared dynamic


static:
	gcc -c $(LIB_SRC)
	ar rcs libblocklibrary.a blocklibrary.o
	gcc -c $(MAIN_SRC)
	gcc tester.o libblocklibrary.a -o tester_static
	./tester_static $(ARGS)


shared:
	gcc -Wall -fPIC -c $(LIB_SRC)
	gcc -Wall -o libblocklibrary.so -shared blocklibrary.o
	gcc -Wl,-rpath=. -L. -o tester_shared $(MAIN_SRC) -lblocklibrary
	./tester_shared $(ARGS)


dynamic:
	gcc -Wall -fPIC -c $(LIB_SRC)
	gcc -Wall -o libblocklibrary.so -shared blocklibrary.o
	gcc -L. -o tester_dynamic $(MAIN_DYN_SRC) -ldl -D DLL
	./tester_dynamic $(ARGS)	


clean: 
	rm -f *.o *.so *.a tester_dynamic tester_shared tester_static
