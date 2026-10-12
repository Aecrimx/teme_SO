CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDLIBS = -lncurses

TARGET = breakout
SRC = main.c
OBJ = $(SRC: .c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJ) $(TARGET)