#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "bindata.h"

#define EXE_HDR_SIZE 64
#define BW_HDR_SIZE 176

typedef struct {
    int src;
    int trg;
} src_trg;

void src_trg_free(src_trg *p) {
}
src_trg src_trg_copy(src_trg *p) {
    src_trg st; st.src = p->src; st.trg = p->trg;
    return st;
}
int src_trg_compare(src_trg *a, src_trg *b) {
    return a->src - b->src;
}
#define T src_trg
#include <set.h>

typedef struct reloc {
    set_src_trg map;
} reloc_t;

reloc_t *reloc_init() {
    reloc_t *reloc;
    if (!(reloc = malloc(sizeof(*reloc))))
        return NULL;
    reloc->map = set_src_trg_init(src_trg_compare);
    return reloc;
}
void reloc_add(reloc_t *reloc, int src, int trg) {
    src_trg st; st.src = src; st.trg = trg;
    set_src_trg_insert(&reloc->map, st);
}
int reloc_find(reloc_t *reloc, int src) {
    set_src_trg_node *s;
    src_trg st; st.src = src; st.trg = 0;
    s = set_src_trg_find(&reloc->map, st);
    return s ? s->key.trg : 0;
}

struct le_hdr {
    int id;
    int format_level;
    int cpu_os_type;
    int module_version;
    int module_flags;
    int module_num_pages;
    int eip_object_num;
    int eip;
    int esp_object_num;
    int esp;
    int page_size;
    int last_page_size;
    int fixup_section_size;
    int fixup_section_checksum;
    int loader_section_size;
    int loader_section_checksum;
    int object_table_offset;
    int num_objects_in_module;
    int object_page_table_offset;
    int object_iter_pages_offset;
    int resource_table_offset;
    int num_resource_table_entries;
    int resident_name_tbl_offset;
    int entry_tbl_offset;
    int module_directives_offset;
    int num_module_directives;
    int fixup_page_table_offset;
    int fixup_record_table_offset;
    int import_module_tbl_offset;
    int num_import_mod_entries;
    int import_proc_tbl_offset;
    int per_page_checksum_offset;
    int data_pages_offset;
    int num_preload_pages;
    int non_res_name_tbl_offset;
    int non_res_name_tbl_length;
    int non_res_name_tbl_checksum;
    int auto_ds_object_num;
    int debug_info_offset;
    int debug_info_length;
    int num_instance_preload;
    int num_instance_demand;
    int heapsize;
};

struct obj_entry {
    int virtual_size;
    int reloc_base_addr;
    int flags;
    int page_table_index;
    int page_table_count;
    int reserved;
};


void *loadle(const char *filename, void **segptrs, reloc_t **preloc) {
    FILE *f = NULL;
    char buf[256];
    reloc_t *reloc = NULL;
    void *data_ptr = NULL;
    int exesize, ofs, img_ofs, le_ofs;
    struct le_hdr hdr;
    struct obj_entry objs[2];
    int base, data_size;
    int *fixup_pages;
    char *fixup_records;
    int fixup_size;
    int i, data_ofs;

    if (preloc)
        reloc = *preloc = reloc_init();

    if (!(f = fopen(filename, "rb"))) {
        fprintf(stderr, "cannot open %s: %s\n", filename, strerror(errno));
        goto err;
    }
    if (fread(buf, EXE_HDR_SIZE, 1, f) != 1) {
        perror("cannot read");
        goto err;
    }
    if (buf[0] != 'M' || buf[1] != 'Z') {
        fprintf(stderr, "invalid exe hdr");
        goto err;
    }
    exesize = (get16(buf, 4) << 9) + (get16(buf, 2) -
        (get16(buf, 2) == 0 ? 0 : 512));
    ofs = 0;
    if (get16(buf, 0x18) < 0x40) {
        ofs = exesize;
        do {
            fseek(f, ofs, SEEK_SET);
            if (fread(buf, BW_HDR_SIZE, 1, f) != 1) {
                perror("cannot read bw");
                goto err;
            }
            if (buf[0] != 'B' || buf[1] != 'W')
                break;
            ofs = get32(buf, 28);
        } while (ofs != 0);
        if (buf[0] != 'M' || buf[1] != 'Z') {
            fprintf(stderr, "original exe header not found\n");
            goto err;
        }
    }
    img_ofs = ofs;
    le_ofs = ofs + get32(buf, 60);
    fseek(f, le_ofs, SEEK_SET);
    if (fread(&hdr, sizeof(hdr), 1, f) != 1) {
        perror("cannot read le her");
        goto err;
    }
    if (hdr.id != 0x0000454c) {
        fprintf(stderr, "le header not found at 0x%x\n", le_ofs);
        goto err;
    }

    //printf("img ofs %x\n", img_ofs);

    fseek(f, le_ofs + hdr.object_table_offset, SEEK_SET);
    if (fread(&objs, sizeof(objs), 1, f) != 1) {
        perror("cannot read le objs");
        goto err;
    }

    //printf("obj0 virt size %x\n", objs[0].virtual_size);

    base = objs[0].reloc_base_addr;
    data_size = (objs[1].reloc_base_addr - base) + objs[1].virtual_size;
    
    if (!(fixup_pages = malloc((hdr.module_num_pages + 1) * 4))) {
        fprintf(stderr, "alloc fixup record\n");
        goto err;
    }
    fseek(f, le_ofs + hdr.fixup_page_table_offset, SEEK_SET);
    if (fread(fixup_pages, (hdr.module_num_pages + 1) * 4, 1, f) != 1) {
        perror("cannot read fixup pages");
        goto err;
    }
    //printf("fixup pages 0 %x 1 %x -2 %x -1 %x\n", fixup_pages[0], fixup_pages[1],
    //    fixup_pages[hdr.module_num_pages - 1], fixup_pages[hdr.module_num_pages]);


    fixup_size = fixup_pages[hdr.module_num_pages];
    if (!(fixup_records = malloc(fixup_size))) {
        fprintf(stderr, "alloc fixup record\n");
        goto err;
    }
    fseek(f, le_ofs + hdr.fixup_record_table_offset, SEEK_SET);
    if (fread(fixup_records, fixup_size, 1, f) != 1) {
        perror("cannot read fixup records");
        goto err;
    }
    
    
    //printf("data_size %x\n", data_size);
    //printf("ok\n");

    //void *p = (void *)base;
    //int data_size = 4096 * hdr.module_num_pages
    data_ptr = malloc(data_size);
    if (!data_ptr)
        goto err;
    // mmap((void*) p, data_size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANON|MAP_PRIVATE|MAP_FIXED, -1, 0);
    //printf("memptr=%p\n", data_ptr);

    data_ofs = img_ofs + hdr.data_pages_offset;
    fseek(f, data_ofs, SEEK_SET);
    for (i = 0; i < 2; i++) {
        int pn, size;
        void *obj_dest = (char *)data_ptr + objs[i].reloc_base_addr - base;
        if (segptrs)
            segptrs[i] = obj_dest;
        //printf("obj %d addr %p\n", i, obj_dest);
        size = objs[i].page_table_count * 4096;
        if (i == 1 && hdr.last_page_size != 0)
            size += hdr.last_page_size - 4096;
        if (fread(obj_dest, size, 1, f) != 1) {
            perror("cannot read data");
            goto err;
        }
        for (pn = objs[i].page_table_index; pn < objs[i].page_table_index + objs[i].page_table_count; pn++) {
            char *fixup = fixup_records + fixup_pages[pn - 1];
            char *fixup_end = fixup_records + fixup_pages[pn];
            char *page = (char *)obj_dest + (pn - objs[i].page_table_index) * 4096;
            while (fixup < fixup_end) {
                int fixup_size, srcofs, dst, obj, src_ofs, dst_ofs;
                char *src;
                if (fixup[0] == 2 && fixup[1] == 0) {
                    //char *src = page + get16(fixup, 2); // + objs[fixup[4]].reloc_base_addr
                    //printf("%p set selector obj %02x\n", src, fixup[4]);
                    fixup += 5;
                    // ignored...
                    continue;
                }
                if (fixup[0] != 7) {
                    fprintf(stderr, "unknown fixup %02x at %x\n", fixup[0], (int)(fixup - fixup_records));
                    goto err;
                }
                fixup_size = fixup[1] & 0x10 ? 9 : 7;
                srcofs = (short)get16(fixup, 2); // + objs[fixup[4]].reloc_base_addr
                dst = fixup[1] & 0x10 ? get32(fixup, 5) : get16(fixup, 5);
                obj = (unsigned char)fixup[4] - 1;
                fixup += fixup_size;

                if (srcofs < 0) // duplicate
                    continue;

                src = page + srcofs;
                src_ofs = src - (char *)data_ptr;
                dst_ofs = dst + objs[obj].reloc_base_addr - base;
                if (reloc) {
                    reloc_add(reloc, src_ofs, dst_ofs);
                    //printf("%x -> %x\n", src_ofs, dst_ofs);
                }
                
                //if (get32(src, 0) != dst) // will happen on second page boundary fixup
                //    printf("%p got %x exp %x target %02x obj %02x\n", src, get32(src, 0), dst, fixup[1], fixup[4]);
                set32(src, 0, base + dst_ofs);
            }
        }
    }
    fclose(f);
    return data_ptr;
err:
    if (data_ptr)
        free(data_ptr);
    if (f)
        fclose(f);
    return NULL;
}
