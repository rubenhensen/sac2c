/*
 *
 * $Log$
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
#include "Error.h"
#include "dbug.h"

/*
 * INFO structure
 */
struct INFO {
    node *module;
    node *nums;
    int counter;
};

/*
 * INFO macros
 */
#define INFO_RSP_MODULE(n) ((n)->module)
#define INFO_RSP_NUMS(n) ((n)->nums)
#define INFO_RSP_COUNTER(n) ((n)->counter)

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
            ERROR (line, ("Invalid argument of pragma 'readonly` or 'refcounting`:"));
            CONT_ERROR (
              ("Entry no.%d does not match a function parameter !", i, NUMS_VAL (tmp)));
        }

        tmp = NUMS_NEXT (tmp);
        i++;
    }

    DBUG_VOID_RETURN;
}

static void
CheckLinkSignNums (int line, int size, node *nums)
{
    int i;
    node *tmp;

    DBUG_ENTER ("CheckLinkSignNums");

    for (i = 0, tmp = nums; (i < size) && (tmp != NULL); i++, tmp = NUMS_NEXT (tmp)) {
        DBUG_PRINT ("PRAGMA", ("Nums value is %d", NUMS_VAL (tmp)));

        if ((NUMS_VAL (tmp) < 0) || (NUMS_VAL (tmp) > size)) {
            ERROR (line, ("Invalid argument of pragma 'linksign`"));
            CONT_ERROR (
              ("Entry no.%d does not match a valid parameter position !", i + 1));
        }
    }

    if (i < size) {
        ERROR (line, ("Invalid argument of pragma 'linksign`"));
        CONT_ERROR (("Less entries (%d) than parameters of function (%d) !", i, size));
    }

    if (tmp != NULL) {
        do {
            i++;

            DBUG_PRINT ("PRAGMA", ("Nums value is %d", NUMS_VAL (tmp)));

            tmp = NUMS_NEXT (tmp);
        } while (tmp != NULL);

        ERROR (line, ("Invalid argument of pragma 'linksign`:"));
        CONT_ERROR (("More entries (%d) than function parameters (%d) !", i, size));
    }

    DBUG_VOID_RETURN;
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

node *
AnnotateRefcounting (node *arg_node, info *arg_info, node *nums)
{
    DBUG_ENTER ("AnnotateRefcounting");

    INFO_RSP_COUNTER (arg_info) = 0;
    INFO_RSP_NUMS (arg_info) = nums;

    FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);

    INFO_RSP_COUNTER (arg_info) = 0;
    INFO_RSP_NUMS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
RSPret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSPret");

    if (TCnumsContains (INFO_RSP_COUNTER (arg_info), INFO_RSP_NUMS (arg_info))) {
        RET_ISREFCOUNTED (arg_node) = TRUE;
    } else {
        RET_ISREFCOUNTED (arg_node) = FALSE;
    }

    INFO_RSP_COUNTER (arg_info)++;

    DBUG_RETURN (arg_node);
}

node *
RSParg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSParg");

    if (TCnumsContains (INFO_RSP_COUNTER (arg_info), INFO_RSP_NUMS (arg_info))) {
        ARG_ISREFCOUNTED (arg_node) = TRUE;
    } else {
        ARG_ISREFCOUNTED (arg_node) = FALSE;
    }

    INFO_RSP_COUNTER (arg_info)++;

    DBUG_RETURN (arg_node);
}

node *
RSPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSPFundef");

    DBUG_PRINT ("RSP", ("Processing function '%s'...", FUNDEF_NAME (arg_node)));

    if (FUNDEF_PRAGMA (arg_node) != NULL) {
        node *pragma = FUNDEF_PRAGMA (arg_node);

        DBUG_PRINT ("RSP",
                    ("Processing pragmas for function '%s'...", FUNDEF_NAME (arg_node)));

        PRAGMA_NUMPARAMS (pragma) = TCcountFunctionParams (arg_node);

        if (PRAGMA_FREEFUN (pragma) != NULL) {
            WARN (NODE_LINE (arg_node), ("Pragma 'freefun` has no effect on function"));
            PRAGMA_FREEFUN (pragma) = ILIBfree (PRAGMA_FREEFUN (pragma));
        }

        if (PRAGMA_INITFUN (pragma) != NULL) {
            WARN (NODE_LINE (arg_node), ("Pragma 'initfun` has no effect on function"));
            PRAGMA_INITFUN (pragma) = ILIBfree (PRAGMA_INITFUN (pragma));
        }

        if (PRAGMA_LINKSIGN (pragma) != NULL) {
            CheckLinkSignNums (NODE_LINE (arg_node), PRAGMA_NUMPARAMS (pragma),
                               PRAGMA_LINKSIGN (pragma));
        }

        if (PRAGMA_REFCOUNTING (pragma) != NULL) {
            CheckRefReadNums (NODE_LINE (arg_node), PRAGMA_NUMPARAMS (pragma),
                              PRAGMA_REFCOUNTING (pragma));

            arg_node
              = AnnotateRefcounting (arg_node, arg_info, PRAGMA_REFCOUNTING (pragma));

            PRAGMA_REFCOUNTING (pragma) = FREEdoFreeTree (PRAGMA_REFCOUNTING (pragma));
        }

        if (PRAGMA_READONLY (pragma) != NULL) {
            WARN (NODE_LINE (arg_node), ("Pragma 'readonly` has been disabled"));
            PRAGMA_INITFUN (pragma) = ILIBfree (PRAGMA_INITFUN (pragma));
        }

        /*
         * if this function needs an external module, add it to
         * the external dependencies of this module.
         */
        if (PRAGMA_LINKMOD (pragma) != NULL) {
            MODULE_DEPENDENCIES (INFO_RSP_MODULE (arg_info))
              = STRSadd (PRAGMA_LINKMOD (pragma), STRS_extlib,
                         MODULE_DEPENDENCIES (INFO_RSP_MODULE (arg_info)));

            PRAGMA_LINKMOD (pragma) = ILIBfree (PRAGMA_LINKMOD (pragma));
        }

        /*
         * if this function is defined by an external object file,
         * add it to the dependencies
         */
        if (PRAGMA_LINKOBJ (pragma) != NULL) {
            MODULE_DEPENDENCIES (INFO_RSP_MODULE (arg_info))
              = STRSadd (PRAGMA_LINKOBJ (pragma), STRS_objfile,
                         MODULE_DEPENDENCIES (INFO_RSP_MODULE (arg_info)));

            PRAGMA_LINKOBJ (pragma) = ILIBfree (PRAGMA_LINKOBJ (pragma));
        }

        /*
         * TODO: implement TOUCH and EFFECT, as soon as classes work
         */

        if ((PRAGMA_LINKNAME (pragma) == NULL) && (PRAGMA_LINKOBJ (pragma) == NULL)
            && (PRAGMA_LINKNAME (pragma) == NULL) && (PRAGMA_LINKMOD (pragma) == NULL)
            && (PRAGMA_LINKSIGN (pragma) == NULL)
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
RSPmodul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSPModul");

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
