/*
 *
 * $Log$
 * Revision 1.2  2004/11/08 14:20:38  sah
 * moved some code
 *
 * Revision 1.1  2004/11/07 18:04:33  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "resolvepragma.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "stringset.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "Error.h"

/*
 * INFO structure
 */
struct INFO {
    node *module;
};

/*
 * INFO macros
 */
#define INFO_RSP_MODULE(n) ((n)->module)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_RSP_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

static int *
Nums2BoolArray (int line, int size, nums *numsp)
{
    int *ret, i;
    nums *tmp;

    DBUG_ENTER ("Nums2BoolArray");

    ret = (int *)Malloc (size * sizeof (int));

    DBUG_PRINT ("PRAGMA", ("Converting nums to bool[%d]", size));

    for (i = 0; i < size; i++) {
        ret[i] = 0;
    }

    tmp = numsp;
    i = 1;

    while (tmp != NULL) {
        DBUG_PRINT ("PRAGMA", ("Nums value is %d", NUMS_NUM (tmp)));

        if ((NUMS_NUM (tmp) < 0) || (NUMS_NUM (tmp) >= size)) {
            ERROR (line, ("Invalid argument of pragma 'readonly` or 'refcounting`:"));
            CONT_ERROR (
              ("Entry no.%d does not match a function parameter !", i, NUMS_NUM (tmp)));
        } else {
            ret[NUMS_NUM (tmp)] = 1;
        }

        tmp = NUMS_NEXT (tmp);
        i++;
    }

    FreeAllNums (numsp);

    DBUG_RETURN (ret);
}

static int *
Nums2IntArray (int line, int size, nums *numsp)
{
    int *ret, i;
    nums *tmp;

    DBUG_ENTER ("Nums2IntArray");

    ret = (int *)Malloc (size * sizeof (int));

    DBUG_PRINT ("PRAGMA", ("Converting nums to int[%d]", size));

    for (i = 0, tmp = numsp; (i < size) && (tmp != NULL); i++, tmp = NUMS_NEXT (tmp)) {
        DBUG_PRINT ("PRAGMA", ("Nums value is %d", NUMS_NUM (tmp)));

        if ((NUMS_NUM (tmp) < 0) || (NUMS_NUM (tmp) > size)) {
            ERROR (line, ("Invalid argument of pragma 'linksign`"));
            CONT_ERROR (
              ("Entry no.%d does not match a valid parameter position !", i + 1));
        }

        ret[i] = NUMS_NUM (tmp);
    }

    if (i < size) {
        ERROR (line, ("Invalid argument of pragma 'linksign`"));
        CONT_ERROR (("Less entries (%d) than parameters of function (%d) !", i, size));
    }

    if (tmp != NULL) {
        do {
            i++;

            DBUG_PRINT ("PRAGMA", ("Nums value is %d", NUMS_NUM (tmp)));

            tmp = NUMS_NEXT (tmp);
        } while (tmp != NULL);

        ERROR (line, ("Invalid argument of pragma 'linksign`:"));
        CONT_ERROR (("More entries (%d) than function parameters (%d) !", i, size));
    }

    FreeAllNums (numsp);

    DBUG_RETURN (ret);
}

static node *
ResolvePragmaReadonly (node *arg_node, node *pragma)
{
    int cnt;
    node *tmp_args;
    types *tmp_types;

    DBUG_ENTER ("ResolvePragmaReadonly");

    DBUG_PRINT ("PRAGMA",
                ("Resolving pragma readonly for function %s", ItemName (arg_node)));

    PRAGMA_READONLY (pragma)
      = Nums2BoolArray (NODE_LINE (arg_node), PRAGMA_NUMPARAMS (pragma),
                        PRAGMA_READONLYNUMS (pragma));

    cnt = 0;
    tmp_types = FUNDEF_TYPES (arg_node);

    while (tmp_types != NULL) {
        cnt++;
        tmp_types = TYPES_NEXT (tmp_types);
    }

    tmp_args = FUNDEF_ARGS (arg_node);

    while (tmp_args != NULL) {
        if (PRAGMA_READONLY (pragma)[cnt]) {
            if (ARG_ATTRIB (tmp_args) == ST_reference) {
                ARG_ATTRIB (tmp_args) = ST_readonly_reference;
            } else {
                WARN (NODE_LINE (arg_node),
                      ("Parameter no. %d of function '%s` is not a reference "
                       "parameter, so pragma 'readonly` has no effect on it",
                       cnt, ItemName (arg_node)));
            }
        }

        tmp_args = ARG_NEXT (tmp_args);
        cnt++;
    }

    PRAGMA_READONLY (pragma) = Free (PRAGMA_READONLY (pragma));

    DBUG_RETURN (arg_node);
}

node *
RSPFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSPFundef");

    DBUG_PRINT ("RSP", ("Processing function '%s'...", FUNDEF_NAME (arg_node)));

    if (FUNDEF_PRAGMA (arg_node) != NULL) {
        node *pragma = FUNDEF_PRAGMA (arg_node);

        DBUG_PRINT ("RSP",
                    ("Processing pragmas for function '%s'...", FUNDEF_NAME (arg_node)));

        PRAGMA_NUMPARAMS (pragma) = CountFunctionParams (arg_node);

        if (PRAGMA_FREEFUN (pragma) != NULL) {
            WARN (NODE_LINE (arg_node), ("Pragma 'freefun` has no effect on function"));
            PRAGMA_FREEFUN (pragma) = Free (PRAGMA_FREEFUN (pragma));
        }

        if (PRAGMA_INITFUN (pragma) != NULL) {
            WARN (NODE_LINE (arg_node), ("Pragma 'initfun` has no effect on function"));
            PRAGMA_INITFUN (pragma) = Free (PRAGMA_INITFUN (pragma));
        }

        if (PRAGMA_LINKSIGNNUMS (pragma) != NULL) {
            PRAGMA_LINKSIGN (pragma)
              = Nums2IntArray (NODE_LINE (arg_node), PRAGMA_NUMPARAMS (pragma),
                               PRAGMA_LINKSIGNNUMS (pragma));

            PRAGMA_LINKSIGNNUMS (pragma) = FreeAllNums (PRAGMA_LINKSIGNNUMS (pragma));
        }

        if (PRAGMA_REFCOUNTINGNUMS (pragma) != NULL) {
            PRAGMA_REFCOUNTING (pragma)
              = Nums2BoolArray (NODE_LINE (arg_node), PRAGMA_NUMPARAMS (pragma),
                                PRAGMA_REFCOUNTINGNUMS (pragma));

            PRAGMA_REFCOUNTINGNUMS (pragma)
              = FreeAllNums (PRAGMA_REFCOUNTINGNUMS (pragma));
        }

        if (PRAGMA_READONLYNUMS (pragma) != NULL) {
            arg_node = ResolvePragmaReadonly (arg_node, pragma);

            PRAGMA_READONLYNUMS (pragma) = FreeAllNums (PRAGMA_READONLYNUMS (pragma));
        }

        /*
         * if this function needs an external module, add it to
         * the external dependencies of this module.
         */
        if (PRAGMA_LINKMOD (pragma) != NULL) {
            MODUL_DEPENDENCIES (INFO_RSP_MODULE (arg_info))
              = SSAdd (PRAGMA_LINKMOD (pragma), SS_extlib,
                       MODUL_DEPENDENCIES (INFO_RSP_MODULE (arg_info)));

            PRAGMA_LINKMOD (pragma) = Free (PRAGMA_LINKMOD (pragma));
        }

        /*
         * if this function is defined by an external object file,
         * add it to the dependencies
         */
        if (PRAGMA_LINKOBJ (pragma) != NULL) {
            MODUL_DEPENDENCIES (INFO_RSP_MODULE (arg_info))
              = SSAdd (PRAGMA_LINKOBJ (pragma), SS_objfile,
                       MODUL_DEPENDENCIES (INFO_RSP_MODULE (arg_info)));

            PRAGMA_LINKOBJ (pragma) = Free (PRAGMA_LINKOBJ (pragma));
        }

        /*
         * TODO: implement TOUCH and EFFECT, as soon as classes work
         */

        if ((PRAGMA_LINKNAME (pragma) == NULL) && (PRAGMA_LINKOBJ (pragma) == NULL)
            && (PRAGMA_LINKNAME (pragma) == NULL) && (PRAGMA_LINKMOD (pragma) == NULL)
            && (PRAGMA_LINKSIGN (pragma) == NULL)
            && (PRAGMA_REFCOUNTING (pragma) == NULL)) {
            FUNDEF_PRAGMA (arg_node) = FreeNode (pragma);
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSPModul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSPModul");

    INFO_RSP_MODULE (arg_info) = arg_node;

    if (MODUL_FUNDECS (arg_node) != NULL) {
        MODUL_FUNDECS (arg_node) = Trav (MODUL_FUNDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

void
DoResolvePragmas (node *syntax_tree)
{
    funtab *store_tab;
    info *info;

    DBUG_ENTER ("DoResolvePragmas");

    store_tab = act_tab;
    act_tab = rsp_tab;

    info = MakeInfo ();

    syntax_tree = Trav (syntax_tree, info);

    info = FreeInfo (info);

    act_tab = store_tab;

    DBUG_VOID_RETURN;
}
