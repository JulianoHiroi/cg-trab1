CC = g++

GLLIBS = -lglut -lGLEW -lGL

all: triangle.cpp primitives.cpp rectangle.cpp rectangle2.cpp 
	$(CC) triangle.cpp -o triangle $(GLLIBS)
	$(CC) primitives.cpp ../lib/utils.cpp -o primitives $(GLLIBS)
	$(CC) rectangle.cpp ../lib/utils.cpp -o rectangle $(GLLIBS)
	$(CC) rectangle2.cpp ../lib/utils.cpp -o rectangle2 $(GLLIBS)

clean:
	rm -f triangle primitives rectangle rectangle2
