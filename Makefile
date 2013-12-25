# SO zadanie zaliczeniowe nr 2
# Michal Wos mw336071

CFLAGS = -O2

all: serwer klient

serwer: serwer.c err.o mesg.h
	gcc $(CFLAGS) -pthread serwer.c mesg.h err.o -o serwer

klient: klient.c err.o mesg.h
	gcc $(CFLAGS) klient.c mesg.h err.o -o klient

err.o: err.c err.h
	gcc $(CFLAGS) -c err.c -o err.o

clean:
	rm -f *.o serwer klient
