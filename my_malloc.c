#define _XOPEN_SOURCE 500
#include <unistd.h>

typedef struct freenode
{
    size_t size;
    struct freenode *next;
} freenode;

#define HEAP_CHUNK_SIZE 4096

freenode *freelist = NULL;

void *malloc(size_t size)
{
    if (size < 1)
    {
        return NULL;
    }

    const size_t MIN_SIZE = 32 - sizeof(freenode) - sizeof(size_t);

    if (size < MIN_SIZE)
    {
        size = MIN_SIZE;
    }
    else
    {
        size += sizeof(freenode) + sizeof(size_t);

        if (size % 16 != 0)
        {
            size = ((size + 15) / 16) * 16;
        }
    }

    if (freelist == NULL)
    {
        void *mem = sbrk(HEAP_CHUNK_SIZE);

        if (mem == (void *)-1)
        {
            return NULL;
        }

        if (mem == NULL)
        {
            return NULL;
        }

        freelist = (freenode *)mem;
        freelist->size = HEAP_CHUNK_SIZE - sizeof(freenode);
        freelist->next = NULL;
    }

    freenode *prev = NULL;
    freenode *cur = freelist;

    while (cur != NULL)
    {
        if (cur->size >= size)
        {
            void *ptr = (void *)(cur + 1);

            if (cur->size - size < sizeof(freenode))
            {
                if (prev)
                {
                    prev->next = cur->next;
                }
                else
                {
                    freelist = cur->next;
                }
            }
            else
            {
                freenode *split = (freenode *)((void *)cur + size);
                split->size = cur->size - size - sizeof(freenode);
                split->next = cur->next;

                if (prev)
                {
                    prev->next = split;
                }
                else
                {
                    freelist = split;
                }
            }
            cur->size = size - sizeof(freenode);
            *((size_t *)((void *)ptr - sizeof(size_t))) = cur->size;

            return ptr;
        }

        prev = cur;
        cur = cur->next;
    }

    size_t chunk = HEAP_CHUNK_SIZE;

    if (HEAP_CHUNK_SIZE > size + sizeof(freenode))
    {
        chunk = size + sizeof(freenode);
    }

    void *mem = sbrk(chunk);

    if (mem == (void *)-1)
    {
        return NULL;
    }
    if (mem == NULL)
    {
        return NULL;
    }

    freenode *add = (freenode *)mem;
    add->size = chunk - sizeof(freenode);
    add->next = freelist;
    freelist = add;
    return malloc(size);
}

void free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    freenode *new_node = (freenode *)(ptr - 8);

    new_node->next = freelist;
    freelist = new_node;

    return;
}

