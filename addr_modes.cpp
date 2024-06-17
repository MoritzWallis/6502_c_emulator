# include "6502v2.h"


byte Dodgy6502::imp(){
    fetched = a;
    return 0;
}

byte Dodgy6502::imm(){
    fetched = pc++; // max immediate value is 255/0xff
    return 0;
}

byte Dodgy6502::zp(){
    fetched = memory[pc++];
    return 0;
}

byte Dodgy6502::zpx(){
    abs_addr = memory[pc++] + x; // wraps around/overflows
    fetched = memory[abs_addr];
    return 0;
}

byte Dodgy6502::zpy(){
    abs_addr = memory[pc++] + y; // wraps around
    fetched = memory[abs_addr];
    return 0;
}

/*
byte Dodgy6502::abs(){
    abs_addr = memory[pc] | (memory[pc+1] << 8);
    pc += 2;
    fetched = memory[abs_addr];
    return 0;
}
*/

byte Dodgy6502::abx(){
    abs_addr = (memory[pc] | (memory[pc+1] << 8)) + x;
    pc += 2;
    fetched = memory[abs_addr];
    return 0;
}

byte Dodgy6502::aby(){
    abs_addr = (memory[pc] | (memory[pc+1] << 8)) + y;
    pc += 2;
    fetched = memory[abs_addr];
    return 0;
}

byte Dodgy6502::ind(){
    word ind_addr = memory[pc] | (memory[pc+1] << 8);
    pc += 2;
    abs_addr = memory[ind_addr] | (memory[ind_addr+1] << 8);
    fetched = memory[abs_addr];
    return 0;
}

/*
// adds x to indirect addr / zero page
byte Dodgy6502::izx(){
    fetched = ((memory[pc] | (memory[pc+1] << 8)) + x); // just reusing variables
    pc += 2;
    abs_addr = memory[fetched] | (memory[fetched+1] << 8); // wraps around when fetching abs. addr.
    fetched = memory[abs_addr];
    return 0;
}


// adds y to ZP addr / zero page
byte Dodgy6502::izy(){
    fetched = (memory[pc] | (memory[pc+1] << 8));
    pc += 2;
    abs_addr = memory[fetched] | (memory[fetched+1] << 8)  + y;
    fetched = memory[abs_addr];
    return 0;
}
*/
