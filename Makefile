SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .

LDLIBS = -lSDL2 -lSDL2_mixer
SRCS = 8080emu.c main.c machine.c screen.c tests.c
OBJS = $(SRCS:.c=.o)
TARG = emulator
CC = gcc
CFLAGS = -Wall -g

all: $(TARG)
$(TARG): $(OBJS)
	$(CC) -o $(TARG) $(OBJS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

clean: 
	rm -f $(OBJS) $(TARG)
