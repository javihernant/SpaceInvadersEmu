#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

int init_screen(unsigned char *vram, int width, int height);
int toggle_fs();
int render_bf_2(unsigned char *vram);
void screen_off();
int load_audio();


#endif
