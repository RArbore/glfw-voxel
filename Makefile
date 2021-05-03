##
# GLFW Voxel Engine
#
# @file
# @version 0.1

LFLAGS=-lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -lGLU -lglut
WFLAGS=-Wno-implicit-function-declaration -Wall
OUTPUT=voxel-engine.out

$(OUTPUT): objects/open-simplex-noise.o objects/glad.o objects/texture.o objects/world.o objects/main.o
	gcc $(WFLAGS) -o $(OUTPUT) objects/open-simplex-noise.o objects/glad.o objects/texture.o objects/world.o objects/main.o $(LFLAGS)
objects/open-simplex-noise.o: include/open-simplex-noise.h source/open-simplex-noise.c
	gcc $(WFLAGS) -c -o objects/open-simplex-noise.o source/open-simplex-noise.c $(LFLAGS)
objects/glad.o: include/glad/glad.h source/glad.c
	gcc $(WFLAGS) -c -o objects/glad.o source/glad.c $(LFLAGS)
objects/texture.o: include/texture.h source/texture.c
	gcc $(WFLAGS) -c -o objects/texture.o source/texture.c $(LFLAGS)
objects/world.o: include/texture.h include/constants.h include/world.h source/world.c
	gcc $(WFLAGS) -c -o objects/world.o source/world.c $(LFLAGS)
objects/main.o: include/shaders/shaders.h include/texture.h include/constants.h include/world.h source/main.c
	gcc $(WFLAGS) -c -o objects/main.o source/main.c $(LFLAGS)
clean:
	rm objects/*.o *.out

# end
