# OpenGL issues

Turn on debugging output and run simulation again

```
export LIBGL_DEBUG=verbose
./simulation
```

Check OpenGL information:

```
glxinfo | grep -i OpenGL
```

Driver information:

```
lspci | grep -i "vga compatible"
```

## Faulty OpenGL version detection with software renderer

https://python.develop-bugs.com/article/10118323/paraview+needs+higher+OpenGL+in+Mesa
When using llvmpipe/gallium with mesa, a software renderer, the OpenGL capabilities can be incorrectly detected. The simplest way to fix that is to force it :

```
MESA_GL_VERSION_OVERRIDE=3.3 ./bin/paraview
```

Simulation output:

```
...
libGL error: failed to open drm device: No such file or directory
libGL error: failed to load driver: i965
libGL: OpenDriver: trying /usr/lib64/dri/tls/swrast_dri.so
libGL: OpenDriver: trying /usr/lib64/dri/swrast_dri.so
libGL: Can't open configuration file /home/testuser/.drirc: No such file or directory.
libGL: Can't open configuration file /home/testuser/.drirc: No such file or directory.
libGL error: failed to open drm device: No such file or directory
libGL error: failed to load driver: i965
libGL: OpenDriver: trying /usr/lib64/dri/tls/swrast_dri.so
libGL: OpenDriver: trying /usr/lib64/dri/swrast_dri.so
libGL: Can't open configuration file /home/testuser/.drirc: No such file or directory.
libGL: Can't open configuration file /home/testuser/.drirc: No such file or directory.
ERROR: In /home/testuser/bdm-build-third-party/paraview/VTK/Rendering/OpenGL2/vtkOpenGLRenderWindow.cxx, line 793
vtkXOpenGLRenderWindow (0x4ba2790): GL version 2.1 with the gpu_shader4 extension is not supported by your graphics driver but is required for the new OpenGL rendering backend.	 Please update your OpenGL driver. If you are using Mesa please make sure you have version 10.6.5 or later and make sure your driver in Mesa supports OpenGL 3.2.

ERROR: In /home/testuser/bdm-build-third-party/paraview/VTK/Rendering/OpenGL2/vtkShaderProgram.cxx, line 445
vtkShaderProgram (0x4b93030): 1: #version 120
...

```

`glxinfo`
```
glxinfo | grep -i OpenGL
OpenGL vendor string: VMware, Inc.
OpenGL renderer string: llvmpipe (LLVM 6.0, 256 bits)
OpenGL version string: 2.1 Mesa 18.0.5
OpenGL shading language version string: 1.30
OpenGL extensions:
OpenGL ES profile version string: OpenGL ES 2.0 Mesa 18.0.5
OpenGL ES profile shading language version string: OpenGL ES GLSL ES 1.0.16
OpenGL ES profile extensions:
```


## Mount host graphics card to container

https://stackoverflow.com/questions/42438619/run-chromium-inside-container-libgl-error

```
# for intel cards
docker run --device=/dev/dri:/dev/dri ...
```
