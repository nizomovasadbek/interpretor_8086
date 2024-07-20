#include "../lib/cpu.h"
#include "../lib/memory.h"
#include "../lib/instruction.h"

#include <stdlib.h>
#include <string.h>

void mov(CPU* cpu, uint8_t mod, uint8_t reg, uint16_t ea, uint8_t suffix, uint8_t* memory) {
    uint8_t high, low;
    if(mod == 3) { // ea treated as register
        if(suffix & DISP) {
            setValue(cpu, reg, ea, suffix & WORD);
        } else {
            setValue(cpu, ea, (suffix & WORD)?cpu->_16bits[reg]:cpu->_8bits[reg], suffix & WORD);
        }
    } else {
        if(suffix & DISP) {
            setValue(cpu, reg, combineBytes(memory[physicalToLogical(cpu->segments[DS], ea+1)], memory[physicalToLogical(cpu->segments[DS], ea)]), suffix & WORD);
        } else {
            if(suffix & WORD) {
                splitWord(cpu->_16bits[reg], &high, &low);
                memory[physicalToLogical(cpu->segments[DS], ea)] = low;
                memory[physicalToLogical(cpu->segments[DS], ea + 1)] = high;
            } else {
                memory[physicalToLogical(cpu->segments[DS], ea)] = cpu->_8bits[reg];
            }
        }
    }
}

void mov_immidiate(CPU* cpu, uint8_t mod, uint16_t ea, uint8_t suffix, uint8_t* memory, uint8_t* data_word) {
    if(mod == 3) {
        setValue(cpu, ea, combineBytes(data_word[1], data_word[0]), suffix & WORD);
    } else {
        memory[physicalToLogical(cpu->segments[DS], ea)] = combineBytes(data_word[1], data_word[0]);
    }
}

void mov_segment(CPU* cpu, uint8_t mod, uint8_t sreg, uint16_t ea, uint8_t suffix, uint8_t* memory) {
    if(mod == 3) {
        if(!(suffix & DISP)) {
            cpu->segments[sreg] = cpu->_16bits[ea];
        } else {
            cpu->_16bits[ea] = cpu->segments[sreg];
        }
    } else {
        if(!(suffix & DISP)) {
        cpu->segments[sreg] = memory[physicalToLogical(cpu->segments[DS], ea)];
        } else {
            memory[physicalToLogical(cpu->segments[DS], ea)] = cpu->segments[sreg];
        }
    }
}

CPU cpu_init(void) {

    CPU cpu;

    memset(&cpu, 0, sizeof(CPU));
    cpu._16bits[SP] = 0xFFFF;

    return cpu;
}

void push(CPU* cpu, uint8_t* memory, uint8_t mod, uint16_t ea) {
    uint8_t high, low;
    if(mod == 3) {
        splitWord(cpu->_16bits[ea], &high, &low);
        memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP] - 1)] = high;
        memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP] - 2)] = low;
    } else {
        memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP] - 2)] = memory[physicalToLogical(cpu->segments[DS], ea)];
        memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP] - 1)] = memory[physicalToLogical(cpu->segments[DS], ea + 1)];
    }
    cpu->_16bits[SP] -= 2;
}

void setValue(CPU* cpu, uint8_t reg, uint16_t value, bool w) {
    if(w) {
        cpu->_16bits[reg] = value;
        if(reg <= BX) {
            cpu->_8bits[reg] = (uint8_t) (value & 0x0F);
            cpu->_8bits[reg + 4] = (uint8_t) ((value & 0xF0) >> 8);
        }
    } else {
        uint16_t total = 0;
        cpu->_8bits[reg] = (uint8_t) value;
        if(reg > BL) {
            total = value;
            total <<= 8;
            total |= (cpu->_16bits[reg-4] & 0x00FF);
        } else {
            total = (cpu->_16bits[reg-4] & 0xFF00);
            total |= (uint8_t) value;
        }

        cpu->_16bits[reg-4] = total;
    }
}