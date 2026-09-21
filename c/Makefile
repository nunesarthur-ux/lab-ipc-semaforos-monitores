CC=gcc
CFLAGS=-Wall -Wextra -pthread

all: base solucao

base:
	$(CC) $(CFLAGS) base/produtor_consumidor.c -o base/pc

solucao:
	$(CC) $(CFLAGS) solucao/produtor_consumidor.c -o solucao/pc

clean:
	rm -f base/pc solucao/pc

.PHONY: all base solucao clean
