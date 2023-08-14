chip8: main.cpp
	g++ -c -g main.cpp 
	g++ main.o -g -o chip8 -lsfml-graphics -lsfml-window -lsfml-system
