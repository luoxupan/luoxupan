all: ygxqqx

ygxqqx: ygxqqx.c
	gcc -W -Wall -lpthread -oygxqqx ygxqqx.c util.c http.c rio.c

clean:
	rm ygxqqx
