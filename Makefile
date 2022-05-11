SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .

SRCS = 8080emu.c main.c
OBJS = $(SRCS:.c=.o)
TARG = emulator
CC = gcc
OPTS = -Wall -O

all: $(TARG)
$(TARG): $(OBJS)
	$(CC) -o $(TARG) $(OBJS)

%.o: %.c
	$(CC) $(OPTS) -c $< -o $@

clean: 
	rm -f $(OBJS) $(TARG)
