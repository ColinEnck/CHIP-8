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
void fetchPC(uint16_t &PC, uint8_t* mem, char* num); // result is stored in num

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
  uint16_t PC = 512; // program counter
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

    // logic starts here
    fetchPC(PC, mem, instruction);
    int X = instruction[1] - '0';
    int Y = instruction[2] - '0';
    int N = instruction[3] - '0';
    int NN = Y * 16 + N;
    int NNN = X * 256 + Y * 16 + N;
    int xpos = registers[X] & 63;
    int ypos = registers[Y] & 31;
    uint8_t spriteline;

    switch (instruction[0]) {
      case '0':
        if (instruction[2] == 'E' & instruction[3] == '0')
          for (int i = 0; i < 32; ++i)
            for (int j = 0; j < 64; ++j)
              screen[i][j] = false;
        break;
      case '1':
        PC = NNN;
        break;
      case '2':
        break;
      case '3':
        break;
      case '4':
        break;
      case '5':
        break;
      case '6':
        registers[X] = NN;
        break;
      case '7':
        registers[X] += NN;
        break;
      case '8':
        break;
      case '9':
        break;
      case 'A':
        I = NNN;
        break;
      case 'B':
        break;
      case 'C':
        break;
      case 'D':
        registers[0xF] = 0;
        for (int y = 0; y < N; ++y)
        {
          if (ypos+y >= 32) break;
          spriteline = mem[I+y-512];
          for (int x = 0; x < 8; ++x)
          {
            if (xpos+x >= 64) break;
            if ((spriteline >> (7-x)) & 1) // finds the jth bit of the sprite line
            {
              if (screen[ypos+y][xpos+x])
                registers[0xF] = 1;
              screen[ypos+y][xpos+x] = !(screen[ypos+y][xpos+x]);
            }
          }
        }
        break;
      case 'E':
        break;
      case 'F':
        break;
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

void fetchPC(uint16_t &PC, uint8_t* mem, char* num)
{
  uint8_t firstbyte = mem[PC-512];
  uint8_t secondbyte = mem[PC-512+1];
  PC += 2;
  sprintf(num, "%02X%02X", firstbyte, secondbyte);
}
