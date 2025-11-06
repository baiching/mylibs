#define INTERNAL
#include "memorytracker.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#define STBDS_NO_SHORT_NAMES
#include <stdio.h>
#include <stdlib.h>

struct memoryblk
{
    size_t size;
    const char *file;
    const char *expr;
    int line;
    struct memoryblk **addr;
    int padding[3]; // Make it an even 16 byte length (total size=32 bytes in 32-bit mode).
};

// tracking the original list
struct memoryblk **memblkList = NULL;

static void f_trackMemBlkDetails(struct memoryblk *mb){
    printf("%zu bytes allocated with \"%s\" at %s: %d\n", mb->size, mb->expr, mb->file, mb->line);
}


void *f_malloc_tracker(size_t size, const char *expr, const char *file, int line){
    struct memoryblk *memblk = (malloc)(size + sizeof(*memblk));
    if (!memblk)
    {
        printf("Malloc failed!\n");
        return NULL;
    }
    *memblk = (struct memoryblk){
        .size = size,
        .file = file,
        .expr = expr,
        .line = line,
        .addr = (void *)&memblk[1]
    };

    stbds_arrput(memblkList, memblk);

    return (void *)&memblk[1];
}

void f_tracK_free(void *ptr, const char *expr, const char *file, int line){
    if (!ptr)
    {
        printf("Nothing to Free!\n");
        return;
    }
    else{
        struct memoryblk *memblk = &((struct memoryblk *)(ptr))[-1];

        for (int i =0; i < stbds_arrlen(memblkList); i++) {
            if (memblk->addr == memblkList[i]->addr) {
                stbds_arrdel(memblkList, i);
            }
        }

        (free)(memblk);
    }
    
}

void f_trackListAllocations(){
    printf("Allocation List start from here:\n");

    if (!memblkList)
    {
        printf(">>> EMPTY <<<\n");
    }
    else{
        for (int i =0; i < stbds_arrlen(memblkList); i++) {
            f_trackMemBlkDetails(memblkList[i]);
        }
    }
    printf("Allocation List End Here.\n");
}