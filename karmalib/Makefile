.SUFFIXES: .c .o

CC = gcc
#CFLAGS = -Wall -g -pg -fprofile-arcs -ftest-coverage
CFLAGS = -Wall -Os
BIN = libkarma.a

OBJS = \
       adsr.o \
       channel.o \
       karma.o \
       program.o \
       waveform.o 


$(BIN): $(OBJS)
	ar rcs $(BIN) $(OBJS)

all: $(BIN)

clean:
	rm -f *.o *.gz *.bb *.bbg *.da *.gcov *.out *~ $(BIN)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
