/*
 *
 * $Log$
 * Revision 1.2  1995/07/14 17:04:31  asi
 * added macro SWAP, it swaps two pointers
 *
 * Revision 1.1  1995/03/28  12:01:50  hw
 * Initial revision
 *
 *
 */

#ifndef _internal_lib_h

#define _internal_lib_h

void *Malloc (int size);
char *StringCopy (char *source);

#define SWAP(ptr1, ptr2)                                                                 \
    {                                                                                    \
        void *tmp;                                                                       \
        tmp = (void *)ptr1;                                                              \
        ptr1 = (void *)ptr2;                                                             \
        ptr2 = (void *)tmp;                                                              \
    }

#endif /* _internal_lib_h */
