// Hello from the Aspen Software Foundation!
// This file is part of the Aspen Ganamede Leak Checker (AGLC).

    // _    ____  ____  _____ _   _ 
   // / \  / ___||  _ \| ____| \ | |
  // /   \ \___ \| |_) |  _| |  \| |
 // /     \ ___) |  __/| |___| |\  |
// /       \____/|_|   |_____|_| \_|

#ifndef GANAMEDE_H
#define GANAMEDE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct GanamedeBlock {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    struct GanamedeBlock* next;
} GanamedeBlock;

static GanamedeBlock* __ganamede_head = NULL;

// Add a block to track memory allocation
static void __ganamede_add_block(void* ptr, size_t size, const char* file, int line) {
    printf("Allocating memory: %zu bytes at %p (file: %s, line: %d)\n", size, ptr, file, line);

    GanamedeBlock* block = (GanamedeBlock*)malloc(sizeof(GanamedeBlock));
    block->ptr = ptr;
    block->size = size;
    block->file = file;
    block->line = line;
    block->next = __ganamede_head;
    __ganamede_head = block;
}

// Remove a block when memory is freed
static void __ganamede_remove_block(void* ptr) {
    printf("Freeing memory: %p\n", ptr);

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

// Tracked malloc
static void* __ganamede_tracked_malloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (ptr) {
        __ganamede_add_block(ptr, size, file, line);
    }
    return ptr;
}

// Tracked free
static void __ganamede_tracked_free(void* ptr) {
    __ganamede_remove_block(ptr);
    free(ptr);
}

// Leak dump with 64-byte display
static void _CrtDumpMemoryLeaks() {
    if (__ganamede_head == NULL) {
        printf("No memory leaks detected.\n");
    } else {
        printf("Memory leaks detected:\n");
        GanamedeBlock* curr = __ganamede_head;
        while (curr) {
            printf(" Leak: %zu bytes at %p (allocated at %s:%d)\n",
                   curr->size, curr->ptr, curr->file, curr->line);

            size_t to_display = curr->size < 64 ? curr->size : 64;
            uint8_t* bytes = (uint8_t*)curr->ptr;

            printf("  Contents (up to 64 bytes):\n   HEX : ");
            for (size_t i = 0; i < to_display; i++) {
                printf("%02X ", bytes[i]);
                if ((i + 1) % 16 == 0 && i + 1 < to_display)
                    printf("\n         ");
            }

            printf("\n  ASCII: ");
            for (size_t i = 0; i < to_display; i++) {
                printf("%c", (bytes[i] >= 32 && bytes[i] <= 126) ? bytes[i] : '.');
                if ((i + 1) % 16 == 0 && i + 1 < to_display)
                    printf("\n         ");
            }

            printf("\n\n");
            curr = curr->next;
        }
    }
}

// Auto-override malloc/free when included
#define malloc(x) __ganamede_tracked_malloc(x, __FILE__, __LINE__)
#define free(x)   __ganamede_tracked_free(x)

#endif // GANAMEDE_H


