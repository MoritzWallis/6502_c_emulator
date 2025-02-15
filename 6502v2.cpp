# include "6502v2.h"
# include <iostream>
# include <cstring>


Dodgy6502::Dodgy6502(){
    add_all_instructions();
    memory = new byte[64*1024]; // 64KB RAM, 16 bit address space
    memset(memory, 0x1a, 64*1024); // 0x1a = NOP - set all memory to NOP
    reset();
}

Dodgy6502::~Dodgy6502(){
    delete[] memory;
}

void Dodgy6502::reset() {
    a = x = y = 0;
    sp = 0xfd;
    sb = 0;
    pc = 0;//0xfffc;
}

void Dodgy6502::set_flag(FLAGS6502 flag, bool v){
    if(v) sb |= flag; else sb &= ~flag;
}

bool Dodgy6502::read_flag(FLAGS6502 flag) const{
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
    try{
        while(true){
            current_instruction = &instructions[memory[pc++]];
            (this->*current_instruction->addr_mode)();
            (this->*current_instruction->implementation)();
        }

    } catch(std::exception& e)
    {
        std::cerr << "Caught exception:\n\t " << e.what() << std::endl;
        std::cout << "Caught exception: " << e.what() << std::endl;
        std::cin.get();
        raise;
    }
}

int main(int argc, char* argv[]){
    Dodgy6502 cpu;
    byte rom[] = {0x09, 0x11, 0x2A}; // ORA IMM, ROL ACC
    cpu.load_memory(&rom[0], sizeof(rom), ROM_LOCATION);
    cpu.run();
    return 0;
}