/*
 *
 * $Log$
 * Revision 1.2  2002/08/06 08:26:49  sbs
 * some vars initialized to please gcc for the product version.
 *
 * Revision 1.1  2002/08/05 16:58:34  sbs
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include "dbug.h"

#include "sig_deps.h"
#include "new_typecheck.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

/******************************************************************************
 ***
 ******************************************************************************/

struct SIG_DEP {
    ct_funptr ct_fun; /* function for computing the return type */
    te_info *info;    /* information about the creation of the sig dep */
    ntype *args;      /* product type of the actual argument types */
    ntype *res;       /* product type containing the result type variables */
    int rc;           /* refererence counter for this sig_dep structure */
};

#define SD_FUN(n) (n->ct_fun)
#define SD_INFO(n) (n->info)
#define SD_ARGS(n) (n->args)
#define SD_RES(n) (n->res)
#define SD_RC(n) (n->rc)

/******************************************************************************
 ***
 ***                   Some static helper functions:
 ***                   -----------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    sig_dep *MakeSig( ct_funptr ct_fun, te_info *info, ntype *args, ntype *res, int rc)
 *
 * description:
 *    constructor function for signature dependencies
 *
 ******************************************************************************/

static sig_dep *
MakeSig (ct_funptr ct_fun, te_info *info, ntype *args, ntype *results, int rc)
{
    sig_dep *res;

    DBUG_ENTER ("MakeSig");

    res = (sig_dep *)Malloc (sizeof (sig_dep));
    SD_FUN (res) = ct_fun;
    SD_INFO (res) = info;
    SD_ARGS (res) = args;
    SD_RES (res) = results;
    SD_RC (res) = rc;

    DBUG_RETURN (res);
}

/******************************************************************************
 ***
 ***  Here, the exported functions for handling signature dependencies follow:
 ***  ------------------------------------------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    ntype *SDCreateSignatureDependency( ct_funptr CtFun, te_info *info, ntype *args)
 *
 * description:
 *    from a given argument type (product type of the argument types) and a
 *    pointer to the fundef node holding the wrapper function, a sig_dep
 *    structure is built. It is attached as an assumtion to all non-fixed type
 *    variables and the ref-count is set accordingly.
 *    Furthermore, new type variables for the return types (contained in yet
 *    another product type) are created and returned.
 *    Last not least, HandleContradiction is called in order to trigger a
 *    (potential) first approximation to the return type.
 *
 ******************************************************************************/

ntype *
SDCreateSignatureDependency (ct_funptr CtFun, te_info *info, ntype *args)
{
    sig_dep *sig;
    node *wrapper;
    ntype *arg_t, *res_t;
    int num_args, num_res, i;
    bool ok = TRUE;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("SDCreateSignatureDependency");

    /*
     * First, we create the return type as it is part of the sig_dep structure:
     */
    if (TEGetWrapper (info) != NULL) {
        wrapper = TEGetWrapper (info);
        num_res = CountTypes (FUNDEF_TYPES (wrapper));
    } else {
        num_res = 1;
    }
    res_t = TYMakeEmptyProductType (num_res);
    for (i = 0; i < num_res; i++) {
        res_t = TYSetProductMember (res_t, i, TYMakeAlphaType (NULL));
    }

    /*
     * Now, we create the structure itself:
     */
    sig = MakeSig (CtFun, info, TYCopyType (args), res_t, TYCountNonFixedAlpha (args));

    /*
     * Inserting the dependency into the non fixed vars:
     */
    num_args = TYGetProductSize (args);
    for (i = 0; i < num_args; i++) {
        arg_t = TYGetProductMember (args, i);
        if (TYIsNonFixedAlpha (arg_t)) {
            ok = ok && SSIAssumeLow (TYGetAlpha (arg_t), sig);
        }
    }
    DBUG_ASSERT (ok, "Something went wrong creating a signature dependency");

    DBUG_EXECUTE ("SSI", tmp_str = SDSigDep2DebugString (sig););
    DBUG_PRINT ("SSI", ("sig dep created: handle %p : %s", sig, tmp_str));
    DBUG_EXECUTE ("SSI", tmp_str = Free (tmp_str););

    /*
     * Finally, we try to create a first result type approximation
     */
    ok = SDHandleContradiction (sig);

    DBUG_ASSERT (ok, "Something went wrong creating a fundef signature dependency");

    DBUG_RETURN (TYCopyType (SD_RES (sig)));
}

/******************************************************************************
 *
 * function:
 *    bool SDHandleContradiction( node *fun_sig)
 *
 * description:
 *
 ******************************************************************************/

bool
SDHandleContradiction (sig_dep *fun_sig)
{
    ntype *res_vars, *res_t, *res, *args;
    bool ok;
    te_info *info;
    int i;
#ifndef DBUG_OFF
    char *tmp_str, *tmp2_str;
#endif

    DBUG_ENTER ("SDHandleContradiction");

    /*
     * First, we check whether there is enough information available for
     * making a (new) result type approximation.
     */
    if (TYCountNoMinAlpha (SD_ARGS (fun_sig)) > 0) {
        ok = TRUE;
    } else {
        info = SD_INFO (fun_sig);
        /*
         * Now, we compute a new approximation:
         */
        args = TYFixAndEliminateAlpha (SD_ARGS (fun_sig));
        res_t = SD_FUN (fun_sig) (info, args);
        res_t = TYEliminateAlpha (res_t);

        DBUG_EXECUTE ("SSI", tmp_str = TYType2String (args, FALSE, 0););
        DBUG_EXECUTE ("SSI", tmp2_str = TYType2String (res_t, FALSE, 0););
        DBUG_PRINT ("SSI", ("approximating %s \"%s\" for %s yields %s",
                            TEGetKindStr (info), TEGetNameStr (info), tmp_str, tmp2_str));
        DBUG_EXECUTE ("SSI", tmp_str = Free (tmp_str););
        DBUG_EXECUTE ("SSI", tmp2_str = Free (tmp_str););

        /*
         * and insert the findings into the return types:
         */
        res_vars = SD_RES (fun_sig);
        ok = TRUE;
        for (i = 0; i < TYGetProductSize (res_vars); i++) {
            res = TYGetProductMember (res_t, i);
            if (TYIsAlpha (res)) {
                ok = ok
                     && SSINewRel (TYGetAlpha (res),
                                   TYGetAlpha (TYGetProductMember (res_vars, i)));
            } else {
                ok = ok && SSINewMin (TYGetAlpha (TYGetProductMember (res_vars, i)), res);
            }
        }
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *    bool SDHandleElimination( sig_dep *fun_sig)
 *
 * description:
 *
 ******************************************************************************/

bool
SDHandleElimination (sig_dep *fun_sig)
{
    DBUG_ENTER ("SDHandleElimination");
    DBUG_RETURN (TRUE);
}

/******************************************************************************
 *
 * function:
 *    char *SDSigDep2DebugString( sig_dep *fun_sig)
 *
 * description:
 *
 ******************************************************************************/

char *
SDSigDep2DebugString (sig_dep *fun_sig)
{
    static char buf[256];
    char *tmp = &buf[0];
    char *tmp_str;
    te_info *info;

    DBUG_ENTER ("SDSigDep2DebugString");

    info = SD_INFO (fun_sig);
    tmp += sprintf (tmp, "%s \"%s\"", TEGetKindStr (info), TEGetNameStr (info));

    tmp_str = TYType2String (SD_ARGS (fun_sig), FALSE, 0);
    tmp += sprintf (tmp, "%s -> ", tmp_str);
    tmp_str = Free (tmp_str);

    tmp_str = TYType2String (SD_RES (fun_sig), FALSE, 0);
    tmp += sprintf (tmp, "%s rc:%d", tmp_str, SD_RC (fun_sig));
    tmp_str = Free (tmp_str);

    DBUG_RETURN (StringCopy (buf));
}
