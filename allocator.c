#include<stdio.h>
#include<unistd.h>
#include<assert.h>
#include<stdbool.h>
#include<string.h>

struct block_head {
    size_t size;
    bool free;
    struct block_head *next;
};

static struct block_head *base = NULL;

size_t META_SIZE = sizeof(struct block_head);

void *cus_malloc(size_t requested_size);
void cus_free(void *ptr); 
void *cus_realloc(void *ptr, size_t requested_size); 
void *cus_calloc(size_t count, size_t requested_size); 
struct block_head *get_head_block(void *ptr);

int main() {
    int *ptr1 = cus_malloc(100);
    ptr1[0] = 69;
    printf("ptr first vlaue: %d\n", ptr1[0]);
    printf("ptr1 size: %zu\n", get_head_block(ptr1)->size);
    assert(get_head_block(ptr1) == (void *)base);

    void *ptr2 = cus_malloc(200);
    printf("ptr2 size: %zu\n", get_head_block(ptr2)->size);
    printf("ptr2 is free?: %d\n", get_head_block(ptr2)->free);
    printf("ptr2 address: %p\n", ptr2);
    cus_free(ptr2);
    
    void *ptr3 = cus_malloc(150);
    printf("ptr3 is free?: %d\n", get_head_block(ptr3)->free);
    printf("ptr3 address: %p\n", ptr3);
	cus_free(ptr3);

    int *ptr4 = cus_realloc(ptr1, 150);
    printf("ptr4 is free?: %d\n", get_head_block(ptr4)->free);
    printf("ptr4 after reallocation size: %zu\n", get_head_block(ptr4)->size);
    printf("ptr4 address: %p\n", ptr4);
    printf("ptr first vlaue: %d\n", ptr4[0]);
    
    int *ptr5 = cus_calloc(5, sizeof(int));
    printf("ptr5 value: %d\n", ptr5[0]);

}

void *cus_malloc(size_t requested_size) {
    
    if (base == NULL) {
	    void *current_heap_top = sbrk(0);
        base = current_heap_top;        
        void *ptr = sbrk(requested_size + META_SIZE);        

        if (ptr == ((void *)-1)) {
            return NULL;
        }

        base->next = NULL;
        base->size = requested_size;
        base->free = false;
        return (void *) (base + 1);
    } else {
        struct block_head *curr = base;
        // search for free space from already allocated memory
        while(curr != NULL) {
            if (curr->size >= requested_size && curr->free == true) {
                curr->size = requested_size;
                curr->free = false;
                return (void *) (curr + 1);
            }
            curr = curr->next;
        }
        
        // couldn't find space in the list, so allocated at the end of the list
	    void *current_heap_top = sbrk(0);
        struct block_head *last_node = current_heap_top;
        void *ptr = sbrk(requested_size + META_SIZE);        

        if (ptr == ((void *)-1)) {
            return NULL;
        }

        struct block_head *current = base;
        while(current->next != NULL) {
            current = current->next;
        }
        
        current->next = last_node; 

        last_node->size = requested_size;
        last_node->free = 0;
        last_node->next = NULL;
        return (void *) (last_node + 1);
    }
} 

void cus_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    struct block_head *freptr = ((struct block_head *)ptr)-1;
    freptr->free = true;
}

struct block_head *get_head_block(void *ptr) { 
    return ((struct block_head *)ptr)-1;
}

void *cus_realloc(void *ptr, size_t requested_size) {
    if (ptr == NULL || base == NULL) {
        return NULL;
    }

    if (requested_size == 0) {
   	    cus_free(ptr);
        return NULL;	
    }

	size_t old_size = get_head_block(ptr)->size;
    if (old_size >= requested_size) {
        return ptr;
    }
    
    struct block_head *previous, *current;
    previous = base;
    current = previous;
    bool found = false;
    
    while (current != NULL) {
        if (current->size >= requested_size && current->free) {
            found = true;
            break;
        }

        previous = current;
        current = current->next;
    }

    if (!found) {
        struct block_head *new_ptr = sbrk(requested_size + META_SIZE);
        
        if (new_ptr == ((void *)-1)) {
            return NULL;
        }

        current = new_ptr;
        current->next = NULL;
        previous->next = current;
    }
    
    size_t copied_size = old_size < requested_size ? old_size : requested_size;
    void *reallocated_ptr = ((struct block_head*)current)+1;
    memcpy(reallocated_ptr, ptr, copied_size);

    current->free = false;
    current->size = requested_size;

    cus_free(ptr);

    return (void *)reallocated_ptr;
} 

void *cus_calloc(size_t count, size_t requested_size) {
    size_t size = count * requested_size;

    void *ptr = cus_malloc(size);

    if (ptr == NULL) {
        return NULL;
    }

    memset(ptr, 0, size);

}
