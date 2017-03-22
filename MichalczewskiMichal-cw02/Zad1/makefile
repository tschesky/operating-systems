# source files.
SRC =  	gen_lib.c
SRC2 = gen_system.c
SRC3 = sort_lib.c
SRC4 = sort_system.c
OUT = gen_lib
OUT2 = gen_system
OUT3= sort_lib
OUT4= sort_system
 
# C compiler flags
CCFLAGS = -ansi -pedantic -std=c99 -W -Wall

# compiler
CC = gcc

all : $(OUT) $(OUT2) $(OUT3) $(OUT4) run clean

$(OUT): $(SRC)
	$(CC) $(CCFLAGS) $(SRC) -o $(OUT)

$(OUT2): $(SRC2)
	$(CC) $(CCFLAGS) $(SRC2) -o $(OUT2)

$(OUT3): $(SRC3)
	$(CC) $(CCFLAGS) $(SRC3) -o $(OUT3)

$(OUT4): $(SRC4)
	$(CC) $(CCFLAGS) $(SRC3) -o $(OUT4)

run: all
	@echo "Generating files: "
	./$(OUT) 4_1.txt 2000 4
	./$(OUT) 4_2.txt 1000 4
	./$(OUT) 4_1s.txt 2000 4
	./$(OUT) 4_2s.txt 1000 4
	./$(OUT) 512_1.txt 2000 512
	./$(OUT) 512_2.txt 1000 512
	./$(OUT) 512_1s.txt 2000 512
	./$(OUT) 512_2s.txt 1000 512
	./$(OUT) 4096_1.txt 2000 4096
	./$(OUT) 4096_2.txt 1000 4096
	./$(OUT) 4096_1s.txt 2000 4096
	./$(OUT) 4096_2s.txt 1000 4096
	./$(OUT) 8192_1.txt 2000 8192
	./$(OUT) 8192_2.txt 1000 8192
	./$(OUT) 8192_1s.txt 2000 8192
	./$(OUT) 8192_2s.txt 1000 8192
	@echo "Sorting records of length 4, 2000 lines"
	./$(OUT3) 4_1.txt 4
	./$(OUT4) 4_1s.txt 4
	@echo "Sorting records of length 4, 1000 lines"
	./$(OUT3) 4_2.txt 4
	./$(OUT4) 4_2s.txt 4
	@echo "Sorting records of length 512, 2000 lines"
	./$(OUT3) 512_1.txt 512
	./$(OUT4) 512_1s.txt 512
	@echo "Sorting records of length 512, 1000 lines"
	./$(OUT3) 512_2.txt 512
	./$(OUT4) 512_2s.txt 512
	@echo "Sorting records of length 4096, 2000 lines"
	./$(OUT3) 4096_1.txt 4096
	./$(OUT4) 4096_1s.txt 4096
	@echo "Sorting records of length 4096, 1000 lines"
	./$(OUT3) 4096_2.txt 4096
	./$(OUT4) 4096_2s.txt 4096
	@echo "Sorting records of length 8192, 2000 lines"
	./$(OUT3) 8192_1.txt 8192
	./$(OUT4) 8192_1s.txt 8192
	@echo "Sorting records of length 8192, 1000 lines"
	./$(OUT3) 8192_2.txt 8192
	./$(OUT4) 8192_2s.txt 8192

clean:
	-rm *.txt





# gcc -std=c99 ListTest.c -o ListTest -L. -lListShared <------ shared
# gcc -std=c99 ListTest.c -o ListTest -L. -lListStatic

