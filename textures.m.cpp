#include <SDL.h>
#include <array>
#include <iostream>

void draw_main_window(SDL_Renderer *renderer, SDL_Texture *shared_texture) {
  SDL_SetRenderTarget(renderer, NULL);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_Rect src = {3, 3, 100, 100};
  SDL_Rect dst = {100, 100, 100, 100};
  SDL_RenderCopy(renderer, shared_texture, &src, &dst);

  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
  std::array<SDL_Rect, 2> rects = {{{110, 110, 50, 50}, {10, 10, 10, 10}}};
  SDL_RenderDrawRects(renderer, rects.data(), rects.size());

  SDL_RenderPresent(renderer);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
  std::cout << "Hello World" << std::endl;

  if (0 != SDL_Init(0)) {
    return 0;
  }

  SDL_Window *main_window =
      SDL_CreateWindow("Texture test", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 600, 400, SDL_WINDOW_OPENGL);

  if (nullptr == main_window) {
    return 0;
  }

  SDL_Window *second_window =
      SDL_CreateWindow("Texture test Debugger", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 128, 128, SDL_WINDOW_OPENGL);

  if (nullptr == second_window) {
    return 0;
  }

  SDL_Renderer *main_renderer =
      SDL_CreateRenderer(main_window, -1, SDL_RENDERER_ACCELERATED);

  if (nullptr == main_renderer) {
    return 0;
  }

  SDL_Renderer *second_renderer =
      SDL_CreateRenderer(second_window, -1, SDL_RENDERER_ACCELERATED);

  if (nullptr == second_renderer) {
    return 0;
  }

  SDL_Texture *shared_cache_texture =
      SDL_CreateTexture(main_renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_TARGET, 128, 128);

  if (nullptr == shared_cache_texture) {
    return 0;
  }

  SDL_SetRenderTarget(main_renderer, shared_cache_texture);

  SDL_SetRenderDrawColor(main_renderer, 100, 100, 70, 255);
  SDL_RenderClear(main_renderer);
  std::array<SDL_Rect, 4> rects = {
      {{10, 13, 5, 3}, {4, 23, 5, 10}, {7, 10, 11, 5}, {10, 13, 5, 5}}};
  SDL_SetRenderDrawColor(main_renderer, 255, 0, 0, 255);
  SDL_RenderFillRects(main_renderer, rects.data(), rects.size());

  SDL_SetRenderTarget(main_renderer, NULL);

  SDL_Surface *tmp =
      SDL_CreateRGBSurfaceWithFormat(0, 128, 128, 32, SDL_PIXELFORMAT_ARGB8888);

  SDL_Event e;

  while (true) {
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
      case SDL_QUIT:
        return 0;
      case SDL_KEYDOWN: {
        switch (e.key.keysym.sym) {
        case SDLK_ESCAPE:
          return 0;
        }
      } break;
      } // event type switch
    }   // while poll event


    SDL_Rect dst = {0,0,128,128};
    SDL_SetRenderTarget(main_renderer, shared_cache_texture);
    SDL_RenderReadPixels(main_renderer, &dst, SDL_PIXELFORMAT_ARGB8888, tmp->pixels, tmp->pitch);

    // SDL_SetRenderDrawColor(second_renderer, 0, 0, 0, 255);
    // SDL_RenderClear(second_renderer);
    SDL_Texture *tmp_tex = SDL_CreateTextureFromSurface(second_renderer, tmp);
    SDL_RenderCopy(second_renderer, tmp_tex, &dst, &dst);
    SDL_DestroyTexture(tmp_tex);
    
    SDL_RenderPresent(second_renderer);
    
    draw_main_window(main_renderer, shared_cache_texture);

    SDL_Delay(10);
  } // while true

  SDL_FreeSurface(tmp);
  return 0;
}