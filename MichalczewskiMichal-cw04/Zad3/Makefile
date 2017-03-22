CC		= gcc
CFLAGS		= -std=c99 -Wall 
OBJ = child.o
OBJ2 =	parent.o
TARGET = child
TARGET2 =parent

all : compile  clean

compile: $(TARGET) $(TARGET2)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(TARGET2): $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ2) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	@rm -f *.o 

