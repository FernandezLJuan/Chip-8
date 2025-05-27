#pragma once

void set_register(uint8_t reg, uint8_t value){

	if(reg == 0xF){
		return;
	}

	registers[reg] = value;
}

void add_immediate(uint8_t reg, uint8_t value){
	registers[reg] += value;
}

void assign_reg(uint8_t vx, uint8_t vy){
	registers[vx] = registers[vy];
}

void or_reg(uint8_t vx, uint8_t vy){
	registers[vx] = registers[vx] | registers[vy];
}

void and_reg(uint8_t vx, uint8_t vy){
	registers[vx] = registers[vx] & registers[vy];
}

void xor_reg(uint8_t vx, uint8_t vy){
    registers[vx] = registers[vx] ^ registers[vy];
}

void add_registers(uint8_t vx, uint8_t vy){
	uint16_t sum = registers[vx] + registers[vy];

	registers[vx] = sum & 0xFF; /*8-bit warparound*/
	registers[0xF] = (sum>0xFF) ? 1 : 0; /*set VF if overflow happens*/
}

void substract_registers(uint8_t vx, uint8_t vy){
	registers[0xF] = (registers[vx]>=registers[vy]) ? 1 : 0; /*set VF to 1 if there is borrow*/
	registers[vx] -= registers[vy];
}

void vy_sub_vx(uint8_t vx, uint8_t vy){
	registers[0xF] = (registers[vy]>=registers[vx]) ? 1 : 0; /*set VF to 1 if there is borrow*/
	registers[vx] = registers[vy] - registers[vx]; 
}

void rshift_register(uint8_t vx){
	registers[0xF] = registers[vx] & 1; /*extract LSB of VX and store it in VF*/
	registers[vx] = registers[vx]>>1;
}

void lshift_register(uint8_t vx){
	registers[0xF] = (registers[vx] & 0x80) >> 7; /*extract MSB of VX and store it in VF*/
	registers[vx] = registers[vx]<<1;
}

void call_subroutine(uint16_t address) {
    if (sp < sizeof(stack) / sizeof(stack[0])) { // Ensure sp < 48
        stack[sp] = pc;
        sp++;
        pc = address;
    } else {
        printf("Stack overflow!\n");
		while(1){

		}
    }
}

void return_from_subroutine() {
    if (sp > 0) {
        sp--;
        pc = stack[sp];
    } else {
        printf("Stack underflow!\n");
    }
}

void reg_dump(uint8_t vx){
	for(uint8_t i = 0; i<=vx; i++){
		memory[i+I] = registers[i];
	}
}

void reg_load(uint8_t vx){
	for(uint8_t i = 0; i<=vx; i++){
		if(I+i<4096)
			registers[i] = memory[I+i];
	}
}
