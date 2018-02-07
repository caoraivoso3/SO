all: useful client ServerH server

useful: useful.h
	gcc useful.h -o useful

client: client.c
	gcc client.c -o client -pthread -lncurses -lm

ServerH: ServerH.h
	 gcc ServerH.h -o ServerH

server: server.c
	gcc server.c -o server -pthread -lm



