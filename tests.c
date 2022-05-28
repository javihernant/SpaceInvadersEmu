#include <stdio.h> //putchar
#include "tests.h"
#include "machine.h"
#include "8080emu.h"

void test_cpu()
{
    uint8_t *buffer = load_rom("rom/cpudiag.bin", 0x100);
    State8080 st = StateCreat(buffer);
    st.pc = 0x100;
    st.memory[0x06] = 0xC9; //Add a RET instruction to return from the syscall

    st.memory[0x59d] = 0xc3; //JMP 0x05C2 to skip testing DAA
    st.memory[0x59e] = 0xc2;
    st.memory[0x59f] = 0x05;

    while(st.pc != 0x00){
        if (st.pc == 0x05){
            if (st.c == 2){
                putchar(st.e);
            }else if (st.c == 9){
                unsigned char *ch = &st.memory[(st.d << 8) | st.e];
                while(*ch != '$'){
                    putchar(*ch);
                    ch += 1;
                }
            }
            st.pc += 1;
        }else{
            Emulate8080Op(&st);
        }
    }
}
