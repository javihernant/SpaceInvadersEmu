#include <stdlib.h>
#include <stdio.h>
#include "8080emu.h"
#include "machine.h"
#include <stdint.h>

int main (int argc, char **argv)
{
    FILE *f= fopen(argv[1], "rb");
    if (f==NULL)
    {
        printf("error: Couldn't open %s\n", argv[1]);
        exit(1);
    }

    //Get the file size and read it into a memory buffer
    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    unsigned char *buffer=malloc(fsize);

    fread(buffer, fsize, 1, f);
    fclose(f);

    State8080 state = StateCreat(buffer);

    while (state.pc < fsize)
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
    return 0;
}
