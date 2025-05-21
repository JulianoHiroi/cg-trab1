CC = g++

GLLIBS = -lglut -lGLEW -lGL -lassimp

all: mesh.cpp ../lib/utils.cpp
	$(CC) mesh.cpp ../lib/utils.cpp -o mesh $(GLLIBS)

clean:
	rm -f mesh
