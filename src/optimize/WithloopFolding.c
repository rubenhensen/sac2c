/*      $Id$
 *
 * $Log$
 * Revision 1.7  1998/03/18 08:33:07  srs
 * first running version of WLI
 *
 * Revision 1.6  1998/03/06 13:32:54  srs
 * added some WLI functions
 *
 * Revision 1.5  1998/02/27 13:38:10  srs
 * checkin with deactivated traversal
 *
 * Revision 1.4  1998/02/24 14:19:20  srs
 * *** empty log message ***
 *
 * Revision 1.3  1998/02/09 15:58:20  srs
 * *** empty log message ***
 *
 *
 */

/*******************************************************************************
 This file realizes the WL-folding for the new SAC-WLs.

 Withloop folding is done in 2 phases:
 1) WLI to gather information about the WLs
 2) WLF to find and fold suitable WLs.

 Assumption: We assume that all generators of a WL have the same
 shape.  Furthermore we assume that, if an N_Ncode is referenced by
 more than one generator, all these generators' indexes (vector and
 scalar) have the same names. This 'same name assumption' can even be
 expanded to all generators a WL has. This is true as a consequence of
 the folding mechanism we apply (induction): At the beginning, all WL
 have only one generator and it is obviously true. When another WL's
 body (foreign body) is folded into a this WL's body, the body is
 copied inclusive the generator indices. The foreign indices of the
 other WL are transformed (based on the origial indicies) to temp
 variables, which are applied to the substituted foreign body.

 Usage of arg_info (WLI):
 - node[0]: store old information in nested WLs
 - node[1]: reference to base node of current WL (N_Nwith)
 - node[2]: always the last N_assign node (see WLIassign)
 - counter: nesting level of WLs.

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "types.h"
#include "internal_lib.h"
#include "free.h"
#include "print.h"
#include "DupTree.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "optimize.h"
#include "ConstantFolding.h"
#include "WithloopFolding.h"

/******************************************************************************
 *
 * types, global variables
 *
 ******************************************************************************/

/* The following struct may only be annotated to N_assign nodes which are
   inside a WL body and which have ASSIGN_INSTRs N_let and N_prf(F_psi). */
typedef struct INDEX_INFO {
    int vector;               /* this is an index vector (>0) or a scalar (0)
                                 in case of a vector this number is the
                                 shape of the vector. */
    int *permutation;         /* Describes the permutation of index vars in
                                 this vector (if this is a vector) or stores
                                 the base scalar index in permutation[0].
                                 The index scarales are counted starting
                                 with 1. E.g. in [i,j,k] j has the no 2.
                                 If one elements is not based on an index
                                 (ONLY a constant, else the vector is not
                                 valid) this value is 0 and the constant
                                 can be found in const_arg. */
    struct INDEX_INFO **last; /* points to last transformations */

    /* the next 3 components describe the executed transformation */
    prf prf;        /* prf +,-,*,/ or */
    int *const_arg; /* the constant arg has to be an integer.
                       For every element of a vector there is an
                       own constant arg.
                       If this struct is an annotation for a scalar,
                       only const_arg[0] is valid.
                       If the corresponding permutation is 0, the
                       vector's element is a constant which is
                       stored here (no prf arg). */
    int arg_no;     /* const_arg is the first (1) or second (2)
                       argument of prf. arg_no may be 0 which
                       means that no prf given. Can only be in:
                        tmp = [i,j,c];
                        val = psi(...,tmp);
                       Well, can also happen when CF is deacivated.
                       If arg_no is 0, prf is undefined */
} index_info;

#define INDEX(n) ((index_info *)ASSIGN_INDEX (n))
#define DEF_MASK 0
#define USE_MASK 1

#define FREE_INDEX(tmp)                                                                  \
    {                                                                                    \
        FREE (tmp->permutation);                                                         \
        FREE (tmp->last);                                                                \
        FREE (tmp->const_arg);                                                           \
        FREE (tmp);                                                                      \
        tmp = NULL;                                                                      \
    }

/******************************************************************************
 *
 * function:
 *   void DbugIndexInfo(index info *iinfo)
 *
 * description:
 *   prints history of iinfo.
 *
 *
 ******************************************************************************/

void
DbugIndexInfo (index_info *iinfo)
{
    int i, sel;
    index_info *tmpii;

    DBUG_ENTER ("DbugIndexInfo");

    if (!iinfo)
        printf ("\nNULL\n");
    else if (iinfo->vector) {
        printf ("\nVECTOR shape [%d]:\n", iinfo->vector);
        for (i = 0; i < iinfo->vector; i++) {
            printf ("---%d---\n", i);

            if (!iinfo->permutation[i]) { /* constant */
                printf ("  constant %d\n", iinfo->const_arg[i]);
                continue;
            }

            printf ("  base %d\n", iinfo->permutation[i]);
            tmpii = iinfo;
            while (tmpii) {
                sel = tmpii->vector ? i : 0;
                if (tmpii->arg_no) {
                    if (1 == tmpii->arg_no)
                        printf ("   %d%s. ", tmpii->const_arg[sel],
                                prf_string[tmpii->prf]);
                    else
                        printf ("   .%s%d ", prf_string[tmpii->prf],
                                tmpii->const_arg[sel]);
                } else
                    printf ("   no prf ");
                printf ("(p:%d, v:%d)\n", tmpii->permutation[sel], tmpii->vector);
                tmpii = tmpii->last[sel];
            }
        }
    } else {
        printf ("\nSCALAR:\n");
        printf ("  base %d\n", iinfo->permutation[0]);
        tmpii = iinfo;
        sel = 0;
        if (tmpii->arg_no) {
            if (1 == tmpii->arg_no)
                printf ("   %d%s. ", tmpii->const_arg[sel], prf_string[tmpii->prf]);
            else
                printf ("   %s%d. ", prf_string[tmpii->prf], tmpii->const_arg[sel]);
            printf ("(p:%d, v:%d)\n", tmpii->permutation[sel], tmpii->vector);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   index_info *CreateIndex(int vector)
 *
 * description:
 *   create an incarnation of INDEX_INFO.
 *
 * remark:
 *   the argument 'vector' behaves like the component 'vector' of INDEX_INFO.
 *
 * reference:
 *   FREE_INDEX
 *
 ******************************************************************************/

index_info *
CreateIndex (int vector)
{
    index_info *pindex;
    DBUG_ENTER ("CreateInfoInfo");

    pindex = Malloc (sizeof (index_info));
    pindex->vector = vector;

    if (!vector)
        vector = 1;
    pindex->permutation = Malloc (sizeof (int) * vector);
    pindex->last = Malloc (sizeof (index_info *) * vector);
    pindex->const_arg = Malloc (sizeof (int) * vector);

    pindex->arg_no = 0;

    DBUG_RETURN (pindex);
}

/******************************************************************************
 *
 * function:
 *   index_info *DuplicateIndexInfo(index_info *iinfo)
 *
 * description:
 *   duplicates struct
 *
 *
 ******************************************************************************/

index_info *
DuplicateIndexInfo (index_info *iinfo)
{
    index_info *new;
    int i, to;
    DBUG_ENTER ("DuplicateIndexInfo");
    DBUG_ASSERT (iinfo, ("parameter NULL"));

    new = CreateIndex (iinfo->vector);

    to = iinfo->vector ? iinfo->vector : 1;
    for (i = 0; i < to; i++) {
        new->permutation[i] = iinfo->permutation[i];
        new->last[i] = iinfo->last[i];
        new->const_arg[i] = iinfo->const_arg[i];
    }

    new->prf = iinfo->prf;
    new->arg_no = iinfo->arg_no;

    DBUG_RETURN (new);
}

/******************************************************************************
 *
 * function:
 *   prf SimplifyFun(prf prf)
 *
 * description:
 *   Transforms special prf names (e.g. F_add_SxA) to the base name (F_add).
 *   Does this for add, sub, mul, div.
 *
 ******************************************************************************/

prf
SimplifyFun (prf prf)
{
    DBUG_ENTER ("SimplifyFun");

    switch (prf) {
    case F_add_SxA:
    case F_add_AxS:
    case F_add_AxA:
        prf = F_add;
        break;

    case F_sub_SxA:
    case F_sub_AxS:
    case F_sub_AxA:
        prf = F_sub;
        break;

    case F_mul_SxA:
    case F_mul_AxS:
    case F_mul_AxA:
        prf = F_mul;
        break;

    case F_div_SxA:
    case F_div_AxS:
    case F_div_AxA:
        prf = F_div;
        break;

    default:
        break;
    }

    DBUG_RETURN (prf);
}

/******************************************************************************
 *
 * function:
 *   node *ValicLocalId(node *idn)
 *
 * description:
 *   returns pointer to index_info if Id (idn) is a valid variable within
 *   the WL body (index vars are excluded). Returns NULL otherwise.
 *
 ******************************************************************************/

index_info *
ValidLocalId (node *idn)
{
    index_info *iinfo;

    DBUG_ENTER ("ValicLocalId");
    DBUG_ASSERT (N_id == NODE_TYPE (idn), ("not an id node"));

    idn = MRD (ID_VARNO (idn));
    if (idn)
        iinfo = INDEX (idn);
    else
        iinfo = NULL;

    DBUG_RETURN (iinfo);
}

/******************************************************************************
 *
 * function:
 *   int LocateIndexVar(node *idn, node* wln)
 *
 * description:
 *   Searches for the Id (idn) in the WL generator (index var).
 *   The N_with node has to be available to find the index vars.
 *
 * return:
 *   -1: Id is the index vector
 *    0: Id not found
 *    x with x gt 0: Id is the x'th scalar index variable.
 *
 * remark:
 *   we exploit here that the index variables of all N_Nwithid nodes in
 *   one WL have the same names.
 *
 ******************************************************************************/

int
LocateIndexVar (node *idn, node *wln)
{
    ids *_ids;
    int result = 0, i;

    DBUG_ENTER ("LocateIndexVar");
    DBUG_ASSERT (N_Nwith == NODE_TYPE (wln), ("wln is not N_Nwith node"));

    wln = NPART_WITHID (NWITH_PART (wln));
    _ids = NWITHID_VEC (wln);

    if (!strcmp (IDS_NAME (_ids), ID_NAME (idn)))
        result = -1;

    i = 1;
    _ids = NWITHID_IDS (wln);
    while (_ids && !result) {
        if (!strcmp (IDS_NAME (_ids), ID_NAME (idn)))
            result = i;
        i++;
        _ids = IDS_NEXT (_ids);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   void CheckOptimizePsi(node **psi, node *arg_info)
 *
 * description:
 *   checks if *psi is an application with index vector and constant. If
 *   possible, the prf psi is replaced by A scalar index vector.
 *
 * example:
 *   tmp = iv[[1]];       =>        tmp = j;
 *
 ******************************************************************************/

void
CheckOptimizePsi (node **psi, node *arg_info)
{
    int index;
    node *ivn, *indexn, *datan;
    ids *_ids;

    DBUG_ENTER ("CheckOptimizePsi");

    /* first check if the array is the index vector and the index is a
       constant in range. */
    ivn = PRF_ARG2 ((*psi));
    indexn = PRF_ARG1 ((*psi));
    if (N_id == NODE_TYPE (indexn))
        MRD_GETDATA (datan, ID_VARNO (indexn), INFO_VARNO);

    if (datan && N_array == NODE_TYPE (datan)
        && N_num == NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (datan)))
        && -1 == LocateIndexVar (ivn, INFO_WLI_WL (arg_info))) {
        index = NUM_VAL (EXPRS_EXPR (ARRAY_AELEMS (datan)));

        /* find index'th scalar index var */
        _ids = NPART_IDS (NWITH_PART (INFO_WLI_WL (arg_info)));
        while (index > 0 && IDS_NEXT (_ids)) {
            index--;
            _ids = IDS_NEXT (_ids);
        }

        if (!index) { /* found scalar index var. */
            INFO_USE[ID_VARNO (ivn)]--;
            if (N_id == NODE_TYPE (indexn))
                INFO_USE[ID_VARNO (indexn)]--;
            FreeTree (*psi);
            *psi = MakeId (IDS_NAME (_ids), NULL, ST_regular);
            ID_VARDEC ((*psi)) = IDS_VARDEC (_ids);
            INFO_USE[IDS_VARNO (_ids)]++;
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void CheckOptimizeArray(node **array, node *arg_info)
 *
 * description:
 *   Checks if array is constructed from all scalar index variables
 *   with permutation == identity. Then the array subtree is replaced
 *   by the index vector.
 *
 * example:
 *   sel = [i,j,k];       =>       sel = iv;
 *
 ******************************************************************************/

void
CheckOptimizeArray (node **array, node *arg_info)
{
    int elts, i;
    ids *_ids;
    node *tmpn;

    DBUG_ENTER ("CheckOptimizeArray");
    DBUG_ASSERT (N_array == NODE_TYPE (*array), ("no N_array node"));

    /* shape of index vector */
    _ids = NPART_VEC (NWITH_PART (INFO_WLI_WL (arg_info)));

    tmpn = ARRAY_AELEMS ((*array));
    elts = 0;
    while (tmpn) {
        if (N_id == NODE_TYPE (EXPRS_EXPR (tmpn))
            && LocateIndexVar (EXPRS_EXPR (tmpn), INFO_WLI_WL (arg_info)) == elts + 1) {
            elts++;
            tmpn = EXPRS_NEXT (tmpn);
        } else {
            tmpn = NULL;
            elts = 0;
        }
    }

    if (elts == IDS_SHAPE (_ids, 0)) { /* change to index vector */
        /* adjust USE mask */
        tmpn = ARRAY_AELEMS ((*array));
        for (i = 0; i < elts; i++) {
            INFO_USE[ID_VARNO (EXPRS_EXPR (tmpn))]--;
            tmpn = EXPRS_NEXT (tmpn);
        }

        /* free subtree and make new id node. */
        FreeTree (*array);
        *array = MakeId (IDS_NAME (_ids), NULL, ST_regular);
        ID_VARDEC ((*array)) = IDS_VARDEC (_ids);
        INFO_USE[IDS_VARNO (_ids)]++;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   index_info *Scalar2ArrayIndex(node *letn, node *wln);
 *
 * description:
 *   arrayn has to be N_array. This function checks if the array is
 *   a valid construction of an index vector within the body. The
 *   elements have to be constants, scalar index vars or transformations
 *   of scalar index vars.
 *   Every scalar base index may only appear once (i.e. no [i,2,i] is allowed).
 *   This can be switched by TRANSF_TRUE_PERMUTATIONS.
 *   If the array is equivalent to the index vector it is replaced. This is
 *   an optimization for the compiler phase.
 *
 * example:
 *   (... <= iv=[i,j] < ...)       (index vector)
 *   t = 3*i;
 *   sel = [j,t,3]                 (valid)
 *
 * return:
 *   returns NULL if vector is not valid or else index_info with arg_no == 0.
 *
 ******************************************************************************/

index_info *
Scalar2ArrayIndex (node *arrayn, node *wln)
{
    index_info *iinfo, *tmpii;
    int elts = 1, ok = 1, i, *valid_permutation;
    node *idn;

    DBUG_ENTER ("Scalar2ArrayIndex");
    DBUG_ASSERT (N_array == NODE_TYPE (arrayn), ("wrong nodetype (array)"));

    elts = TYPES_SHAPE (ARRAY_TYPE (arrayn), 0);
    arrayn = ARRAY_AELEMS (arrayn);

    iinfo = CreateIndex (elts);
    valid_permutation = Malloc (sizeof (int) * elts);
    for (i = 0; i < elts;)
        valid_permutation[i++] = 0;

    for (i = 0; i < elts && ok; i++) {
        /* check each element. */
        ok = 0;
        iinfo->last[i] = NULL;
        idn = EXPRS_EXPR (arrayn);
        if (N_num == NODE_TYPE (idn)) { /* this is a constant */
            iinfo->permutation[i] = 0;
            iinfo->const_arg[i] = NUM_VAL (idn);
            ok = 1;
        }

        if (N_id == NODE_TYPE (idn)) {
            tmpii = ValidLocalId (idn);
            if (tmpii &&          /* this is a local id, not index var */
                !tmpii->vector) { /* and scalar, not vector */
                iinfo->permutation[i] = tmpii->permutation[0];
                iinfo->last[i] = tmpii;
                /* may only be incremented once */
#ifdef TRANSF_TRUE_PERMUTATIONS
                ok = 1 == ++valid_permutation[iinfo->permutation[i] - 1];
#else
                ok = 1;
#endif
            } else if (0 < (iinfo->permutation[i] = LocateIndexVar (idn, wln))) {
                /* index scalar */
#ifdef TRANSF_TRUE_PERMUTATIONS
                ok = 1 == ++valid_permutation[iinfo->permutation[i] - 1];
#else
                ok = 1;
#endif
            }
        }

        arrayn = EXPRS_NEXT (arrayn);
    }

    if (!ok)
        FREE (iinfo);
    FREE (valid_permutation);

    DBUG_RETURN (iinfo);
}

/******************************************************************************
 *
 * function:
 *   void CreateIndexInfoId(node *idn, node *arg_info)
 *
 * description:
 *   creates an index_info from an Id. This could be an index vector, an
 *   index scalar or just a local valid variable.
 *   If the Id is vaild, the index_info is created/duplicated and assigned
 *   to INDEX(assignn).
 *
 ******************************************************************************/

void
CreateIndexInfoId (node *idn, node *arg_info)
{
    index_info *iinfo;
    node *assignn, *wln;
    int index_var, i, elts;

    DBUG_ENTER ("CreateIndexInfoId");

    assignn = INFO_WLI_ASSIGN (arg_info);
    wln = INFO_WLI_WL (arg_info);

    /* index var? */
    index_var = LocateIndexVar (idn, wln);
    if (index_var) {
        iinfo = CreateIndex (index_var > 0 ? 0 : ID_SHAPE (idn, 0));
        INDEX (assignn) = iinfo; /* make this N_assign valid */

        if (-1 == index_var) { /* index vector */
            elts = ID_SHAPE (idn, 0);
            for (i = 0; i < elts; i++) {
                iinfo->last[i] = NULL;
                iinfo->permutation[i] = i + 1;
            }
        } else { /* index scalar */
            iinfo->permutation[0] = index_var;
            iinfo->last[0] = NULL;
        }
    }

    /* valid local variable */
    iinfo = ValidLocalId (idn);
    if (iinfo)
        INDEX (assignn) = DuplicateIndexInfo (iinfo);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void CreateIndexInfoSxS(node *prfn, node *arg_info)
 *
 * description:
 *   checks if application of the prfn is a valid transformation in
 *   this WL wln and, if valid, creates an INDEX_INFO at the assignn.
 *
 ******************************************************************************/

void
CreateIndexInfoSxS (node *prfn, node *arg_info)
{
    int id_no = 0, index_var = 0;
    index_info *iinfo;
    node *idn, *assignn, *wln;

    DBUG_ENTER (" CreateIndexInfoSxS");

    assignn = INFO_WLI_ASSIGN (arg_info);
    wln = INFO_WLI_WL (arg_info);

    /* CF has been done, so we just search for an Id and a constant.
       Since we do not want to practice constant folding here we ignore
       prfs with two constants. */
    if (N_id == NODE_TYPE (PRF_ARG1 (prfn)) &&  /* first arg is an Id */
        N_num == NODE_TYPE (PRF_ARG2 (prfn))) { /* second arg is a numeric constant */
        id_no = 1;
        idn = PRF_ARG1 (prfn);
    } else if (N_id == NODE_TYPE (PRF_ARG2 (prfn))
               && N_num == NODE_TYPE (PRF_ARG1 (prfn))) {
        id_no = 2;
        idn = PRF_ARG2 (prfn);
    }

    if (id_no) {
        /* we found a constant and an Id. If this Id is a vaild Id (i.e.
           it is declared in the generator or it is a valid local Id)
           this transformation is valid, too. */
        iinfo = ValidLocalId (idn);
        if (!iinfo) /* maybe it's an index var. */
            index_var = LocateIndexVar (idn, wln);

        if (iinfo || index_var) {
            iinfo = CreateIndex (0); /* create a scalar index_info */
            INDEX (assignn) = iinfo; /* make this N_assign valid */

            /* if the Id is an index var... */
            if (index_var) {
                iinfo->last[0] = NULL;
                iinfo->permutation[0] = index_var; /* set permutation, always != -1 */
            } else {
                iinfo->last[0] = ValidLocalId (idn);
                /* else copy permutation from last index_info */
                iinfo->permutation[0] = iinfo->last[0]->permutation[0];
            }

            iinfo->prf = SimplifyFun (PRF_PRF (prfn));
            iinfo->const_arg[0]
              = NUM_VAL ((1 == id_no ? PRF_ARG2 (prfn) : PRF_ARG1 (prfn)));
            iinfo->arg_no = 1 == id_no ? 2 : 1;
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void CreateIndexInfoA(node *prfn, node *arg_info)
 *
 * description:
 *   checks if application of the prfn is a valid transformation in
 *   this WL wln and, if valid, creates an INDEX_INFO at the assignn.
 *   In contrast to CreateIndexInfoSxS the result of the prfn always is a
 *   vector.
 *
 *   We have to detect the following cases:
 *   (iv: id of an index vector, i : index scalar, c : constant.
 *    Something like [c,c,c] stands for [c1,c2,c3,...])
 *
 *   AxS (SxA): iv prfop c
 *              [i,i,c] prfop c
 *   AxA      : iv prfop [c,c,c]
 *              [i,i,c] prfop [c,c,c]
 *
 *   not valid: i prfop [c,c,c]
 *
 ******************************************************************************/

void
CreateIndexInfoA (node *prfn, node *arg_info)
{
    int id_no = 0, elts, i, index, val;
    node *idn, *constn, *tmpn, *cf_node, *args[2], *assignn, *wln;
    index_info *iinfo, *tmpinfo;
    types *type;

    DBUG_ENTER (" CreateIndexInfoA");

    assignn = INFO_WLI_ASSIGN (arg_info);
    wln = INFO_WLI_WL (arg_info);

    /* Which argument is the constant (so which will be the Id)? */
    if (N_num == NODE_TYPE (PRF_ARG1 (prfn))
        || IsConstantArray (PRF_ARG1 (prfn), N_num)) {
        id_no = 2;
        idn = PRF_ARG2 (prfn);
        constn = PRF_ARG1 (prfn);
    } else if (N_num == NODE_TYPE (PRF_ARG2 (prfn))
               || IsConstantArray (PRF_ARG2 (prfn), N_num)) {
        id_no = 1;
        idn = PRF_ARG1 (prfn);
        constn = PRF_ARG2 (prfn);
    }

    /* is this array exactly the index vector ? Then optimize. */
    if (N_array == NODE_TYPE (idn))
        if (1 == id_no)
            CheckOptimizeArray (&(PRF_ARG1 (prfn)), arg_info);
        else
            CheckOptimizeArray (&(PRF_ARG2 (prfn)), arg_info);

    /* Is idn an Id of an index vector (or transformation)? */
    if (id_no && N_id == NODE_TYPE (idn)) {
        tmpinfo = ValidLocalId (idn);
        index = LocateIndexVar (idn, wln);

        /* The Id is the index vector itself or, else it has to
           be an Id which is a valid vector. It must not be based
           on an index scalar (we do want "i prfop [c,c,c]"). */
        if (-1 == index ||                                 /* index vector itself */
            (tmpinfo && 1 == TYPES_DIM (ID_TYPE (idn)))) { /* valid local Id (vector) */
            elts = ID_SHAPE (idn, 0);
            iinfo = CreateIndex (elts);
            INDEX (assignn) = iinfo; /* make this N_assign valid */

            iinfo->arg_no = 1 == id_no ? 2 : 1;
            iinfo->prf = SimplifyFun (PRF_PRF (prfn));

            if (N_num == NODE_TYPE (constn))
                val = NUM_VAL (constn);
            else
                tmpn = ARRAY_AELEMS (constn);
            for (i = 0; i < elts; i++) {
                if (N_num != NODE_TYPE (constn)) {
                    DBUG_ASSERT (tmpn, ("Too few elements in array"));
                    val = NUM_VAL (EXPRS_EXPR (tmpn));
                    tmpn = EXPRS_NEXT (tmpn);
                }

                if (-1 == index) { /* index vector */
                    iinfo->last[i] = NULL;
                    iinfo->permutation[i] = i + 1;
                } else { /* local var, not index vector */
                    if ((iinfo->permutation[i] = tmpinfo->permutation[i]))
                        iinfo->last[i] = tmpinfo;
                    else
                        iinfo->last[i] = NULL; /* elt is constant */
                }

                if (iinfo->permutation[i])
                    iinfo->const_arg[i] = val;
                else {
                    type = MakeType (T_int, 0, NULL, NULL, NULL);
                    args[0] = MakeNum (val);
                    args[1] = MakeNum (tmpinfo->const_arg[i]);
                    cf_node = ScalarPrf (args, PRF_PRF (prfn), type, 2 == iinfo->arg_no);
                    iinfo->const_arg[i] = NUM_VAL (cf_node);
                    FREE (type);
                    FREE (args[0]); /* *cf_node == *args[0] */
                    FREE (args[1]);
                }
            }
        } /* this Id is valid. */
    }     /* this is an Id. */

    /* Is it a contruction based on index scalars ([i,i,c])? */
    if (id_no && N_array == NODE_TYPE (idn)) {
        iinfo = Scalar2ArrayIndex (idn, wln);
        if (iinfo) {
            /* This is a valid vector. Permutation and last of index_info are
               already set. But we still need to handle the prf. */
            elts = iinfo->vector;
            INDEX (assignn) = iinfo;

            iinfo->arg_no = 1 == id_no ? 2 : 1;
            iinfo->prf = SimplifyFun (PRF_PRF (prfn));

            if (N_num == NODE_TYPE (constn))
                val = NUM_VAL (constn);
            else
                tmpn = ARRAY_AELEMS (constn);
            for (i = 0; i < elts; i++) {
                if (N_num != NODE_TYPE (constn)) {
                    DBUG_ASSERT (tmpn, ("Too few elements in array"));
                    val = NUM_VAL (EXPRS_EXPR (tmpn));
                    tmpn = EXPRS_NEXT (tmpn);
                }

                /* is the element in the other vector a constant, too?
                   Then we have to fold immedeately. */
                if (!iinfo->permutation[i]) {
                    type = MakeType (T_int, 0, NULL, NULL, NULL);
                    args[0] = MakeNum (val);
                    args[1] = MakeNum (iinfo->const_arg[i]);
                    cf_node = ScalarPrf (args, PRF_PRF (prfn), type, 2 == iinfo->arg_no);
                    iinfo->const_arg[i] = NUM_VAL (cf_node);
                    FREE (type);
                    FREE (args[0]); /* *cf_node == *args[0] */
                    FREE (args[1]);
                } else
                    iinfo->const_arg[i] = val;
            }
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *WLIfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   we search in every function separately for withloops.
 *   The folding always happens exclusively in a function body.
 *
 *   The optimization traversal OPTTrav is included in WLI traversal to
 *   modify USE and DEF and to create MRD masks.
 *
 ******************************************************************************/

node *
WLIfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIfundef");

    INFO_WLI_WL (arg_info) = NULL;

    if (FUNDEF_BODY (arg_node))
        FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);
    FUNDEF_NEXT (arg_node) = OPTTrav (FUNDEF_NEXT (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIassign(node *arg_node, node *arg_info)
 *
 * description:
 *   needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIassign");

    INFO_WLI_ASSIGN (arg_info) = arg_node;
    if (INDEX (arg_node))
        FREE_INDEX (INDEX (arg_node)); /* this is important. Only index transformations
                                          with a non-null INDEX are valid. See WLIlet. */

    ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);
    if (INDEX (arg_node))
        DbugIndexInfo (INDEX (arg_node));
    ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIcond(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIcond");

    /* we do not need to traverse the condition
       COND_COND(arg_node) = OPTTrav(COND_COND(arg_node), arg_info, arg_node);
    */

    /* traverse bodies. MRDs are build locally in the bodies. The DEF mask is
       evaluated in the superior OPTTrav to modify the actual MRD list. */
    COND_THENINSTR (arg_node) = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
    COND_ELSEINSTR (arg_node) = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIdo(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIdo");

    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);
    DO_COND (arg_node) = OPTTrav (DO_COND (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIwhile(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIwhile");

    WHILE_COND (arg_node) = OPTTrav (WHILE_COND (arg_node), arg_info, arg_node);
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIwith(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIwith");

    WITH_GEN (arg_node) = OPTTrav (WITH_GEN (arg_node), arg_info, arg_node);
    switch (NODE_TYPE (WITH_OPERATOR (arg_node))) {
    case N_genarray:
        BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_modarray:
        BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldprf:
        BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldfun:
        BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    default:
        DBUG_ASSERT (0, "Operator not implemented for with_node");
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIid(node *arg_node, node *arg_info)
 *
 * description:
 *   counts all references to WLs
 *
 *
 ******************************************************************************/

node *
WLIid (node *arg_node, node *arg_info)
{
    node *mrdn;

    DBUG_ENTER ("WLIid");

    mrdn = MRD (ID_VARNO (arg_node));
    if (mrdn &&                                                        /* exists */
        (mrdn = ASSIGN_INSTR (mrdn)) && N_let == NODE_TYPE (mrdn) &&   /* is N_let */
        0 == strcmp (ID_NAME (arg_node), IDS_NAME (LET_IDS (mrdn))) && /* is right name */
        N_Nwith == NODE_TYPE (LET_EXPR (mrdn))) {                      /* is WL */

        NWITH_REFERENCED (LET_EXPR (mrdn))++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIlet(node *arg_node, node *arg_info)
 *
 * description:
 *   We are interested in prf applications. This may be transformations of
 *   later used index vectors for arrays to fold.
 *   If we find an F_psi, we check, based on the above made transformation
 *   checks, if the index vector is vald (i.e. the array reference is
 *   foldable).
 *
 ******************************************************************************/

node *
WLIlet (node *arg_node, node *arg_info)
{
    node *exprn;
    prf prf;
    index_info *iinfo;

    DBUG_ENTER ("WLIlet");

    /* if we are inside a WL we have to search for valid index transformations. */
    if (INFO_WLI_WL (arg_info)) {
        /* if this is a prf, we are interrested in transformations like +,*,-,/
           and in indexing (F_psi). */
        exprn = LET_EXPR (arg_node);
        if (N_prf == NODE_TYPE (exprn)) {
            prf = PRF_PRF (exprn);
            switch (prf) {
            /* this may ba an assignment which calculates an index for an
               array to fold. */
            case F_add:
            case F_sub:
            case F_mul:
            case F_div: /* both args are scalars */
                CreateIndexInfoSxS (exprn, arg_info);
                break;

            case F_add_SxA:
            case F_sub_SxA:
            case F_mul_SxA:
            case F_div_SxA:

            case F_add_AxS:
            case F_sub_AxS:
            case F_mul_AxS:
            case F_div_AxS:

            case F_add_AxA:
            case F_sub_AxA:
            case F_mul_AxA:
            case F_div_AxA:
                CreateIndexInfoA (exprn, arg_info);
                break;

            case F_psi:
                /* If we select the index vector itself with a constant we can
                   replace that with an index scalar. iv[1] -> k. */
                CheckOptimizePsi (&(LET_EXPR (arg_node)), arg_info);
                if (N_prf == NODE_TYPE (exprn)) { /* optimization not successful. */
                    /* check if index (1st argument) is valid. If yes, the array
                       could be folded.
                       We have 3 possibilities here:
                       - the array's Id (2nd argument) is not based on a foldable
                          withloop. So no index_info is created. Else...
                       - the first argument is the index vector from the generator.
                         Then no index_info for this transformation exists right now.
                         Create it.
                       - the first argument is a reference to a locally defined
                         valid (or invalid) transformation. If valid, duplicate it. */
                    if (N_id == NODE_TYPE (PRF_ARG1 (exprn))) {
                        iinfo = ValidLocalId (PRF_ARG1 (exprn));
                        if ((iinfo && iinfo->vector)
                            || -1
                                 == LocateIndexVar (PRF_ARG1 (exprn),
                                                    INFO_WLI_WL (arg_info)))
                            CreateIndexInfoId (PRF_ARG1 (exprn), arg_info);
                    }
                }
                break;

            default:
                break;
            }
        }

        /* can we transform this array to the index vector? */
        if (N_array == NODE_TYPE (exprn))
            CheckOptimizeArray (&(LET_EXPR (arg_node)), arg_info);

        /* The let expr may be an index scalar or the index vector, too. */
        if (N_id == NODE_TYPE (exprn))
            CreateIndexInfoId (exprn, arg_info);

        /* The let expr still may be a construction of a vector (without a prf). */
        if (N_array == NODE_TYPE (exprn)) {
            INDEX (INFO_WLI_ASSIGN (arg_info))
              = Scalar2ArrayIndex (exprn, INFO_WLI_WL (arg_info));
        }

    } /* is this a WL? */

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLINwith(node *arg_node, node *arg_info)
 *
 * description:
 *   start gathering information for this WL.
 *   First some initialisations in the WL structure are done, then the
 *   N_Nparts are traversed (which call the appropriate N_Ncode subtrees).
 *
 ******************************************************************************/

node *
WLINwith (node *arg_node, node *arg_info)
{
    node *tmpn;

    DBUG_ENTER ("WLINwith");

    /* inside the body of this WL we may find another WL. So we better
       save the old arg_info information. */
    tmpn = MakeInfo ();
    tmpn->mask[0] = INFO_DEF; /* DEF and USE information have */
    tmpn->mask[1] = INFO_USE; /* to be identical. */
    tmpn->varno = INFO_VARNO;

    INFO_WLI_NEXT (tmpn) = arg_info;
    arg_info = tmpn;

    /* initialize WL traversal */
    INFO_WLI_WL (arg_info) = arg_node; /* store the current node for later */
    tmpn = NWITH_CODE (arg_node);
    while (tmpn) { /* reset traversal flag for each code */
        NCODE_FLAG (tmpn) = 0;
        tmpn = NCODE_NEXT (tmpn);
    }
    NWITH_REFERENCED (arg_node) = 0;
    NWITH_REFERENCED_FOLD (arg_node) = 0;
    NWITH_COMPLEX (arg_node) = 0;
    NWITH_FOLDABLE (arg_node) = 1;

    /* traverse all parts (and implicitely bodies).
       It is not possible that WLINpart calls the NPART_NEXT node because
       the superior OPTTrav mechanism has to finisch before calling the
       next part. Else modified USE and DEF masks will case errors. */
    tmpn = NWITH_PART (arg_node);
    while (tmpn) {
        tmpn = OPTTrav (tmpn, arg_info, arg_node);
        tmpn = NPART_NEXT (tmpn);
    }

    /* restore arg_info */
    tmpn = arg_info;
    arg_info = INFO_WLI_NEXT (arg_info);
    INFO_DEF = tmpn->mask[0];
    INFO_USE = tmpn->mask[1];
    INFO_VARNO = tmpn->varno;
    FREE (tmpn);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLINpart(node *arg_node, node *arg_info)
 *
 * description:
 *   constant bounds, step and width vectors are substituted into the
 *   generator. Then the appropriate body is traversed.
 *
 *
 ******************************************************************************/

node *
WLINpart (node *arg_node, node *arg_info)
{
    node *tmpn, **bound;
    int i;

    DBUG_ENTER ("WLINpart");

    /* Can we replace any identifyer in the generator with a constant vector? */
    for (i = 1; i <= 4; i++) {
        switch (i) {
        case 1:
            bound = &NGEN_BOUND1 (NPART_GEN (arg_node));
            break;
        case 2:
            bound = &NGEN_BOUND2 (NPART_GEN (arg_node));
            break;
        case 3:
            bound = &NGEN_STEP (NPART_GEN (arg_node));
            break;
        case 4:
            bound = &NGEN_WIDTH (NPART_GEN (arg_node));
            break;
        }

        if (*bound && N_id == NODE_TYPE ((*bound))) {
            MRD_GETDATA (tmpn, ID_VARNO ((*bound)), INFO_VARNO);
            if (IsConstantArray (tmpn, N_num)) {
                /* this bound references a constant array, which can be substituted. */
                /* first adjust USE mask and kill node */
                NPART_MASK (arg_node, USE_MASK)[ID_VARNO ((*bound))]--;
                FreeTree (*bound);
                /* copy const array to *bound */
                *bound = DupTree (tmpn, NULL);
            } else
                /* variable bounds: this WL cannot be folded. */
                NWITH_FOLDABLE (INFO_WLI_WL (arg_info)) = 0;
        }
    }

    /* traverse code. But do this only once, even if there are more than
       one referencing generators.
       This is just a cross reference, so just traverse, do not assign the
       resulting node.*/
    if (!NCODE_FLAG (NPART_CODE (arg_node)))
        OPTTrav (NPART_CODE (arg_node), arg_info, INFO_WLI_WL (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLINcode(node *arg_node, node *arg_info)
 *
 * description:
 *   marks this N_Ncode node as processed and enters the code block.
 *
 *
 ******************************************************************************/

node *
WLINcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLINcode");

    DBUG_ASSERT (!NCODE_FLAG (arg_node), ("Body traversed a second time in WLI"));
    NCODE_FLAG (arg_node) = 1; /* this body has been traversed and all
                                  information has been gathered. */

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFNwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLFNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFNwith");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFWithloopFolding(node *arg_node, node* arg_info)
 *
 * description:
 *   starting point for the withloop folding.
 *
 *
 ******************************************************************************/

node *
WLFWithloopFolding (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFWithloopFolding");

    DBUG_ASSERT (!arg_info, ("at the beginning of WLF: arg_info != NULL"));
    arg_info = MakeInfo ();

    /* WLI traversal: search information */
    act_tab = wli_tab;
    Trav (arg_node, arg_info);

    /* WLF traversal: fold WLs */
    act_tab = wlf_tab;
    Trav (arg_node, arg_info);

    FREE (arg_info);

    DBUG_RETURN (arg_node);

    /* srs: remarks
       ============

       remember to free additional information in ASSIGN_INDEX

       */
}
