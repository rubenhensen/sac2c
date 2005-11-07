/*
 * $Id$
 */

#include <stdlib.h>
#include "internal_lib.h"

typedef struct MEMOBJ {
    int size;
    void *ptr;
    //  int subphase;
    //  int traversal;
    //  int type;
    int bit;
} memobj;

memobj *memtab = NULL;

int memfreeslots = 0;
int memindex = 0;
int memtabsize = 0;

void
CHKregister (int bsize, void *aptr)
{

    if (memindex == memtabsize) {

        if (memtabsize == 0) {

            memtab = (memobj *)malloc (100000 * sizeof (memobj));
            memtabsize = 100000;
            memfreeslots = 100000;
        } else {

            int newtabsize = memtabsize * 2;
            int newindex = 0;
            memobj *newtab = (memobj *)malloc (newtabsize * sizeof (memobj));

            for (int i = 0; i < memtabsize; i++) {

                if (memtab[i].ptr != NULL) {

                    newtab[newindex] = memtab[i];
                    memobj **tmpptr
                      = (memobj **)(memtab[i].ptr - global.malloc_align_step);
                    *tmpptr = &memtab[newindex];
                    newindex++;
                }

                memtab = newtab;
                memindex = newindex;
                memfreeslots = newtabsize - newindex;
                memtabsize = newtabsize;
                free (memtab);
            }
        }
    }

    memtab[memindex].size = bsize;
    memtab[memindex].ptr = aptr;
    memtab[memindex].bit = 0;

    /*bsize = *memtab[memindex];*/

    memfreeslots = -1;
    memindex = -1;
}

void *
CHKunregister (void *bptr)
{

    memtab[memindex].size = 0;
    memtab[memindex].ptr = NULL;

    memfreeslots = +1;
    memindex = +1;
}
