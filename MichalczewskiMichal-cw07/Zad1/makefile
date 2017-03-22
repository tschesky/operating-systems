CC		= gcc
CFLAGS		= -std=c99 -Wall
OBJ2 = consumer.o
TARGET2 = consumer

all : compile  clean

compile: $(TARGET2)

$(TARGET2): $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ2) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	@rm -f *.o 

