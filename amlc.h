//Hello from the Aspen Software Foundation!

   //  _    ____  ____  _____ _   _ 
   // / \  / ___||  _ \| ____| \ | |
  // /   \ \___ \| |_) |  _| |  \| |
 // /     \ ___) |  __/| |___| |\  |
// /       \____/|_|   |_____|_| \_|

#ifndef AMLC_H
#define AMLC_H

#include <stdio.h>
#include <stdlib.h>

typedef struct MemBlock {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    struct MemBlock* next;
} MemBlock;

static MemBlock* __amlc_head = NULL;

static void __amlc_add_block(void* ptr, size_t size, const char* file, int line) {
    MemBlock* block = malloc(sizeof(MemBlock));
    block->ptr = ptr;
    block->size = size;
    block->file = file;
    block->line = line;
    block->next = __amlc_head;
    __amlc_head = block;
}

static void __amlc_remove_block(void* ptr) {
    MemBlock** curr = &__amlc_head;
    while (*curr) {
        if ((*curr)->ptr == ptr) {
            MemBlock* temp = *curr;
            *curr = temp->next;
            free(temp);
            return;
        }
        curr = &(*curr)->next;
    }
}

static void* __amlc_tracked_malloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (ptr) {
        __amlc_add_block(ptr, size, file, line);
    }
    return ptr;
}

static void __amlc_tracked_free(void* ptr) {
    __amlc_remove_block(ptr);
    free(ptr);
}

static void _CrtDumpMemoryLeaks() {
    if (__amlc_head == NULL) {
        printf("No memory leaks detected.\n");
    } else {
        printf("Memory leaks detected:\n");
        MemBlock* curr = __amlc_head;
        while (curr) {
            printf(" Leak: %zu bytes at %p (allocated at %s:%d)\n",
                   curr->size, curr->ptr, curr->file, curr->line);
            curr = curr->next;
        }
    }
}

// Macros to override malloc/free
#define malloc(x) __amlc_tracked_malloc(x, __FILE__, __LINE__)
#define free(x) __amlc_tracked_free(x)

#endif // AMLC_H

