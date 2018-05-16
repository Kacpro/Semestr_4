all: barber.c client.c
	gcc -Wall barber.c -o barber -lpthread -lrt
	gcc -Wall client.c -o client -lpthread -lrt

clean:
	rm barber
	rm client
