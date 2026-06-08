CC = clang
CFLAGS = -Wall -Wextra -Wpedantic # Add additional includes and library paths here i guess
LIBS = -lvulkan -lglfw

$(CC) $(CFLAGS) -o main main.c $(LIBS)
