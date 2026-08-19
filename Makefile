CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
TARGET = processflow
SRC = src/main.c src/task.c

run: $(TARGET)
	.\$(TARGET).exe

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	del /Q $(TARGET).exe