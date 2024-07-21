#ifndef _CPU_H
#define _CPU_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {

    uint16_t _16bits[8];
    uint8_t _8bits[8];
    uint16_t segments[4];
    uint16_t flags;
    uint64_t ip;

} __attribute__((packed)) CPU;


//16bit registers
#define AX 0
#define CX 1
#define DX 2
#define BX 3
#define SP 4
#define BP 5
#define SI 6
#define DI 7

//8bit registers

#define AL 0
#define CL 1
#define DL 2
#define BL 3
#define AH 4
#define CH 5
#define DH 6
#define BH 7

//segment registers

#define ES 0
#define CS 1
#define SS 2
#define DS 3


// Flags
#define CF 0
#define PF 2
#define AF 4
#define ZF 6
#define SF 7
#define TF 8
#define IF 9
#define DF 10
#define OF 11

#define _16BIT 0x01
#define _8BIT  0x02

CPU cpu_init(void);

void setValue(CPU* cpu, uint8_t reg, uint16_t value, bool w);
/*
    set register to the desired value
    uses 16bit registers if w is true
    synchorinzation 16 bit general purpose registers.
*/

void mov(CPU* cpu, uint8_t mod, uint8_t reg, uint16_t ea, uint8_t suffix, uint8_t* memory);
void mov_immidiate(CPU* cpu, uint8_t mod, uint16_t ea, uint8_t suffix, uint8_t* memory, uint8_t* data_word);
void mov_segment(CPU* cpu, uint8_t mod, uint8_t sreg, uint16_t ea, uint8_t suffix, uint8_t* memory);

void push(CPU* cpu, uint8_t* memory, uint8_t mod, uint16_t ea);
void pop(CPU* cpu, uint8_t* memory, uint8_t mod, uint16_t ea);
void xchg(CPU* cpu, uint8_t* memory, uint8_t reg, uint8_t mod, uint8_t ea, uint8_t suffix);
void add(CPU* cpu, uint8_t* memory, uint8_t reg, uint8_t mod, uint8_t ea, uint8_t suffix, uint8_t carry);
void add_immidiate(CPU* cpu, uint8_t* memory, uint8_t mod, uint8_t ea, uint8_t suffix, uint8_t* data, uint8_t carry);

#endif