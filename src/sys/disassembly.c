#include "../lib/disassembly.h"
#include "../lib/instruction.h"
#include "../lib/memory.h"
#include "../lib/cpu.h"
#include "../lib/interrupt.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

IST table[] = {
    { .ins = MOV_REG_TO_REG, .len = 6, .type = MOV, .skip = 2, .config = DISP | WORD | CONTROLLER | REGISTER}, // -
    { IMDT_REG_TO_MEM, 7, MOV, 3, .config = WORD | CONTROLLER | DATA_WORD}, // -
    { IMDT_REG, 4, MOV, 2, .config = WORD | REGISTER | DATA_WORD }, // -
    { MEM_TO_ACCUMUL, 7, MOV, 3, .config = WORD | DATA_WORD }, // -
    { ACCUMUL_TO_MEM, 7, MOV, 3, .config = WORD | DATA_WORD }, // -
    { MEM_TO_SEGREG, 8, MOV, 2, .config = CONTROLLER }, // -
    { SEGREG_TO_MEM, 8, MOV, 2, .config = CONTROLLER }, // -

    // PUSH
    { PUSH_REG_MEM, 8, PUSH, 2, .config = CONTROLLER | WORD }, // -
    { PUSH_REG, 5, PUSH, 1, .config = REGISTER }, // -
    { PUSH_SREG, 8, PUSH, 1, .config = 0 }, // -

    // POP
    { POP_REG_MEM, 8, POP, 2, .config = CONTROLLER }, // -
    { POP_REG, 5, POP, 1, .config = REGISTER }, // -
    { POP_SREG, 8, POP, 1, .config = 0 }, // -

    //XCHG
    { XCHG_REGMEM_REG, 7, XCHG, 2, .config = CONTROLLER }, // -
    { XCHG_REG_ACCUMUL, 5, XCHG, 1, .config = 0 }, //

    // ADD
    { ADD_REGMEM_TO_REG, 6, ADD, 2, .config = CONTROLLER | WORD | DISP }, // -
    { ADD_IMDT_REGMEM, 6, ADD, 3, .config = CONTROLLER | WORD | DISP | DATA_WORD }, // -
    { ADD_IMDT_ACCUMUL, 7, ADD, 2, .config = DATA_WORD | WORD }, // -

    //ADC
    { ADC_REGMEM_REG, 6, ADC, 2, .config = CONTROLLER | WORD | DISP }, // -
    { ADC_IMDT_REGMEM, 6, ADC, 3, .config = CONTROLLER | WORD | DISP | DATA_WORD }, // -
    { ADC_IMDT_ACCUMUL, 7, ADC, 2, .config = DATA_WORD | WORD }, // -

    //INC
    { INC_REGMEM, 7, INC, 2, .config = WORD | CONTROLLER }, // -
    { INC_REG, 5, INC, 1, .config = REGISTER }, // -

    //SUB
    { SUB_REGMEM_TO_REG, 6, SUB, 2, .config = DISP | WORD | CONTROLLER | REGISTER }, // -
    { SUB_IMDT_REGMEM, 6, SUB, 3, .config = DISP | WORD | CONTROLLER | DATA_WORD }, // -
    { SUB_IMDT_ACCUMUL, 7, SUB, 2, .config = WORD | DATA_WORD }, // -

    // SSB
    { SSB_REGMEM_TO_REG, 6, SSB, 2, .config = CONTROLLER | DISP | WORD }, // -
    { SSB_IMDT_REGMEM, 6, SSB, 3, .config = DISP | WORD | CONTROLLER | DATA_WORD }, // -
    { SSB_IMDT_ACCUMUL, 7, SSB, 2, .config = WORD | DATA_WORD }, // -

    // DEC
    { DEC_REG, 5, DEC, 1, .config = REGISTER }, // -

    //JMP
    { JMP_WITHIN_SGMT, 8, JMP, 3, .config = DATA_WORD }, //

    // INT    
    { INT_TYPESPEC, 8, INT, 2, .config = DATA_WORD }, // -
};

#define TABLE_SIZE sizeof(table) / sizeof(IST)

size_t filesize = 0;

void fetch(FILE* f, uint8_t* memory) {
    size_t size = 0;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    filesize = size;
    fseek(f, 0, SEEK_SET);

    fread(memory, size, 1, f);
}

uint16_t getEffectiveAddress(CPU* cpu, uint8_t mod, uint8_t rm, uint8_t* memory, uint8_t* delta) {

    #define IF_SIGNEXTD (mod==1?(int8_t)disp:disp)

    uint16_t disp = 0;

    switch(mod) {

        case 0:
            disp = 0;
            break;
        
        case 1:
            *delta = *delta + 1;
            disp = (int16_t) memory[cpu->ip+2];
            break;

        case 2:
            *delta = *delta + 2;
            disp = combineBytes(memory[cpu->ip+3], memory[cpu->ip+2]);
            break;

        case 3:
            return rm;

        default:
            break;
    }

    if(mod == 3) {
        return rm;
    }

    uint16_t ea = 0;

    switch(rm) {
        case 0:
            ea = cpu->_16bits[BX] + cpu->_16bits[SI] + IF_SIGNEXTD;
            return ea;
        case 1:
            ea = cpu->_16bits[BX] + cpu->_16bits[DI] + IF_SIGNEXTD;
            return ea;
        case 2:
            ea = cpu->_16bits[BP] + cpu->_16bits[SI] + IF_SIGNEXTD;
            return ea;
        case 3:
            ea = cpu->_16bits[BP] + cpu->_16bits[DI] + IF_SIGNEXTD;
            return ea;
        case 4:
            ea = cpu->_16bits[SI] + IF_SIGNEXTD;
            return ea;
        case 5:
            ea = cpu->_16bits[DI] + IF_SIGNEXTD;
            return ea;
        case 6:
            if(!mod) {
                disp = combineBytes(memory[cpu->ip+3], memory[cpu->ip+2]);
                *delta = *delta + 2;
                return disp;
            }
            ea = cpu->_16bits[BP] + IF_SIGNEXTD;
            return ea;
        case 7:
            ea = cpu->_16bits[BX] + IF_SIGNEXTD;
            return ea;
    }

    return rm;
}

void controllerFetch(uint8_t instruction, uint8_t* mod, uint8_t* reg, uint8_t* rm) {
    *mod = (instruction & 0xC0) >> 6;
    *reg = (instruction & 0x38) >> 0x03;
    *rm = instruction & 0x07;
}

void execute(uint8_t* memory, CPU* cpu) {
    uint8_t delta = 0;
    uint16_t ea = 0;
    uint8_t mod = 0, rm = 0, reg = 0; // controller
    uint8_t suffix = 0; // w, d, sw variables
    uint8_t data[4]; // 0 - low, 1 - high, 2 - low, 3 - high
    IST ist;
    ist.type = NONE;

    while(cpu->ip < filesize) { // end of memory
        mod = 0; reg = 0; rm = 0; suffix = 0;
        ist = identify(memory[cpu->ip], table, TABLE_SIZE);
        
        if(ist.config & WORD) {
            suffix |= memory[cpu->ip] & 0x01;
        }
        if(ist.config & DISP) {
            suffix |= memory[cpu->ip] & 0x02;
        }
        if(ist.config & CONTROLLER) {
            controllerFetch(memory[cpu->ip+1], &mod, &reg, &rm);
            ea = getEffectiveAddress(cpu, mod, rm, memory, &delta);
        }
        if(ist.config & DATA_WORD) {
            data[0] = memory[cpu->ip+1+delta+(!!(ist.config & CONTROLLER))];
            data[1] = memory[cpu->ip+2+delta+(!!(ist.config & CONTROLLER))];
        }
        if(ist.config & DATA_QWORD) {
            data[2] = memory[cpu->ip+3+delta+(!!(ist.config & CONTROLLER))];
            data[3] = memory[cpu->ip+4+delta+(!!(ist.config & CONTROLLER))];
        }
        switch(ist.ins) {
            case MOV_REG_TO_REG:

                // if d = 1, reg is source
                mov(cpu, mod, reg, ea, suffix, memory);

                suffix = 0;
                break;

            case IMDT_REG:

                ea = memory[cpu->ip] & 0x07;
                mod = 3;
                suffix |= (memory[cpu->ip] & 0x08) >> 3;

            case IMDT_REG_TO_MEM:

                mov_immidiate(cpu, mod, ea, suffix, memory, data);

                break;

            case MEM_TO_ACCUMUL:

                suffix |= DISP;

            case ACCUMUL_TO_MEM:

                ea = combineBytes(data[1], data[0]);
                mov(cpu, mod, reg, ea, suffix, memory);

                suffix = 0;
                break;

            case SEGREG_TO_MEM:
            case MEM_TO_SEGREG:
                
                mov_segment(cpu, mod, reg, ea, suffix, memory);

                break;

            case INT_TYPESPEC:

                cpu_irq(data[0]);
                break;

            case PUSH_REG:
                mod = 3;
                ea = memory[cpu->ip] & 0x07;

            case PUSH_REG_MEM:

                push(cpu, memory, mod, ea);

                break;

            case PUSH_SREG:

                reg = (memory[cpu->ip] & 0x18) >> 3;
                memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP] - 1)] = (cpu->segments[reg] & 0xFF00) >> 8;
                memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP] - 2)] = (cpu->segments[reg] & 0x00FF);

                cpu->_16bits[SP] -= 2;

                break;

            case POP_SREG:

                reg = (memory[cpu->ip] & 0x18) >> 3;
                cpu->segments[reg] = combineBytes(memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP] + 1)], 
                    memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP])]);

                cpu->_16bits[SP] += 2;

                break;

            case POP_REG:
                mod = 3;
                ea = memory[cpu->ip] & 0x07;

            case POP_REG_MEM:

                pop(cpu, memory, mod, ea);

                break;

            case XCHG_REG_ACCUMUL:

                mod = 3;
                reg = memory[cpu->ip] & 0x07;
                suffix |= WORD;
                ea = 0;
                /*
                    Since if mod == 3 then ea treated as register ea=0 represents ax (Accumulator) register
                */

            case XCHG_REGMEM_REG:

                xchg(cpu, memory, reg, mod, ea, suffix);

                break;

            case ADC_REGMEM_REG:
            case ADD_REGMEM_TO_REG:

                add(cpu, memory, reg, mod, ea, suffix, ist.ins == ADC_REGMEM_REG);
                suffix = 0;

                break;

            case ADC_IMDT_ACCUMUL:
                reg = 2;
            case ADD_IMDT_ACCUMUL:
                mod = 3;
                ea = 0;

            case ADD_IMDT_REGMEM:

                if(reg == 5) {
                    sub_immidiate(cpu, memory, mod, ea, suffix, data, false);
                    break;
                }

                add_immidiate(cpu, memory, mod, ea, suffix, data, reg == 2);

                break;

            case INC_REG:
                mod = 3;
                suffix |= WORD;
                ea = memory[cpu->ip] & 0x07;
                inc(cpu, memory, mod, ea, suffix, 1);
                break;

            case INC_REGMEM:

                if(reg == 1) {
                    inc(cpu, memory, mod, ea, suffix, -1);
                } else
                    inc(cpu, memory, mod, ea, suffix, 1); // can be used as decrement with passing direction -1
                suffix = 0;

                break;

            case SUB_REGMEM_TO_REG:

                sub(cpu, memory, reg, mod, ea, suffix, false);
                suffix = 0;

                break;

            case SUB_IMDT_ACCUMUL:

                sub_immidiate(cpu, memory, 3, 0, suffix, data, false);

                break;

            case SSB_REGMEM_TO_REG:

                sub(cpu, memory, reg, mod, ea, suffix, true);
                suffix = 0;

                break;

            case SSB_IMDT_ACCUMUL:

                sub_immidiate(cpu, memory, mod, ea, suffix, data, true);

                break;

            case DEC_REG:

                reg = memory[cpu->ip] & 0x07;
                inc(cpu, memory, 3, reg, WORD, -1);

                break;

            case JMP_WITHIN_SGMT:

                

                break;

            default:
                delta = 1;
                break;
        }

        if(suffix & WORD) {
            delta += 1;
        }

        cpu->ip += delta + ist.skip;
        delta = 0;
        ea = 0;
        suffix = 0;

        printf("IP: %llX, AX: %X, BX: %X, CX: %X, DX: %X, DI: %X, SI: %X, BP: %X, SP: %X\n", cpu->ip, cpu->_16bits[AX], cpu->_16bits[BX],
             cpu->_16bits[CX],
             cpu->_16bits[DX], cpu->_16bits[DI], cpu->_16bits[SI], cpu->_16bits[BP], cpu->_16bits[SP]);
    }

    for(int i = 0; i < 3; i++)
            printf("[%X]=%X, ", cpu->_16bits[SP] + i, memory[physicalToLogical(cpu->segments[SS], cpu->_16bits[SP] + i)]);
        printf("\n");

}