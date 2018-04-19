all: client.c server.c properties.h
	gcc -Wall client.c -o client -lrt
	gcc -Wall server.c -o server -lrt
	gcc -Wall properties.h

clean:
	rm client
	rm server
	rm properties.h.gch
