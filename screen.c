#include <stdio.h> //printf
#include "screen.h"

static unsigned char *g_buffer;
static SDL_Window *g_window;
static SDL_Renderer *g_renderer;
//static SDL_Event g_e;
//static SDL_Surface* g_surface;
static SDL_Texture *g_texture;
//const static int g_width = 256;
//const static int g_height = 224;
static int g_width = 256;
static int g_height = 224;

int bit2byte_parse(unsigned char *src);

int init_SDL()
{
    
	if (SDL_Init( SDL_INIT_VIDEO ) < 0){
		printf("Error initializing SDL! SDL_Error:%s", SDL_GetError() );
		return 1;
	} 

	g_window = SDL_CreateWindow("Space Invaders Emulator",
        SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, g_width, g_height,
        SDL_WINDOW_SHOWN);


	if( g_window == NULL )
	{
		printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
		return 1;
	}

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (g_renderer == NULL){
        printf("Renderer could not be initialized! SDL_error:%s\n", SDL_GetError());
        return 1;
    }

    SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF );

    return 0;
}

int init_video()
{
    init_SDL();
    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGB332,
    SDL_TEXTUREACCESS_STREAMING, g_width, g_height);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    return 0;
}


int init_screen(unsigned char *vram, int width, int height)
{
    g_width = width;
    g_height = height;
    init_SDL();
    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGB332,
        SDL_TEXTUREACCESS_STREAMING, g_width, g_height);
    //SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);

    g_buffer = malloc (width*height);

 
    return 0;
}

int render_bf_2(unsigned char *vram)
{
    bit2byte_parse(vram);
    SDL_UpdateTexture(g_texture, NULL, (void *) g_buffer, g_width);
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);

    return 0;
}

int render_bf()
{
    SDL_UpdateTexture(g_texture, NULL, g_buffer, g_width);
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);

    return 0;
}

void paint_at(unsigned char *vram, int x, int y, int ncols)
{
    vram[y*ncols+x] = 0xff;
}

//recreate vram
void test_draw(unsigned char *src)
{
    for (int j=0; j<g_width; j++){
        paint_at(src, g_height/8-1, j, g_height/8);
    }
    /* 
    for (int i=0; i<=g_height; i++){
        paint_at(vram, i, 11, g_height);
    }
    for (int i=0; i<=g_height; i++){
        paint_at(vram, i, 12, g_height);
    }
    */
}

int bit2byte_parse_test(unsigned char *src)
{
    unsigned char byte;
    int bit_mask;

    for (int i=0; i<g_width; i++)
    {
        for (int j=0; j<g_height; j++){

            bit_mask = (1 << (7-(j%8))) & 0xff;
            //byte = src[((g_width-1-j)*g_height)/8+(g_height-1-i)/8];
            byte = src[i*g_height/8+j/8];
            g_buffer[i*g_height + j] = (byte & bit_mask) != 0 ? 0xff: 0x00;
        }

    }

    return 0;
}
//dest has wxh bytes. src has (wxh)/8 bytes.
int bit2byte_parse(unsigned char *src)
{
    unsigned char byte;
    int bit_mask;
    //go through vram, which is transposed (hxw)
    for (int i=0; i<g_width; i++)
    {
        for (int j=0; j<g_height; j++){
            
            bit_mask = (1 << (7-(j%8))) & 0xff;
            byte = src[i*g_height/8+j/8];
            g_buffer[((g_height-1-j)*g_width) + i] = (byte & bit_mask) != 0 ? 0xff: 0x00;
        }

    }
    return 0;
}

void screen_off()
{
    SDL_DestroyTexture(g_texture);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
    //free(buffer);
}

/*
int main()
{

    unsigned char *src = calloc(g_width*g_height/8, 1);
    test_draw(src);
    g_buffer = malloc(g_width*g_height);
    //bit2byte_parse_test(src);
    bit2byte_parse(src);
    init_video();
    render_bf();

    int running = 1;
    while (running)
    {

        while(SDL_PollEvent(&g_e) != 0){
            if(g_e.type == SDL_QUIT){
                running = 0;
            }
        }

    }

    SDL_DestroyTexture(g_texture);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return 0;
}
*/
