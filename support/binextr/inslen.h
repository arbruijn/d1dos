enum asmop { AO_UNK, AO_MOV, AO_LEA,
    AO_ADD, AO_OR, AO_ADC, AO_SBB, AO_AND, AO_SUB, AO_XOR, AO_CMP,
    AO_ROL, AO_ROR, AO_RCL, AO_RCR, AO_SHL, AO_SHR, AO_SAL, AO_SAR,
    AO_TEST, AO_F7_1, AO_NOT, AO_NEG, AO_MUL, AO_IMUL, AO_DIV, AO_IDIV,
    AO_INC, AO_DEC, AO_CALL, AO_CALLF, AO_JMP, AO_JMPF, AO_PUSH, AO_FF_7,
    AO_NUM };

extern const char *asmop_name[AO_NUM];

int inslen(void *buf, int buflen, enum asmop *op);

