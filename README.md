# Simple Vulkan Triangle

## How to Build?

On Linux, ensure that the Vulkan development files (on CachyOS they are known 
as `vulkan-devel` - check with your distro to see what you need to install)
along with GLFW's development files (some distros such as CachyOS bundles the 
development files with the library package directly so again check with your 
distro). Ensure that the Vulkan libraries and glslc are installed as well. Then, 
it is as simple as:

```bash
make all
```

On other systems, I have no idea.

## How to Run?

On Linux, after you build the project it should produce an executable named 
`main` which you can run directly.

```bash
./main
```

If you wish to run with validation layers, you can do so easily, assuming they
are installed.

```bash
VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation ./main
```

As for other systems, I have no idea.
