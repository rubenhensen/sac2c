#ifdef SHOW_MALLOC

/*
 * $Id$ check_mem.c
 *
 * PREFIX: CHKM
 *
 * description:
 *   the checkmechanism for memory leaks
 *
 ******************************************************************************/

#include <stdlib.h>
#include "internal_lib.h"

#include "types.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "check_lib.h"

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

#define MEMOBJ_SIZE(n) ((n)->size)
#define MEMOBJ_PTR(n) ((n)->ptr)
#define MEMOBJ_NODETYPE(n) ((n)->nodetype)
#define MEMOBJ_FILE(n) ((n)->file)
#define MEMOBJ_LINE(n) ((n)->line)
#define MEMOBJ_USEDBIT(n) ((n)->used_bit)
#define MEMOBJ_SHAREDBIT(n) ((n)->shared_bit)

#define SHIFT2ORIG(n) ((memobj **)((char *)n - malloc_align_step))
#define SHIFT2MEMOBJ(n) (*(SHIFT2ORIG (n)))

#define ORIG2SHIFT(n) ((char *)n + malloc_align_step)

static memobj *memtab = NULL;
static int memfreeslots = 0;
static int memindex = 0;
static int memtabsize = 0;

void *
CHKMregisterMem (int size, void *orig_ptr)
{
    void *shifted_ptr = NULL;
    memobj *memobj_ptr;

    DBUG_ENTER ("CHKMregisterMem");

    shifted_ptr = ORIG2SHIFT (orig_ptr);

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
    memobj_ptr = memtab + memindex;

    MEMOBJ_SIZE (memobj_ptr) = size;
    MEMOBJ_PTR (memobj_ptr) = orig_ptr;
    MEMOBJ_USEDBIT (memobj_ptr) = 0;
    MEMOBJ_SHAREDBIT (memobj_ptr) = 0;
    MEMOBJ_NODETYPE (memobj_ptr) = N_undefined;

    *(memobj **)orig_ptr = memobj_ptr;

    memfreeslots = memfreeslots - 1;
    memindex = memindex + 1;

    DBUG_RETURN (shifted_ptr);
}

void *
CHKMunregisterMem (void *shifted_ptr)
{
    memobj *memobj_ptr;
    void *orig_ptr;

    DBUG_ENTER ("CHKMunregisterMEM");

    orig_ptr = SHIFT2ORIG (shifted_ptr);
    memobj_ptr = SHIFT2MEMOBJ (shifted_ptr);

    if ((MEMOBJ_SIZE (memobj_ptr) == 0) && (MEMOBJ_PTR (memobj_ptr) == NULL)) {
        CTIwarn ("%s", "double free"); /* miss where */
    }

    MEMOBJ_SIZE (memobj_ptr) = 0;
    MEMOBJ_PTR (memobj_ptr) = NULL;
    memfreeslots = memfreeslots + 1;

    DBUG_RETURN ((void *)orig_ptr);
}

node *
CHKMdoCheckMemory (node *syntax_tree, info *arg_info)
{

    DBUG_ENTER ("CHKMdoCheckMemory");

    DBUG_PRINT ("CHKM", ("Traversing heap..."));

    TRAVpush (TR_chkm);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    DBUG_PRINT ("CHKM", ("Heap traversal complete"));

    DBUG_PRINT ("CHKM", ("Analyzing"));

    DBUG_PRINT ("CHKM", ("CheckSpacemechanism complete"));

    DBUG_RETURN (syntax_tree);
}

node *
CHKMassignSpaceLeaks (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CHKMeliminateSpaceLeaks");

    memobj *memobj_ptr;

    memobj_ptr = SHIFT2MEMOBJ (arg_node);

    if (MEMOBJ_USEDBIT (memobj_ptr) == 1) {
        MEMOBJ_SHAREDBIT (memobj_ptr) = 1;
    } else {
        MEMOBJ_USEDBIT (memobj_ptr) = 1;
    }

    DBUG_RETURN (arg_node);
}

/*
void *CHKMeliminateSpaceLeaks() {

  DBUG_ENTER( "CHKMeliminateSpaceLeaks");

  for (int i = 0; i < memtabsize; i++){

    if( MEMOBJ_PTR( memtab[i]) != NULL && MEMOBJ_USEDBIT( memtab[i]) == 1)
      {
        char *string = NULL;

        NODE_ERROR( arg_node) = CHKinsertError( NODE_ERROR( arg_node),
                                                string);
        MEMOBJ_USEDBIT( memtab[i]) == 1);
      }

    if( MEMOBJ_PTR( memtab[i]) != NULL && MEMOBJ_SHAREDBIT( memtab[i]) == 1)
      {
        char *string = NULL;

        NODE_ERROR( arg_node) = CHKinsertError( NODE_ERROR( arg_node),
                                                string);
        MEMOBJ_SHAREDBIT( memtab[i]) == 1);
      }
  }
  DBUG_VOID_RETURN;
}
*/

void
CHKMsetNodeType (node *shifted_ptr, nodetype newnodetype)
{
    memobj *memobj_ptr;

    DBUG_ENTER ("CHKMsetNodeType");

    memobj_ptr = SHIFT2MEMOBJ (shifted_ptr);

    MEMOBJ_NODETYPE (memobj_ptr) = newnodetype;

    DBUG_VOID_RETURN;
}

void
CHKMsetLocation (node *shifted_ptr, char *file, int line)
{
    memobj *memobj_ptr;

    DBUG_ENTER ("CHKMsetLocation");

    memobj_ptr = SHIFT2MEMOBJ (shifted_ptr);

    MEMOBJ_FILE (memobj_ptr) = file;

    MEMOBJ_LINE (memobj_ptr) = line;

    DBUG_VOID_RETURN;
}

int
CHKMgetSize (node *shifted_ptr)
{
    memobj *memobj_ptr;
    int size;

    DBUG_ENTER ("CHKMgetSize");

    memobj_ptr = SHIFT2MEMOBJ (shifted_ptr);

    size = MEMOBJ_SIZE (memobj_ptr);

    DBUG_RETURN (size);
}

#endif /* SHOW_MALLOC */
