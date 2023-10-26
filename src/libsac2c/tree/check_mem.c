/******************************************************************************
 *
 * PREFIX: CHKM
 *
 * description:
 *   the checkmechanism for memory leaks
 *
 ******************************************************************************/

#ifndef DBUG_OFF
#include <stdlib.h>
#include "check_mem.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "check_lib.h"
#include "types_trav.h"
#include "phase_info.h"
#include "print.h"
#include "check_node.h"

#define DBUG_PREFIX "CHKM"
#include "debug.h"

#include "globals.h"
#include "ctinfo.h"
#include "memory.h"
#include "uthash.h"

extern mallocinfo_t *malloctable;
extern mallocphaseinfo_t phasetable[];

void
CHKMdeinitialize (void)
{
    DBUG_ENTER ();

    DBUG_RETURN ();
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

    DBUG_ENTER ();

    DBUG_PRINT ("Traversing syntax tree...");

    TRAVpush (TR_chkm);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_PRINT ("Syntax tree traversal complete");

    DBUG_PRINT ("Analyzing memory table...");

    CHKMcheckLeakedMemory ();

    DBUG_PRINT ("Analysis of memory table complete.");

    DBUG_RETURN (syntax_tree);
}

void
CHKMtouch (void *ptr, info *arg_info)
{
    if (global.memcheck) {
        mallocinfo_t *info;
        HASH_FIND_PTR (malloctable, &ptr, info);
        if (info) {
            info->wasintree = TRUE;
            info->isreachable = TRUE;
        }
    }
    return;
}

void
CHKMdoNotReport (void *shifted_ptr)
{
    return;
}

void
CHKMisNode (void *ptr, nodetype newnodetype)
{
    if (global.memcheck) {
        mallocinfo_t *info;
        HASH_FIND_PTR (malloctable, &ptr, info);
        if (info) {
            info->isnode = TRUE;
        }
    }
    return;
}

static void *
foldmemcheck (void *init, void *key, void *value)
{
    mallocinfo_t *info = value;
    mallocinfo_t *iterator;
    bool ispresent = FALSE;

    if (!info->wasintree) {
    } else if (info->isreachable) {
        info->isreachable = FALSE;
    } else { // was in tree, but not reachable
             // wasintree set ONLY by CHKMisNode (above)
        iterator = phasetable[global.compiler_anyphase].leaked;
        while (iterator) {
            if ((!strcmp (iterator->file, info->file))
                && iterator->line == info->line) { // Match on file and line #
                iterator->occurrence++;
                iterator->size += info->size;
                ispresent = TRUE;
                break;
            }
            iterator = iterator->next;
        }

        if (!ispresent) {
            info->next = phasetable[global.compiler_anyphase].leaked;
            phasetable[global.compiler_anyphase].leaked = info;
        }
        phasetable[global.compiler_anyphase].leakedsize += info->size;
        phasetable[global.compiler_anyphase].nleaked++;
        HASH_DEL (malloctable, info);
    }

    return NULL;
}

void
CHKMcheckLeakedMemory (void)
{
    mallocinfo_t *iter, *tmp;
    global.memcheck = FALSE;
    HASH_ITER (hh, malloctable, iter, tmp) {
        foldmemcheck (malloctable, iter->key, iter);
    }
    global.memcheck = TRUE;
}
#else /*DBUG_OFF*/
#include "check_mem.h"
#endif
