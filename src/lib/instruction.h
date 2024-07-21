#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

#include <stdint.h>

typedef enum {

    // MOV
    MOV_REG_TO_REG=         0b10001000, // move to register/memory from register.
    IMDT_REG_TO_MEM=        0b11000110, // immidiate to register/memory
    IMDT_REG =              0b10110000, // immidiate to register
    MEM_TO_ACCUMUL =        0b10100000, //　memory to accumulator
    ACCUMUL_TO_MEM =        0b10100010, // accumulator to memory
    MEM_TO_SEGREG =         0b10001110, // memory/register to segment register
    SEGREG_TO_MEM =         0b10001100, // segment register to memory/register

    // PUSH
    PUSH_REG_MEM =          0b11111111, // Push register/memory
    PUSH_REG =              0b01010000, // Push register
    PUSH_SREG =             0b00000110, // Push segment register

    // POP
    POP_REG_MEM =           0b10001111, // pop register/memory
    POP_REG =               0b01011000, // pop register
    POP_SREG =              0b00000111, // pop segment register

    // XCHG
    XCHG_REGMEM_REG =       0b10000110, // exchange register/memory with register
    XCHG_REG_ACCUMUL =      0b10010000, // Exchange with accumulator

    // ADD
    ADD_REGMEM_TO_REG =     0b00000000, // Add Register/memory with register either
    ADD_IMDT_REGMEM =       0b10000000, // Add immidiate to register/memory
    ADD_IMDT_ACCUMUL =      0b00000100, // Add immidiate to accumulator

    //ADC
    ADC_REGMEM_REG =        0b00010000, // Add register/memory with register either carry
    ADC_IMDT_REGMEM =       0b10000000, // Add immidiate to register/memory carry
    ADC_IMDT_ACCUMUL =      0b00010100, // Add immidiate to accumulator carry

    // INT
    INT_TYPESPEC =          0b11001101, // Type specified

} Instruction;

typedef enum {
    MOV,
    INT,
    PUSH,
    POP,
    XCHG,
    ADD,
    ADC,

    NONE
} Type;

typedef struct {
    Instruction ins;
    uint8_t len;
    Type type;
    uint8_t skip;
    uint8_t config;
} IST;

// config
#define WORD        0x01
#define DISP        0x02
#define CONTROLLER  0x04 // MOD and RM
#define DATA_WORD   0x08
#define DATA_QWORD  0x18
#define PORT        0x20
#define REGISTER    0x40 // REG
#define RESERVED_5  0x80

IST identify(uint8_t ins, IST[], uint32_t table_size);

#endif