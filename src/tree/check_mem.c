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
#include <string.h>

#include "check_mem.h"

#include "internal_lib.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "check_lib.h"
#include "types_trav.h"
#include "phase.h"
#include "print.h"

/**
 * INFO structure
 */
struct INFO {
    node *error;
};

/**
 * INFO macros
 */
#define INFO_ERROR(n) (n->error)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_ERROR (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

typedef struct MEMOBJ {
    int size;
    void *ptr;
    nodetype nodetype;
    char *file;
    int line;
    compiler_subphase_t subphase;
    const char *traversal;
    struct {
        unsigned int IsUsed : 1;
        unsigned int IsShared : 1;
        unsigned int IsReported : 1;
    } flags;
} memobj;

#define MEMOBJ_SIZE(n) ((n)->size)
#define MEMOBJ_PTR(n) ((n)->ptr)
#define MEMOBJ_NODETYPE(n) ((n)->nodetype)
#define MEMOBJ_FILE(n) ((n)->file)
#define MEMOBJ_LINE(n) ((n)->line)
#define MEMOBJ_SUBPHASE(n) ((n)->subphase)
#define MEMOBJ_TRAVERSAL(n) ((n)->traversal)
#define MEMOBJ_USEDBIT(n) ((n)->flags.IsUsed)
#define MEMOBJ_SHAREDBIT(n) ((n)->flags.IsShared)
#define MEMOBJ_REPORTED(n) ((n)->flags.IsReported)

#define SHIFT2ORIG(n) ((memobj **)((char *)n - malloc_align_step))
#define SHIFT2MEMOBJ(n) (*(SHIFT2ORIG (n)))

#define ORIG2SHIFT(n) ((char *)n + malloc_align_step)

static void CHKManalyzeMemtab (memobj *, int);

static char *MemobjToErrorMessage (char *, memobj *);

static bool memcheck = FALSE;
static memobj *memtab = NULL;
static int memfreeslots = 0;
static int memindex = 0;
static int memtabsize = 0;

static int all_cnt_sharedmem = 0;
static int all_cnt_node_spaceleaks = 0;
static int all_cnt_non_node_spaceleaks = 0;

/** <!--********************************************************************-->
 *
 * @fn memobj *AllocateMemtab( int memtabsize)
 *
 *
 *
 *****************************************************************************/
static memobj *
AllocateMemtab (int memtabsize)
{
    memobj *memtab;
    int size;

    DBUG_ENTER ("AllocateMemtab");

    size = memtabsize * sizeof (memobj);

    if (global.current_allocated_mem + size < global.current_allocated_mem) {
        DBUG_ASSERT ((0), "counter for allocated memory: overflow detected");
    }

    global.current_allocated_mem += size;

    if (global.max_allocated_mem < global.current_allocated_mem) {
        global.max_allocated_mem = global.current_allocated_mem;
    }

    memtab = (memobj *)malloc (size);

    DBUG_PRINT ("MEM_ALLOC", ("Alloc memory: %d Bytes at adress: " F_PTR, size, memtab));

    DBUG_PRINT ("MEM_TOTAL",
                ("Currently allocated memory: %u", global.current_allocated_mem));

    DBUG_RETURN (memtab);
}

/** <!--********************************************************************-->
 *
 * @fn memobj *FreeMemtab( memobj *memtab, int memtabsize)
 *
 *
 *
 *****************************************************************************/
static void *
FreeMemtab (memobj *memtab, int memtabsize)
{
    int size;

    DBUG_ENTER ("FreeMemtab");

    size = memtabsize * sizeof (memobj);

    DBUG_PRINT ("MEM_ALLOC", ("Free memory: %d Bytes at adress: " F_PTR, size, memtab));

    if (global.current_allocated_mem < global.current_allocated_mem - size) {
        DBUG_ASSERT ((0), "counter for allocated memory: overflow detected");
    }

    global.current_allocated_mem -= size;

    DBUG_PRINT ("MEM_TOTAL",
                ("Currently allocated memory: %u", global.current_allocated_mem));
    free (memtab);

    DBUG_RETURN (NULL);
}

/** <!--********************************************************************-->
 *
 * @fn void CHKMinitialize( int argc, char *argv[])
 *
 * the initial function
 *
 *****************************************************************************/
void
CHKMinitialize (int argc, char *argv[])
{
    int i;

    DBUG_ENTER ("CHKMinitialize");

    for (i = 0; i < (argc - 1); i++) {
        if (strcmp (argv[i], "-d") == 0) {
            if (strcmp (argv[i + 1], "memcheck") == 0) {
                memcheck = TRUE;
            }
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn bool CHKMisMemcheckActive()
 *
 * the initial function
 *
 *****************************************************************************/
bool
CHKMisMemcheckActive ()
{
    DBUG_ENTER ("CHKMisMemcheckActive");
    DBUG_RETURN (memcheck);
}

/** <!--********************************************************************-->
 *
 * @fn void *CHKMdeinitialize()
 *
 * the initial function
 *
 *****************************************************************************/
void
CHKMdeinitialize ()
{
    DBUG_ENTER ("CHKMdeinitialize");

    if (memcheck) {
        global.current_allocated_mem -= memtabsize * sizeof (memobj);
    }

    DBUG_VOID_RETURN;
}

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

    info *info;

    DBUG_ENTER ("CHKMdoMemCheck");

    DBUG_PRINT ("CHKM", ("Traversing heap..."));

    info = MakeInfo ();

    TRAVpush (TR_chkm);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

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
    memobj *ptr_to_memobj;

    DBUG_ENTER ("CHKMregisterMem");

    if (memcheck) {

        shifted_ptr = ORIG2SHIFT (orig_ptr);

        /* change the memtabsize if the memtab is full or empty */
        if (memindex == memtabsize) {

            /* at beginning the memtab ist empty so: */
            if (memtabsize == 0) {

                DBUG_PRINT ("CHKM", ("Allocating memtab for 1000 memory objects"
                                     " size: %d bytes",
                                     1000 * sizeof (memobj)));

                memtab = AllocateMemtab (1000);
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

                newtab = AllocateMemtab (newtabsize);

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
                memtab = FreeMemtab (memtab, memtabsize);
                memtab = newtab;
                memindex = newindex;
                memfreeslots = newtabsize - newindex;
                memtabsize = newtabsize;
            }
        }

        /* <<<<<<<<  default entry for the memtab  >>>>>>>>> */

        ptr_to_memobj = memtab + memindex;

        MEMOBJ_PTR (ptr_to_memobj) = orig_ptr;
        MEMOBJ_SIZE (ptr_to_memobj) = size;
        MEMOBJ_NODETYPE (ptr_to_memobj) = N_undefined;
        MEMOBJ_SUBPHASE (ptr_to_memobj) = global.compiler_subphase;
        MEMOBJ_TRAVERSAL (ptr_to_memobj) = TRAVgetName ();
        MEMOBJ_USEDBIT (ptr_to_memobj) = FALSE;
        MEMOBJ_SHAREDBIT (ptr_to_memobj) = FALSE;
        MEMOBJ_REPORTED (ptr_to_memobj) = FALSE;

        *(memobj **)orig_ptr = ptr_to_memobj;

        memfreeslots = memfreeslots - 1;
        memindex = memindex + 1;
    } else {

        *(int *)orig_ptr = size;

        shifted_ptr = ORIG2SHIFT (orig_ptr);
    }

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
    memobj *ptr_to_memobj;
    void *orig_ptr;

    DBUG_ENTER ("CHKMunregisterMEM");

    orig_ptr = SHIFT2ORIG (shifted_ptr);

    if (memcheck) {
        ptr_to_memobj = SHIFT2MEMOBJ (shifted_ptr);

        if ((MEMOBJ_SIZE (ptr_to_memobj) == 0) && (MEMOBJ_PTR (ptr_to_memobj) == NULL)) {
            CTIwarn ("%s", "double free"); /* miss where */
        }

        MEMOBJ_SIZE (ptr_to_memobj) = 0;
        MEMOBJ_PTR (ptr_to_memobj) = NULL;
        memfreeslots = memfreeslots + 1;
    }

    DBUG_RETURN ((void *)orig_ptr);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKMpreffun( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CHKMprefun (node *arg_node, info *arg_info)
{
    memobj *ptr_to_memobj;

    DBUG_ENTER ("CHKMprefun");

    ptr_to_memobj = SHIFT2MEMOBJ (arg_node);

    if ((memtab <= ptr_to_memobj) && (ptr_to_memobj < memtab + memtabsize)
        && (MEMOBJ_PTR (ptr_to_memobj) == SHIFT2ORIG (arg_node))) {

        if (MEMOBJ_USEDBIT (ptr_to_memobj)) {
            MEMOBJ_SHAREDBIT (ptr_to_memobj) = TRUE;
        } else {
            MEMOBJ_USEDBIT (ptr_to_memobj) = TRUE;
        }
    } else {
        /*
         * this is a Zombie --> dangling pointer
         */
        if (INFO_ERROR (arg_info) == NULL) {
            INFO_ERROR (arg_info)
              = CHKinsertError (INFO_ERROR (arg_info), "Dangling Pointer");
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CHKMpostfun() node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CHKMpostfun (node *arg_node, info *arg_info)
{
    memobj *ptr_to_memobj;

    DBUG_ENTER ("CHKMpostfun");

    ptr_to_memobj = SHIFT2MEMOBJ (arg_node);

    if ((memtab <= ptr_to_memobj) && (ptr_to_memobj < memtab + memtabsize)
        && (MEMOBJ_PTR (ptr_to_memobj) == SHIFT2ORIG (arg_node))
        && (INFO_ERROR (arg_info) != NULL)) {

        ERROR_NEXT (INFO_ERROR (arg_info)) = NODE_ERROR (arg_node);
        NODE_ERROR (arg_node) = INFO_ERROR (arg_info);
        INFO_ERROR (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static void *CHKManalyzeMemtab( memobj *arg_memtab, int memindex)
 *
 *****************************************************************************/
static void
CHKManalyzeMemtab (memobj *arg_memtab, int arg_memindex)
{
    int index;
    int orig_tab_index;
    memobj *ptr_to_memobj;
    memobj *copy_memtab;

    memobj *orig_ptr_to_memobj;
    memobj *copy_ptr_to_memobj;

    int copy_index;
    node *arg_node;
    char *memtab_info;

    int cnt_sharedmem;
    int cnt_node_spaceleaks;
    int cnt_non_node_spaceleaks;

    DBUG_ENTER ("CHKManalyzeMemtab");

    /************************************
       must copy the memtab(!) to freeze, because Node_Error expand again the
       memtab and so can possibly change the back pointers
    **********************************/
    copy_memtab = AllocateMemtab (arg_memindex);

    for (index = 0; index < arg_memindex; index++) {
        copy_memtab[index] = arg_memtab[index];
        MEMOBJ_USEDBIT (arg_memtab + index) = FALSE;
        MEMOBJ_SHAREDBIT (arg_memtab + index) = FALSE;
    }

    copy_index = arg_memindex;
    cnt_sharedmem = 0;
    cnt_node_spaceleaks = 0;
    cnt_non_node_spaceleaks = 0;

    for (index = 0; index < copy_index; index++) {

        ptr_to_memobj = copy_memtab + index;

        if (MEMOBJ_PTR (ptr_to_memobj) != NULL) {

            if (MEMOBJ_NODETYPE (ptr_to_memobj) != N_undefined) {

                arg_node = (node *)ORIG2SHIFT (MEMOBJ_PTR (ptr_to_memobj));

                /*
                 * Nodetypes
                 */
                if (!MEMOBJ_USEDBIT (ptr_to_memobj)) {

                    if (!MEMOBJ_REPORTED (ptr_to_memobj)) {

                        MEMOBJ_REPORTED (ptr_to_memobj) = TRUE;
                        memtab_info
                          = MemobjToErrorMessage ("Node spaceleak:", ptr_to_memobj);

                        CTIwarn ("%s", memtab_info);
                        PRTdoPrint (arg_node);

                        memtab_info = ILIBfree (memtab_info);
                    }
                    cnt_node_spaceleaks++;
                } else {
                    if (MEMOBJ_SHAREDBIT (ptr_to_memobj)) {

                        if (!MEMOBJ_REPORTED (ptr_to_memobj)) {

                            MEMOBJ_REPORTED (ptr_to_memobj) = TRUE;
                            memtab_info = MemobjToErrorMessage ("illegal memory sharing:",
                                                                ptr_to_memobj);

                            NODE_ERROR (arg_node)
                              = CHKinsertError (NODE_ERROR (arg_node), memtab_info);
                            memtab_info = ILIBfree (memtab_info);
                        }
                        cnt_sharedmem++;
                    }
                }
            } else {
                /*
                 * not Nodetypes
                 */
                if (!MEMOBJ_USEDBIT (ptr_to_memobj)) {

                    if (!MEMOBJ_REPORTED (ptr_to_memobj)) {

                        MEMOBJ_REPORTED (ptr_to_memobj) = TRUE;

                        memtab_info
                          = MemobjToErrorMessage ("Non-node spaceleak:", ptr_to_memobj);

                        CTIwarn ("%s", memtab_info);

                        memtab_info = ILIBfree (memtab_info);
                    }
                    cnt_non_node_spaceleaks++;
                }
            }
        }
    }

    /*
     * transfer
     */
    orig_tab_index = 0;

    for (index = 0; index < copy_index; index++) {

        copy_ptr_to_memobj = copy_memtab + index;

        if (MEMOBJ_REPORTED (copy_ptr_to_memobj)) {

            while ((orig_tab_index < memindex)
                   && (MEMOBJ_PTR (copy_ptr_to_memobj)
                       != MEMOBJ_PTR (memtab + orig_tab_index))) {
                orig_tab_index++;
            }

            if (orig_tab_index < memindex) {
                MEMOBJ_REPORTED (orig_ptr_to_memobj) = TRUE;
                orig_tab_index++;
            }
        }
    }

    copy_memtab = FreeMemtab (copy_memtab, copy_index);

    if ((cnt_node_spaceleaks != 0) || (cnt_sharedmem != 0) || (cnt_non_node_spaceleaks)) {

        CTIwarn (">>>>> counter node spaceleaks: %d , "
                 "counter non-node spaceleaks: %d, "
                 "counter illegal memory sharing: %d",
                 cnt_node_spaceleaks, cnt_non_node_spaceleaks, cnt_sharedmem);

        CTIwarn (">>>>> Counter all node spaceleaks: %d , "
                 "counter all non-node spaceleaks: %d, "
                 "all illegal memory sharing: %d",
                 all_cnt_node_spaceleaks, all_cnt_non_node_spaceleaks, all_cnt_sharedmem);
    }
    all_cnt_node_spaceleaks += cnt_node_spaceleaks;
    all_cnt_non_node_spaceleaks += cnt_non_node_spaceleaks;
    all_cnt_sharedmem += cnt_sharedmem;

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
    memobj *ptr_to_memobj;

    DBUG_ENTER ("CHKMsetNodeType");

    if (memcheck) {

        ptr_to_memobj = SHIFT2MEMOBJ (shifted_ptr);

        MEMOBJ_NODETYPE (ptr_to_memobj) = newnodetype;
    }

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
    memobj *ptr_to_memobj;

    DBUG_ENTER ("CHKMsetLocation");

    if (memcheck) {

        ptr_to_memobj = SHIFT2MEMOBJ (shifted_ptr);

        MEMOBJ_FILE (ptr_to_memobj) = file;

        MEMOBJ_LINE (ptr_to_memobj) = line;
    }

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
    memobj *ptr_to_memobj;
    int *ptr_to_size;
    int size;

    DBUG_ENTER ("CHKMgetSize");

    if (memcheck) {

        ptr_to_memobj = SHIFT2MEMOBJ (shifted_ptr);

        size = MEMOBJ_SIZE (ptr_to_memobj);
    } else {

        ptr_to_size = (int *)SHIFT2ORIG (shifted_ptr);

        size = *ptr_to_size;
    }

    DBUG_RETURN (size);
}

static char *
MemobjToErrorMessage (char *kind_of_error, memobj *ptr_to_memobj)
{
    char *str;
    int test = 0;

    DBUG_ENTER ("CHKMtoString");

    str = (char *)ILIBmalloc (sizeof (char) * 1024);

    test = snprintf (str, 1024,
                     "%s Address: 0x%x, allocated at: %s:%d, "
                     "Traversal: %s, Subphase: %s\n",
                     kind_of_error, (unsigned int)MEMOBJ_PTR (ptr_to_memobj),
                     MEMOBJ_FILE (ptr_to_memobj), MEMOBJ_LINE (ptr_to_memobj),
                     MEMOBJ_TRAVERSAL (ptr_to_memobj),
                     PHsubPhaseName (MEMOBJ_SUBPHASE (ptr_to_memobj)));

    DBUG_ASSERT (test < 1024, "buffer is too small");

    DBUG_RETURN (str);
};

#endif /* SHOW_MALLOC */
