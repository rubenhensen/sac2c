#ifndef TLS_MALLOC_H
#define TLS_MALLOC_H

#if defined(__mt_freestanding__) && (defined(__alpha__) || defined(__mtalpha__))

#include <svp/compiler.h>
#include <cstddef.h>

// do not set this to less than 8
#define MIN_CHUNK_SIZE 64

// current microgrid default:
#define TLS_BITS 30 /* 2Gb  */

// area 51: mandatory space between heap and stack
#define TLS_HEAP_PROTECT 256

__attribute__ ((__always_inline__)) static void
tls_malloc_init (void)
{
    register unsigned long sp __asm__("$30");

    // get the address of the first word of TLS:
    unsigned long vp;
    vp = ((sp >> TLS_BITS) - 1) << TLS_BITS;
    void **p = (void **)(void *)vp;

    // set the HWM to the next cache line:
    *p = (void *)((char *)(void *)p + MIN_CHUNK_SIZE);
}

__attribute__ ((__always_inline__)) static void *
tls_malloc (size_t sz)
{
    // align the size on a cache line boundary
    sz = (sz & (MIN_CHUNK_SIZE - 1)) ? ((sz + MIN_CHUNK_SIZE) & ~(MIN_CHUNK_SIZE - 1))
                                     : sz;

    register unsigned long sp __asm__("$30");

    // get the address of the first word of TLS:
    unsigned long vp;
    vp = ((sp >> TLS_BITS) - 1) << TLS_BITS;
    char **p = (char **)(void *)vp;

    // check against stack pointer:
    if (unlikely ((unsigned long)(*p + sz + TLS_HEAP_PROTECT) > sp))
        return 0;

    // allocate:
    void *res = *p;
    *p = *p + sz;
    return res;
}

#define tls_malloc_cleanup() ((void)0)

#else
#define tls_malloc_cleanup() ((void)0)
#define tls_malloc(...) malloc (__VA_ARGS__)
#define tls_malloc_init() ((void)0)
#endif

#endif
