#!/usr/bin/env fish

set CC clang # If you wanna use some other compilers you could
#
# Add additional includes and library paths here i guess
set CFLAGS -Wall -Wextra -Wpedantic -Ivendor/glfw/include

# If GLFW has yet to be built, we build that one first
if not test -e "vendor/glfw/src/libglfw3.a"
    cmake -S vendor/glfw -B vendor/glfw \
        -D GLFW_BUILD_TESTS=NO -D GLFW_BUILD_EXAMPLES=NO -D GLFW_BUILD_DOCS=NO \
        -D CMAKE_C_COMPILER=$CC -G Ninja
    cmake --build vendor/glfw
end

set LIBS -lvulkan -lm vendor/glfw/src/libglfw3.a

set CMD $CC $CFLAGS -o main main.c $LIBS
$CMD

if not test -e "compile_commands.json"
    echo "[{" >> compile_commands.json
    echo "\"directory\": \"$PWD\"," >> compile_commands.json
    echo "\"file\": \"main.c\"," >> compile_commands.json
    echo "\"command\": \"$CMD\"" >> compile_commands.json
    echo "]}" >> compile_commands.json
end
