/*
 * Several logical parts to arbitrary precision:
 * 1. store large values across multiple atomic storage units (e.g. 32-bit ints)
 * 2. convert to and from other useful representations e.g. strings
 * 3. perform operations on the stored large values
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "bbi.h"

bbi_chunk *_bbi_chunk_create() {
    bbi_chunk *ptr = malloc(sizeof(bbi_chunk));
    ptr->val = (unsigned int) 0;
    ptr->left = NULL;
    ptr->right = NULL;
    return ptr;
}

bbi_chunk *bbi_create() {
    return _bbi_chunk_create();
}

bbi_chunk *bbi_create_nchunks(unsigned int nchunks) {
    unsigned int diff;
    
    if (nchunks == 0 || nchunks == 1) {
        return bbi_create();
    }
    diff = nchunks - 1;
    bbi_chunk *ptr = _bbi_chunk_create();
    return bbi_extend(ptr, diff);
}

unsigned int _bbi_count_chunks(bbi_chunk *list) {
    unsigned int count = 0;
    
    while (list->left != NULL) {
        list = list->left;
    }
    count = 1;
    while (list->right != NULL) {
        list = list->right;
        count++;
    }
    return count;
}

/* Extend a list (always on the left) by nchunks */
bbi_chunk *bbi_extend(bbi_chunk *list, unsigned int nchunks) {
    unsigned int i;
    bbi_chunk *ptr;

    assert(nchunks > 0);    /* TODO assertion disabling */
    while (list->left != NULL) {
        list = list->left;
    }
    for (i = 0; i < nchunks; i++) {
        ptr = _bbi_chunk_create();
        list->left = ptr;
        ptr->right = list;
        list = ptr;
    }
    return list;
}

/* Pad lists a and b to be the same length, whichever is currently longest */
void bbi_pad(bbi_chunk *list_a, bbi_chunk *list_b) {
    unsigned int list_a_len;
    unsigned int list_b_len;
    unsigned int diff;
    
    list_a_len = _bbi_count_chunks(list_a);
    list_b_len = _bbi_count_chunks(list_b);
    if (list_a_len == list_b_len) {
        return;
    }
    if (list_a_len < list_b_len) {
        diff = list_b_len - list_a_len;
        bbi_extend(list_a, diff);
    }
    if (list_b_len < list_a_len) {
        diff = list_a_len - list_b_len;
        bbi_extend(list_b, diff);
    }
}

bbi_chunk *bbi_copy(bbi_chunk *list) {
    unsigned int listlen = _bbi_count_chunks(list);
    bbi_chunk *newlist = bbi_create_nchunks(listlen);
    /* Walk both pointers to least significant bit */
    while (list->right != NULL) {
        list = list->right;
    }
    while (newlist->right != NULL) {
        newlist = newlist->right;
    }
    while (list->left != NULL) {
        newlist->val = list->val;
        list = list->left;
        newlist = newlist->left;
    }
    /* Copy final value */
    newlist->val = list->val;
    return newlist;
}

/* Load a value from a string, in decimal - caller must bbi_destory()! */
bbi_chunk *bbi_fromstring_dec(const unsigned char *s) {
    size_t slen = strlen(s); 
    unsigned int sidx;
    unsigned int curval;
    
    bbi_chunk *list = bbi_create();
    sidx = slen - 1;
    curval = 0;

    do {
        curval *= 10;
        curval += (s[sidx] - '0'); 
        sidx--;
    } while (sidx > 0);

    return list;
}

/* Convert a value to a string represenation in binary - caller must handle memory */
void _bbi_dump_binary_val(unsigned char *buf, unsigned int val) {
    size_t uint_size = sizeof(unsigned int);
    unsigned int mask = 1 << (8*uint_size-1); // 8-bit bytes
    int digitcount = 0;
    while (mask) {
        if (val & mask) {
            *buf = '1';
        } else {
            *buf = '0';
        }
        digitcount++;
        buf++;
        if (digitcount % 8 == 0 && digitcount != 8*sizeof(unsigned int)) {
            *buf = ' ';
            buf++;
        }
        mask >>= 1;
    }
    *buf = '\0';
}

/* Walk chunks from most-significant bit to least, outputing one per line */
void bbi_dump_binary(bbi_chunk *list) {
    unsigned char buf[sizeof(unsigned int)*8+3+1];  /* TODO assumes 32-bit unsigned int */
    unsigned int chunknum = 0;
    while (list->left != NULL) {
        list = list->left;
    }
    while (list->right != NULL) {
        _bbi_dump_binary_val(buf, list->val);
        printf("%03d: %s (%p)\n", chunknum, buf, list); 
        chunknum++;
        list = list->right;
    } 
    /* Handle single chunk */
    _bbi_dump_binary_val(buf, list->val);
    printf("%03d: %s (%p)\n", chunknum, buf, list); 

    putchar('\n');
}

void bbi_destroy(bbi_chunk *list) {
    bbi_chunk *to_free;
    
    /* walk to MSB end of list */
    while (list->left != NULL) {
        list = list->left;
    }
    while (list->right != NULL) {
        to_free = list;
        list = list->right;
        assert(to_free != NULL);
        free(to_free);
        to_free = NULL;
    }
    assert(list != NULL);
    free(list);
    list = NULL;
}

void bbi_and(bbi_chunk *list_a, bbi_chunk *list_b) {
    
}

/*
int main() {
    bbi_chunk *list = bbi_create();
    bbi_extend(list, 9);
    while (list->right != NULL) {
        list = list->right;
    }
    list = list->left;
    list->val = 4000000000;
    bbi_dump_binary(list);
    bbi_destroy(list);
}
*/
