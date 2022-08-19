CC=gcc
CFLAGS=-Wall -O2
DBFLAGS=-g -Wall

debug: USB-Watcher.c
	$(CC) $(DBFLAGS) $< -o build/usb-watcher

clean: 
	rm build/usb-watcher

build: USB-Watcher.c
	$(CC) $(CFLAGS) $< -o build/usb-watcher

install: USB-Watcher.c 
	$(CC) $(CFLAGS) $< -o build/usb-watcher
	cp build/usb-watcher /usr/bin/
