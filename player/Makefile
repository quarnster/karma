#
# sstrip available at http://die.die.ms/avoozl/ld/sstrip.c
#

.SUFFIXES: .cpp .c .o

CC = gcc
CFLAGS = -Wall -Os -I../karmalib
LIBS = -L../karmalib -lkarma -L/usr/X11R6/lib -lm -lgcc_s

BIN = player
BIN2 = player.lnx
OBJS = \
       player.o \
       unixdevice.o


all: $(BIN2)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LIBS)

$(BIN2): $(BIN)
	sstrip $(BIN)
	gzip -f --best $(BIN)
	cat gunzip.head $(BIN).gz > $(BIN2)
	rm $(BIN).gz
	chmod +x $(BIN2)



clean:
	rm -f *.o *.gz *.bb *.bbg *.da *.gcov *.out *~ $(BIN) $(BIN2)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@
