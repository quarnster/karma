#
# sstrip available at http://die.die.ms/avoozl/ld/sstrip.c
#

.SUFFIXES: .cpp .c .o

CC = gcc
CFLAGS = -Wall
LIBS = -lstdc++

BIN = midi2qm
OBJS = \
	MidiLoader.o \
	midi2qm.o


all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LIBS)


clean:
	rm -f *.o *.gz *.bb *.bbg *.da *.gcov *.out *~ $(BIN) $(BIN2)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@
