/*
 *
 * $Log$
 * Revision 1.13  2005/09/13 16:28:59  sah
 * now, even wrong pragmas are handeled correctly
 *
 * Revision 1.12  2005/06/27 13:44:55  sah
 * now multiple LINKOBJ and LINKMOD per file
 * function are possible and handeled correctly.
 *
 * Revision 1.11  2005/06/04 12:59:43  sbs
 * as FUNDEF_RETS and FUNDEF_ARGS are optional we should never
 * traverse them w/o checking whether they exist.
 *
 * Revision 1.10  2005/02/18 10:37:23  sah
 * module system fixes
 *
 * Revision 1.9  2005/02/16 22:29:13  sah
 * fixed handling of pragmas
 *
 * Revision 1.8  2005/01/10 16:59:45  cg
 * Converted error messages from Error.h to ctinfo.c
 *
 * Revision 1.7  2004/12/19 23:16:52  sbs
 * TCcountFunctionParams replaced by TCcountArgs and TCcountRets
 *
 * Revision 1.6  2004/11/26 19:18:30  skt
 * renamed RSPmodul into RSPmodule
 *
 * Revision 1.5  2004/11/25 18:12:34  sah
 * added proper initialisation of REFCOUNTED and LINKSIGN for
 * args and rets
 *
 * Revision 1.4  2004/11/24 11:29:33  sah
 * COMPILES
 *
 * Revision 1.3  2004/11/21 22:45:20  sbs
 * SacDevCamp04
 *
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
#include "internal_lib.h"
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
    node *module;
    node *nums;
    int counter;
    enum { RSP_default, RSP_refcnt, RSP_linksign } travmode;
};

/*
 * INFO macros
 */
#define INFO_RSP_MODULE(n) ((n)->module)
#define INFO_RSP_NUMS(n) ((n)->nums)
#define INFO_RSP_COUNTER(n) ((n)->counter)
#define INFO_RSP_TRAVMODE(n) ((n)->travmode)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_RSP_MODULE (result) = NULL;
    INFO_RSP_NUMS (result) = NULL;
    INFO_RSP_COUNTER (result) = 0;
    INFO_RSP_TRAVMODE (result) = RSP_default;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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

/**
 *
 * The following code is pre SacDevCamp04. Due to its limited effect, we
 * decided to (at least temporarily) disable these pragmas.
 * In case a later re-activtion is desired, the code still resides here....

static
node *ResolvePragmaReadonly(node *arg_node, node *pragma)
{
  int cnt;
  node *tmp_args;
  types *tmp_types;

  DBUG_ENTER("ResolvePragmaReadonly");

  DBUG_PRINT("PRAGMA",("Resolving pragma readonly for function %s",
                       ItemName(arg_node)));

  CheckRefReadNums(NODE_LINE(arg_node), PRAGMA_NUMPARAMS(pragma),
PRAGMA_READONLY(pragma));

  tmp_types=FUNDEF_TYPES(arg_node);
  cnt=CountTypes(tmp_types);

  tmp_args=FUNDEF_ARGS(arg_node);

  while (tmp_args!=NULL) {
    if (PRAGMA_READONLY(pragma)[cnt]) {
      if (ARG_ATTRIB(tmp_args)==ST_reference) {
        ARG_ATTRIB(tmp_args)=ST_readonly_reference;
      }
      else {
        WARN(NODE_LINE(arg_node),
             ("Parameter no. %d of function '%s` is not a reference "
              "parameter, so pragma 'readonly` has no effect on it",
              cnt, ItemName(arg_node)));
      }
    }

    tmp_args=ARG_NEXT(tmp_args);
    cnt++;
  }

  PRAGMA_READONLY(pragma) = ILIBfree(PRAGMA_READONLY(pragma));

  DBUG_RETURN(arg_node);
}
 *
 */

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

    INFO_RSP_COUNTER (arg_info) = 0;
    INFO_RSP_NUMS (arg_info) = nums;
    INFO_RSP_TRAVMODE (arg_info) = RSP_refcnt;

    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    INFO_RSP_COUNTER (arg_info) = 0;
    INFO_RSP_NUMS (arg_info) = NULL;
    INFO_RSP_TRAVMODE (arg_info) = RSP_default;

    DBUG_RETURN (arg_node);
}

static node *
AnnotateLinksign (node *arg_node, info *arg_info, node *nums)
{
    DBUG_ENTER ("AnnotateLinksign");

    INFO_RSP_NUMS (arg_info) = nums;
    INFO_RSP_TRAVMODE (arg_info) = RSP_linksign;

    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    INFO_RSP_NUMS (arg_info) = NULL;
    INFO_RSP_TRAVMODE (arg_info) = RSP_default;

    DBUG_RETURN (arg_node);
}

node *
RSPret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSPret");

    if (INFO_RSP_TRAVMODE (arg_info) == RSP_refcnt) {
        if (TCnumsContains (INFO_RSP_COUNTER (arg_info), INFO_RSP_NUMS (arg_info))) {

            RET_ISREFCOUNTED (arg_node) = TRUE;
        }

        INFO_RSP_COUNTER (arg_info)++;
    } else if (INFO_RSP_TRAVMODE (arg_info) == RSP_linksign) {
        RET_LINKSIGN (arg_node) = NUMS_VAL (INFO_RSP_NUMS (arg_info));
        RET_HASLINKSIGNINFO (arg_node) = TRUE;

        INFO_RSP_NUMS (arg_info) = NUMS_NEXT (INFO_RSP_NUMS (arg_info));
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

    if (INFO_RSP_TRAVMODE (arg_info) == RSP_refcnt) {
        if (TCnumsContains (INFO_RSP_COUNTER (arg_info), INFO_RSP_NUMS (arg_info))) {

            ARG_ISREFCOUNTED (arg_node) = TRUE;
        }

        INFO_RSP_COUNTER (arg_info)++;
    } else if (INFO_RSP_TRAVMODE (arg_info) == RSP_linksign) {
        ARG_LINKSIGN (arg_node) = NUMS_VAL (INFO_RSP_NUMS (arg_info));
        ARG_HASLINKSIGNINFO (arg_node) = TRUE;

        INFO_RSP_NUMS (arg_info) = NUMS_NEXT (INFO_RSP_NUMS (arg_info));
    }

    if (ARG_NEXT (arg_node) != 0) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSPFundef");

    DBUG_PRINT ("RSP", ("Processing function '%s'...", FUNDEF_NAME (arg_node)));

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

        if (PRAGMA_FREEFUN (pragma) != NULL) {
            CTIwarnLine (NODE_LINE (arg_node),
                         "Pragma 'freefun` has no effect on function");
            PRAGMA_FREEFUN (pragma) = ILIBfree (PRAGMA_FREEFUN (pragma));
        }

        if (PRAGMA_INITFUN (pragma) != NULL) {
            CTIwarnLine (NODE_LINE (arg_node),
                         "Pragma 'initfun` has no effect on function");
            PRAGMA_INITFUN (pragma) = ILIBfree (PRAGMA_INITFUN (pragma));
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

        if (PRAGMA_READONLY (pragma) != NULL) {
            CTIwarnLine (NODE_LINE (arg_node), "Pragma 'readonly` has been disabled");
            PRAGMA_INITFUN (pragma) = ILIBfree (PRAGMA_INITFUN (pragma));
        }

        /*
         * if this function needs an external module, add it to
         * the external dependencies of this module.
         */
        if (PRAGMA_LINKMOD (pragma) != NULL) {
            MODULE_DEPENDENCIES (INFO_RSP_MODULE (arg_info))
              = STRSjoin (PRAGMA_LINKMOD (pragma),
                          MODULE_DEPENDENCIES (INFO_RSP_MODULE (arg_info)));

            PRAGMA_LINKMOD (pragma) = NULL;
        }

        /*
         * if this function is defined by an external object file,
         * add it to the dependencies
         */
        if (PRAGMA_LINKOBJ (pragma) != NULL) {
            MODULE_DEPENDENCIES (INFO_RSP_MODULE (arg_info))
              = STRSjoin (PRAGMA_LINKOBJ (pragma),
                          MODULE_DEPENDENCIES (INFO_RSP_MODULE (arg_info)));

            PRAGMA_LINKOBJ (pragma) = NULL;
        }

        if (PRAGMA_TOUCH (pragma) != NULL) {
            CTIwarnLine (NODE_LINE (arg_node),
                         "Pragma 'touch` has no effect on function");
            PRAGMA_TOUCH (pragma) = ILIBfree (PRAGMA_TOUCH (pragma));
        }

        if (PRAGMA_EFFECT (pragma) != NULL) {
            CTIwarnLine (NODE_LINE (arg_node),
                         "Pragma 'effect` has no effect on function");
            PRAGMA_EFFECT (pragma) = ILIBfree (PRAGMA_EFFECT (pragma));
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
            && (PRAGMA_LINKSIGN (pragma) == NULL) && (PRAGMA_TOUCH (pragma) == NULL)
            && (PRAGMA_EFFECT (pragma) == NULL)
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

    INFO_RSP_MODULE (arg_info) = arg_node;

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

void
RSPdoResolvePragmas (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("RSPdoResolvePragmas");

    info = MakeInfo ();

    TRAVpush (TR_rsp);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_VOID_RETURN;
}
