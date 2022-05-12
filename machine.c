#include "regs.h"
#include "machine.h"
#include <stdint.h> 

static uint8_t shift_offset;
static uint8_t shift_val = 0;

void machine_in(State8080 *st, int port)
{
    switch(port){
        case 0:
        {
            st->a = 1;
            break;
        }
        case 1:
        {
            st->a = 0;
            break;
        }
        case 3:
        {
            st->a = shift_val >> (8-shift_offset) ;
        }
        break;
    }
}

void machine_out(State8080 *st, int port)
{
    switch(port){
        case 2:
        {
            shift_offset = (st->a);
        }
        break;

        case 4:
        {
            shift_val = (st->a)<<8 | ((shift_val >> 8) & 0xff); 
        }
        break;
    }
}
