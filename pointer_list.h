
#ifndef _POINTER_LIST_H_
#define _POINTER_LIST_H_

typedef struct {
    void** list;
    int cap;
    int len;
} pointer_list_t;

pointer_list_t* create_pointer_list();
void destroy_pointer_list(pointer_list_t*);
void add_pointer_list(pointer_list_t*, void*);
int len_pointer_list(pointer_list_t*);
void* index_pointer_list(pointer_list_t*, int);
void* iterate_pointer_list(pointer_list_t*, int*);

#endif /* _POINTER_LIST_H_ */
