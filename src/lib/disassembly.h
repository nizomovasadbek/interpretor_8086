#ifndef _DISASSEMBLY_H
#define _DISASSEMBLY_H

#include "cpu.h"
#include <stdio.h>

typedef struct {
    uint8_t w;
    uint8_t d;
    uint8_t displow;
    uint8_t disphigh;
    uint8_t addrlow;
    uint8_t addrhigh;
    uint8_t reg;
    uint8_t mod;
    uint8_t rm;
} Architecture;

void fetch(FILE* f, uint8_t* memory);
void execute(uint8_t* memory, CPU* cpu);
void controllerFetch(uint8_t instruction, uint8_t* mod, uint8_t* reg, uint8_t* rm);

uint16_t getEffectiveAddress(CPU* cpu, uint8_t mod, uint8_t rm, uint8_t* memory, uint8_t* delta);

#endif