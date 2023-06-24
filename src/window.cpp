#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define FPS 120
#define FRAME_DELAY  (1000/FPS)

enum Select { TTF, IMG };

void init() {
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();
}

SDL_Window* window_init() {
  SDL_Window* window = SDL_CreateWindow("Meteor shooter",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        WINDOW_WIDTH,
                                        WINDOW_HEIGHT,
                                        0);

  if (!window) { printf("Could not create window: %s\n", SDL_GetError()); }
  return window;
}

SDL_Texture* texture_create(SDL_Window* window,
                            SDL_Renderer* renderer,
                            const char* path,
                            Select select) {
  // Load image
  switch (select) {
    case IMG: {
      SDL_Surface* image = IMG_Load(path);
      if(!image) {
        printf("Image not found\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
      }

      // Create texture from loaded image
      SDL_Texture* texture_img = SDL_CreateTextureFromSurface(renderer, image);
      SDL_FreeSurface(image);
      return texture_img;
      break;
    }

    case TTF: {
      // Create font
      TTF_Font* font = TTF_OpenFont(path, 50);
      if (!font) {
        printf("NOT FONT FOUND\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
      }

      SDL_Color color_text = {255, 255, 255};
      SDL_Surface* surface_text = TTF_RenderText_Solid(font, "Space", color_text);
      SDL_Texture* texture_text = SDL_CreateTextureFromSurface(renderer, surface_text);
      SDL_FreeSurface(surface_text);
      return texture_text;
      break;

    }
  }
}

SDL_Rect rect_create(SDL_Texture* texture, int x, int y) {
  SDL_Rect rect = {x, y, 0, 0};
  SDL_QueryTexture(texture, NULL, NULL, &(rect.w), &(rect.h));
  return rect;
}

int main() {
  init();
  SDL_Window* window = window_init();
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

  // ------------------------------------------------------------------------------------
  Select img = IMG;
  Select ttf = TTF;

  SDL_Texture* texture_bg = texture_create(window, renderer, "assets/graphics/background.png", img);
  SDL_Texture* texture_ship = texture_create(window, renderer, "assets/graphics/ship.png", img);
  SDL_Texture* texture_text = texture_create(window, renderer, "assets/graphics/subatomic.ttf", ttf);

  SDL_Rect rect_text = rect_create(texture_text, (WINDOW_WIDTH/2 - 80), (WINDOW_HEIGHT-80));
  SDL_Rect rect_ship = rect_create(texture_ship, (WINDOW_WIDTH/2 - 40), (WINDOW_HEIGHT/2));

  // ------------------------------------------------------------------------------------

  Uint32 frame_start;
  int frame_time;

  bool quit = false;
  SDL_Event event;
  while (!quit) {
    frame_start = SDL_GetTicks();

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }


    if (rect_ship.y > 0) rect_ship.y -= 4;

    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, texture_bg, NULL, NULL);
    SDL_RenderCopy(renderer, texture_ship, NULL, &rect_ship);
    SDL_RenderCopy(renderer, texture_text, NULL, &rect_text);

    SDL_RenderPresent(renderer);

    frame_time = SDL_GetTicks() - frame_start;
    if (frame_time < FRAME_DELAY) {SDL_Delay(FRAME_DELAY - frame_time);}

  }
  // ------------------------------------------------------------------------------------

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}