CC = gcc
CFLAGS = -Wall -O2
TARGET = lisp
SRC = main.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c lisp15.h
	$(CC) $(CFLAGS) -c $<

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

clean:
	rm -f $(OBJ) $(TARGET)


.PHONY: check
check:
	cppcheck --enable=warning,performance,portability --std=c17 --library=posix -j4 .