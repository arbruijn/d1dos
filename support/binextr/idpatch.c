#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

char *memmem(const char *a, int alen, const char *b, int blen) {
    int c;
    const char *p, *aend;
    if (!blen)
        return (char *)a;
    if (alen < blen)
        return NULL;
    c = *b++;
    blen--;
    aend = a + alen - blen;
    while ((p = memchr(a, c, aend - a))) {
        if (memcmp(p + 1, b, blen) == 0)
            return (char *)p;
        a = p + 1;
    }
    return NULL;
}

int file_write(const char *fn, char *buf, int size) {
    FILE *f;
    if (!(f = fopen(fn, "wb"))) {
        fprintf(stderr, "Error creating %s\n", fn);
        goto err;
    }
    if (fwrite(buf, size, 1, f) != 1) {
        fprintf(stderr, "Error writing %s\n", fn);
        goto err;
    }
    fclose(f);
    return 0;
err:
    if (f)
        fclose(f);
    return -1;
}

int file_read(const char *fn, char **pbuf, int *psize) {
    FILE *f = NULL;
    char *buf = NULL;
    int size;
    if (!(f = fopen(fn, "rb"))) {
        fprintf(stderr, "Error opening %s\n", fn);
        goto err;
    }
    fseek(f, 0, SEEK_END);
    size = (int)ftell(f);
    fseek(f, 0, SEEK_SET);
    if (!(buf = malloc(size)))
        goto err;
    if (fread(buf, size, 1, f) != 1) {
        fprintf(stderr, "Error reading %s\n", fn);
        goto err;
    }
    fclose(f);
    *pbuf = buf;
    *psize = size;
    return 0;
err:
    if (buf)
        free(buf);
    if (f)
        fclose(f);
    return -1;
}

char *find_id(char *buf, int size, int *idsize) {
    char *p, *pe;
    if ((p = memmem(buf, size, "$Id: ", 5)) &&
        (pe = memchr(p + 1, '$', buf + size - p))) {
        *idsize = pe - p + 1;
        return p;
    }
    return NULL;
}

int file_exist(const char *fn) {
    struct stat st;
    return stat(fn, &st) == 0 && S_ISREG(st.st_mode);
}

const char *dirs[] = {"main", "bios", "texmap", "2d", "3d", "cfile", "misc", "iff", "vga", "fix", "vecmat", "mem", "pa_null"};
#define N_DIRS (sizeof(dirs) / sizeof(dirs[0]))

int patch_id(char *fn, const char *id, int idsize) {
    char *srcbuf = NULL;
    int srcsize, srcidsize;
    char *srcid;

    if (file_read(fn, &srcbuf, &srcsize))
        goto err;
    srcid = find_id(srcbuf, srcsize, &srcidsize);
    if (!srcid) {
        printf("no id found in %s!\n", fn);
        goto err;
    }
    if (memchr(srcid, '\n', srcidsize)) {
        printf("bad id in %s!\n", fn);
    }
    if (idsize == srcidsize && memcmp(srcid, id, idsize) == 0) {
    } else {
        if (srcidsize < idsize) {
            int ofs = srcid - srcbuf;
            if (!(srcbuf = realloc(srcbuf, srcsize + idsize - srcidsize)))
                goto err;
            srcid = srcbuf + ofs;
            
        }
        memmove(srcid + idsize, srcid + srcidsize,
            srcbuf + srcsize - (srcid + srcidsize));
        memcpy(srcid, id, idsize);
        srcsize += idsize - srcidsize;
        //strcat(fn, ".new");
        if (file_write(fn, srcbuf, srcsize))
            goto err;
        printf("%s updated\n", fn);
    }
    free(srcbuf);
    return 0;
err:
    if (srcbuf)
        free(srcbuf);
    return -1;
}


int main(int argc, char **argv) {
    int exesize;
    char *exebuf, *p;
    int idsize;
    char srcfn[256];
    
    if (argc < 2) {
        fprintf(stderr, "missing source file argument\n");
        return 1;
    }
    
    if (file_read(argv[1], &exebuf, &exesize))
        goto err;
    p = exebuf;
    while ((p = find_id(p, exebuf + exesize - p, &idsize))) {
        const char *idfn;
        char *p2;
        int i;
    
        if (idsize < 5 || idsize > 256) {
            p += idsize;
            continue;
        }
        //printf("%.*s\n", idsize, p);
        idfn = p + 5;
        p2 = memchr(idfn, ' ', idsize - (idfn - p));
        if (!p2) {
            p += idsize;
            continue;
        }
        
        for (i = 0; i < N_DIRS; i++) {
            sprintf(srcfn, "source/%s/%.*s", dirs[i], (int)(p2 - idfn), idfn);
            if (file_exist(srcfn))
                break;
        }
        if (i == N_DIRS) {
            printf("%.*s not found!\n", (int)(p2 - idfn), idfn);
        } else {
            patch_id(srcfn, p, idsize); // ignore error
        }
        p += idsize;
    }
    free(exebuf);
    return 0;
err:
    return 1;
}
