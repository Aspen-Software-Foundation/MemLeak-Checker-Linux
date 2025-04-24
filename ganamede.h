// Hello from the Aspen Software Foundation!
// This file is part of the Aspen Ganamede Leak Checker (AMLC).

   //  _    ____  ____  _____ _   _ 
   // / \  / ___||  _ \| ____| \ | |
  // /   \ \___ \| |_) |  _| |  \| |
 // /     \ ___) |  __/| |___| |\  |
// /       \____/|_|   |_____|_| \_|

#ifndef GANAMEDE_H
#define GANAMEDE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct GanamedeBlock {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    struct GanamedeBlock* next;
} GanamedeBlock;

static GanamedeBlock* __ganamede_head = NULL;

static void __ganamede_add_block(void* ptr, size_t size, const char* file, int line) {
    GanamedeBlock* block = malloc(sizeof(GanamedeBlock));
    block->ptr = ptr;
    block->size = size;
    block->file = file;
    block->line = line;
    block->next = __ganamede_head;
    __ganamede_head = block;
}

static void __ganamede_remove_block(void* ptr) {
    GanamedeBlock** curr = &__ganamede_head;
    while (*curr) {
        if ((*curr)->ptr == ptr) {
            GanamedeBlock* temp = *curr;
            *curr = temp->next;
            free(temp);
            return;
        }
        curr = &(*curr)->next;
    }
}

static void* __ganamede_tracked_malloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (ptr) {
        __ganamede_add_block(ptr, size, file, line);
    }
    return ptr;
}

static void __ganamede_tracked_free(void* ptr) {
    __ganamede_remove_block(ptr);
    free(ptr);
}

static void _CrtDumpMemoryLeaks() {
    if (__ganamede_head == NULL) {
        printf("No memory leaks detected.\n");
    } else {
        printf("Memory leaks detected:\n");
        GanamedeBlock* curr = __ganamede_head;
        while (curr) {
            printf(" Leak: %zu bytes at %p (allocated at %s:%d)\n",
                   curr->size, curr->ptr, curr->file, curr->line);
            curr = curr->next;
        }
    }
}

#ifdef _CRTDBG_MAP_ALLOC
    #define malloc(x) __ganamede_tracked_malloc(x, __FILE__, __LINE__)
    #define free(x)   __ganamede_tracked_free(x)
#endif

#endif // GANAMEDE_H

