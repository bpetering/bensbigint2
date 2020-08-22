#include <criterion/criterion.h>
#include <stdio.h>
#include <string.h>
#include "bbi.h"

/* 
   This is testing implementation, not interface,
   but it's useful for bootstrapping to get our
   implementation working and can be removed later
*/
Test(bbi_structures, list_len_one) {
    bbi_chunk *list = bbi_create();
    list->val = 0;
    cr_assert(list->val == 0);
    cr_assert(_bbi_count_chunks(list) == 1);
    list->val = 1000000;
    cr_assert(list->val == 1000000);
    bbi_destroy(list);
}

Test(bbi_structures, list_len_two_extend) {
    bbi_chunk *list = bbi_create();
    cr_assert(list->val == 0);
    cr_assert(_bbi_count_chunks(list) == 1);
    bbi_extend(list, 1);
    cr_assert(_bbi_count_chunks(list) == 2);
    list = list->left;
    cr_assert(list->val == 0);
    bbi_destroy(list);
}

Test(bbi_structures, list_len_1000_extend) {
    int i;
    bbi_chunk *list = bbi_create();
    cr_assert(list->val == 0);
    cr_assert(_bbi_count_chunks(list) == 1);
    bbi_extend(list, 999);
    cr_assert(_bbi_count_chunks(list) == 1000);
    list = _find_left(list);
    list->val = 10000;
    i = 0;
    while (list->right != NULL && i < 100) {
        list = list->right;
        i++;
    }
    list->val = 12345;
    bbi_destroy(list);
}

Test(bbi_structures, list_len_one_create) {
    bbi_chunk *list = bbi_create_nchunks(1);
    cr_assert(_bbi_count_chunks(list) == 1);
    bbi_extend(list, 20);
    cr_assert(_bbi_count_chunks(list) == 21);
    bbi_destroy(list);
}

Test(bbi_structures, list_len_two_create) {
    bbi_chunk *list = bbi_create_nchunks(2);
    cr_assert(_bbi_count_chunks(list) == 2);
    bbi_destroy(list);
}

Test(bbi_structures, list_len_1000_create) {
    bbi_chunk *list = bbi_create_nchunks(1000);
    cr_assert(_bbi_count_chunks(list) == 1000);
    bbi_destroy(list);
}

Test(bbi_structures, list_pad1) {
    bbi_chunk *list_a = bbi_create_nchunks(3);
    bbi_chunk *list_b = bbi_create();
    cr_assert(_bbi_count_chunks(list_a) == 3);
    cr_assert(_bbi_count_chunks(list_b) == 1);
    bbi_pad(list_a, list_b);
    cr_assert(_bbi_count_chunks(list_a) == 3);
    cr_assert(_bbi_count_chunks(list_b) == 3);
}

Test(bbi_structures, list_pad2) {
    bbi_chunk *list_a = bbi_create();
    bbi_chunk *list_b = bbi_create();
    bbi_extend(list_a, 15);
    cr_assert(_bbi_count_chunks(list_a) == 16);
    cr_assert(_bbi_count_chunks(list_b) == 1);
    bbi_pad(list_a, list_b);
    cr_assert(_bbi_count_chunks(list_a) == 16);
    cr_assert(_bbi_count_chunks(list_b) == 16);
    bbi_extend(list_b, 20);
    cr_assert(_bbi_count_chunks(list_a) == 16);
    cr_assert(_bbi_count_chunks(list_b) == 36);
    bbi_pad(list_a, list_b);
    cr_assert(_bbi_count_chunks(list_a) == 36);
    cr_assert(_bbi_count_chunks(list_b) == 36);
}

Test(bbi_structures, list_copy) {
    bbi_chunk *list = bbi_create_nchunks(10);
    bbi_chunk *listcopy;
    int i = 0;

    cr_assert(_bbi_count_chunks(list) == 10);
    /* Create some values to copy - 0-9, going towards more-significant bits, 1 per chunk */
    while (list->left != NULL) {
        list->val = i;
        i++;
        list = list->left;
    }
    list->val = i;
    /*
    printf("list:\n");
    bbi_dump_binary(list);
    */

    listcopy = bbi_copy(list);
    /*
    printf("list copy:\n");
    bbi_dump_binary(listcopy);
    */
    cr_assert(_bbi_count_chunks(list) == 10);
    i = 0;
    while (listcopy->left != NULL) {
        cr_assert(listcopy->val == i);
        i++;
        listcopy = listcopy->left;
    }
}

/* Storage and retrieval */
/*
Test(bbi_storage, load_dec_string_0) {
    bbi_chunk *new = bbi_fromstring_dec("0");
    while (new->right != NULL) {
        new = new->right;
    }   
    cr_assert(new->val == 0);
    bbi_destroy(new);
}

Test(bbi_storage, load_dec_string_16bits) {
    bbi_chunk *new = bbi_fromstring_dec("12345");
    while (new->right != NULL) {
        new = new->right;
    }
    cr_assert(new->val == 12345);
    bbi_destroy(new);
}
*/

/* Test either side of 32- and 64-bit boundaries, and bigger higher boundaries */
/*
Test(bbi_storage, load_dec_string_32bits) {
    bbi_chunk *new = bbi_fromstring_dec("4294967295");
    while (new->right != NULL) {
        new = new->right;
    }
    cr_assert(new->val == 4294967295);
    bbi_destroy(new);
}

Test(bbi_storage, load_dec_string_32bitsplusone) {
    bbi_chunk *new = bbi_fromstring_dec("4294967296");
    while (new->right != NULL) {
        new = new->right;
    }
    cr_assert(new->val == 0);
    cr_assert(new->left != NULL);
    cr_assert(new->left->val == 1);
    bbi_destroy(new);
}
*/

/* Bitwise operations */
Test(bbi_bitwise, not_inplace_copy_1chunk) {
    bbi_chunk *list = bbi_create();
    list->val = 100;
    bbi_not_inplace(list);
    cr_assert(list->val == ~ (unsigned int) 100);
}

Test(bbi_bitwise, not_inplace_manychunks) {
    bbi_chunk *list = bbi_create_nchunks(50);
    unsigned int i;
    unsigned int tmp = 1;
    while (list->left != NULL) {
        list->val = tmp;
        list = list->left;
        tmp++;
    }
    list->val = tmp;

    list = bbi_not_inplace(list);
    for (i = 1; i <= 50; i++) {
        cr_assert(list->val = ~i);
        if (list->left != NULL) {
            list = list->left;
        }
    }
    bbi_destroy(list);
}

Test(bbi_bitwise, not_copy_1chunk) {
    bbi_chunk *list = bbi_create();
    bbi_chunk *result;
    list->val = 10;
    result = bbi_not(list);
    cr_assert(result != list);
    cr_assert(result->val == ~ list->val);
    bbi_destroy(list);
    bbi_destroy(result);
}

Test(bbi_bitwise, not_copy_manychunks) {
    bbi_chunk *list = bbi_create_nchunks(50);
    bbi_chunk *result;
    unsigned int i;
    unsigned int tmp = 1;
    while (list->left != NULL) {
        list->val = tmp;
        list = list->left;
        tmp++;
    }
    list->val = tmp;
    result = bbi_not(list);
    for (i = 1; i <= 50; i++) {
        cr_assert(result->val = ~i);
        if (result->left != NULL) {
            result = result->left;
        }
    }
    bbi_destroy(result);
    bbi_destroy(list);
}

/*
Test(bbi_bitwise, and) {
    bbi_chunk *list_a = bbi_create();
    bbi_chunk *list_b = bbi_create_nchunks(2);
    bbi_chunk *result;
    
    list_a->val = 0;
    list_b->val = 0;
    result = bbi_and(list_a, list_b);
    cr_assert(result->val == 0);
    cr_assert(_bbi_count_chunks(result) == 2);
    cr_assert(result->left->val = 0);
}
*/
/* Helper */
Test(bbi_helper, dump_binary) {
    unsigned n = sizeof(unsigned int)*8+3+1;
    char buf[sizeof(unsigned int)*8+3+1];   /* 3 spaces TODO assumes 32bit uint, 1 null terminator */
    unsigned int val;

    val = 0;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000000 00000000") == 0); /* TODO assumes 32-bit uint */

    val = 1;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000000 00000001") == 0);
    
    val = 2;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000000 00000010") == 0);

    val = 17;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000000 00010001") == 0);

    val = 255;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000000 11111111") == 0);
    
    val = 256;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000001 00000000") == 0);

    val = 266;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000001 00001010") == 0);

    val = 266;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000001 00001010") == 0);

    val = 4194304;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "00000000 01000000 00000000 00000000") == 0);

    val = 16711680;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "00000000 11111111 00000000 00000000") == 0);

    val = 4278190080;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "11111111 00000000 00000000 00000000") == 0);
    
    val = 4278190339;
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "11111111 00000000 00000001 00000011") == 0);

    val = 4294967295;   /* TODO some places not safe */
    _bbi_dump_binary_val(buf, val);
    cr_assert(strcmp(buf, "11111111 11111111 11111111 11111111") == 0);
} 

