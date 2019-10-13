#pragma once

#include <SDL2/SDL.h>
#include <stdio.h>

class Display {
private:
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	int SCREEN_WIDTH, SCREEN_HEIGHT;
public:
	Display(int width, int height, int scale);
	~Display();
	void draw(unsigned char gfx[], int width, int height);
	void delay(int ms);
	bool key_listener(unsigned char keys[], int num_keys);
};
