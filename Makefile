CC = g++

GLLIBS = -lglut -lGLEW -lGL -lassimp

all: mesh2.cpp ../lib/utils.cpp
	$(CC) mesh2.cpp ../lib/utils.cpp -o mesh2 $(GLLIBS)

clean:
	rm -f mesh
