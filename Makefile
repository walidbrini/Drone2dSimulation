# Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
CFLAGS = -Wall -Wextra -std=c99 -pthread

SRCS = jeu.c utilitaires.c threads_utility.c

OBJS = $(SRCS:.c=.o)
EXEC = my_program

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
