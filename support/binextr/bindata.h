static int get16(void *d, int ofs) {
    unsigned char *p = d;
    return ((unsigned char)p[ofs]) | (((unsigned char)p[ofs + 1]) << 8);
}

static int get32(void *d, int ofs) {
    unsigned char *p = d;
    return ((unsigned char)p[ofs]) | (((unsigned char)p[ofs + 1]) << 8) |
        (((unsigned char)p[ofs + 2]) << 16) | (((unsigned char)p[ofs + 3]) << 24);
}

static void set16(void *d, int ofs, int val) {
    unsigned char *p = d;
    p[ofs] = val;
    p[ofs + 1] = val >> 8;
}

static void set32(void *d, int ofs, int val) {
    unsigned char *p = d;
    p[ofs] = val;
    p[ofs + 1] = val >> 8;
    p[ofs + 2] = val >> 16;
    p[ofs + 3] = val >> 24;
}
