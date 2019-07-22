all:
	gcc serwer.c -lncurses -lpthread -std=gnu99 -o serwer
	gcc klient.c -std=gnu99 -o klient