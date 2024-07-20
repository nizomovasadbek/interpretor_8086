gcc -c -g src/sys/cpu.c -o build/sys/cpu.o -Wall -Wextra
gcc -c -g src/sys/disassembly.c -o build/sys/disassembly.o -Wall -Wextra
gcc -c -g src/sys/interrupt.c -o build/sys/interrupt.o -Wall -Wextra
gcc -c -g src/sys/memory.c -o build/sys/memory.o -Wall -Wextra
gcc -c -g src/sys/instruction.c -o build/sys/instruction.o -Wall -Wextra
gcc -g -Wall -Wextra build/sys/*.o src/main.c -o build/main