# include "6502v2.h"
#include <fstream>
#include <unordered_map>
//# include "inst_impl.h"

byte Dodgy6502::read(word address) const{
    return memory[address];
}

void Dodgy6502::write(word address, byte data){
    memory[address] = data;
}

void Dodgy6502::load_memory(byte* memory, word size=((1 << 16)-1), word offset=0) const{
    memcpy(this->memory + offset, memory, size);
}

void Dodgy6502::load_rom(const char *filename) {
    FILE *file = fopen(filename, "rb");

    if(!file){
        throw std::runtime_error("Failed to load ROM");
    }

    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(fileSize > 0xFFFF){
        throw std::runtime_error("ROM too large");
    }

    unsigned char* buffer = (unsigned char *)malloc(fileSize);
    if (!buffer) {
        fclose(file);
        throw std::runtime_error("ROM too large");
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        free(buffer);
        fclose(file);
        throw std::runtime_error("Failed reading ROM");
    }

    memcpy(memory, buffer, fileSize);
    free(buffer);
    fclose(file);
}

void Dodgy6502::add_instruction(
        byte opc,
        const std::string &name = "NOT NAMED",
        byte(Dodgy6502::*addr_mode)() = nullptr,
        byte(Dodgy6502::*implementation)() = nullptr,
        byte cycles = 0,
        const std::string &description = "NO DESCRIPTION"
){

    instructions[opc] = {
            name,
            addr_mode,
            implementation,
            cycles,
            std::to_string(opc) + ": " + description
    };
}

//// get opc from name
//byte Dodgy6502::get_opc(const std::string& name){
//    return opc_lookup_table[name];
//}

// generated with the python script
void Dodgy6502::add_all_instructions(){

    for(int i = 0; i < 256; i++){
        add_instruction(i, "ILLEGAL_NOP", &Dodgy6502::imp, &Dodgy6502::NOP, 0, "ILLEGAL NOP");
    }

    add_instruction(0, "BRK", &Dodgy6502::imp, &Dodgy6502::BRK, 0, "0x00 BRK: Force Break");
    
    add_instruction(1, "0RA", &Dodgy6502::zpx, &Dodgy6502::ORA, 0, "0x01 0RA-zpx: 'OR' Accum with Memory");
    
    add_instruction(5, "ORA", &Dodgy6502::zp, &Dodgy6502::ORA, 0, "0x05 ORA-zp: NO DESCRIPTION");
    
    add_instruction(6, "ASL", &Dodgy6502::zp, &Dodgy6502::ASL, 0, "0x06 ASL-zp: Shift Left One Bit. (Mem)");
    
    add_instruction(8, "PHI", &Dodgy6502::imp, &Dodgy6502::PHP, 0, "0x08 PHI: Push Processor Status on Stack");
    
    add_instruction(9, "ORA", &Dodgy6502::imm, &Dodgy6502::ORA, 0, "0x09 ORA-imm: NO DESCRIPTION");
    
    add_instruction(10, "ASL", &Dodgy6502::imp, &Dodgy6502::ASL, 0, "0xOA ASL-A: Shift Left One Bit (Accum)");
    
    add_instruction(13, "ORA", &Dodgy6502::zp, &Dodgy6502::ORA, 0, "0xOD ORA-abs: NO DESCRIPTION");
    
    add_instruction(14, "ASL", &Dodgy6502::zp, &Dodgy6502::ASL, 0, "0xOE ASL-abs: NO DESCRIPTION");
    
    add_instruction(16, "BPL", &Dodgy6502::imp, &Dodgy6502::BPL, 0, "0x10 BPL: Branch on Plus");
    
    add_instruction(17, "0RA", &Dodgy6502::zpy, &Dodgy6502::ORA, 0, "0x11 0RA-zpy: NO DESCRIPTION");
    
    add_instruction(21, "ORA", &Dodgy6502::zpx, &Dodgy6502::ORA, 0, "0x15 ORA-zpx: NO DESCRIPTION");
    
    add_instruction(22, "ASL", &Dodgy6502::zpx, &Dodgy6502::ASL, 0, "0x16 ASL-zpx: NO DESCRIPTION");
    
    add_instruction(24, "CLC", &Dodgy6502::imp, &Dodgy6502::CLC, 0, "0x18 CLC: Clear Carry Flag");
    
    add_instruction(25, "ORA", &Dodgy6502::aby, &Dodgy6502::ORA, 0, "0x19 ORA-aby: NO DESCRIPTION");
    
    add_instruction(29, "ORA", &Dodgy6502::abx, &Dodgy6502::ORA, 0, "0xID ORA-abx: NO DESCRIPTION");
    
    add_instruction(30, "ASL", &Dodgy6502::abx, &Dodgy6502::ASL, 0, "0xIE ASL-abx: NO DESCRIPTION");
    
    add_instruction(32, "JSR", &Dodgy6502::imp, &Dodgy6502::JSR, 0, "0x20 JSR: Jump to Subroutine");
    
    add_instruction(33, "AND", &Dodgy6502::zpx, &Dodgy6502::AND, 0, "0x21 AND-zpx: 'AND' Accumulator with Memory");
    
    add_instruction(36, "BIT", &Dodgy6502::zp, &Dodgy6502::BIT, 0, "0x24 BIT-zp: Test Bits in Accum with Memory");
    
    add_instruction(37, "AND", &Dodgy6502::zp, &Dodgy6502::AND, 0, "0x25 AND-zp: NO DESCRIPTION");
    
    add_instruction(38, "ROL", &Dodgy6502::zp, &Dodgy6502::ROL, 0, "0x26 ROL-zp: Rotate One Bit Left (Mem)");
    
    add_instruction(40, "PLP", &Dodgy6502::imp, &Dodgy6502::PLP, 0, "0x28 PLP: Pull Processor Status from Stack");
    
    add_instruction(41, "AND", &Dodgy6502::imm, &Dodgy6502::AND, 0, "0x29 AND-imm: NO DESCRIPTION");
    
    add_instruction(42, "ROL", &Dodgy6502::imp, &Dodgy6502::ROL, 0, "0x2A ROL-A: Rotate One Bit Left (Accum)");
    
    add_instruction(44, "BIT", &Dodgy6502::zp, &Dodgy6502::BIT, 0, "0x2C BIT-abs: NO DESCRIPTION");
    
    add_instruction(45, "AND", &Dodgy6502::zp, &Dodgy6502::AND, 0, "0x2D AND-abs: NO DESCRIPTION");
    
    add_instruction(46, "ROL", &Dodgy6502::zp, &Dodgy6502::ROL, 0, "0x2E ROL-abs: NO DESCRIPTION");
    
    add_instruction(48, "BMI", &Dodgy6502::imp, &Dodgy6502::BMI, 0, "0x30 BMI: Branch on Minus");
    
    add_instruction(49, "AND", &Dodgy6502::zpy, &Dodgy6502::AND, 0, "0x31 AND-zpy: NO DESCRIPTION");
    
    add_instruction(53, "AND", &Dodgy6502::zpx, &Dodgy6502::AND, 0, "0x35 AND-zpx: NO DESCRIPTION");
    
    add_instruction(54, "ROL", &Dodgy6502::zpx, &Dodgy6502::ROL, 0, "0x36 ROL-zpx: NO DESCRIPTION");
    
    add_instruction(56, "SEC", &Dodgy6502::imp, &Dodgy6502::SEC, 0, "0x38 SEC: Set Carry Flag");
    
    add_instruction(57, "AND", &Dodgy6502::aby, &Dodgy6502::AND, 0, "0x39 AND-aby: NO DESCRIPTION");
    
    add_instruction(61, "AND", &Dodgy6502::abx, &Dodgy6502::AND, 0, "0x31 AND-abx: NO DESCRIPTION");
    
    add_instruction(62, "ROL", &Dodgy6502::abx, &Dodgy6502::ROL, 0, "0x32 ROL-abx:              !");
    
    add_instruction(64, "RTI", &Dodgy6502::imp, &Dodgy6502::RTI, 0, "0x40 RTI: Return from Interrupt");
    
    add_instruction(65, "EOR", &Dodgy6502::zpx, &Dodgy6502::EOR, 0, "0x41 EOR-zpx: 'Exclusive Or' Accum with Memory");
    
    add_instruction(69, "EOR", &Dodgy6502::zp, &Dodgy6502::EOR, 0, "0x45 EOR-zp: NO DESCRIPTION");
    
    add_instruction(70, "LSR", &Dodgy6502::zp, &Dodgy6502::LSR, 0, "0x46 LSR-zp: Shift One Bit Right (Mem)");
    
    add_instruction(72, "PHA", &Dodgy6502::imp, &Dodgy6502::PHA, 0, "0x48 PHA: Push Accum onto Stack");
    
    add_instruction(73, "EOR", &Dodgy6502::imm, &Dodgy6502::EOR, 0, "0x49 EOR-IMM: NO DESCRIPTION");
    
    add_instruction(74, "LSR", &Dodgy6502::imp, &Dodgy6502::LSR, 0, "0x4A LSR-A: Shift One B it Right (Accum)");
    
    add_instruction(76, "JMP", &Dodgy6502::zp, &Dodgy6502::JMP, 0, "0x4C JMP-abs: Jump");
    
    add_instruction(77, "EOR", &Dodgy6502::zp, &Dodgy6502::EOR, 0, "0x4D EOR-abs: NO DESCRIPTION");
    
    add_instruction(78, "LSR", &Dodgy6502::zp, &Dodgy6502::LSR, 0, "0x4E LSR-abs: NO DESCRIPTION");
    
    add_instruction(80, "BVC", &Dodgy6502::imp, &Dodgy6502::BVC, 0, "0x50 BVC: Branch on Overflow Clear");
    
    add_instruction(81, "EOR", &Dodgy6502::zpy, &Dodgy6502::EOR, 0, "0x51 EOR-zpy: NO DESCRIPTION");
    
    add_instruction(85, "EOR", &Dodgy6502::zpx, &Dodgy6502::EOR, 0, "0x55 EOR-zpx: NO DESCRIPTION");
    
    add_instruction(86, "LSR", &Dodgy6502::zpx, &Dodgy6502::LSR, 0, "0x56 LSR-zpx: NO DESCRIPTION");
    
    add_instruction(88, "CLI", &Dodgy6502::imp, &Dodgy6502::CLI, 0, "0x58 CLI: Clear Interrupt Disable B it");
    
    add_instruction(89, "EOR", &Dodgy6502::aby, &Dodgy6502::EOR, 0, "0x59 EOR-aby: NO DESCRIPTION");
    
    add_instruction(93, "EOR", &Dodgy6502::abx, &Dodgy6502::EOR, 0, "0x5D EOR-abx: NO DESCRIPTION");
    
    add_instruction(94, "LSR", &Dodgy6502::zpx, &Dodgy6502::LSR, 0, "0x5E LSR-AES,X: NO DESCRIPTION");
    
    add_instruction(96, "RTS", &Dodgy6502::imp, &Dodgy6502::RTS, 0, "0x60 RTS: Return from Subroutine");
    
    add_instruction(97, "ADC", &Dodgy6502::zpx, &Dodgy6502::ADC, 0, "0x61 ADC-zpx: Add Memory to Accum with Carry");
    
    add_instruction(101, "ADC", &Dodgy6502::zp, &Dodgy6502::ADC, 0, "0x65 ADC-zp: NO DESCRIPTION");
    
    add_instruction(104, "PLA", &Dodgy6502::imp, &Dodgy6502::PLA, 0, "0x68 PLA: Pull Accumulator from Stack");
    
    add_instruction(105, "ADC", &Dodgy6502::imm, &Dodgy6502::ADC, 0, "0x69 ADC-imm: NO DESCRIPTION");
    
    add_instruction(108, "JMP", &Dodgy6502::ind, &Dodgy6502::JMP, 0, "0x6C JMP-ind: NO DESCRIPTION");
    
    add_instruction(109, "ADC", &Dodgy6502::zp, &Dodgy6502::ADC, 0, "0x6D ADC-abs: NO DESCRIPTION");
    
    add_instruction(112, "BVS", &Dodgy6502::imp, &Dodgy6502::BVS, 0, "0x70 BVS: Branch on Overflow Set");
    
    add_instruction(113, "ADC", &Dodgy6502::zpy, &Dodgy6502::ADC, 0, "0x71 ADC-zpy: NO DESCRIPTION");
    
    add_instruction(117, "ADC", &Dodgy6502::zpx, &Dodgy6502::ADC, 0, "0x75 ADC-zpx: NO DESCRIPTION");
    
    add_instruction(120, "SEI", &Dodgy6502::imp, &Dodgy6502::SEI, 0, "0x78 SEI: Set Interrupt Disable Status");
    
    add_instruction(121, "ADC", &Dodgy6502::zpy, &Dodgy6502::ADC, 0, "0x79 ADC-zpy: NO DESCRIPTION");
    
    add_instruction(125, "ADC", &Dodgy6502::imp, &Dodgy6502::ADC, 0, "0x7D : ADC-abx");
    
    add_instruction(129, "STA", &Dodgy6502::zpx, &Dodgy6502::STA, 0, "0x81 STA-zpx: Store Accum in Memory");
    
    add_instruction(132, "STY", &Dodgy6502::zp, &Dodgy6502::STY, 0, "0x84 STY-zp: NO DESCRIPTION");
    
    add_instruction(133, "STA", &Dodgy6502::zp, &Dodgy6502::STA, 0, "0x85 STA-zp: NO DESCRIPTION");
    
    add_instruction(134, "STX", &Dodgy6502::zp, &Dodgy6502::STX, 0, "0x86 STX-zp: NO DESCRIPTION");
    
    add_instruction(136, "DEY", &Dodgy6502::imp, &Dodgy6502::DEY, 0, "0x88 DEY: NO DESCRIPTION");
    
    add_instruction(138, "TXA", &Dodgy6502::imp, &Dodgy6502::TXA, 0, "0x8A TXA: NO DESCRIPTION");
    
    add_instruction(140, "STY", &Dodgy6502::zp, &Dodgy6502::STY, 0, "0x8C STY-abs: NO DESCRIPTION");
    
    add_instruction(141, "STA", &Dodgy6502::zp, &Dodgy6502::STA, 0, "0x8D STA-abs: NO DESCRIPTION");
    
    add_instruction(142, "STX", &Dodgy6502::zp, &Dodgy6502::STX, 0, "0x8E STX-abs: NO DESCRIPTION");
    
    add_instruction(144, "BCC", &Dodgy6502::imp, &Dodgy6502::BCC, 0, "0x90 BCC: NO DESCRIPTION");
    
    add_instruction(145, "STA", &Dodgy6502::zpy, &Dodgy6502::STA, 0, "0x91 STA-zpy: NO DESCRIPTION");
    
    add_instruction(148, "STY", &Dodgy6502::zpx, &Dodgy6502::STY, 0, "0x94 STY-zpx: NO DESCRIPTION");
    
    add_instruction(149, "STA", &Dodgy6502::zpx, &Dodgy6502::STA, 0, "0x95 STA-zpx: NO DESCRIPTION");
    
    add_instruction(150, "STX", &Dodgy6502::zpy, &Dodgy6502::STX, 0, "0x96 STX-zpy: NO DESCRIPTION");
    
    add_instruction(152, "TYA", &Dodgy6502::imp, &Dodgy6502::TYA, 0, "0x98 TYA: NO DESCRIPTION");
    
    add_instruction(153, "STA", &Dodgy6502::aby, &Dodgy6502::STA, 0, "0x99 STA-aby: NO DESCRIPTION");
    
    add_instruction(154, "TXS", &Dodgy6502::imp, &Dodgy6502::TXS, 0, "0x9A TXS: NO DESCRIPTION");
    
    add_instruction(157, "STA", &Dodgy6502::abx, &Dodgy6502::STA, 0, "0x9D STA-abx: NO DESCRIPTION");
    
    add_instruction(160, "LDY", &Dodgy6502::imm, &Dodgy6502::LDY, 0, "0xA0 LDY-imm: NO DESCRIPTION");
    
    add_instruction(161, "LDA", &Dodgy6502::zpx, &Dodgy6502::LDA, 0, "0xA1 LDA-zpx: NO DESCRIPTION");
    
    add_instruction(162, "LDX", &Dodgy6502::imm, &Dodgy6502::LDX, 0, "0xA2 LDX-imm: NO DESCRIPTION");
    
    add_instruction(164, "LDY", &Dodgy6502::zp, &Dodgy6502::LDY, 0, "0xA4 LDY-zp: NO DESCRIPTION");
    
    add_instruction(165, "LDA", &Dodgy6502::zp, &Dodgy6502::LDA, 0, "0xA5 LDA-zp: NO DESCRIPTION");
    
    add_instruction(166, "LDX", &Dodgy6502::zp, &Dodgy6502::LDX, 0, "0xA6 LDX-zp: NO DESCRIPTION");
    
    add_instruction(168, "TAY", &Dodgy6502::imp, &Dodgy6502::TAY, 0, "0xA8 TAY: NO DESCRIPTION");
    
    add_instruction(169, "LDA", &Dodgy6502::imm, &Dodgy6502::LDA, 0, "0xA9 LDA-imm: NO DESCRIPTION");
    
    add_instruction(170, "TAX", &Dodgy6502::imp, &Dodgy6502::TAX, 0, "0xAA TAX: NO DESCRIPTION");
    
    add_instruction(172, "LDY", &Dodgy6502::zp, &Dodgy6502::LDY, 0, "0xAC LDY-abs: NO DESCRIPTION");
    
    add_instruction(173, "LDA", &Dodgy6502::zp, &Dodgy6502::LDA, 0, "0xAD LDA-abs: NO DESCRIPTION");
    
    add_instruction(174, "LDX", &Dodgy6502::zpx, &Dodgy6502::LDX, 0, "0xAE LDX-zpx: NO DESCRIPTION");
    
    add_instruction(176, "BCS", &Dodgy6502::imp, &Dodgy6502::BCS, 0, "0xB0 BCS: NO DESCRIPTION");
    
    add_instruction(171, "LDA", &Dodgy6502::zpy, &Dodgy6502::LDA, 0, "0xB1 LDA-zpy: NO DESCRIPTION");
    
    add_instruction(180, "LDY", &Dodgy6502::zpx, &Dodgy6502::LDY, 0, "0xB4 LDY-zpx: NO DESCRIPTION");
    
    add_instruction(181, "LDA", &Dodgy6502::zpx, &Dodgy6502::LDA, 0, "0xB5 LDA-zpx: NO DESCRIPTION");
    
    add_instruction(182, "LDX", &Dodgy6502::zpy, &Dodgy6502::LDX, 0, "0xB6 LDX-zpy: NO DESCRIPTION");
    
    add_instruction(184, "CLV", &Dodgy6502::imp, &Dodgy6502::CLV, 0, "0xB8 CLV: NO DESCRIPTION");
    
    add_instruction(185, "LDA", &Dodgy6502::aby, &Dodgy6502::LDA, 0, "0xB9 LDA-aby: NO DESCRIPTION");
    
    add_instruction(186, "TSX", &Dodgy6502::imp, &Dodgy6502::TSX, 0, "0xBA TSX: NO DESCRIPTION");
    
    add_instruction(188, "LDY", &Dodgy6502::abx, &Dodgy6502::LDY, 0, "0xDC LDY-abx: NO DESCRIPTION");
    
    add_instruction(189, "LDA", &Dodgy6502::abx, &Dodgy6502::LDA, 0, "0xBD LDA-abx: NO DESCRIPTION");
    
    add_instruction(190, "LDX", &Dodgy6502::aby, &Dodgy6502::LDX, 0, "0xBE LDX-aby: NO DESCRIPTION");
    
    add_instruction(192, "CPY", &Dodgy6502::imm, &Dodgy6502::CPY, 0, "0xCO CPY-imm: NO DESCRIPTION");
    
    add_instruction(193, "CMP", &Dodgy6502::zpx, &Dodgy6502::CMP, 0, "0xCl CMP-zpx: NO DESCRIPTION");
    
    add_instruction(196, "CPY", &Dodgy6502::zp, &Dodgy6502::CPY, 0, "0xC4 CPY-zp: NO DESCRIPTION");
    
    add_instruction(197, "CMP", &Dodgy6502::zp, &Dodgy6502::CMP, 0, "0xC5 CMP-zp: NO DESCRIPTION");
    
    add_instruction(198, "DEC", &Dodgy6502::zp, &Dodgy6502::DEC, 0, "0xC6 DEC-zp: NO DESCRIPTION");
    
    add_instruction(200, "INY", &Dodgy6502::imp, &Dodgy6502::INY, 0, "0xC8 INY: NO DESCRIPTION");
    
    add_instruction(201, "CMP", &Dodgy6502::imm, &Dodgy6502::CMP, 0, "0xC9 CMP-imm: NO DESCRIPTION");
    
    add_instruction(202, "DEX", &Dodgy6502::imp, &Dodgy6502::DEX, 0, "0xCA DEX: NO DESCRIPTION");
    
    add_instruction(204, "CPY", &Dodgy6502::zp, &Dodgy6502::CPY, 0, "0xCC CPY-abs: NO DESCRIPTION");
    
    add_instruction(205, "CMP", &Dodgy6502::zp, &Dodgy6502::CMP, 0, "0xCD CMP-abs: NO DESCRIPTION");
    
    add_instruction(206, "DEC", &Dodgy6502::zp, &Dodgy6502::DEC, 0, "0xCE DEC-abs: NO DESCRIPTION");
    
    add_instruction(208, "BNE", &Dodgy6502::imp, &Dodgy6502::BNE, 0, "0xDO BNE: NO DESCRIPTION");
    
    add_instruction(209, "CMP", &Dodgy6502::zpy, &Dodgy6502::CMP, 0, "0xD1 CMP-zpy: NO DESCRIPTION");
    
    add_instruction(213, "CMP", &Dodgy6502::zpx, &Dodgy6502::CMP, 0, "0xD5 CMP-zpx: NO DESCRIPTION");
    
    add_instruction(214, "DEC", &Dodgy6502::zpx, &Dodgy6502::DEC, 0, "0xD6 DEC-zpx: NO DESCRIPTION");
    
    add_instruction(215, "CLD", &Dodgy6502::imp, &Dodgy6502::CLD, 0, "0xD8 CLD: NO DESCRIPTION");
    
    add_instruction(217, "CMP", &Dodgy6502::aby, &Dodgy6502::CMP, 0, "0xD9 CMP-aby: NO DESCRIPTION");
    
    add_instruction(221, "CMP", &Dodgy6502::abx, &Dodgy6502::CMP, 0, "0xDD CMP-abx: NO DESCRIPTION");
    
    add_instruction(222, "DEC", &Dodgy6502::abx, &Dodgy6502::DEC, 0, "0xDE DEC-abx: NO DESCRIPTION");
    
    add_instruction(224, "CPX", &Dodgy6502::imm, &Dodgy6502::CPX, 0, "0xEO CPX-imm: NO DESCRIPTION");
    
    add_instruction(225, "SBC", &Dodgy6502::zpx, &Dodgy6502::SBC, 0, "0xEl SBC-zpx: NO DESCRIPTION");
    
    add_instruction(228, "CPX", &Dodgy6502::zp, &Dodgy6502::CPX, 0, "0xE4 CPX-zp: NO DESCRIPTION");
    
    add_instruction(229, "SBC", &Dodgy6502::zp, &Dodgy6502::SBC, 0, "0xE5 SBC-zp: NO DESCRIPTION");
    
    add_instruction(230, "INC", &Dodgy6502::zp, &Dodgy6502::INC, 0, "0xE6 INC-zp: NO DESCRIPTION");
    
    add_instruction(232, "INX", &Dodgy6502::imp, &Dodgy6502::INX, 0, "0xE8 INX: NO DESCRIPTION");
    
    add_instruction(233, "SBC", &Dodgy6502::imm, &Dodgy6502::SBC, 0, "0xE9 SBC-imm: NO DESCRIPTION");
    
    add_instruction(234, "NOP", &Dodgy6502::imp, &Dodgy6502::NOP, 0, "0xEA NOP: NO DESCRIPTION");
    
    add_instruction(236, "CPX", &Dodgy6502::zp, &Dodgy6502::CPX, 0, "0xEC CPX-abs: NO DESCRIPTION");
    
    add_instruction(237, "SBC", &Dodgy6502::zp, &Dodgy6502::SBC, 0, "0xED SBC-abs: NO DESCRIPTION");
    
    add_instruction(238, "INC", &Dodgy6502::zp, &Dodgy6502::INC, 0, "0xEE INC-abs: NO DESCRIPTION");
    
    add_instruction(240, "BEQ", &Dodgy6502::imp, &Dodgy6502::BEQ, 0, "0xFO BEQ: Branch If Equal (Result - 0)");
    
    add_instruction(241, "SBC", &Dodgy6502::zpy, &Dodgy6502::SBC, 0, "0xF1 SBC-zpy: NO DESCRIPTION");
    
    add_instruction(245, "SBC", &Dodgy6502::zpx, &Dodgy6502::SBC, 0, "0xF5 SBC-zpx: NO DESCRIPTION");
    
    add_instruction(246, "INC", &Dodgy6502::zpx, &Dodgy6502::INC, 0, "0xF6 INC-zpx: NO DESCRIPTION");
    
    add_instruction(248, "SED", &Dodgy6502::imp, &Dodgy6502::SED, 0, "0xF8 SED: Set Decimal Mode");
    
    add_instruction(249, "SBC", &Dodgy6502::aby, &Dodgy6502::SBC, 0, "0xF9 SBC-aby: NO DESCRIPTION");
    
    add_instruction(253, "SBC", &Dodgy6502::abx, &Dodgy6502::SBC, 0, "0xFD SBC-abx: NO DESCRIPTION");
    
    add_instruction(254, "INC", &Dodgy6502::abx, &Dodgy6502::INC, 0, "0xFE INC-abx: NO DESCRIPTION");
};


//for (int i = 0; i < 256; i++){
//    Dodgy6502::opc_lookup[Dodgy6502::instructions[i].name] = i;
//}