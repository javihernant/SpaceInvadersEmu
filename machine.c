#include <stdlib.h>
#include <stdio.h>
#include "regs.h"
#include "machine.h"
#include "8080emu.h"
#include <stdint.h> 
#include "SDL2/SDL_mixer.h"

static uint8_t shift_offset;
static uint16_t shift_val = 0;
static uint8_t in_port1 = 0;
static uint8_t in_port2 = 0;
static uint8_t out3_last = 0;
static uint8_t out5_last = 0;
extern Mix_Chunk *sounds[];

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
        case 0:
        {
            st->a = 0xff;
            break;
        }
        case 1:
        {
            st->a = in_port1;
            break;
        }
        case 2:
        {
            st->a = in_port2;
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
        case 3:
        {
            handle_sound(st->a, 3);
            break;
        }
        case 4:
        {
            shift_val = (st->a)<<8 | ((shift_val >> 8) & 0xff); 
            break;
        }
        case 5:
        {
            handle_sound(st->a, 5);
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

void handle_sound(uint8_t a, int port)
{
    static int playing = 0;
    if (port == 3){
        if(a != out3_last){
            if ((a & 0x1) != (out3_last & 0x1)){
                if (!playing){
                    if(Mix_PlayChannel(0, sounds[0], -1)==-1) {
                        printf("Mix_PlayChannel: %s\n",Mix_GetError());
                    }
                    //printf("Sound 0\n");
                    playing=1;
                }else{
                    Mix_HaltChannel(0);
                    playing=0;
                }
            }
            if ((a & 0x2) != (out3_last & 0x2)){
                if(Mix_PlayChannel(-1, sounds[1], 0)==-1) {
                    printf("Mix_PlayChannel: %s\n",Mix_GetError());
                }
                //printf("Sound 1\n");
            }
            if ((a & 0x4) != (out3_last & 0x4)){
                if(Mix_PlayChannel(-1, sounds[2], 0)==-1) {
                    printf("Mix_PlayChannel: %s\n",Mix_GetError());
                }
                //printf("Sound 2\n");
            }
            if ((a & 0x8) != (out5_last & 0x8)){
                if(Mix_PlayChannel(-1, sounds[3], 0)==-1) {
                    printf("Mix_PlayChannel: %s\n",Mix_GetError());
                }
                //printf("Sound 3\n");
            }
            out3_last = a;
        }
    }else{
        if(a != out5_last){
            if ((a & 0x1) != (out5_last & 0x1)){
                if(Mix_PlayChannel(-1, sounds[4], 0)==-1) {
                    printf("Mix_PlayChannel: %s\n",Mix_GetError());
                }
                //printf("Sound 4\n");
            }
            if ((a & 0x2) != (out5_last & 0x2)){
                if(Mix_PlayChannel(-1, sounds[5], 0)==-1) {
                    printf("Mix_PlayChannel: %s\n",Mix_GetError());
                }
                //printf("Sound 5\n");
            }
            if ((a & 0x4) != (out5_last & 0x4)){
                if(Mix_PlayChannel(-1, sounds[6], 0)==-1) {
                    printf("Mix_PlayChannel: %s\n",Mix_GetError());
                }
                //printf("Sound 6\n");
            }
            if ((a & 0x8) != (out5_last & 0x8)){
                if(Mix_PlayChannel(-1, sounds[7], 0)==-1) {
                    printf("Mix_PlayChannel: %s\n",Mix_GetError());
                }
                //printf("Sound 7\n");
            }
            if ((a & 0x10) != (out5_last & 0x10)){
                if(Mix_PlayChannel(-1, sounds[8], 0)==-1) {
                    printf("Mix_PlayChannel: %s\n",Mix_GetError());
                }
                //printf("Sound 8\n");
            }
        }
            out5_last = a;
    }
}

void handle_keys(SDL_Event *e)
{
    if (e->type == SDL_KEYDOWN){
        switch(e->key.keysym.sym){
            case SDLK_c:
            {
                in_port1 |= 0x01;
                break;
            }
            case SDLK_2:
            {
                in_port1 |= 0x02;
                break;
            }
            case SDLK_1:
            {
                in_port1 |= 0x04;
                break;
            }
            case SDLK_a:
            {
                in_port1 |= 0x20;
                in_port2 |= 0x20;
                break;
            }
            case SDLK_j:
            {
                in_port1 |= 0x10;
                in_port2 |= 0x10;
                break;
            }
            case SDLK_d:
            {
                in_port1 |= 0x40;
                in_port2 |= 0x40;
                break;
            }
        }
    }else if (e->type == SDL_KEYUP){
        switch(e->key.keysym.sym){
            case SDLK_c:
            {
                in_port1 &= ~0x01;
                break;
            }
            case SDLK_2:
            {
                in_port1 &= ~0x02;
                break;
            }
            case SDLK_1:
            {
                in_port1 &= ~0x04;
                break;
            }
            case SDLK_a:
            {
                in_port1 &= ~0x20;
                in_port2 &= ~0x20;
                break;
            }
            case SDLK_d:
            {
                in_port1 &= ~0x40;
                in_port2 &= ~0x40;
                break;
            }
            case SDLK_j:
            {
                in_port1 &= ~0x10;
                in_port2 &= ~0x10;
                break;
            }
        }
    }
}
