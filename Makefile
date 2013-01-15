
CC=g++

scenario: scenario.cpp
	$(CC) -o scenario scenario.cpp tga.cpp -O2 -lglut -lGL -lGLU
