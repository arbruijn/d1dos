typedef struct binmap binmap_t;

binmap_t *binmap_load(const char *filename);
void binmap_free(binmap_t *map);
char *binmap_find_ofs(binmap_t *map, int ofs);
char *binmap_find_ofs_leq(binmap_t *binmap, int ofs, int *nofs);
char *binmap_find_ofs_geq(binmap_t *binmap, int ofs, int *nofs);
int binmap_find_name(binmap_t *map, const char *name);
binmap_t *binmap_create();
void binmap_add(binmap_t *binmap, int ofs, const char *name);
void binmap_clear(binmap_t *binmap);
