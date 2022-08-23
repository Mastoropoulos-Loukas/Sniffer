REQ = sniffer.o listener.o worker.o
CC = g++

listener: listener.cpp
	$(CC) -o listener listener.cpp

worker: worker.cpp
	$(CC) -o worker worker.cpp

sniffer : sniffer.cpp
	$(CC) -o sniffer sniffer.cpp

all: listener.cpp worker.cpp sniffer.cpp
	$(CC) -o listener listener.cpp
	$(CC) -o worker worker.cpp
	$(CC) -o sniffer sniffer.cpp

clean:
	rm sniffer listener worker
	