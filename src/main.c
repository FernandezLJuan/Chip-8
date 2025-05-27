#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <nfd.h>
#include "rendering.h"
#include "chip8.h"
#include "handlers.h"

typedef void (*opcode_handler)(uint16_t instruction);

opcode_handler opcode_table[16];

uint8_t registers[16]; //16 registers from v0 to vf
uint8_t memory[4096]; //memory layout of the device, starting at 0x200 (512 bytes reserved)
uint16_t stack[48];
uint8_t keyboard[16];
uint8_t display[64][32] = {0};
uint16_t pc = 0x200; //Program Counter
uint16_t sp = 0;
uint16_t I = 0;
uint8_t delay_timer = 0;
uint8_t sound_timer = 0;

void initialize_chip8(){
	opcode_table[0x0] = &handle_0x0;
	opcode_table[0x1] = &handle_0x1NNN;
	opcode_table[0x2] = &handle_0x2NNN;
	opcode_table[0x3] = &handle_0x3XNN;
	opcode_table[0x4] = &handle_0x4XNN;
	opcode_table[0x5] = &handle_0x5XY0;
	opcode_table[0x6] = &handle_0x6XNN;
	opcode_table[0x7] = &handle_0x7XNN;
	opcode_table[0x8] = &handle_0x8XYN;
	opcode_table[0x9] = &handle_0x9XY0;
	opcode_table[0xA] = &handle_0xANNN;
	opcode_table[0xB] = &handle_0xBNNN;
	opcode_table[0xC] = &handle_0xCXNN;
	opcode_table[0xD] = &handle_0xDXYN;
	opcode_table[0xE] = &handle_0xE;
	opcode_table[0xF] = &handle_0xF;
}

void fetch_decode_execute(){
	uint16_t instruction = (memory[pc] << 8) | memory[pc + 1];
	pc +=2;

	uint8_t table_index = instruction>>12;
	if(opcode_table[table_index] == NULL){
		printf("What is that 0x%X\n", instruction);
	}
	opcode_table[table_index](instruction);
}

void update_keys(SDL_Event e) {
    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
        uint8_t key_state = (e.type == SDL_KEYDOWN) ? 1 : 0;

        switch (e.key.keysym.sym) {
            case SDLK_1: keyboard[0x0] = key_state; break;
            case SDLK_2: keyboard[0x1] = key_state; break;
            case SDLK_UP: keyboard[0x2] = key_state; break;
            case SDLK_4: keyboard[0x3] = key_state; break;
            case SDLK_LEFT: keyboard[0x4] = key_state; break;
            case SDLK_w: keyboard[0x5] = key_state; break;
            case SDLK_RIGHT: keyboard[0x6] = key_state; break;
            case SDLK_r: keyboard[0x7] = key_state; break;
            case SDLK_DOWN: keyboard[0x8] = key_state; break;
            case SDLK_s: keyboard[0x9] = key_state; break;
			case SDLK_d: keyboard[0xA] = key_state; break;
            case SDLK_f: keyboard[0xB] = key_state; break;
            case SDLK_z: keyboard[0xC] = key_state; break;
            case SDLK_x: keyboard[0xD] = key_state; break;
            case SDLK_c: keyboard[0xE] = key_state; break;
            case SDLK_v: keyboard[0xF] = key_state; break;
        }
    }
}

void update_timers() {
    static uint32_t last_time = 0;
    uint32_t current_time = SDL_GetTicks();
    uint32_t delta = current_time - last_time;

    if (delta >= (1000 / 60)) { // ~16.67ms per tick
        if (delay_timer > 0) delay_timer--;
        if (sound_timer > 0) sound_timer--;
        last_time = current_time;
    }
}

void reset_system(){
	memset(registers, 0, sizeof(registers));
	memset(memory, 0, sizeof(memory));
	memset(stack, 0, sizeof(stack));
	memset(keyboard, 0, sizeof(keyboard));
	clear_screen();
	delay_timer = 0;
	sound_timer = 0;
	I = 0;
	sp = 0;
	pc = 0x200;
}

void read_ROM(const char* path){

	reset_system();

	FILE* ROM = fopen(path, "rb");
	if(ROM==NULL){
		printf("Could not open rom!\n");
		return;
	}

	size_t bytes_read = fread((memory+0x200), sizeof(uint8_t),4096-0x200,ROM);
	memcpy(memory+FONT_BASE_ADDRESS, font_data, sizeof(uint8_t)*80);
	
	if(bytes_read == 0){
		printf("Could not read ROM!\n");
		return;
	}

	fclose(ROM);

}

int main(int argc, char* argv[]){
    if (argc < 2) {
        printf("Usage: %s </path/to/ROM file>\n", argv[0]);
        return 1;
    }
    
    read_ROM(argv[1]);

	init_sdl();

	SDL_Event e;
	bool running = true;
	const uint8_t IPS = 11;

	printf("Initializing CHIP8\n");
	initialize_chip8();

	while(running){
		uint32_t start_time = SDL_GetTicks();
		while(SDL_PollEvent(&e)){
			if(e.type == SDL_QUIT){
				running = false;
			}
			update_keys(e);
		}

		for(int i = 0; i<IPS; i++){
			fetch_decode_execute();
		}

		update_timers();

		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(renderer);

		for(uint8_t x=0; x<64; x++){
			for(uint8_t y=0; y<32; y++){
				if(display[x][y]){
					SDL_Rect pixel_rect = {x*10, y*10+20, 10, 10};
					SDL_SetRenderDrawColor(renderer, 0xFF,0xFF,0xFF, 0xFF);
					SDL_RenderFillRect(renderer, &pixel_rect);
				}
			}
		}

		SDL_RenderPresent(renderer);

		if(sound_timer>0){
			Mix_PlayChannel(-1, beep, 0);
		}

		uint32_t elapsed_time = SDL_GetTicks() - start_time;
		if (elapsed_time < 1000/60) {
			SDL_Delay(1000/60 - elapsed_time);  // Delay to maintain 60 FPS
		}
	}

	Mix_FreeChunk(beep);
	Mix_CloseAudio();
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
