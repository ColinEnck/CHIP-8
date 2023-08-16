#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <stack>
#include <string.h>
#define LENGTH 64
#define WIDTH 32

void* check_malloc(unsigned int size);
void usage();
char* fetchPC(uint16_t &PC, uint8_t* mem, char* num);

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    usage();
    return 0;
  }
  // setting up the environment
  uint8_t* mem = (uint8_t*) check_malloc(4096-512); // emulated memory minus free space at beginning
  std::stack<uint16_t> stack; // 16-bit sized stack
  uint16_t PC; // program counter
  uint16_t I; // index register
  uint8_t SP; // stack pointer; 1 byte long
  uint8_t delay; // delay timer
  uint8_t sound; // sound timer
  int8_t registers[16]; // one-byte registers;
  char instruction[5]; // stores the instruction as a string

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

  sf::RenderWindow window(sf::VideoMode(LENGTH * 20, WIDTH * 20), "Screen");

  bool screen[32][64];
  for (int i = 0; i < 32; ++i)
    for (int j = 0; j < 64; ++j)
      screen[i][j] = false;

  sf::RectangleShape drawer(sf::Vector2f(20.f, 20.f));

  bool keys[16];
   
  while (window.isOpen())
  {
    for (int i = 0; i < 16; ++i)
      keys[i] = false;

    // handle user input here
    sf::Event event;
    while (window.pollEvent(event))
    {
      // key-press logic goes here
      if (event.type == sf::Event::Closed)
        window.close();
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        window.close();
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num0))
        keys[0] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
        keys[1] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
        keys[2] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
        keys[3] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4))
        keys[4] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5))
        keys[5] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6))
        keys[6] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num7))
        keys[7] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num8))
        keys[8] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num9))
        keys[9] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        keys[10] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::B))
        keys[11] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::C))
        keys[12] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        keys[13] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        keys[14] = true;
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
        keys[15] = true;
    }
    
    // draw stuff
    window.clear();
    for (int i = 0; i < 32; ++i)
      for (int j = 0; j < 64; ++j)
        if (screen[i][j])
        {
          drawer.setPosition(j*20, i*20);
          window.draw(drawer);
        }
    
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

char* fetchPC(uint16_t &PC, uint8_t* mem, char* num)
{
  uint8_t firstbyte = mem[PC-512];
  uint8_t secondbyte = mem[PC-512+1];
  PC += 2;
  sprintf(num, "%02X%02X", firstbyte, secondbyte);
  return num;
}

