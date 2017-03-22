CC		= gcc
CFLAGS		= -std=c99 -Wall
OBJS = grep.o
TARGETS = grep

all : compile  clean

compile: $(TARGETS)

$(TARGETS): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	@rm -f *.o 

