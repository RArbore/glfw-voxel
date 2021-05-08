# GLFW-Voxel

This is a voxel rendering engine I've made using C, GLFW, and OpenGL. It loads and unloads chunks as you traverse the world. Chunks are meshed only when loaded or a neighboring chunk is updated (currently only happens if neighboring chunk loads). Chunk meshing is multi-threaded and culls occluded faces - that is, if a block face is next to another block, it will not be added to the mesh.

I've used frustum culling to improve rendering performance. The shaders are relatively simple - they have support for ambient, diffused, and specular lighting. Much of the OpenGL is based off of [LearnOpenGL](https://www.learnopengl.com).

To build, just run `make`. There are dependencies related to OpenGL, GLFW, GLUT, CGLM. I've tested on Ubuntu Linux; I know it currently doesn't work on MacOS (I've tried simply running with `make` as is, but MacOS does something weird with OpenGL).

This project also includes a Python script to convert `.png` files into `.c` and `.h` files - I use these to create the texture arrays I use in the project as an alternative to loading the images at runtime. For this project, it's used to convert a texture atlas.
