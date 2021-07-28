CFLAGS = -Wall -Iheader
LIBS = -lm -lrt
CC = gcc
TESTDIR = test/

TARGET = pingpong-mqueue
HEADERS = $(wildcard header/*.h)
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
TEST = $(notdir $(patsubst %.c, %, $(wildcard test/*.c)))

.PHONY: all default
default: $(TARGET)
all: $(TEST)
debug: CFLAGS += -DDEBUG -g
debug: $(TARGET)

$(TESTDIR)%.o: $(TESTDIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST): %:$(OBJECTS) $(TESTDIR)%.o $(HEADERS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	-rm -f *.o core
	-rm -f ./test/*.o

purge:
	-rm -f *.o core
	-rm -f $(TEST)
	-rm -f ./test/*.o