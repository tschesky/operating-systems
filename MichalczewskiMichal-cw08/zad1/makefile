CC		= gcc
CFLAGS		= -std=c99 -Wall -pthread
OBJ = mainA.o
OBJ2 = mainB.o 
OBJ3 = mainC.o
TARGET = mainA
TARGET2 = mainB
TARGET3 = mainC 

all : compile  clean

compile: $(TARGET) $(TARGET2) $(TARGET3)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(TARGET2): $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ2) -o $@

$(TARGET3): $(OBJ3)
	$(CC) $(CFLAGS) $(OBJ3) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	@rm -f *.o 

