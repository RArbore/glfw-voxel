##
# GLFW Voxel Engine
#
# @file
# @version 0.1

CC=gcc
CFLAGS=-lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
OUTPUT=voxel-engine.out

main: src/main.c
	$(CC) -o $(OUTPUT) src/main.c src/glad.c $(CFLAGS)

# end
