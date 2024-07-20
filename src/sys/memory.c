#include "../lib/memory.h"

uint32_t physicalToLogical(uint16_t segment, uint16_t offset) {
    uint32_t physicalAddr = (segment << 4) + offset;
    return physicalAddr;
}

uint16_t combineBytes(uint8_t high, uint8_t low) {
    uint16_t result = high;
    result <<= 8;
    result |= low;

    return result;
}

void splitWord(uint16_t word, uint8_t* high, uint8_t* low) {
    *low = *(uint8_t*)(&word);
    *high = *((uint8_t*)(&word) + 1);
}