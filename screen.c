#include <stdio.h> //printf
#include "screen.h"

static unsigned char *g_buffer;
static SDL_Window *g_window;
static SDL_Renderer *g_renderer;
static SDL_Texture *g_texture;
static int g_height = 256;
static int g_width = 224;
Mix_Chunk *sounds[9];

int bit2byte_parse(unsigned char *src);

int init_SDL()
{
    
	if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0){
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

    if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 1024 ) < 0 )
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        return 1;
    }
    //reserve channel 0 for ovni sound
    int reserved_count=Mix_ReserveChannels(1);
    if(reserved_count!=1) {
        printf("reserved %d channels from default mixing.\n",reserved_count);
    }

    return 0;
}

int load_audio()
{
    sounds[0] = Mix_LoadWAV( "sounds/30.wav" );
    if( sounds[0] == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        return 1;
    }

    sounds[1] = Mix_LoadWAV( "sounds/31.wav" );
    if( sounds[1] == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        return 1;
    }

    sounds[2] = Mix_LoadWAV( "sounds/32.wav" );
    if( sounds[2] == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        return 1;
    }

    sounds[3] = Mix_LoadWAV( "sounds/33.wav" );
    if( sounds[3] == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        return 1;
    }

    sounds[4] = Mix_LoadWAV( "sounds/50.wav" );
    if( sounds[4] == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        return 1;
    }

    sounds[5] = Mix_LoadWAV( "sounds/51.wav" );
    if( sounds[5] == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        return 1;
    }

    sounds[6] = Mix_LoadWAV( "sounds/52.wav" );
    if( sounds[6] == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        return 1;
    }

    sounds[7] = Mix_LoadWAV( "sounds/53.wav" );
    if( sounds[7] == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        return 1;
    }

    sounds[8] = Mix_LoadWAV( "sounds/54.wav" );
    if( sounds[8] == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        return 1;
    }
    return 0;
}


int init_screen(unsigned char *vram, int width, int height)
{
    g_width = width;
    g_height = height;
    if (init_SDL() != 0){
        exit(1);
    }
    if (load_audio() != 0){
        printf("Error loading audio!\n");
        exit(1);
    }
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
            byte = i;
            g_buffer[i*g_height + j] = (((byte & bit_mask) != 0) ? 0xff: 0x00);
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
            
            bit_mask = (1 << (j%8)) & 0xff;
            byte = src[i*g_height/8+j/8];
            //byte = i;
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

    for(int i=0; i<9; i++){
        Mix_FreeChunk( sounds[i] );
    }

    Mix_Quit();
    SDL_Quit();
    free(g_buffer);
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
