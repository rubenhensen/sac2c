#include "resolvepragma.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "stringset.h"
#include "free.h"
#include "ctinfo.h"

#define DBUG_PREFIX "RSP"
#include "debug.h"

#include "globals.h"

/*
 * INFO structure
 */

enum travmode_t { RSP_default, RSP_refcnt, RSP_linksign, RSP_gpumem };

struct INFO {
    node *nums;
    node *module;
    int counter;
    bool annotated;
    enum travmode_t travmode;
};

/*
 * INFO macros
 */
#define INFO_NUMS(n) ((n)->nums)
#define INFO_MODULE(n) ((n)->module)
#define INFO_COUNTER(n) ((n)->counter)
#define INFO_TRAVMODE(n) ((n)->travmode)
#define INFO_ANNOTATED(n) ((n)->annotated)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_NUMS (result) = NULL;
    INFO_MODULE (result) = NULL;
    INFO_COUNTER (result) = 0;
    INFO_TRAVMODE (result) = RSP_default;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static void
CheckRefReadNums (struct location loc, int size, node *nums)
{
    int i;
    node *tmp;

    DBUG_ENTER ();

    tmp = nums;
    i = 1;

    while (tmp != NULL) {
        DBUG_PRINT_TAG ("PRAGMA", "Nums value is %d", NUMS_VAL (tmp));

        if ((NUMS_VAL (tmp) < 0) || (NUMS_VAL (tmp) >= size)) {
            CTIerror (loc,
                      "Invalid argument of pragma 'readonly`, 'refcounting`, or 'gpumem`: "
                      "Entry no. %d with value %d does not match a function parameter",
                      i, NUMS_VAL (tmp));
        }

        tmp = NUMS_NEXT (tmp);
        i++;
    }

    DBUG_RETURN ();
}

static bool
CheckLinkSignNums (struct location loc, int size, node *nums)
{
    int i;
    node *tmp;
    bool result = TRUE;

    DBUG_ENTER ();

    for (i = 0, tmp = nums; (i < size) && (tmp != NULL); i++, tmp = NUMS_NEXT (tmp)) {
        DBUG_PRINT_TAG ("PRAGMA", "Nums value is %d", NUMS_VAL (tmp));

        if ((NUMS_VAL (tmp) < 0) || (NUMS_VAL (tmp) > size)) {
            CTIerror (loc,
                      "Invalid argument of pragma 'linksign`: "
                      "Entry no. %d does not match a valid parameter position",
                      i + 1);
            result = FALSE;
        }
    }

    if (i < size) {
        CTIerror (loc,
                  "Invalid argument of pragma 'linksign` :"
                  "Less entries (%d) than parameters of function (%d)",
                  i, size);
        result = FALSE;
    }

    if (tmp != NULL) {
        do {
            i++;

            DBUG_PRINT_TAG ("PRAGMA", "Nums value is %d", NUMS_VAL (tmp));

            tmp = NUMS_NEXT (tmp);
        } while (tmp != NULL);

        CTIerror (loc,
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
    DBUG_ENTER ();

    if (rets != NULL) {
        DBUG_ASSERT (NODE_TYPE (rets) == N_ret, "found a non N_ret node");

        RET_ISREFCOUNTED (rets) = FALSE;
        RET_ISGPUMEM (rets) = FALSE;

        RET_NEXT (rets) = InitFundefRetsForExtern (RET_NEXT (rets));
    }

    DBUG_RETURN (rets);
}

static node *
InitFundefArgsForExtern (node *args)
{
    DBUG_ENTER ();

    if (args != NULL) {
        DBUG_ASSERT (NODE_TYPE (args) == N_arg, "found a non N_arg node");

        ARG_ISREFCOUNTED (args) = FALSE;
        ARG_ISGPUMEM (args) = FALSE;

        ARG_NEXT (args) = InitFundefArgsForExtern (ARG_NEXT (args));
    }

    DBUG_RETURN (args);
}

static node *
AnnotateRefcounting (node *arg_node, info *arg_info, node *nums)
{
    DBUG_ENTER ();

    INFO_COUNTER (arg_info) = 0;
    INFO_NUMS (arg_info) = nums;
    INFO_TRAVMODE (arg_info) = RSP_refcnt;

    FUNDEF_RETS (arg_node) = TRAVopt(FUNDEF_RETS (arg_node), arg_info);
    FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);

    INFO_COUNTER (arg_info) = 0;
    INFO_NUMS (arg_info) = NULL;
    INFO_TRAVMODE (arg_info) = RSP_default;

    DBUG_RETURN (arg_node);
}

static node *
AnnotateGpuMem (node *arg_node, info *arg_info, node *nums)
{
    DBUG_ENTER ();

    INFO_COUNTER (arg_info) = 0;
    INFO_ANNOTATED (arg_info) = FALSE;
    INFO_NUMS (arg_info) = nums;
    INFO_TRAVMODE (arg_info) = RSP_gpumem;

    FUNDEF_RETS (arg_node) = TRAVopt(FUNDEF_RETS (arg_node), arg_info);
    FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);

    FUNDEF_ISGPUFUNCTION (arg_node) = INFO_ANNOTATED (arg_info);
    DBUG_PRINT ("setting FUNDEF_ISGPUFUNCTION () to %s",
                 (INFO_ANNOTATED (arg_info)? "TRUE" : "FALSE"));

    INFO_ANNOTATED (arg_info) = FALSE;
    INFO_COUNTER (arg_info) = 0;
    INFO_NUMS (arg_info) = NULL;
    INFO_TRAVMODE (arg_info) = RSP_default;

    DBUG_RETURN (arg_node);
}

static node *
AnnotateLinksign (node *arg_node, info *arg_info, node *nums)
{
    DBUG_ENTER ();

    INFO_NUMS (arg_info) = nums;
    INFO_TRAVMODE (arg_info) = RSP_linksign;

    FUNDEF_RETS (arg_node) = TRAVopt(FUNDEF_RETS (arg_node), arg_info);
    FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);

    INFO_NUMS (arg_info) = NULL;
    INFO_TRAVMODE (arg_info) = RSP_default;

    DBUG_RETURN (arg_node);
}

node *
RSPret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_TRAVMODE (arg_info) == RSP_refcnt) {
        if (TCnumsContains (INFO_COUNTER (arg_info), INFO_NUMS (arg_info))) {

            RET_ISREFCOUNTED (arg_node) = TRUE;
        }

        INFO_COUNTER (arg_info)++;
    } else if (INFO_TRAVMODE (arg_info) == RSP_gpumem) {
        if (TCnumsContains (INFO_COUNTER (arg_info), INFO_NUMS (arg_info))) {

            RET_ISGPUMEM (arg_node) = TRUE;
            INFO_ANNOTATED (arg_info) = TRUE;
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
    DBUG_ENTER ();

    if (INFO_TRAVMODE (arg_info) == RSP_refcnt) {
        if (TCnumsContains (INFO_COUNTER (arg_info), INFO_NUMS (arg_info))) {

            ARG_ISREFCOUNTED (arg_node) = TRUE;
        }

        INFO_COUNTER (arg_info)++;
    } else if (INFO_TRAVMODE (arg_info) == RSP_gpumem) {
        if (TCnumsContains (INFO_COUNTER (arg_info), INFO_NUMS (arg_info))) {

            ARG_ISGPUMEM (arg_node) = TRUE;
            INFO_ANNOTATED (arg_info) = TRUE;
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

    DBUG_ENTER ();

    DBUG_PRINT ("Processing typedef '%s'...", CTIitemName (arg_node));

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

    TYPEDEF_NEXT (arg_node) = TRAVopt(TYPEDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSPobjdef (node *arg_node, info *arg_info)
{
    node *pragma;

    DBUG_ENTER ();

    DBUG_PRINT ("Processing objdef '%s'...", CTIitemName (arg_node));

    if (OBJDEF_PRAGMA (arg_node) != NULL) {
        pragma = OBJDEF_PRAGMA (arg_node);

        OBJDEF_LINKNAME (arg_node) = STRcpy (PRAGMA_LINKNAME (pragma));

        OBJDEF_PRAGMA (arg_node) = FREEdoFreeNode (OBJDEF_PRAGMA (arg_node));
    }

    OBJDEF_NEXT (arg_node) = TRAVopt(OBJDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Processing function '%s'...", CTIitemName (arg_node));

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

        DBUG_ASSERT (FUNDEF_ISEXTERN (arg_node),
                     "Found a pragma at a non external function!");

        DBUG_PRINT ("Processing pragmas for function '%s'...", FUNDEF_NAME (arg_node));

        PRAGMA_NUMPARAMS (pragma)
          = TCcountArgs (FUNDEF_ARGS (arg_node)) + TCcountRets (FUNDEF_RETS (arg_node));

        DBUG_PRINT ("number of arguments: %d", PRAGMA_NUMPARAMS (pragma));

        if (PRAGMA_COPYFUN (pragma) != NULL) {
            CTIwarn (NODE_LOCATION (arg_node),
                        "Pragma 'copyfun` has no effect on function");
            PRAGMA_COPYFUN (pragma) = MEMfree (PRAGMA_COPYFUN (pragma));
        }

        if (PRAGMA_FREEFUN (pragma) != NULL) {
            CTIwarn (NODE_LOCATION (arg_node),
                        "Pragma 'freefun` has no effect on function");
            PRAGMA_FREEFUN (pragma) = MEMfree (PRAGMA_FREEFUN (pragma));
        }

        if (PRAGMA_INITFUN (pragma) != NULL) {
            CTIwarn (NODE_LOCATION (arg_node),
                        "Pragma 'initfun` has no effect on function");
            PRAGMA_INITFUN (pragma) = MEMfree (PRAGMA_INITFUN (pragma));
        }

        if (PRAGMA_LINKSIGN (pragma) != NULL) {
            if (CheckLinkSignNums (NODE_LOCATION (arg_node), PRAGMA_NUMPARAMS (pragma),
                                   PRAGMA_LINKSIGN (pragma))) {
                arg_node
                  = AnnotateLinksign (arg_node, arg_info, PRAGMA_LINKSIGN (pragma));
            }

            PRAGMA_LINKSIGN (pragma) = FREEdoFreeTree (PRAGMA_LINKSIGN (pragma));
        }

        if (PRAGMA_REFCOUNTING (pragma) != NULL) {
            CheckRefReadNums (NODE_LOCATION (arg_node), PRAGMA_NUMPARAMS (pragma),
                              PRAGMA_REFCOUNTING (pragma));

            arg_node
              = AnnotateRefcounting (arg_node, arg_info, PRAGMA_REFCOUNTING (pragma));

            PRAGMA_REFCOUNTING (pragma) = FREEdoFreeTree (PRAGMA_REFCOUNTING (pragma));
        }

        if (PRAGMA_GPUMEM (pragma) != NULL) {
            DBUG_PRINT ("...processing gpumem pragma");
            CheckRefReadNums (NODE_LOCATION (arg_node), PRAGMA_NUMPARAMS (pragma),
                              PRAGMA_GPUMEM (pragma));

            arg_node
              = AnnotateGpuMem (arg_node, arg_info, PRAGMA_GPUMEM (pragma));

            PRAGMA_GPUMEM (pragma) = FREEdoFreeTree (PRAGMA_GPUMEM (pragma));
        }

        if (PRAGMA_REFCOUNTDOTS (pragma)) {
            if (FUNDEF_HASDOTARGS (arg_node) || FUNDEF_HASDOTRETS (arg_node)) {
                FUNDEF_REFCOUNTDOTS (arg_node) = PRAGMA_REFCOUNTDOTS (pragma);
            } else {
                CTIwarn (NODE_LOCATION (arg_node),
                            "Pragma 'refcountdots' has no effect on function");
            }
        }

        if (PRAGMA_MUTCTHREADFUN (pragma)) {
            FUNDEF_ISTHREADFUN (arg_node) = TRUE;
        }

        if (PRAGMA_NOINLINE (pragma)) {
            FUNDEF_NOINLINE (arg_node) = TRUE;
            DBUG_PRINT_TAG ("RSP-A", "Set %s to noinline", CTIitemName (arg_node));
        }

        if (PRAGMA_HEADER (pragma)) {
            MODULE_HEADERS (INFO_MODULE (arg_info))
              = STRSjoin (PRAGMA_HEADER (pragma), MODULE_HEADERS (INFO_MODULE (arg_info)));

            FUNDEF_HEADER (arg_node) = TRUE;
            PRAGMA_HEADER (pragma) = NULL;
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

        if (PRAGMA_CUDALINKNAME (pragma) != NULL) {
            if (FUNDEF_LINKNAME (arg_node) == NULL) {
                CTIwarn (NODE_LOCATION (arg_node),
                            "Implicit declaration of 'C' version of external function"
                            " use linkname to explicitly declare 'C' version");
            }
            FUNDEF_CUDALINKNAME (arg_node) = PRAGMA_CUDALINKNAME (pragma);
            PRAGMA_CUDALINKNAME (pragma) = NULL;
        }

        if ((PRAGMA_LINKNAME (pragma) == NULL) && (PRAGMA_CUDALINKNAME (pragma) == NULL)
            && (PRAGMA_LINKOBJ (pragma) == NULL) && (PRAGMA_LINKSIGN (pragma) == NULL)
            && (PRAGMA_LINKMOD (pragma) == NULL) && (PRAGMA_LINKSIGN (pragma) == NULL)
            && (PRAGMA_EFFECT (pragma) == NULL) && (PRAGMA_REFCOUNTING (pragma) == NULL)
            && (PRAGMA_HEADER (pragma) == NULL)) {
            FUNDEF_PRAGMA (arg_node) = FREEdoFreeNode (pragma);
        }
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RSPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Processing modules...");

    INFO_MODULE (arg_info) = arg_node;

    MODULE_OBJS (arg_node) = TRAVopt(MODULE_OBJS (arg_node), arg_info);

    MODULE_TYPES (arg_node) = TRAVopt(MODULE_TYPES (arg_node), arg_info);

    MODULE_FUNDECS (arg_node) = TRAVopt(MODULE_FUNDECS (arg_node), arg_info);

    INFO_MODULE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
RSPdoResolvePragmas (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_rsp);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
