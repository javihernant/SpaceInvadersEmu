#include <stdlib.h>
#include <stdio.h>
#include "regs.h"
#include "machine.h"
#include "8080emu.h"
#include <stdint.h> 

static uint8_t shift_offset;
static uint16_t shift_val = 0;

/* SPACE INVADERS MEMORY MAP:

    0000-1FFF 8K ROM
    2000-23FF 1K RAM
    2400-3FFF 7K Video RAM
    4000- RAM mirror (Not implemented. Is it needed?)
*/

unsigned char *load_rom(char *rom_path, int offset)
{
    unsigned char *mem = malloc(0x4000);
    FILE *f= fopen(rom_path, "rb");
    if (f==NULL)
    {
        printf("error: Couldn't open %s\n", rom_path);
        exit(1);
    }

    //Get the file size and read it into a memory buffer
    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    size_t written = fread(&mem[offset], 1, fsize, f);
    if (written != fsize){
        printf("fread: error reading to buffer\n");
    }
    fclose(f);

    return mem;
}

void machine_in(State8080 *st, int port)
{
    switch(port){
        case 1:
        {
            st->a = 1;
            break;
        }
        case 2:
        {
            st->a = 0;
            break;
        }
        case 3:
        {
            st->a = shift_val >> (8-shift_offset);
            break;
        }
    }
    st->pc += 1;
}

void machine_out(State8080 *st, int port)
{
    switch(port){
        case 2:
        {
            shift_offset = (st->a);
            break;
        }
        case 4:
        {
            shift_val = (st->a)<<8 | ((shift_val >> 8) & 0xff); 
            break;
        }
    }
    st->pc += 1;
}

void gen_int(State8080 *st, int num)
{
    st->int_enable = 0;
    PUSH(st->memory, (st->pc&0xff00)>>8, st->pc&0xff, &st->sp);
    st->pc = num*8;
}
