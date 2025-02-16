#pragma once
#include <string>
#include <exception>
#include <unordered_map>

#ifndef INC_6502_6502V2_H

#define STACK_BASE 0x0100
#define ROM_LOCATION 0x0
typedef unsigned char byte;
typedef unsigned short word;

class Dodgy6502;

struct Instruction {
    std::string name;
    byte(Dodgy6502::*addr_mode)() = nullptr;
    byte(Dodgy6502::*implementation)() = nullptr;
    byte cycles;
    std::string description;
};

class Dodgy6502{
public:
    Dodgy6502();
    ~Dodgy6502();
    void reset();
    void irq(); // maskable interrupt
    void nmi(); // non-maskabl e interrupt
    void load_rom(const char *filename);
    void print_mem_around_location(int num_of_mem_adr,  int in_a_row) const;
    void display_info(int mem_neigh_size);
    void step();
    [[noreturn]] void run(bool info);
    void info_step();
    byte read(word address) const;
    void write(word address, byte data);
    void load_memory(byte* memory, word size, word offset) const;
    void push(byte data);
    byte pop();


    // registers
    byte* memory;
    byte a, x, y, sp, sb;
    word pc;

    // flag offsets
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

    // helper functions
    void set_flag(FLAGS6502 flag, bool v);
    bool read_flag(FLAGS6502 flag) const;
    void read_word(word address);
    void read_word(byte low, byte high);
    //byte get_opc(const std::string& name);
    //std::unordered_map<std::string, byte> opc_lookup_table;

    // helper variables
    word abs_addr = 0x0000;
    byte fetched = 0x00;
    word temp = 0x0000;
    Instruction* current_instruction = nullptr;


    // Addressing modes:
        byte imp(); // Implied
        byte imm(); // Immediate
        byte zp();  // Zero Page
        byte zpx(); // Zero Page X
        byte zpy(); // Zero Page Y
        //byte abs();  // Absolute == zp()
        byte abx(); // Absolute X
        byte aby(); // Absolute Y
        byte ind(); // Indirect
        //byte izx();  // Indirect X == zpx()
        //byte izy(); // Indirect Y == zpy()

    // Instructions:
    byte ADC(); byte AND(); byte ASL(); byte BCC(); byte BCS(); byte BEQ(); byte BIT(); byte BMI(); byte BNE(); byte BPL(); byte BRK(); byte BVC(); byte BVS(); byte CLC(); byte CLD(); byte CLI(); byte CLV(); byte CMP(); byte CPX(); byte CPY(); byte DEC(); byte DEX(); byte DEY(); byte EOR(); byte INC(); byte INX(); byte INY(); byte JMP(); byte JSR(); byte LDA(); byte LDX(); byte LDY(); byte LSR(); byte NOP(); byte ORA(); byte PHA(); byte PHP(); byte PLA(); byte PLP(); byte ROL(); byte ROR(); byte RTI(); byte RTS(); byte SBC(); byte SEC(); byte SED(); byte SEI(); byte STA(); byte STX(); byte STY(); byte TAX(); byte TAY(); byte TSX(); byte TXA(); byte TXS(); byte TYA();

    // Opcode lookup table (array of function pointers)
    Instruction instructions[256];

    void add_instruction(
            byte opc,
            const std::string &name, // "NOT NAMED"
            byte(Dodgy6502::*addr_mode)(),
            byte(Dodgy6502::*implementation)(),
            byte cycles,
            const std::string &description // "NO DESCRIPTION"
    );

    void add_all_instructions();


};

#define INC_6502_6502V2_H
#endif //INC_6502_6502V2_H
