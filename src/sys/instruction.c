#include "../lib/instruction.h"

IST identify(uint8_t ins, IST table[], uint32_t table_size) {
    uint8_t mask = 0;
    uint8_t identity = 0;
    for(uint8_t i = 0; i < table_size; i++) {
        identity = ins;
        mask = (2 << (table[i].len - 1)) - 1;
        mask <<= 8-table[i].len;
        identity &= mask;
        if(identity == table[i].ins) {
            return table[i];
        }
    }

    IST ist;
    ist.type = NONE;
    return ist;
}