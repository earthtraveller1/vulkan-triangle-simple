# Of course, we first need to build the deps, but only if the dependencies 
# are not already built
if (!(Test-Path -Path "vendor/glfw/src/glfw3.lib"))
{
    cmake -S vendor/glfw -B vendor/glfw `
        -G Ninja -D CMAKE_C_COMPILER=clang `
        -D GLFW_BUILD_TESTS=OFF -D GLFW_BUILD_EXAMPLES=OFF -D GLFW_BUILD_DOCS=OFF
    cmake --build glfw
}

$CC = "clang"
$CFLAGS = "-Wall", "-Wpedantic", "-Wextra" 

$INCLUDES = "-I$env:VULKAN_SDK/Include", "-Ivendor/glfw/include"
$LIBS = "$env:VULKAN_SDK/Lib/vulkan-1.lib", "vendor/glfw/src/glfw3.lib", "-luser32", "-lshell32", "-lgdi32" 

& $CC $CFLAGS $INCLUDES -o main.exe main.c $LIBS

# If the compilation database doesn't exist, we generate it, of course
# This is of course so that clangd works properly
if (!(Test-Path -Path "compile_commands.json"))
{
    $command = @(
        [PSCustomObject]@{
            directory = "$PWD"
            arguments = @($CC) + $CFLAGS + $INCLUDES + @("-o", "main.exe", "main.c") + $LIBS
            file = "main.c"
        }
    )

    $command_json = $command | ConvertTo-Json -AsArray
    Set-Content -Path "compile_commands.json" -Value $command_json
}
