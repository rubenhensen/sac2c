/*
 *
 * $Log$
 * Revision 1.7  1995/12/20 08:19:59  cg
 * converted usage of macro WITH_BODY to WITH_OPERATOR
 *
 * Revision 1.6  1995/11/10  15:05:38  cg
 * converted to new error macros
 *
 * Revision 1.5  1995/11/06  19:00:29  cg
 * debug facilities added
 * bug in functions CheckDefined and CheckApplied fixed
 * running properly !!
 *
 * Revision 1.4  1995/11/06  14:21:13  cg
 * first working revision
 *
 * Revision 1.3  1995/11/06  09:23:57  cg
 * first compilable revision
 *
 * Revision 1.2  1995/11/03  16:40:41  cg
 * added traversal mechanism and check functions,
 * but still not compilable.
 *
 * Revision 1.1  1995/11/02  16:57:08  cg
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

/************************************************************************
 *  local type definitions
 ************************************************************************/

typedef enum {
    H_then_enter,
    H_then_leave,
    H_else_enter,
    H_else_leave,
    H_while_enter,
    H_while_leave,
    H_while_skipped,
    H_while_repeat,
    H_with_enter,
    H_with_leave,
    H_with_skipped,
    H_do_enter,
    H_do_leave,
    H_do_repeat
} historytype;

typedef struct HISTORYLIST {
    historytype history;
    struct HISTORYLIST *next;
} historylist;

typedef struct UNQSTATELIST {
    int *state;
    historylist *history;
    struct UNQSTATELIST *next;
} unqstatelist;

/************************************************************************
 *  basic access macros for local types
 ************************************************************************/

#define HL_HISTORY(l) (l->history)
#define HL_NEXT(l) (l->next)

#define UNQ_STATE(l) (l->state)
#define UNQ_HISTORY(l) (l->history)
#define UNQ_NEXT(l) (l->next)

/************************************************************************
 *  more local macro definitions
 ************************************************************************/

/*
 *
 *  macro name    : IDS_IS_UNIQUE
 *  arg types     : 1) ids*
 *  result type   : int (bool)
 *  description   : checks if the given ids is unique or not
 *  global vars   : ---
 *  funs          : ---
 *
 *  remarks       : This macro will only work properly with the VARNO
 *                  settings done in uniquecheck.c. For this reason
 *                  it was located in this file and not in
 *                  tree_compound.h
 *
 */

#define IDS_IS_UNIQUE(i)                                                                 \
    ((NODE_TYPE (IDS_VARDEC (i)) == N_vardec) ? (VARDEC_VARNO (IDS_VARDEC (i)) >= 0)     \
                                              : (ARG_VARNO (IDS_VARDEC (i)) >= 0))

/*
 *
 *  macro name    : ID_IS_UNIQUE
 *  arg types     : 1) node* (N_id)
 *  result type   : int (bool)
 *  description   : checks if the given id is unique or not
 *  global vars   : ---
 *  funs          : ---
 *
 *  remarks       : This macro will only work properly with the VARNO
 *                  settings done in uniquecheck.c. For this reason
 *                  it was located in this file and not in
 *                  tree_compound.h
 *
 */

#define ID_IS_UNIQUE(n)                                                                  \
    ((NODE_TYPE (ID_VARDEC (n)) == N_vardec) ? (VARDEC_VARNO (ID_VARDEC (n)) >= 0)       \
                                             : (ARG_VARNO (ID_VARDEC (n)) >= 0))

/************************************************************************
 *  global variable definitions
 ************************************************************************/

static int varno; /* to count arguments and local variables */
static int argno; /* to count arguments */
static unqstatelist *unqstate;

/************************************************************************
 *  The 'main' function of this compiler module
 ************************************************************************/

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

    act_tab = unique_tab;

    DBUG_RETURN (Trav (syntax_tree, NULL));
}

/************************************************************************
 *  Basic manipulation functions for local data structures
 ************************************************************************/

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

void
FreeUnqstate (unqstatelist *unqstate)
{
    unqstatelist *tmp;
    historylist *tmp_hist, *hist;

    DBUG_ENTER ("FreeUnqstate");

    while (unqstate != NULL) {
        tmp = unqstate;
        unqstate = UNQ_NEXT (unqstate);
        hist = UNQ_HISTORY (tmp);

        while (hist != NULL) {
            tmp_hist = hist;
            hist = HL_NEXT (hist);
            free (tmp_hist);
        }

        free (UNQ_STATE (tmp));
        free (tmp);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PrintUnqstate
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
PrintUnqstate (unqstatelist *unq)
{
    int i;

    DBUG_ENTER ("PrintUnqstate");

    while (unq != NULL) {
        for (i = 0; i < varno; i++)
            fprintf (stderr, "%d ", UNQ_STATE (unq)[i]);
        fprintf (stderr, "\n");
        unq = UNQ_NEXT (unq);
    }

    DBUG_VOID_RETURN;
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

unqstatelist *
CopyUnqstate (unqstatelist *unqstate)
{
    unqstatelist *new;
    int i;

    DBUG_ENTER ("CopyUnqstate");

    new = (unqstatelist *)Malloc (sizeof (unqstatelist));

    UNQ_HISTORY (new) = UNQ_HISTORY (unqstate);

    UNQ_STATE (new) = (int *)Malloc (varno * sizeof (int));

    for (i = 0; i < varno; i++) {
        UNQ_STATE (new)[i] = UNQ_STATE (unqstate)[i];
    }

    if (UNQ_NEXT (unqstate) == NULL) {
        UNQ_NEXT (new) = NULL;
    } else {
        UNQ_NEXT (new) = CopyUnqstate (UNQ_NEXT (unqstate));
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
            tmp = second;
            second = UNQ_NEXT (second);
            UNQ_NEXT (tmp) = NULL;
            FreeUnqstate (tmp);
        }
    }

    DBUG_RETURN (first);
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
AddHistory (unqstatelist *unq, historytype history)
{
    DBUG_ENTER ("AddHistory");

    while (unq != NULL) {
        UNQ_HISTORY (unq) = MakeHistory (history, UNQ_HISTORY (unq));
        unq = UNQ_NEXT (unq);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PrintHistory
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
PrintHistory (historylist *history)
{
    DBUG_ENTER ("PrintHistory");

    if (history == NULL) {
        CONT_ERROR (("Uniqueness check path:"));
        CONT_ERROR (("-> function body"));
        message_indent = 2;
    } else {
        PrintHistory (HL_NEXT (history));

        switch (HL_HISTORY (history)) {
        case H_then_enter:
            CONT_ERROR (("-> then-branch"));
            message_indent += 2;
            break;

        case H_then_leave:
            message_indent -= 2;
            CONT_ERROR (("<- then-branch"));
            break;

        case H_else_enter:
            CONT_ERROR (("-> else-branch"));
            message_indent += 2;
            break;

        case H_else_leave:
            message_indent -= 2;
            CONT_ERROR (("<- else-branch"));
            break;

        case H_while_enter:
            CONT_ERROR (("-> while/for-loop"));
            message_indent += 2;
            break;

        case H_while_leave:
            message_indent -= 2;
            CONT_ERROR (("<- while/for-loop"));
            break;

        case H_while_skipped:
            CONT_ERROR (("-- skipping while/for-loop"));
            break;

        case H_while_repeat:
            CONT_ERROR (("-- repeating while/for-loop"));
            break;

        case H_do_enter:
            CONT_ERROR (("-> do-loop"));
            message_indent += 2;
            break;

        case H_do_leave:
            message_indent -= 2;
            CONT_ERROR (("<- do-loop"));
            break;

        case H_do_repeat:
            CONT_ERROR (("-- repeating do-loop"));
            break;

        case H_with_enter:
            CONT_ERROR (("-> with-loop"));
            message_indent += 2;
            break;

        case H_with_leave:
            message_indent -= 2;
            CONT_ERROR (("<- with-loop"));
            break;

        case H_with_skipped:
            CONT_ERROR (("-- skipping with-loop"));
        }
    }

    DBUG_VOID_RETURN;
}

/************************************************************************
 *  The check functions for defined and applied occurrences of objects
 ************************************************************************/

/*
 *
 *  functionname  : CheckDefined
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
CheckDefined (ids *var, int line)
{
    unqstatelist *tmp = unqstate;
    int not_yet_warned = 1, number;

    DBUG_ENTER ("CheckDefined");

    number = (NODE_TYPE (IDS_VARDEC (var)) == N_vardec) ? VARDEC_VARNO (IDS_VARDEC (var))
                                                        : ARG_VARNO (IDS_VARDEC (var));

    while (tmp != NULL) {
        if ((UNQ_STATE (tmp)[number] == 1) && not_yet_warned) {
            ERROR (line, ("Object '%s` already existing", IDS_NAME (var)));
            PrintHistory (UNQ_HISTORY (tmp));
            message_indent = 0;
            not_yet_warned = 0;
        } else {
            UNQ_STATE (tmp)[number] = 1;
        }

        tmp = UNQ_NEXT (tmp);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : CheckApplied
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
CheckApplied (node *var)
{
    unqstatelist *tmp = unqstate;
    int not_yet_errored = 1, number;
    statustype attrib;

    DBUG_ENTER ("CheckApplied");

    number = (NODE_TYPE (ID_VARDEC (var)) == N_vardec) ? VARDEC_VARNO (ID_VARDEC (var))
                                                       : ARG_VARNO (ID_VARDEC (var));

    attrib = (NODE_TYPE (ID_VARDEC (var)) == N_vardec) ? VARDEC_ATTRIB (ID_VARDEC (var))
                                                       : ARG_ATTRIB (ID_VARDEC (var));

    if (attrib != ST_readonly_reference) {
        while (tmp != NULL) {
            if ((UNQ_STATE (tmp)[number] == 0) && not_yet_errored) {
                ERROR (NODE_LINE (var), ("Object '%s` already deleted", ID_NAME (var)));
                CONT_ERROR ((" -> Uniqueness Violation <-"));

                PrintHistory (UNQ_HISTORY (tmp));
                message_indent = 0;
                not_yet_errored = 0;
            } else {
                UNQ_STATE (tmp)[number] = 0;
            }

            tmp = UNQ_NEXT (tmp);
        }
    }

    DBUG_VOID_RETURN;
}

/************************************************************************
 *  The general traversal functions
 ************************************************************************/

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

        if (FUNDEF_VARDEC (arg_node) != NULL) {
            FUNDEF_VARDEC (arg_node) = Trav (FUNDEF_VARDEC (arg_node), arg_info);
        }

        if (varno > 0) {
            unqstate = InitUnqstate ();

            DBUG_PRINT ("UNQ", ("Uniqueness Check: %s", ItemName (arg_node)));

            DBUG_EXECUTE ("UNQ",
                          fprintf (stderr, "\nUnq-state at beginning of fun-body\n");
                          PrintUnqstate (unqstate););

            Trav (FUNDEF_BODY (arg_node), arg_info);

            DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nUnq-state at end of fun-body\n");
                          PrintUnqstate (unqstate););

            FreeUnqstate (unqstate);
        }
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
 *  functionname  : UNQlet
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
UNQlet (node *arg_node, node *arg_info)
{
    ids *tmp;

    DBUG_ENTER ("UNQlet");

    Trav (LET_EXPR (arg_node), arg_info);

    tmp = LET_IDS (arg_node);

    while (tmp != NULL) {
        if (IDS_IS_UNIQUE (tmp)) {
            CheckDefined (tmp, NODE_LINE (arg_node));
            if (arg_info != NULL) {
                WARN (NODE_LINE (arg_node),
                      ("Modification of object '%s` in body of with-loop "
                       "may cause non-deterministic results",
                       IDS_NAME (tmp)));
            }
        }

        tmp = IDS_NEXT (tmp);
    }

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nUnq-state after let\n");
                  PrintUnqstate (unqstate););

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNQid
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
UNQid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNQid");

    if (ID_IS_UNIQUE (arg_node)) {
        CheckApplied (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNQdo
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
UNQdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNQdo");

    AddHistory (unqstate, H_do_enter);
    Trav (DO_BODY (arg_node), arg_info);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nUnq-state after do-loop(1)\n");
                  PrintUnqstate (unqstate););

    AddHistory (unqstate, H_do_repeat);
    Trav (DO_BODY (arg_node), arg_info);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nUnq-state after do-loop(2)\n");
                  PrintUnqstate (unqstate););

    AddHistory (unqstate, H_do_leave);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNQwhile
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
UNQwhile (node *arg_node, node *arg_info)
{
    unqstatelist *skipped_state;

    DBUG_ENTER ("UNQwhile");

    skipped_state = CopyUnqstate (unqstate);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nskipped state of while\n");
                  PrintUnqstate (skipped_state););

    AddHistory (unqstate, H_while_enter);
    AddHistory (skipped_state, H_while_skipped);

    Trav (WHILE_BODY (arg_node), arg_info);
    AddHistory (unqstate, H_while_repeat);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nUnq-state after while(1)\n");
                  PrintUnqstate (unqstate););

    Trav (WHILE_BODY (arg_node), arg_info);

    AddHistory (unqstate, H_while_leave);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nUnq-state after while(2)\n");
                  PrintUnqstate (unqstate););

    unqstate = MergeUnqstate (skipped_state, unqstate);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nUnq-state after merging skip and while\n");
                  PrintUnqstate (unqstate););

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNQcond
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
UNQcond (node *arg_node, node *arg_info)
{
    unqstatelist *then_state, *else_state;

    DBUG_ENTER ("UNQcond");

    else_state = CopyUnqstate (unqstate);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nthen-state before cond\n");
                  PrintUnqstate (unqstate););

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nelse-state before cond\n");
                  PrintUnqstate (else_state););

    AddHistory (unqstate, H_then_enter);
    AddHistory (else_state, H_else_enter);

    Trav (COND_THEN (arg_node), arg_info);

    then_state = unqstate;
    unqstate = else_state;

    Trav (COND_ELSE (arg_node), arg_info);

    AddHistory (then_state, H_then_leave);
    AddHistory (unqstate, H_else_leave);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nthen-state after cond\n");
                  PrintUnqstate (then_state););

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nelse-state after cond\n");
                  PrintUnqstate (unqstate););

    unqstate = MergeUnqstate (then_state, unqstate);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nUnq-state after merging then and else\n");
                  PrintUnqstate (unqstate););

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : UNQwith
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
UNQwith (node *arg_node, node *arg_info)
{
    unqstatelist *skipped_state;

    DBUG_ENTER ("UNQwith");

    skipped_state = CopyUnqstate (unqstate);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nskipped state of with\n");
                  PrintUnqstate (skipped_state););

    AddHistory (unqstate, H_with_enter);
    AddHistory (skipped_state, H_with_skipped);

    Trav (WITH_OPERATOR (arg_node), arg_node);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nUnq-state after with\n");
                  PrintUnqstate (unqstate););

    AddHistory (unqstate, H_with_leave);

    unqstate = MergeUnqstate (skipped_state, unqstate);

    DBUG_EXECUTE ("UNQ", fprintf (stderr, "\nUnq-state after merging skip and with\n");
                  PrintUnqstate (unqstate););

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
