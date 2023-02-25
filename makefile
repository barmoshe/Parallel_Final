CC = mpicc
CFLAGS = -Wall -Wextra -g
TARGET = ManagerPar
SRC = ManagerPar.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

run: 
	./Manager

run2:
	mpiexec -n 2 ./ManagerPar