CFLAGS = -Wall -Iheader -g
LIBS = -lm
CC = gcc

TESTOBJS = $(patsubst %.c, %.o, $(wildcard test/*.c))
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard header/*.h)

TEST = $(notdir $(patsubst %.c, %, $(wildcard test/*.c)))
PPOS = $(wildcard *.c)

.PHONY: clean purge debug
default: $(TEST)
# .precious: $(TEST) default
	
debug: CFLAGS += -DDEBUG -g
debug: $(TEST)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST): $(OBJECTS)
	$(CC) $(CFLAGS) test/$@.c $(OBJECTS) -o $@

clean:
	-rm -f *.o core
	-rm -f ./test/*.o

purge:
	-rm -f *.o core
	-rm -f $(TEST)
	-rm -f ./test/*.o