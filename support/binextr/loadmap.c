#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "loadmap.h"

typedef struct {
    int ofs;
    char *name;
} ofs_name;

void ofs_name_free(ofs_name *p) {
    free(p->name);
}
ofs_name ofs_name_copy(ofs_name *p) {
    ofs_name on; on.ofs = p->ofs; on.name = strdup(p->name);
    return on;
}
int ofs_name_compare(ofs_name *a, ofs_name *b) {
    return a->ofs - b->ofs; // || strcmp(a->name, b->name);
}
#define T ofs_name
#include <set.h>

#define T ofs_name
#define A JOIN(set, T)
#define B JOIN(A, node)
static inline B*
JOIN(B, prev)(B* self)
{
    if(self->l)
    {
        self = self->l;
        while(self->r)
            self = self->r;
    }
    else
    {
        B* parent = self->p;
        while(parent && self == parent->l)
        {
            self = parent;
            parent = parent->p;
        }
        self = parent;
    }
    return self;
}
static inline B*
JOIN(A, find_leq)(A* self, T key)
{
    B* node = self->root, *nnode;
    while(node)
    {
        int diff = self->compare(&key, &node->key);
        if(diff == 0)
            return node;
        else
        if(diff < 0)
            nnode = node->l;
        else
            nnode = node->r;
        if (!nnode)
            return diff < 0 ? JOIN(B, prev)(node) : node;
        node = nnode;
    }
    return NULL;
}
static inline B*
JOIN(A, find_geq)(A* self, T key)
{
    B* node = self->root, *nnode;
    while(node)
    {
        int diff = self->compare(&key, &node->key);
        if(diff == 0)
            return node;
        else
        if(diff < 0)
            nnode = node->l;
        else
            nnode = node->r;
        if (!nnode)
            return diff < 0 ? node : JOIN(B, next)(node);
        node = nnode;
    }
    return NULL;
}
#undef T
#undef A
#undef B

typedef struct {
    char *name;
    int ofs;
} name_ofs;

void name_ofs_free(name_ofs *p) {
    free(p->name);
}
name_ofs name_ofs_copy(name_ofs *p) {
    name_ofs no; no.name = strdup(p->name); no.ofs = p->ofs;
    return no;
}
int name_ofs_compare(name_ofs *a, name_ofs *b) {
    return strcmp(a->name, b->name);
}
#undef T
#define T name_ofs
#include <set.h>

enum tp { TP_UNK, TP_MEM, TP_SEG };
enum mode { M_UNK, M_HDR, M_DATA };

struct binmap {
    set_ofs_name ofsmap;
    set_name_ofs namemap;
};

char *binmap_find_ofs(binmap_t *binmap, int ofs) {
    set_ofs_name_node *s;
    ofs_name on = { 0, 0 }; on.ofs = ofs;
    s = set_ofs_name_find(&binmap->ofsmap, on);
    return s ? s->key.name : NULL;
}

char *binmap_find_ofs_leq(binmap_t *binmap, int ofs, int *nofs) {
    set_ofs_name_node *s;
    ofs_name on = { 0, 0 }; on.ofs = ofs;
    s = set_ofs_name_find_leq(&binmap->ofsmap, on);
    if (s)
        *nofs = s->key.ofs;
    return s ? s->key.name : NULL;
}

char *binmap_find_ofs_geq(binmap_t *binmap, int ofs, int *nofs) {
    set_ofs_name_node *s;
    ofs_name on = { 0, 0 }; on.ofs = ofs;
    s = set_ofs_name_find_geq(&binmap->ofsmap, on);
    if (s)
        *nofs = s->key.ofs;
    return s ? s->key.name : NULL;
}

int binmap_find_name(binmap_t *binmap, const char *name) {
    set_name_ofs_node *s;
    name_ofs no = { 0, 0 }; no.name = (char *)name;
    s = set_name_ofs_find(&binmap->namemap, no);
    return s ? s->key.ofs : -1;
}

void binmap_free(binmap_t *binmap) {
    free(binmap);
}

binmap_t *binmap_create() {
    binmap_t *binmap;
    if (!(binmap = malloc(sizeof(*binmap))))
        return NULL;
    
    binmap->ofsmap = set_ofs_name_init(ofs_name_compare);
    binmap->namemap = set_name_ofs_init(name_ofs_compare);
    
    return binmap;
}

void binmap_add(binmap_t *binmap, int ofs, const char *name) {
    ofs_name on;
    name_ofs no;
    on.ofs = ofs; on.name = strdup(name);
    no.name = strdup(name); no.ofs = ofs;
    set_ofs_name_insert(&binmap->ofsmap, on);
    set_name_ofs_insert(&binmap->namemap, no);
}

void binmap_clear(binmap_t *binmap) {
    set_ofs_name_clear(&binmap->ofsmap);
    set_name_ofs_clear(&binmap->namemap);
}

binmap_t *binmap_load(const char *filename) {
    FILE *f;
    char line[256], *p;
    enum tp tp = TP_UNK;
    enum mode mode = M_UNK;
    int segmax[2] = {0, 0};
    int segofs[2] = {0, 0};
    binmap_t *binmap;

    if (!(binmap = binmap_create()))
        goto err;
    
    if (!(f = fopen(filename, "r")))
        goto err;

    
    while (fgets(line, sizeof(line), f)) {
        if ((p = strchr(line, '\n'))) {
            if (p > line && p[-1] == '\r')
                p--;
            *p = 0;
        }
        p = line;
        while (*p == ' ')
            p++;
        if (*p == '|') {
            tp = TP_UNK;
            if (strstr(p, "Memory Map")) {
                tp = TP_MEM;
                segofs[0] = 0;
                segofs[1] = segofs[0] + ((segmax[0] + 65535) & ~65535);
            }
            if (strstr(p, "Segments"))
                tp = TP_SEG;
            mode = M_HDR;
            continue;
        }
        if (mode == M_HDR && line[0] == '=') {
            mode = M_DATA;
            continue;
        }
        if (mode != M_DATA)
            continue;
        if (tp == TP_SEG && *p) {
            char *fld[5];
            int seg, ofs, len, end;
            int i = 0;
            while (*p && i < 5) {
                while (*p && *p == ' ')
                    p++;
                fld[i++] = p;
                while (*p && *p != ' ')
                    p++;
                if (!*p)
                    break;
                *p++ = 0;
            }
            if (i < 4)
                continue;
            if (i == 4) { // fld[2] is optional
                fld[4] = fld[3];
                fld[3] = fld[2];
                fld[2] = "";
            }
            seg = (int)strtol(fld[3], NULL, 16);
            ofs = (int)strtol(fld[3] + 5, NULL, 16);
            len = (int)strtol(fld[4], NULL, 16);
            if (seg >= 2)
                continue;
            end = ofs + len;
            if (end > segmax[seg - 1])
                segmax[seg - 1] = end;
        }
        if (tp == TP_MEM && strlen(line) > 15 && line[4] == ':') {
            int seg = strtoul(line, NULL, 16);
            int ofs = strtoul(line + 5, NULL, 16);
            ofs += segofs[seg - 1];
            p = line + 15;
            //printf("%08x %s\n", ofs, p);
            binmap_add(binmap, ofs, p);
        }
    }
    fclose(f);
    
    return binmap;

#if 0
    #define strc(n) (*(str *)&(n))
    #define sym_lookup(a, n) set_sym_find(a, (sym) { n, 0, 0 })
    #if 0
    const char *n = "game_render_frame_";
    //str name = str_init("game_render_frame_");
    set_sym_node *s = set_sym_find(&syms, (sym) { *(str *)&n, 0, 0 });
    //str_free(&name);
    if  (!s)
        printf("not found!\n");
    else
        printf("render_frame %04x:%08x\n", s->key.seg, s->key.ofs);
    set_sym_free(&syms);
    #endif
    for (struct symptr *sp = symtab; sp->name; sp++) {
        set_sym_node *s;
        s = sym_lookup(&syms, strc(sp->name));
        if (!s && sp->name[0] == 'p') {
            const char *p = sp->name + 1;
            s = sym_lookup(&syms, strc(p));
            if (!s) {
                str n = str_init("_");
                str_append(&n, sp->name + 1);
                s = sym_lookup(&syms, n);
                str_free(&n);
            }
        }
        if (!s) {
            str n = str_init(sp->name);
            str_append(&n, "_");
            s = sym_lookup(&syms, n);
            str_free(&n);
        }
        if (!s) {
            str n = str_init("_");
            str_append(&n, sp->name);
            s = sym_lookup(&syms, n);
            str_free(&n);
        }
        if (!s) {
            fprintf(stderr, "not found: %s\n", sp->name);
            abort();
        }
        if (segptrs) {
            *sp->ptr = segptrs[s->key.seg - 1] + s->key.ofs;
            //printf("%s -> %p\n", sp->name, *sp->ptr);
        }
    }
#endif

err:
    if (binmap)
        binmap_free(binmap);
    return NULL;
}
