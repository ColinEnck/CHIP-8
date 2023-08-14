#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include "funcs.h"
#define LENGTH 64
#define WIDTH 32

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    usage();
    return 0;
  }
  // setting up the environment
  uint16_t* mem = (uint16_t*) check_malloc(4096 - 512); // emulated memory minus free space at beginning
  int PC; // program counter
  int I; // index register
  char SP; // stack pointer; 1 byte long
  int delay; // delay timer
  int sound; // sound timer
  char registers[16]; // one-byte register  

  // reading from file
  FILE *fptr;
  long size;
  fptr = fopen(argv[1], "rb");
  if (fptr == NULL)
  {
    perror("Error opening file");
    return 1;
  }
  // find length of file
  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  fseek(fptr, 0, SEEK_SET);
  // read from file and close it
  fread(mem, 1, size, fptr);
  fclose(fptr);

  for (int i = 0; i < size/2; ++i)
    printf("%04X\n", mem[i]);

  sf::RenderWindow window(sf::VideoMode(LENGTH * 20, WIDTH * 20), "Screen");

  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear();
    window.display();
  }

  return 0;
}
