/*
 * $Id$ check_mem.c
 */

#include <stdlib.h>
#include "internal_lib.h"

#include "types.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

typedef struct MEMOBJ {
    int size;
    void *ptr;
    nodetype nodetype;
    char *file;
    int line;
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
CHKMregisterMem (int bsize, void *aptr)
{
    DBUG_ENTER ("CHKMregisterMem");

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
    memtab[memindex].nodetype = N_undefined;

    *(memobj **)aptr = memtab + memindex;

    memfreeslots = memfreeslots - 1;
    memindex = memindex + 1;

    DBUG_RETURN (bptr);
}

void *
CHKMunregisterMem (void *bptr)
{
    memobj **aptr;

    DBUG_ENTER (" CHKMunregisterMEM");

    aptr = (memobj **)((char *)bptr - malloc_align_step);

    if (((**aptr).size == 0) && ((**aptr).ptr = NULL)) {

        CTIwarn ("%s", "double free"); /* miss where */
    }

    (**aptr).size = 0;
    (**aptr).ptr = NULL;

    memfreeslots = memfreeslots + 1;

    DBUG_RETURN ((void *)aptr);
}

node *
CHKMdoTreeWalk (node *syntax_tree, info *arg_info)
{

    DBUG_ENTER ("CHKMdoTreewalk");

    DBUG_PRINT ("CHKM", ("Starting the CheckSpacemechanism"));

    TRAVpush (TR_chkm);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    DBUG_PRINT ("CHKM", ("CheckSpacemechanism complete"));

    DBUG_RETURN (syntax_tree);
}

void
CHKMspaceLeaks ()
{

    DBUG_ENTER ("CHKMspaceLeaks");

    /* durch den Baum traversieren und die Nodes die besucht werden, das bit flippen */
}

void
CHKMsetNodeType (node *bptr, nodetype newnodetype)
{

    memobj **tmpobj;

    tmpobj = (memobj **)((char *)bptr - malloc_align_step);

    (**tmpobj).nodetype = newnodetype;
}

void
CHKMsetLocation (node *bptr, char *file, int line)
{

    memobj **tmpobj;

    tmpobj = (memobj **)((char *)bptr - malloc_align_step);

    (**tmpobj).file = file;

    (**tmpobj).line = line;
}

int
CHKMgetSize (void *bptr)
{
    DBUG_ENTER ("CMgetSize");

    int tmpsize = 0;
    memobj **tmpobj;

    tmpobj = (memobj **)((char *)bptr - malloc_align_step);

    tmpsize = (**tmpobj).size;

    DBUG_RETURN (tmpsize);
}
