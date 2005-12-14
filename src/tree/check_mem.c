/*
 * $Id$ check_mem.c
 */

#include <stdlib.h>
#include "internal_lib.h"

typedef struct MEMOBJ {
    int size;
    void *ptr;
    nodetype nodetype;
#if 0
    int subphase;
    int traversal;
#endif
    int bit;
} memobj;

memobj *memtab = NULL;

int memfreeslots = 0;
int memindex = 0;
int memtabsize = 0;

void *
CMregisterMem (int bsize, void *aptr)
{
    DBUG_ENTER ("CMregisterMem");

    void *bptr = NULL;

    bptr = (char *)aptr + malloc_align_step;

    if (memindex == memtabsize) {

        if (memtabsize == 0) {

            memtab = (memobj *)malloc (1000 * sizeof (memobj));
            memtabsize = 1000;
            memfreeslots = 1000;
        } else {

            int newtabsize = (memtabsize - memfreeslots) * 2;
            int newindex = 0;
            memobj *newtab = (memobj *)malloc (newtabsize * sizeof (memobj));

            for (int i = 0; i < memtabsize; i++) {

                if (memtab[i].ptr != NULL) {

                    newtab[newindex] = memtab[i];
                    memobj **tmpptr = (memobj **)newtab[newindex].ptr;
                    *tmpptr = newtab + newindex;
                    newindex++;
                }
            }

            free (memtab);
            memtab = newtab;
            memindex = newindex;
            memfreeslots = newtabsize - newindex;
            memtabsize = newtabsize;
        }
    }
    memtab[memindex].size = bsize;
    memtab[memindex].ptr = aptr;
    memtab[memindex].bit = 0;

    *(memobj **)aptr = memtab + memindex;

    memfreeslots = memfreeslots - 1;
    memindex = memindex + 1;

    DBUG_RETURN (bptr);
}

void *
CMunregisterMem (void *bptr)
{
    memobj **aptr;

    DBUG_ENTER (" CMunregisterMEM");

    aptr = (memobj **)((char *)bptr - malloc_align_step);

    if (((**aptr).size == 0) && ((**aptr).ptr = NULL)) {

        CTIwarn ("%s", "double free"); /* miss where */
    }

    (**aptr).size = 0;
    (**aptr).ptr = NULL;

    memfreeslots = memfreeslots + 1;

    DBUG_RETURN ((void *)aptr);
}

int
CMgetSize (void *bptr)
{
    DBUG_ENTER ("CMgetSize");

    int tmpsize = 0;
    memobj **tmpobj;

    tmpobj = (memobj **)((char *)bptr - malloc_align_step);

    tmpsize = (**tmpobj).size;

    DBUG_RETURN (tmpsize);
}

void
CMtreewalker ()
{

    /* durch den Baum traversieren und die Nodes die besucht werden, das bit flippen */
}

void
CMsetNodeType (node *bptr, nodetype newnodetype)
{

    memobj **tmpobj;

    tmpobj = (memobj **)((char *)bptr - malloc_align_step);

    (**tmpobj).nodetype = newnodetype;
}
