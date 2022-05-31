#ifndef EMU_H
#define EMU_H

#include "regs.h"

void LogicFlagsA(State8080 *state);
int parity(int n, int size);
ConditionCodes InitCC();
State8080 StateCreat(uint8_t *buff);
void printState(State8080 *st);
void UnimplementedInstruction(State8080 *state);
void Emulate8080Op(State8080 *state);
int Disassemble8080Op(unsigned char *codebuffer, int pc);

void CALL(State8080 *st, uint8_t h, uint8_t l);
#endif
