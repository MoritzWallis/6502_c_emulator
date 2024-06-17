#pragma once
#include <string>

#ifndef INC_6502_6502V2_H

#define STACK_BASE 0x0100

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
    void run();
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
    char ADC(); char AND(); char ASL(); char BCC(); char BCS(); char BEQ(); char BIT(); char BMI(); char BNE(); char BPL(); char BRK(); char BVC(); char BVS(); char CLC(); char CLD(); char CLI(); char CLV(); char CMP(); char CPX(); char CPY(); char DEC(); char DEX(); char DEY(); char EOR(); char INC(); char INX(); char INY(); char JMP(); char JSR(); char LDA(); char LDX(); char LDY(); char LSR(); char NOP(); char ORA(); char PHA(); char PHP(); char PLA(); char PLP(); char ROL(); char ROR(); char RTI(); char RTS(); char SBC(); char SEC(); char SED(); char SEI(); char STA(); char STX(); char STY(); char TAX(); char TAY(); char TSX(); char TXA(); char TXS(); char TYA();

    // Opcode lookup table (array of function pointers)
    static Instruction instructions[256];

    static void add_instruction(
            byte opc,
            const std::string &name, // "NOT NAMED"
            byte(Dodgy6502::*addr_mode)(),
            byte(Dodgy6502::*implementation)(),
            byte cycles,
            const std::string &description // "NO DESCRIPTION"
    );

    static void add_all_instructions();

};

#define INC_6502_6502V2_H
#endif //INC_6502_6502V2_H
