all: server.c client.c
	rm -f test
	gcc -Wall server.c -o server -lpthread
	gcc -Wall client.c -o client -lpthread


clean: 
	rm client
	rm server

