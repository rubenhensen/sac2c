/*
 *
 * $Id$
 *
 *
 */

#include "resolvepragma.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "stringset.h"
#include "free.h"
#include "ctinfo.h"
#include "dbug.h"

/*
 * INFO structure
 */
struct INFO {
    node *nums;
    int counter;
    enum { RSP_default, RSP_refcnt, RSP_linksign } travmode;
};

/*
 * INFO macros
 */
#define INFO_NUMS(n) ((n)->nums)
#define INFO_COUNTER(n) ((n)->counter)
#define INFO_TRAVMODE(n) ((n)->travmode)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_NUMS (result) = NULL;
    INFO_COUNTER (result) = 0;
    INFO_TRAVMODE (result) = RSP_default;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static void
CheckRefReadNums (int line, int size, node *nums)
{
    int i;
    node *tmp;

    DBUG_ENTER ("CheckRefReadNums");

    tmp = nums;
    i = 1;

    while (tmp != NULL) {
        DBUG_PRINT ("PRAGMA", ("Nums value is %d", NUMS_VAL (tmp)));

        if ((NUMS_VAL (tmp) < 0) || (NUMS_VAL (tmp) >= size)) {
            CTIerrorLine (line,
                          "Invalid argument of pragma 'readonly` or 'refcounting`: "
                          "Entry no. %d does not match a function parameter",
                          i, NUMS_VAL (tmp));
        }

        tmp = NUMS_NEXT (tmp);
        i++;
    }

    DBUG_VOID_RETURN;
}

static bool
CheckLinkSignNums (int line, int size, node *nums)
{
    int i;
    node *tmp;
    bool result = TRUE;

    DBUG_ENTER ("CheckLinkSignNums");

    for (i = 0, tmp = nums; (i < size) && (tmp != NULL); i++, tmp = NUMS_NEXT (tmp)) {
        DBUG_PRINT ("PRAGMA", ("Nums value is %d", NUMS_VAL (tmp)));

        if ((NUMS_VAL (tmp) < 0) || (NUMS_VAL (tmp) > size)) {
            CTIerrorLine (line,
                          "Invalid argument of pragma 'linksign`: "
                          "Entry no. %d does not match a valid parameter position",
                          i + 1);
            result = FALSE;
        }
    }

    if (i < size) {
        CTIerrorLine (line,
                      "Invalid argument of pragma 'linksign` :"
                      "Less entries (%d) than parameters of function (%d)",
                      i, size);
        result = FALSE;
    }

    if (tmp != NULL) {
        do {
            i++;

            DBUG_PRINT ("PRAGMA", ("Nums value is %d", NUMS_VAL (tmp)));

            tmp = NUMS_NEXT (tmp);
        } while (tmp != NULL);

        CTIerrorLine (line,
                      "Invalid argument of pragma 'linksign`: "
                      "More entries (%d) than function parameters (%d)",
                      i, size);
        result = FALSE;
    }

    DBUG_RETURN (result);
}

static node *
InitFundefRetsForExtern (node *rets)
{
    DBUG_ENTER ("InitFundefRetsForExtern");

    if (rets != NULL) {
        DBUG_ASSERT ((NODE_TYPE (rets) == N_ret), "found a non N_ret node");

        RET_ISREFCOUNTED (rets) = FALSE;

        RET_NEXT (rets) = InitFundefRetsForExtern (RET_NEXT (rets));
    }

    DBUG_RETURN (rets);
}

static node *
InitFundefArgsForExtern (node *args)
{
    DBUG_ENTER ("InitFundefArgsForExtern");

    if (args != NULL) {
        DBUG_ASSERT ((NODE_TYPE (args) == N_arg), "found a non N_arg node");

        ARG_ISREFCOUNTED (args) = FALSE;

        ARG_NEXT (args) = InitFundefArgsForExtern (ARG_NEXT (args));
    }

    DBUG_RETURN (args);
}

static node *
AnnotateRefcounting (node *arg_node, info *arg_info, node *nums)
{
    DBUG_ENTER ("AnnotateRefcounting");

    INFO_COUNTER (arg_info) = 0;
    INFO_NUMS (arg_info) = nums;
    INFO_TRAVMODE (arg_info) = RSP_refcnt;

    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    INFO_COUNTER (arg_info) = 0;
    INFO_NUMS (arg_info) = NULL;
    INFO_TRAVMODE (arg_info) = RSP_default;

    DBUG_RETURN (arg_node);
}

static node *
AnnotateLinksign (node *arg_node, info *arg_info, node *nums)
{
    DBUG_ENTER ("AnnotateLinksign");

    INFO_NUMS (arg_info) = nums;
    INFO_TRAVMODE (arg_info) = RSP_linksign;

    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    INFO_NUMS (arg_info) = NULL;
    INFO_TRAVMODE (arg_info) = RSP_default;

    DBUG_RETURN (arg_node);
}

node *
RSPret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSPret");

    if (INFO_TRAVMODE (arg_info) == RSP_refcnt) {
        if (TCnumsContains (INFO_COUNTER (arg_info), INFO_NUMS (arg_info))) {

            RET_ISREFCOUNTED (arg_node) = TRUE;
        }

        INFO_COUNTER (arg_info)++;
    } else if (INFO_TRAVMODE (arg_info) == RSP_linksign) {
        RET_LINKSIGN (arg_node) = NUMS_VAL (INFO_NUMS (arg_info));
        RET_HASLINKSIGNINFO (arg_node) = TRUE;

        INFO_NUMS (arg_info) = NUMS_NEXT (INFO_NUMS (arg_info));
    }

    if (RET_NEXT (arg_node) != 0) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSParg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSParg");

    if (INFO_TRAVMODE (arg_info) == RSP_refcnt) {
        if (TCnumsContains (INFO_COUNTER (arg_info), INFO_NUMS (arg_info))) {

            ARG_ISREFCOUNTED (arg_node) = TRUE;
        }

        INFO_COUNTER (arg_info)++;
    } else if (INFO_TRAVMODE (arg_info) == RSP_linksign) {
        ARG_LINKSIGN (arg_node) = NUMS_VAL (INFO_NUMS (arg_info));
        ARG_HASLINKSIGNINFO (arg_node) = TRUE;

        INFO_NUMS (arg_info) = NUMS_NEXT (INFO_NUMS (arg_info));
    }

    if (ARG_NEXT (arg_node) != 0) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSPtypedef (node *arg_node, info *arg_info)
{
    node *pragma;

    DBUG_ENTER ("RSPtypedef");

    DBUG_PRINT ("RSP", ("Processing typedef '%s'...", CTIitemName (arg_node)));

    if (TYPEDEF_PRAGMA (arg_node) != NULL) {
        pragma = TYPEDEF_PRAGMA (arg_node);

        TYPEDEF_FREEFUN (arg_node) = STRcpy (PRAGMA_FREEFUN (pragma));
        TYPEDEF_INITFUN (arg_node) = STRcpy (PRAGMA_INITFUN (pragma));
        TYPEDEF_COPYFUN (arg_node) = STRcpy (PRAGMA_COPYFUN (pragma));

        /*
         * if this typedef depends on any external object files,
         * add them to the dependencies
         */
        if (PRAGMA_LINKOBJ (pragma) != NULL) {
            global.dependencies = STRSjoin (PRAGMA_LINKOBJ (pragma), global.dependencies);

            PRAGMA_LINKOBJ (pragma) = NULL;
        }

        /*
         * if this typedef needs an external module, add it to
         * the external dependencies of this module.
         */
        if (PRAGMA_LINKMOD (pragma) != NULL) {
            global.dependencies = STRSjoin (PRAGMA_LINKMOD (pragma), global.dependencies);

            PRAGMA_LINKMOD (pragma) = NULL;
        }

        TYPEDEF_PRAGMA (arg_node) = FREEdoFreeNode (TYPEDEF_PRAGMA (arg_node));
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSPobjdef (node *arg_node, info *arg_info)
{
    node *pragma;

    DBUG_ENTER ("RSPobjdef");

    DBUG_PRINT ("RSP", ("Processing objdef '%s'...", CTIitemName (arg_node)));

    if (OBJDEF_PRAGMA (arg_node) != NULL) {
        pragma = OBJDEF_PRAGMA (arg_node);

        OBJDEF_LINKNAME (arg_node) = STRcpy (PRAGMA_LINKNAME (pragma));

        OBJDEF_PRAGMA (arg_node) = FREEdoFreeNode (OBJDEF_PRAGMA (arg_node));
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSPFundef");

    DBUG_PRINT ("RSP", ("Processing function '%s'...", CTIitemName (arg_node)));

    /*
     * first we set an initial state for all external
     * functions, which is then updated by the pragmas
     * if they exist.
     */

    if (FUNDEF_ISEXTERN (arg_node)) {
        FUNDEF_RETS (arg_node) = InitFundefRetsForExtern (FUNDEF_RETS (arg_node));
        FUNDEF_ARGS (arg_node) = InitFundefArgsForExtern (FUNDEF_ARGS (arg_node));
    }

    if (FUNDEF_PRAGMA (arg_node) != NULL) {
        node *pragma = FUNDEF_PRAGMA (arg_node);

        DBUG_ASSERT ((FUNDEF_ISEXTERN (arg_node)),
                     "Found a pragma at a non external function!");

        DBUG_PRINT ("RSP",
                    ("Processing pragmas for function '%s'...", FUNDEF_NAME (arg_node)));

        PRAGMA_NUMPARAMS (pragma)
          = TCcountArgs (FUNDEF_ARGS (arg_node)) + TCcountRets (FUNDEF_RETS (arg_node));

        if (PRAGMA_COPYFUN (pragma) != NULL) {
            CTIwarnLine (NODE_LINE (arg_node),
                         "Pragma 'copyfun` has no effect on function");
            PRAGMA_COPYFUN (pragma) = MEMfree (PRAGMA_COPYFUN (pragma));
        }

        if (PRAGMA_FREEFUN (pragma) != NULL) {
            CTIwarnLine (NODE_LINE (arg_node),
                         "Pragma 'freefun` has no effect on function");
            PRAGMA_FREEFUN (pragma) = MEMfree (PRAGMA_FREEFUN (pragma));
        }

        if (PRAGMA_INITFUN (pragma) != NULL) {
            CTIwarnLine (NODE_LINE (arg_node),
                         "Pragma 'initfun` has no effect on function");
            PRAGMA_INITFUN (pragma) = MEMfree (PRAGMA_INITFUN (pragma));
        }

        if (PRAGMA_LINKSIGN (pragma) != NULL) {
            if (CheckLinkSignNums (NODE_LINE (arg_node), PRAGMA_NUMPARAMS (pragma),
                                   PRAGMA_LINKSIGN (pragma))) {
                arg_node
                  = AnnotateLinksign (arg_node, arg_info, PRAGMA_LINKSIGN (pragma));
            }

            PRAGMA_LINKSIGN (pragma) = FREEdoFreeTree (PRAGMA_LINKSIGN (pragma));
        }

        if (PRAGMA_REFCOUNTING (pragma) != NULL) {
            CheckRefReadNums (NODE_LINE (arg_node), PRAGMA_NUMPARAMS (pragma),
                              PRAGMA_REFCOUNTING (pragma));

            arg_node
              = AnnotateRefcounting (arg_node, arg_info, PRAGMA_REFCOUNTING (pragma));

            PRAGMA_REFCOUNTING (pragma) = FREEdoFreeTree (PRAGMA_REFCOUNTING (pragma));
        }

        /*
         * if this function needs an external module, add it to
         * the external dependencies of this module.
         */
        if (PRAGMA_LINKMOD (pragma) != NULL) {
            global.dependencies = STRSjoin (PRAGMA_LINKMOD (pragma), global.dependencies);

            PRAGMA_LINKMOD (pragma) = NULL;
        }

        /*
         * if this function is defined by an external object file,
         * add it to the dependencies
         */
        if (PRAGMA_LINKOBJ (pragma) != NULL) {
            global.dependencies = STRSjoin (PRAGMA_LINKOBJ (pragma), global.dependencies);

            PRAGMA_LINKOBJ (pragma) = NULL;
        }

        if (PRAGMA_EFFECT (pragma) != NULL) {
            FUNDEF_AFFECTEDOBJECTS (arg_node) = PRAGMA_EFFECT (pragma);
            PRAGMA_EFFECT (pragma) = NULL;
        }

        /*
         * annotate linkname pragma to fundef
         */
        if (PRAGMA_LINKNAME (pragma) != NULL) {
            FUNDEF_LINKNAME (arg_node) = PRAGMA_LINKNAME (pragma);
            PRAGMA_LINKNAME (pragma) = NULL;
        }

        if ((PRAGMA_LINKNAME (pragma) == NULL) && (PRAGMA_LINKOBJ (pragma) == NULL)
            && (PRAGMA_LINKSIGN (pragma) == NULL) && (PRAGMA_LINKMOD (pragma) == NULL)
            && (PRAGMA_LINKSIGN (pragma) == NULL) && (PRAGMA_EFFECT (pragma) == NULL)
            && (PRAGMA_REFCOUNTING (pragma) == NULL)) {
            FUNDEF_PRAGMA (arg_node) = FREEdoFreeNode (pragma);
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSPModule");

    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSPdoResolvePragmas (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("RSPdoResolvePragmas");

    info = MakeInfo ();

    TRAVpush (TR_rsp);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
