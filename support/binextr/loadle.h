#ifndef LOADLE_H_
#define LOADLE_H_
typedef struct reloc reloc_t;
void *loadle(const char *filename, void **segptrs, reloc_t **preloc);
int reloc_find(reloc_t *reloc, int src);
#endif
