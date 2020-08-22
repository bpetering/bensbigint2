#ifndef BBI_H
#define BBI_H

/* TODO optimization, maybe, after benchmarking: store chunks in a structure with better cache locality - 
   contiguously allocated like an array, with sizing chosen to only reallocate, on average, a small
   percentage of the time (see Java HashMap for example of efficient realloc) */
struct bbi_chunk {
    unsigned int val;
    struct bbi_chunk *left;
    struct bbi_chunk *right;
};
typedef struct bbi_chunk bbi_chunk;

/* Chunk list management functions */
bbi_chunk *_bbi_chunk_create();
bbi_chunk *bbi_create();
bbi_chunk *bbi_create_nchunks(unsigned int nchunks);
unsigned int _bbi_count_chunks(bbi_chunk *list);
bbi_chunk *bbi_extend(bbi_chunk *list, unsigned int nchunks);
void bbi_pad(bbi_chunk *list_a, bbi_chunk *list_b);
bbi_chunk *bbi_copy(bbi_chunk *list);
void bbi_destroy(bbi_chunk *list);

/* Loading values */
bbi_chunk *bbi_fromstring_dec(const unsigned char *s);

/* Bitwise operations */
bbi_chunk *bbi_not(bbi_chunk *list);
bbi_chunk *bbi_not_inplace(bbi_chunk *list);

bbi_chunk *bbi_and(bbi_chunk *list_a, bbi_chunk *list_b);
bbi_chunk *bbi_and_inplace(bbi_chunk *list_a, bbi_chunk *list_b);

/* Helper */
void _bbi_dump_binary_val(unsigned char *buf, unsigned int val);
void bbi_dump_binary(bbi_chunk *list);
bbi_chunk *_find_left(bbi_chunk *list);
bbi_chunk *_find_right(bbi_chunk *list);

#endif

