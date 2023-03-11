CC = gcc
CFLAGS = -Wall -Wextra -g  
TARGET = main
SRC = main.c
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
	mpiexec -n 2 ./ManagerPar