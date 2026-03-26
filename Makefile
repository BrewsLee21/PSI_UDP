CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ_SENDER = sender.o utils.o
OBJ_RECEIVER = receiver.o utils.o

all: sender receiver

sender: $(OBJ_SENDER)
	$(CC) $(CFLAGS) -o sender $(OBJ_SENDER)

receiver: $(OBJ_RECEIVER)
	$(CC) $(CFLAGS) -o receiver $(OBJ_RECEIVER)

sender.o: sender.c utils.h
	$(CC) $(CFLAGS) -c sender.c

receiver.o: receiver.c utils.h
	$(CC) $(CFLAGS) -c receiver.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f *.o sender receiver
