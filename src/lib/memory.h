#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 1024*1024

uint32_t physicalToLogical(uint16_t segment, uint16_t offset);
uint16_t combineBytes(uint8_t high, uint8_t low);
void splitWord(uint16_t word, uint8_t* high, uint8_t* low);

#endif