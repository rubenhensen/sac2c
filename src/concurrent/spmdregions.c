/*
 *
 * $Log$
 * Revision 1.1  1998/04/17 17:22:07  dkr
 * Initial revision
 *
 * Revision 1.6  1998/04/16 21:45:54  dkr
 * fixed a few bugs in ConcAssert:
 *   AP_FUNDEF of concregion-ap is set correctly
 *   refcounters of lifted concregion are now (more ;) correct
 *
 * Revision 1.5  1998/04/09 21:33:33  dkr
 * fixed a bug in ConcAssign
 *
 * Revision 1.4  1998/04/09 14:01:37  dkr
 * new funs ConcFundef, ConcNcode, ConcNpart, ...
 *
 * Revision 1.3  1998/04/03 21:07:33  dkr
 * changed ConcAssign
 *
 * Revision 1.2  1998/04/03 11:59:40  dkr
 * include order is now correct :)
 *
 * Revision 1.1  1998/04/03 11:37:21  dkr
 * Initial revision
 *
 *
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"

#include "internal_lib.h"
#include "traverse.h"

#include "typecheck.h"
#include "optimize.h"
#include "DupTree.h"
#include "dbug.h"

#include "spmdregions.h"

/* returns 0 for refcounting-objects and -1 otherwise */
#define GET_ZERO_REFCNT(vardec)                                                          \
    (NODE_TYPE (vardec) == N_arg) ? ((ARG_REFCNT (vardec) >= 0) ? 0 : -1)                \
                                  : ((VARDEC_REFCNT (vardec) >= 0) ? 0 : -1)

/******************************************************************************
 *
 * function:
 *   char *SpmdFunName(char *name)
 *
 * description:
 *   create a name for a spmd-fun.
 *   this name is build from the name of the current scope ('name')
 *    and an unambiguous number.
 *
 ******************************************************************************/

char *
SpmdFunName (char *name)
{
    static no;
    char *funname;

    DBUG_ENTER ("SpmdFunName");

    funname = (char *)Malloc ((strlen (name) + 10) * sizeof (char));
    sprintf (funname, "SPMD_%s_%d", name, no);

    DBUG_RETURN (funname);
}

/******************************************************************************
 *
 * function:
 *   node *FindVardec(int varno, node *fundef)
 *
 * description:
 *   returns the vardec of var number 'varno'
 *
 ******************************************************************************/

node *
FindVardec (int varno, node *fundef)
{
    node *tmp, *result = NULL;

    DBUG_ENTER ("FindVardec");

    if (result == NULL) {
        tmp = FUNDEF_ARGS (fundef);
        while (tmp != NULL) {
            if (ARG_VARNO (tmp) == varno) {
                result = tmp;
                break;
            } else {
                tmp = ARG_NEXT (tmp);
            }
        }
    }

    if (result == NULL) {
        tmp = BLOCK_VARDEC (FUNDEF_BODY (fundef));
        while (tmp != NULL) {
            if (VARDEC_VARNO (tmp) == varno) {
                result = tmp;
                break;
            } else {
                tmp = VARDEC_NEXT (tmp);
            }
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *CpmdFundef(node *arg_node, node *arg_info)
 *
 * description:
 *   fills INFO_SPMD_FUNDEF(info_node) with the current fundef
 *    --- needed for creation of spmd-funs.
 *
 ******************************************************************************/

node *
SpmdFundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SpmdFundef");

    /*
     * save current fundef in INFO_SPMD_FUNDEF
     */
    INFO_SPMD_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SpmdAssign(node *arg_node, node *arg_info)
 *
 * description:
 *   At the moment we simply generate one SPMD region for each
 *    first level with-loop.
 *   Then we build the fundec, funap and local vardecs for the lifted
 *    spmd-function and save them in SPMD_FUNDEC, SPMD_AP_LET and
 *    SPMD_VARDEC.
 *
 ******************************************************************************/

node *
SpmdAssign (node *arg_node, node *arg_info)
{
    node *spmdlet, *fundefs, *old_vardec;
    node *args, *arg, *last_arg;
    node *fargs, *farg, *last_farg;
    ids *lets, *let, *last_let;
    node *localvars, *localvar, *last_localvar;
    node *retexprs, *retexpr, *last_retexpr;
    types *rettypes, *rettype, *last_rettype;
    node *ret, *fundec, *ap, *spmd;
    char *name;
    int varno, i;

    DBUG_ENTER ("SpmdAssign");

    spmdlet = ASSIGN_INSTR (arg_node);

    if ((NODE_TYPE (spmdlet) == N_let) && (NODE_TYPE (LET_EXPR (spmdlet)) == N_Nwith)) {

        /*
         * current assignment contains a with-loop
         *  -> create a spmd region containing the current assignment only
         *      and insert it into the syntaxtree.
         */
        spmd = MakeSPMD (MakeBlock (MakeAssign (spmdlet, NULL), NULL));
        ASSIGN_INSTR (arg_node) = spmd;

        /*
         * we will often need the following values
         */
        fundefs = INFO_SPMD_FUNDEF (arg_info);
        varno = FUNDEF_VARNO (INFO_SPMD_FUNDEF (arg_info));

        /*
         * generate masks (for SPMD_USEDVARS, SPMD_DEFVARS, ...)
         *
         * we must start at the fundef node, to get the right value for VARNO
         *  in GenerateMasks().
         */
        INFO_SPMD_FUNDEF (arg_info) = GenerateMasks (INFO_SPMD_FUNDEF (arg_info), NULL);

        /*
         * traverse the 'spmdlet' node to generate INFO_SPMD_LOCALUSED,
         *  INFO_SPMD_LOCALDEF
         */
        INFO_SPMD_LOCALUSED (arg_info)
          = ReGenMask (INFO_SPMD_LOCALUSED (arg_info), varno);
        INFO_SPMD_LOCALDEF (arg_info) = ReGenMask (INFO_SPMD_LOCALDEF (arg_info), varno);
        spmdlet = Trav (spmdlet, arg_info);

        /*
         * subtract the local vars from SPMD_...VARS
         *  because these vars will not become args or returns of the
         *  lifted spmd-fun.
         */
        MinusMask (SPMD_USEDVARS (spmd), INFO_SPMD_LOCALUSED (arg_info), varno);
        MinusMask (SPMD_DEFVARS (spmd), INFO_SPMD_LOCALDEF (arg_info), varno);

        /*
         * generate SPMD_FUNDEC, SPMD_VARDEC, SPMD_AP_LET
         *  for this spmd region
         */
        name = SpmdFunName (FUNDEF_NAME (INFO_SPMD_FUNDEF (arg_info)));

        args = NULL;
        fargs = NULL;
        lets = NULL;
        localvars = NULL;
        retexprs = NULL;
        rettypes = NULL;
        for (i = 0; i < varno; i++) {
            DBUG_ASSERT ((((SPMD_USEDVARS (spmd))[i] >= 0)
                          && ((SPMD_DEFVARS (spmd))[i] >= 0)),
                         "wrong mask entry found");

            if ((SPMD_USEDVARS (spmd))[i] > 0) {

                old_vardec = FindVardec (i, fundefs);

                if (NODE_TYPE (old_vardec) == N_vardec) {
                    arg = MakeId (StringCopy (VARDEC_NAME (old_vardec)), NULL,
                                  VARDEC_STATUS (old_vardec));
                    ID_ATTRIB (arg) = VARDEC_ATTRIB (old_vardec);

                    farg = MakeArg (StringCopy (VARDEC_NAME (old_vardec)),
                                    DuplicateTypes (VARDEC_TYPE (old_vardec), 1),
                                    VARDEC_STATUS (old_vardec),
                                    VARDEC_ATTRIB (old_vardec), NULL);
                } else {
                    arg = MakeId (StringCopy (ARG_NAME (old_vardec)), NULL,
                                  ARG_STATUS (old_vardec));
                    ID_ATTRIB (arg) = ARG_ATTRIB (old_vardec);

                    farg
                      = MakeArg (StringCopy (ARG_NAME (old_vardec)),
                                 DuplicateTypes (ARG_TYPE (old_vardec), 1),
                                 ARG_STATUS (old_vardec), ARG_ATTRIB (old_vardec), NULL);
                }

                ID_VARDEC (arg) = old_vardec;
                ID_MAKEUNIQUE (arg) = 0;
                ID_REFCNT (arg) = GET_ZERO_REFCNT (old_vardec); /* dummy value */
                arg = MakeExprs (arg, NULL);

                ARG_REFCNT (farg) = GET_ZERO_REFCNT (old_vardec); /* dummy value */
                ARG_VARNO (farg) = -1; /* dummy value --- not needed anymore! */

                if (args == NULL) {
                    args = arg;
                    fargs = farg;
                } else {
                    EXPRS_NEXT (last_arg) = arg;
                    ARG_NEXT (last_farg) = farg;
                }
                last_arg = arg;
                last_farg = farg;
            }

            if ((SPMD_DEFVARS (spmd))[i] > 0) {

                old_vardec = FindVardec (i, fundefs);

                if (NODE_TYPE (old_vardec) == N_vardec) {
                    let = MakeIds (StringCopy (VARDEC_NAME (old_vardec)), NULL,
                                   VARDEC_STATUS (old_vardec));
                    IDS_ATTRIB (let) = VARDEC_ATTRIB (old_vardec);

                    if ((SPMD_USEDVARS (spmd))[i] > 0) {
                        /* the current var is an arg of the conregion-fun */
                        localvar = NULL;
                    } else {
                        /* the current var is a local var of the conregion-fun */
                        localvar = DupNode (old_vardec);
                    }

                    retexpr = MakeId (StringCopy (VARDEC_NAME (old_vardec)), NULL,
                                      VARDEC_STATUS (old_vardec));
                    ID_ATTRIB (retexpr) = VARDEC_ATTRIB (old_vardec);

                    rettype = DuplicateTypes (VARDEC_TYPE (old_vardec), 1);
                } else {
                    let = MakeIds (StringCopy (ARG_NAME (old_vardec)), NULL,
                                   ARG_STATUS (old_vardec));
                    IDS_ATTRIB (let) = ARG_ATTRIB (old_vardec);

                    if ((SPMD_USEDVARS (spmd))[i] > 0) {
                        /* the current var is an arg of the conregion-fun */
                        localvar = NULL;
                    } else {
                        /* the current var is a local var of the conregion-fun */
                        localvar
                          = MakeVardec (StringCopy (ARG_NAME (old_vardec)),
                                        DuplicateTypes (ARG_TYPE (old_vardec), 1), NULL);
                        VARDEC_STATUS (localvar) = ARG_STATUS (old_vardec);
                        VARDEC_ATTRIB (localvar) = ARG_ATTRIB (old_vardec);
                    }

                    retexpr = MakeId (StringCopy (ARG_NAME (old_vardec)), NULL,
                                      ARG_STATUS (old_vardec));
                    ID_ATTRIB (retexpr) = ARG_ATTRIB (old_vardec);

                    rettype = DuplicateTypes (ARG_TYPE (old_vardec), 1);
                }

                IDS_VARDEC (let) = old_vardec;
                /* dummy value --- do not change rc after aplication: */
                IDS_REFCNT (let) = GET_ZERO_REFCNT (old_vardec) + 1;

                if (localvar != NULL) {
                    VARDEC_TYPEDEF (localvar) = NULL;
                    VARDEC_REFCNT (localvar)
                      = GET_ZERO_REFCNT (old_vardec); /* dummy value */
                    VARDEC_VARNO (localvar) = -1; /* dummy value --- not needed anymore */
                }

                if ((SPMD_USEDVARS (spmd))[i] > 0) {
                    /* the current var is an arg of the conregion-fun */
                    ID_VARDEC (retexpr) = farg;
                } else {
                    /* the current var is a local var of the conregion-fun */
                    ID_VARDEC (retexpr) = localvar;
                }
                ID_MAKEUNIQUE (retexpr) = 0;
                ID_REFCNT (retexpr) = GET_ZERO_REFCNT (old_vardec); /* dummy value */
                retexpr = MakeExprs (retexpr, NULL);

                if (lets == NULL) {
                    lets = let;
                    retexprs = retexpr;
                    rettypes = rettype;
                } else {
                    IDS_NEXT (last_let) = let;
                    EXPRS_NEXT (last_retexpr) = retexpr;
                    TYPES_NEXT (last_rettype) = rettype;
                }
                last_let = let;
                last_retexpr = retexpr;
                last_rettype = rettype;

                if (localvar != NULL) {
                    if (localvars == NULL) {
                        localvars = localvar;
                    } else {
                        VARDEC_NEXT (last_localvar) = localvar;
                    }
                    last_localvar = localvar;
                }
            }
        }

        ret = MakeReturn (retexprs);
        RETURN_INWITH (ret) = 0;

        fundec = MakeFundef (name, NULL, rettypes, fargs, NULL, NULL);
        FUNDEF_STATUS (fundec) = ST_spmdfun;
        FUNDEF_ATTRIB (fundec) = ST_regular;
        FUNDEF_INLINE (fundec) = 0;
        FUNDEF_RETURN (fundec) = ret;

        ap = MakeAp (name, NULL, args);
        AP_FUNDEF (ap) = fundec;
        AP_ATFLAG (ap) = 1;

        SPMD_FUNDEC (spmd) = fundec;
        SPMD_VARDEC (spmd) = localvars;
        SPMD_AP_LET (spmd) = MakeLet (ap, lets);

        /*
         * we only traverse the following assignments to prevent nested
         *  spmd regions
         */
    } else {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SpmdNpart(node *arg_node, node *arg_info)
 *
 * description:
 *   all defined vars of the N_Npart node are local
 *    -> save them in INFO_SPMD_LOCALDEF.
 *
 ******************************************************************************/

node *
SpmdNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SpmdNpart");

    /*
     * the masks are correctly generated by 'refcount'
     */
    PlusMask (INFO_SPMD_LOCALDEF (arg_info), NPART_MASK (arg_node, 0),
              FUNDEF_VARNO (INFO_SPMD_FUNDEF (arg_info)));

    NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);
    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);
    if (NPART_NEXT (arg_node) != NULL) {
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SpmdNcode(node *arg_node, node *arg_info)
 *
 * description:
 *   all used and defined vars of the N_Ncode node are local
 *    -> save them in INFO_SPMD_LOCALUSED, INFO_SPMD_LOCALDEF.
 *
 ******************************************************************************/

node *
SpmdNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SpmdNcode");

    /*
     * the masks are correctly generated by 'refcount'
     */
    PlusMask (INFO_SPMD_LOCALUSED (arg_info), NPART_MASK (arg_node, 1),
              FUNDEF_VARNO (INFO_SPMD_FUNDEF (arg_info)));
    PlusMask (INFO_SPMD_LOCALDEF (arg_info), NPART_MASK (arg_node, 0),
              FUNDEF_VARNO (INFO_SPMD_FUNDEF (arg_info)));

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }
    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SpmdRegions(node *syntax_tree)
 *
 * description:
 *   in this compiler phase we mark the regions of the syntax-tree
 *    (chains of assignments actually) we want to generate
 *    spmd C-code for.
 *
 ******************************************************************************/

node *
SpmdRegions (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("SpmdRegions");

    info = MakeInfo ();

    act_tab = spmdregions_tab;
    syntax_tree = Trav (syntax_tree, info);

    FREE (info);

    DBUG_RETURN (syntax_tree);
}
