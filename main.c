#include <stdlib.h>
#include <stdio.h>
#include "8080emu.h"
#include "machine.h"
#include <stdint.h>

int main (int argc, char **argv)
{
    if (argc < 2)
    {
        printf("init_machine: error, no rom was provided\n");
        exit(1);
    }

    unsigned char *buffer = init_machine(argv[1]);

    State8080 state = StateCreat(buffer);

    //TODO: change running condition. Right now it is hardcoded to 0x4000
    while (state.pc < 0x4000)
    {
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

        printState(&state);
    }

    free(buffer);
    return 0;
}
