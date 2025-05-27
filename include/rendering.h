#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "chip8.h"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
Mix_Chunk* beep = nullptr;

void init_sdl(){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);
	printf("Initializing audio...\n");

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	printf("Reading beep sound effect...\n");
	beep = Mix_LoadWAV("../assets/beep.wav");
	
	printf("Creating window...\n");
	window = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 340, SDL_WINDOW_RESIZABLE);

	printf("Creating renderer...\n");
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	printf("Setting renderer attributes...\n");
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);
}


void draw_sprite(uint8_t x, uint8_t y, uint8_t height) {
    registers[0xF] = 0; // Reset collision flag

    for (uint8_t row = 0; row < height; row++) {
        uint8_t sprite_data = memory[I + row];

        for (uint8_t col = 0; col < 8; col++) {
            if (sprite_data & (0x80 >> col)) { // Check if the pixel is set
                uint8_t pixel_x = (x + col) % 64; // Valid range: 0–63
                uint8_t pixel_y = (y + row) % 32; // Valid range: 0–31

                // Check collision BEFORE modifying the pixel
                if (display[pixel_x][pixel_y] == 1) {
                    registers[0xF] = 1;
                }

                // XOR the pixel in the display buffer
                display[pixel_x][pixel_y] ^= 1;
            }
        }
    }
}

void render_display(SDL_Renderer* renderer){

	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);

	for(uint8_t x=0; x<64; x++){
		for(uint8_t y=0; y<32; y++){
			if(display[x][y]){
				SDL_Rect pixel_rect = {x*10, y*10, 10, 10};
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderFillRect(renderer, &pixel_rect);
			}
		}
	}

	SDL_RenderPresent(renderer);
}

void clear_screen(){
	SDL_SetRenderDrawColor(renderer, 0,0,0,255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	memset(&display, 0, sizeof(display));
}