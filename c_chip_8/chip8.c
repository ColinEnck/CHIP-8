#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 640
#define WIDTH 64
#define HEIGHT 320

bool init();
bool loadMedia();
void closeStuff();

// rendering window
SDL_Window* gWindow = NULL;
// surface contained by the window
SDL_Surface* gScreenSurface = NULL;
// image to be shown on screen
SDL_Surface* gHelloWorld = NULL;

int main(int argc, char *argv[])
{
  // start up SDL and create window
  if (!init())
    printf("Failed to initialize!\n");
  else 
  {
    // load media
    if (!loadMedia())
      printf("Failed to load media!\n");
    else
    {
      // apply the image
      SDL_BlitSurface(gHelloWorld, NULL, gScreenSurface, NULL);
      // update the surface
      SDL_UpdateWindowSurface(gWindow);
      // hack to get window to stay up
      SDL_Event e; bool quit = false; while (quit == false) { while (SDL_PollEvent(&e)) { if (e.type == SDL_QUIT) quit = true; } }
    }
  }
  closeStuff();

  return 0;
}

bool init() 
{
  bool success = true;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("SDL could not initialize! SDL_Error %s\n", SDL_GetError());
    success = false;
  }
  else 
  {
    // create window
    gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL)
    {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
      success = false;
    }
    else
    {
      // get window surface
      gScreenSurface = SDL_GetWindowSurface(gWindow);
    }
  }
  return success;
}

bool loadMedia()
{
  // loading success flag
  bool success = true;

  // load splash image
  gHelloWorld = SDL_LoadBMP("preview.bmp");
  if (gHelloWorld == NULL)
  {
    printf("Unable to load image %s! SDL_Error: %s\n", "preview.bmp", SDL_GetError());
    success = false;
  }
  return success;
}

void closeStuff()
{
  // deallocate surface
  SDL_FreeSurface(gHelloWorld);
  gHelloWorld = NULL;

  // destroy window
  SDL_DestroyWindow(gWindow);

  // quit SDL subsystem
  SDL_Quit();
}
