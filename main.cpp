#include <iostream>
#include "chip8.h"
#include "display.h"

using namespace std;

int main(int argc, char* argv[]){
	if (argc != 2){
		cout << "Usage: ./emulator PATH_TO_ROM" << endl;
		return 0;
	}
	Chip8 cpu;
	cpu.initialize();
	cpu.loadGame(argv[1]);
	Display disp(WIDTH, HEIGHT, 20);
	int i = 0;
	while (disp.key_listener(cpu.key, NUM_KEYS)){
		cpu.emulateCycle();
		if (cpu.drawFlag){
			disp.draw(cpu.gfx, WIDTH, HEIGHT);
			cpu.drawFlag = false;
			/*for (int i = 0; i < 20; ++i) cout << endl;
			for (int y = 0; y < HEIGHT; ++y){
				for (int x = 0; x < WIDTH; ++x){
					if (cpu.gfx[y*WIDTH + x] == 0) cout << ".";
					else cout << "#";
				}
				cout << endl;
			}*/
		}
		if (cpu.soundFlag){
			cout << "Beep" << endl;
			cpu.soundFlag = false;
		}
		disp.delay(5);
	}
}
