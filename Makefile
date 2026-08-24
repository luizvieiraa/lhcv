CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic

all: processflow

processflow:
	$(CC) $(CFLAGS) src/main.c src/task.c src/job.c -o processflow

run:
	./processflow

clean:
	rm -f processflow