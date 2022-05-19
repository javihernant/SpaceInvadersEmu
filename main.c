#include <stdlib.h>
#include <stdio.h>
#include "8080emu.h"
#include "machine.h"
#include <stdint.h>
#include "screen.h"

int main (int argc, char **argv)
{

    if (argc < 2)
    {
        printf("error, no rom was provided\n");
        return 1;
    }

    unsigned char *buffer = init_machine(argv[1]);
    unsigned char *vram = &buffer[0x2400];

    State8080 state = StateCreat(buffer);
    init_screen(vram, 256, 224);
    SDL_Event e;

    int running = 1;
    while (running)
    {

		while(SDL_PollEvent(&e) != 0){
			if(e.type == SDL_QUIT){
				running = 0;
			}
		}


        uint8_t *opcode = &state.memory[state.pc];

        if (*opcode == 0xdb) {
            uint8_t port = opcode[1];
            machine_in(&state, port);
            state.pc += 1;
            
        }else if (*opcode == 0xd3) {
            uint8_t port = opcode[1];
            machine_out(&state, port);
            state.pc += 1;
        }else{
            Emulate8080Op(&state);
        }

        //printState(&state);
        render_bf_2(vram);
    }

    screen_off();
    free(buffer);

    return 0;
}
