// Hello from the Aspen Software Foundation!
// This file is part of the Aspen Memory Leak Checker (AMLC).

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
    MemBlock* block = (MemBlock*)malloc(sizeof(MemBlock));
    if (!block) {
        printf("Memory allocation failed for tracking memory block!\n");
        return;
    }
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
#define AMLC_MALLOC(size) __amlc_tracked_malloc(size, __FILE__, __LINE__)
#define AMLC_FREE(ptr) __amlc_tracked_free(ptr)

// Override the standard malloc/free with the custom functions
#define malloc(size) AMLC_MALLOC(size)
#define free(ptr) AMLC_FREE(ptr)

#endif // AMLC_H

