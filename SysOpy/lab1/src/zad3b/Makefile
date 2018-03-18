ARGS = 2048 4096 1 1 1000 1 1000
LIB_SRC = ../zad1/blocklibrary.c
MAIN_SRC = ../zad2/tester.c
MAIN_DYN_SRC = ../zad3a/tester_dynamic.c

all: staticO0 staticO1 staticOs sharedO0 sharedO1 sharedOs dynamicO0 dynamicO1 dynamicOs


staticO0:
	gcc -c $(LIB_SRC)
	ar rcs libblocklibrary.a blocklibrary.o
	gcc -c $(MAIN_SRC)
	gcc tester.o libblocklibrary.a -o tester_static
	./tester_static $(ARGS)


staticO1:
	gcc -c -O1 $(LIB_SRC)
	ar rcs libblocklibrary.a blocklibrary.o
	gcc -c -O1 $(MAIN_SRC)
	gcc tester.o libblocklibrary.a -O1 -o tester_static
	./tester_static $(ARGS)


staticOs:
	gcc -c -Os $(LIB_SRC)
	ar rcs libblocklibrary.a blocklibrary.o
	gcc -c -Os $(MAIN_SRC)
	gcc tester.o libblocklibrary.a -Os -o tester_static
	./tester_static $(ARGS)


sharedO0:
	gcc -Wall -fPIC -c $(LIB_SRC)
	gcc -Wall -o libblocklibrary.so -shared blocklibrary.o
	gcc -Wl,-rpath=. -L. -o tester_shared $(MAIN_SRC) -lblocklibrary
	./tester_shared $(ARGS)


sharedO1:
	gcc -Wall -fPIC -c $(LIB_SRC) -O1
	gcc -Wall -o libblocklibrary.so -shared blocklibrary.o -O1
	gcc -Wl,-rpath=. -L. -o tester_shared $(MAIN_SRC) -lblocklibrary -O1
	./tester_shared $(ARGS)

	
sharedOs:
	gcc -Wall -fPIC -c $(LIB_SRC) -Os
	gcc -Wall -o libblocklibrary.so -shared blocklibrary.o -Os
	gcc -Wl,-rpath=. -L. -o tester_shared $(MAIN_SRC) -lblocklibrary -Os
	./tester_shared $(ARGS)


dynamicO0:
	gcc -Wall -fPIC -c $(LIB_SRC)
	gcc -Wall -o libblocklibrary.so -shared blocklibrary.o
	gcc -L. -o tester_dynamic $(MAIN_DYN_SRC) -ldl -D DLL
	./tester_dynamic $(ARGS)	


dynamicO1:
	gcc -Wall -fPIC -c $(LIB_SRC) -O1
	gcc -Wall -o libblocklibrary.so -shared blocklibrary.o -O1
	gcc -L. -o tester_dynamic $(MAIN_DYN_SRC) -ldl -D DLL -O1
	./tester_dynamic $(ARGS)


dynamicOs:
	gcc -Wall -fPIC -c $(LIB_SRC) -Os
	gcc -Wall -o libblocklibrary.so -shared blocklibrary.o -Os
	gcc -L. -o tester_dynamic $(MAIN_DYN_SRC) -ldl -D DLL -Os
	./tester_dynamic $(ARGS)

clean: 
	rm -f *.o *.so *.a tester_dynamic tester_shared tester_static
