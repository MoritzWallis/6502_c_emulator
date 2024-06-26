#include "6502v2.h"
#include <stdexcept>

# define NEGATIVE(_a) (_a & 0x80)
# define ZERO(_a) (_a == 0)

// add with carry
byte Dodgy6502::ADC() {
    temp = a + fetched + read_flag(C);
    a = temp & 0x00FF;

    set_flag(FLAGS6502::N, NEGATIVE(a));
    set_flag(FLAGS6502::Z, ZERO(a));
    set_flag(FLAGS6502::C, temp > 255);
    set_flag(FLAGS6502::V, temp > 255);

    return 0;
}

// and (with accumulator)
byte Dodgy6502::AND() {
    a &= fetched;
    set_flag(FLAGS6502::N, NEGATIVE(a));
    set_flag(FLAGS6502::Z, ZERO(a));
    return 0;
}

// arithmetic shift left
byte Dodgy6502::ASL() {
    temp = fetched << 1;
    set_flag(FLAGS6502::C, temp & 0xFF00);
    set_flag(FLAGS6502::N, NEGATIVE(a));
    set_flag(FLAGS6502::Z, ZERO(a));

    if(current_instruction->addr_mode == &Dodgy6502::imp)
        a = temp;
    else
        memory[abs_addr] = temp;

    return 0;
}

// branch on carry clear
byte Dodgy6502::BCC() {
    if(!read_flag(FLAGS6502::C))
        pc += (signed char)fetched;
    return 0;
}

// branch on carry set
byte Dodgy6502::BCS() {
    if(read_flag(FLAGS6502::C))
        pc += (signed char)fetched;
    return 0;
}

// branch on equal (zero set)
byte Dodgy6502::BEQ() {
    if(read_flag(FLAGS6502::Z))
        pc += (signed char)fetched;
    return 0;
}

// bit test
byte Dodgy6502::BIT() {
    set_flag(FLAGS6502::V, fetched & (1 << 6));
    set_flag(FLAGS6502::N, fetched & (1 << 7));
    set_flag(FLAGS6502::Z, fetched & a);
    return 0;
}

// branch on minus (negative set)
byte Dodgy6502::BMI() {
    if(read_flag(FLAGS6502::N))
        pc += (signed char)fetched;
    return 0;
}

// branch on not equal (zero clear)
byte Dodgy6502::BNE() {
    if(!read_flag(FLAGS6502::Z))
        pc += (signed char)fetched;
    return 0;
}

// branch on plus (negative clear)
byte Dodgy6502::BPL() {
    if(!read_flag(FLAGS6502::N))
        pc += (signed char)fetched;
    return 0;
}

// break / interrupt
byte Dodgy6502::BRK() {
    throw std::runtime_error("BRK instruction not implemented");
}

// branch on overflow clear
byte Dodgy6502::BVC() {
    if(!read_flag(FLAGS6502::V))
        pc += (signed char)fetched;
    return 0;
}

// branch on overflow set
byte Dodgy6502::BVS() {
    if(read_flag(FLAGS6502::V))
        pc += (signed char)fetched;
    return 0;
}

// clear carry
byte Dodgy6502::CLC() {
    set_flag(FLAGS6502::C, false);
    return 0;
}

// clear decimal
byte Dodgy6502::CLD() {
    set_flag(FLAGS6502::D, false);
    return 0;
}

// clear interrupt disable
byte Dodgy6502::CLI() {
    set_flag(FLAGS6502::I, false);
    return 0;
}

// clear overflow
byte Dodgy6502::CLV() {
    set_flag(FLAGS6502::V, false);
    return 0;
}

// compare (with accumulator)
byte Dodgy6502::CMP() {  // TODO CRITICAL: UNSIGNED FLAG SETTING
    temp = a - fetched;
    set_flag(FLAGS6502::N, NEGATIVE(temp));
    set_flag(FLAGS6502::Z, ZERO(temp));
    set_flag(FLAGS6502::C, a >= fetched);
    return 0;
}

// compare with X
byte Dodgy6502::CPX() {
    temp = x - fetched;
    set_flag(FLAGS6502::N, NEGATIVE(temp));
    set_flag(FLAGS6502::Z, ZERO(temp));
    set_flag(FLAGS6502::C, x >= fetched);
    return 0;
}

// compare with Y
byte Dodgy6502::CPY() {
    temp = y - fetched;
    set_flag(FLAGS6502::N, NEGATIVE(temp));
    set_flag(FLAGS6502::Z, ZERO(temp));
    set_flag(FLAGS6502::C, y >= fetched);
    return 0;
}

// decrement
byte Dodgy6502::DEC() {
    memory[abs_addr] = --fetched;
    set_flag(FLAGS6502::N, NEGATIVE(fetched));
    set_flag(FLAGS6502::Z, ZERO(fetched));
    return 0;
}

// decrement X
byte Dodgy6502::DEX() {
    x--;
    set_flag(FLAGS6502::N, NEGATIVE(x));
    set_flag(FLAGS6502::Z, ZERO(x));
    return 0;
}

// decrement Y
byte Dodgy6502::DEY() {
    y--;
    set_flag(FLAGS6502::N, NEGATIVE(y));
    set_flag(FLAGS6502::Z, ZERO(y));
    return 0;
}

// exclusive or (with accumulator)
byte Dodgy6502::EOR() {
    a ^= fetched;
    set_flag(FLAGS6502::N, NEGATIVE(a));
    set_flag(FLAGS6502::Z, ZERO(a));
    return 0;
}

// increment
byte Dodgy6502::INC() {
    memory[abs_addr] = ++fetched;
    set_flag(FLAGS6502::N, NEGATIVE(fetched));
    set_flag(FLAGS6502::Z, ZERO(fetched));
    return 0;
}

// increment X
byte Dodgy6502::INX() {
    x++;
    set_flag(FLAGS6502::N, NEGATIVE(x));
    set_flag(FLAGS6502::Z, ZERO(x));
    return 0;
}

// increment Y
byte Dodgy6502::INY() {
    y++;
    set_flag(FLAGS6502::N, NEGATIVE(y));
    set_flag(FLAGS6502::Z, ZERO(y));
    return 0;
}

// jump
byte Dodgy6502::JMP() {
    pc = abs_addr;
    return 0;
}

// jump subroutine
byte Dodgy6502::JSR() {
    push(pc+2);
    pc = abs_addr;
    return 0;
}

// load accumulator
byte Dodgy6502::LDA() {
    a = fetched;
    set_flag(FLAGS6502::N, NEGATIVE(a));
    set_flag(FLAGS6502::Z, ZERO(a));
    return 0;
}

// load X
byte Dodgy6502::LDX() {
    x = fetched;
    set_flag(FLAGS6502::N, NEGATIVE(x));
    set_flag(FLAGS6502::Z, ZERO(x));
    return 0;
}

// load Y
byte Dodgy6502::LDY() {
    y = fetched;
    set_flag(FLAGS6502::N, NEGATIVE(y));
    set_flag(FLAGS6502::Z, ZERO(y));
    return 0;
}

// logical shift right
byte Dodgy6502::LSR() {
    set_flag(FLAGS6502::C, fetched & 0x1);
    fetched >>= 1;
    set_flag(FLAGS6502::N, false);
    set_flag(FLAGS6502::Z, ZERO(fetched));

    if(current_instruction->addr_mode == &Dodgy6502::imp)
        a = fetched;
    else
        memory[abs_addr] = fetched;

    return 0;
}

// no operation
byte Dodgy6502::NOP() {
    return 0;
}

// or with accumulator
byte Dodgy6502::ORA() {
    a |= fetched;
    set_flag(FLAGS6502::N, NEGATIVE(a));
    set_flag(FLAGS6502::Z, ZERO(a));
    return 0;
}

// push accumulator
byte Dodgy6502::PHA() {
    push(a);
    return 0;
}

// push processor status (SR)
byte Dodgy6502::PHP() {
    push(sb);
    return 0;
}

// pull accumulator
byte Dodgy6502::PLA() {
    a = pop();
    set_flag(FLAGS6502::N, NEGATIVE(a));
    set_flag(FLAGS6502::Z, ZERO(a));
    return 0;
}

// pull processor status (SR)
byte Dodgy6502::PLP() {
    sb = pop();
    return 0;
}

// rotate left
byte Dodgy6502::ROL() {
    temp = NEGATIVE(fetched);
    fetched = (fetched << 1) | read_flag(FLAGS6502::C);
    set_flag(FLAGS6502::C, temp);
    set_flag(FLAGS6502::N, NEGATIVE(fetched));
    set_flag(FLAGS6502::Z, ZERO(fetched));

    if(current_instruction->addr_mode == &Dodgy6502::imp)
        a = fetched;
    else
        memory[abs_addr] = fetched;

    return 0;
}

// rotate right
byte Dodgy6502::ROR() {
    temp = fetched & 0x1;
    fetched = (fetched >> 1) | (read_flag(FLAGS6502::C) << 7);
    set_flag(FLAGS6502::C, temp);
    set_flag(FLAGS6502::N, NEGATIVE(fetched));
    set_flag(FLAGS6502::Z, ZERO(fetched));

    if(current_instruction->addr_mode == &Dodgy6502::imp)
        a = fetched;
    else
        memory[abs_addr] = fetched;

    return 0;
}

// return from interrupt
byte Dodgy6502::RTI() {
    sb = pop();
    pc = pop();
    pc |= pop() << 8;
    return 0;
}

// return from subroutine
byte Dodgy6502::RTS() {
    pc = pop();
    pc |= pop() << 8;
    return 0;
}

// subtract with carry
byte Dodgy6502::SBC() {
    throw std::runtime_error("SBC instruction not implemented");
    return 0;
}

// set carry
byte Dodgy6502::SEC() {
    set_flag(FLAGS6502::C, true);
    return 0;
}

// set decimal
byte Dodgy6502::SED() {
    set_flag(FLAGS6502::D, true);
    return 0;
}

// set interrupt disable
byte Dodgy6502::SEI() {
    set_flag(FLAGS6502::I, true);
    return 0;
}

// store accumulator
byte Dodgy6502::STA() {
    memory[abs_addr] = a;
    return 0;
}

// store X
byte Dodgy6502::STX() {
    memory[abs_addr] = x;
    return 0;
}

// store Y
byte Dodgy6502::STY() {
    memory[abs_addr] = y;
    return 0;
}

// transfer accumulator to X
byte Dodgy6502::TAX() {
    x = a;
    set_flag(FLAGS6502::N, NEGATIVE(x));
    set_flag(FLAGS6502::Z, ZERO(x));
    return 0;
}

// transfer accumulator to Y
byte Dodgy6502::TAY() {
    y = a;
    set_flag(FLAGS6502::N, NEGATIVE(y));
    set_flag(FLAGS6502::Z, ZERO(y));
    return 0;
}

// transfer stack pointer to X
byte Dodgy6502::TSX() {
    x = sp;
    set_flag(FLAGS6502::N, NEGATIVE(x));
    set_flag(FLAGS6502::Z, ZERO(x));
    return 0;
}

// transfer X to accumulator
byte Dodgy6502::TXA() {
    a = x;
    set_flag(FLAGS6502::N, NEGATIVE(a));
    set_flag(FLAGS6502::Z, ZERO(a));
    return 0;
}

// transfer X to stack pointer
byte Dodgy6502::TXS() {
    sp = x;
    return 0;
}

// transfer Y to accumulator
byte Dodgy6502::TYA() {
    a = y;
    set_flag(FLAGS6502::N, NEGATIVE(a));
    set_flag(FLAGS6502::Z, ZERO(a));
    return 0;
}
