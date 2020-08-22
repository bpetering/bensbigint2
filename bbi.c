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
    ptr->val = 0;       /* TODO C rules for assigning a literal to a type */
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

/* TODO optimization: chunk lists store their current length */
unsigned int _bbi_count_chunks(bbi_chunk *list) {
    unsigned int count = 0;
   
    list = _find_left(list);
    count = 1;
    while (list->right != NULL) {
        list = list->right;
        count++;
    }
    return count;
}

/* 
 * Following the principle "be conservative in what you send, and liberal in what you accept",
 * functions take any chunk in a list to operate on the chunk list, but always return the 
 * least-significant-bit chunk 
 */

/* Get the leftmost chunk (Most-Significant Bit) of a chunk list */
bbi_chunk *_find_left(bbi_chunk *list) {
    while (list->left != NULL) {
        list = list->left;
    }
    return list;
}

/* Get the rightmost chunk (Least-Significant Bit) of a chunk list */
bbi_chunk *_find_right(bbi_chunk *list) {
    while (list->right != NULL) {
        list = list->right;
    }
    return list;
}

/* Extend a list (always on the left) by nchunks */
bbi_chunk *bbi_extend(bbi_chunk *list, unsigned int nchunks) {
    unsigned int i;
    bbi_chunk *ptr;

    assert(nchunks > 0);    /* TODO assertion disabling */
    list = _find_left(list);
    for (i = 0; i < nchunks; i++) {
        ptr = _bbi_chunk_create();
        list->left = ptr;
        ptr->right = list;
        list = ptr;
    }
    /* Always return rightmost (Least-Significant Bit) chunk as pointer */
    return _find_right(list);
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

    list = _find_right(list);
    while (list->left != NULL) {
        newlist->val = list->val;
        list = list->left;
        newlist = newlist->left;
    }
    /* Copy final value */
    newlist->val = list->val;
    return _find_right(newlist);
}

/* Load a value from a string, in decimal - caller must bbi_destory()! */
/* More usefully, a general-case function to handle at least binary, octal, decimal and hex
   is a good idea, but this is a starting point */
/*
 * Converting a string representation of an integer to an actual value is trivial in any base. 
   The general algorithm is:
   - initialise the value variable to 0
   - start at the right-most digit, and walk to the left-most digit one digit at a time, for each digit
        - multiply the value variable by the base (e.g. 10)
        - add the value of the digit to the value variable
   
   The challenge is to do this across chunks - because we can't just use one "value variable",
   we have to use many - and we don't know how many we need in advance.


*/
bbi_chunk *bbi_fromstring_dec(const unsigned char *s) {
    size_t slen = strlen(s); 
    unsigned int sidx;
    unsigned int curval;
    unsigned int oldcurval;
    
    bbi_chunk *list = bbi_create();
    sidx = slen - 1;
    curval = 0;

    do {
        oldcurval = curval;
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

unsigned int max(unsigned int a, unsigned int b) {
    if (a > b) {
        return a;
    }
    else {
        return b;
    }
}

void bbi_destroy(bbi_chunk *list) {
    bbi_chunk *to_free;
    
    list = _find_left(list);
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

/* Bitwise operations have two functions for each operation - an in-place operation, and one
   that produces a new bigint, Binary in-place operations store the result in the
   first operand */

/* Bitwise NOT a value */
bbi_chunk *bbi_not_inplace(bbi_chunk *list) {
    bbi_chunk *list_right;
    list = list_right = _find_right(list);
    while (list->left != NULL) {
        list->val = ~ list->val;
        list = list->left;
    }
    list->val = ~ list->val;
    return list_right;
}

bbi_chunk *bbi_not(bbi_chunk *list) {
    bbi_chunk *result = bbi_copy(list);
    return bbi_not_inplace(result);
}

/* Bitwise AND two values. If one chunk list is longer than the other, the missing values are implicitly 
   all 0 bits. In-place version stores result in first operand, and extends to length of second operand
   if it's smaller. */
bbi_chunk *bbi_and_inplace(bbi_chunk *list_a, bbi_chunk *list_b) {
    unsigned int len_a;
    unsigned int len_b;
    unsigned int result_len;

    len_a = _bbi_count_chunks(list_a);
    len_b = _bbi_count_chunks(list_b);
    result_len = max(len_a, len_b);
    if (result_len > len_a) {
        bbi_extend(list_a, result_len - len_a);
    }
    assert(_bbi_count_chunks(list_a) == _bbi_count_chunks(list_b));

    list_a = _find_right(list_a);
    list_b = _find_right(list_b);
    while (list_a->left != NULL && list_b->left != NULL) {
        list_a->val &= list_b->val;
        list_a = list_a->left;
        list_b = list_b->left;
    }
    /* Two cases to consider: 1) lists have equal length, then handle the final & op. 2) lists have 
       unequal length, then set abs(len_a - len_b) values to 0, since 0 & anything == 0. */
    if (len_a == len_b) {
        list_a->val &= list_b->val;
    }
    else {
        while (list_a->left != NULL) {
            list_a->val = 0;
            list_a = list_a->left;
        }
        list_a->val = 0;
    }
    return _find_right(list_a);
}

bbi_chunk *bbi_and(bbi_chunk *list_a, bbi_chunk *list_b) {
    bbi_chunk *result = bbi_copy(list_a);
    return bbi_and_inplace(result, list_b);
}

/* Bitwise OR two values, calling semantics as bbi_and_inplace(). */
bbi_chunk *bbi_or_inplace(bbi_chunk *list_a, bbi_chunk *list_b) {
    unsigned int len_a;
    unsigned int len_b;
    unsigned int result_len;

    len_a = _bbi_count_chunks(list_a);
    len_b = _bbi_count_chunks(list_b);
    result_len = max(len_a, len_b);
    if (result_len > len_a) {
        bbi_extend(list_a, result_len - len_a);
    }

}

bbi_chunk *bbi_or(bbi_chunk *list_a, bbi_chunk *list_b) {
    bbi_chunk *result = bbi_copy(list_a);
    return bbi_or_inplace(result, list_b);
}

bbi_chunk *bbi_xor_inplace(bbi_chunk *list_a, bbi_chunk *list_b) {

}

bbi_chunk *bbi_xor(bbi_chunk *list_a, bbi_chunk *list_b) {
    bbi_chunk *result = bbi_copy(list_a);
    return bbi_xor_inplace(result, list_b);
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
