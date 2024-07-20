#include <stdio.h>
#include "lib/disassembly.h"
#include "lib/memory.h"
#include "lib/cpu.h"
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char* argv[]) {

    if(argc != 2) {
        printf("Syntax error\n");
        return -1;
    }

    FILE* f = fopen(argv[1], "r");
    if(!f) {
        printf("File error\n");
        return -2;
    }

    uint8_t* memory = (uint8_t*) malloc(MEMORY_SIZE);
    CPU cpu = cpu_init();
    
    fetch(f, memory);
    execute(memory, &cpu);

    fclose(f);
    free(memory);
    return 0;
}