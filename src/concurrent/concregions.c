/*
 *
 * $Log$
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

#include "concregions.h"

/******************************************************************************
 *
 * function:
 *   char *ConcFunName(char *name)
 *
 * description:
 *   create a name for a concregion-fun.
 *   this name is build from the name of the current scope ('name')
 *    and an unambiguous number.
 *
 ******************************************************************************/

char *
ConcFunName (char *name)
{
    static no;
    char *funname;

    DBUG_ENTER ("ConcFunName");

    funname = (char *)Malloc ((strlen (name) + 10) * sizeof (char));
    sprintf (funname, "CONC_%s_%d", name, no);

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
 *   node *ConcFundef(node *arg_node, node *arg_info)
 *
 * description:
 *   fills INFO_CONC_FUNDEF(info_node) with the current fundef
 *    --- needed for creation of concregion--funs.
 *
 ******************************************************************************/

node *
ConcFundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ConcFundef");

    /*
     * save current fundef in INFO_CONC_FUNDEF
     */
    INFO_CONC_FUNDEF (arg_info) = arg_node;

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
 *   node *ConcAssign(node *arg_node, node *arg_info)
 *
 * description:
 *   At the moment we simply generate one concurrent region for each
 *    first level with-loop.
 *   Then we build the fundec, funap and local vardecs for the lifted
 *    concregion-function and save them in CONC_FUNDEC, CONC_AP_LET and
 *    CONC_VARDEC.
 *
 ******************************************************************************/

node *
ConcAssign (node *arg_node, node *arg_info)
{
    node *conclet, *fundefs, *old_vardec;
    node *args, *arg, *last_arg;
    node *fargs, *farg, *last_farg;
    ids *lets, *let, *last_let;
    node *localvars, *localvar, *last_localvar;
    node *retexprs, *retexpr, *last_retexpr;
    types *rettypes, *rettype, *last_rettype;
    node *ret, *fundec, *ap, *conc;
    char *name;
    int varno, i;

    DBUG_ENTER ("ConcAssign");

    conclet = ASSIGN_INSTR (arg_node);

    if ((NODE_TYPE (conclet) == N_let) && (NODE_TYPE (LET_EXPR (conclet)) == N_Nwith)) {

        /*
         * current assignment contains a with-loop
         *  -> create a concurrent region containing the current assignment only
         *      and insert it into the syntaxtree.
         */
        conc = MakeConc (MakeBlock (MakeAssign (conclet, NULL), NULL));
        ASSIGN_INSTR (arg_node) = conc;

        /*
         * we will often need the following values
         */
        fundefs = INFO_CONC_FUNDEF (arg_info);
        varno = FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info));

        /*
         * generate masks (for CONC_USEDVARS, CONC_DEFVARS, ...)
         *
         * we must start at the fundef node, to get the right value for VARNO
         *  in GenerateMasks().
         */
        INFO_CONC_FUNDEF (arg_info) = GenerateMasks (INFO_CONC_FUNDEF (arg_info), NULL);

        /*
         * traverse the 'conclet' node to generate INFO_CONC_LOCALUSED,
         *  INFO_CONC_LOCALDEF
         */
        INFO_CONC_LOCALUSED (arg_info)
          = ReGenMask (INFO_CONC_LOCALUSED (arg_info), varno);
        INFO_CONC_LOCALDEF (arg_info) = ReGenMask (INFO_CONC_LOCALDEF (arg_info), varno);
        conclet = Trav (conclet, arg_info);

        /*
         * subtract the local vars from CONC_...VARS
         *  because these vars will not become args or returns of the
         *  lifted concregion-fun.
         */
        MinusMask (CONC_USEDVARS (conc), INFO_CONC_LOCALUSED (arg_info), varno);
        MinusMask (CONC_DEFVARS (conc), INFO_CONC_LOCALDEF (arg_info), varno);

        /*
         * generate CONC_FUNDEC, CONC_VARDEC, CONC_AP_LET
         *  for this concurrent region
         */
        name = ConcFunName (FUNDEF_NAME (INFO_CONC_FUNDEF (arg_info)));

        args = NULL;
        fargs = NULL;
        lets = NULL;
        localvars = NULL;
        retexprs = NULL;
        rettypes = NULL;
        for (i = 0; i < varno; i++) {
            DBUG_ASSERT ((((CONC_USEDVARS (conc))[i] >= 0)
                          && ((CONC_DEFVARS (conc))[i] >= 0)),
                         "wrong mask entry found");

            if ((CONC_USEDVARS (conc))[i] > 0) {

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
                ID_REFCNT (arg) = -1; /* not needed anymore !?! */
                arg = MakeExprs (arg, NULL);

                /* the function body is statically refcounted! */
                ARG_REFCNT (farg) = 0;
                ARG_VARNO (farg) = -1; /* not needed anymore !?! */

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

            if ((CONC_DEFVARS (conc))[i] > 0) {

                old_vardec = FindVardec (i, fundefs);

                if (NODE_TYPE (old_vardec) == N_vardec) {
                    let = MakeIds (StringCopy (VARDEC_NAME (old_vardec)), NULL,
                                   VARDEC_STATUS (old_vardec));
                    IDS_ATTRIB (let) = VARDEC_ATTRIB (old_vardec);

                    if ((CONC_USEDVARS (conc))[i] > 0) {
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

                    if ((CONC_USEDVARS (conc))[i] > 0) {
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
                IDS_REFCNT (let) = -1; /* not needed anymore !?! */

                if (localvar != NULL) {
                    VARDEC_TYPEDEF (localvar) = NULL;
                    VARDEC_REFCNT (localvar) = 0; /* ??? */
                    VARDEC_VARNO (localvar) = -1; /* ??? */
                }

                if ((CONC_USEDVARS (conc))[i] > 0) {
                    /* the current var is an arg of the conregion-fun */
                    ID_VARDEC (retexpr) = farg;
                } else {
                    /* the current var is a local var of the conregion-fun */
                    ID_VARDEC (retexpr) = localvar;
                }
                ID_MAKEUNIQUE (retexpr) = 0;
                ID_REFCNT (retexpr) = -1; /* not needed anymore !?! */
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
        FUNDEF_STATUS (fundec) = ST_concfun;
        FUNDEF_ATTRIB (fundec) = ST_regular;
        FUNDEF_INLINE (fundec) = 0;
        FUNDEF_RETURN (fundec) = ret;

        ap = MakeAp (name, NULL, args);
        AP_ATFLAG (ap) = 1;

        CONC_FUNDEC (conc) = fundec;
        CONC_VARDEC (conc) = localvars;
        CONC_AP_LET (conc) = MakeLet (ap, lets);

        /*
         * we only traverse the following assignments to prevent nested
         *  concurrent regions
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
 *   node *ConcNpart(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
ConcNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ConcNpart");

    PlusMask (INFO_CONC_LOCALDEF (arg_info), NPART_MASK (arg_node, 0),
              FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info)));

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
 *   node *ConcNcode(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
ConcNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("ConcNcode");

    PlusMask (INFO_CONC_LOCALUSED (arg_info), NPART_MASK (arg_node, 1),
              FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info)));

    PlusMask (INFO_CONC_LOCALDEF (arg_info), NPART_MASK (arg_node, 0),
              FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info)));

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
 *   node *ConcRegions(node *syntax_tree)
 *
 * description:
 *   in this compiler phase we mark the regions of the syntax-tree
 *    (chains of assignments actually) we want to generate
 *    concurrent C-code for.
 *
 ******************************************************************************/

node *
ConcRegions (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("ConcRegions");

    info = MakeInfo ();

    act_tab = concregions_tab;
    syntax_tree = Trav (syntax_tree, info);

    FREE (info);

    DBUG_RETURN (syntax_tree);
}
