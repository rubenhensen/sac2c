#include <stdio.h>

#include "sig_deps.h"

#define DBUG_PREFIX "SSI"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "private_heap.h"

#include "ssi.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "type_errors.h"

/**
 *
 * @addtogroup ntc
 *
 * @{
 */

/**
 *
 * @file sig_deps.c
 *
 * This file provides encapsulates the signature dependencies.
 */

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
    bool strict;      /* when true, contradictions will only be handled when
                         ALL argument types have at least one approximation */
    ntype *res;       /* product type containing the result type variables */
    int rc;           /* refererence counter for this sig_dep structure */
};

#define SD_FUN(n) (n->ct_fun)
#define SD_INFO(n) (n->info)
#define SD_ARGS(n) (n->args)
#define SD_STRICT(n) (n->strict)
#define SD_RES(n) (n->res)
#define SD_RC(n) (n->rc)

static heap *sig_dep_heap = NULL;

/******************************************************************************
 ***
 ***                   Some static helper functions:
 ***                   -----------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    sig_dep *MakeSig( ct_funptr ct_fun,
 *                      te_info *info,
 *                      ntype *args,
 *                      bool   strict,
 *                      ntype *res, int rc)
 *
 * description:
 *    constructor function for signature dependencies
 *
 ******************************************************************************/

static sig_dep *
MakeSig (ct_funptr ct_fun, te_info *info, ntype *args, bool strict, ntype *results,
         int rc)
{
    sig_dep *res;

    DBUG_ENTER ();

    if (sig_dep_heap == NULL) {
        sig_dep_heap = PHPcreateHeap (sizeof (sig_dep), 1000);
    }

    res = (sig_dep *)PHPmalloc (sig_dep_heap);
    SD_FUN (res) = ct_fun;
    SD_INFO (res) = info;
    SD_ARGS (res) = args;
    SD_STRICT (res) = strict;
    SD_RES (res) = results;
    SD_RC (res) = rc;

    DBUG_RETURN (res);
}

void
SDfreeAllSignatureDependencies (void)
{
    DBUG_ENTER ();

    sig_dep_heap = PHPfreeHeap (sig_dep_heap);

    DBUG_RETURN ();
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
 *    ntype *SDcreateSignatureDependency( ct_funptr CtFun,
 *                                        te_info *info,
 *                                        ntype *args
 *                                        bool strict)
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
 *    The argument "strict" indicates when approximations are to be computed:
 *      - if TRUE, approximations are only made if ALL args have at least
 *        one approximation.
 *      - if FLASE, approximations are made whenever new argument information
 *        is added.
 *
 ******************************************************************************/

ntype *
SDcreateSignatureDependency (ct_funptr CtFun, te_info *info, ntype *args, bool strict)
{
    sig_dep *sig;
    ntype *arg_t, *res_t;
    size_t num_args, num_res, i;
    bool ok = TRUE;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    /*
     * First, we create the return type as it is part of the sig_dep structure:
     */
    num_res = TEgetNumRets (info);

    res_t = TYmakeEmptyProductType (num_res);
    for (i = 0; i < num_res; i++) {
        res_t = TYsetProductMember (res_t, i, TYmakeAlphaType (NULL));
    }

    /*
     * Now, we create the structure itself:
     */
    sig = MakeSig (CtFun, info, TYcopyType (args), strict, res_t,
                   TYcountNonFixedAlpha (args));

    /*
     * Inserting the dependency into the non fixed vars:
     */
    num_args = TYgetProductSize (args);
    for (i = 0; i < num_args; i++) {
        arg_t = TYgetProductMember (args, i);
        if (TYisNonFixedAlpha (arg_t)) {
            ok = ok && SSIassumeLow (TYgetAlpha (arg_t), sig);
        }
    }
    DBUG_ASSERT (ok, "Something went wrong creating a signature dependency");

    DBUG_EXECUTE (tmp_str = SDsigDep2DebugString (sig));
    DBUG_PRINT ("sig dep created: handle %p : %s", (void *)sig, tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    /*
     * Finally, we try to create a first result type approximation
     */
    ok = SDhandleContradiction (sig);

    DBUG_ASSERT (ok, "Something went wrong creating a fundef signature dependency");

    DBUG_RETURN (TYcopyType (SD_RES (sig)));
}

/******************************************************************************
 *
 * function:
 *    bool SDhandleContradiction( node *fun_sig)
 *
 * description:
 *
 ******************************************************************************/

bool
SDhandleContradiction (sig_dep *fun_sig)
{
    ntype *res_vars, *res_t, *res, *args, *bottom;
    bool ok = FALSE;
    te_info *info;
    size_t i;
#ifndef DBUG_OFF
    char *tmp_str = NULL, *tmp2_str = NULL;
#endif

    DBUG_ENTER ();

    /*
     * First, we check whether there is enough information available for
     * making a (new) result type approximation.
     */
    if (SD_STRICT (fun_sig) && (TYcountNoMinAlpha (SD_ARGS (fun_sig)) > 0)) {
        /**
         * we cannot approximate yet, as the function is strict AND
         * some arguments are missing!
         */
        ok = TRUE;
    } else {
        info = SD_INFO (fun_sig);
        /*
         * Now, we compute a new approximation:
         */
        args = TYfixAndEliminateAlpha (SD_ARGS (fun_sig));
        bottom = TYgetBottom (args);
        if (bottom != NULL && SD_STRICT (fun_sig)) {
            /**
             * insert copies of bottom into the return types:
             */
            res_vars = SD_RES (fun_sig);
            ok = TRUE;
            for (i = 0; i < TYgetProductSize (res_vars); i++) {
                ok = ok
                     && SSInewMin (TYgetAlpha (TYgetProductMember (res_vars, i)),
                                   TYcopyType (bottom));
            }

        } else {
            /**
             * Compute an approximation!
             * BEWARE: in case of !SD_STRICT( fun_sig), we may have
             *         bottom args as well as noMIN args!!!
             */

            res_t = SD_FUN (fun_sig) (info, args);
            res_t = TYeliminateAlpha (res_t);

            DBUG_EXECUTE (tmp_str = TYtype2String (args, FALSE, 0));
            DBUG_EXECUTE (tmp2_str = TYtype2String (res_t, FALSE, 0));
            DBUG_PRINT ("approximating %s \"%s\" for %s yields %s", TEgetKindStr (info),
                        TEgetNameStr (info), tmp_str, tmp2_str);
            DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));
            DBUG_EXECUTE (tmp2_str = MEMfree (tmp_str));

            /*
             * and insert the findings into the return types:
             */
            res_vars = SD_RES (fun_sig);
            ok = TRUE;
            for (i = 0; i < TYgetProductSize (res_vars); i++) {
                res = TYgetProductMember (res_t, i);
                if (TYisAlpha (res)) {
                    ok = ok
                         && SSInewRel (TYgetAlpha (res),
                                       TYgetAlpha (TYgetProductMember (res_vars, i)));
                } else {
                    ok
                      = ok
                        && SSInewMin (TYgetAlpha (TYgetProductMember (res_vars, i)), res);
                }
            }
        }
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *    bool SDhandleElimination( sig_dep *fun_sig)
 *
 * description:
 *
 ******************************************************************************/

bool
SDhandleElimination (sig_dep *fun_sig)
{
    DBUG_ENTER ();
    DBUG_RETURN (TRUE);
}

/******************************************************************************
 *
 * function:
 *    char *SDsigDep2DebugString( sig_dep *fun_sig)
 *
 * description:
 *
 ******************************************************************************/

char *
SDsigDep2DebugString (sig_dep *fun_sig)
{
    static char buf[256];
    char *tmp = &buf[0];
    char *tmp_str;
    te_info *info;

    DBUG_ENTER ();

    info = SD_INFO (fun_sig);
    tmp += sprintf (tmp, "%s \"%s\"", TEgetKindStr (info), TEgetNameStr (info));

    tmp_str = TYtype2String (SD_ARGS (fun_sig), FALSE, 0);
    tmp += sprintf (tmp, "%s -> ", tmp_str);
    tmp_str = MEMfree (tmp_str);

    tmp_str = TYtype2String (SD_RES (fun_sig), FALSE, 0);
    tmp += sprintf (tmp, "%s rc:%d", tmp_str, SD_RC (fun_sig));
    tmp_str = MEMfree (tmp_str);

    DBUG_RETURN (STRcpy (buf));
}

/* @} */ /* addtogroup ntc */

#undef DBUG_PREFIX
