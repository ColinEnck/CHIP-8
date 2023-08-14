#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#define LENGTH 64
#define WIDTH 32

void* check_malloc(unsigned int size);
void usage();
char* fetch(int &PC, uint16_t mem[]);

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    usage();
    return 0;
  }
  // setting up the environment
  uint16_t* mem = (uint16_t*) check_malloc(4096-512); // emulated memory minus free space at beginning
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
  bool screen[32][64];
  for (int i = 0; i < 32; ++i)
    for (int j = 0; j < 64; ++j)
      screen[i][j] = false;

  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      // key-press logic goes here
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear();
    window.display();
  }

  return 0;
}

void* check_malloc(unsigned int size)
{
  void* ptr = malloc(size);
  if (ptr == NULL)
  {
    perror("malloc failure");
    exit(1);
  }

  return ptr;
}

void usage()
{
  printf("Please specify the path of the chip-8 file as a command line argument.\n");
  printf("USAGE:\n");
  printf("./chip8 [FILENAME]\n");
}

char* fetch(int &PC, uint16_t mem[])
{
  char* instruction = (char*) check_malloc(4);
  int num = mem[(PC - 512)/2]; // offset for blank memory and uint16_t length
  sprintf(instruction, "%04X", num);
  PC += 2;
  return instruction;
}
