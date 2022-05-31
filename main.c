#include <stdlib.h>
#include <stdio.h>
#include "8080emu.h"
#include "machine.h"
#include <stdint.h>
#include "screen.h"
#include "tests.h" //test_cpu()
#include <unistd.h> //getopt()

void help_msg()
{
    printf("Commands:\n");
    printf("\t -t: perform tests\n");
    printf("\t -f <rom>: load rom\n");
}

void run_cpu(char *path)
{
    unsigned char *buffer = load_rom(path, 0x00);
    unsigned char *vram = &buffer[0x2400];

    State8080 state = StateCreat(buffer);
    init_screen(vram, 256, 224);
    SDL_Event e;

    unsigned int last_time = 0;
    unsigned int curr_time;
    int interr = 1;

    int running = 1;
    while (running)
    {
        curr_time = SDL_GetTicks();
        if (curr_time>last_time + 16)
        {
            if (state.int_enable){
                last_time = curr_time;
                gen_int(&state, interr);
                interr = interr == 1 ? 2 : 1;
                //render_bf_2(vram);
            }
        }

		while(SDL_PollEvent(&e) != 0){
			if(e.type == SDL_QUIT){
				running = 0;
			}
		}

        Emulate8080Op(&state);

        //printState(&state);
    }

    screen_off();
    free(buffer);
}

int main (int argc, char **argv)
{
    char *file_pth = NULL;
    int test_fg = 0;
    char c;
    while((c = getopt(argc, argv, "tf:")) != -1) {

        switch(c) {
            case 't':
                test_fg = 1;
                break;
            case 'f':
                file_pth = optarg;
                break;
        }
    }

    if (test_fg){
        //TODO:test rom load
        test_cpu();
    }else if (file_pth != NULL){
        run_cpu(file_pth);
    }else{
        help_msg();
    }

    return 0;
}
