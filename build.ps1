$CC = "clang"
$CFLAGS = "-Wall", "-Wpedantic", "-Wextra" 

$INCLUDES = "-I$VULKAN_SDK/Include", "-Iglfw/include"
$LIBS = "$VULKAN_SDK/Lib/vulkan-1.lib", "glfw/src/glfw3.lib"

# Obviously we first need to build GLFW
cmake -S glfw -B glfw `
    -G Ninja -D CMAKE_C_COMPILER=$CC `
    -D GLFW_BUILD_TESTS=OFF -D GLFW_BUILD_EXAMPLES=OFF -D GLFW_BUILD_DOCS=OFF
cmake --build glfw

& $CC $CFLAGS $INCLUDES -o main main.c $LIBS
