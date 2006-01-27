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
    bool used_bit;
    bool shared_bit;
} memobj;

static memobj *memtab = NULL;
static int memfreeslots = 0;
static int memindex = 0;
static int memtabsize = 0;

void *
CHKMregisterMem (int bsize, void *aptr)
{
    void *bptr = NULL;

    DBUG_ENTER ("CHKMregisterMem");

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
    memtab[memindex].used_bit = 0;
    memtab[memindex].shared_bit = 0;
    memtab[memindex].nodetype = N_undefined;

    *(memobj **)aptr = memtab + memindex;

    memfreeslots = memfreeslots - 1;
    memindex = memindex + 1;

    /*
  #else
    bptr = aptr;
  #endif
  */
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

    /*
    #else
      aptr = (memobj **) bptr;
    #endif
    */
    DBUG_RETURN ((void *)aptr);
}

node *
CHKMdoCheckMemory (node *syntax_tree, info *arg_info)
{

    DBUG_ENTER ("CHKMdoCheckMemory");

    DBUG_PRINT ("CHKM", ("Starting the CheckSpacemechanism"));

    TRAVpush (TR_chkm);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    DBUG_PRINT ("CHKM", ("CheckSpacemechanism complete"));

    DBUG_RETURN (syntax_tree);
}

node *
CHKMilliminateSpaceLeaks (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CHKMilliminateSpaceLeaks");

    memobj **tmpobj;

    tmpobj = (memobj **)((char *)arg_node - malloc_align_step);

    (**tmpobj).used_bit = 1;

    for (int i = 0; i < memtabsize; i++) {

        if (memtab[i].ptr != NULL) {
        }
    }

    DBUG_RETURN (arg_node);
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
    int tmpsize = 0;

    DBUG_ENTER ("CMgetSize");

    memobj **tmpobj;

    tmpobj = (memobj **)((char *)bptr - malloc_align_step);

    tmpsize = (**tmpobj).size;

    DBUG_RETURN (tmpsize);
}
