#include <criterion/criterion.h>
#include <stdio.h>
#include <string.h>
#include "bbi.h"

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
    while (list->left != NULL) {
        list = list->left;
    }
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

/*
Test(bbi_structures, list_copy) {
    bbi_chunk *list = bbi_create_nchunks(10);
    bbi_chunk *listcopy;
    int i = 0;

    cr_assert(_bbi_count_chunks(list) == 10);
    while (list->right != NULL) {
        list = list->right;
    }
    for (i = 0; i < 10 && list->left != NULL; i++) {
        list->val = i;
        list = list->left;
    }
    printf("hello\n");
    listcopy = bbi_copy(list);
    cr_assert(_bbi_count_chunks(list) == 10);
    while (listcopy->right != NULL) {
        listcopy = listcopy->right;
    }
    for (i = 0; i < 10; i++) {
        cr_assert(listcopy->val == i);
    }
}
*/

/* Storage and retrieval */

/* Helper */
Test(bbi_helper, dump_binary) {
    unsigned n = sizeof(unsigned int)*8+3+1;
    char buf[sizeof(unsigned int)*8+3+1];   /* 3 spaces, 1 null terminator */
    unsigned int val;

    val = 0;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000000 00000000") == 0); /* TODO assumes 32-bit uint */

    val = 1;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000000 00000001") == 0);
    
    val = 2;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000000 00000010") == 0);

    val = 17;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000000 00010001") == 0);

    val = 255;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000000 11111111") == 0);
    
    val = 256;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000001 00000000") == 0);

    val = 266;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000001 00001010") == 0);

    val = 266;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "00000000 00000000 00000001 00001010") == 0);

    val = 4194304;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "00000000 01000000 00000000 00000000") == 0);

    val = 16711680;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "00000000 11111111 00000000 00000000") == 0);

    val = 4278190080;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "11111111 00000000 00000000 00000000") == 0);
    
    val = 4278190339;
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "11111111 00000000 00000001 00000011") == 0);

    val = 4294967295;   /* TODO some places not safe */
    _bbi_dump_binary(buf, val);
    cr_assert(strcmp(buf, "11111111 11111111 11111111 11111111") == 0);
} 

