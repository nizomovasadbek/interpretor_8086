#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

#include <stdint.h>

typedef enum {

    MOV_REG_TO_REG=         0b10001000, // move to register/memory from register.
    IMDT_REG_TO_MEM=        0b11000110, // immidiate to register/memory
    IMDT_REG =              0b10110000, // immidiate to register
    MEM_TO_ACCUMUL =        0b10100000, //　memory to accumulator
    ACCUMUL_TO_MEM =        0b10100010, // accumulator to memory
    MEM_TO_SEGREG =         0b10001110, // memory/register to segment register
    SEGREG_TO_MEM =         0b10001100, // segment register to memory/register

} Instruction;

typedef enum {
    MOV,
    

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