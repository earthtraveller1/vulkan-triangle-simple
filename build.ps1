# Of course, we first need to build the deps, but only if the dependencies 
# are not already built
if (!(Test-Path -Path "glfw/src/glfw3.lib")) {
    cmake -S glfw -B glfw `
        -G Ninja -D CMAKE_C_COMPILER=clang `
        -D GLFW_BUILD_TESTS=OFF -D GLFW_BUILD_EXAMPLES=OFF -D GLFW_BUILD_DOCS=OFF
    cmake --build glfw
}

$CC = "clang"
$CFLAGS = "-Wall", "-Wpedantic", "-Wextra" 

$INCLUDES = "-I$env:VULKAN_SDK/Include", "-Iglfw/include"
$LIBS = "$env:VULKAN_SDK/Lib/vulkan-1.lib", "glfw/src/glfw3.lib", "-luser32", "-lshell32", "-lgdi32" 

& $CC $CFLAGS $INCLUDES -o main main.c $LIBS
