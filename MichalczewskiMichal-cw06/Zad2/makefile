CC		= gcc
CFLAGS		= -std=c99 -Wall
OBJ = client.o 
OBJ2 = server.o
TARGET = client 
TARGET2 = server

all : compile  clean

compile: $(TARGET) $(TARGET2)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -lrt -o $@

$(TARGET2): $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ2) -lrt -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -lrt -o $@

.PHONY: clean
clean:
	@rm -f *.o 

