# include "6502v2.h"
# include <iostream>
# include <string>



Dodgy6502::Dodgy6502(){
    memory = new byte[64*1024]; // 64KB RAM, 16 bit address space
    reset();
    run();
}

Dodgy6502::~Dodgy6502(){
    delete[] memory;
}

void Dodgy6502::reset() {
    a = x = y = 0;
    sp = 0xfd;
    sb = 0;
    pc = 0xfffc;
}

void Dodgy6502::set_flag(FLAGS6502 flag, bool v){
    if(v) sb |= flag; else sb &= ~flag;
}

bool Dodgy6502::read_flag(FLAGS6502 flag){
    return sb & flag;
}

void Dodgy6502::read_word(word address){
    fetched = memory[address] | (memory[address+1] << 8);
}
void Dodgy6502::read_word(byte low, byte high){
    fetched = memory[low | (high << 8)];
}

void Dodgy6502::push(byte data){
    memory[STACK_BASE + sp--] = data;
}

byte Dodgy6502::pop(){
    return memory[STACK_BASE + ++sp];
}

void Dodgy6502::irq(){}
void Dodgy6502::nmi(){}

void Dodgy6502::run(){
    while(true){
        current_instruction = &instructions[memory[pc++]];
        (this->*current_instruction->addr_mode)();
        (this->*current_instruction->implementation)();
    }
}


int main(int argc, char *argv[]){
    return 0;
}