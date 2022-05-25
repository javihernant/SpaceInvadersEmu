#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "8080emu.h"
#include "machine.h" //IN and OUT

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
    //"PC=%04X SP=%04X AF=%04X BC=%04X DE=%04X HL=%04X INTE=%01X"
    //trace inv_regs.tr,0,noloop,{tracelog "PC=%04X SP=%04X AF=%04X BC=%04X DE=%04X HL=%04X INTE=%d",PC,SP,AF,BC,DE,HL,INTE}
 
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

void set_cy_sub(ConditionCodes *cc, int a, int b)
{
    if (b > a){
        cc->cy = 1;
    }else{
        cc->cy = 0;
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

void CPI(State8080 *st, uint8_t byte)
{
    uint16_t res = st->a - byte;
    set_flags(&st->cc, res, 8, Z_FG|S_FG|P_FG);
    set_cy_sub(&st->cc, st->a, byte);
        
    st->pc += 1;
}

void SUB(State8080 *st, uint8_t byte)
{
    uint16_t res = st->a - byte;
    set_flags(&st->cc, res, 8, Z_FG|S_FG|P_FG);
    set_cy_sub(&st->cc, st->a, byte);
    
    st->a = res & 0xff;
    st->pc += 1;
}

void SBB(State8080 *st, uint8_t byte)
{
    uint16_t res = st->a - byte - st->cc.cy;
    set_flags(&st->cc, res, 8, Z_FG|S_FG|P_FG);
    set_cy_sub(&st->cc, st->a, byte);

    st->a = res & 0xff;
    st->pc += 1;
}

void MVI(uint8_t *reg, uint8_t val, uint16_t *pc)
{
            *reg = val;
            *pc += 1;
}

void RET(State8080 *st)
{

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
            st->l = (new+1) & 0xff;
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

void LXI(uint8_t *h, uint8_t *l, uint8_t new_h, uint8_t new_l, uint16_t *pc)
{
    *h = new_h;
    *l = new_l;
    *pc += 2;
}


void LXI_SP(uint16_t *sp, uint8_t sp_h, uint8_t sp_l, uint16_t *pc)
{
    *sp = (sp_h<<8)|sp_l;
    *pc += 2;
}

void DCR(uint8_t *reg, ConditionCodes *cc){
    *reg -= 1;
    set_flags(cc, *reg, 8, Z_FG|S_FG|P_FG|AC_FG);
}


void PUSH(uint8_t h, uint8_t l, uint8_t *mem, uint16_t *sp)
{
    mem[*sp-1] = h;
    mem[*sp-2] = l;
    *sp -= 2;
}

void DI(uint8_t *ints)
{
    *ints = 0;    
}

void EI(uint8_t *ints)
{
    *ints = 1;    
}

void JMP(State8080 *st, uint16_t addr)
{
    st->pc = addr;
}
 
void JZ(State8080 *st, uint16_t addr)
{
    if (st->cc.z){
        st->pc = addr;
    }else{
        st->pc += 2;
    }
}

void JNC(State8080 *st, uint16_t addr)
{
    if (st->cc.cy == 0){
        st->pc = addr;
    }else{
        st->pc += 2;
    }
}

void JPE(State8080 *st, uint16_t addr)
{
    if (st->cc.p){
        st->pc = addr;
    }else{
        st->pc += 2;
    }
}

void JPO(State8080 *st, uint16_t addr)
{
    if (st->cc.p == 0){
        st->pc = addr;
    }else{
        st->pc += 2;
    }
}

void JP(State8080 *st, uint16_t addr)
{
    if (st->cc.s == 0){
        st->pc = addr;
    }else{
        st->pc += 2;
    }
}

void JM(State8080 *st, uint16_t addr)
{
    if (st->cc.s){
        st->pc = addr;
    }else{
        st->pc += 2;
    }
}
void JC(State8080 *st, uint16_t addr)
{
    if (st->cc.cy){
        st->pc = addr;
    }else{
        st->pc += 2;
    }
}

void XTHL(State8080 *st)
{
    uint8_t *mem = &st->memory[st->sp];
    uint8_t tmp_h = st->h;
    uint8_t tmp_l = st->l;
    st->h = mem[1];
    st->l = mem[0];
    mem[1] = tmp_h;
    mem[0] = tmp_l;
}
//////////////////////////////////////////////////////

void Emulate8080Op(State8080 *state)
{
    Disassemble8080Op(state->memory, state->pc);
    unsigned char *opcode = &state->memory[state->pc];
    state->pc += 1; 
    switch(*opcode)
    {
        case 0x00: break;
        case 0x01:
            LXI(&state->b, &state->c, opcode[2], opcode[1], &state->pc);
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
            DCR(&state->b, &state->cc);
            break;
        }
        case 0x06:
        {
            MVI(&state->b,opcode[1], &state->pc);
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
            DCR(&state->c, &state->cc);
            break;
        }
        case 0x0e:
        {
            MVI(&state->c,opcode[1], &state->pc);
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
            LXI(&state->d, &state->e, opcode[2], opcode[1], &state->pc);
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
        case 0x15:
        {
            DCR(&state->d, &state->cc);
            break;
        }
        case 0x16:
        {
            MVI(&state->d,opcode[1], &state->pc);
            break;
        }
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
        case 0x1d:
        {
            DCR(&state->e, &state->cc);
            break;
        }
        case 0x1e:
        {
            MVI(&state->e,opcode[1], &state->pc);
            break;
        }
        case 0x1f: UnimplementedInstruction(state); break;
        case 0x20: UnimplementedInstruction(state); break;
        case 0x21: 
        {
            LXI(&state->h, &state->l, opcode[2], opcode[1], &state->pc);
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
        case 0x25:
        {
            DCR(&state->h, &state->cc);
            break;
        }
        case 0x26: 
        {
            MVI(&state->h,opcode[1], &state->pc);
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
        case 0x2d:
        {
            DCR(&state->l, &state->cc);
            break;
        }
        case 0x2e:
        {
            MVI(&state->l,opcode[1], &state->pc);
            break;
        }
        case 0x2f: UnimplementedInstruction(state); break;
        case 0x30: UnimplementedInstruction(state); break;
        case 0x31: 
        {
            LXI_SP(&state->sp, opcode[2], opcode[1], &state->pc);
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
        case 0x35:
        {
            DCR(&state->memory[state->h<<8 | state->l], &state->cc);
            break;
        }
        case 0x36:
        {
            uint16_t offset = (state->h<<8) | state->l;
            MVI(&state->memory[offset],opcode[1], &state->pc);
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
            MVI(&state->a,opcode[1], &state->pc);
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
        case 0x90:
        {
            SUB(state, state->b);
            break;
        }
        case 0x91:
        {
            SUB(state, state->c);
            break;
        }
        case 0x92:
        {
            SUB(state, state->d);
            break;
        }
        case 0x93:
        {
            SUB(state, state->e);
            break;
        }
        case 0x94:
        {
            SUB(state, state->h);
            break;
        }
        case 0x95:
        {
            SUB(state, state->l);
            break;
        }
        case 0x96:
        {
            uint16_t offset = (state->h << 8) | state->l;
            SUB(state, state->memory[offset]);
            break;
        }
        case 0x97:
        {
            SUB(state, state->a);
            break;
        }
        case 0x98:
        {
            SBB(state, state->b);
            break;
        }
        case 0x99:
        {
            SBB(state, state->c);
            break;
        }
        case 0x9a:
        {
            SBB(state, state->d);
            break;
        }
        case 0x9b:
        {
            SBB(state, state->e);
            break;
        }
        case 0x9c:
        {
            SBB(state, state->h);
            break;
        }
        case 0x9d:
        {
            SBB(state, state->l);
            break;
        }
        case 0x9e:
        {
            uint16_t offset = (state->h << 8) | state->l;
            SBB(state, state->memory[offset]);
            break;
        }
        case 0x9f:
        {
            SBB(state, state->a);
            break;
        }
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
            uint16_t addr = (opcode[2] << 8)| opcode[1];
            JMP(state, addr);
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
        case 0xca:
        {
            uint16_t addr = (opcode[2] << 8)| opcode[1];
            JZ(state, addr);
            break;
        }
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
        case 0xd2:
        {
            uint16_t addr = (opcode[2] << 8)| opcode[1];
            JNC(state, addr);
            break;
        }
        case 0xd3: 
        {
            machine_out(state, opcode[1]);
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
        case 0xda:
        {
            uint16_t addr = (opcode[2] << 8)| opcode[1];
            JC(state, addr);
            break;
        }
        case 0xdb: //IN b (machine specific)
        {
            machine_in(state, opcode[1]);
            break;
        }
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
        case 0xe2:
        {
            uint16_t addr = (opcode[2] << 8)| opcode[1];
            JPO(state, addr);
            break;
        }
        case 0xe3:
        {
            XTHL(state);
            break;
        }
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
        case 0xea:
        {
            uint16_t addr = (opcode[2] << 8)| opcode[1];
            JPE(state, addr);
            break;
        }
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
        case 0xf2:
        {
            uint16_t addr = (opcode[2] << 8)| opcode[1];
            JP(state, addr);
            break;
        }
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
        case 0xfa:
        {
            uint16_t addr = (opcode[2] << 8)| opcode[1];
            JM(state, addr);
            break;
        }
        case 0xfb: 
        {
            state->int_enable = 1;
            break;
        }
        case 0xfc: UnimplementedInstruction(state); break;
        case 0xfd: UnimplementedInstruction(state); break;
        case 0xfe:
        {
            CPI(state, opcode[1]);
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
    printf ("%04X: ", pc);    
    switch (*code)    
    {    
        case 0x00: printf("nop"); break;    
        case 0x01: printf("lxi  b,$%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x02: printf("stax b"); break;    
        case 0x03: printf("inx  b"); break;    
        case 0x04: printf("inr  b"); break;    
        case 0x05: printf("dcr  b"); break;    
        case 0x06: printf("mvi  b,$%02x", code[1]); opbytes=2; break;    
        case 0x07: printf("rlc"); break;    
        case 0x08: printf("nop"); break;    
        case 0x09: printf("dad  b"); break;    
        case 0x0a: printf("ldax b"); break;    
        case 0x0b: printf("dcx  b"); break;    
        case 0x0c: printf("inr  c"); break;    
        case 0x0d: printf("dcr  c"); break;    
        case 0x0e: printf("mvi  c,$%02x", code[1]); opbytes=2; break;    
        case 0x0f: printf("rrc"); break;    
        case 0x10: printf("nop"); break;    
        case 0x11: printf("lxi  d,$%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x12: printf("stax d"); break;    
        case 0x13: printf("inx  d"); break;    
        case 0x14: printf("inr  d"); break;    
        case 0x15: printf("dcr  d"); break;    
        case 0x16: printf("mvi  d,$%02x", code[1]); opbytes=2; break;    
        case 0x17: printf("ral"); break;    
        case 0x18: printf("nop"); break;    
        case 0x19: printf("dad  d"); break;    
        case 0x1a: printf("ldax d"); break;    
        case 0x1b: printf("dcx  d"); break;    
        case 0x1c: printf("inr  e"); break;    
        case 0x1d: printf("dcr  e"); break;    
        case 0x1e: printf("mvi  e,$%02x", code[1]); opbytes=2; break;    
        case 0x1f: printf("rar"); break;    
        case 0x20: printf("rim"); break;    
        case 0x21: printf("lxi  h,$%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x22: printf("shld $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x23: printf("inx  h"); break;    
        case 0x24: printf("inr  h"); break;    
        case 0x25: printf("dcr  h"); break;    
        case 0x26: printf("mvi  h,$%02x", code[1]); opbytes=2; break;    
        case 0x27: printf("daa"); break;    
        case 0x28: printf("nop"); break;    
        case 0x29: printf("dad  h"); break;    
        case 0x2a: printf("lhld $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x2b: printf("dcx  h"); break;    
        case 0x2c: printf("inr  l"); break;    
        case 0x2d: printf("dcr  l"); break;    
        case 0x2e: printf("mvi  l,$%02x", code[1]); opbytes=2; break;    
        case 0x2f: printf("cma"); break;    
        case 0x30: printf("sim"); break;    
        case 0x31: printf("lxi  sp,$%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x32: printf("sta  $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x33: printf("inx  sp"); break;    
        case 0x34: printf("inr  m"); break;    
        case 0x35: printf("dcr  m"); break;    
        case 0x36: printf("mvi  m,$%02x", code[1]); opbytes=2; break;    
        case 0x37: printf("stc"); break;    
        case 0x38: printf("nop"); break;    
        case 0x39: printf("dad  sp"); break;    
        case 0x3a: printf("lda  $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0x3b: printf("dcx  sp"); break;    
        case 0x3c: printf("inr  a"); break;    
        case 0x3d: printf("dcr  a"); break;    
        case 0x3e: printf("mvi  a,$%02x", code[1]); opbytes=2; break;    
        case 0x3f: printf("cmc"); break;    
        case 0x40: printf("mov  b,b"); break;    
        case 0x41: printf("mov  b,c"); break;    
        case 0x42: printf("mov  b,d"); break;    
        case 0x43: printf("mov  b,e"); break;    
        case 0x44: printf("mov  b,h"); break;    
        case 0x45: printf("mov  b,l"); break;    
        case 0x46: printf("mov  b,m"); break;    
        case 0x47: printf("mov  b,a"); break;    
        case 0x48: printf("mov  c,b"); break;    
        case 0x49: printf("mov  c,c"); break;    
        case 0x4a: printf("mov  c,d"); break;    
        case 0x4b: printf("mov  c,e"); break;    
        case 0x4c: printf("mov  c,h"); break;    
        case 0x4d: printf("mov  c,l"); break;    
        case 0x4e: printf("mov  c,m"); break;    
        case 0x4f: printf("mov  c,a"); break;    
        case 0x50: printf("mov  d,b"); break;    
        case 0x51: printf("mov  d,c"); break;    
        case 0x52: printf("mov  d,d"); break;    
        case 0x53: printf("mov  d,e"); break;    
        case 0x54: printf("mov  d,h"); break;    
        case 0x55: printf("mov  d,l"); break;    
        case 0x56: printf("mov  d,m"); break;    
        case 0x57: printf("mov  d,a"); break;    
        case 0x58: printf("mov  e,b"); break;    
        case 0x59: printf("mov  e,c"); break;    
        case 0x5a: printf("mov  e,d"); break;    
        case 0x5b: printf("mov  e,e"); break;    
        case 0x5c: printf("mov  e,h"); break;    
        case 0x5d: printf("mov  e,l"); break;    
        case 0x5e: printf("mov  e,m"); break;    
        case 0x5f: printf("mov  e,a"); break;    
        case 0x60: printf("mov  h,b"); break;    
        case 0x61: printf("mov  h,c"); break;    
        case 0x62: printf("mov  h,d"); break;    
        case 0x63: printf("mov  h,e"); break;    
        case 0x64: printf("mov  h,h"); break;    
        case 0x65: printf("mov  h,l"); break;    
        case 0x66: printf("mov  h,m"); break;    
        case 0x67: printf("mov  h,a"); break;    
        case 0x68: printf("mov  l,b"); break;    
        case 0x69: printf("mov  l,c"); break;    
        case 0x6a: printf("mov  l,d"); break;    
        case 0x6b: printf("mov  l,e"); break;    
        case 0x6c: printf("mov  l,h"); break;    
        case 0x6d: printf("mov  l,l"); break;    
        case 0x6e: printf("mov  l,m"); break;    
        case 0x6f: printf("mov  l,a"); break;    
        case 0x70: printf("mov  m,b"); break;    
        case 0x71: printf("mov  m,c"); break;    
        case 0x72: printf("mov  m,d"); break;    
        case 0x73: printf("mov  m,e"); break;    
        case 0x74: printf("mov  m,h"); break;    
        case 0x75: printf("mov  m,l"); break;    
        case 0x76: printf("hlt"); break;    
        case 0x77: printf("mov  m,a"); break;    
        case 0x78: printf("mov  a,b"); break;    
        case 0x79: printf("mov  a,c"); break;    
        case 0x7a: printf("mov  a,d"); break;    
        case 0x7b: printf("mov  a,e"); break;    
        case 0x7c: printf("mov  a,h"); break;    
        case 0x7d: printf("mov  a,l"); break;    
        case 0x7e: printf("mov  a,m"); break;    
        case 0x7f: printf("mov  a,a"); break;    
        case 0x80: printf("add  b"); break;    
        case 0x81: printf("add  c"); break;    
        case 0x82: printf("add  d"); break;    
        case 0x83: printf("add  e"); break;    
        case 0x84: printf("add  h"); break;    
        case 0x85: printf("add  l"); break;    
        case 0x86: printf("add  m"); break;    
        case 0x87: printf("add  a"); break;    
        case 0x88: printf("adc  b"); break;    
        case 0x89: printf("adc  c"); break;    
        case 0x8a: printf("adc  d"); break;    
        case 0x8b: printf("adc  e"); break;    
        case 0x8c: printf("adc  h"); break;    
        case 0x8d: printf("adc  l"); break;    
        case 0x8e: printf("adc  m"); break;    
        case 0x8f: printf("adc  a"); break;    
        case 0x90: printf("sub  b"); break;    
        case 0x91: printf("sub  c"); break;    
        case 0x92: printf("sub  d"); break;    
        case 0x93: printf("sub  e"); break;    
        case 0x94: printf("sub  h"); break;    
        case 0x95: printf("sub  l"); break;    
        case 0x96: printf("sub  m"); break;    
        case 0x97: printf("sub  a"); break;    
        case 0x98: printf("sbb  b"); break;    
        case 0x99: printf("sbb  c"); break;    
        case 0x9a: printf("sbb  d"); break;    
        case 0x9b: printf("sbb  e"); break;    
        case 0x9c: printf("sbb  h"); break;    
        case 0x9d: printf("sbb  l"); break;    
        case 0x9e: printf("sbb  m"); break;    
        case 0x9f: printf("sbb  a"); break;    
        case 0xa0: printf("ana  b"); break;    
        case 0xa1: printf("ana  c"); break;    
        case 0xa2: printf("ana  d"); break;    
        case 0xa3: printf("ana  e"); break;    
        case 0xa4: printf("ana  h"); break;    
        case 0xa5: printf("ana  l"); break;    
        case 0xa6: printf("ana  m"); break;    
        case 0xa7: printf("ana  a"); break;    
        case 0xa8: printf("xra  b"); break;    
        case 0xa9: printf("xra  c"); break;    
        case 0xaa: printf("xra  d"); break;    
        case 0xab: printf("xra  e"); break;    
        case 0xac: printf("xra  h"); break;    
        case 0xad: printf("xra  l"); break;    
        case 0xae: printf("xra  m"); break;    
        case 0xaf: printf("xra  a"); break;    
        case 0xb0: printf("ora  b"); break;    
        case 0xb1: printf("ora  c"); break;    
        case 0xb2: printf("ora  d"); break;    
        case 0xb3: printf("ora  e"); break;    
        case 0xb4: printf("ora  h"); break;    
        case 0xb5: printf("ora  l"); break;    
        case 0xb6: printf("ora  m"); break;    
        case 0xb7: printf("ora  a"); break;    
        case 0xb8: printf("cmp  b"); break;    
        case 0xb9: printf("cmp  c"); break;    
        case 0xba: printf("cmp  d"); break;    
        case 0xbb: printf("cmp  e"); break;    
        case 0xbc: printf("cmp  h"); break;    
        case 0xbd: printf("cmp  l"); break;    
        case 0xbe: printf("cmp  m"); break;    
        case 0xbf: printf("cmp  a"); break;    
        case 0xc0: printf("rnz"); break;    
        case 0xc1: printf("pop  b"); break;    
        case 0xc2: printf("jnz  $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xc3: printf("jmp  $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xc4: printf("cnz  $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xc5: printf("push b"); break;    
        case 0xc6: printf("adi  $%02x", code[1]); opbytes=2; break;    
        case 0xc7: printf("rst  0"); break;    
        case 0xc8: printf("rz"); break;    
        case 0xc9: printf("ret"); break;    
        case 0xca: printf("jz   $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xcb: printf("nop"); break;    
        case 0xcc: printf("cz   $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xcd: printf("call $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xce: printf("aci  $%02x", code[1]); opbytes=2; break;    
        case 0xcf: printf("rst  1"); break;    
        case 0xd0: printf("rnc"); break;    
        case 0xd1: printf("pop  d"); break;    
        case 0xd2: printf("jnc  $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xd3: printf("out  $%02x", code[1]); opbytes=2; break;    
        case 0xd4: printf("cnc  $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xd5: printf("push d"); break;    
        case 0xd6: printf("sui  $%02x", code[1]); opbytes=2; break;    
        case 0xd7: printf("rst  2"); break;    
        case 0xd8: printf("rc"); break;    
        case 0xd9: printf("nop"); break;    
        case 0xda: printf("jc   $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xdb: printf("in   $%02x", code[1]); opbytes=2; break;    
        case 0xdc: printf("cc   $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xdd: printf("nop"); break;    
        case 0xde: printf("sbi  $%02x", code[1]); opbytes=2; break;    
        case 0xdf: printf("rst  3"); break;    
        case 0xe0: printf("rpo"); break;    
        case 0xe1: printf("pop  h"); break;    
        case 0xe2: printf("jpo  $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xe3: printf("xthl"); break;    
        case 0xe4: printf("cpo  $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xe5: printf("push h"); break;    
        case 0xe6: printf("ani  $%02x", code[1]); opbytes=2; break;    
        case 0xe7: printf("rst  4"); break;    
        case 0xe8: printf("rpe"); break;    
        case 0xe9: printf("pchl"); break;    
        case 0xea: printf("jpe    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xeb: printf("xchg"); break;    
        case 0xec: printf("cpe    $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xed: printf("nop"); break;    
        case 0xee: printf("xri  $%02x", code[1]); opbytes=2; break;    
        case 0xef: printf("rst  5"); break;    
        case 0xf0: printf("rp"); break;    
        case 0xf1: printf("pop  psw"); break;    
        case 0xf2: printf("jp   $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xf3: printf("di"); break;    
        case 0xf4: printf("cp   $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xf5: printf("push psw"); break;    
        case 0xf6: printf("ori  $%02x", code[1]); opbytes=2; break;    
        case 0xf7: printf("rst  6"); break;    
        case 0xf8: printf("rm"); break;    
        case 0xf9: printf("sphl"); break;    
        case 0xfa: printf("jm   $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xfb: printf("ei"); break;    
        case 0xfc: printf("cm     $%02x%02x", code[2], code[1]); opbytes=3; break;    
        case 0xfd: printf("nop"); break;    
        case 0xfe: printf("cpi  $%02x", code[1]); opbytes=2; break;    
        case 0xff: printf("rst  7"); break;    
   }
        
    printf("\n");    

    return opbytes;    
}   

