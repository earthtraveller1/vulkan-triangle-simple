$CC = "clang"
$CFLAGS = "-Wall", "-Wpedantic", "-Wextra" 

$INCLUDES = "-I$VULKAN_SDK/Include", "-I./vendor/glfw/include"
$LIBS = "-L$VULKAN_SDK/Lib", "-L./vendor/glfw/libs", "-lvulkan-1", "-lglfw3"

& $CC $CFLAGS $INCLUDES -o main main.c $LIBS
