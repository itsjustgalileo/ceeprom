CC=clang
CFLAGS=-Wall -Wextra -Werror

ceeprom: ceeprom.o
	$(CC) -o ceeprom ceeprom.o

ceeprom.o: ceeprom.c
	$(CC) -c ceeprom.c -o ceeprom.o $(CFLAGS)
