#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "8080emu.h"

int parity(int n, int size)
{
    int x = (n & ((1<<size) - 1));
    int p = 0;
    for (int i=0; i<size; i++){
        if (x&1){
            p++;
        }
        x = x>>1;
    }
    return (p&1) == 0;
}

ConditionCodes InitCC()
{
    ConditionCodes cc;
    cc.z = 0;
    cc.s = 0;
    cc.p = 0;
    cc.cy = 0;
    cc.ac = 0;
    cc.pad = 0;

    return cc;
}

State8080 StateCreat(uint8_t *buff)
{
    State8080 st;
    st.a = 0;
    st.b = 0;
    st.c = 0;
    st.d = 0;
    st.e = 0;
    st.h = 0;
    st.l = 0;
    st.sp = 0;
    st.pc = 0;
    st.cc = InitCC();
    st.memory = buff;
    st.int_enable = 0;

    return st;
}

void printState(State8080 *st)
{
    printf("A=%02x\tB=%02x\tC=%02x\tD=%02x\nE=%02x\tH=%02x\tL=%02x\tSP=%04x\nPC=%02x\tIE=%d\n",st->a, st->b, st->c, st->d, st->e, st->h, st->l, st->sp, st->pc,
    st->int_enable);
    ConditionCodes cc = st->cc;
    printf("Z=%d\tS=%d\tP=%d\tCY=%d\nAC=%d\tPAD=%d\n",cc.z, cc.s, cc.p, cc.cy, cc.ac, cc.pad);
 
}


//TODO: check that it works
void set_flags(ConditionCodes *cc, int result, int size, int flags){
    int mask = (1<<size)-1;
    result = (result & mask);
    if (flags & Z_FG){
        cc->z = (result & mask) == 0;
    }
    if (flags & S_FG){
        cc->s = ((result & mask) & (1<<(size-1))) != 0;
    }
    if (flags & P_FG){
        cc->p = parity((result & mask), size);
    }
    if (flags & CY_FG){
        cc->cy = (result & ~mask) != 0;
    }
    if (flags & AC_FG){
        //TODO
    }
}

////////////////////8080 instructions//////////////////

void UnimplementedInstruction(State8080 *state)
{
    //pc will have advanced one, so undo that
    state->pc -= 1;
    printf("Error: Unimplemented instruction\n");
    exit(1);
}
void RET(State8080 *st){

    st->pc = (st->memory[st->sp+1]<<8) | st->memory[st->sp];
    st->sp += 2;

}

void INX(State8080 *st, char x)
{
    uint16_t new;
    switch(x){
        case 'B':
        {
            new = (st->b<<8) | st->c;
            st->b = ((new+1)>>8) & 0xff;
            st->c = ((new+1)) & 0xff;
            break;
        }
        case 'D':
        {
            new = (st->d<<8) | st->e;
            st->d = ((new+1)>>8) & 0xff;
            st->e = ((new+1)) & 0xff;
            break;
        }
        case 'H':
        {
            new = (st->h<<8) | st->l;
            st->h = ((new+1)>>8) & 0xff;
            st->l = ((new+1)) & 0xff;
            break;
        }
        case 'S': //SP
        {
            st->sp+=1;
            break;
        }
    }
}

void DAD(State8080 *st, char x) 
{

    uint32_t hl = (st->h<<8) | st->l;
    uint32_t aux = 0;
    switch(x){
        case 'B':
        {
            aux = (st->b<<8) | st->c;
            break;
        }
        case 'D':
        {
            aux = (st->b<<8) | st->c;
            break;
        }
        case 'H':
        {
            aux = (st->b<<8) | st->c;
            break;
        }
        case 'S': //SP
        {
            aux = (st->b<<8) | st->c;
            break;
        }
    }

    uint32_t res = hl + aux;
    st->h = (res>>8)&0xff;
    st->l = res&0xff;
    set_flags(&st->cc, res, 16, CY_FG);
}
//////////////////////////////////////////////////////

void Emulate8080Op(State8080 *state)
{
    Disassemble8080Op(state->memory, state->pc);
    unsigned char *opcode = &state->memory[state->pc];
    state->pc += 1; 
    switch(*opcode)
    {
        case 0x00: break; break;
        case 0x01: 
            state->c = opcode[1];
            state->b = opcode[2];
            state->pc += 2;
            break;
        case 0x02: UnimplementedInstruction(state); break;
        case 0x03:
        {
            INX(state, 'B');
            break;
        }
        case 0x04: UnimplementedInstruction(state); break;
        case 0x05: 
        {
            uint8_t ans = state->b - 1;
            state->cc.z = ((ans&0xff) == 0);
            state->cc.s = ((ans&0x80) != 0);
            state->cc.p = parity(ans, 8);
            state->b = ans;
            break;
        }
        case 0x06:
        {
            state->b = opcode[1];
            state->pc += 1;
            break;
        }
        case 0x07: UnimplementedInstruction(state); break;
        case 0x08: UnimplementedInstruction(state); break;
        case 0x09:
        {
            DAD(state, 'B');
            break;
        }
          
        case 0x0a: UnimplementedInstruction(state); break;
        case 0x0b: UnimplementedInstruction(state); break;
        case 0x0c: UnimplementedInstruction(state); break;
        case 0x0d:
        {
            uint8_t res = state->c - 1;
            state->cc.z = (res&0xff) == 0;
            state->cc.s = (res&0x80) != 0;
            state->cc.p = parity(res&0xff,8);
            state->c = res;
            break;
        }
        case 0x0e:
        {
            state->c = opcode[1];
            state->pc += 1;
            break;
        }
        case 0x0f:
        {
            uint8_t bit0 = state->a&1;
            state->a = (state->a>>1) | bit0<<7;
            state->cc.cy = bit0;
            break;
        }
        case 0x10: UnimplementedInstruction(state); break;
        case 0x11: 
        {
            state->d = opcode[2];
            state->e = opcode[1];
            state->pc += 2;
            break;
        }
        case 0x12: UnimplementedInstruction(state); break;
        case 0x13:
        {
            INX(state, 'D');
            break;
        }
        case 0x14:
        {
            state->d += 1;
            set_flags(&state->cc, state->d, 8, Z_FG|S_FG|P_FG|CY_FG|AC_FG);
            break;
        }
        case 0x15: UnimplementedInstruction(state); break;
        case 0x16: UnimplementedInstruction(state); break;
        case 0x17: UnimplementedInstruction(state); break;
        case 0x18: UnimplementedInstruction(state); break;
        case 0x19:
        {
            DAD(state, 'D');
            break;
        }
        case 0x1a: 
        {
            uint16_t adr = (state->d<<8) | state->e;
            state->a = state->memory[adr];
            break;
        }
        case 0x1b: UnimplementedInstruction(state); break;
        case 0x1c: UnimplementedInstruction(state); break;
        case 0x1d: UnimplementedInstruction(state); break;
        case 0x1e: UnimplementedInstruction(state); break;
        case 0x1f: UnimplementedInstruction(state); break;
        case 0x20: UnimplementedInstruction(state); break;
        case 0x21: 
        {
            state->l = opcode[1];
            state->h = opcode[2];
            state->pc += 2;
            break;
        }
        case 0x22: UnimplementedInstruction(state); break;
        case 0x23: 
        {
            INX(state, 'H');
            break;
        }
        case 0x24: 
        {
            uint8_t ans = state->h + 1;
            state->cc.z = ((ans&0xff) == 0);
            state->cc.s = ((ans&0x80) != 0);
            state->cc.p = parity(ans, 8);
            state->h = ans;
            break;
        }
        case 0x25: UnimplementedInstruction(state); break;
        case 0x26: 
        {
            state->h = opcode[1];
            state->pc += 1;
            break;
        }
        case 0x27: UnimplementedInstruction(state); break;
        case 0x28: UnimplementedInstruction(state); break;
        case 0x29: 
        {
            DAD(state, 'H');
            break;
        }
        case 0x2a: UnimplementedInstruction(state); break;
        case 0x2b: UnimplementedInstruction(state); break;
        case 0x2c: UnimplementedInstruction(state); break;
        case 0x2d: UnimplementedInstruction(state); break;
        case 0x2e: UnimplementedInstruction(state); break;
        case 0x2f: UnimplementedInstruction(state); break;
        case 0x30: UnimplementedInstruction(state); break;
        case 0x31: 
        {
            state->sp = (opcode[2]<<8) | opcode[1];
            state->pc += 2;
            break;
        }
        case 0x32: 
        {
            uint16_t offset = (opcode[2]<<8) | opcode[1];
            state->memory[offset] = state->a;
            state->pc += 2;
            break;
        }
        case 0x33:
        {
            INX(state, 'S');
            break;
        }
        case 0x34: UnimplementedInstruction(state); break;
        case 0x35: UnimplementedInstruction(state); break;
        case 0x36:
        {
            uint16_t offset = (state->h<<8) | state->l;
            state->memory[offset] = opcode[1];
            state->pc += 1;
            break;
        }
        case 0x37: UnimplementedInstruction(state); break;
        case 0x38: UnimplementedInstruction(state); break;
        case 0x39:
        {
            DAD(state, 'S');
            break;
        }

        case 0x3a:
        {
            uint16_t offset = (opcode[2]<<8) | opcode[1];
            state->a = state->memory[offset];
            state->pc += 2;
            break;
        }
        case 0x3b: UnimplementedInstruction(state); break;
        case 0x3c: UnimplementedInstruction(state); break;
        case 0x3d: UnimplementedInstruction(state); break;
        case 0x3e: 
        {
            state->a = opcode[1];
            state->pc++;
            break;
        }
        case 0x3f: UnimplementedInstruction(state); break;
        case 0x40: state->b = state->b; break;
        case 0x41: state->b = state->c; break;
        case 0x42: state->b = state->d; break;
        case 0x43: state->b = state->e; break;
        case 0x44: state->b = state->h; break;
        case 0x45: state->b = state->l; break;
        case 0x46: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->b = state->memory[offset]; 
            break;
        }
        case 0x47: state->b = state->a; break;
        case 0x48: state->c = state->b; break;
        case 0x49: state->c = state->c; break;
        case 0x4a: state->c = state->d; break;
        case 0x4b: state->c = state->e; break;
        case 0x4c: state->c = state->h; break;
        case 0x4d: state->c = state->l; break;
        case 0x4e: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->c = state->memory[offset]; 
            break;
        }
        case 0x4f: state->c = state->a; break;
        case 0x50: state->d = state->b; break;
        case 0x51: state->d = state->c; break;
        case 0x52: state->d = state->d; break;
        case 0x53: state->d = state->e; break;
        case 0x54: state->d = state->h; break;
        case 0x55: state->d = state->l; break;
        case 0x56: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->d = state->memory[offset]; 
            break;
        }
        case 0x57: state->e = state->a; break;
        case 0x58: state->e = state->b; break;
        case 0x59: state->e = state->c; break;
        case 0x5a: state->e = state->d; break;
        case 0x5b: state->e = state->e; break;
        case 0x5c: state->e = state->h; break;
        case 0x5d: state->e = state->l; break;
        case 0x5e: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->e = state->memory[offset]; 
            break;
        }
        case 0x5f: state->h = state->a; break;
        case 0x60: state->h = state->b; break;
        case 0x61: state->h = state->c; break;
        case 0x62: state->h = state->d; break;
        case 0x63: state->h = state->e; break;
        case 0x64: state->h = state->h; break;
        case 0x65: state->h = state->l; break;
        case 0x66: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->h = state->memory[offset]; 
            break;
        }
        case 0x67: state->h = state->a; break;
        case 0x68: state->l = state->b; break;
        case 0x69: state->l = state->c; break;
        case 0x6a: state->l = state->d; break;
        case 0x6b: state->l = state->e; break;
        case 0x6c: state->l = state->h; break;
        case 0x6d: state->l = state->l; break;
        case 0x6e: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->l = state->memory[offset]; 
            break;
        }
        case 0x6f: state->l = state->a; break;
        case 0x70: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->memory[offset] = state->b;
            break;
        }
        case 0x71: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->memory[offset] = state->c;
            break;
        }
        case 0x72: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->memory[offset] = state->d;
            break;
        }
        case 0x73: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->memory[offset] = state->e;
            break;
        }
        case 0x74: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->memory[offset] = state->h;
            break;
        }
        case 0x75: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->memory[offset] = state->l;
            break;
        }
        case 0x76: UnimplementedInstruction(state); break;
        case 0x77: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->memory[offset] = state->a;
            break;
        }
        case 0x78: state->a = state->b; break;
        case 0x79: state->a = state->c; break;
        case 0x7a: state->a = state->d; break;
        case 0x7b: state->a = state->e; break;
        case 0x7c: state->a = state->h; break;
        case 0x7d: state->a = state->l; break;
        case 0x7e: 
        {
            uint16_t offset = (state->h<<8) | (state->l);
            state->a = state->memory[offset]; 
            break;
        }
        case 0x7f: state->a = state->a; break;
        case 0x80: UnimplementedInstruction(state); break;
        case 0x81: UnimplementedInstruction(state); break;
        case 0x82: UnimplementedInstruction(state); break;
        case 0x83: UnimplementedInstruction(state); break;
        case 0x84: UnimplementedInstruction(state); break;
        case 0x85: UnimplementedInstruction(state); break;
        case 0x86: UnimplementedInstruction(state); break;
        case 0x87: UnimplementedInstruction(state); break;
        case 0x88: UnimplementedInstruction(state); break;
        case 0x89: UnimplementedInstruction(state); break;
        case 0x8a: UnimplementedInstruction(state); break;
        case 0x8b: UnimplementedInstruction(state); break;
        case 0x8c: UnimplementedInstruction(state); break;
        case 0x8d: UnimplementedInstruction(state); break;
        case 0x8e: UnimplementedInstruction(state); break;
        case 0x8f: UnimplementedInstruction(state); break;
        case 0x90: UnimplementedInstruction(state); break;
        case 0x91: UnimplementedInstruction(state); break;
        case 0x92: UnimplementedInstruction(state); break;
        case 0x93: UnimplementedInstruction(state); break;
        case 0x94: UnimplementedInstruction(state); break;
        case 0x95: UnimplementedInstruction(state); break;
        case 0x96: UnimplementedInstruction(state); break;
        case 0x97: UnimplementedInstruction(state); break;
        case 0x98: UnimplementedInstruction(state); break;
        case 0x99: UnimplementedInstruction(state); break;
        case 0x9a: UnimplementedInstruction(state); break;
        case 0x9b: UnimplementedInstruction(state); break;
        case 0x9c: UnimplementedInstruction(state); break;
        case 0x9d: UnimplementedInstruction(state); break;
        case 0x9e: UnimplementedInstruction(state); break;
        case 0x9f: UnimplementedInstruction(state); break;
        case 0xa0:
        {
            state->a = state->a & state->b;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xa1:
        {
            state->a = state->a & state->c;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xa2:
        {
            state->a = state->a & state->d;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xa3:
        {
            state->a = state->a & state->e;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xa4:
        {
            state->a = state->a & state->h;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xa5:
        {
            state->a = state->a & state->l;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xa6: UnimplementedInstruction(state); break;
        case 0xa7:
        {
            state->a = state->a & state->a;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xa8: 
        {
            state->a = state->a ^ state->b;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xa9: 
        {
            state->a = state->a ^ state->c;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xaa:
        {
            state->a = state->a ^ state->d;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xab: 
        {
            state->a = state->a ^ state->e;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xac:
        {
            state->a = state->a ^ state->h;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xad:
        {
            state->a = state->a ^ state->l;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xae: UnimplementedInstruction(state); break;
        case 0xaf: 
        {
            state->a = state->a ^ state->a;
            state->cc.z = ((state->a&0xff) == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = state->cc.ac = 0;
            break;
        }
        case 0xb0: UnimplementedInstruction(state); break;
        case 0xb1: UnimplementedInstruction(state); break;
        case 0xb2: UnimplementedInstruction(state); break;
        case 0xb3: UnimplementedInstruction(state); break;
        case 0xb4: UnimplementedInstruction(state); break;
        case 0xb5: UnimplementedInstruction(state); break;
        case 0xb6: UnimplementedInstruction(state); break;
        case 0xb7: UnimplementedInstruction(state); break;
        case 0xb8: UnimplementedInstruction(state); break;
        case 0xb9: UnimplementedInstruction(state); break;
        case 0xba: UnimplementedInstruction(state); break;
        case 0xbb: UnimplementedInstruction(state); break;
        case 0xbc: UnimplementedInstruction(state); break;
        case 0xbd: UnimplementedInstruction(state); break;
        case 0xbe: UnimplementedInstruction(state); break;
        case 0xbf: UnimplementedInstruction(state); break;
        case 0xc0: UnimplementedInstruction(state); break;
        case 0xc1:
        {
            state->b = state->memory[state->sp+1];
            state->c = state->memory[state->sp];
            state->sp += 2;
            break;
        }
        case 0xc2:
        {
            if (state->cc.z == 0) {
                state->pc = (opcode[2]<<8) | opcode[1];
            }else{
                state->pc += 2;
            }
            break;
        }
        case 0xc3: 
        {
            uint16_t adr = (opcode[2] << 8)| opcode[1];
            state->pc = adr;
            break;
        }
        case 0xc4: UnimplementedInstruction(state); break;
        case 0xc5:
        {
            state->memory[state->sp-1] = state->b;
            state->memory[state->sp-2] = state->c;
            state->sp -= 2;
            break;
        }
        case 0xc6:
        {
            uint16_t res = state->a + opcode[1];
            state->cc.z = (res == 0);
            state->cc.s = ((res&0x80) != 0);
            state->cc.p = parity(res,8);
            state->cc.cy = (res > (res&0xff));
            state->cc.ac = 0;
            state->a = res;
            state->pc++;
            break;
        }
        case 0xc7: UnimplementedInstruction(state); break;
        case 0xc8: UnimplementedInstruction(state); break;
        case 0xc9: 
        {
            RET(state);
            break;
        }
        case 0xca: UnimplementedInstruction(state); break;
        case 0xcb: UnimplementedInstruction(state); break;
        case 0xcc: UnimplementedInstruction(state); break;
        case 0xcd:
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp - 2] = (ret & 0xff);
            state->memory[state->sp - 1] = ((ret>>8) & 0xff);
            state->sp -= 2;

            state->pc = (opcode[2]<<8) | opcode[1];
            break;
        }
        case 0xce: UnimplementedInstruction(state); break;
        case 0xcf: UnimplementedInstruction(state); break;
        case 0xd0: 
        {
            if (state->cc.cy == 0){
                RET(state);
            }
            break;
        }
        case 0xd1:
        {
            state->d = state->memory[state->sp+1];
            state->e = state->memory[state->sp];
            state->sp += 2;
            break;
        }
        case 0xd2: UnimplementedInstruction(state); break;
        case 0xd3: 
        {
            state->pc++;
            break;
        }
        case 0xd4: UnimplementedInstruction(state); break;
        case 0xd5:
        {
            state->memory[state->sp-1] = state->d;
            state->memory[state->sp-2] = state->e;
            state->sp -= 2;
            break;
        }
        case 0xd6: UnimplementedInstruction(state); break;
        case 0xd7: UnimplementedInstruction(state); break;
        case 0xd8: UnimplementedInstruction(state); break;
        case 0xd9: UnimplementedInstruction(state); break;
        case 0xda: UnimplementedInstruction(state); break;
        case 0xdb: UnimplementedInstruction(state); break;
        case 0xdc: UnimplementedInstruction(state); break;
        case 0xdd: UnimplementedInstruction(state); break;
        case 0xde: UnimplementedInstruction(state); break;
        case 0xdf: UnimplementedInstruction(state); break;
        case 0xe0: UnimplementedInstruction(state); break;
        case 0xe1:
        {
            state->h = state->memory[state->sp+1];
            state->l = state->memory[state->sp];
            state->sp += 2;
            break;
        }
        case 0xe2: UnimplementedInstruction(state); break;
        case 0xe3: UnimplementedInstruction(state); break;
        case 0xe4: UnimplementedInstruction(state); break;
        case 0xe5:
        {
            state->memory[state->sp-1] = state->h;
            state->memory[state->sp-2] = state->l;
            state->sp -= 2;
            break;
        }
        case 0xe6:
        {
            state->a = state->a & opcode[1];
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a&0x80) != 0);
            state->cc.p = parity(state->a,8);
            state->cc.cy = state->cc.ac = 0;
            state->pc++;
            break;
        }
        case 0xe7: UnimplementedInstruction(state); break;
        case 0xe8: UnimplementedInstruction(state); break;
        case 0xe9: UnimplementedInstruction(state); break;
        case 0xea: UnimplementedInstruction(state); break;
        case 0xeb: 
        {
            uint8_t tmp1 = state->d;
            uint8_t tmp2 = state->e;
            state->d = state->h;
            state->e = state->l;
            state->h = tmp1;
            state->l = tmp2;
            break;
        }
        case 0xec: UnimplementedInstruction(state); break;
        case 0xed: UnimplementedInstruction(state); break;
        case 0xee: UnimplementedInstruction(state); break;
        case 0xef: UnimplementedInstruction(state); break;
        case 0xf0: UnimplementedInstruction(state); break;
        case 0xf1:
        {
            uint8_t cc = state->memory[state->sp];
            state->a = state->memory[state->sp+1];

            state->cc.z = (cc)&1;
            state->cc.s = (cc>>1)&1;
            state->cc.p = (cc>>2)&1;
            state->cc.cy = (cc>>3)&1;
            state->cc.ac = (cc>>4)&1;

            state->sp += 2;
            break;
        }
        case 0xf2: UnimplementedInstruction(state); break;
        case 0xf3: UnimplementedInstruction(state); break;
        case 0xf4: UnimplementedInstruction(state); break;
        case 0xf5: 
        {
            uint8_t psw = (state->cc.z) | (state->cc.s<<1) | (state->cc.p<<2) | (state->cc.cy<<3) |
            (state->cc.ac<<4); 
            state->memory[state->sp-1] = state->a;
            state->memory[state->sp-2] = psw;
            state->sp -= 2;
            break;
        }
        case 0xf6: UnimplementedInstruction(state); break;
        case 0xf7: UnimplementedInstruction(state); break;
        case 0xf8: UnimplementedInstruction(state); break;
        case 0xf9: UnimplementedInstruction(state); break;
        case 0xfa: UnimplementedInstruction(state); break;
        case 0xfb: 
        {
            state->int_enable = 1;
            break;
        }
        case 0xfc: UnimplementedInstruction(state); break;
        case 0xfd: UnimplementedInstruction(state); break;
        case 0xfe:
        {
            uint16_t res = state->a - opcode[1];
            state->cc.z = (res&0xff) == 0;
            state->cc.s = (res&0x80) != 0;
            state->cc.p = parity(res&0xff,8);
            state->cc.cy = state->a < opcode[1];
            state->pc += 1;
            break;
        }
        case 0xff: UnimplementedInstruction(state); break;
    }

}


/*    
    *codebuffer is a valid pointer to 8080 assembly code    
    pc is the current offset into the code    

    returns the number of bytes of the op    
*/    

int Disassemble8080Op(unsigned char *codebuffer, int pc)    
{    
    unsigned char *code = &codebuffer[pc];    
    int opbytes = 1;    
    printf ("%04x ", pc);    
    switch (*code)    
    {    
        case 0x00: printf("NOP"); break;    
        case 0x01: printf("LXI    B,#$%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x02: printf("STAX   B"); break;    
        case 0x03: printf("INX    B"); break;    
        case 0x04: printf("INR    B"); break;    
        case 0x05: printf("DCR    B"); break;    
        case 0x06: printf("MVI    B,#$%02x", code[1]); opbytes=2; break;    
        case 0x07: printf("RLC"); break;    
        case 0x08: printf("NOP"); break;    
        case 0x09: printf("DAD    B"); break;    
        case 0x0a: printf("LDAX   B"); break;    
        case 0x0b: printf("DCX    B"); break;    
        case 0x0c: printf("INR    C"); break;    
        case 0x0d: printf("DCR    C"); break;    
        case 0x0e: printf("MVI    C,#$%02x", code[1]); opbytes=2; break;    
        case 0x0f: printf("RRC"); break;    
        case 0x10: printf("NOP"); break;    
        case 0x11: printf("LXI    D,#$%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x12: printf("STAX   D"); break;    
        case 0x13: printf("INX    D"); break;    
        case 0x14: printf("INR    D"); break;    
        case 0x15: printf("DCR    D"); break;    
        case 0x16: printf("MVI    D,#$%02x", code[1]); opbytes=2; break;    
        case 0x17: printf("RAL"); break;    
        case 0x18: printf("NOP"); break;    
        case 0x19: printf("DAD    D"); break;    
        case 0x1a: printf("LDAX   D"); break;    
        case 0x1b: printf("DCX    D"); break;    
        case 0x1c: printf("INR    E"); break;    
        case 0x1d: printf("DCR    E"); break;    
        case 0x1e: printf("MVI    E,#$%02x", code[1]); opbytes=2; break;    
        case 0x1f: printf("RAR"); break;    
        case 0x20: printf("RIM"); break;    
        case 0x21: printf("LXI    H,#$%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x22: printf("SHLD   $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x23: printf("INX    H"); break;    
        case 0x24: printf("INR    H"); break;    
        case 0x25: printf("DCR    H"); break;    
        case 0x26: printf("MVI    H,#$%02x", code[1]); opbytes=2; break;    
        case 0x27: printf("DAA"); break;    
        case 0x28: printf("NOP"); break;    
        case 0x29: printf("DAD    H"); break;    
        case 0x2a: printf("LHLD   $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x2b: printf("DCX    H"); break;    
        case 0x2c: printf("INR    L"); break;    
        case 0x2d: printf("DCR    L"); break;    
        case 0x2e: printf("MVI    L,#$%02x", code[1]); opbytes=2; break;    
        case 0x2f: printf("CMA"); break;    
        case 0x30: printf("SIM"); break;    
        case 0x31: printf("LXI    SP,#$%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x32: printf("STA    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x33: printf("INX    SP"); break;    
        case 0x34: printf("INR    M"); break;    
        case 0x35: printf("DCR    M"); break;    
        case 0x36: printf("MVI    M,#$%02x", code[1]); opbytes=2; break;    
        case 0x37: printf("STC"); break;    
        case 0x38: printf("NOP"); break;    
        case 0x39: printf("DAD    SP"); break;    
        case 0x3a: printf("LDA    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x3b: printf("DCX    SP"); break;    
        case 0x3c: printf("INR    A"); break;    
        case 0x3d: printf("DCR    A"); break;    
        case 0x3e: printf("MVI    A,#$%02x", code[1]); opbytes=2; break;    
        case 0x3f: printf("CMC"); break;    
        case 0x40: printf("MOV    B,B"); break;    
        case 0x41: printf("MOV    B,C"); break;    
        case 0x42: printf("MOV    B,D"); break;    
        case 0x43: printf("MOV    B,E"); break;    
        case 0x44: printf("MOV    B,H"); break;    
        case 0x45: printf("MOV    B,L"); break;    
        case 0x46: printf("MOV    B,M"); break;    
        case 0x47: printf("MOV    B,A"); break;    
        case 0x48: printf("MOV    C,B"); break;    
        case 0x49: printf("MOV    C,C"); break;    
        case 0x4a: printf("MOV    C,D"); break;    
        case 0x4b: printf("MOV    C,E"); break;    
        case 0x4c: printf("MOV    C,H"); break;    
        case 0x4d: printf("MOV    C,L"); break;    
        case 0x4e: printf("MOV    C,M"); break;    
        case 0x4f: printf("MOV    C,A"); break;    
        case 0x50: printf("MOV    D,B"); break;    
        case 0x51: printf("MOV    D,C"); break;    
        case 0x52: printf("MOV    D,D"); break;    
        case 0x53: printf("MOV    D,E"); break;    
        case 0x54: printf("MOV    D,H"); break;    
        case 0x55: printf("MOV    D,L"); break;    
        case 0x56: printf("MOV    D,M"); break;    
        case 0x57: printf("MOV    D,A"); break;    
        case 0x58: printf("MOV    E,B"); break;    
        case 0x59: printf("MOV    E,C"); break;    
        case 0x5a: printf("MOV    E,D"); break;    
        case 0x5b: printf("MOV    E,E"); break;    
        case 0x5c: printf("MOV    E,H"); break;    
        case 0x5d: printf("MOV    E,L"); break;    
        case 0x5e: printf("MOV    E,M"); break;    
        case 0x5f: printf("MOV    E,A"); break;    
        case 0x60: printf("MOV    H,B"); break;    
        case 0x61: printf("MOV    H,C"); break;    
        case 0x62: printf("MOV    H,D"); break;    
        case 0x63: printf("MOV    H,E"); break;    
        case 0x64: printf("MOV    H,H"); break;    
        case 0x65: printf("MOV    H,L"); break;    
        case 0x66: printf("MOV    H,M"); break;    
        case 0x67: printf("MOV    H,A"); break;    
        case 0x68: printf("MOV    L,B"); break;    
        case 0x69: printf("MOV    L,C"); break;    
        case 0x6a: printf("MOV    L,D"); break;    
        case 0x6b: printf("MOV    L,E"); break;    
        case 0x6c: printf("MOV    L,H"); break;    
        case 0x6d: printf("MOV    L,L"); break;    
        case 0x6e: printf("MOV    L,M"); break;    
        case 0x6f: printf("MOV    L,A"); break;    
        case 0x70: printf("MOV    M,B"); break;    
        case 0x71: printf("MOV    M,C"); break;    
        case 0x72: printf("MOV    M,D"); break;    
        case 0x73: printf("MOV    M,E"); break;    
        case 0x74: printf("MOV    M,H"); break;    
        case 0x75: printf("MOV    M,L"); break;    
        case 0x76: printf("HLT"); break;    
        case 0x77: printf("MOV    M,A"); break;    
        case 0x78: printf("MOV    A,B"); break;    
        case 0x79: printf("MOV    A,C"); break;    
        case 0x7a: printf("MOV    A,D"); break;    
        case 0x7b: printf("MOV    A,E"); break;    
        case 0x7c: printf("MOV    A,H"); break;    
        case 0x7d: printf("MOV    A,L"); break;    
        case 0x7e: printf("MOV    A,M"); break;    
        case 0x7f: printf("MOV    A,A"); break;    
        case 0x80: printf("ADD    B"); break;    
        case 0x81: printf("ADD    C"); break;    
        case 0x82: printf("ADD    D"); break;    
        case 0x83: printf("ADD    E"); break;    
        case 0x84: printf("ADD    H"); break;    
        case 0x85: printf("ADD    L"); break;    
        case 0x86: printf("ADD    M"); break;    
        case 0x87: printf("ADD    A"); break;    
        case 0x88: printf("ADC    B"); break;    
        case 0x89: printf("ADC    C"); break;    
        case 0x8a: printf("ADC    D"); break;    
        case 0x8b: printf("ADC    E"); break;    
        case 0x8c: printf("ADC    H"); break;    
        case 0x8d: printf("ADC    L"); break;    
        case 0x8e: printf("ADC    M"); break;    
        case 0x8f: printf("ADC    A"); break;    
        case 0x90: printf("SUB    B"); break;    
        case 0x91: printf("SUB    C"); break;    
        case 0x92: printf("SUB    D"); break;    
        case 0x93: printf("SUB    E"); break;    
        case 0x94: printf("SUB    H"); break;    
        case 0x95: printf("SUB    L"); break;    
        case 0x96: printf("SUB    M"); break;    
        case 0x97: printf("SUB    A"); break;    
        case 0x98: printf("SBB    B"); break;    
        case 0x99: printf("SBB    C"); break;    
        case 0x9a: printf("SBB    D"); break;    
        case 0x9b: printf("SBB    E"); break;    
        case 0x9c: printf("SBB    H"); break;    
        case 0x9d: printf("SBB    L"); break;    
        case 0x9e: printf("SBB    M"); break;    
        case 0x9f: printf("SBB    A"); break;    
        case 0xa0: printf("ANA    B"); break;    
        case 0xa1: printf("ANA    C"); break;    
        case 0xa2: printf("ANA    D"); break;    
        case 0xa3: printf("ANA    E"); break;    
        case 0xa4: printf("ANA    H"); break;    
        case 0xa5: printf("ANA    L"); break;    
        case 0xa6: printf("ANA    M"); break;    
        case 0xa7: printf("ANA    A"); break;    
        case 0xa8: printf("XRA    B"); break;    
        case 0xa9: printf("XRA    C"); break;    
        case 0xaa: printf("XRA    D"); break;    
        case 0xab: printf("XRA    E"); break;    
        case 0xac: printf("XRA    H"); break;    
        case 0xad: printf("XRA    L"); break;    
        case 0xae: printf("XRA    M"); break;    
        case 0xaf: printf("XRA    A"); break;    
        case 0xb0: printf("ORA    B"); break;    
        case 0xb1: printf("ORA    C"); break;    
        case 0xb2: printf("ORA    D"); break;    
        case 0xb3: printf("ORA    E"); break;    
        case 0xb4: printf("ORA    H"); break;    
        case 0xb5: printf("ORA    L"); break;    
        case 0xb6: printf("ORA    M"); break;    
        case 0xb7: printf("ORA    A"); break;    
        case 0xb8: printf("CMP    B"); break;    
        case 0xb9: printf("CMP    C"); break;    
        case 0xba: printf("CMP    D"); break;    
        case 0xbb: printf("CMP    E"); break;    
        case 0xbc: printf("CMP    H"); break;    
        case 0xbd: printf("CMP    L"); break;    
        case 0xbe: printf("CMP    M"); break;    
        case 0xbf: printf("CMP    A"); break;    
        case 0xc0: printf("RNZ"); break;    
        case 0xc1: printf("POP    B"); break;    
        case 0xc2: printf("JNZ    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xc3: printf("JMP    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xc4: printf("CNZ    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xc5: printf("PUSH   B"); break;    
        case 0xc6: printf("ADI    #$%02x", code[1]); opbytes=2; break;    
        case 0xc7: printf("RST    0"); break;    
        case 0xc8: printf("RZ"); break;    
        case 0xc9: printf("RET"); break;    
        case 0xca: printf("JZ     $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xcb: printf("NOP"); break;    
        case 0xcc: printf("CZ     $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xcd: printf("CALL   $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xce: printf("ACI    #$%02x", code[1]); opbytes=2; break;    
        case 0xcf: printf("RST    1"); break;    
        case 0xd0: printf("RNC"); break;    
        case 0xd1: printf("POP    D"); break;    
        case 0xd2: printf("JNC    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xd3: printf("OUT    #$%02x", code[1]); opbytes=2; break;    
        case 0xd4: printf("CNC    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xd5: printf("PUSH   D"); break;    
        case 0xd6: printf("SUI    #$%02x", code[1]); opbytes=2; break;    
        case 0xd7: printf("RST    2"); break;    
        case 0xd8: printf("RC"); break;    
        case 0xd9: printf("NOP"); break;    
        case 0xda: printf("JC     $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xdb: printf("IN     #$%02x", code[1]); opbytes=2; break;    
        case 0xdc: printf("CC     $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xdd: printf("NOP"); break;    
        case 0xde: printf("SBI    #$%02x", code[1]); opbytes=2; break;    
        case 0xdf: printf("RST    3"); break;    
        case 0xe0: printf("RPO"); break;    
        case 0xe1: printf("POP    H"); break;    
        case 0xe2: printf("JPO    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xe3: printf("XTHL"); break;    
        case 0xe4: printf("CPO    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xe5: printf("PUSH   H"); break;    
        case 0xe6: printf("ANI    #$%02x", code[1]); opbytes=2; break;    
        case 0xe7: printf("RST    4"); break;    
        case 0xe8: printf("RPE"); break;    
        case 0xe9: printf("PCHL"); break;    
        case 0xea: printf("JPE    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xeb: printf("XCHG"); break;    
        case 0xec: printf("CPE    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xed: printf("NOP"); break;    
        case 0xee: printf("XRI    #$%02x", code[1]); opbytes=2; break;    
        case 0xef: printf("RST    5"); break;    
        case 0xf0: printf("RP"); break;    
        case 0xf1: printf("POP    PSW"); break;    
        case 0xf2: printf("JP     $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xf3: printf("DI"); break;    
        case 0xf4: printf("CP     $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xf5: printf("PUSH   PSW"); break;    
        case 0xf6: printf("ORI    #$%02x", code[1]); opbytes=2; break;    
        case 0xf7: printf("RST    6"); break;    
        case 0xf8: printf("RM"); break;    
        case 0xf9: printf("SPHL"); break;    
        case 0xfa: printf("JM     $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xfb: printf("EI"); break;    
        case 0xfc: printf("CM     $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xfd: printf("NOP"); break;    
        case 0xfe: printf("CPI    #$%02x", code[1]); opbytes=2; break;    
        case 0xff: printf("RST    7"); break;    
   }
        
    printf("\n");    

    return opbytes;    
}   

