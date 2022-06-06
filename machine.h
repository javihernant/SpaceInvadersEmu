#ifndef MACHINE_H
#define MACHINE_H
#include "regs.h"
#include "SDL2/SDL.h"

void machine_in(State8080 *st, int port);
void machine_out(State8080 *st, int port);
unsigned char *load_rom(char *path, int offset);
void gen_int(State8080 *st, int num);
void handle_keys(SDL_Event *e);
#endif
    
