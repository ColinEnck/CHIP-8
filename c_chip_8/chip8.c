#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 640

int init();
void closeStuff();
void error(char err[]);
void failed(char err[]);
void pixelChange(int screen[], int x, int y, unsigned char *regs);
void updateScreen(int screen[]);

typedef union
{
    struct
    {
        unsigned int fourth : 4;
        unsigned int third : 4;
        unsigned int second : 4;
        unsigned int first : 4;
    } nibbles;
    struct
    {
        unsigned char second;
        unsigned char first;
    } value;
} Instruction;

typedef union
{
    struct
    {
        unsigned char eigth : 1;
        unsigned char seventh : 1;
        unsigned char sixth : 1;
        unsigned char fifth : 1;
        unsigned char fourth : 1;
        unsigned char third : 1;
        unsigned char second : 1;
        unsigned char first : 1;
    } bits;
    unsigned char value;
} Sprite;

// rendering window
SDL_Window *gWindow = NULL;
// surface contained by the window
SDL_Surface *gScreenSurface = NULL;

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("USAGE: %s [FILENAME]\n", argv[0]);
        return 0;
    }

    // the available memory (4kb)
    unsigned char *memory = (char *)malloc(4096);
    if (memory == NULL)
    {
        printf("'memory' failed allocation\n");
        return 1;
    }
    bzero(memory, 4096);

    // opens file
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        return 0;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(file_size);

    // Read the entire file into the buffer
    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != file_size)
    {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return 0;
    }

    // put code into memory
    memcpy(memory + 0x200, buffer, read_size);

    // represents the 64x32 screen
    int *screen = (int *)malloc(sizeof(int) * 64 * 32);
    if (screen == NULL)
    {
        printf("'screen' failed allocation\n");
        return 1;
    }
    bzero(screen, sizeof(int) * 64 * 32);

    // program counter
    unsigned char *pc = &memory[0x200];

    // index register, holds the index of a memory location
    unsigned int index = 0;

    // stack array; 64 entries for good measure
    unsigned int stack[64];
    bzero(stack, sizeof(int)*64);
    // unsigned char** sp = stack[0];
    unsigned int sp = 0;

    // delay timer
    unsigned char delay = 0;

    // sound timer
    unsigned char sound = 0;

    // 16 1-byte registers
    unsigned char *regs = (char *)malloc(16);
    if (regs == NULL)
    {
        printf("'regs' failed to allocate\n");
        return 1;
    }
    bzero(regs, 16);

    // keyboard array
    int *keyboard = (int *)malloc(sizeof(int) * 16);
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
    memcpy(memory + 0x50, font, 80);

    Instruction instruction;
    // whole instruction
    unsigned short whole;
    unsigned char x, y, n, nn;
    unsigned short nnn;

    // timer stuff
    struct timespec start, end;
    unsigned long long elapsed = 0;

    // misc.
    int result, temp;

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
                    case SDLK_1:
                        keyboard[0] = 1;
                        break;
                    case SDLK_2:
                        keyboard[1] = 1;
                        break;
                    case SDLK_3:
                        keyboard[2] = 1;
                        break;
                    case SDLK_4:
                        keyboard[3] = 1;
                        break;
                    case SDLK_q:
                        keyboard[4] = 1;
                        break;
                    case SDLK_w:
                        keyboard[5] = 1;
                        break;
                    case SDLK_e:
                        keyboard[6] = 1;
                        break;
                    case SDLK_r:
                        keyboard[7] = 1;
                        break;
                    case SDLK_a:
                        keyboard[8] = 1;
                        break;
                    case SDLK_s:
                        keyboard[9] = 1;
                        break;
                    case SDLK_d:
                        keyboard[10] = 1;
                        break;
                    case SDLK_f:
                        keyboard[11] = 1;
                        break;
                    case SDLK_z:
                        keyboard[12] = 1;
                        break;
                    case SDLK_x:
                        keyboard[13] = 1;
                        break;
                    case SDLK_c:
                        keyboard[14] = 1;
                        break;
                    case SDLK_v:
                        keyboard[15] = 1;
                        break;
                    default:
                        break;
                    }
            }

            clock_gettime(CLOCK_MONOTONIC, &start);

            // fetch-decode-execute steps are here

            instruction.value.first = *pc;
            instruction.value.second = *(pc + 1);
            whole = instruction.value.first * 16 * 16 + instruction.value.second;
            pc += 2;
            x = instruction.nibbles.second;
            y = instruction.nibbles.third;
            n = instruction.nibbles.fourth;
            nn = y * 16 + n;
            nnn = x * 16 * 16 + nn;

            switch (instruction.nibbles.first)
            {
            case 0x0:
                if (whole == 0x00E0)
                    for (int i = 0; i < 64 * 32; ++i)
                        screen[i] = 0;
                else if (whole == 0x00EE)
                {
                    pc = &memory[stack[sp]];
                    stack[sp] = 0;
                    sp--;
                }
                break;
            case 0x1:
                pc = &memory[nnn];
                break;
            case 0x2:
                sp++;
                stack[sp] = pc - memory;
                pc = &memory[nnn];
                break;
            case 0x3:
                if (regs[x] == nn)
                    pc += 2;
                break;
            case 0x4:
                if (regs[x] != nn)
                    pc += 2;
            case 0x5:
                if (regs[x] == regs[y])
                    pc += 2;
                break;
            case 0x6:
                regs[x] = nn;
                break;
            case 0x7:
                regs[x] += nn;
                break;
            case 0x8:
                switch (instruction.nibbles.fourth){
                    case 0x0:
                        regs[x] = regs[y];
                        break;
                    case 0x1:
                        regs[x] |= regs[y];
                        break;
                    case 0x2:
                        regs[x] &= regs[y];
                        break;
                    case 0x3:
                        regs[x] ^= regs[y];
                        break;
                    case 0x4:
                        result = (int) regs[x] + (int) regs[y];
                        if (result > 255)
                        {
                            regs[x] = (unsigned char) result;
                            regs[0xF] = 1;
                        }
                        else 
                        {
                            regs[x] = (unsigned char) result & 0xFF;
                            regs[0xF] = 0;
                        }
                        break;
                    case 0x5:
                        result = (int) regs[x] - (int) regs[y];
                        if (result >= 0)
                        {
                            regs[x] = result;
                            regs[0xF] = 1;
                        }
                        else 
                        {
                            regs[x] = (unsigned char) result;
                            regs[0xF] = 0;
                        }
                        break;
                    case 0x6:
                        temp = regs[x] & 0x01;
                        regs[x] /= 2;
                        regs[0xF] = temp;
                        break;
                    case 0x7:
                        result = (int) regs[y] - (int) regs[x];
                        if (result >= 0)
                        {
                            regs[x] = result;
                            regs[0xF] = 1;
                        }
                        else 
                        {
                            regs[x] = (unsigned char) result;
                            regs[0xF] = 0;
                        }
                        break;
                    case 0xE:
                        temp = regs[x] >> 7;
                        regs[x] *= 2;
                        regs[0xF] = temp;
                        break;
                    default:
                        break;
                }
                break;
            case 0x9:
                if (regs[x] != regs[y])
                    pc += 2;
                break;
            case 0xA:
                index = nnn;
                break;
            case 0xB:
                pc = &memory[nnn];
                break;
            case 0xC:
                regs[x] = rand() & nn;
                break;
            case 0xD:
                unsigned int xcords = regs[x] % 64;
                unsigned int ycords = regs[y] % 32;
                regs[0xF] = 0;
                Sprite sprite;
                for (int i = 0; i < n; ++i)
                {
                    sprite.value = memory[index+i];
                    if (sprite.bits.first)
                        pixelChange(screen, xcords, (ycords + i) % 32, regs);
                    if (sprite.bits.second)
                        pixelChange(screen, (xcords + 1) % 64, (ycords + i) % 32, regs);
                    if (sprite.bits.third)
                        pixelChange(screen, (xcords + 2) % 64, (ycords + i) % 32, regs);
                    if (sprite.bits.fourth)
                        pixelChange(screen, (xcords + 3) % 64, (ycords + i) % 32, regs);
                    if (sprite.bits.fifth)
                        pixelChange(screen, (xcords + 4) % 64, (ycords + i) % 32, regs);
                    if (sprite.bits.sixth)
                        pixelChange(screen, (xcords + 5) % 64, (ycords + i) % 32, regs);
                    if (sprite.bits.seventh)
                        pixelChange(screen, (xcords + 6) % 64, (ycords + i) % 32, regs);
                    if (sprite.bits.eigth)
                        pixelChange(screen, (xcords + 7) % 64, (ycords + i) % 32, regs);
                }
                break;
            case 0xE:
                switch (instruction.nibbles.fourth)
                {
                case 0xE:
                    if (keyboard[regs[x]] == 1)
                        pc += 2;
                    break;
                case 0x1:
                    if (keyboard[regs[x]] != 1)
                        pc += 2;
                    break;
                default:
                    break;
                }
                break;
            case 0xF:
                switch (instruction.nibbles.fourth)
                {
                case 0x3:
                    int hundreds, tens, ones;
                    hundreds = regs[x] / 100;
                    tens = (regs[x] / 10) % 10;
                    ones = regs[x] % 10;
                    memory[index] = hundreds;
                    memory[index+1] = tens;
                    memory[index+2] = ones;
                    break;
                case 0x5:
                    switch (instruction.nibbles.third)
                    {
                    case 0x1:
                        delay = regs[x];
                        break;
                    case 0x5:
                        for (int i = 0; i <= x; ++i)
                            memory[index+i] = regs[i];
                        break;
                    case 0x6:
                        for (int i = 0; i <= x; ++i)
                            regs[i] = memory[index+i];
                        break;
                    default:
                        break;
                    }
                    break;
                case 0x7:
                    regs[x] = delay;
                    break;
                case 0x8:
                    sound = regs[x];
                    break;
                case 0x9:
                    index = 0x50+(regs[x]*5);
                    break;
                case 0xA:
                    int pressed = 0;
                    for (int i = 0; i < 16; ++i)
                        if (keyboard[i])
                        {
                            pressed = 1;
                            break;
                        }
                    if (pressed == 0)
                        pc -= 2;
                    break;
                case 0xE:
                    index += regs[x];
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }

            SDL_FillRect(gScreenSurface, NULL, SDL_MapRGBA(gScreenSurface->format, 0x00, 0x00, 0x00, 0x00));
            updateScreen(screen);
            SDL_UpdateWindowSurface(gWindow);

            clock_gettime(CLOCK_MONOTONIC, &end);
            elapsed += end.tv_nsec - start.tv_nsec;
            if (elapsed >= (1000000000 / 60))
            {
                elapsed = 0;
                if (delay > 0) delay -= 1;
                if (sound > 0) sound -= 1;
            }
        }
    }
    
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

void pixelChange(int screen[], int x, int y, unsigned char *regs)
{
    if (screen[y * 64 + x] == 0)
        screen[y * 64 + x] = 1;
    else
    {
        screen[y * 64 + x] = 0;
        regs[0xF] = 1;
    }
}

void updateScreen(int screen[])
{
    SDL_Rect pixel;
    pixel.h = 20;
    pixel.w = 20;
    for (int i = 0; i < 64 * 32; i++)
    {
        pixel.x = (i % 64) * 20;
        pixel.y = (i / 64) * 20;
        if (screen[i])
            SDL_FillRect(gScreenSurface, &pixel, SDL_MapRGB(gScreenSurface->format, 0xFF, 0xFF, 0xFF));
    }
}