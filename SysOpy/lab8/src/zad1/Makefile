all: main.c generator.c
	gcc -Wall main.c -o main -lpthread -lm
	gcc -Wall generator.c -o generator

clean:
	rm main
	rm generator
