#include <stdio.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

SDL_Window* window_init() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window = SDL_CreateWindow("Meteor shooter",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        WINDOW_WIDTH,
                                        WINDOW_HEIGHT,
                                        0);

  if (!window) {
    printf("Could not create window: %s\n", SDL_GetError());
  }
  return window;
}

int main() {
  SDL_Window* window = window_init();

  // ------------------------------------------------------------------------------------

  bool quit = false;
  SDL_Event event;
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }
  }

  // ------------------------------------------------------------------------------------

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}