all: barber.c client.c
	gcc -Wall barber.c -o barber -pthread -lrt
	gcc -Wall client.c -o client -pthread -lrt

clean:
	rm barber
	rm client
