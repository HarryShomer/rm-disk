CC = g++
CFLAGS = -g -Wall -std=c++17

default: rm-disk

rm-disk: rmDisk.o 
	$(CC) $(CFLAGS) -o rm-disk rmDisk.o

rmDisk.o: rmDisk.cpp
	$(CC) $(CFLAGS) -c rmDisk.cpp

clean:
	rm -f rm-disk *.o *~
