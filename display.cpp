#include "display.h"
#include <iostream>

using namespace std;

Display::Display(int width, int height, int scale) : SCREEN_WIDTH(width*scale), SCREEN_HEIGHT(height*scale) {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0){
		cout << "SDL Error: " << SDL_GetError() << endl;
	}
	// Create window
	window = SDL_CreateWindow(
		"CHIP-8 Emulator",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN
	);
	renderer =  SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);
}

Display::~Display(){
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

void Display::draw(unsigned char gfx[], int width, int height){
	int ratio = SCREEN_WIDTH / width;
	SDL_Rect pixels[width*height];
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x){
			if (gfx[y * width + x] != 0){
				SDL_Rect &r = pixels[y*width + x];
				r.x = ratio*x;
				r.y = ratio*y;
				r.w = ratio;
				r.h = ratio;
				SDL_RenderFillRect(renderer, &r);
			}
		}
	}
	SDL_RenderPresent(renderer);
}

void Display::delay(int ms){
	SDL_Delay(ms);
}

bool Display::key_listener(unsigned char keys[], int num_keys){
	SDL_Event event;
	SDL_PollEvent(&event);
	if (event.type == SDL_QUIT) return false;
	else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP){
		// cout << "Key pressed: " << event.key.keysym.sym << endl;
		unsigned char val = (event.type == SDL_KEYDOWN) ? 1 : 0;
		switch (event.key.keysym.sym){
			case SDLK_1: keys[0x1] = val; break;
			case SDLK_2: keys[0x2] = val; break;
			case SDLK_3: keys[0x3] = val; break;
			case SDLK_4: keys[0xC] = val; break;
			
			case SDLK_q: keys[0x4] = val; break;
			case SDLK_w: keys[0x5] = val; break;
			case SDLK_e: keys[0x6] = val; break;
			case SDLK_r: keys[0xD] = val; break;

			case SDLK_a: keys[0x7] = val; break;
			case SDLK_s: keys[0x8] = val; break;
			case SDLK_d: keys[0x9] = val; break;
			case SDLK_f: keys[0xE] = val; break;
			
			case SDLK_z: keys[0xA] = val; break;
			case SDLK_x: keys[0x0] = val; break;
			case SDLK_c: keys[0xB] = val; break;
			case SDLK_v: keys[0xF] = val; break;
		}
	}
	return true;
}
