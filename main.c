#include <stdlib.h>
#include <stdio.h>
#include "8080emu.h"
#include "machine.h"
#include <stdint.h>
#include "screen.h"
#include "tests.h"
#include <unistd.h> //getopt()

void help_msg()
{
    printf("Commands:\n");
    printf("\t -t: perform tests\n");
    printf("\t -f <rom>: load rom\n");
}

void run_cpu(char *path)
{
    unsigned char *buffer = init_machine(path);
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

        Emulate8080Op(&state);

        //printState(&state);
        render_bf_2(vram);
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
