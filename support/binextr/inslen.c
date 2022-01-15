#include "inslen.h"

const char *asmop_name[] = {"???", "mov", "lea",
    "add", "or", "adc", "sbb", "and", "sub", "xor", "cmp",
    "rol", "ror", "rcl", "rcr", "shl", "shr", "sal", "sar",
    "test", "f6_1", "not", "neg", "mul", "imul", "div", "idiv",
    "inc", "dec", "call", "callf", "jmp", "jmpf", "push", "ff_7"};

int inslen(void *buf, int buflen, enum asmop *op) {
    unsigned char *ins = buf, *insend = ins + buflen, c;
    int o32 = 1;
    int l = 1;
    int rmop = 0;
    int w;
    unsigned char modrm, sib, mod;
    if (op)
        *op = AO_UNK;
    for (;;) {
        if (ins == insend)
            return l;
        c = *ins++;
        if (c == 0x66)
            o32 ^= 1;
        else if ((c & ~0x18) != 0x26 && (c & ~3) != 0x64 && c != 0x9b && c != 0xf0 && (c & ~1) != 0xf2)
            break;
        l++;
    }
    w = o32 ? 4 : 2;
    if (c == 0xf) {
        if (ins == insend)
            return l;
        c = *ins++;
        l++;
        if ((c & ~0xf) == 0x80)
            return l + w;
        if ((c & ~7) == 0xc8 || (c & ~7) == 0x30 ||
            (c >= 0x6 && c <= 0xd) ||
            (c >= 0xa0 && c <= 0xa2) || (c >= 0xa8 && c <= 0xaa))
            return l;
        if (c == 0xba || (c & ~8) == 0xa4)
            l++;
        c = 0xf;
    } else {
        if ((c & ~1) == 0xe8 || (c & ~7) == 0xb8)
            return l + w;
        if (c == 0x9a || c == 0xea)
            return l + w + 2;
        if (c == 0xc8)
            return l + 3;
        if ((c & ~0x39) == 6)
            return l;
        if (c < 0x40 && op)
            *op = AO_ADD + (c >> 3);
        if ((c & ~0x39) == 4 || (c & ~1) == 0xa8)
            return l + (c & 1 ? w : 1);
        if ((c & ~3) == 0xa0) {
            if (op)
                *op = AO_MOV;
            return l + 4;
        }
        if ((c >= 0x40 && c <= 0x61) || (c >= 0x6c && c <= 0x6f) || (c >= 0xec && c <= 0xf5) || (c >= 0xf8 && c <= 0xfd) ||
            (c >= 0x90 && c <= 0xaf) ||
            (c & ~8) == 0xc3 || c == 0xc9 || c == 0xcc || (c & ~1) == 0xce ||
            (c & ~1) == 0xd6)
            return l;
        if ((c & ~2) == 0x68)
            return l + (c & 2 ? 1 : w);
        if ((c & 0xf0) == 0x70 || (c & ~7) == 0xe0 || (c & ~1) == 0xd4 || c == 0xcd || c == 0xeb || (c & ~7) == 0xb0)
            return l + 1;
        if ((c & ~8) == 0xc2)
            return l + 2;
        if ((c & ~3) == 0x80) {
            l += (c & 3) == 1 ? w : 1;
            rmop = AO_ADD;
        }
        if ((c & ~1) == 0xc0 || c == 0x6b || c == 0xc6)
            l++;
        if (c == 0x69 || c == 0xc7)
            l += w;
        if (op) {
            if ((c & ~3) == 0xd0)
                rmop = AO_ROL;
            if ((c & ~1) == 0xf6)
                rmop = AO_TEST;
            if ((c & ~1) == 0xfe)
                rmop = AO_INC;
            if ((c & ~3) == 0x88)
                *op = AO_MOV;
            if (c == 0x8d)
                *op = AO_LEA;
        }
    }
    if (ins == insend)
        return l;
    modrm = *ins++;
    if (rmop && op)
        *op = rmop + ((modrm & 0x38) >> 3);
    l++;
    if ((modrm & 0x7) == 4 && modrm < 0xc0) { // sib
        if (ins == insend)
            return l;
        sib = *ins++;
        l++;
        if ((modrm & 0xc0) == 0 && (sib & ~0xf8) == 5)
            l += 4;
    }
    if ((modrm & 0xc7) == 5)
        l += 4;
    if ((c & ~1) == 0xf6 && (modrm & 0x30) == 0)
        l += c & 1 ? w : 1;
    mod = modrm & 0xc0;
    return l + (mod == 0x40 ? 1 : mod == 0x80 ? 4 : 0);
}
