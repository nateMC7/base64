CFLAGS=-Wall -ggdb -std=c11 -pedantic
LIBS=-lreadline -lm

main: ./src/main.c
	$(CC) $(CFLAGS) ./src/main.c -o ./bin/base64 $(LIBS)
