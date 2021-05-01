##
# GLFW Voxel Engine
#
# @file
# @version 0.1

CC=gcc
CFLAGS=-lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -lGLU -lglut
WFLAGS=-Wno-implicit-function-declaration
OUTPUT=voxel-engine.out

main: src/main.c
	$(CC) $(WFLAGS) -o $(OUTPUT) src/glad.c src/texture.c src/world.c src/main.c $(CFLAGS)

# end
