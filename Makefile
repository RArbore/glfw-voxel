##
# GLFW Voxel Engine
#
# @file
# @version 0.1

CC=gcc
CFLAGS=-lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -lGLU -lglut
WFLAGS=-Wno-implicit-function-declaration
OUTPUT=voxel-engine.out

main: source/main.c
	$(CC) $(WFLAGS) -o $(OUTPUT) source/glad.c source/texture.c source/world.c source/main.c $(CFLAGS)

# end
