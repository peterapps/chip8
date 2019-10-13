#pragma once

#define WIDTH 64
#define HEIGHT 32
#define MEMORY_SIZE 4096
#define STACK_SIZE 16
#define NUM_KEYS 16

class Chip8 {
	// http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
private:
	unsigned short opcode;
	unsigned char memory[4096];
	/*
	 * Memory map:
	 * 0x000 - 0x1FF = chip 8 interpreter (font set)
	 * 0x050 - 0x0A0 = used for built-in 4x5 pixel font set
	 * 0x200 - 0xFFF = program ROM and work RAM
	 */
	unsigned char V[16]; // CPU registers
	unsigned short I; // index register
	unsigned short pc; // program counter
	unsigned char delay_timer;
	unsigned char sound_timer;
	unsigned short stack[STACK_SIZE];
	unsigned short sp; // stack pointer
	void unknown_opcode();
	void op_debug();
public:
	unsigned char gfx[WIDTH * HEIGHT];
	unsigned char key[NUM_KEYS];
	Chip8();
	~Chip8();
	void initialize();
	bool loadGame(const char* filename);
	void emulateCycle();
	bool drawFlag;
	bool soundFlag;
};
