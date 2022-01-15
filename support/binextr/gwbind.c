#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include "bindata.h"

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

#define EXE_HDR_SIZE 64
#define BW_HDR_SIZE 176

int find_gw_size(FILE *f) {
    char buf[256];
    int exesize, ofs;

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
                perror("cannot read bw hdr");
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
    return ofs;
err:
    return -1;
}

int main(int argc, char **argv) {
    int exesize, gwsize;
    char *exebuf, *gwbuf;
    const char *filename, *exefilename;
    FILE *f = NULL, *fo = NULL;
    
    if (argc < 3) {
        fprintf(stderr, "syntax: already-bound.exe to-be-bound.exe\n");
        return 1;
    }

    filename = argv[1];
    exefilename = argv[2];

    if (!(f = fopen(exefilename, "rb"))) {
        fprintf(stderr, "cannot open %s: %s\n", exefilename, strerror(errno));
        goto err;
    }
    gwsize = find_gw_size(f);
    fclose(f);
    f = NULL;
    if (gwsize == -1)
        goto err;
    if (gwsize != 0) {
        fprintf(stderr, "%s already has a bound extender\n");
        goto err;
    }

    if (!(f = fopen(filename, "rb"))) {
        fprintf(stderr, "cannot open %s: %s\n", filename, strerror(errno));
        goto err;
    }

    gwsize = find_gw_size(f);
    if (gwsize == -1)
        goto err;
    if (gwsize == 0) {
        fprintf(stderr, "no bound extender found in %s\n", filename);
        goto err;
    }
    
    if (!(gwbuf = malloc(gwsize))) {
        fprintf(stderr, "%s\n", strerror(ENOMEM));
        goto err;
    }
    fseek(f, 0, SEEK_SET);
    if ((fread(gwbuf, 1, gwsize, f) < gwsize)) {
        fprintf(stderr, "error reading %s\n", filename);
        goto err;
    }
    fclose(f);
    f = NULL;
    
    exefilename = argv[2];
    if (file_read(exefilename, &exebuf, &exesize))
        goto err;
    
    if (!(fo = fopen(exefilename, "wb"))) {
        fprintf(stderr, "creating %s: %s\n", exefilename, strerror(errno));
        goto err;
    }
    
    if ((fwrite(gwbuf, 1, gwsize, fo) < gwsize)) {
        fprintf(stderr, "writing %s failed\n", exefilename);
        goto err;
    }

    if ((fwrite(exebuf, 1, exesize, fo) < exesize)) {
        fprintf(stderr, "writing %s failed\n", exefilename);
        goto err;
    }
    
    fclose(fo);
    free(exebuf);
    free(gwbuf);
    return 0;
err:
    return 1;
}
