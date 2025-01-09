
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "pointer_list.h"

pointer_list_t* create_pointer_list(void) {

    pointer_list_t* ptr = _ALLOC_DS(pointer_list_t);
    assert(ptr != NULL);
    ptr->cap = 1 << 3;
    ptr->len = 0;
    ptr->list = _ALLOC_ARRAY(void*, ptr->cap);
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

    if(lst->len + 1 > lst->cap) {
        lst->cap <<= 1;
        lst->list = _REALLOC_ARRAY(lst->list, void*, lst->cap);
        assert(lst->list != NULL);
    }

    lst->list[lst->len] = ptr;
    lst->len++;
}

int len_pointer_list(pointer_list_t* lst) {

    return lst->len;
}

void* index_pointer_list(pointer_list_t* lst, int idx) {

    assert(lst != NULL);
    if((idx >= 0) && (idx < lst->len))
        return lst->list[idx];
    else
        return NULL;
}

void* iterate_pointer_list(pointer_list_t* lst, int* mark) {

    assert(lst != NULL);
    assert(mark != NULL);

    void* ptr = NULL;

    if((*mark >= 0) && (*mark < lst->len)) {
        ptr = lst->list[*mark];
        *mark = *mark + 1;
    }

    return ptr;
}
