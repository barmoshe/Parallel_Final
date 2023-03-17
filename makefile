CC = mpicc
CFLAGS = -Wall -Wextra -g  
TARGET = main
SRC = newMain.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

run: 
	./main

run2:
	mpiexec -n 3 ./main