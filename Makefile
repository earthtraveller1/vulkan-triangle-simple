CC = clang
CFLAGS = -Wall -Wextra -Wpedantic # Add additional includes and library paths here i guess
LIBS = -lvulkan -lglfw

.PHONY all: main

main: main.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm main
