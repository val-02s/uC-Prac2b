/******************************************************************************
Prac 2 - AVR ASM OpCode Decoder
*******************************************************************************/
#include <stdio.h>
#include <inttypes.h>

const uint8_t flash_mem[] = {
    0x00, 0x24, 0xA0, 0xE0, 0xB2, 0xE0, 0x0D, 0x91, 0x00, 0x30, 0xE9, 0xF7, 0x11, 0x97, 0xC0, 0xE0,
    0xD2, 0xE0, 0x09, 0x91, 0x1E, 0x91, 0x01, 0x17, 0x51, 0xF4, 0x0A, 0x2F, 0x0A, 0x95, 0x1C, 0x2F, 
    0x01, 0x17, 0xB9, 0xF7, 0x0B, 0x2F, 0x1D, 0x2F, 0x01, 0x17, 0x99, 0xF7, 0x03, 0x94, 0x00, 0x00};
    
const uint16_t inst16_table[] = {
    0x0000, //NOP  0000 0000 0000 0000
};
enum {
    e_NOP,
};

// 0x2400
// 0010 0100 0000 0000
// 0010 01rd dddd rrrr -> CLR R0

// 0xE0A0
// 1110 0000 1010 0000
// 1110 KKKK dddd KKKK -> LDI R26, 0x00

// 0xE0B2
// 1110 0000 1011 0010
// 1110 KKKK dddd KKKK -> LDI R27, 0x02

// 0x910D
// 1001 0001 0000 1101
// 1001 000d dddd 1101 -> LD R16, X+

// 0x3000
// 0011 0000 0000 0000
// 0011 KKKK dddd KKKK -> CPI R16, 0x00

// 0xF7E9
// 1111 0111 1110 1001
// 1111 01kk kkkk k001 -> BRNE -2

// 0x9711
// 1001 0111 0001 0001
// 1001 0111 KKdd KKKK -> SBIW R27:R26, 0x01

// 0xE0C0
// 1110 0000 1100 0000
// 1110 KKKK dddd KKKK -> LDI R28, 0x00

// 0xE0D2
// 1110 0000 1101 0010
// 1110 KKKK dddd KKKK -> LDI R29, 0x02

// 0x9109
// 1001 0001 0000 1001
// 1001 000d dddd 1001 -> LD R16, Y+

// 0x911E
// 1001 0001 0001 1110
// 1001 000d dddd 1001 -> LD R17, -X

// 0x1701
// 0001 0111 0000 0001
// 0001 01rd dddd rrrr -> CP R16, R17

// 0xF451
// 1111 0100 0101 0001
// 1111 01kk kkkk k001 -> BRNE +11

// 0x2F0A
// 0010 1111 0000 1010
// 0010 11rd dddd rrrr -> MOV R16, R26

// 0x950A
// 1001 0101 0000 1010
// 1001 010d dddd 1010 -> DEC R16

// 0x2F1C
// 0010 1111 0001 1100
// 0010 11rd dddd rrrr -> MOV R17, R28

// 0x1701
// 0001 0111 0000 0001
// 0001 01rd dddd rrrr -> CP R16, R17

// 0xF7B9
// 1111 0111 1011 1001
// 1111 01kk kkkk k001 -> BRNE -8

// 0x2F0B
// 0010 1111 0000 1011
// 0010 11rd dddd rrrr -> MOV R16, R27

// 0x2F1D
// 0010 1111 0001 1101
// 0010 11rd dddd rrrr -> MOV R17, R29

// 0x1701
// 0001 0111 0000 0001
// 0001 01rd dddd rrrr -> CP R16, R17

// 0xF799
// 1111 0111 1001 1001
// 1111 01kk kkkk k001 -> BRNE -12

// 0x9403
// 1001 0100 0000 0011
// 1001 010d dddd 0011 -> INC R0

// 0x0000
// 0000 0000 0000 0000
// 0000 0000 0000 0000 -> NOP

typedef union {
    uint16_t op16;
    struct {
        uint16_t op4 : 4;
        uint16_t d5  : 5;
        uint16_t op7 : 7;
    } type1; // INC, DEC, LD X+
    struct {
        uint16_t r4  : 4;
        uint16_t d5  : 5;
        uint16_t r1  : 1;
        uint16_t op6 : 6;
    } type2; // CLR, MOV, CP, ADC, SBC
    struct {
        uint16_t kl  : 4;
        uint16_t d4  : 4;
        uint16_t kh  : 4;
        uint16_t op4 : 4;
    } type3; // LDI, CPI, SUBI, ORI
    struct {
        uint16_t kl  : 4;
        uint16_t d2  : 2;
        uint16_t kh  : 2;
        uint16_t op8 : 8;
    } type4; // SBIW
    struct {
        uint16_t s3  : 3;
        uint16_t k7  : 7;
        uint16_t op6 : 6;
    } type5; // BRNE
} Op_Code_t;

int main()
{
    Op_Code_t *inst;
    printf("- Practica 2: AVR OpCode -\n");

    for (uint8_t idx = 0; idx < sizeof(flash_mem); idx += 2)
    {
        inst = (Op_Code_t *) &flash_mem[idx];
        uint16_t op = inst->op16;

        if (op == inst16_table[e_NOP]) {
            printf("NOP\n");
        }

        else if ((op & 0xFC00) == 0x2400 && inst->type2.d5 == ((inst->type2.r1 << 4) | inst->type2.r4)) {
            printf("CLR r%d\n", inst->type2.d5);
        }

        else if ((op & 0xF000) == 0xE000) {
            uint8_t rd = inst->type3.d4 + 16;
            uint8_t k  = (inst->type3.kh << 4) | inst->type3.kl;
            printf("LDI r%d, 0x%02X\n", rd, k);
        }

        else if ((op & 0xFE0F) == 0x900D) {
            printf("LD r%d, X+\n", inst->type1.d5);
        }
        
        else if ((op & 0xFE0F) == 0x9009) {
        printf("LD r%d, Y+\n", inst->type1.d5);
        
        }
        
        else if ((op & 0xFE0F) == 0x900E) {
        printf("LD r%d, -X\n", inst->type1.d5);
        
        }

        else if ((op & 0xF000) == 0x3000) {
            uint8_t rd = inst->type3.d4 + 16;
            uint8_t k  = (inst->type3.kh << 4) | inst->type3.kl;
            printf("CPI r%d, 0x%02X\n", rd, k);
        }

        else if ((op & 0xFC07) == 0xF401) {
            int8_t k = (inst->type5.k7 & 0x40) ? (int8_t)(inst->type5.k7 | 0x80) : (int8_t)inst->type5.k7;
            printf("BRNE .%+d\n", k + 1);
        }

        else if ((op & 0xFF00) == 0x9700) {
            uint8_t rd = 24 + inst->type4.d2 * 2;
            uint8_t k  = (inst->type4.kh << 4) | inst->type4.kl;
            printf("SBIW r%d:r%d, 0x%02X\n", rd + 1, rd, k);
        }

        else if ((op & 0xF000) == 0xD000) {
            int16_t k = (op & 0xFFF);
            if (k & 0x800) k |= 0xF000;
            printf("RCALL .%+d\n", k + 1);
        }

        else if ((op & 0xFC00) == 0x0800) {
            uint8_t rr = (inst->type2.r1 << 4) | inst->type2.r4;
            printf("SBC r%d, r%d\n", inst->type2.d5, rr);
        }

        else if ((op & 0xFC00) == 0x1C00) {
            uint8_t rr = (inst->type2.r1 << 4) | inst->type2.r4;
            printf("ADC r%d, r%d\n", inst->type2.d5, rr);
        }

        else if ((op & 0xFF00) == 0x0100) {
            printf("MOVW r%d, r%d\n", ((op >> 4) & 0xF) * 2, (op & 0xF) * 2);
        }

        else if ((op & 0xF000) == 0x5000) {
            uint8_t rd = inst->type3.d4 + 16;
            uint8_t k  = (inst->type3.kh << 4) | inst->type3.kl;
            printf("SUBI r%d, 0x%02X\n", rd, k);
        }

        else if ((op & 0xF000) == 0x6000) {
            uint8_t rd = inst->type3.d4 + 16;
            uint8_t k  = (inst->type3.kh << 4) | inst->type3.kl;
            printf("ORI r%d, 0x%02X\n", rd, k);
        }

        else if ((op & 0xFC00) == 0x1400) {
            uint8_t rr = (inst->type2.r1 << 4) | inst->type2.r4;
            printf("CP r%d, r%d\n", inst->type2.d5, rr);
        }

        else if ((op & 0xFC00) == 0x2C00) {
            uint8_t rr = (inst->type2.r1 << 4) | inst->type2.r4;
            printf("MOV r%d, r%d\n", inst->type2.d5, rr);
        }

        else if ((op & 0xFE0F) == 0x9403) {
            printf("INC r%d\n", inst->type1.d5);
        }

        else if ((op & 0xFE0F) == 0x940A) {
            printf("DEC r%d\n", inst->type1.d5);
        }
       
    }
    return 0;
}
