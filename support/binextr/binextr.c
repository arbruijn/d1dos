#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "loadle.h"
#include "inslen.h"
#include "bindata.h"
#include "loadmap.h"
#ifdef BD
#include "bddisasm/disasmtypes.h"
#include "bddisasm/bddisasm.h"
#endif

#define T int
int int_copy(int *a) { return *a; }
void int_free(int *a) {}
#include "vec.h"

const char *regs[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *wregs[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *bregs[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
const char *sregs[] = {"es", "cs", "ss", "ds", "fs", "gs", NULL, NULL};

#ifdef BD
const char *opreg(ND_OPDESC_REGISTER *reg) {
    const char **a = reg->Type == ND_REG_GPR ? reg->Size == 1 ? bregs : reg->Size == 2 ? wregs : reg->Size == 4 ? regs : NULL :
        reg->Type == ND_REG_SEG ? sregs : NULL;
    return a ? a[reg->Reg] : "???";
}
#endif

#if defined(__WATCOMC__) && __WATCOMC__ < 1200
#include <stdarg.h>
static int snprintf(char *dst, size_t size, const char *msg, ...) {
    int ret;
    va_list vp;
    va_start(vp, msg);
    ret = vsprintf(dst, msg, vp);
    va_end(vp);
    return ret;
}
#endif

typedef struct {
    int a, b;
} int_int;
void int_int_free(int_int *p) {
}
int_int int_int_copy(int_int *p) {
    int_int ab; ab.a = p->a; ab.b = p->b;
    return ab;
}
int int_int_compare(int_int *a, int_int *b) {
    return a->a - b->a;
}
#define T int_int
#include <set.h>

const char *sizename[] = {"near", "byte", "word", NULL, "dword", NULL, "fword", NULL, "qword"};

typedef struct memblock {
    int s, e;
    int no_label;
} memblock;

int opt_dis;
int memblock_count;
memblock memblocks[8];

//memblock gmemblocks[4] = {{0x10000, 0xf0000}, {0xf0000, 0xf8f0c}, {0xf8f0c,0x110f30}, {0x110f30, 0x02c83e4}};
memblock gmemblocks[4] = {{0x10000, 0xb0000}, {0xb0000, 0xb48a4}, {0xb48a4,0xc6fc0}, {0xc6fc0, 0x223ac0}};

const char *def_block_names[] = {"_TEXT:CODE", "CONST:DATA", "_DATA:DATA", "_BSS:BSS"};
const char *block_names[8];

char *skipws(const char *s) {
    while (*s == ' ' || *s == '\t')
        s++;
    return (char *)s;
}

char *skipnonws(const char *s) {
    while (*s && *s != ' ' && *s != '\t')
        s++;
    return (char *)s;
}

char *skipnonwscomma(const char *s) {
    while (*s && *s != ' ' && *s != '\t' && *s != ',')
        s++;
    return (char *)s;
}

char *skipalnum(const char *s) {
    while ((*s >= '0' && *s <= '9') ||
        (*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z') ||
        *s == '_')
        s++;
    return (char *)s;
}

char *skiphex(const char *s) {
    while ((*s >= '0' && *s <= '9') ||
        (*s >= 'A' && *s <= 'F') || (*s >= 'a' && *s <= 'f'))
        s++;
    return (char *)s;
}

reloc_t *reloc;
binmap_t *binmap;
binmap_t *symofs;
const int base_int = 0x10000;
unsigned char *base;
set_int_int data_set;

int trg_int(int ofs) {
    return get32(base, ofs - base_int);
}

unsigned char trg_byte(int ofs) {
    return base[ofs - base_int];
}

void *trg_ptr(int ofs) {
    return base + (ofs - base_int);
}

int find_memblock(int ofs) {
    int i;
    for (i = 0; i < memblock_count; i++)
        if (ofs >= memblocks[i].s && ofs < memblocks[i].e)
            return i;
    return -1;
}

vec_int extdefs;
int noout, outext;
void add_externdef(int nofs, const char *name, int size, FILE *out, int *hasline) {
    int_int ab; ab.a = nofs; ab.b = size;
    if (!set_int_int_find(&data_set, ab)) {
        set_int_int_insert(&data_set, ab);
        if (outext) {
            if (hasline && *hasline)
                fprintf(out, "\n"), *hasline = 0;
            fprintf(out, "\texterndef %s:%s\n", name, sizename[size]);
        } else
            vec_int_push_back(&extdefs, nofs);
    }
}

void memname(char *buf, int ofs, int size, FILE *out, int *hasline) {
    int nofs, mem_block;
    const char *name;
    if ((name = binmap_find_ofs(symofs, ofs))) { // specific base+ofs
        char nbuf[64];
        char *p = skipalnum(name);
        memcpy(nbuf, name, p - name);
        nbuf[p - name] = 0;
        nofs = binmap_find_name(binmap, nbuf);
        if (nofs)
            nofs += base_int;
        //printf("-> %s = %x diff %x\n", nbuf, nofs, nofs - ofs);
        if (nofs && find_memblock(nofs) == -1)
            add_externdef(nofs, nbuf, size, out, hasline);
        strcpy(buf, name);
        return;
    }
    name = binmap_find_ofs_leq(binmap, ofs - base_int, &nofs);
    if (!name)
        nofs = 0;
    else
        nofs += base_int;
    mem_block = find_memblock(ofs);
    if (mem_block != -1 && nofs < memblocks[mem_block].s) {
        sprintf(buf, "block%d + 0%xh", mem_block, ofs - memblocks[mem_block].s);
        return;
    }
    if (name && mem_block == -1) {
        //printf("add %s at %x from %x block %d\n", name, nofs, ofs, mem_block);
        add_externdef(nofs, name, size, out, hasline);
    }
    if (name && nofs == ofs) {
        //if (ofs >= gmemblocks[0].s && ofs <= gmemblocks[0].e)
        //    fprintf(out, "\tpublic %s\n", name);
        //if (strcmp(name, "__sSOSFillInfo") == 0)
        //    printf("%s ofs %x nofs %x mem2 %x %x\n", name, ofs, nofs, memblocks[2].s, memblocks[2].e);
        strcpy(buf, name);
        return;
    }
    if (!name)
        sprintf(buf, "DAT_%08x", ofs);
    else
        sprintf(buf, "%s + 0%xh", name, ofs - nofs);
}

void dumpmem(FILE *out, int s, int e, int no_label) {
    char buf[256];
    int i;
    for (i = s; i < e; ) {
        int next = e, hasdb = 0;
        if (next > i + 16)
            next = i + 16;
        while (i < next) {
            const char *name = binmap_find_ofs(binmap, i - base_int);
            if (name && !no_label && !noout) {
                if (hasdb)
                    fprintf(out, "\n"), hasdb = 0;
                fprintf(out, "\tpublic %s\n%s:\n", name, name);
            }
            if (reloc_find(reloc, i - base_int)) {
                memname(buf, trg_int(i), 0, out, &hasdb);
                if (!noout) {
                    if (hasdb)
                        fprintf(out, "\n"), hasdb = 0;
                    fprintf(out, "\tdd %s\n", buf);
                }
                i += 4;
                continue;
            }
            if (!noout) {
                if (hasdb)
                    fprintf(out, ",");
                else
                    fprintf(out, "\tdb "), hasdb = 1;
                fprintf(out, "0%02xh", trg_byte(i));
            }
            i++;
        }
        if (hasdb)
            fprintf(out, "\n");
    }
}

void dumpbss(FILE *out, memblock *mb) {
    int i, last = mb->s;
    for (i = last; i < mb->e; i++) {
        const char *name = binmap_find_ofs(binmap, i - base_int);
        if (name) {
            if (i > last) {
                fprintf(out, "\tdb %d dup(?)\n", i - last);
                last = i;
            }
            fprintf(out, "\tpublic %s\n%s:\n", name, name);
        }
    }
    if (i > last)
        fprintf(out, "\tdb %d dup(?)\n", i - last);
}

#ifdef BD
void disassemble_one(FILE *out, INSTRUX *ix, int pos, int spos, int epos) {
    char buf[256];

    const char *name = binmap_find_ofs(binmap, pos - base_int);
    if (name) {
        fprintf(out, "\tpublic %s\n", name);
        fprintf(out, "%s:\n", name);
    }

    /*
    if ((st = NdToText(&ix, (int)p, sizeof(nbuf), nbuf))) {
        fprintf(stderr, "totext failed %x\n", st);
        goto err;
    }
    */
    //fprintf(out, "\t\"%s\"\t\t; l=%d %s %d", nbuf, ix->Length, ix->Mnemonic, ix->OpOffset);
    //int oofs = ix->OpOffset;
    *buf = 0;
    if (ix->HasSeg && ix->ExpOperandsCount == 2 && // ??? strange watcom bug with repeated gs: override?
        ix->Operands[0].Type != ND_OP_MEM && ix->Operands[1].Type != ND_OP_MEM)
        fprintf(out, "\tdb 0%xh\n", trg_byte(pos));
    if (ix->HasRepnzXacquireBnd) {
        if (memcmp(ix->Mnemonic, "MOVS", 4) == 0)
            strcat(buf, "rep ");
        else
            strcat(buf, "repnz ");
    }
    if (ix->HasRepRepzXrelease)
        strcat(buf, "rep ");
    //sprintf(buf, "%-10s", ix->Mnemonic);
    strcat(buf, ix->Mnemonic);
    char *bp = buf;
    while (*bp) {
        *bp = tolower(*bp);
        bp++;
    }
    if (strcmp(buf, "callf") == 0)
        buf[4] = 0;
    if (strcmp(buf, "int3") == 0)
        strcpy(buf, "int 3");
    if (ix->ExpOperandsCount)
        strcat(buf, "\t");
    for (int i = 0; i < ix->ExpOperandsCount; i++) {
        ND_OPERAND *o = &ix->Operands[i];
        if (o->Flags.IsDefault)
            continue;
        int oofs = o->Encoding == ND_OPE_I ? ix->Imm1Offset : o->Encoding == ND_OPE_D && ix->HasMoffset? ix->MoffsetOffset : o->Encoding == ND_OPE_M ? ix->ModRmOffset + 1 + ix->HasSib : 0;
        int dst = oofs == 0 ? 0 : reloc_find(reloc, pos - base_int + oofs);
        if (i)
            strcat(buf, ", ");
        switch (o->Type) {
            case ND_OP_REG:
                strcat(buf, opreg(&o->Info.Register));
                break;
            case ND_OP_IMM:
            case ND_OP_CONST:
            case ND_OP_ADDR:
                if (dst) {
                    strcat(buf, "offset ");
                    memname(buf + strlen(buf), (int)o->Info.Immediate.Imm, 0, out, NULL);
                } else
                    sprintf(buf + strlen(buf), "0%xh", (int)o->Info.Immediate.Imm);
                break;
            case ND_OP_OFFS: {
                int opos = (int)o->Info.RelativeOffset.Rel + ix->Length + pos;
                name = binmap_find_ofs(binmap, opos - base_int);
                if (name) {
                    //fprintf(out, "\texterndef %s:near\n", name);
                    add_externdef(opos, name, 0, out, NULL);
                    strcat(buf, name);
                } else {
                    if (o->RawSize == 4)
                        strcat(buf, "near ptr ");
                    if (opos >= spos && opos <= epos)
                        sprintf(buf + strlen(buf), "$%+d", (int)o->Info.RelativeOffset.Rel + ix->Length);
                    else
                        sprintf(buf + strlen(buf), "0%xh", opos);
                }
                //sprintf(buf + strlen(buf), "0%llxh", o->Info.RelativeOffset.Rel + ix->Length + (int)p);
                break;
            }
            case ND_OP_MEM:
                if (o->Size) {
                    int sz = o->Size;
                    if (trg_byte(pos + ix->PrefLength) == 0x8c ||
                        trg_byte(pos + ix->PrefLength) == 0x8e) // load/store seg use dword ptr to prevent 66 prefix
                        sz = 4;
                    sprintf(buf + strlen(buf), "%s ptr ", sizename[sz]);
                }
                if (ix->HasSeg && o->Info.Memory.HasSeg)
                    sprintf(buf + strlen(buf), "%s:", sregs[o->Info.Memory.Seg]);
                strcat(buf, "[");
                if (o->Info.Memory.HasIndex) {
                    strcat(buf, regs[o->Info.Memory.Index]);
                    if (o->Info.Memory.Scale != 1)
                        sprintf(buf + strlen(buf), "*%d", o->Info.Memory.Scale);
                }
                if (o->Info.Memory.HasBase) {
                    if (o->Info.Memory.HasIndex)
                        strcat(buf, "+");
                    strcat(buf, regs[o->Info.Memory.Base]);
                }
                if (o->Info.Memory.HasDisp) {
                    if (o->Info.Memory.HasBase || o->Info.Memory.HasIndex)
                        strcat(buf, "+");
                    //if (!dst && o->Size && o->Info.Memory.DispSize == 4)
                    //    fprintf(out, "; MISSING DST");
                    if (dst) { //o->Size && o->Info.Memory.DispSize == 4)
                        memname(buf + strlen(buf), (int)o->Info.Memory.Disp, o->Size, out, NULL);
                    } else
                        sprintf(buf + strlen(buf), "0%xh", (int)o->Info.Memory.Disp);
                }
                strcat(buf, "]");
                break;
            default:
                fprintf(out, "unknown type %d\n", o->Type);
        }
        #if 0
        fprintf(out, " [o=%d t=%d, enc=%d, sz=%d, raw=%d, acc=%d, fl=0x%x]", oofs,
            (int)o->Type, (int)o->Encoding,
            o->Size,
            o->RawSize,
            o->Access.Access,
            o->Flags.Flags);
        if (o->Type == ND_OP_MEM) {
            fprintf(out, ":b=%x:i=%x:s=%x:d=%llx:ds=%d", o->Info.Memory.Base, o->Info.Memory.Index, o->Info.Memory.Scale, o->Info.Memory.Disp,o->Info.Memory.DispSize);
        }
        #endif
    }
    //fprintf(out, "\n");
    //fprintf(out, "\t\t; \"%s\"\n", buf);
    if (ix->Length == 2 && ix->HasModRm && ix->ExpOperandsCount == 2 && ix->Operands[0].Type == ND_OP_REG && ix->Operands[1].Type == ND_OP_REG) // assembled differently :(
        fprintf(out, "\tdb 0%xh, 0%xh ; %s\n",
            trg_byte(pos), trg_byte(pos + 1), buf);
    else if (ix->Length == 6 && !ix->HasSib && // unopt add
        trg_byte(pos) == 0x81 &&
            trg_byte(pos + 2) < 0x80 &&
            !trg_byte(pos + 3) && !trg_byte(pos + 4) && !trg_byte(pos + 5))
        fprintf(out, "\tdb 0%xh, 0%xh, 0%xh, 0, 0, 0 ; %s\n",
            trg_byte(pos), trg_byte(pos + 1), trg_byte(pos + 2), buf);
    else
        fprintf(out, "\t%s\n", buf);
    //if (strcmp(buf, nbuf))
    //    fprintf(out, "\t\t; !!!\n");
}
#else
void disassemble_rel(FILE *out, int pos, int spos, int epos) {
    char buf[256];
    unsigned char c;
    int rel, len, opos;

    const char *name = binmap_find_ofs(binmap, pos - base_int);
    if (name && !noout) {
        fprintf(out, "\tpublic %s\n", name);
        fprintf(out, "%s:\n", name);
    }

    c = trg_byte(pos);
    if (c == 0xc3) {
        if (!noout) fprintf(out, "\tretn\n");
        return;
    }
    if (c != 0xe8 && c != 0xe9 && c != 0xeb)
        abort();

    rel = c == 0xeb ? (signed char)trg_byte(pos + 1) :
        trg_int(pos + 1);
    len = c == 0xeb ? 2 : 5;
    strcpy(buf, c == 0xe8 ? "call" : "jmp");
    strcat(buf, "\t");
    opos = rel + len + pos;
    name = binmap_find_ofs(binmap, opos - base_int);
    if (name) {
        add_externdef(opos, name, 0, out, NULL);
        strcat(buf, name);
    } else {
        if (len == 5)
            strcat(buf, "near ptr ");
        if (opos >= spos && opos <= epos)
            sprintf(buf + strlen(buf), "$%+d", rel + len);
        else
            sprintf(buf + strlen(buf), "0%xh", opos);
    }
    if (!noout) fprintf(out, "\t%s\n", buf);
}
#endif

int gencode(FILE *out, int spos, int epos) {
    char buf[256];
    int pos = spos;

    #ifdef BD
    ND_CONTEXT ndc;
    NdInitContext(&ndc);
    ndc.DefCode = ND_CODE_32;
    ndc.DefData = ND_DATA_32;
    ndc.DefStack = ND_STACK_32;
    ndc.VendMode = ND_VEND_ANY;
    ndc.FeatMode = ND_FEAT_NONE;
    #endif

    int next_start = 1;
    while (pos < epos) {
        int len;
        enum asmop op;
        unsigned char c;
    
        if (reloc_find(reloc, pos - base_int)) {
            const char *name = binmap_find_ofs(binmap, pos - base_int);
            if (name) {
                if (!noout) fprintf(out, "\tpublic %s\n", name);
                if (!noout) fprintf(out, "%s:\n", name);
            }
            memname(buf, trg_int(pos), 0, out, NULL);
            if (!noout) fprintf(out, "\tdd %s\n", buf);
            pos += 4;
            continue;
        }

        if (next_start) {
            next_start = 0;
            while (pos < epos) {
                if (trg_byte(pos) == 0) {
                    int i = 0;
                    while (pos < epos && trg_byte(pos) == 0) {
                        pos++;
                        i++;
                    }
                    dumpmem(out, pos - i, pos, 0);
                } else if (pos + 2 <= epos &&
                    trg_byte(pos) == 0x8b && trg_byte(pos + 1) == 0xc0) {
                    if (!noout) fprintf(out, "\tdb 8bh,0c0h\n");
                    pos += 2;
                } else if (pos + 3 <= epos &&
                    trg_byte(pos) == 0x8d && trg_byte(pos + 1) == 0x40 &&
                    trg_byte(pos + 2) == 0) {
                    if (!noout) fprintf(out, "\tdb 8dh,040h,0\n");
                    pos += 3;
                } else
                    break;
            }
            continue;
        }
        
        #ifdef BD
        INSTRUX ix;
        NDSTATUS st;
        if ((st = NdDecodeWithContext(&ix, trg_ptr(pos), epos - pos, &ndc))) {
            fprintf(stderr, "decode failed %x at %x\n", st, pos);
            goto err;
        }
        len = ix.Length;
        #else
        len = inslen(trg_ptr(pos), epos - pos, &op);
        #endif

        c = trg_byte(pos);
        if (!opt_dis && c != 0xe9 && c != 0xe8 && c != 0xeb && c != 0xc3) {
            dumpmem(out, pos, pos + len, 0);
            pos += len;
            continue;
        }

        #ifdef BD
        disassemble_one(out, &ix, pos, spos, epos);
        #else
        disassemble_rel(out, pos, spos, epos);
        #endif
        if (c == 0xc3 || c == 0xe9 || c == 0xeb)
            next_start = 1;
        pos += len;
    }
    return 0;
#ifdef BD
err:
    return 1;
#endif
}

void write_extdefs(FILE *out, int s, int e) {
    foreach(vec_int, &extdefs, it) {
        const char *name;
        int ofs = *it.ref;
        if (ofs < s || ofs >= e)
            continue;
        if (!(name = binmap_find_ofs(binmap, ofs - base_int)))
            continue;
        fprintf(out, "\texterndef %s:near\n", name);
    }
    foreach_end
}

int genfile(FILE *out, int text_data, const char *text_align, const char *text_name, int end_label,
    const char *bss_align, const char *externdef, int oldext) {
    int b, i;
    int has_dgroup = 0;

    vec_int_clear(&extdefs);

    fprintf(out, ".386p\n");

    // scan data reloc references
    if (!oldext)
        outext++;
    for (b = 0; b < memblock_count; b++) {
        const char *name = block_names[b];
        const char *p = strchr(name, ':');
        const char *p2 = strchr(p + 1, ':');
        if (!p2)
            p2 = strchr(p + 1, 0);
        if (p2 - p == 5 && memcmp(p, ":CODE", 5) == 0 && !text_data) {
            if (oldext)
                continue;
            noout++;
            if (gencode(out, memblocks[b].s, memblocks[b].e))
                goto err;
            noout--;
        } else {
            for (i = memblocks[b].s; i < memblocks[b].e; i++)
                if (reloc_find(reloc, i - base_int)) {
                    char buf[256];
                    memname(buf, trg_int(i), 1, out, NULL);
                }
        }
    }
    if (!oldext)
        outext--;
    if (externdef)
        fprintf(out, "\texterndef %s:%s\n", externdef, sizename[0]);

    for (i = 0; i < memblock_count; i++) {
        const char *name = block_names[i];
        const char *p = strchr(name, ':');
        const char *p2 = strchr(p + 1, ':');
        if (!p2)
            p2 = strchr(p + 1, 0);
        if ((p2 - p == 5 && memcmp(p, ":DATA", 5) == 0) ||
            (p2 - p == 4 && memcmp(p, ":BSS", 5) == 0)) {
            if (!has_dgroup)
                fprintf(out, "DGROUP group "), has_dgroup = 1;
            else
                fprintf(out, ", ");
            fprintf(out, "%.*s", (int)(p - name), name);
        }
    }
    if (has_dgroup)
        fprintf(out, "\n");
    
    for (i = 0; i < memblock_count; i++) {
        const char *name = block_names[i];
        const char *p = strchr(name, ':');
        const char *cls = p + 1;
        const char *clse = strchr(cls, ':') ? strchr(cls, ':') : strchr(cls, 0);
        int iscode = clse - cls == 4 && memcmp(cls, "CODE", 4) == 0;
        int isdata = clse - cls == 4 && memcmp(cls, "DATA", 4) == 0;
        int isbss = clse - cls == 3 && memcmp(cls, "BSS", 3) == 0;
        fprintf(out, "%.*s\tsegment %s public use32 '%.*s'\n",
            (int)(p - name), name,
            *clse ? clse + 1 : 
                iscode ? text_align : isbss ? bss_align : "dword",
            (int)(clse - cls), cls);
        
        if (1) { //memblocks[i].s != memblocks[i].e || (end_label && memblocks[i].s)) {
            if (iscode && !text_data) {
                if (gencode(out, memblocks[i].s, memblocks[i].e))
                    goto err;
            } else {
                if (!memblocks[i].no_label)
                    fprintf(out, "block%d:\n", i);
                if (isbss)
                    dumpbss(out, &memblocks[i]);
                else
                    dumpmem(out, memblocks[i].s, memblocks[i].e,
                        memblocks[i].no_label);
            }
            if (end_label && isdata) {
                const char *name;
                if ((name = binmap_find_ofs(binmap, memblocks[i].e - base_int))) {
                    fprintf(out, "public %s\n", name);
                    fprintf(out, "%s:\n", name);
                }
            }
        }

        if (i < 4 && (!memblocks[i].s ||
            strcmp(def_block_names[i], block_names[i]) == 0 ||
            strncmp(text_name, block_names[i], p - name) == 0)) {
            int gblk = strncmp(text_name, block_names[i], p - name) == 0 ? 0 : i;
            write_extdefs(out, gmemblocks[gblk].s, gmemblocks[gblk].e);
        }

        fprintf(out, "%.*s\tends\n", (int)(p - name), name);
    }

    for (i = 0; i < 4; i++) {
        const char *name = def_block_names[i];
        const char *p = strchr(name, ':');
        const char *cls = p + 1;
        const char *clse = strchr(cls, ':') ? strchr(cls, ':') : strchr(cls, 0);
        int n = 0;
        //int iscode = strcmp(cls, "CODE") == 0;
        if (i < memblock_count &&
            (strcmp(block_names[i], def_block_names[i]) == 0 ||
                strncmp(text_name, block_names[i], p - name) == 0))
            continue;
        foreach(vec_int, &extdefs, it)
            if (*it.ref >= gmemblocks[i].s && *it.ref < gmemblocks[i].e)
                n++;
        foreach_end
        if (!n)
            continue;

        fprintf(out, "%.*s\tsegment byte public use32 '%.*s'\n",
            (int)(p - name), name,
            (int)(clse - cls), cls);
        write_extdefs(out, gmemblocks[i].s, gmemblocks[i].e);
        fprintf(out, "%.*s\tends\n", (int)(p - name), name);
    }
    
    
    fprintf(out, "\tend\n");
    return 0;
err:
    return 1;
}

// destructive
int parseint(char **ps, int *pint) {
    char *s = *ps, *p, c;
    int ret;
    s = skipws(s);
    if (*s >= '0' && *s <= '9') {
        ret = (int)strtol(s, &s, 0);
    } else {
        p = skipalnum(s);
        c = *p;
        *p = 0;
        if ((ret = binmap_find_name(binmap, s)) == -1) {
            fprintf(stderr, "unknown symbol %s\n", s);
            return 1;
        }
        ret += base_int;
        *p = c;
        s = p;
    }
    s = skipws(s);
    if (*s == '+' || *s == '-') {
        int neg = *s == '-', n;
        s = skipws(s + 1);
        if (*skiphex(s) == 'h') { // asm style hex
            n = (int)strtol(s, &p, 16);
            p++; // skip h
        } else
            n = (int)strtol(s, &p, 0);
        ret += neg ? -n : n;
        if (p == s)
            fprintf(stderr, "missing number after + at %s\n", s);
        s = p;
    }
    *pint = ret;
    *ps = s;
    return 0;
}

const char *opt_def = NULL, *opt_exe = NULL, *opt_map = NULL, *opt_fun = NULL, *opt_out = NULL;
int opt_parse(int argc, char **argv) {
    int opt_no_opt = 0, i;
    for (i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (arg[0] == '-' && !opt_no_opt) {
            if (arg[1] == '?' && !arg[2])
                continue;
            if (i + 1 == argc) {
                fprintf(stderr, "missing argument to %s\n", arg);
                return -1;
            }
            switch (arg[1]) {
                case 'e':
                    opt_exe = argv[++i];
                    break;
                case 'm':
                    opt_map = argv[++i];
                    break;
                case 'f':
                    opt_fun = argv[++i];
                    break;
                case 'o':
                    opt_out = argv[++i];
                    break;
                case '-':
                    opt_no_opt = 1;
            }
        } else
            opt_def = arg;
    }
    return 0;
}

int read_def(const char *def_fn) {
    char line[512];
    char text_align_global[16], bss_align_global[16];
    const char *text_name_global = "_TEXT";
    int oldext_global = 0;
    FILE *f;
    strcpy(text_align_global, "para");
    strcpy(bss_align_global, "dword");
    if (!(f = fopen(def_fn, "r"))) {
        fprintf(stderr, "error opening def file %s\n", def_fn);
        goto err;
    }
    
    while (fgets(line, sizeof(line), f)) {
        char *p, *p2, *outname, c;
        int text_data = 0;
        int end_label = 0;
        int oldext = oldext_global;
        const char *text_align = text_align_global;
        const char *text_name = text_name_global;
        const char *bss_align = bss_align_global;
        const char *externdef = NULL;
        char *freestrs[8];
        int n_freestrs = 0;
        FILE *fo;

        binmap_clear(symofs);

        if ((p = strchr(line, '\n')))
            *p = 0;
        if ((p = strchr(line, '#'))) {
            while (*p == '#') {
                char *p1 = skipalnum(++p);
                char *p2 = skipws(p1);
                char *arg = p2;
                *p1 = 0;
                if (strcmp(p, "symofs") == 0) {
                    for (;;) {
                        char *p3 = arg;
                        int sofs;
                        if (parseint(&p3, &sofs))
                            goto err;
                        p2 = skipws(p3);
                        c = *p3;
                        *p3 = 0;
                        binmap_add(symofs, sofs, arg);
                        *p3 = c;
                        if (*p2 != ',')
                            break;
                        arg = skipws(p2 + 1);
                    }
                }
                if (strcmp(p, "textdata") == 0)
                    text_data = 1;
                else if (strcmp(p, "oldext") == 0) {
                    oldext = 1;
                    if (p == line + 1)
                        oldext_global = oldext;
                } else if (strcmp(p, "endlabel") == 0)
                    end_label = 1;
                else if (strcmp(p, "textalign") == 0) {
                    char *p3 = skipalnum(arg);
                    p2 = skipws(p3);
                    *p3 = 0;
                    text_align = arg;
                    if (p == line + 1)
                        strcpy(text_align_global, text_align);
                } else if (strcmp(p, "bssalign") == 0) {
                    char *p3 = skipalnum(arg);
                    p2 = skipws(p3);
                    *p3 = 0;
                    bss_align = arg;
                    if (p == line + 1)
                        strcpy(bss_align_global, bss_align);
                } else if (strcmp(p, "textname") == 0) {
                    char *p3 = skipalnum(arg);
                    p2 = skipws(p3);
                    *p3 = 0;
                    text_name = arg;
                } else if (strcmp(p, "externdef") == 0) {
                    char *p3 = skipalnum(arg);
                    p2 = skipws(p3);
                    *p3 = 0;
                    externdef = arg;
                }
                p = p2;
            }
            *strchr(line, '#') = 0;
        }
        p = skipws(line);
        if (!*p)
            continue;
        p2 = skipalnum(line);
        if (*p2 != ' ') {
            fprintf(stderr, "missing space at %s\n", p2);
            goto err;
        }
        outname = p;
        *p2++ = 0, p = p2;
        
        memset(memblocks, 0, sizeof(memblocks));
        memblock_count = 0;
        set_int_int_clear(&data_set);
        
        while (memblock_count < 8) {
            p = skipws(p);
            if (*p == '!') {
                memblocks[memblock_count].no_label = 1;
                p++;
            }
            if (parseint(&p, &memblocks[memblock_count].s)) {
                fprintf(stderr, "in memblock %d in %s\n", memblock_count, line);
                goto err;
            }
            p = skipws(p);
            if (*p != ':') {
                fprintf(stderr, "missing : at %s\n", p);
                goto err;
            }
            p++;
            p = skipws(p);
            if (parseint(&p, &memblocks[memblock_count].e)) {
                fprintf(stderr, "in memblock %d in %s\n", memblock_count, line);
                goto err;
            }
            p = skipws(p);
            if (*p == ':') {
                p = skipws(p + 1);
                p2 = skipalnum(p);
                if (*p2 == ':')
                    p2 = skipalnum(skipws(p2 + 1));
                if (*p2 == ':')
                    p2 = skipalnum(skipws(p2 + 1));
                c = *p2, *p2 = 0;
                block_names[memblock_count] = freestrs[n_freestrs++] =
                    strdup(p);
                *p2 = c;
                p = skipws(p2);
                
            } else if (memblock_count >= 4) {
                fprintf(stderr, "too many blocks in %s\n", line);
                goto err;
            } else
                block_names[memblock_count] = def_block_names[memblock_count];
            memblock_count++;
            if (!*p || *p == '\n')
                break;
            if (*p != ',') {
                fprintf(stderr, "expected , at %s\n", p);
                goto err;
            }
            p++;
        }
        
        strcat(outname, ".asm");
        //printf("creating %s %x:%x, %x:%x %x:%x %x:%x\n", outname, memblocks[0].s, memblocks[0].e, 
        //    memblocks[1].s, memblocks[1].e, memblocks[2].s, memblocks[2].e, memblocks[3].s, memblocks[3].e);
        printf("creating %s\n", outname);
        if (!(fo = fopen(outname, "w"))) {
            fprintf(stderr, "error creating %s\n", outname);
            goto err;
        }
        genfile(fo, text_data, text_align, text_name, end_label, bss_align,
            externdef, oldext);
        fclose(fo);
        while (n_freestrs)
            free(freestrs[--n_freestrs]);
    }

    fclose(f);
    return 0;

err:
    return -1;
}

int read_fun(const char *fun) {
    int ofs, nofs;
    char buf[256];
    const char *name = fun;
    const char *fn;
    char *p;
    FILE *f;

    if ((p = strchr(fun, ':'))) {
        memcpy(buf, fun, p - fun);
        buf[p - fun] = 0;
        name = buf;
    }
    if ((ofs = binmap_find_name(binmap, name)) == -1) {
        fprintf(stderr, "function %s not found\n", name);
        goto err;
    }
    if (p) {
        const char *name2 = p + 1;
        if ((nofs = binmap_find_name(binmap, name2)) == -1) {
            fprintf(stderr, "function %s not found\n", name2);
            goto err;
        }
    } else if (!binmap_find_ofs_geq(binmap, ofs + 1, &nofs)) {
        fprintf(stderr, "next function for %s not found\n", name);
        goto err;
    }
    ofs += base_int;
    nofs += base_int;
    printf("function %s start %x end %x\n", fun, ofs, nofs);
    memset(memblocks, 0, sizeof(memblocks));
    memblocks[0].s = ofs;
    memblocks[0].e = nofs;
    memblock_count = 1;
    
    if (opt_out) {
        fn = opt_out;
    } else {
        if (!p)
            snprintf(buf, sizeof(buf), "%s", fun);
        strcat(buf, ".asm");
        fn = buf;
    }
    if (!(f = fopen(fn, "w"))) {
        fprintf(stderr, "cannot create %s\n", fn);
        goto err;
    }
    opt_dis = 1;
    genfile(f, 0, "dword", "_TEXT", 0, "para", NULL, 0);
    fclose(f);
    
    return 0;
err:
    return -1;
}

int main(int argc, char **argv) {
    void *segptrs[2];

    if (opt_parse(argc, argv))
        goto err;

    if (!opt_def && !opt_fun) {
        fprintf(stderr, "syntax: -e prog.exe -m prog.map lib.def\n");
        goto err;
    }
        
    if (!opt_map) {
        int i = strlen(opt_exe);
        if (i > 4 && opt_exe[i - 4] == '.') {
            char *p = strdup(opt_exe);
            strcpy(p + i - 4, ".map");
            opt_map = p;
        }
    }

    if (!(base = loadle(opt_exe, segptrs, &reloc)))
        goto err;
    if (!(binmap = binmap_load(opt_map))) {
        fprintf(stderr, "load map failed\n");
        goto err;
    }

    data_set = set_int_int_init(int_int_compare);
    extdefs = vec_int_init();

    symofs = binmap_create();

    if (opt_def) {
        if (read_def(opt_def))
            goto err;
    } else if (opt_fun) {
        if (read_fun(opt_fun))
            goto err;
    }

    return 0;
err:
    return 1;
}
