CC		= gcc
CFLAGS		= -std=c99 -Wall -pthread
OBJ = mainA.o
TARGET = mainA

all : compile  clean

compile: $(TARGET) $(TARGET2) $(TARGET3)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	@rm -f *.o 

