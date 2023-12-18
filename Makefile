SOURCE = xhotkey.c
BIN_NAME = xhotkey
CC = gcc
CFLAGS = -Wall -Wextra -lX11

$(BIN_NAME) : xhotkey.c
	$(CC) -o $(BIN_NAME) $(SOURCE) $(CFLAGS)

.PHONY : clean
clean :
	rm -f *.o $(BIN_NAME)
.PHONY : intclean
intclean :
	rm -f *.o
