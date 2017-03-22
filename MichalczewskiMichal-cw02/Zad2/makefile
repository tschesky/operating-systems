# source files.
SRC =  	dirs.c
SRC2 = dirs2.c
OUT = dirs
OUT2 = dirs2
 
# C compiler flags
CCFLAGS = -ansi -pedantic -std=c99 -W -Wall

# compiler
CC = gcc

all : $(OUT) $(OUT2)

$(OUT): $(SRC)
	$(CC) $(CCFLAGS) $(SRC) -o $(OUT)

$(OUT2): $(SRC2)
	$(CC) $(CCFLAGS) $(SRC2) -o $(OUT2)






# gcc -std=c99 ListTest.c -o ListTest -L. -lListShared <------ shared
# gcc -std=c99 ListTest.c -o ListTest -L. -lListStatic

