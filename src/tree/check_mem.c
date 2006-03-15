#ifdef SHOW_MALLOC

/******************************************************************************
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
#include "types_trav.h"
#include "phase.h"

typedef struct MEMOBJ {
    int size;
    void *ptr;
    nodetype nodetype;
    char *file;
    int line;
    compiler_subphase_t subphase;
    const char *traversal;
    bool used_bit;
    bool shared_bit;
} memobj;

#define MEMOBJ_SIZE(n) ((n)->size)
#define MEMOBJ_PTR(n) ((n)->ptr)
#define MEMOBJ_NODETYPE(n) ((n)->nodetype)
#define MEMOBJ_FILE(n) ((n)->file)
#define MEMOBJ_LINE(n) ((n)->line)
#define MEMOBJ_SUBPHASE(n) ((n)->subphase)
#define MEMOBJ_TRAVERSAL(n) ((n)->traversal)
#define MEMOBJ_USEDBIT(n) ((n)->used_bit)
#define MEMOBJ_SHAREDBIT(n) ((n)->shared_bit)

#define SHIFT2ORIG(n) ((memobj **)((char *)n - malloc_align_step))
#define SHIFT2MEMOBJ(n) (*(SHIFT2ORIG (n)))

#define ORIG2SHIFT(n) ((char *)n + malloc_align_step)

static void CHKManalyzeMemtab (memobj *, int);

static char *CHKMtoString (memobj *);

static memobj *memtab = NULL;
static int memfreeslots = 0;
static int memindex = 0;
static int memtabsize = 0;

/** <!--********************************************************************-->
 *
 * @fn node *CHKMdoMemCheck( node *syntax_tree)
 *
 * the traversal start function
 *
 *****************************************************************************/
node *
CHKMdoMemCheck (node *syntax_tree)
{

    DBUG_ENTER ("CHKMdoMemCheck");

    DBUG_PRINT ("CHKM", ("Traversing heap..."));

    TRAVpush (TR_chkm);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_PRINT ("CHKM", ("Heap traversal complete"));

    DBUG_PRINT ("CHKM", ("Analyzing"));

    CHKManalyzeMemtab (memtab, memindex);

    DBUG_PRINT ("CHKM", ("CheckSpacemechanism complete"));

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn void *CHKMregisterMem( int size, void *orig_ptr)
 *
 *****************************************************************************/
void *
CHKMregisterMem (int size, void *orig_ptr)
{
    void *shifted_ptr = NULL;
    memobj *memobj_ptr;

    DBUG_ENTER ("CHKMregisterMem");

    shifted_ptr = ORIG2SHIFT (orig_ptr);

    /* change the memtabsize if the memtab is full or empty */
    if (memindex == memtabsize) {

        /* at beginning the memtab ist empty so: */
        if (memtabsize == 0) {

            DBUG_PRINT ("CHKM", ("Allocating memtab for 1000 memory objects"
                                 " size: %d bytes",
                                 1000 * sizeof (memobj)));

            memtab = (memobj *)malloc (1000 * sizeof (memobj));
            memtabsize = 1000;
            memfreeslots = 1000;
        }

        /* expand or reduce mechanismn for the memtab */
        else {
            int newtabsize = (memtabsize - memfreeslots) * 2;
            int newindex = 0;
            memobj *newtab;

            DBUG_PRINT ("CHKM", ("Allocating memtab for %d memory objects"
                                 " size: %d bytes",
                                 newtabsize, newtabsize * sizeof (memobj)));

            newtab = (memobj *)malloc (newtabsize * sizeof (memobj));

            /* copy the old memtab to new (smaller or bigger) memtab. All gaps
               will ignore */
            for (int i = 0; i < memtabsize; i++) {

                if (memtab[i].ptr != NULL) {
                    memobj **tmpptr;

                    newtab[newindex] = memtab[i];
                    tmpptr = (memobj **)newtab[newindex].ptr;
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
    MEMOBJ_SUBPHASE (memobj_ptr) = global.compiler_subphase;
    MEMOBJ_TRAVERSAL (memobj_ptr) = TRAVgetName ();

    *(memobj **)orig_ptr = memobj_ptr;

    memfreeslots = memfreeslots - 1;
    memindex = memindex + 1;

    DBUG_RETURN (shifted_ptr);
}

/** <!--********************************************************************-->
 *
 * @fn void *CHKMunregisterMem( void *shiftet_ptr)
 *
 *****************************************************************************/
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

/** <!--********************************************************************-->
 *
 * @fn node *CHKMassignSpaceLeaks( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CHKMassignSpaceLeaks (node *arg_node, info *arg_info)
{
    memobj *memobj_ptr;

    DBUG_ENTER ("CHKMassignSpaceLeaks");

    memobj_ptr = SHIFT2MEMOBJ (arg_node);

    if ((memtab <= memobj_ptr) && (memobj_ptr < memtab + memtabsize)) {

        if (MEMOBJ_USEDBIT (memobj_ptr) == 1) {
            MEMOBJ_SHAREDBIT (memobj_ptr) = 1;
        } else {
            MEMOBJ_USEDBIT (memobj_ptr) = 1;
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static void *CHKManalyzeMemtab( memobj *memtab)
 *
 *****************************************************************************/
static void
CHKManalyzeMemtab (memobj *memtab, int memindex)
{
    int i;
    memobj *memobj_ptr;
    memobj *copy_memtab;
    int copy_index;
    node *arg_node;
    char *string;
    char *memtab_info;
    int *cnt_sharedmem;
    int *cnt_spaceleaks;

    DBUG_ENTER ("CHKManalyzeMemtab");

    /************************************
       must copy the memtab(!) to freeze, because Node_Error expand again the
       memtab and so can possibly change the back pointers
    **********************************/
    copy_memtab = memtab;
    copy_index = memindex;
    string = NULL;
    cnt_sharedmem = 0;
    cnt_spaceleaks = 0;

    arg_node = (node *)ORIG2SHIFT (MEMOBJ_PTR (memtab));

    for (i = 0; i < copy_index - 1; i++) {

        memobj_ptr = copy_memtab + i;

        if (MEMOBJ_PTR (memobj_ptr) == NULL) {

            if (MEMOBJ_NODETYPE (memobj_ptr) != N_undefined) {
                /* =========== Nodetypes =========== */
                if (MEMOBJ_USEDBIT (memobj_ptr) || MEMOBJ_SHAREDBIT (memobj_ptr)) {

                    memtab_info = CHKMtoString (memobj_ptr);
                    string = ILIBstringConcat ("dangling Ptr: ", memtab_info);

                    NODE_ERROR (arg_node)
                      = CHKinsertError (NODE_ERROR (arg_node), string);
                }
            }
        } else {

            if (MEMOBJ_NODETYPE (memobj_ptr) != N_undefined) {
                /* =========== Nodetypes =========== */
                if (!MEMOBJ_USEDBIT (memobj_ptr)) {

                    memtab_info = CHKMtoString (memobj_ptr);
                    string = ILIBstringConcat ("Spaceleak(node): ", memtab_info);

                    NODE_ERROR (arg_node)
                      = CHKinsertError (NODE_ERROR (arg_node), string);
                    cnt_spaceleaks++;
                } else {
                    if (MEMOBJ_SHAREDBIT (memobj_ptr)) {

                        memtab_info = CHKMtoString (memobj_ptr);
                        string = ILIBstringConcat ("shared memory: ", memtab_info);

                        NODE_ERROR (arg_node)
                          = CHKinsertError (NODE_ERROR (arg_node), string);
                        cnt_sharedmem++;
                    }
                }
            } else { /*
                  =========== not Nodetypes ===========
                 if (( MEMOBJ_PTR( memobj_ptr) != NULL) &&
                     ( !MEMOBJ_USEDBIT( memobj_ptr))) {

                   memtab_info = CHKMtoString( memobj_ptr);
                   string = ILIBstringConcat( "Spaceleak:", memtab_info);

                   CTIwarn( "spaceleak: File: %s Line: %d Used_Bit: %d",
                            MEMOBJ_FILE( memobj_ptr),
                            MEMOBJ_LINE( memobj_ptr),
                            MEMOBJ_USEDBIT( memobj_ptr));
                 }*/
            }
        }
    }
    CTIwarn (">>>>> Counter Spaceleaks: %d Counter Shared Memory: %d", cnt_spaceleaks,
             cnt_sharedmem);

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CHKMsetNodeType( node *shifted_ptr, nodetype newnodetype)
 *
 *****************************************************************************/
void
CHKMsetNodeType (node *shifted_ptr, nodetype newnodetype)
{
    memobj *memobj_ptr;

    DBUG_ENTER ("CHKMsetNodeType");

    memobj_ptr = SHIFT2MEMOBJ (shifted_ptr);

    MEMOBJ_NODETYPE (memobj_ptr) = newnodetype;

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CHKMsetLocation( node *shiftet_ptr, char *file, int line)
 *
 *****************************************************************************/
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

/** <!--********************************************************************-->
 *
 * @fn void CHKMsetSubPhase( node *shiftet_ptr, compiler_subphase_t subphase)
 *
 *****************************************************************************/
void
CHKMsetSubphase (node *shifted_ptr, compiler_subphase_t subphase)
{
    memobj *memobj_ptr;

    DBUG_ENTER ("CHKMsetSubPhase");

    memobj_ptr = SHIFT2MEMOBJ (shifted_ptr);

    MEMOBJ_SUBPHASE (memobj_ptr) = subphase;

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void CHKMsetTraversal( node *shiftet_ptr, trav_t *traversal)
 *
 *****************************************************************************/
void
CHKMsetTraversal (node *shifted_ptr)
{
    memobj *memobj_ptr;

    DBUG_ENTER ("CHKMsetTraversal");

    memobj_ptr = SHIFT2MEMOBJ (shifted_ptr);

    MEMOBJ_TRAVERSAL (memobj_ptr) = TRAVgetName ();

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn int CHKMgetSize( node *shiftet_ptr)
 *
 *****************************************************************************/
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

static char *
CHKMtoString (memobj *memobj_ptr)
{
    char *str;
    int test = 0;

    DBUG_ENTER ("CHKMtoString");

    str = (char *)ILIBmalloc (sizeof (char) * 1024);

    test = snprintf (str, 1024,
                     "File: %s, Line: %d, Traversal: %s, Subphase: %s, "
                     "Used_bit: %d, Shared_bit: %d \n",
                     MEMOBJ_FILE (memobj_ptr), MEMOBJ_LINE (memobj_ptr),
                     MEMOBJ_TRAVERSAL (memobj_ptr),
                     PHsubPhaseName (MEMOBJ_SUBPHASE (memobj_ptr)),
                     MEMOBJ_USEDBIT (memobj_ptr), MEMOBJ_SHAREDBIT (memobj_ptr));

    if (test >= 1024) {
        snprintf (str, test,
                  "File:%s, Line:%d, Traversal:%s, Subphase:%s, "
                  "Used_bit: %d, Shared_bit: %d \n",
                  MEMOBJ_FILE (memobj_ptr), MEMOBJ_LINE (memobj_ptr),
                  MEMOBJ_TRAVERSAL (memobj_ptr),
                  PHsubPhaseName (MEMOBJ_SUBPHASE (memobj_ptr)),
                  MEMOBJ_USEDBIT (memobj_ptr), MEMOBJ_SHAREDBIT (memobj_ptr));
    }

    DBUG_RETURN (str);
};

#endif /* SHOW_MALLOC */
