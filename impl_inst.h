#pragma once

#include "6502v2.h"

#ifndef INC_6502_IMPL_INST_H
#define INC_6502_IMPL_INST_H


class DDodgy6502 : public Dodgy6502 {
    //add with carry
    char ADC();

    //and (with accumulator)
    char AND();

    //arithmetic shift left
    char ASL();

    //branch on carry clear
    char BCC();

    //branch on carry set
    char BCS();

    //branch on equal (zero set)
    char BEQ();

    //bit test
    char BIT();

    //branch on minus (negative set)
    char BMI();

    //branch on not equal (zero clear)
    char BNE();

    //branch on plus (negative clear)
    char BPL();

    //break / interrupt
    char BRK();

    //branch on overflow clear
    char BVC();

    //branch on overflow set
    char BVS();

    //clear carry
    char CLC();

    //clear decimal
    char CLD();

    //clear interrupt disable
    char CLI();

    //clear overflow
    char CLV();

    //compare (with accumulator)
    char CMP();

    //compare with X
    char CPX();

    //compare with Y
    char CPY();

    //decrement
    char DEC();

    //decrement X
    char DEX();

    //decrement Y
    char DEY();

    //exclusive or (with accumulator)
    char EOR();

    //increment
    char INC();

    //increment X
    char INX();

    //increment Y
    char INY();

    //jump
    char JMP();

    //jump subroutine
    char JSR();

    //load accumulator
    char LDA();

    //load X
    char LDX();

    //load Y
    char LDY();

    //logical shift right
    char LSR();

    //no operation
    char NOP();

    //or with accumulator
    char ORA();

    //push accumulator
    char PHA();

    //push processor status (SR)
    char PHP();

    //pull accumulator
    char PLA();

    //pull processor status (SR)
    char PLP();

    //rotate left
    char ROL();

    //rotate right
    char ROR();

    //return from interrupt
    char RTI();

    //return from subroutine
    char RTS();

    //subtract with carry
    char SBC();

    //set carry
    char SEC();

    //set decimal
    char SED();

    //set interrupt disable
    char SEI();

    //store accumulator
    char STA();

    //store X
    char STX();

    //store Y
    char STY();

    //transfer accumulator to X
    char TAX();

    //transfer accumulator to Y
    char TAY();

    //transfer stack pointer to X
    char TSX();

    //transfer X to accumulator
    char TXA();

    //transfer X to stack pointer
    char TXS();

    //transfer Y to accumulator
    char TYA();
};

#endif //INC_6502_IMPL_INST_H
