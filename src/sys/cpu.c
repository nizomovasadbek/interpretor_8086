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
        memory[physicalToLogical(cpu->segments[DS], ea)] = data_word[0];
        if(suffix & WORD)
            memory[physicalToLogical(cpu->segments[DS], ea + 1)] = data_word[1];
    }
}

void mov_segment(CPU* cpu, uint8_t mod, uint8_t sreg, uint16_t ea, uint8_t suffix, uint8_t* memory) {
    if(mod == 3) {
        if(!(suffix & DISP)) {
            cpu->segments[sreg] = cpu->_16bits[ea];
        } else {
            setValue(cpu, ea, cpu->segments[sreg], true);
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

void pop(CPU* cpu, uint8_t* memory, uint8_t mod, uint16_t ea) {
    if(mod == 3) {
        setValue(cpu, ea, combineBytes(memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP] + 1)], 
            memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP])]), true);
    } else {
        memory[physicalToLogical(cpu->segments[DS], ea)] = memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP])];
        memory[physicalToLogical(cpu->segments[DS], ea + 1)] = memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP] + 1)];
    }

    cpu->_16bits[SP] += 2;
}

void xchg(CPU* cpu, uint8_t* memory, uint8_t reg, uint8_t mod, uint16_t ea, uint8_t suffix) {
    uint16_t temp;
    if(mod == 3) {
        if(suffix & WORD) {
            temp = cpu->_16bits[reg];
            setValue(cpu, reg, cpu->_16bits[ea], true);
            setValue(cpu, ea, temp, true);
        } else {
            temp = cpu->_8bits[reg];
            setValue(cpu, reg, cpu->_8bits[ea], false);
            setValue(cpu, ea, temp, false);
        }
    } else {
        if(suffix & WORD) {
            temp = cpu->_16bits[reg];
            setValue(cpu, reg, combineBytes(memory[physicalToLogical(cpu->segments[DS], ea+1)], 
                memory[physicalToLogical(cpu->segments[DS], ea)]), true);
            memory[physicalToLogical(cpu->segments[DS], ea)] = temp & 0x00FF;
            memory[physicalToLogical(cpu->segments[DS], ea + 1)] = (temp & 0xFF00) >> 8;
        } else {
            temp = cpu->_8bits[reg];
            setValue(cpu, reg, memory[physicalToLogical(cpu->segments[DS], ea)], false);
            memory[physicalToLogical(cpu->segments[DS], ea)] = temp;
        }
    }
}

void add(CPU* cpu, uint8_t* memory, uint8_t reg, uint8_t mod, uint16_t ea, uint8_t suffix, uint8_t carry) {
    uint8_t c = carry?!!(cpu->flags & (1 << CF)):0;
    uint32_t sum = c + (suffix&WORD ? cpu->_16bits[reg] : cpu->_8bits[reg]) + memory[physicalToLogical(cpu->segments[DS], ea + (suffix & WORD))];
    if(sum > 0xFFFF) {
        setFlag(cpu, CF);
    }
    if(!sum) {
        setFlag(cpu, ZF);
    }
    if(mod == 3) { // ea as register
        if(suffix & DISP) { // "to" reg
            setValue(cpu, reg, c + ((suffix & WORD)?(cpu->_16bits[reg] + cpu->_16bits[ea]):(cpu->_8bits[reg]+cpu->_8bits[ea])), suffix & WORD);
        } else {
            setValue(cpu, ea, c + ((suffix & WORD)?(cpu->_16bits[reg] + cpu->_16bits[ea]):(cpu->_8bits[reg] + cpu->_8bits[ea])), suffix & WORD);
        }
    } else {
        if(suffix & DISP) {
            setValue(cpu, reg, c + (suffix&WORD?cpu->_16bits[reg]:cpu->_8bits[reg]) + combineBytes(memory[physicalToLogical(cpu->segments[DS], ea+1)], 
                memory[physicalToLogical(cpu->segments[DS], ea)]), suffix & WORD);
        } else {
            memory[physicalToLogical(cpu->segments[DS], ea)] += c + (suffix&WORD?cpu->_16bits[reg]:cpu->_8bits[reg]) & 0x00FF;
            if(suffix & WORD)
                memory[physicalToLogical(cpu->segments[DS], ea + 1)] += (cpu->_16bits[reg] & 0xFF00) >> 8;
        }
    }
}

void sub(CPU* cpu, uint8_t* memory, uint8_t reg, uint8_t mod, uint16_t ea, uint8_t suffix, uint8_t carry) {
    uint8_t c = carry?!!(cpu->flags & (1 << CF)):0;
    if(mod == 3) { // ea as register
        if(suffix & DISP) { // "to" reg
            setValue(cpu, reg, ((suffix & WORD)?(cpu->_16bits[reg] - cpu->_16bits[ea]):(cpu->_8bits[reg]+cpu->_8bits[ea])) - c, suffix & WORD);
        } else {
            setValue(cpu, ea, ((suffix & WORD)?(cpu->_16bits[reg] - cpu->_16bits[ea]):(cpu->_8bits[reg] + cpu->_8bits[ea])) - c, suffix & WORD);
        }
    } else {
        if(suffix & DISP) {
            setValue(cpu, reg, (suffix&WORD?cpu->_16bits[reg]:cpu->_8bits[reg]) - combineBytes(memory[physicalToLogical(cpu->segments[DS], ea+1)] - c, 
                memory[physicalToLogical(cpu->segments[DS], ea)]), suffix & WORD);
        } else {
            memory[physicalToLogical(cpu->segments[DS], ea)] -= (suffix&WORD?cpu->_16bits[reg]:cpu->_8bits[reg]) & 0x00FF - c;
            if(suffix & WORD)
                memory[physicalToLogical(cpu->segments[DS], ea + 1)] -= (cpu->_16bits[reg] & 0xFF00) >> 8;
        }
    }
}

void add_immidiate(CPU* cpu, uint8_t* memory, uint8_t mod, uint16_t ea, uint8_t suffix, uint8_t* data, uint8_t carry) {
    uint8_t c = carry?!!(cpu->flags & (1 << CF)):0;
    uint16_t r = combineBytes(data[1], data[0]);
    if(mod == 3) {
        if(suffix & WORD) {
            setValue(cpu, ea, c + cpu->_16bits[ea] + (suffix&DISP?(int16_t)r:r), true);
        } else {
            setValue(cpu, ea, c + cpu->_8bits[ea] + (suffix&DISP?(int8_t)r:r), false);
        }
    } else {
        memory[physicalToLogical(cpu->segments[DS], ea)] += c + (suffix&DISP?(int8_t)r:r);
        if(c + (suffix&DISP?(int8_t)r:r) > 0xFF)
            r += 0x0100;
        if(suffix & WORD) {
            memory[physicalToLogical(cpu->segments[DS], ea + 1)] += (suffix&DISP?(int8_t)(r & 0xFF00) >> 8:(r & 0xFF00) >> 8);
        }
    }
}

void sub_immidiate(CPU* cpu, uint8_t* memory, uint8_t mod, uint16_t ea, uint8_t suffix, uint8_t* data, uint8_t carry) {
    uint8_t c = carry?!!(cpu->flags & (1 << CF)):0;
    uint16_t r = combineBytes(data[1], data[0]);
    if(mod == 3) {
        if(suffix & WORD) {
            setValue(cpu, ea, cpu->_16bits[ea] - (suffix & DISP?(int16_t)r:r) - c, true);
            if(cpu->_16bits[ea] == 0) setFlag(cpu, ZF);
        } else {
            setValue(cpu, ea, cpu->_8bits[ea] - (suffix & DISP?(int8_t)r:r) - c, false);
            if(cpu->_8bits[ea] == 0) setFlag(cpu, ZF);
        }
    } else {
        if(suffix & WORD) {
            memory[physicalToLogical(cpu->segments[DS], ea)] -= (suffix&DISP?(int8_t)r:r) - c;
            memory[physicalToLogical(cpu->segments[DS], ea + 1)] -= (suffix&DISP?(int8_t)(r & 0xFF00) >> 8:(r & 0xFF00) >> 8)-c;
            if(combineBytes(memory[physicalToLogical(cpu->segments[DS], ea + 1)], memory[physicalToLogical(cpu->segments[DS], ea)] == 0)) {
                setFlag(cpu, ZF);
            }
        } else {
            memory[physicalToLogical(cpu->segments[DS], ea)] -= (suffix&DISP?(int8_t)r:r) - c;
        }
    }
}

void inc(CPU* cpu, uint8_t* memory, uint8_t mod, uint16_t ea, uint8_t suffix, int8_t direction) {
    if(mod == 3) {
        if(suffix & WORD) {
            if(cpu->_16bits[ea] == 0xFFFF) cpu->flags |= 1 << CF;
            setValue(cpu, ea, cpu->_16bits[ea] + direction, true);
        } else {
            if(cpu->_8bits[ea] == 0xFF) cpu->flags |= 1 << CF;
            setValue(cpu, ea, cpu->_8bits[ea] + direction, false);
        }
    } else {
        if(memory[physicalToLogical(cpu->segments[DS], ea)] == 0xFF && direction == 1) {
            memory[physicalToLogical(cpu->segments[DS], ea)] = 0x00;
            if(!(suffix & WORD)) {
                setFlag(cpu, ZF);
                setFlag(cpu, CF);
            }
            if(suffix & WORD && memory[physicalToLogical(cpu->segments[DS], ea + 1)] == 0xFF) {
                memory[physicalToLogical(cpu->segments[DS], ea + 1)] = 0;
                setFlag(cpu, CF);
                setFlag(cpu, ZF);
            } else 
                memory[physicalToLogical(cpu->segments[DS], ea + 1)] += 1;
        }
        memory[physicalToLogical(cpu->segments[DS], ea)] += direction;
    }
}

void setValue(CPU* cpu, uint8_t reg, uint16_t value, bool w) {
    if(w) {
        cpu->_16bits[reg] = value;
        if(reg <= BX) {
            cpu->_8bits[reg] = (uint8_t) (value & 0xFF);
            cpu->_8bits[reg + 4] = (uint8_t) ((value & 0xFF00) >> 8);
        }
    } else {
        uint16_t total = 0;
        cpu->_8bits[reg] = (uint8_t) value;
        if(reg > BL) {
            total = value;
            total <<= 8;
            total |= (uint8_t) (cpu->_16bits[reg-4] & 0x00FF);
        } else {
            total = (cpu->_16bits[reg] & 0xFF00);
            total |= (uint8_t) value;
        }
        if(reg > BL) reg -= 4;
        cpu->_16bits[reg] = total;
    }
}

void setFlag(CPU* cpu, int flag) {
    cpu->flags |= 1 << flag;
}

void clrFlag(CPU* cpu, int flag) {
    cpu->flags &= ~(1 << flag);
}

void invFlag(CPU* cpu, int flag) {
    cpu->flags ^= 1 << flag;
}