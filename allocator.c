#include<stdio.h>
#include<unistd.h>
#include<assert.h>
#include<stdbool.h>

struct block_head {
    size_t size;
    bool free;
    struct block_head *next;
};

static struct block_head *base = NULL;

size_t META_SIZE = sizeof(struct block_head);

void *cus_malloc(size_t requested_size);
void cus_free(void *ptr); 
struct block_head *get_head_block(void *ptr);

int main() {
    void *ptr1 = cus_malloc(100);
    printf("%zu\n", get_head_block(ptr1)->size);
    assert(get_head_block(ptr1) == (void *)base);

    void *ptr2 = cus_malloc(200);
    printf("%zu\n", get_head_block(ptr2)->size);
    printf("%d\n", get_head_block(ptr2)->free);
    printf("%p\n", ptr2);
    cus_free(ptr2);
    
    void *ptr3 = cus_malloc(200);
    printf("%d\n", get_head_block(ptr3)->free);
    printf("%p\n", ptr3);
	cus_free(ptr3);

    void *ptr4 = cus_malloc(100);
    printf("%d\n", get_head_block(ptr4)->free);
    printf("%p\n", ptr4);

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
