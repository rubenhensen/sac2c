/*
 *
 * $Log$
 * Revision 1.1  1995/11/02 16:57:08  cg
 * Initial revision
 *
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "dbug.h"
#include "traverse.h"
#include "internal_lib.h"
#include "Error.h"

#include <malloc.h>

/*
 *  local type definitions
 */

typedef enum { H_then, H_else, H_while, H_with, H_for } historytype;

typedef struct HISTORYLIST {
    historytype history;
    struct HISTORYLIST *next;
} historylist;

typedef struct UNQSTATELIST {
    int *state;
    historylist history;
    struct UNQSTATELIST *next;
} unqstatelist;

/*
 *  basic access macros for local types
 */

#define HL_HISTORY(l) (l->history)
#define HL_NEXT(l) (l->next)

#define UNQ_STATE(l) (l->state)
#define UNQ_HISTORY(l) (l->history)
#define UNQ_NEXT(l) (l->next)

/*
 *  global variable definitions
 */

static int varno; /* to count arguments and local variables */
static int argno; /* to count arguments */
static unqstatelist unqstate;

/*
 *
 *  functionname  : UniquenessCheck
 *  arguments     : 1) syntax tree
 *  description   : starts the traversal mechanism for the uniqueness
 *                  checker. The correctness of all object-related
 *                  code is checked, esp. uniqueness where necessary.
 *  global vars   : act_tab, unq_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG
 *
 *  remarks       :
 *
 */

node *
UniquenessCheck (node *syntax_tree)
{
    DBUG_ENTER ("UniquenessCheck");

    act_tab = unq_tab;

    DBUG_RETURN (Trav (syntax_tree, NULL));
}

/*
 *
 *  functionname  : MakeHistory
 *  arguments     : 1) history item
 *                  2) history list
 *  description   : creates a new historylist entry which is initialized
 *                  with (1) and concatenated with (2)
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc
 *  macros        : DBUG, HL
 *
 *  remarks       :
 *
 */

historylist *
MakeHistory (historytype history, historylist *old)
{
    historylist *tmp;

    DBUG_ENTER ("MakeHistory");

    tmp = (historylist *)Malloc (sizeof (historylist));

    HL_HISTORY (tmp) = history;
    HL_NEXT (tmp) = old;

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : InitUnqstate
 *  arguments     : ---
 *  description   : generates a 1-element unqstatelist
 *                  Used to initialize unqstate before traversing the
 *                  body of a function. For each variable of the function
 *                  (parameter and local) an integer is created and
 *                  initialized with 0 for locals (not defined) and
 *                  1 for parameters (defined).
 *  global vars   : argno, varno
 *  internal funs : ---
 *  external funs : Malloc
 *  macros        : DBUG, UNQ
 *
 *  remarks       :
 *
 */

unqstatelist *
InitUnqstate ()
{
    unqstatelist *tmp;
    int i;

    DBUG_ENTER ("InitUnqstate");

    tmp = (unqstatelist *)Malloc (sizeof (unqstatelist));
    UNQ_HISTORY (tmp) = NULL;
    UNQ_NEXT (tmp) = NULL;

    UNQ_STATE (tmp) = (int *)Malloc (varno * sizeof (int));

    for (i = 0; i < argno; i++) {
        UNQ_STATE (tmp)[i] = 1;
    }

    for (i = argno; i < varno; i++) {
        UNQ_STATE (tmp)[i] = 0;
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : CopyUnqstate
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

unqstatelist
CopyUnqstate (unqstatelist *old)
{
    unqstatelist *new;
    int i;

    DBUG_ENTER ("CopyUnqstate");

    new = (unqstatelist *)Malloc (sizeof (unqstatelist));

    UNQ_HISTORY (new) = UNQ_HISTORY (old);

    UNQ_STATE (new) = (int *)Malloc (varno * sizeof (int));

    for (i = 0; i < varno; i++) {
        UNQ_STATE (new)[i] = UNQ_STATE (old)[i]
    }

    if (UNQ_NEXT (old) == NULL) {
        UNQ_NEXT (new) = NULL;
    } else {
        UNQ_NEXT (new) = CopyUnqstate (UNQ_NEXT (old));
    }

    DBUG_RETURN (new);
}

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

int
CmpUnqstate (unqstatelist *first, unqstatelist *second)
{
    int i;

    DBUG_ENTER ("CmpUnqstate");

    for (i = 0; i < varno; i++) {
        if (UNQ_STATE (first)[i] != UNQ_STATE (second)[i]) {
            break;
        }
    }

    if (i < varno) {
        i = 0;
    }

    DBUG_RETURN (i);
}

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

unqstatelist *
MergeUnqstate (unqstatelist *first, unqstatelist *second)
{
    unqstatelist *tmp, *last;

    DBUG_ENTER ("MergeUnqstate");

    last = first;

    while (UNQ_NEXT (last) != NULL) {
        last = UNQ_NEXT (last);
    }

    while (second != NULL) {
        tmp = first;

        while (tmp != NULL) {
            if (CmpUnqstate (second, tmp)) {
                break;
            } else {
                tmp = UNQ_NEXT (tmp);
            }
        }

        if (tmp == NULL) {
            UNQ_NEXT (last) = second;
            last = second;
            second = UNQ_NEXT (second);
            UNQ_NEXT (last) = NULL;
        } else {
            second = UNQ_NEXT (second);
        }
    }

    DBUG_RETURN (first);
}

/*
 *
 *  functionname  : FreeUnqstate
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void *
FreeUnqstate (unqstatelist *unqstate)
{
    unqstatelist *tmp;

    DEBUG_ENTER ("FreeUnqstate");

    while (unqstate != NULL) {
        tmp = unqstate;
        unqstate = UNQ_NEXT (unqstate);
        free (UNQ_STATE (tmp));
        free (tmp);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AddHistory
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
AddHistory (historytype history)
{
    unqstatelist *tmp = unqstate;

    DBUG_ENTER ("AddHistory");

    while (tmp != NULL) {
        UNQ_HISTORY (tmp) = MakeHistory (history, UNQ_HISTORY (tmp));
        tmp = UNQ_NEXT (tmp);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : UNQmodul
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
UNQmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNQmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNQfundef
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
UNQfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNQfundef");

    if (FUNDEF_STATUS (arg_node) != ST_imported) {
        argno = 0;

        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        }

        varno = argno;

        Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNQblock
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
UNQblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNQblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNQvardec
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
UNQvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNQvardec");

    if (VARDEC_ATTRIB (arg_node) == ST_unique) {
        VARDEC_VARNO (arg_node) = varno;
        varno += 1;
    } else {
        VARDEC_VARNO (arg_node) = -1;
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNQarg
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
UNQarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNQarg");

    if (ARG_ATTRIB (arg_node) != ST_regular) {
        ARG_VARNO (arg_node) = argno;
        argno += 1;
    } else {
        ARG_VARNO (arg_node) = -1;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
