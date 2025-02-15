# include "6502v2.h"
# include <iostream>
# include <cstring>


Dodgy6502::Dodgy6502(){
    add_all_instructions();
    get_interaction = _get_interaction;
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

/*
 enum FLAGS6502{
        C = (1 << 0), // Carry Bit
        Z = (1 << 1), // Zero
        I = (1 << 2), // Disable Interrupts
        D = (1 << 3), // Decimal Mode
        B = (1 << 4), // Break
        COMPLETE = (1 << 5), // originally unused
        V = (1 << 6), // Overflow
        N = (1 << 7), // Negative
    } flags;
 */

void Dodgy6502::_get_interaction(char answer){
switch(answer){
    case ' ':
    case '\n':
        break;
    case 'l':
        // print last values
        break;
    case 't':
        // call_terminal();
    case 'r':
        // reset();
        break;
    case 27: // ESC
        exit(0);
        break;
    default:
        throw std::runtime_error("Invalid input");
        break;
}
}

void Dodgy6502::display_info(int mem_neigh_size = 10){
    // print out the registers and flags
    std::cout << "  PC: " << pc << "  SP: " << (int)sp << "  SB: " << (int)sb << \
    '\n' << '\n' << "  A: " << (int)a << "  X: " << (int)x << "  Y: " << (int)y << \
    '\n' << "  C: " << read_flag(C) << "  Z: " << read_flag(Z) << "  I: " << read_flag(I) << "  D: " << read_flag(D) << "  B: " << read_flag(B) << "  V: " << read_flag(V) << "  N: " << read_flag(N)<<
    '\n' << '\n' << std::endl;

    // Printout current memory neighboured
    byte first_neigh = (pc - mem_neigh_size) < 0 ? 0 : pc - mem_neigh_size;
    for(int i = first_neigh; i < pc + mem_neigh_size; i++){
        std::cout << "   " << i << ": " << (int)memory[i] << std::endl;
    }

    char answer;
    std::cin.get(answer);
    get_interaction(answer)();
}

void Dodgy6502::step(){
    try{
        current_instruction = &instructions[memory[pc++]];
        (this->*current_instruction->addr_mode)();
        (this->*current_instruction->implementation)();

    } catch(std::exception& e)
    {
        std::cerr << "Caught exception:\n\t " << e.what() << std::endl;
        std::cout << "Caught exception: " << e.what() << std::endl;
        std::cin.get();
        raise;
    }
}

void Dodgy6502::run(){
    while(true){
        step();
    }
}

void Dodgy6502::info_step(){
    step();
    display_info();
}

int main(int argc, char* argv[]){
    Dodgy6502 cpu;
    byte rom[] = {0x09, 0x11, 0x2A}; // ORA IMM, ROL ACC
    cpu.load_memory(&rom[0], sizeof(rom), ROM_LOCATION);
    cpu.run();
    return 0;
}