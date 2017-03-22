CC		= gcc
CFLAGS		= -std=c99 -Wall -pthread
OBJ2 = writer.o
TARGET2 = writer

all : compile  clean

compile: $(TARGET) $(TARGET2)

$(TARGET2): $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ2) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	@rm -f *.o 

