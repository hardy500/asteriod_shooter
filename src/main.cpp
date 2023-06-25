#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include <vector>
#include <cstdlib>
#include <random>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

enum Select { TTF, IMG };

void sdl_init() {
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

      SDL_Color    color_text      = {255, 255, 255};
      SDL_Surface* surface_text = TTF_RenderText_Solid(font, "Space", color_text);
      SDL_Texture* texture_text = SDL_CreateTextureFromSurface(renderer, surface_text);
      SDL_FreeSurface(surface_text);
      return texture_text;
      break;

    }
  }
}

SDL_Rect rect_create(SDL_Texture* texture, int x=0, int y=0) {
  SDL_Rect rect = {x, y, 0, 0};
  SDL_QueryTexture(texture, NULL, NULL, &(rect.w), &(rect.h));
  return rect;
}

void update_ship(SDL_Rect* rect_ship, int x=0, int y=0) {
  SDL_GetMouseState(&x, &y);
  rect_ship->x = (x - 50);
  rect_ship->y = (y - 30);
}

void update_laser(std::vector<SDL_Rect>& lasers, float delta, int speed=300) {
    auto it = lasers.begin();
    while (it != lasers.end()) {
      it->y -= round(300 * delta);
      if (it->y < 0) {
        it = lasers.erase(it);
      } else {
        it++;
      }
    }
}

float randomf(float x1, float x2) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(x1, x2);
  return dis(gen);
}

void update_meteor(std::vector<SDL_Rect>& meteors, float delta, int speed=300) {
    auto it = meteors.begin();
    while (it != meteors.end()) {
      it->y += round(speed * delta);
      if (it->y > WINDOW_HEIGHT) {
        it = meteors.erase(it);
      } else {
        it++;
      }
    }
}


bool laser_timer(bool can_shoot, Uint32 time_shoot, int duration=500) {
  if (!can_shoot) {
    Uint32 time_current = SDL_GetTicks();
    if ((time_current - time_shoot) > duration) {
      can_shoot = true;
    }
  }
  return can_shoot;
}

int main() {
  sdl_init();

  SDL_Window* window     = window_init();
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

  // ------------------------------------------------------------------------------------

  Select img = IMG;
  Select ttf = TTF;

  SDL_Texture* texture_bg      = texture_create(window, renderer, "assets/graphics/background.png", img);
  SDL_Texture* texture_ship    = texture_create(window, renderer, "assets/graphics/ship.png", img);
  SDL_Texture* texture_meteor  = texture_create(window, renderer, "assets/graphics/meteor.png", img);
  SDL_Texture* texture_laser   = texture_create(window, renderer, "assets/graphics/laser.png", img);
  SDL_Texture* texture_text    = texture_create(window, renderer, "assets/graphics/subatomic.ttf", ttf);

  SDL_Rect rect_text  = rect_create(texture_text, (WINDOW_WIDTH/2 - 80), (WINDOW_HEIGHT-80));
  SDL_Rect rect_ship  = rect_create(texture_ship, (WINDOW_WIDTH/2 - 40), (WINDOW_HEIGHT/2));

  std::vector<SDL_Rect> lasers;
  std::vector<SDL_Rect> meteors;

  // ------------------------------------------------------------------------------------

  Uint32 ticks_previous = SDL_GetTicks();
  Uint32 ticks_current;
  Uint32 time_shoot;
  int frequency = 50;
  float delta;
  bool can_shoot = true;

  const float framerate_target = 120;

  //Uint32 frame_start;
  int frame_time;
  Uint32 frame_delay;

  bool quit = false;
  SDL_Event event;
  while (!quit) {
    ticks_current = SDL_GetTicks();
    frame_time = ticks_current - ticks_previous;
    delta = frame_time / 1000.0f;
    ticks_previous = ticks_current;
    int meteor_shower = (rand()%frequency == 1);

    // Cap fram rate
    frame_delay = 1000 / framerate_target;
    if (frame_time < frame_delay) { SDL_Delay(frame_delay - frame_time); }

    while (SDL_PollEvent(&event)) {

      if (event.type == SDL_QUIT) {
        quit = true;
      }

      if ((event.type == SDL_MOUSEBUTTONDOWN) && can_shoot) {
        SDL_Rect rect_laser = rect_create(texture_laser, (WINDOW_WIDTH/2 - 40), (WINDOW_HEIGHT/2));
        rect_laser.x = rect_ship.x + 43;
        rect_laser.y = rect_ship.y;
        lasers.push_back(rect_laser);

        can_shoot = false;
        time_shoot = SDL_GetTicks();
      }

      if (meteor_shower) {
        SDL_Rect rect_meteor = rect_create(texture_meteor,randomf(-100, WINDOW_WIDTH + 100.0f), randomf(-100, -50));
        rect_meteor.x += randomf(-0.5f, 0.5f)*delta;
        meteors.push_back(rect_meteor);
      }
    }

    update_ship(&rect_ship);
    update_laser(lasers, delta);
    update_meteor(meteors, delta);
    can_shoot = laser_timer(can_shoot, time_shoot);

    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, texture_bg, NULL, NULL);
    SDL_RenderCopy(renderer, texture_text, NULL, &rect_text);

    for (SDL_Rect laser: lasers) { SDL_RenderCopy(renderer, texture_laser, NULL, &laser); }
    for (SDL_Rect meteor: meteors) { SDL_RenderCopy(renderer, texture_meteor, NULL, &meteor); }

    SDL_RenderCopy(renderer, texture_ship, NULL, &rect_ship);

    SDL_RenderPresent(renderer);
  }
  // ------------------------------------------------------------------------------------

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}