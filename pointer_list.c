
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "pointer_list.h"

pointer_list_t* create_pointer_list() {

    pointer_list_t* ptr = malloc(sizeof(pointer_list_t));
    assert(ptr != NULL);
    ptr->cap = 1 << 3;
    ptr->len = 0;
    ptr->list = (void**)malloc(sizeof(void*) * ptr->cap);
    assert(ptr->list != NULL);

    return ptr;
}

void destroy_pointer_list(pointer_list_t* lst) {

    if(lst != NULL) {
        if(lst->list != NULL)
            free(lst->list);
        free(lst);
    }
}

void add_pointer_list(pointer_list_t* lst, void* ptr) {

    assert(lst != NULL);
    assert(ptr != NULL);

    if(lst->len+1 < lst->cap) {
        lst->cap <<= 1;
        lst->list = realloc(lst->list, sizeof(void*) * lst->cap);
        assert(lst->list != NULL);
    }

    lst->list[lst->len] = ptr;
    lst->len++;
}

void* index_pointer_list(pointer_list_t* lst, int idx) {

    assert(lst != NULL);
    assert((idx >= 0) && (idx < lst->len));

    return lst->list[idx];
}

void* iterate_pointer_list(pointer_list_t* lst, int* mark) {

    assert(lst != NULL);
    assert(mark != NULL);

    void* ptr = NULL;

    if((*mark >= 0) && (*mark < lst->len)) {
        ptr = lst->list[*mark++];
    }

    return ptr;
}
