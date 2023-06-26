#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <math.h>
#include <vector>
#include <cstdlib>
#include <random>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

void sdl_init() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Init(SDL_INIT_AUDIO);
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
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

SDL_Texture* texture_create(SDL_Window* window, SDL_Renderer* renderer, const char* path) {
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
      it->y -= round(speed * delta);
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

int collision_meteor_ship(std::vector<SDL_Rect>& rect1, SDL_Rect& rect2) {
  for (auto r1 : rect1) {
    if (SDL_HasIntersection(&r1, &rect2)) {
      return 1;
    }
  }
  return 0;
}

int play_wav_sound(Mix_Chunk* sound, int loop=-1) {
  if (!sound) {
    printf("FAILED TO LOAD WAVE FILE: %s", Mix_GetError());
    return 1;
  }

  int channel = Mix_PlayChannel(-1, sound, loop);
  if (channel == -1) {
    printf("FAILED TO PLAY WAVE FILE: %s\n", Mix_GetError());
    return 1;
  }

  Mix_Playing(channel);
  return 0;
}

int play_ogg_sound(Mix_Music* sound) {
  if (!sound) {
    printf("FAILED TO LOAD WAVE FILE: %s", Mix_GetError());
    return 1;
  }

  if (Mix_PlayMusic(sound, 0) < 0) {
    printf("FAILED TO PLAY MUSIC: %s", Mix_GetError());
  }

  Mix_PlayingMusic();
  return 0;
}

void collision_meteor_laser(std::vector<SDL_Rect>& rect1, std::vector<SDL_Rect>& rect2, Mix_Chunk* sound) {
  for (auto rect1_iter = rect1.begin(); rect1_iter != rect1.end();) {
   bool collide = false;
   for (auto rect2_iter = rect2.begin(); rect2_iter != rect2.end();) {
     if (SDL_HasIntersection(&(*rect1_iter), &(*rect2_iter))) {
       rect2_iter = rect2.erase(rect2_iter);
       collide = true;
       play_wav_sound(sound, 0);
     } else {
       rect2_iter++;
     }
   }

   if (collide) {
     rect1_iter = rect1.erase(rect1_iter);
   } else {
     rect1_iter++;
   }
  }
}

void game_over(SDL_Window* window, SDL_Renderer* renderer) {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void destroy_sound(Mix_Chunk* sound, Mix_Music* music) {
  Mix_FreeChunk(sound);
  Mix_FreeMusic(music);
  Mix_CloseAudio();
}

typedef struct {
  const char* bg;
  const char* ship;
  const char* meteor;
  const char* laser;
  const char* text;

  const char* sound_bg;
  const char* sound_explosion;
  const char* sound_laser;
} Path;

void display_score(SDL_Renderer* renderer, TTF_Font* font) {
  SDL_Surface* surface = TTF_RenderText_Solid(font, ("Score: " + std::to_string(SDL_GetTicks() / 1000)).c_str(), { 255, 255, 255 });
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_Rect rect_text = { (WINDOW_WIDTH / 2) - (surface->w / 2), (WINDOW_HEIGHT - 80), surface->w, surface->h };

  SDL_RenderCopy(renderer, texture, NULL, &rect_text);

  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
}


int main() {
  sdl_init();
  SDL_Window* window     = window_init();
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

  // ------------------------------------------------------------------------------------

  Path path;
  path = {
    .bg="assets/graphics/background.png",
    .ship="assets/graphics/ship.png",
    .meteor="assets/graphics/meteor.png",
    .laser="assets/graphics/laser.png",
    .text="assets/graphics/subatomic.ttf",
    .sound_bg="assets/sound/music.wav",
    .sound_explosion="assets/sound/explosion.wav",
    .sound_laser="assets/sound/laser.ogg"
  };

  SDL_Texture* texture_bg      = texture_create(window, renderer, path.bg);
  SDL_Texture* texture_ship    = texture_create(window, renderer, path.ship);
  SDL_Texture* texture_meteor  = texture_create(window, renderer, path.meteor);
  SDL_Texture* texture_laser   = texture_create(window, renderer, path.laser);
  TTF_Font* font = TTF_OpenFont(path.text, 50);

  SDL_Rect rect_ship  = rect_create(texture_ship, (WINDOW_WIDTH/2 - 40), (WINDOW_HEIGHT/2));

  Mix_Chunk* sound_bg = Mix_LoadWAV(path.sound_bg);
  Mix_Chunk* sound_explosion = Mix_LoadWAV(path.sound_explosion);
  Mix_Music* sound_laser = Mix_LoadMUS(path.sound_laser);
  play_wav_sound(sound_bg, -1);

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
        play_ogg_sound(sound_laser);
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

    collision_meteor_laser(meteors, lasers, sound_explosion);
    if (collision_meteor_ship(meteors, rect_ship)) {
      game_over(window, renderer);
    }

    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, texture_bg, NULL, NULL);
    display_score(renderer, font);

    for (SDL_Rect laser: lasers) { SDL_RenderCopy(renderer, texture_laser, NULL, &laser); }
    for (SDL_Rect meteor: meteors) { SDL_RenderCopy(renderer, texture_meteor, NULL, &meteor); }

    SDL_RenderCopy(renderer, texture_ship, NULL, &rect_ship);

    SDL_RenderPresent(renderer);
  }
  // ------------------------------------------------------------------------------------

  destroy_sound(sound_bg, sound_laser);
  game_over(window, renderer);

  return 0;
}
