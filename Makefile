CC = gcc
CFLAGS = -Wall -Wextra -g

OBJ_COMMON = utils.o network.o
OBJ_SENDER = sender.o $(OBJ_COMMON)
OBJ_RECEIVER = receiver.o $(OBJ_COMMON)

all: sender receiver

sender: $(OBJ_SENDER)
	$(CC) $(CFLAGS) -o sender $(OBJ_SENDER)

receiver: $(OBJ_RECEIVER)
	$(CC) $(CFLAGS) -o receiver $(OBJ_RECEIVER)

sender.o: sender.c utils.h network.h
	$(CC) $(CFLAGS) -c sender.c

receiver.o: receiver.c utils.h network.h
	$(CC) $(CFLAGS) -c receiver.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

network.o: network.c network.h
	$(CC) $(CFLAGS) -c network.c

clean:
	rm -f *.o sender receiver
