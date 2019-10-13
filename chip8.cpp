#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "chip8.h"
#include "chip8_fontset.h"

using namespace std;

Chip8::Chip8(){}
Chip8::~Chip8(){}

// Clear the memory, registers, and screen
void Chip8::initialize(){
	pc = 0x200; // Start of program counter
	opcode = 0; // Reset current opcode
	I = 0; // Reset index register
	sp = 0; // Reset stack pointer
	// Reset random seed
	srand(time(NULL));
	// Clear display
	for (int i = 0; i < WIDTH * HEIGHT; ++i) gfx[i] = 0;
	drawFlag = true;
	// Clear stack
	for (int i = 0; i < STACK_SIZE; ++i) stack[i] = 0;
	// Clear keys (all unpressed)
	for (int i = 0; i < NUM_KEYS; ++i) key[i] = 0;
	// Clear registers V0-VF
	for (int i = 0; i <= 0xF; ++i) V[i] = 0;
	// Clear memory
	for (int i = 0; i < MEMORY_SIZE; ++i) memory[i] = 0;
	// Load fontset
	for (int i = 0; i < 80; ++i){
		memory[i] = chip8_fontset[i];
	}
	// Reset timers
	delay_timer = 0;
	sound_timer = 0;
	soundFlag = false;
}

// Copy the program into the memory
bool Chip8::loadGame(const char* filename){
	FILE * pFile = fopen(filename, "rb");
	assert(pFile != NULL);
	// Check file size
	fseek(pFile , 0 , SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	// Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL) {
		fputs ("Memory error", stderr);
		return false;
	}
	// Copy the file into the buffer
	size_t result = fread (buffer, 1, lSize, pFile);
	if (result != lSize) {
		fputs("Reading error",stderr);
		return false;
	}
	// Copy buffer to Chip8 memory
	if(lSize >= 4096 - 512){
		printf("Error: ROM too big for memory");
		return false;
	}
	for(int i = 0; i < lSize; ++i) memory[i + 512] = buffer[i];
	// Close file, free buffer
	fclose(pFile);
	free(buffer);
	return true;
}

void print_hex(unsigned short c){
	cout << hex << "0x" << ((c<0x10)?"0":"") << ((c<0x100)?"0":"") << ((c<0x1000)?"0":"") << (static_cast<unsigned int>(c) & 0xFFFF) << dec << endl;
}
bool op_error = false;
void Chip8::unknown_opcode(){
	cout << "Unknown opcode: ";
	print_hex(opcode);
	op_error = true;
}

void Chip8::op_debug(){
	cout << "PC: " << pc << ", opcode: ";
	print_hex(opcode);
}

// Emulate one cycle of the system
void Chip8::emulateCycle(){
	if (op_error) return;
	// Fetch opcode
	opcode = memory[pc] << 8 | memory[pc + 1]; // Uses bit-shifting and bitwise OR to combine the two byte opcode into one piece of data
	// Decode opcode and execute opcode
	// Reference: https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
	unsigned short NNN = opcode & 0x0FFF;
	unsigned short NN = opcode & 0x00FF;
	unsigned short N = opcode & 0x000F;
	unsigned short X = (opcode & 0x0F00) >> 8;
	unsigned short Y = (opcode & 0x00F0) >> 4;

	switch (opcode & 0xF000){
	case 0x0000:
		switch (N){
		case 0x0000: // 00E0: Clears the screen
			for (int i = 0; i < WIDTH * HEIGHT; ++i) gfx[i] = 0;
			drawFlag = true;
			pc += 2;
		break;
		case 0x000E: // 00EE: Returns from subroutine
			--sp;
			pc = stack[sp] + 2;
		break;
		default:
			unknown_opcode();
		}
	break;
	case 0x1000: // 1NNN: Jumps to address at NNN
		pc = NNN;
	break;
	case 0x2000: // 2NNN: Calls subroutine at NNN
		stack[sp] = pc;
		++sp;
		pc = NNN;
	break;
	case 0x3000: // 3XNN: Skips the next instruction if VX equals NN
		if (V[X] == NN) pc += 4;
		else pc += 2;
	break;
	case 0x4000: // 4XNN: Skips the next instruction if VX doesn't equal NN
		if (V[X] != NN) pc += 4;
		else pc += 2;
	break;
	case 0x5000: // 5XY0: Skips the next instruction if VX equals VY
		if (V[X] == V[Y]) pc += 4;
		else pc += 2;
	break;
	case 0x6000: // 6XNN: Sets VX to NN
		V[X] = NN;
		pc += 2;
	break;
	case 0x7000: // 7XNN: Addes NN to VX
		V[X] += NN;
		pc += 2;
	break;
	case 0x8000:
		switch(N){
		case 0x0000: // 8XY0: Sets VX to the value of VY
			V[X] = V[Y];
			pc += 2;
		break;
		case 0x0001: // 8XY1: Sets VX to VX or VY
			V[X] |= V[Y];
			pc += 2;
		break;
		case 0x0002: // 8XY2: Sets VX to VX and VY
			V[X] &= V[Y];
			pc += 2;
		break;
		case 0x0003: // 8XY3: Sets VX to VX xor VY
			V[X] ^= V[Y];
			pc += 2;
		break;
		case 0x0004: // 8XY4: Adds VY to VX. VF set to 1 if there's a carry, else 0
			if (V[X] + V[Y] > 0xFF) V[0xF] = 1; // carry
			else V[0xF] = 0; // No carry
			V[X] += V[Y];
			pc += 2;
		break;
		case 0x0005: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, else 1
			if (V[X] - V[Y] < 0) V[0xF] = 0; // borrow
			else V[0xF] = 1; // no borrow
			V[X] -= V[Y];
			pc += 2;
		break;
		case 0x0006: // 8XY6: Stores the least sig. bit of VX in VF and then shifts VX to the right by 1
			V[0xF] = V[X] & 0x000F;
			V[X] >>= 1;
			pc += 2;
		break;
		case 0x0007: // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, else 1
			if (V[Y] - V[X] < 0) V[0xF] = 0; // borrow
			else V[0xF] = 1; // no borrow
			V[X] = V[Y] - V[X];
			pc += 2;
		break;
		case 0x000E: // 8XYE: Stores the most sig. bit of VX in VF and then shifts VX to the left by 1
			V[0xF] = V[X] >> 7; // Most significant bit
			V[X] <<= 1;
			pc += 2;
		break;
		default:
			unknown_opcode();
		}
	break;
	case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY
		if (V[X] != V[Y]) pc += 4;
		else pc += 2;
	break;
	case 0xA000: // ANNN: Sets I to the address NNN
		I = NNN;
		pc += 2;
	break;
	case 0xB000: // BNNN: Jumps to the address NNN plus V0
		pc = NNN + V[0];
	break;
	case 0xC000: // CXNN: Sets VX to the result of a bitwise and operation on a random number (0 to 255) and NN
		V[X] = (rand() % 255) & NN;
		pc += 2;
	break;
	case 0xD000: // DXYN: Draws 8xN pixel sprite at (VX, VY). See reference table
	{
		unsigned short x = V[X]; // Fetch coordinate value from registers
		unsigned short y = V[Y];
		unsigned short pixel; // Pixel value
		V[0xF] = 0; // Reset VF register
		// Loop through each row
		for (int yline = 0; yline < N; ++yline){
			pixel = memory[I + yline]; // Fetch pixel value from memory starting at I
			// Loop over 8 bits of one row
			for (int xline = 0; xline < 8; ++xline){
				// Scan through each byte of currently evaluated pixel to check if it is 1
				if ((pixel & (0x80 >> xline)) != 0){
					// Check for collision and register result in VF
					int index = x + xline + ((y + yline) * WIDTH);
					if(gfx[index] == 1) V[0xF] = 1;
					// Set pixel value with XOR
					gfx[index] ^= 1; 
				}
			}
		}
		drawFlag = true; // To be used by graphics implementation to update screen
		pc += 2;
	}
	break;
	case 0xE000:
		switch (NN){
		case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
			if (key[V[X]] != 0) pc += 4;
			else pc += 2;
		break;
		case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
			if (key[V[X]] == 0) pc += 4;
			pc += 2;
		break;
		default:
			unknown_opcode();
		}
	break;
	case 0xF000:
		switch (NN){
		case 0x0007: // FX07: Sets VX to the value of the delay timer
			V[X] = delay_timer;
			pc += 2;
		break;
		case 0x000A: // FX0A: A key press is awaited, and then stored in VX. Blocking Operation.
			for (int i = 0; i < NUM_KEYS; ++i){
				if (key[i] != 0){
					V[X] = i;
					pc += 2;
					break;
				}
			}
			// Don't increment pc because it's a blocking operation
		break;
		case 0x0015: // FX15: Sets the delay timer to VX
			delay_timer = V[X];
			pc += 2;
		break;
		case 0x0018: // FX18: Sets the sound timer to VX
			sound_timer = V[X];
			pc += 2;
		break;
		case 0x001E: // FX1E: Adds VX to I
			if (I + V[X] > 0xFFF) V[0xF] = 1; // range overflow
			else V[0xF] = 0;
			I += V[X];
			pc += 2;
		break;
		case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX
			I = V[X] * 5; // Memory of fonts starts at 0x000, and each character is 5 px tall
			pc += 2;
		break;
		case 0x0033: // FX33: Stores the binary-coded decimal representation of VX. See reference table for specs.
			memory[I] = V[X] / 100; // Hundreds digit
			memory[I + 1] = (V[X] / 10) % 10; // Tens digit
			memory[I + 2] = (V[X] % 100) % 10; // Ones digit
			pc += 2;
		break;
		case 0x0055: // FX55: Stores V0 to VX (including VX) in memory starting at address I
			for (int i = 0; i <= X; ++i){
				memory[I + i] = V[i];
			}
			I += X + 1;
			pc += 2;
		break;
		case 0x0065: // FX65: Fills V0 to VX (including VX) with values from memory starting at address I
			for (int i = 0; i <= X; ++i){
				V[i] = memory[I + i];
			}
			I += X + 1;
			pc += 2;
		break;
		default:
			unknown_opcode();
		}
	break;
	default:
		unknown_opcode();

	}
	// Update timers
	if (delay_timer > 0) --delay_timer;
	if (sound_timer > 0){
		if (sound_timer == 1) soundFlag = true;
		--sound_timer;
	}
}
