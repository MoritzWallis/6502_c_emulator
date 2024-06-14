#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SET_FLAG(flag) self->SR |= (1 << flag);
#define CLEAR_FLAG(flag) self->SR &= ~(1 << flag);
#define STACK_BASE 0x0100
#define MSB 0x80
#define LSB 0x01

/*
    Accumulator (AC): 8 bits
    Index registers (X and Y): 8 bits each
    Status register (P): 8 bits (Contains 7 flag bits and one reserved bit)
    Stack pointer (S): 8 bits
    Program counter (PC): 16 bits (internally divided into 8-bit high (PCH) and low (PCL) parts)
*/

typedef struct {
    // Accumulator (AC) / index Registers: 8 bits
    char AC;
    char X;
    char Y;

    // Stack pointer (S): 8 bits
    char SP;

    // Program counter (PC): 16 bits (internally divided into 8-bit high (PCH) and low (PCL) parts)
    short PC;

    /*
      Status register (P): 8 bits (Contains 7 flag bits and one reserved bit)
        N	Negative (bit 8)
        V	Overflow
        -	ignored
        B	Break
        D	Decimal (use BCD for arithmetics)
        I	Interrupt (IRQ disable)
        Z	Zero
        C	Carry (bit 0)
    */
    char SR;

    // RAM: 64KB - 16 bit address space
    // - Zero Page (0x0000 to 0x00FF): Used for quick access to frequently used variables.
    // - Stack (0x0100 to 0x01FF): Stack memory.
    // - Program Code and Data (0x0200 to 0xFFFF): Program instructions and data.
    char memory[65536];


    int (*run)(void* self);
    int (*reset)();

    unsigned int cycles_executed;
} Mos6502;

typedef enum {
    IMPLIED,
    IMMEDIATE,
    ABSOLUTE,
    ZERO_PAGE,
    ZERO_PAGE_OFFSET,
    INDEXED,
    INDIRECT,
    RELATIVE
} ADDRESSING_MODE;

typedef enum { // N V - B D I Z C
    C, Z, I, D, B, _, V, N
} FLAG_OFFSET;

typedef struct {
    const char *mnemonic;
    void (*execute)();
    char cycles;
    ADDRESSING_MODE addressingMode;
} Instruction;

int print_state(Mos6502* cpu);
int crash(Mos6502* cpu, char* message);
int run(void* _self);


void adcImmediate(Mos6502* self){
    self->AC = self->memory[self->PC++];
}

void adcZeroPage(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++]];
}

void adcZeroPageX(Mos6502* self){
self->AC = self->memory[self->memory[self->PC++] + self->X];
}

void adcAbsolute(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8)];
}

void adcAbsoluteX(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8) + self->X];
}

void adcAbsoluteY(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8) + self->Y];
}

// In Indirect, X addressing mode, the instruction specifies a zero-page address which points to the low byte of a 16-bit address in memory. The X register is then added to this zero-page address to compute the final effective address.
void adcIndirectX(Mos6502* self){
    self->AC = self->memory[self->PC++ + self->X];
}

void adcIndirectY(Mos6502* self){
    self->AC = self->memory[self->PC++ + self->Y];
}

void andImmediate(Mos6502* self){
    self->AC &= self->memory[self->PC++];
}

void andZeroPage(Mos6502* self){
    self->AC &= self->memory[self->memory[self->PC++]];
}

void andZeroPageX(Mos6502* self){
    self->AC &= self->memory[self->memory[self->PC++] + self->X];
}

void andAbsolute(Mos6502* self){
self->AC &= self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8)];
}

void andAbsoluteX(Mos6502* self){
    self->AC &= self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8) + self->X];
}

void andAbsoluteY(Mos6502* self){
    self->AC &= self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8) + self->Y];
}

void andIndirectX(Mos6502* self){
    self->AC &= self->memory[self->PC++ + self->X];
}

void andIndirectY(Mos6502* self) {
    self->AC &= self->memory[self->PC++ + self->Y];
}

void aslImmediate(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++]];
}

void aslZeroPage(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++]];
}

void aslZeroPageX(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++] + self->X];
}

void aslAbsolute(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8)];
}

void aslAbsoluteX(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8) + self->X];
}

void bccRelative(Mos6502* self){
    if(!(self->SR & (1 << C)))
        self->PC += self->memory[self->PC];
    self->PC++;
}

void bcsRelative(Mos6502* self){
    if(self->SR & (1 << C))
        self->PC += self->memory[self->PC];
    self->PC++;
}

void beqRelative(Mos6502* self){
    if(self->SR & (1 << Z))
        self->PC += self->memory[self->PC];
    self->PC++;
}

void bitZeroPage(Mos6502* self){
    crash(self, "Not implemented");
    //self->AC = self->memory[self->memory[self->PC++]];
}

void bitAbsolute(Mos6502* self){
    crash(self, "Not implemented"); //self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8)];
}

void bmiRelative(Mos6502* self){
    if(self->SR & (1 << N))
        self->PC += self->memory[self->PC];
    self->PC++;
}

void bneRelative(Mos6502* self){
    if(!(self->SR & (1 << Z)))
        self->PC += self->memory[self->PC];
    self->PC++;
}

void bplRelative(Mos6502* self){
    if(!(self->SR & (1 << N)))
        self->PC += self->memory[self->PC];
    self->PC++;
}

void brkImplied(Mos6502* self){
    crash(self, "Not implemented");
}

/*The BVC instruction tests the Overflow flag (V) in the Processor Status Register (P). If the Overflow flag is clear (indicating that the result of the previous operation did not overflow), the program counter (PC) is incremented by the relative offset specified in the instruction, causing a branch to the new memory address. If the Overflow flag is set (indicating that the result overflowed), the instruction is not taken, and program execution continues with the next instruction.*/
void bvcRelative(Mos6502* self){
    if(!(self->SR & (1 << V)))
        self->PC += self->memory[self->PC];
    self->PC++;
}

void bvsRelative(Mos6502* self){
    if(self->SR & (1 << V))
        self->PC += self->memory[self->PC];
    self->PC++;
}


void clcImplied(Mos6502* self){
    self->SR &= ~(1 << C);
}

void cldImplied(Mos6502* self){
    self->SR &= ~(1 << D);
}

void cliImplied(Mos6502* self){
    self->SR &= ~(1 << I);
}

void clvImplied(Mos6502* self){
    self->SR &= ~(1 << V);
}

void cmpImmediate(Mos6502* self){
    crash(self, "Not implemented");//self->AC = self->memory[self->memory[self->PC++]];
}

void cmpZeroPage(Mos6502* self){
    crash(self, "Not implemented");
}

void cmpZeroPageX(Mos6502* self){
    crash(self, "Not implemented");
}

void cmpAbsolute(Mos6502* self){
    crash(self, "Not implemented");
}

void cmpAbsoluteX(Mos6502* self){
    crash(self, "Not implemented");
}

void cmpAbsoluteY(Mos6502* self){
    crash(self, "Not implemented");
}

void cmpIndirectX(Mos6502* self){
    crash(self, "Not implemented");
}

void cmpIndirectY(Mos6502* self){
    crash(self, "Not implemented");
}


void cpxImmediate(Mos6502* self){
    if(self->X == self->memory[self->PC++])
        SET_FLAG(Z)
    else
        CLEAR_FLAG(Z)
}

void cpxZeroPage(Mos6502* self){
    if(self->X == self->memory[self->memory[self->PC++]])
        SET_FLAG(Z)
    else
        CLEAR_FLAG(Z)
}

void cpxAbsolute(Mos6502* self){
    if(self->X == self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8)])
        SET_FLAG(Z)
    else
        SET_FLAG(Z)
}

void cpyImmediate(Mos6502* self){
    if(self->Y == self->memory[self->PC++])
        SET_FLAG(Z)
    else
        CLEAR_FLAG(Z)
}

void cpyZeroPage(Mos6502* self){
    if(self->Y == self->memory[self->memory[self->PC++]])
        SET_FLAG(Z)
    else
        CLEAR_FLAG(Z)
}

void cpyAbsolute(Mos6502* self){
    if(self->Y == self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8)])
        SET_FLAG(Z)
    else
        CLEAR_FLAG(Z)
}

void decZeroPage(Mos6502* self){
    self->memory[self->memory[self->PC++]]--;
}

void decZeroPageX(Mos6502* self){
    self->memory[self->memory[self->PC++] + self->X]--;
}

void decAbsolute(Mos6502* self){
    self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8)]--;
}

void decAbsoluteX(Mos6502* self){
    self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8) + self->X]--;
}

void dexImplied(Mos6502* self){
    self->X--;
}

void deyImplied(Mos6502* self){
    self->Y--;
}

void eorImmediate(Mos6502* self){
    self->AC ^= self->memory[self->PC++];
}

void eorZeroPage(Mos6502* self){
    self->AC ^= self->memory[self->memory[self->PC++]];
}

void eorZeroPageX(Mos6502* self){
    self->AC ^= self->memory[self->memory[self->PC++] + self->X];
}

void eorAbsolute(Mos6502* self){
    self->AC ^= self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8)];
}

void eorAbsoluteX(Mos6502* self){
    self->AC ^= self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8) + self->X];
}

void eorAbsoluteY(Mos6502* self){
    self->AC ^= self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8) + self->Y];
}

void eorIndirectX(Mos6502* self) {
    self->AC ^= self->memory[self->PC++ + self->X];
}

void eorIndirectY(Mos6502* self) {
    self->AC ^= self->memory[self->PC++ + self->Y];
}

void incZeroPage(Mos6502* self) {
    self->memory[self->memory[self->PC++]]++;
}

void incZeroPageX(Mos6502* self) {
    self->memory[self->memory[self->PC++] + self->X]++;
}

void incAbsolute(Mos6502* self) {
    self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8)]++;
}

void incAbsoluteX(Mos6502* self) {
    self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8) + self->X]++;
}

void inxImplied(Mos6502* self){
    self->X++;
}

void inyImplied(Mos6502* self){
    self->Y++;
}

void jmpAbsolute(Mos6502* self){
    self->PC = self->memory[self->PC++] | (self->memory[self->PC++] << 8);
}

void jmpIndirect(Mos6502* self){
    self->PC = self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8)];
}

void jsrAbsolute(Mos6502* self){
    // Calculate the return address
    short returnAddress = self->PC + 2;

    // Push the return address onto the stack in little-endian order
    self->memory[STACK_BASE + self->SP--] = returnAddress & 0xFF; // LSB
    self->memory[STACK_BASE + self->SP--] = returnAddress >> 8;   // MSB

    // Jump to the address specified by the operand
    self->PC = self->memory[self->PC] | (self->memory[self->PC + 1] << 8);
}

void ldaImmediate(Mos6502* self){
    self->AC = self->memory[self->PC++];
}

void ldaZeroPage(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++]];
}

void ldaZeroPageX(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++] + self->X];
}

void ldaAbsolute(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8)];
}

void ldaAbsoluteX(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8) + self->X];
}

void ldaAbsoluteY(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++] + (self->memory[self->PC++] << 8) + self->Y];
}

void ldaIndirectX(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++ + self->X]];
}

void ldaIndirectY(Mos6502* self){
    self->AC = self->memory[self->memory[self->PC++] + self->Y];
}

void ldxImmediate(Mos6502* self){
    self->X = self->memory[self->PC++];
}

void ldxZeroPage(Mos6502* self){
    self->X = self->memory[self->memory[self->PC++]];
}

void ldxZeroPageY(Mos6502* self){
    self->X = self->memory[self->memory[self->PC++] + self->Y];
}

void ldxAbsolute(Mos6502* self){
    self->X = self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8)];
}

void ldxAbsoluteY(Mos6502* self){
    self->X = self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8) + self->Y];
}

void ldyImmediate(Mos6502* self){
    self->Y = self->memory[self->PC++];
}

void ldyZeroPage(Mos6502* self){
    self->Y = self->memory[self->memory[self->PC++]];
}

void ldyZeroPageX(Mos6502* self){
    self->Y = self->memory[self->memory[self->PC++] + self->X];
}

void ldyAbsolute(Mos6502* self){
    self->Y = self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8)];
}

void ldyAbsoluteX(Mos6502* self){
    self->Y = self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8) + self->X];
}

void lsrImmediate(Mos6502* self){
    // Set carry flag to MSB of AC
    self->SR &= ((MSB & self->AC) >> 7) | 0xFE;
    self->AC >>= 1; // Shift right
    // Clear negative flag
    self->SR &= 0x7F;
}

void lsrZeroPage(Mos6502* self){
    // Set carry flag to MSB of AC
    self->SR &= ((MSB & self->memory[self->memory[self->PC]]) >> 7) | 0xFE;
    self->memory[self->memory[self->PC++]] >>= 1; // Shift right

    // Clear negative flag
    self->SR &= 0x7F;
}

void lsrZeroPageX(Mos6502* self){
    // Set carry flag to MSB of AC
    self->SR &= ((MSB & (self->memory[self->memory[self->PC] + self->X])) >> 7) | 0xFE;
    self->memory[self->memory[self->PC++] + self->X] >>= 1; // Shift right

    // Clear negative flag
    self->SR &= 0x7F;
}

void lsrAbsolute(Mos6502* self){
    // Set carry flag to MSB of AC

    self->SR &= ((MSB & (self->memory[self->memory[self->PC] | (self->memory[self->PC + 1] << 8)])) >> 7) | 0xFE;
    self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8)] >>= 1; // Shift right

    // Clear negative flag
    self->SR &= 0x7F;
}

void lsrAbsoluteX(Mos6502* self){
    // Set carry flag to MSB of AC
    self->SR &= ((MSB & (self->memory[self->memory[self->PC] | (self->memory[self->PC + 1] << 8) + self->X])) >> 7) | 0xFE;
    self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8) + self->X] >>= 1; // Shift right

    // Clear negative flag
    self->SR &= 0x7F;
}

void nopImplied(Mos6502* self){
}

void oraImmediate(Mos6502* self){
    self->AC |= self->memory[self->PC++];
}

void oraZeroPage(Mos6502* self){
    self->AC |= self->memory[self->memory[self->PC++]];
}

void oraZeroPageX(Mos6502* self){
    self->AC |= self->memory[self->memory[self->PC++] + self->X];
}

void oraAbsolute(Mos6502* self){
self->AC |= self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8)];
}

void oraAbsoluteX(Mos6502* self){
    self->AC |= self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8) + self->X];
}

void oraAbsoluteY(Mos6502* self){
    self->AC |= self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8) + self->Y];
}

void oraIndirectX(Mos6502* self){
    self->AC |= self->memory[self->memory[self->PC++] + self->X];
}

void oraIndirectY(Mos6502* self){
    self->AC |= self->memory[self->memory[self->PC++] + self->Y];
}

void phaImplied(Mos6502* self){
    self->memory[STACK_BASE + self->SP--] = self->AC;
}

void phpImplied(Mos6502* self){
    self->memory[STACK_BASE + self->SP--] = self->SR;
}

void plaImplied(Mos6502* self){
    self->AC = self->memory[STACK_BASE + ++self->SP];
}

void plpImplied(Mos6502* self){
    self->SR = self->memory[STACK_BASE + ++self->SP];
}

void rolImmediate(Mos6502* self){
    char carry = self->SR & 1;
    char new_carry = (self->AC & MSB) >> 7;
    self->AC <<= 1;
    self->AC |= carry;
    self->SR &= 0xFE;
    self->SR |= new_carry;
}

void rolZeroPage(Mos6502* self){
    char carry = self->SR & 1;
    char new_carry = (self->memory[self->memory[self->PC]] & MSB) >> 7;
    self->memory[self->memory[self->PC]] <<= 1;
    self->memory[self->memory[self->PC++]] |= carry;
    self->SR &= 0xFE;
    self->SR |= new_carry;
}

void rolZeroPageX(Mos6502* self){
    char carry = self->SR & 1;
    char new_carry = (self->memory[self->memory[self->PC] + self->X] & MSB) >> 7;
    self->memory[self->memory[self->PC] + self->X] <<= 1;
    self->memory[self->memory[self->PC++] + self->X] |= carry;
    self->SR &= 0xFE;
    self->SR |= new_carry;
}

void rolAbsolute(Mos6502* self){
    char carry = self->SR & 1;
    char* location = &self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8)];
    char new_carry = (*location & MSB) >> 7;
    *location <<= 1;
    *location |= carry;
    self->SR &= 0xFE;
    self->SR |= new_carry;
}

void rolAbsoluteX(Mos6502* self){
    char carry = self->SR & 1;
    char* location = &self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8) + self->X];
    char new_carry = (*location & MSB) >> 7;
    *location <<= 1;
    *location |= carry;
    self->SR &= 0xFE;
    self->SR |= new_carry;
}


void rorImmediate(Mos6502* self){
    char carry = self->SR & 1;
    char new_carry = self->AC & 1;
    self->AC >>= 1;
    self->AC |= carry << 7;
    self->SR &= 0xFE;
    self->SR |= new_carry;
}

void rorZeroPage(Mos6502* self){
    char carry = self->SR & 1;
    char* location = &self->memory[self->memory[self->PC]];
    char new_carry = *location & 1;
    *location >>= 1;
    *location |= carry << 7;
    self->PC++;
    self->SR &= 0xFE;
    self->SR |= new_carry;
}

void rorZeroPageX(Mos6502* self){
    char carry = self->SR & 1;
    char* location = &(self->memory[self->memory[self->PC] + self->X]);
    char new_carry = *location & 1;
    *location >>= 1;
    *location |= carry << 7;
    self->PC++;
    self->SR &= 0xFE;
    self->SR |= new_carry;
}

void rorAbsolute(Mos6502* self){
    char carry = self->SR & 1;
    char* location = &self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8)];
    char new_carry = *location & 1;
    *location >>= 1;
    *location |= carry << 7;
    self->SR &= 0xFE;
    self->SR |= new_carry;
}

void rorAbsoluteX(Mos6502* self){
    char carry = self->SR & 1;
    char* location = &self->memory[self->memory[self->PC++] | (self->memory[self->PC++] << 8) + self->X];
    char new_carry = *location & 1;
    *location >>= 1;
    *location |= carry << 7;
    self->SR &= 0xFE;
    self->SR |= new_carry;
}

void rtiImplied(Mos6502* self){

}

void rtsImplied(Mos6502* self){
}

void sbcImmediate(Mos6502* self){
}

void sbcZeroPage(Mos6502* self){
}

void sbcZeroPageX(Mos6502* self){
}

void sbcAbsolute(Mos6502* self){
}

void sbcAbsoluteX(Mos6502* self){
}

void sbcAbsoluteY(Mos6502* self){
}

void sbcIndirectX(Mos6502* self){
}

void sbcIndirectY(Mos6502* self){
}

void secImplied(Mos6502* self){
}

void sedImplied(Mos6502* self){
}

void seiImplied(Mos6502* self){
}

void staZeroPage(Mos6502* self){
}

void staZeroPageX(Mos6502* self){
}

void staAbsolute(Mos6502* self){
}

void staAbsoluteX(Mos6502* self){
}

void staAbsoluteY(Mos6502* self){
}

void staIndirectX(Mos6502* self){
}

void staIndirectY(Mos6502* self){
}

void stxZeroPage(Mos6502* self){
}

void stxZeroPageY(Mos6502* self){
}

void stxAbsolute(Mos6502* self){
}

void styZeroPage(Mos6502* self){
}

void styZeroPageX(Mos6502* self){
}

void styAbsolute(Mos6502* self){
}

void taxImplied(Mos6502* self){
}

void tayImplied(Mos6502* self){
}

void tsxImplied(Mos6502* self){
}

void txaImplied(Mos6502* self){
}

void txsImplied(Mos6502* self){
}

void tyaImplied(Mos6502* self){
}




Instruction instructionSet[256] = {
        /*
         ADC
            Add Memory to Accumulator with Carry

            A + M + C -> A, C
            N	Z	C	I	D	V
            +	+	+	-	-	+
            addressing	assembler	opc	bytes	cycles
            immediate	ADC #oper	69	2	    2
            zeropage	ADC oper	65	2	    3
            zeropage,X	ADC oper,X	75	2	    4
            absolute	ADC oper	6D	3	    4
            absolute,X	ADC oper,X	7D	3	    4*
            absolute,Y	ADC oper,Y	79	3	    4*
            (indirect,X)	ADC (oper,X) 61	2	6
            (indirect),Y	ADC (oper),Y 71	2	5*
         */
        [0x69] = {"ADC", ldaImmediate, 2, IMMEDIATE},
        [0x65] = {"ADC", ldaZeroPage, 2, ZERO_PAGE},
        [0x75] = {"ADC", ldaZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0x6D] = {"ADC", ldaAbsolute, 3, ABSOLUTE},
        [0x7D] = {"ADC", ldaAbsoluteX, 3, INDEXED},
        [0x79] = {"ADC", ldaAbsoluteY, 3, INDEXED},
        [0x61] = {"ADC", ldaIndirectX, 2, INDIRECT},
        [0x71] = {"ADC", ldaIndirectY, 2, INDIRECT},
        /*
         ASL
            Shift Left One Bit (Memory or Accumulator)

            C <- [76543210] <- 0
            N	Z	C	I	D	V
            +	+	+	-	-	-
            addressing	assembler	opc	bytes	cycles
            accumulator	ASL A	0A	1	2
            zeropage	ASL oper	06	2	5
            zeropage,X	ASL oper,X	16	2	6
            absolute	ASL oper	0E	3	6
            absolute,X	ASL oper,X	1E	3	7
         */
        [0x0A] = {"ASL", aslImmediate, 1, IMPLIED},
        [0x06] = {"ASL", aslZeroPage, 2, ZERO_PAGE},
        [0x16] = {"ASL", aslZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0x0E] = {"ASL", aslAbsolute, 3, ABSOLUTE},
        [0x1E] = {"ASL", aslAbsoluteX, 3, INDEXED},
        /*
         BCC
            Branch on Carry Clear

            branch on C = 0
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            relative	BCC oper	90	2	2**
         */
        [0x90] = {"BCC", bccRelative, 2, RELATIVE},
        /*BCS
            Branch on Carry Set

            branch on C = 1
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            relative	BCS oper	B0	2	2**
        */
        [0xB0] = {"BCS", bcsRelative, 2, RELATIVE},
        /*
         BEQ
            Branch on Result Zero

            branch on Z = 1
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            relative	BEQ oper	F0	2	2**
         */
        [0xF0] = {"BEQ", beqRelative, 2, RELATIVE},
        /*
         BIT
            Test Bits in Memory with Accumulator

            bits 7 and 6 of operand are transfered to bit 7 and 6 of SR (N,V);
            the zero-flag is set according to the result of the operand AND
            the accumulator (set, if the result is zero, unset otherwise).
            This allows a quick check of a few bits at once without affecting
            any of the registers, other than the status register (SR).

            A AND M -> Z, M7 -> N, M6 -> V
            N	Z	C	I	D	V
            M7	+	-	-	-	M6
            addressing	assembler	opc	bytes	cycles
            zeropage	BIT oper	24	2	3
            absolute	BIT oper	2C	3	4
        */
        [0x24] = {"BIT", bitZeroPage, 2, ZERO_PAGE},
        [0x2C] = {"BIT", bitAbsolute, 3, ABSOLUTE},
        /*
        BMI
        Branch on Result Minus

        branch on N = 1
        N	Z	C	I	D	V
        -	-	-	-	-	-
        addressing	assembler	opc	bytes	cycles
        relative	BMI oper	30	2	2**
        */
        [0x30] = {"BMI", bmiRelative, 2, RELATIVE},
        /*
         BNE
            Branch on Result not Zero

            branch on Z = 0
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            relative	BNE oper	D0	2	2**
         */
        [0xD0] = {"BNE", bneRelative, 2, RELATIVE},
        /*
         BPL
            Branch on Result Plus

            branch on N = 0
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            relative	BPL oper	10	2	2**
        */
        [0x10] = {"BPL", bplRelative, 2, RELATIVE},
        /*
        BRK
            Force Break

            BRK initiates a software interrupt similar to a hardware
            interrupt (IRQ). The return address pushed to the stack is
            PC+2, providing an extra byte of spacing for a break mark
            (identifying a reason for the break.)
            The status register will be pushed to the stack with the break
            flag set to 1. However, when retrieved during RTI or by a PLP
            instruction, the break flag will be ignored.
            The interrupt disable flag is not set automatically.

            interrupt,
            push PC+2, push SR
            N	Z	C	I	D	V
            -	-	-	1	-	-
            addressing	assembler	opc	bytes	cycles
            implied	BRK	00	1	7
        */
        [0x00] = {"BRK", brkImplied, 1, IMPLIED},
        /*
        BVC
            Branch on Overflow Clear

            branch on V = 0
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            relative	BVC oper	50	2	2**
        */
        [0x50] = {"BVC", bvcRelative, 2, RELATIVE},
        /*
        BVS
        Branch on Overflow Set

        branch on V = 1
        N	Z	C	I	D	V
        -	-	-	-	-	-
        addressing	assembler	opc	bytes	cycles
        relative	BVS oper	70	2	2**
        */
        [0x70] = {"BVS", bvsRelative, 2, RELATIVE},
        /*
         CLC
            Clear Carry Flag

            0 -> C
            N	Z	C	I	D	V
            -	-	0	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	CLC	18	1	2
         */
        [0x18] = {"CLC", clcImplied, 1, IMPLIED},
        /*
        CLD
            Clear Decimal Mode

            0 -> D
            N	Z	C	I	D	V
            -	-	-	-	0	-
            addressing	assembler	opc	bytes	cycles
            implied	CLD	D8	1	2
        */
        [0xD8] = {"CLD", cldImplied, 1, IMPLIED},
        /*
        CLI
            Clear Interrupt Disable Bit

            0 -> I
            N	Z	C	I	D	V
            -	-	-	0	-	-
            addressing	assembler	opc	bytes	cycles
            implied	CLI	58	1	2
            CLV
            Clear Overflow Flag

            0 -> V
            N	Z	C	I	D	V
            -	-	-	-	-	0
            addressing	assembler	opc	bytes	cycles
            implied	CLV	B8	1	2
        */
        [0x58] = {"CLI", cliImplied, 1, IMPLIED},
        [0xB8] = {"CLV", clvImplied, 1, IMPLIED},
        /*
        CMP
            Compare Memory with Accumulator

            A - M
            N	Z	C	I	D	V
            +	+	+	-	-	-
            addressing	assembler	opc	bytes	cycles
            immediate	CMP #oper	C9	2	2
            zeropage	CMP oper	C5	2	3
            zeropage,X	CMP oper,X	D5	2	4
            absolute	CMP oper	CD	3	4
            absolute,X	CMP oper,X	DD	3	4*
            absolute,Y	CMP oper,Y	D9	3	4*
            (indirect,X)	CMP (oper,X)	C1	2	6
            (indirect),Y	CMP (oper),Y	D1	2	5*
        */
        [0xC9] = {"CMP", cmpImmediate, 2, IMMEDIATE},
        [0xC5] = {"CMP", cmpZeroPage, 2, ZERO_PAGE},
        [0xD5] = {"CMP", cmpZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0xCD] = {"CMP", cmpAbsolute, 3, ABSOLUTE},
        [0xDD] = {"CMP", cmpAbsoluteX, 3, INDEXED},
        [0xD9] = {"CMP", cmpAbsoluteY, 3, INDEXED},
        [0xC1] = {"CMP", cmpIndirectX, 2, INDIRECT},
        [0xD1] = {"CMP", cmpIndirectY, 2, INDIRECT},
        /*
        CPX
            Compare Memory and Index X

            X - M
            N	Z	C	I	D	V
            +	+	+	-	-	-
            addressing	assembler	opc	bytes	cycles
            immediate	CPX #oper	E0	2	2
            zeropage	CPX oper	E4	2	3
            absolute	CPX oper	EC	3	4
        */
        [0xE0] = {"CPX", cpxImmediate, 2, IMMEDIATE},
        [0xE4] = {"CPX", cpxZeroPage, 2, ZERO_PAGE},
        [0xEC] = {"CPX", cpxAbsolute, 3, ABSOLUTE},
        /*
        CPY
            Compare Memory and Index Y

            Y - M
            N	Z	C	I	D	V
            +	+	+	-	-	-
            addressing	assembler	opc	bytes	cycles
            immediate	CPY #oper	C0	2	2
            zeropage	CPY oper	C4	2	3
            absolute	CPY oper	CC	3	4
        */
        [0xC0] = {"CPY", cpyImmediate, 2, IMMEDIATE},
        [0xC4] = {"CPY", cpyZeroPage, 2, ZERO_PAGE},
        [0xCC] = {"CPY", cpyAbsolute, 3, ABSOLUTE},
        /*
        DEC
            Decrement Memory by One

            M - 1 -> M
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            zeropage	DEC oper	C6	2	5
            zeropage,X	DEC oper,X	D6	2	6
            absolute	DEC oper	CE	3	6
            absolute,X	DEC oper,X	DE	3	7
        */
        [0xC6] = {"DEC", decZeroPage, 2, ZERO_PAGE},
        [0xD6] = {"DEC", decZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0xCE] = {"DEC", decAbsolute, 3, ABSOLUTE},
        [0xDE] = {"DEC", decAbsoluteX, 3, INDEXED},
        /*
        DEX
            Decrement Index X by One

            X - 1 -> X
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	DEX	CA	1	2
            DEY
            Decrement Index Y by One

            Y - 1 -> Y
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	DEY	88	1	2
        */
        [0xCA] = {"DEX", dexImplied, 1, IMPLIED},
        [0x88] = {"DEY", deyImplied, 1, IMPLIED},
        /*
        EOR
            Exclusive-OR Memory with Accumulator

            A EOR M -> A
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            immediate	EOR #oper	49	2	2
            zeropage	EOR oper	45	2	3
            zeropage,X	EOR oper,X	55	2	4
            absolute	EOR oper	4D	3	4
            absolute,X	EOR oper,X	5D	3	4*
            absolute,Y	EOR oper,Y	59	3	4*
            (indirect,X)	EOR (oper,X)	41	2	6
            (indirect),Y	EOR (oper),Y	51	2	5*
        */
        [0x49] = {"EOR", eorImmediate, 2, IMMEDIATE},
        [0x45] = {"EOR", eorZeroPage, 2, ZERO_PAGE},
        [0x55] = {"EOR", eorZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0x4D] = {"EOR", eorAbsolute, 3, ABSOLUTE},
        [0x5D] = {"EOR", eorAbsoluteX, 3, INDEXED},
        [0x59] = {"EOR", eorAbsoluteY, 3, INDEXED},
        [0x41] = {"EOR", eorIndirectX, 2, INDIRECT},
        [0x51] = {"EOR", eorIndirectY, 2, INDIRECT},
        /*
        INC
            Increment Memory by One

            M + 1 -> M
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            zeropage	INC oper	E6	2	5
            zeropage,X	INC oper,X	F6	2	6
            absolute	INC oper	EE	3	6
            absolute,X	INC oper,X	FE	3	7
        */
        [0xE6] = {"INC", incZeroPage, 2, ZERO_PAGE},
        [0xF6] = {"INC", incZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0xEE] = {"INC", incAbsolute, 3, ABSOLUTE},
        [0xFE] = {"INC", incAbsoluteX, 3, INDEXED},
        /*
        INX
            Increment Index X by One

            X + 1 -> X
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	INX	E8	1	2
            INY
            Increment Index Y by One

            Y + 1 -> Y
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	INY	C8	1	2
        */
        [0xE8] = {"INX", inxImplied, 1, IMPLIED},
        [0xC8] = {"INY", inyImplied, 1, IMPLIED},
        /*
        JMP
            Jump to New Location

            operand 1st byte -> PCL
            operand 2nd byte -> PCH
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            absolute	JMP oper	4C	3	3
            indirect	JMP (oper)	6C	3	5
        */
        [0x4C] = {"JMP", jmpAbsolute, 3, ABSOLUTE},
        [0x6C] = {"JMP", jmpIndirect, 3, INDIRECT},

        /*
        JSR
            Jump to New Location Saving Return Address

            push (PC+2),
            operand 1st byte -> PCL
            operand 2nd byte -> PCH
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            absolute	JSR oper	20	3	6
        */
        [0x20] = {"JSR", jsrAbsolute, 3, ABSOLUTE},

        /*
        LDA
            Load Accumulator with Memory

            M -> A
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            immediate	LDA #oper	A9	2	2
            zeropage	LDA oper	A5	2	3
            zeropage,X	LDA oper,X	B5	2	4
            absolute	LDA oper	AD	3	4
            absolute,X	LDA oper,X	BD	3	4*
            absolute,Y	LDA oper,Y	B9	3	4*
            (indirect,X)	LDA (oper,X)	A1	2	6
            (indirect),Y	LDA (oper),Y	B1	2	5*
        */
        [0xA9] = {"LDA", ldaImmediate, 2, IMMEDIATE},
        [0xA5] = {"LDA", ldaZeroPage, 2, ZERO_PAGE},
        [0xB5] = {"LDA", ldaZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0xAD] = {"LDA", ldaAbsolute, 3, ABSOLUTE},
        [0xBD] = {"LDA", ldaAbsoluteX, 3, INDEXED},
        [0xB9] = {"LDA", ldaAbsoluteY, 3, INDEXED},
        [0xA1] = {"LDA", ldaIndirectX, 2, INDIRECT},
        [0xB1] = {"LDA", ldaIndirectY, 2, INDIRECT},

        /*
        LDX
            Load Index X with Memory

            M -> X
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            immediate	LDX #oper	A2	2	2
            zeropage	LDX oper	A6	2	3
            zeropage,Y	LDX oper,Y	B6	2	4
            absolute	LDX oper	AE	3	4
            absolute,Y	LDX oper,Y	BE	3	4*
        */
        [0xA2] = {"LDX", ldxImmediate, 2, IMMEDIATE},
        [0xA6] = {"LDX", ldxZeroPage, 2, ZERO_PAGE},
        [0xB6] = {"LDX", ldxZeroPageY, 2, ZERO_PAGE_OFFSET},
        [0xAE] = {"LDX", ldxAbsolute, 3, ABSOLUTE},
        [0xBE] = {"LDX", ldxAbsoluteY, 3, INDEXED},

        /*
        LDY
            Load Index Y with Memory

            M -> Y
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            immediate	LDY #oper	A0	2	2
            zeropage	LDY oper	A4	2	3
            zeropage,X	LDY oper,X	B4	2	4
            absolute	LDY oper	AC	3	4
            absolute,X	LDY oper,X	BC	3	4*
        */
        [0xA0] = {"LDY", ldyImmediate, 2, IMMEDIATE},
        [0xA4] = {"LDY", ldyZeroPage, 2, ZERO_PAGE},
        [0xB4] = {"LDY", ldyZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0xAC] = {"LDY", ldyAbsolute, 3, ABSOLUTE},
        [0xBC] = {"LDY", ldyAbsoluteX, 3, INDEXED},

        /*
        LSR
            Shift One Bit Right (Memory or Accumulator)

            0 -> [76543210] -> C
            N	Z	C	I	D	V
            0	+	+	-	-	-
            addressing	assembler	opc	bytes	cycles
            accumulator	LSR A	4A	1	2
            zeropage	LSR oper	46	2	5
            zeropage,X	LSR oper,X	56	2	6
            absolute	LSR oper	4E	3	6
            absolute,X	LSR oper,X	5E	3	7
        */
        [0x4A] = {"LSR", lsrImmediate, 1, IMPLIED},
        [0x46] = {"LSR", lsrZeroPage, 2, ZERO_PAGE},
        [0x56] = {"LSR", lsrZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0x4E] = {"LSR", lsrAbsolute, 3, ABSOLUTE},
        [0x5E] = {"LSR", lsrAbsoluteX, 3, INDEXED},

        /*
        NOP
            No Operation

            ---
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	NOP	EA	1	2
        */
        [0xEA] = {"NOP", nopImplied, 1, IMPLIED},

        /*
        ORA
            OR Memory with Accumulator

            A OR M -> A
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            immediate	ORA #oper	09	2	2
            zeropage	ORA oper	05	2	3
            zeropage,X	ORA oper,X	15	2	4
            absolute	ORA oper	0D	3	4
            absolute,X	ORA oper,X	1D	3	4*
            absolute,Y	ORA oper,Y	19	3	4*
            (indirect,X)	ORA (oper,X)	01	2	6
            (indirect),Y	ORA (oper),Y	11	2	5*
        */
        [0x09] = {"ORA", oraImmediate, 2, IMMEDIATE},
        [0x05] = {"ORA", oraZeroPage, 2, ZERO_PAGE},
        [0x15] = {"ORA", oraZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0x0D] = {"ORA", oraAbsolute, 3, ABSOLUTE},
        [0x1D] = {"ORA", oraAbsoluteX, 3, INDEXED},
        [0x19] = {"ORA", oraAbsoluteY, 3, INDEXED},
        [0x01] = {"ORA", oraIndirectX, 2, INDIRECT},
        [0x11] = {"ORA", oraIndirectY, 2, INDIRECT},

        /*
        PHA
            Push Accumulator on Stack

            push A
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	PHA	48	1	3
        */
        [0x48] = {"PHA", phaImplied, 1, IMPLIED},

        /*
        PHP
            Push Processor Status on Stack

            The status register will be pushed with the break
            flag and bit 5 set to 1.

            push SR
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	PHP	08	1	3
        */
        [0x08] = {"PHP", phpImplied, 1, IMPLIED},

        /*
        PLA
            Pull Accumulator from Stack

            pull A
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	PLA	68	1	4
        */
        [0x68] = {"PLA", plaImplied, 1, IMPLIED},

        /*
        PLP
            Pull Processor Status from Stack

            The status register will be pulled with the break
            flag and bit 5 ignored.

            pull SR
            N	Z	C	I	D	V
            from stack
            addressing	assembler	opc	bytes	cycles
            implied	PLP	28	1	4
        */
        [0x28] = {"PLP", plpImplied, 1, IMPLIED},

        /*
        ROL
            Rotate One Bit Left (Memory or Accumulator)

            C <- [76543210] <- C
            N	Z	C	I	D	V
            +	+	+	-	-	-
            addressing	assembler	opc	bytes	cycles
            accumulator	ROL A	2A	1	2
            zeropage	ROL oper	26	2	5
            zeropage,X	ROL oper,X	36	2	6
            absolute	ROL oper	2E	3	6
            absolute,X	ROL oper,X	3E	3	7
        */
        [0x2A] = {"ROL", rolImmediate, 1, IMPLIED},
        [0x26] = {"ROL", rolZeroPage, 2, ZERO_PAGE},
        [0x36] = {"ROL", rolZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0x2E] = {"ROL", rolAbsolute, 3, ABSOLUTE},
        [0x3E] = {"ROL", rolAbsoluteX, 3, INDEXED},

        /*
        ROR
            Rotate One Bit Right (Memory or Accumulator)

            C -> [76543210] -> C
            N	Z	C	I	D	V
            +	+	+	-	-	-
            addressing	assembler	opc	bytes	cycles
            accumulator	ROR A	6A	1	2
            zeropage	ROR oper	66	2	5
            zeropage,X	ROR oper,X	76	2	6
            absolute	ROR oper	6E	3	6
            absolute,X	ROR oper,X	7E	3	7
        */
        [0x6A] = {"ROR", rorImmediate, 1, IMPLIED},
        [0x66] = {"ROR", rorZeroPage, 2, ZERO_PAGE},
        [0x76] = {"ROR", rorZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0x6E] = {"ROR", rorAbsolute, 3, ABSOLUTE},
        [0x7E] = {"ROR", rorAbsoluteX, 3, INDEXED},

        /*
        RTI
            Return from Interrupt

            The status register is pulled with the break flag
            and bit 5 ignored. Then PC is pulled from the stack.

            pull SR, pull PC
            N	Z	C	I	D	V
            from stack
            addressing	assembler	opc	bytes	cycles
            implied	RTI	40	1	6
        */
        [0x40] = {"RTI", rtiImplied, 1, IMPLIED},

        /*
        RTS
            Return from Subroutine

            pull PC, PC+1 -> PC
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	RTS	60	1	6
        */
        [0x60] = {"RTS", rtsImplied, 1, IMPLIED},

        /*
        SBC
            Subtract Memory from Accumulator with Borrow

            A - M - C̅ -> A
            N	Z	C	I	D	V
            +	+	+	-	-	+
            addressing	assembler	opc	bytes	cycles
            immediate	SBC #oper	E9	2	2
            zeropage	SBC oper	E5	2	3
            zeropage,X	SBC oper,X	F5	2	4
            absolute	SBC oper	ED	3	4
            absolute,X	SBC oper,X	FD	3	4*
            absolute,Y	SBC oper,Y	F9	3	4*
            (indirect,X)	SBC (oper,X)	E1	2	6
            (indirect),Y	SBC (oper),Y	F1	2	5*
        */
        [0xE9] = {"SBC", sbcImmediate, 2, IMMEDIATE},
        [0xE5] = {"SBC", sbcZeroPage, 2, ZERO_PAGE},
        [0xF5] = {"SBC", sbcZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0xED] = {"SBC", sbcAbsolute, 3, ABSOLUTE},
        [0xFD] = {"SBC", sbcAbsoluteX, 3, INDEXED},
        [0xF9] = {"SBC", sbcAbsoluteY, 3, INDEXED},
        [0xE1] = {"SBC", sbcIndirectX, 2, INDIRECT},
        [0xF1] = {"SBC", sbcIndirectY, 2, INDIRECT},

        /*
        SEC
            Set Carry Flag

            1 -> C
            N	Z	C	I	D	V
            -	-	1	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	SEC	38	1	2
        */
        [0x38] = {"SEC", secImplied, 1, IMPLIED},

        /*
        SED
            Set Decimal Flag

            1 -> D
            N	Z	C	I	D	V
            -	-	-	-	1	-
            addressing	assembler	opc	bytes	cycles
            implied	SED	F8	1	2
            SEI
            Set Interrupt Disable Status

            1 -> I
            N	Z	C	I	D	V
            -	-	-	1	-	-
            addressing	assembler	opc	bytes	cycles
            implied	SEI	78	1	2
        */
        [0xF8] = {"SED", sedImplied, 1, IMPLIED},
        [0x78] = {"SEI", seiImplied, 1, IMPLIED},

        /*
        STA
            Store Accumulator in Memory

            A -> M
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            zeropage	STA oper	85	2	3
            zeropage,X	STA oper,X	95	2	4
            absolute	STA oper	8D	3	4
            absolute,X	STA oper,X	9D	3	5
            absolute,Y	STA oper,Y	99	3	5
            (indirect,X)	STA (oper,X)	81	2	6
            (indirect),Y	STA (oper),Y	91	2	6
        */
        [0x85] = {"STA", staZeroPage, 2, ZERO_PAGE},
        [0x95] = {"STA", staZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0x8D] = {"STA", staAbsolute, 3, ABSOLUTE},
        [0x9D] = {"STA", staAbsoluteX, 3, INDEXED},
        [0x99] = {"STA", staAbsoluteY, 3, INDEXED},
        [0x81] = {"STA", staIndirectX, 2, INDIRECT},
        [0x91] = {"STA", staIndirectY, 2, INDIRECT},

        /*
        STX
            Store Index X in Memory

            X -> M
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            zeropage	STX oper	86	2	3
            zeropage,Y	STX oper,Y	96	2	4
            absolute	STX oper	8E	3	4
        */
        [0x86] = {"STX", stxZeroPage, 2, ZERO_PAGE},
        [0x96] = {"STX", stxZeroPageY, 2, ZERO_PAGE_OFFSET},
        [0x8E] = {"STX", stxAbsolute, 3, ABSOLUTE},

        /*
        STY
            Sore Index Y in Memory

            Y -> M
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            zeropage	STY oper	84	2	3
            zeropage,X	STY oper,X	94	2	4
            absolute	STY oper	8C	3	4
        */
        [0x84] = {"STY", styZeroPage, 2, ZERO_PAGE},
        [0x94] = {"STY", styZeroPageX, 2, ZERO_PAGE_OFFSET},
        [0x8C] = {"STY", styAbsolute, 3, ABSOLUTE},

        /*
        TAX
            Transfer Accumulator to Index X

            A -> X
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	TAX	AA	1	2
        */
        [0xAA] = {"TAX", taxImplied, 1, IMPLIED},

        /*
        TAY
            Transfer Accumulator to Index Y

            A -> Y
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	TAY	A8	1	2
        */
        [0xA8] = {"TAY", tayImplied, 1, IMPLIED},

        /*
        TSX
            Transfer Stack Pointer to Index X

            SP -> X
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	TSX	BA	1	2
        */
        [0xBA] = {"TSX", tsxImplied, 1, IMPLIED},

        /*
        TXA
            Transfer Index X to Accumulator

            X -> A
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	TXA	8A	1	2
        */
        [0x8A] = {"TXA", txaImplied, 1, IMPLIED},

        /*
        TXS
            Transfer Index X to Stack Register

            X -> SP
            N	Z	C	I	D	V
            -	-	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	TXS	9A	1	2
        */
        [0x9A] = {"TXS", txsImplied, 1, IMPLIED},

        /*
        TYA
            Transfer Index Y to Accumulator

            Y -> A
            N	Z	C	I	D	V
            +	+	-	-	-	-
            addressing	assembler	opc	bytes	cycles
            implied	TYA	98	1	2
        */

        [0x98] = {"TYA", tyaImplied, 1, IMPLIED}
};


/*
 * Initialise a 6502, pass it:
 *  - a pointer to a Mos6502 object
 *  - a function pointer to a constructor
 *  - a void pointer to an argument list the constructor uses
 *  - a function pointer to a function
 */
int init(Mos6502* cpu, char* arguments){
    memset(&(cpu->AC), 0, 7);
    cpu->PC = 0x0200;
    cpu->SP = 0xFD;
    cpu->cycles_executed = 0;

    return run(cpu);
}

int run(void* _self){
    Mos6502* self = (Mos6502*) _self;
    while (1){
        // fetch instruction
        char opcode = self->memory[self->PC++];

        Instruction instr = instructionSet[opcode];
        if (instr.execute != NULL) {
            self->cycles_executed += instr.cycles;
            instr.execute();
        } else {
            crash(self, "Missing OPC");
        }

        break;
    }
    return 0;
}

int crash(Mos6502* cpu, char* message) {
    printf("CRITICAL - %s\n", message);
    print_state(cpu);
    exit(-1);
}

int print_state(Mos6502* cpu) {
    printf("  Crash state:\n\t");
    printf("AC: %d\tX: %d\tY: %d\n\t", cpu->AC, cpu->X, cpu->Y);
    printf("PC: %d\n\tSP: %d  Flags: %d\n\t", cpu->PC, cpu->SP, cpu->SR);
    printf("Cycles executed: %d\n", cpu->cycles_executed);
    return 0;
}

/*
int main() {
    Mos6502 cpu;
    char args[10];

    printf("%i", init(&cpu, args));

}*/