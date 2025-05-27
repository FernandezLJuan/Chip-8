#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "chip8.h"
#include "register_operations.h"

void handle_0x0(uint16_t instruction){
    //extract the last nibble of the instruction to determine if it returns from a subroutine or clears the screen
    uint8_t operation = instruction & 0x00FF;

    switch (operation)
    {
    case 0xE0:
        clear_screen();
        break;

    case 0xEE:
        return_from_subroutine();
        break;
    
    default:
        break;
    }
}

void handle_0x1NNN(uint16_t instruction){
    uint16_t address = instruction & 0x0FFF;  // Extract address
    if (address>=0x200 && address <= 0xFFF) {  // Ensure jump target is within memory bounds
        pc = address;
    }
}


void handle_0x2NNN(uint16_t instruction) {
    uint16_t address = instruction & 0xFFF;
    if (address >= 0x200 && address <= 0xFFF) {
        call_subroutine(address);
    } else {
        printf("Invalid call address: 0x%X\n", address);
    }
}


void handle_0x3XNN(uint16_t instruction){
    //skip next instruction if VX == NN
    uint8_t vx = instruction>>8 & 0x0F;
    uint8_t value = instruction & 0xFF;

    if(registers[vx] == value){
        pc += 2;
    }
}

void handle_0x4XNN(uint16_t instruction){
    //skip next instruction if VX != NN
    uint8_t vx = instruction>>8 & 0x0F;
    uint8_t value = instruction & 0xFF;

    if (registers[vx] != value){
        pc += 2;
    }
}

void handle_0x5XY0(uint16_t instruction){
    //skip next instruction if VX == VY
    uint8_t vx = instruction>>8 & 0x0F;
    uint8_t vy = instruction>>4 & 0x0F;

    if(registers[vx] == registers[vy]){
        pc += 2;
    }
}

void handle_0x6XNN(uint16_t instruction){
    uint8_t reg = instruction>>8 & 0x0F; //get the register
    uint8_t value = instruction & 0xFF; //get the value

    set_register(reg, value); //execute the instruction
}

void handle_0x7XNN(uint16_t instruction){
    uint8_t reg = instruction>>8 & 0x0F; //get the register
    uint8_t value = instruction & 0XFF; //get the value

    add_immediate(reg, value); //execute the instruction
}

void handle_0x8XYN(uint16_t instruction){
    uint8_t vx = instruction>>8 & 0x0F; //get X
    uint8_t vy = instruction>>4 & 0x0F; //get Y
    uint8_t operation = instruction & 0X0F; //get the instruction code

    switch(operation){
        case 0x0: assign_reg(vx, vy); break;
        case 0x1: or_reg(vx,vy); break;
        case 0x2: and_reg(vx,vy); break;
        case 0x3: xor_reg(vx,vy); break;
        case 0x4: add_registers(vx,vy); break;
        case 0x5: substract_registers(vx,vy); break;
        case 0x6: rshift_register(vx); break;
        case 0x7: vy_sub_vx(vx,vy); break;
        case 0xE: lshift_register(vx); break;
        default: printf("Unknown 0x8XYN operation %X\n", operation);
    }
}

void handle_0x9XY0(uint16_t instruction){
    //skips next instruction if VX != VY
    uint8_t vx = instruction>>8 & 0x0F;
    uint8_t vy = instruction>>4 & 0x0F;

    if(registers[vx] != registers[vy]){
		pc+= 2;
	}
}

void handle_0xANNN(uint16_t instruction){
    uint16_t address = instruction & 0xFFF;

    I = address;
}

void handle_0xBNNN(uint16_t instruction) {
    uint16_t offset = instruction & 0xFFF;
    uint16_t target = registers[0x0] + offset;
    if (target >= 0x200 && target <= 0xFFF) {
        pc = target;
    } else {
        printf("Invalid jump address: 0x%X\n", target);
    }
}

void handle_0xCXNN(uint16_t instruction){
    uint8_t vx = instruction>>8 & 0x0F;
    uint8_t value = instruction & 0xFF;

    int random_value = rand()%256;

    registers[vx] = value & random_value;
}

void handle_0xDXYN(uint16_t instruction){
    //printf("Drawing\n");
    uint8_t vx = instruction>>8 & 0x0F;
    uint8_t vy = instruction>>4 & 0x0F;
    uint8_t height = instruction & 0x0F;

    draw_sprite(registers[vx], registers[vy], height);
}

void handle_0xE(uint16_t instruction){
    uint8_t vx = instruction>>8 & 0x0F;
    uint8_t operation = instruction & 0xFF;
    uint8_t lowest_nibble = registers[vx] & 0x0F;

    switch (operation)
    {
    case 0x9E:
        //skip the next instruction if the key in vx is pressed (only consider the lowest nibble)
        if(keyboard[lowest_nibble]){
            //printf("Skipping the next instruction because 0x%x is pressed\n", registers[vx]);
            pc+=2;
        }
    break;

    case 0xA1:
    //skip the next instruction if the key in vx is pressed (only consider the lowest nibble)
    if(!keyboard[lowest_nibble]){
            //printf("Skipping the nexte instruction because 0x%x is NOT pressed\n", registers[vx]);
            pc+=2;
        }
        break;
    
    default:
        break;
    }
}

void handle_0xF(uint16_t instruction){
    uint8_t vx = instruction>>8 & 0x0F;
    uint8_t operation = instruction & 0xFF;
    
    switch (operation)
    {
    case 0x07:{
        registers[vx] = delay_timer;
        break;
    }

    case 0x0A:{
        registers[vx] = 0xFF; // Default invalid key
        for (uint8_t key = 0; key < 16; key++) {
            printf("Waiting for key: 0x%X\n", key);
            if (keyboard[key]) {
                registers[vx] = key;
                break;
            }
        }
        if (registers[vx] == 0xFF) {
            pc -= 2; // Re-execute the instruction until a key is pressed
        }
        break;
    }
        
    case 0x15:{
        delay_timer = registers[vx];
        break;
    }

    case 0x18:{
        sound_timer = registers[vx];
        break;
    }

    case 0x1E:{
        I += registers[vx];
        break;
    }

    case 0x29:{
        uint8_t lowest_nibble = registers[vx] & 0x0F;
        I = FONT_BASE_ADDRESS + (lowest_nibble*5);
        break;
    }

    case 0x33:{
        memory[I] = registers[vx] / 100;
        memory[I+1] = (registers[vx] / 10) % 10;
        memory[I+2] = registers[vx] % 10;
        break;
    }

    case 0x55:{
        reg_dump(vx);
        break;
    }

    case 0x65:{
        reg_load(vx);
        break;
    }

    default:
        break;
    }
}