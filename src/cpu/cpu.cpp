#include "cpu.h"

#include <src/memory/Bus.h>

int CPU::BPL_rel()
{
    int8_t imm = Bus::read8(pc++);

    printf("[6502]: bpl %04x\n", pc + imm);

    int cycles = 2;

    if (!GetFlag(N_FLAG))
    {
        cycles++;
        uint16_t old_pc = pc;
        pc += imm;
        if ((old_pc & 0xff00) != (pc & 0xff00))
        {
            cycles++;
        }
    }
    
    return cycles;
}

int CPU::CLC_imp()
{
    SetFlag(C_FLAG, false);
    printf("[6502]: clc\n");
    return 2;
}

int CPU::JSR_abs()
{
    uint16_t addr = Bus::read16(pc);
    pc += 2;
    Bus::write8(0x0100 + s, (pc >> 8) & 0x00FF);
    s--;
    Bus::write8(0x0100 + s, pc & 0x00ff);
    s--;

    pc = addr;

    printf("[6502]: jsr $%04x\n", pc);
    return 6;
}

int CPU::AND_zp0()
{
    uint8_t imm = Bus::read8(pc++);
    uint8_t val = Bus::read8(imm);
    uint8_t result = a & val;
    a = result;
    SetFlag(Z_FLAG, result == 0);
    SetFlag(N_FLAG, (result >> 7) & 1);
    printf("[6502]: and $%02x\n", imm);
    return 0;
}

int CPU::AND_imm()
{
    uint8_t imm = Bus::read8(pc++);
    uint8_t result = a & imm;
    SetFlag(Z_FLAG, result == 0);
    SetFlag(N_FLAG, (result >> 7) & 1);
    a = result;
    printf("[6502]: and #$%02x\n", a);
    return 2;
}

int CPU::EOR_zp0()
{
    uint8_t imm = Bus::read8(pc++);
    uint8_t val = Bus::read8(imm);
    a = a ^ val;
    SetFlag(N_FLAG, (a >> 7) & 1);
    SetFlag(Z_FLAG, a == 0);
    printf("[6502]: eor $%02x\n", imm);
    return 3;
}

int CPU::PHA_imp()
{
    Bus::write8(0x0100 + s, a);
    s--;
    printf("[6502]: pha\n");
    return 3;
}

int CPU::EOR_imm()
{
    uint8_t imm = Bus::read8(pc++);
    a = a ^ imm;
    SetFlag(N_FLAG, (a >> 7) & 1);
    SetFlag(Z_FLAG, a == 0);
    printf("[6502]: eor #$%02x\n", imm);
    return 2;
}

int CPU::LSR_imp()
{
    SetFlag(C_FLAG, a & 1);
    a >>= 1;
    SetFlag(N_FLAG, 0);
    SetFlag(Z_FLAG, a == 0);
    printf("[6502]: lsr a\n");
    return 2;
}

int CPU::JMP_abs()
{
    uint16_t new_pc = Bus::read16(pc);
    pc += 2;
    pc = new_pc;
    printf("[6502]: jmp $%04x\n", new_pc);
    return 3;
}

int CPU::RTS_imp()
{
    s++;
    pc = Bus::read8(0x100 + s);
    s++;
    pc |= Bus::read8(0x100 + s) << 8;
    printf("[6502]: rts\n");
    return 6;
}

int CPU::ADC_zp0()
{
    uint8_t imm = Bus::read8(pc++);
    uint8_t val = Bus::read8(imm);
    uint8_t result = a + val + GetFlag(C_FLAG);
    SetFlag(C_FLAG, result < a);
    SetFlag(Z_FLAG, (result == 0));
    SetFlag(N_FLAG, (result >> 7) & 1);
    SetFlag(V_FLAG, ((result >> 7) & 1) != ((a >> 7) & 1) && ((a >> 7) & 1) == ((val >> 7) & 1));
    a = result;
    printf("[6502]: adc $%02x\n", imm);
    return 3;
}

int CPU::ROR_zp0()
{
    uint8_t imm = Bus::read8(pc++);
    uint8_t val = Bus::read8(imm);
    bool old_c = GetFlag(C_FLAG);
    SetFlag(C_FLAG, val & 1);
    val = (val >> 1) | (old_c << 7);
    SetFlag(Z_FLAG, val == 0);
    SetFlag(N_FLAG, (val >> 7) & 1);
    Bus::write8(imm, val);
    printf("[6502]: ror $%02x\n", imm);
    return 5;
}

int CPU::PLA_imp()
{
    s++;
    a = Bus::read8(0x100 + s);
    printf("[6502]: pla\n");
    return 4;
}

int CPU::ADC_imm()
{
    uint8_t val = Bus::read8(pc++);
    uint8_t result = a + val + GetFlag(C_FLAG);
    SetFlag(C_FLAG, result < a);
    SetFlag(Z_FLAG, (result == 0));
    SetFlag(N_FLAG, (result >> 7) & 1);
    SetFlag(V_FLAG, ((result >> 7) & 1) != ((a >> 7) & 1) && ((a >> 7) & 1) == ((val >> 7) & 1));
    a = result;
    printf("[6502]: adc #$%02x\n", val);
    return 3;
}

int CPU::SEI_imp()
{
    SetFlag(I_FLAG, 1);
    printf("[6502]: sei\n");
    return 2;
}

int CPU::STY_ZP0()
{
    uint8_t off = Bus::read8(pc++);
    printf("[6502]: sty $%02x\n", off);
    Bus::write8(off, y);
    return 3;
}

int CPU::STA_ZP0()
{
    uint8_t off = Bus::read8(pc++);
    printf("[6502]: sta $%02x\n", off);
    Bus::write8(off, a);
    return 3;
}

int CPU::DEY_imp()
{
    y -= 1;
    SetFlag(Z_FLAG, y == 0);
    SetFlag(N_FLAG, (y >> 7) & 1);
    printf("[6502]: dey\n");
    return 2;
}

int CPU::TXA_imp()
{
    a = x;
    printf("[6502]: txa\n");
    return 2;
}

int CPU::STA_abs()
{
    uint16_t addr = Bus::read16(pc);
    pc += 2;
    printf("[6502]: sta $%04x\n", addr);
    Bus::write8(addr, a);
    return 4;
}

int CPU::STA_idy()
{
    uint8_t imm = Bus::read8(pc++);
    uint16_t addr = Bus::read16(imm);
    addr += y;
    Bus::write8(addr, a);
    printf("[6502]: sta ($%02x), y\n", imm);
    return 6;
}

int CPU::TYA_imp()
{
    a = y;
    printf("[6502]: tya\n");
    return 2;
}

int CPU::TXS_imp()
{
    s = x;
    printf("[6502]: txs\n");
    return 2;
}

int CPU::LDY_imm()
{
    uint8_t imm = Bus::read8(pc++);
    y = imm;
    printf("[6502]: ldy #$%02x\n", y);
    SetFlag(Z_FLAG, (y == 0));
    SetFlag(N_FLAG, (y >> 7) & 1);
    return 2;   
}

int CPU::LDX_imm()
{
    uint8_t imm = Bus::read8(pc++);
    x = imm;
    printf("[6502]: ldx #$%02x\n", x);
    SetFlag(Z_FLAG, (x == 0));
    SetFlag(N_FLAG, (x >> 7) & 1);
    return 2;   
}

int CPU::LDA_zp0()
{
    uint8_t imm = Bus::read8(pc++);
    a = Bus::read8(imm);
    printf("[6502]: lda $%02x\n", imm);
    return 3;
}

int CPU::LDX_zp0()
{
    uint8_t imm = Bus::read8(pc++);
    x = Bus::read8(imm);
    printf("[6502]: ldx $%02x\n", imm);
    return 3;
}

int CPU::TAY_imp()
{
    y = a;
    printf("[6502]: tay\n");
    return 2;
}

int CPU::LDA_imm()
{
    uint8_t imm = Bus::read8(pc++);
    a = imm;
    printf("[6502]: lda #$%02x\n", a);
    SetFlag(Z_FLAG, (a == 0));
    SetFlag(N_FLAG, (a >> 7) & 1);
    return 2;
}

int CPU::TAX_imp()
{
    x = a;
    printf("[6502]: tax\n");
    return 2;
}

int CPU::LDA_abs()
{
    uint16_t addr = Bus::read16(pc);
    pc += 2;
    printf("[6502]: lda $%04x\n", addr);
    a = Bus::read8(addr);
    SetFlag(Z_FLAG, (a == 0));
    SetFlag(N_FLAG, (a >> 7) & 1);
    return 4;
}

int CPU::LDA_iny()
{
    uint8_t imm = Bus::read8(pc++);
    uint16_t addr = Bus::read16(imm);
    addr += y;
    a = Bus::read8(addr);
    printf("[6502]: lda ($%02x), y\n", imm);
    return 6;
}

int CPU::DEC_zp0()
{
    uint8_t imm = Bus::read8(pc++);
    printf("[6502]: dec $%02x\n", imm);
    uint8_t data = Bus::read8(imm);
    data -= 1;
    Bus::write8(imm, data);
    SetFlag(N_FLAG, (data >> 7) & 1);
    SetFlag(Z_FLAG, data == 0);
    return 5;
}

int CPU::INY_imp()
{
    y++;
    SetFlag(N_FLAG, (y >> 7) & 1);
    SetFlag(Z_FLAG, y == 0);
    printf("[6502]: iny\n");
    return 2;
}

int CPU::CMP_imm()
{
    uint8_t imm = Bus::read8(pc++);
    uint8_t result = a - imm;
    SetFlag(Z_FLAG, result == 0);
    SetFlag(N_FLAG, (result >> 7) & 1);
    SetFlag(C_FLAG, imm > a);
    printf("[6502]: cmp #$%02x\n", imm);
    return 2;
}

int CPU::DEX_imp()
{
    x--;
    SetFlag(N_FLAG, (x >> 7) & 1);
    SetFlag(Z_FLAG, x == 0);
    printf("[6502]: dex\n");
    return 2;
}

int CPU::BNE_rel()
{
    int8_t imm = Bus::read8(pc++);

    printf("[6502]: bne %04x\n", pc + imm);

    int cycles = 2;

    if (!GetFlag(Z_FLAG))
    {
        cycles++;
        uint16_t old_pc = pc;
        pc += imm;
        if ((old_pc & 0xff00) != (pc & 0xff00))
        {
            cycles++;
        }
    }
    
    return cycles;
}

int CPU::CLD_imp()
{
    SetFlag(D_FLAG, 0);
    printf("[6502]: cld\n");
    return 2;
}

int CPU::BEQ_rel()
{
    int8_t imm = Bus::read8(pc++);

    printf("[6502]: beq %04x\n", pc + imm);

    int cycles = 2;

    if (GetFlag(Z_FLAG))
    {
        cycles++;
        uint16_t old_pc = pc;
        pc += imm;
        if ((old_pc & 0xff00) != (pc & 0xff00))
        {
            cycles++;
        }
    }
    
    return cycles;
}

CPU::CPU()
{
    uint16_t reset_addr = Bus::read8(0xFFFC);
    reset_addr |= (Bus::read8(0xFFFD) << 8);

    pc = reset_addr;

    opcodes[0x10] = std::bind(&CPU::BPL_rel, this);
    opcodes[0x18] = std::bind(&CPU::CLC_imp, this);
    opcodes[0x20] = std::bind(&CPU::JSR_abs, this);
    opcodes[0x25] = std::bind(&CPU::AND_zp0, this);
    opcodes[0x29] = std::bind(&CPU::AND_imm, this);
    opcodes[0x45] = std::bind(&CPU::EOR_zp0, this);
    opcodes[0x48] = std::bind(&CPU::PHA_imp, this);
    opcodes[0x49] = std::bind(&CPU::EOR_imm, this);
    opcodes[0x4A] = std::bind(&CPU::LSR_imp, this);
    opcodes[0x4C] = std::bind(&CPU::JMP_abs, this);
    opcodes[0x60] = std::bind(&CPU::RTS_imp, this);
    opcodes[0x65] = std::bind(&CPU::ADC_zp0, this);
    opcodes[0x66] = std::bind(&CPU::ROR_zp0, this);
    opcodes[0x68] = std::bind(&CPU::PLA_imp, this);
    opcodes[0x69] = std::bind(&CPU::ADC_imm, this);
    opcodes[0x78] = std::bind(&CPU::SEI_imp, this);
    opcodes[0x84] = std::bind(&CPU::STY_ZP0, this);
    opcodes[0x85] = std::bind(&CPU::STA_ZP0, this);
    opcodes[0x88] = std::bind(&CPU::DEY_imp, this);
    opcodes[0x8A] = std::bind(&CPU::TXA_imp, this);
    opcodes[0x8D] = std::bind(&CPU::STA_abs, this);
    opcodes[0x91] = std::bind(&CPU::STA_idy, this);
    opcodes[0x98] = std::bind(&CPU::TYA_imp, this);
    opcodes[0x9A] = std::bind(&CPU::TXS_imp, this);
    opcodes[0xA0] = std::bind(&CPU::LDY_imm, this);
    opcodes[0xA2] = std::bind(&CPU::LDX_imm, this);
    opcodes[0xA5] = std::bind(&CPU::LDA_zp0, this);
    opcodes[0xA6] = std::bind(&CPU::LDX_zp0, this);
    opcodes[0xA8] = std::bind(&CPU::TAY_imp, this);
    opcodes[0xA9] = std::bind(&CPU::LDA_imm, this);
    opcodes[0xAA] = std::bind(&CPU::TAX_imp, this);
    opcodes[0xAD] = std::bind(&CPU::LDA_abs, this);
    opcodes[0xB1] = std::bind(&CPU::LDA_iny, this);
    opcodes[0xC6] = std::bind(&CPU::DEC_zp0, this);
    opcodes[0xC8] = std::bind(&CPU::INY_imp, this);
    opcodes[0xC9] = std::bind(&CPU::CMP_imm, this);
    opcodes[0xCA] = std::bind(&CPU::DEX_imp, this);
    opcodes[0xD0] = std::bind(&CPU::BNE_rel, this);
    opcodes[0xD8] = std::bind(&CPU::CLD_imp, this);
    opcodes[0xF0] = std::bind(&CPU::BEQ_rel, this);

    a = x = y = s = p = 0;
}

void CPU::Dump()
{
    printf("[6502]: a\t->\t#$%02x\n", a);
    printf("[6502]: x\t->\t#$%02x\n", x);
    printf("[6502]: y\t->\t#$%02x\n", y);
    printf("[6502]: s\t->\t#$%02x\n", s);
    printf("[6502]: pc\t->\t#$%04x\n", pc);
    printf("[6502]: [%s%s%s]\n", GetFlag(N_FLAG) ? "n" : ".", 
                                 GetFlag(Z_FLAG) ? "z" : ".", 
                                 GetFlag(C_FLAG) ? "c" : ".");
}

int CPU::clock()
{
    uint8_t op = Bus::read8(pc++);

    if (!opcodes[op])
    {
        printf("[6502]: Unimplemented opcode 0x%02x\n", op);
        exit(1);
    }

    return opcodes[op]();
}
