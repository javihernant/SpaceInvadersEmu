#ifndef EMU_H
#define EMU_H

#include "regs.h"

void LogicFlagsA(State8080 *state);
int parity(int n, int size);
ConditionCodes InitCC();
State8080 StateCreat(uint8_t *buff);
void printState(State8080 *st);
void UnimplementedInstruction(State8080 *state);
int Emulate8080Op(State8080 *state);
int Disassemble8080Op(unsigned char *codebuffer, int pc);

void PUSH(uint8_t *mem, uint8_t h, uint8_t l, uint16_t *sp);
#endif
