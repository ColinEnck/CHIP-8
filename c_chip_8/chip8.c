#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 640

int init();
void closeStuff();
void error(char err[]);
void failed(char err[]);
void pixelChange(int screen[], int x, int y, unsigned char* regs);
void updateScreen(int screen[]);

// I have to reverse them
// I forget why
// has something to do with little-endian stucture, I think
typedef union {
    struct {
        unsigned int first:4;
        unsigned int second:4;
        unsigned int third:4;
        unsigned int fourth:4;
    } nibbles;
    unsigned short value;
} Instruction;

typedef union {
    struct {
        unsigned char eigth:1;
        unsigned char seventh:1;
        unsigned char sixth:1;
        unsigned char fifth:1;
        unsigned char fourth:1;
        unsigned char third:1;
        unsigned char second:1;
        unsigned char first:1;
    } bits;
    unsigned char value;
} Sprite;

// rendering window
SDL_Window* gWindow = NULL;
// surface contained by the window
SDL_Surface* gScreenSurface = NULL;

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("USAGE: %s [FILENAME]\n", argv[0]);
        return 0;
    }

    // the available memory (4kb)
    unsigned char* memory = (char*) malloc(4096);
    if (memory == NULL)
    {
        printf("'memory' failed allocation\n");
        return 1;
    }
    bzero(memory, 4096);

    // opens file
    FILE* file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror("Error opening file");
        return 0;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* buffer = (char*) malloc(file_size);

    // Read the entire file into the buffer
    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != file_size) {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return 0;
    }

    // put code into memory
    memcpy(memory+0x200, buffer, read_size);

    // represents the 64x32 screen
    int* screen = (int*) malloc(sizeof(int)*64*32);
    if (screen == NULL)
    {
        printf("'screen' failed allocation\n");
        return 1;
    }
    bzero(screen, sizeof(int)*64*32);

    // program counter
    unsigned short* pc = (unsigned short*) &memory[0x200];

    // index register
    unsigned short* index = NULL;

    // stack array; 64 entries for good measure
    Stack stack;
    stack.array = (unsigned short*) malloc(sizeof(short) * 64);
    if (stack.array == NULL)
    {
        printf("'stack.array' failed allocation\n");
        return 1;
    }
    bzero(stack.array, 64);
    stack.size = 0;

    // delay timer
    unsigned char delay = 0;

    // sound timer 
    unsigned char sound = 0;

    // 16 1-byte registers
    unsigned char* regs = (char*) malloc(16);
    if (regs == NULL)
    {
        printf("'regs' failed to allocate\n");
        return 1;
    }
    bzero(regs, 16);

    // keyboard array
    int* keyboard = (int*) malloc(sizeof(int)*16);
    bzero(keyboard, 16);

    // font
    char font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    memcpy(memory+0x50, font, 80);

    Instruction instruction;
    unsigned char x, y, n, nn;
    unsigned short nnn;

    if (!init())
        failed("initialize");
    else
    {
        int quit = 0;
        SDL_Event e;
        while (!quit)
        {
            while (SDL_PollEvent(&e) != 0)
            {
                // reest the keyboard
                for (int i = 0; i < 16; i++)
                    keyboard[i] = 0;
                if (e.type == SDL_QUIT)
                    quit = 1;
                else if (e.type == SDL_KEYDOWN)
                    switch (e.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        quit = 1;
                        break;
                    case SDLK_0:
	                    keyboard[0] = 1;
	                    break;
                    case SDLK_1:
	                    keyboard[1] = 1;
		                break;
                    case SDLK_2:
	                    keyboard[2] = 1;
	                    break;
                    case SDLK_3:
	                    keyboard[3] = 1;
	                    break;
                    case SDLK_4:
	                    keyboard[4] = 1;
	                    break;
                    case SDLK_5:
	                    keyboard[5] = 1;
	                    break;
                    case SDLK_6:
	                    keyboard[6] = 1;
                        break;
                    case SDLK_7:
	                    keyboard[7] = 1;
	                    break;
                    case SDLK_8:
	                    keyboard[8] = 1;
	                    break;
                    case SDLK_9:
	                    keyboard[9] = 1;
	                    break;
                    case SDLK_a:
	                    keyboard[10] = 1;
	                    break;
                    case SDLK_b:
	                    keyboard[11] = 1;
	                    break;
                    case SDLK_c:
	                    keyboard[12] = 1;
	                    break;
                    case SDLK_d:
	                    keyboard[13] = 1;
	                    break;
                    case SDLK_e:
	                    keyboard[14] = 1;
	                    break;
                    case SDLK_f:
	                    keyboard[15] = 1;
	                    break;
                    default:
                        break;
                    }

                // fetch-decode-execute steps are here

                instruction.value = *pc;
                pc++;
                // test
                instruction.value = 0xa3e7;
                // end test
                x = instruction.nibbles.second;
                y = instruction.nibbles.third;
                n = instruction.nibbles.fourth;
                nn = y * 16 + n;
                nnn = x * 16 * 16 + nn;

                switch (instruction.nibbles.first)
                {
                case 0x0:
                    if (instruction.value == 0x00E0)
                        for (int i = 0; i < 64*32; ++i)
                            screen[i] = 0;
                    break;
                case 0x1:
                    pc = &memory[nnn];
                    break;
                case 0x6:
                    regs[x] = nn;
                    break;
                case 0x7:
                    regs[x] += nn;
                    break;
                case 0xA:
                    index = &memory[nnn];
                    break;
                case 0xD:
                    unsigned int xcords = regs[x] % 64;
                    unsigned int ycords =  regs[y] % 32;
                    regs[0xF] = 0;
                    Sprite sprite;
                    for (int i = 0; i < n; ++i)
                    {
                        sprite.value = *(index + i);
                        if (sprite.bits.first)
                            pixelChange(screen, xcords, ycords+i, regs);
                        if (sprite.bits.second)
                            pixelChange(screen, xcords+1, ycords+i, regs);
                        if (sprite.bits.third)
                            pixelChange(screen, xcords+2, ycords+i, regs);
                        if (sprite.bits.fourth)
                            pixelChange(screen, xcords+3, ycords+i, regs);
                        if (sprite.bits.fifth)
                            pixelChange(screen, xcords+4, ycords+i, regs);
                        if (sprite.bits.sixth)
                            pixelChange(screen, xcords+5, ycords+i, regs);
                        if (sprite.bits.seventh)
                            pixelChange(screen, xcords+6, ycords+i, regs);
                        if (sprite.bits.eigth)
                            pixelChange(screen, xcords+7, ycords+i, regs);
                    }
                    break;
                default:
                    break;
                }

                SDL_FillRect(gScreenSurface, NULL, SDL_MapRGBA(gScreenSurface->format, 0x00, 0x00, 0x00, 0x00));
                updateScreen(screen);
                SDL_UpdateWindowSurface(gWindow);
            }
        }
    }

    // free(memory);
    // free(pc);
    // free(index);
    // fclose(file);
    // free(buffer);

    return 0;
}

int init() 
{
    int success = 1;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        error("initialize");
        success = 0;
    }
    else
    {
        gWindow = SDL_CreateWindow("CHIP-8!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            error("create window");
            success = 0;
        }
        else 
        {
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }
    }
    return success;
}

void closeStuff()
{
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}

void error(char err[])
{
    printf("SDL could not %s! SDL_Error: %s\n", err, SDL_GetError());
}

void failed(char err[])
{
    printf("Failed to %s! SDL_Error: %s\n", err, SDL_GetError());
}

void pixelChange(int screen[], int x, int y, unsigned char* regs)
{
    if (screen[y*64 + x] == 0)
        screen[y*64 + x] = 1;
    else 
    {
        screen[y*64 + x] = 0;
        regs[0xF] = 1;
    }
}

void updateScreen(int screen[])
{
    SDL_Rect pixel;
    pixel.h = 20;
    pixel.w = 20;
    for (int i = 0; i < 64*32; i++)
    {
        pixel.x = (i % 64) * 20;
        pixel.y = (i / 64) * 20;
        if (screen[i])
            SDL_FillRect(gScreenSurface, &pixel, SDL_MapRGB(gScreenSurface->format, 0xFF, 0xFF, 0xFF));
    }
}