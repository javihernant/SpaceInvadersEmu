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
    init_screen(vram,224, 256);
    SDL_Event e;

    
    unsigned int t0, t1, dt;
    int exed; //cycles already executed in current frame
    int interr = 1;

    int running = 1;
    t0 = SDL_GetTicks();
    while (running)
    {
        //Call Emulate until frame is done (how do I check for that?). Then delay for 16ms
        //For every frame, execute 33334 (2MHZ; 2000000 cycles per second; refresh rate 60HZ (60 fps); each frame
        //execute 2000000/60 cycles)

        exed = 0;
        while (exed < 33334){
            exed += Emulate8080Op(&state);
        }

        if (state.int_enable){
           gen_int(&state, interr);
           interr = interr == 1 ? 2 : 1;
        }

        
        while(SDL_PollEvent(&e) != 0){
			if(e.type == SDL_QUIT){
				running = 0;
			}
		}
        

        render_bf_2(vram);
        t1 = SDL_GetTicks();
        dt = t1-t0;

        if (dt <= 16){
            SDL_Delay(16-dt);
            t0 += 16;
        }else{
            //Too slow
            t0 = t1;
        }

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
