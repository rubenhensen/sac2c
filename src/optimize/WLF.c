/*    $Id$
 *
 * $Log$
 * Revision 1.4  1998/04/20 09:00:09  srs
 * new functions
 *
 * Revision 1.3  1998/04/08 20:33:53  srs
 * new WLF functions
 *
 * Revision 1.2  1998/04/07 16:50:06  srs
 * new WLF functions
 *
 * Revision 1.1  1998/03/22 18:21:46  srs
 * Initial revision
 *
 */

/*******************************************************************************

 This file realizes the withlop folding.

 *******************************************************************************

 Usage of arg_info:

 wlfm_search_WL  :
 wlfm_search_ref :
 - node[0]: NEXT   : store old information in nested WLs
 - node[1]: WL     : reference to base node of current WL (N_Nwith)
 - flag   : FLAG   : collects information in the wlfm_search phase if there is
                     at least one foldable reference in the WL.

 wlfm_replace    :
 wlfm_rename     :
 - node[0]: SUBST  : Pointer to subst N_Ncode/ pointer to copy of subst N_Ncode
 - node[1]: NEW_ID : pointer to N_id which shall replace the prf psi()
                     in the target Code.
 - node[4]: ID     : pointr to original N_id node in target WL to replace
                     by subst WL.
 - node[5]: NCA    : points to the assignment of the subst WL. Here new
                     assignments are inserted to define new variables.

 all wlfm phases :
 - node[2]: ASSIGN : always the last N_assign node
 - node[3]: FUNDEF : pointer to last fundef node. Needed to access vardecs

 - varno  : number of variables in this function, see optimize.c
 - mask[0]: DEF mask, see optimize.c
 - mask[1]: USE mask, see optimize.c

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
#include "WLF.h"

/******************************************************************************
 *
 *   typedefs
 *
 ******************************************************************************/

/* Several traverse functions of this file are traversed for different
   purposes. This enum determines ths function. */
typedef enum {
    wlfm_search_WL,  /* find new WLs */
    wlfm_search_ref, /* find references within WL to fold. */
    wlfm_rename,     /* traverse copied body of substWL and
                        rename variables to avoid name clashes. */
    wlfm_replace     /* traverse copied body of targetWL and
                        replace substituted reference. */
} wlf_mode_type;

typedef struct CODE_CONSTRUCTION {
    node *target;
    node *subst;
    node *new;
    struct CODE_CONSTRUCTION *next;
} code_constr_type;

typedef struct RENAMING {
    char *old;
    char *new;
    node *vardec; /* vardec for new */
    struct RENAMING *next;
} renaming_type;

/******************************************************************************
 *
 *   global variables
 *
 ******************************************************************************/

renaming_type *renaming; /* stores information for wlfm_rename. */
wlf_mode_type wlf_mode;
code_constr_type *code_constr; /* list of combinded code blocks */
node *new_codes;               /* list of created N_Ncode nodes */
node *ref_mode_arg_info;       /* saves arg_info for CreateCode(). Else
                                  we would have to pass arg_info through
                                  many functions (which costs time) */

intern_gen *new_ig;     /* new generators derived by a traversel
                           of one Ncode block. */
intern_gen *all_new_ig; /* new generators derived from all Ncode
                           nodes. */
intern_gen *orig_ig;    /* original generators of the target WL */

/* global vars to speed up function call of IntersectGrids(). They are only used
   to transfer information between IntersectGrids() and IntersectInternGen(). */
int *intersect_grids_ot; /* grid offsets used by IntersectGrids() */
int *intersect_grids_os;
intern_gen *intersect_grids_tig, *intersect_grids_sig, *intersect_grids_baseig;
intern_gen *intersect_intern_gen; /* resulting igs of IntersectInternGen. Global
                                     var to speed up function call.*/

/******************************************************************************
 *
 * function:
 *   renaming_type *AddRen(char *old_name, char *new_name, types *type, node *arg_info)
 *
 * description:
 *   Add new entry to renaiming list. No own mem for strings (old and new) is
 *   allocated. So the char pointers should point to string which will not be
 *   deleted soon (especially old_name: best point to vardec).
 *
 *   Additionally, this function inserts the assignment 'new_name = old_name'
 *   after the subst-WL. This can be switched by the parameter type. If
 *   type == NULL, no assignment is inserted. BUT in this case a vardec
 *   for new_name must exist.
 *
 ******************************************************************************/

renaming_type *
AddRen (char *old_name, char *new_name, types *type, node *arg_info)
{
    renaming_type *ren;
    node *assignn, *letn, *idn;
    ids *_ids;

    DBUG_ENTER ("AddRen");

    /* alloc new mem, search (or create) vardec for new_name and insert
       new element into renaming list. */
    ren = Malloc (sizeof (renaming_type));
    ren->old = old_name;
    ren->new = new_name;
    ren->vardec
      = CreateVardec (new_name, type, &FUNDEF_VARDEC (INFO_WLI_FUNDEF (arg_info)));

    ren->next = renaming;
    renaming = ren;

    /* assign old value to new variable */
    if (type) {
        idn = MakeId (StringCopy (old_name), NULL, ST_regular);
        ID_VARDEC (idn)
          = CreateVardec (old_name, NULL, &FUNDEF_VARDEC (INFO_WLI_FUNDEF (arg_info)));
        _ids = MakeIds (StringCopy (new_name), NULL, ST_regular);
        IDS_VARDEC (_ids) = ren->vardec;
        letn = MakeLet (idn, _ids);
        assignn = MakeAssign (letn, ASSIGN_INSTR (INFO_WLI_NCA (arg_info)));

        /* add node to syntax tree. See WLFassign for correct integration. */
        ASSIGN_NEXT (assignn) = ASSIGN_INSTR (INFO_WLI_NCA (arg_info));
        ASSIGN_INSTR (INFO_WLI_NCA (arg_info)) = assignn;
    }

    DBUG_RETURN (ren);
}

/******************************************************************************
 *
 * function:
 *   renaming_type *FreeRen(void)
 *
 * description:
 *   Free whole renaming list
 *
 *
 ******************************************************************************/

renaming_type *
FreeRen (void)
{
    renaming_type *ren;

    DBUG_ENTER ("FreeRen");

    while (renaming) {
        ren = renaming;
        renaming = renaming->next;
        FREE (ren);
    }

    DBUG_RETURN (renaming);
}

/******************************************************************************
 *
 * function:
 *   renaming_type *SearchRen(char *old_name)
 *
 * description:
 *   Search for the given old_name in the renaming list and return pointer
 *   to entry if found.
 *
 ******************************************************************************/

renaming_type *
SearchRen (char *old_name)
{
    renaming_type *ren;

    DBUG_ENTER ("SearchRen");

    ren = renaming;
    while (ren && strcmp (old_name, ren->old))
        ren = ren->next;

    DBUG_RETURN (ren);
}

/******************************************************************************
 *
 * function:
 *   void AddCC(node *targetn, node *substn, node *resultn)
 *
 * description:
 *   adds entry to the global code_constr list.
 *
 *
 ******************************************************************************/

void
AddCC (node *targetn, node *substn, node *resultn)
{
    code_constr_type *cc;

    DBUG_ENTER ("AddCC");

    cc = Malloc (sizeof (code_constr_type));
    cc->target = targetn;
    cc->subst = substn;
    cc->new = resultn;
    cc->next = code_constr;
    code_constr = cc;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   code_constr_type *SearchCC(node *targetn, node *substn)
 *
 * description:
 *   Searches for an existing entry of targetn and substn and returns it
 *   or NULL if not found
 *
 ******************************************************************************/

code_constr_type *
SearchCC (node *targetn, node *substn)
{
    code_constr_type *cc;

    DBUG_ENTER ("SearchCC");

    cc = code_constr;
    while (cc && (cc->target != targetn || cc->subst != substn))
        cc = cc->next;

    DBUG_RETURN (cc);
}

/******************************************************************************
 *
 * function:
 *   void FreeCC(code_constr_type *cc)
 *
 * description:
 *   sets whole cc list free
 *
 *
 ******************************************************************************/

void
FreeCC (code_constr_type *cc)
{
    code_constr_type *tmpcc;

    DBUG_ENTER ("FreeCC");

    while (cc) {
        tmpcc = cc;
        cc = cc->next;
        FREE (tmpcc);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   int KgV(int a, int b)
 *
 * description:
 *   calculates the smalest common multiple
 *
 *
 ******************************************************************************/

int
KgV (int a, int b)
{
    int c, m;

    DBUG_ENTER ("KgV");

    if (b > a) {
        c = b;
        b = a;
        a = c;
    }

    m = a * b;

    while (b) { /* euklid */
        c = a % b;
        a = b;
        b = c;
    }

    DBUG_RETURN (m / a);
}

/******************************************************************************
 *
 * function:
 *   node *CreateCode(node *target, node *subst)
 *
 * description:
 *   substitutes the code block subst into target, introduces new vardecs
 *   and assignments, returns a new N_Ncode node. Most of the work in done
 *   in the usual traversal steps (with mode wlfm_replace and wlfm_remane).
 *
 ******************************************************************************/

node *
CreateCode (node *target, node *subst)
{
    node *coden, *new_arg_info;

    DBUG_ENTER ("CreateCode");
    DBUG_ASSERT (N_Ncode == NODE_TYPE (target), ("wrong Parameter"));
    DBUG_ASSERT (N_Ncode == NODE_TYPE (subst), ("wrong Parameter"));

    wlf_mode = wlfm_replace;

    /* create new arg_info to avoid modifications of the old one and copy
       all relevant information */
    new_arg_info = MakeInfo ();
    INFO_WLI_FUNDEF (new_arg_info) = INFO_WLI_FUNDEF (ref_mode_arg_info);
    INFO_WLI_ID (new_arg_info) = INFO_WLI_ID (ref_mode_arg_info);
    INFO_WLI_NCA (new_arg_info) = INFO_WLI_NCA (ref_mode_arg_info);
    INFO_WLI_SUBST (new_arg_info) = subst;
    INFO_WLI_NEW_ID (new_arg_info) = NULL;

    coden = DupTree (NCODE_CBLOCK (target), NULL);
    coden = Trav (coden, new_arg_info);
    coden = MakeNCode (coden, DupTree (NCODE_CEXPR (target), NULL));

    FreeNode (new_arg_info);

    wlf_mode = wlfm_search_ref;

    DBUG_RETURN (coden);
}

/******************************************************************************
 *
 * function:
 *   void IntersectGrids(int dim, intern_gen *ig)
 *
 * description:
 *   recursive function to intersect two grids. Information are made available
 *   in global variables intersect_grids* to speed up recursive function calls.
 *
 ******************************************************************************/

void
IntersectGrids (int dim)
{
    int dc, first, last, d;
    intern_gen *ig;
    code_constr_type *cc;
    node *coden;

    DBUG_ENTER ("IntersectGrids");

    dc = 0;

    while (dc < intersect_grids_baseig->step[dim]) {
        /* search common dc */
        if ((!intersect_grids_tig->step
             || (dc + intersect_grids_ot[dim]) % intersect_grids_tig->step[dim]
                  < intersect_grids_tig->width[dim])
            && (!intersect_grids_sig->step
                || (dc + intersect_grids_os[dim]) % intersect_grids_sig->step[dim]
                     < intersect_grids_sig->width[dim])) {
            first = dc;
            /* search first dc where either in tig or sig the element is
               not present. */
            do {
                dc++;
            } while (
              (!intersect_grids_tig->step
               || (dc + intersect_grids_ot[dim]) % intersect_grids_tig->step[dim]
                    < intersect_grids_tig->width[dim])
              && (!intersect_grids_sig->step
                  || (dc + intersect_grids_os[dim]) % intersect_grids_sig->step[dim]
                       < intersect_grids_sig->width[dim])
              && dc < intersect_grids_baseig->step[dim]);
            last = dc;

            if (dim < intersect_grids_baseig->shape - 1) {
                /* process inner dimensions */
                intersect_grids_baseig->l[dim] += first;
                intersect_grids_baseig->width[dim] = last - first;
                IntersectGrids (dim + 1);
                intersect_grids_baseig->l[dim] -= first; /* restore old value */
            } else {
                /* create new ig and add it to structures */
                ig = CreateInternGen (intersect_grids_baseig->shape, 1);
                for (d = 0; d < intersect_grids_baseig->shape; d++) {
                    ig->l[d] = intersect_grids_baseig->l[d];
                    ig->u[d] = intersect_grids_baseig->u[d];
                    ig->step[d] = intersect_grids_baseig->step[d];
                    ig->width[d] = intersect_grids_baseig->width[d];
                }
                ig->l[d - 1] += first;
                ig->width[d - 1] = last - first;

                /* add craetes code to code_constr list and to new_codes. */
                cc = SearchCC (intersect_grids_tig->code, intersect_grids_sig->code);
                if (cc)
                    ig->code = cc->new;
                else {
                    coden
                      = CreateCode (intersect_grids_tig->code, intersect_grids_sig->code);
                    ig->code = coden;
                    AddCC (intersect_grids_tig->code, intersect_grids_sig->code, coden);
                    NCODE_NEXT (coden) = new_codes;
                    new_codes = coden;
                }

                /*         printf("--------------------\n"); */
                /*         DbugInternGen(intersect_grids_tig); */
                /*         printf("\n"); */
                /*         DbugInternGen(intersect_grids_sig); */
                /*         printf("\n"); */
                /*         DbugInternGen(ig); */

                /* add new generator to intersect_intern_gen list. */
                ig->next = intersect_intern_gen;
                intersect_intern_gen = ig;
            }
        }

        dc++;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   intern_gen *IntersectInternGen(intern_gen *tig, intern_gen *sig)
 *
 * description:
 *   Intersects the generators of the currently processed code (tig) and all
 *   generators of the subst WL (sig). Appends new codes to code_constr.
 *   Returns intern_gen list of new intersections.
 *
 ******************************************************************************/

intern_gen *
IntersectInternGen (intern_gen *target_ig, intern_gen *subst_ig)
{
    intern_gen *sig, *new_gen;
    int d, max_dim, l, u, create_steps;
    code_constr_type *cc;
    node *coden;

    DBUG_ENTER ("IntersectInternGen");
    DBUG_ASSERT (target_ig->shape == subst_ig->shape, ("wrong parameters"));

    max_dim = target_ig->shape;
    create_steps = target_ig->step || subst_ig->step;
    new_gen = CreateInternGen (target_ig->shape, create_steps);
    intersect_intern_gen = NULL;

    while (target_ig) {
        sig = subst_ig;
        while (sig) {
            /* here we have to intersect target_ig and sig */
            /* first the bounds, ignore step/width */
            for (d = 0; d < max_dim; d++) {
                l = MAX (target_ig->l[d], sig->l[d]);
                u = MIN (target_ig->u[d], sig->u[d]);
                if (l >= u)
                    break; /* empty intersection */
                else {
                    new_gen->l[d] = l;
                    new_gen->u[d] = u;
                }
            }

            if (d == max_dim) {
                /* we have a new ig. Maybe. Because if both generators are
                   grids they may still be disjuct. */
                if (create_steps) {
                    /* prepare parameters for recursive function IntersectGrids().
                       Global pointers are used to speed up function call. */
                    /* new_gen will be the reference structure for new igs
                       created within IntersectGrids(). */
                    if (!target_ig->step)
                        for (d = 0; d < max_dim; d++)
                            new_gen->step[d] = sig->step[d];
                    else if (!sig->step)
                        for (d = 0; d < max_dim; d++)
                            new_gen->step[d] = target_ig->step[d];
                    else
                        for (d = 0; d < max_dim; d++)
                            new_gen->step[d] = KgV (target_ig->step[d], sig->step[d]);

                    /* compute offsets */
                    if (target_ig->step)
                        for (d = 0; d < max_dim; d++)
                            intersect_grids_ot[d]
                              = (new_gen->l[d] - target_ig->l[d]) % target_ig->step[d];
                    if (sig->step)
                        for (d = 0; d < max_dim; d++)
                            intersect_grids_os[d]
                              = (new_gen->l[d] - sig->l[d]) % sig->step[d];

                    intersect_grids_baseig = new_gen;
                    intersect_grids_tig = target_ig;
                    intersect_grids_sig = sig;

                    IntersectGrids (0);
                    /* the mem of new_gen can be used for the next intersection.
                       Don't free it. */
                } else {
                    /* craete new code and add it to code_constr list and to new_codes. */
                    cc = SearchCC (target_ig->code, sig->code);
                    if (cc)
                        new_gen->code = cc->new;
                    else {
                        coden = CreateCode (target_ig->code, sig->code);
                        new_gen->code = coden;
                        AddCC (target_ig->code, sig->code, coden);
                        NCODE_NEXT (coden) = new_codes;
                        new_codes = coden;
                    }

                    /* add new generator to intersect_intern_gen list. */
                    new_gen->next = intersect_intern_gen;
                    intersect_intern_gen = new_gen;

                    /* allocate new mem for new_gen */
                    new_gen = CreateInternGen (target_ig->shape, create_steps);
                }
            }

            sig = sig->next; /* go on with next subst ig */
        }
        target_ig = target_ig->next; /* go on with next target ig */
    }

    FREE_INTERN_GEN (new_gen);

    DBUG_RETURN (intersect_intern_gen);
}

/******************************************************************************
 *
 * Function:
 *   void Fold(node *idn, node *targetwln)
 *
 * description:
 *   The Id idn in the current (arg_info) WL shall be folded.
 *
 *
 ******************************************************************************/

void
Fold (node *idn, node *targetwln)
{
    intern_gen *target_ig; /* transformed igs of target WL */
    intern_gen *subst_ig;  /* transformed igs of subst WL */
    intern_gen *ig, *tmpig, *intersect_ig;
    node *substwln;

    DBUG_ENTER ("Fold");

    substwln = LET_EXPR (ASSIGN_INSTR (ID_WL (idn)));

    code_constr = NULL;

    /* transformations */
    target_ig = NULL;
    ig = orig_ig; /* create target_ig */
    while (ig) {
        tmpig = CopyInternGen (ig);
        if (target_ig) {
            tmpig->next = target_ig;
            target_ig = tmpig;
        } else
            target_ig = tmpig;
        ig = ig->next;
    }

    subst_ig = Tree2InternGen (substwln, NULL);
    intersect_intern_gen = NULL;

    /* intersect target_ig and subst_ig
       and create new code blocks */
    intersect_ig = IntersectInternGen (target_ig, subst_ig);

    /* results are in intersect_ig. At the moment, just append to new_ig. */
    tmpig = new_ig;
    if (!tmpig)
        new_ig = intersect_ig;
    else {
        while (tmpig->next)
            tmpig = tmpig->next;
        tmpig->next = intersect_ig;
    }

    /* free all created lists */
    FreeInternGenChain (target_ig);
    FreeInternGenChain (subst_ig);
    FreeCC (code_constr);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   int FoldDecision(node *target_wl, node* subst_wl)
 *
 * description:
 *   decides whether subst_wl shall be folded into target_wl or not.
 *
 *
 ******************************************************************************/

int
FoldDecision (node *target_wl, node *subst_wl)
{
    int result;

    DBUG_ENTER ("FoldDecision");

    subst_wl = LET_EXPR (ASSIGN_INSTR (subst_wl));

    result
      = (NWITH_PARTS (target_wl) > 0 && NWITH_PARTS (subst_wl) > 0
         && NWITH_REFERENCED (subst_wl) == NWITH_REFERENCED_FOLD (subst_wl)
         && (WO_genarray != NWITH_TYPE (subst_wl) || WO_modarray != NWITH_TYPE (subst_wl))
         && !NWITH_NO_CHANCE (subst_wl));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *CheckForSuperfuousCodes(node *wln)
 *
 * description:
 *   remove all unused N_Ncode nodes of the given WL.
 *
 *
 ******************************************************************************/

node *
CheckForSuperfuousCodes (node *wln)
{
    node **tmp;

    DBUG_ENTER ("CheckForSuperfuousCodes");

    tmp = &NWITH_CODE (wln);
    while (*tmp)
        if (!NCODE_USED ((*tmp)))
            *tmp = FreeNode (*tmp);
        else
            tmp = &NCODE_NEXT ((*tmp));

    DBUG_RETURN (wln);
}

/******************************************************************************
 *
 * function:
 *   node *Modarray2Genarray(node *wln)
 *
 * description:
 *   transforms the withop of the given WL into genarray. This can always be
 *   done for WL which have a full partition.
 *
 ******************************************************************************/

node *
Modarray2Genarray (node *wln)
{
    node *shape, *eltn;
    types *type;
    int dimensions, i;

    DBUG_ENTER ("Modarray2Genarray");
    DBUG_ASSERT (WO_modarray == NWITH_TYPE (wln), ("wrong withop for Modarray2Genarray"));

    /* compute shape of WL for NWITHOP_SHAPE() */
    type = ID_TYPE (NWITHOP_ARRAY (NWITH_WITHOP (wln)));
    dimensions = IDS_SHAPE (NPART_VEC (NWITH_PART (wln)), 0);

    eltn = NULL;
    for (i = 0; i < dimensions; i++)
        eltn = MakeExprs (MakeNum (TYPES_SHAPE (type, i)), eltn);

    shape = MakeArray (eltn);

    /* delete old withop and create new one */
    FreeTree (NWITH_WITHOP (wln));
    NWITH_WITHOP (wln) = MakeNWithOp (WO_genarray);
    NWITHOP_SHAPE (NWITH_WITHOP (wln)) = shape;

    DBUG_RETURN (wln);
}

/******************************************************************************
 *
 * function:
 *   node *WLFfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   initializations to arg_info.
 *
 *
 ******************************************************************************/

node *
WLFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFfundef");

    INFO_WLI_WL (arg_info) = NULL;
    INFO_WLI_FUNDEF (arg_info) = arg_node;
    wlf_mode = wlfm_search_WL;

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFassign(node *arg_node, node *arg_info)
 *
 * description:
 *   set arg_info to remember this N_assign node.
 *
 *
 ******************************************************************************/

node *
WLFassign (node *arg_node, node *arg_info)
{
    node *tmpn, *last_assign, *substn;

    DBUG_ENTER ("WLFassign");

    INFO_WLI_ASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    switch (wlf_mode) {
    case wlfm_replace:
        if (INFO_WLI_NEW_ID (arg_info)) {
            /* paste the new code (subst WL code) into the assign chain (target WL
               code). The current assign node is appended to the new INFO_WLI_SUBST
               chain. Therefor, reuse the current assign node because the
               previous assign node (which annot be reached from here) points
               to is. */
            last_assign = MakeAssign (ASSIGN_INSTR (arg_node), ASSIGN_NEXT (arg_node));
            substn = INFO_WLI_SUBST (arg_info);

            ASSIGN_INSTR (arg_node) = ASSIGN_INSTR (substn);
            ASSIGN_NEXT (arg_node) = ASSIGN_NEXT (substn);

            ASSIGN_INSTR (substn) = NULL;
            FreeNode (substn);

            tmpn = arg_node;
            while (ASSIGN_NEXT (tmpn))
                tmpn = ASSIGN_NEXT (tmpn);
            ASSIGN_NEXT (tmpn) = last_assign;

            /* transform psi(.,array) into Id. */
            tmpn = ASSIGN_INSTR (ASSIGN_NEXT (tmpn));
            LET_EXPR (tmpn) = FreeTree (LET_EXPR (tmpn));
            LET_EXPR (tmpn) = INFO_WLI_NEW_ID (arg_info);
        } else {
            if (ASSIGN_NEXT (arg_node))
                ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

            /* Now we returned from processing the following assign nodes.
               If new code has been inserted, we find N_assign nodes at the
               ASSIGN_INSTR sons. These have to be inserted correctly into the
               assign chain. */
            if (N_assign == NODE_TYPE (ASSIGN_INSTR (arg_node))) {
                last_assign = ASSIGN_NEXT (arg_node);
                ASSIGN_NEXT (arg_node) = ASSIGN_INSTR (arg_node);
                tmpn = arg_node;
                while (N_assign == NODE_TYPE (ASSIGN_NEXT (tmpn)))
                    tmpn = ASSIGN_NEXT (tmpn);
                ASSIGN_INSTR (arg_node) = ASSIGN_NEXT (tmpn);
                ASSIGN_NEXT (tmpn) = last_assign;
            }
        }

        break;

    default:
        if (ASSIGN_NEXT (arg_node))
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFid(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
WLFid (node *arg_node, node *arg_info)
{
    node *substn, *vectorn, *argsn, *letn, *partn, *old_arg_info_assign;
    ids *subst_wl_ids, *_ids;
    int count;

    DBUG_ENTER ("WLFid");

    switch (wlf_mode) {
    case wlfm_search_WL:
        /* check if we want to fold this Id.
           If no, clear ID_WL(). This is the sign for the following phases to fold.
           If yes, set INFO_WLI_FLAG. */
        if (ID_WL (arg_node))             /* a WL is referenced (this is an array Id), */
            if (INFO_WLI_WL (arg_info) && /* inside WL, */
                INDEX (INFO_WLI_ASSIGN (arg_info)) && /* valid index transformation */
                FoldDecision (INFO_WLI_WL (arg_info), ID_WL (arg_node)))
                INFO_WLI_FLAG (arg_info) = 1;
            else
                ID_WL (arg_node) = NULL;

        break;

    case wlfm_search_ref:
        /* conditional of N_do, N_while or N_cond. Do nothing. */
        break;

    case wlfm_replace:
        if (ID_WL (arg_node) == INFO_WLI_ID (arg_info)) {
            /* this is the Id which has to be replaced. Create substitution code. */
            substn = INFO_WLI_SUBST (arg_info);
            INFO_WLI_NEW_ID (arg_info)
              = DupTree (NCODE_CEXPR (substn), NULL); /* usage: see WLFassign */
            substn = DupTree (BLOCK_INSTR (NCODE_CBLOCK (substn)), NULL);

            /* trav subst code with wlfm_rename to avoid name clashes. */
            wlf_mode = wlfm_rename;
            old_arg_info_assign = INFO_WLI_ASSIGN (arg_info); /* save arg_info */
            substn = Trav (substn, arg_info);
            INFO_WLI_ASSIGN (arg_info) = old_arg_info_assign;
            wlf_mode = wlfm_replace;

            /* add assignments to rename variables which index the array we want
               to replace.
               Example: We want to replace psi(sel,A) and A is a WL with a
               generator iv=[i,j,k]. Then we add:
                 i = psi([0],sel);
                 j = psi([1],sel);
                 k = psi([2],sel);
                 iv = sel;
               Remember that iv,i,j,k all are temporary variables, inserted
               in flatten. No name clashes can happen. */
            vectorn = PRF_ARG1 (LET_EXPR (ASSIGN_INSTR (INFO_WLI_ASSIGN (arg_info))));
            partn = NWITH_PART (LET_EXPR (ASSIGN_INSTR (ID_WL (INFO_WLI_ID (arg_info)))));
            subst_wl_ids = NPART_IDS (partn);
            count = 0;
            while (subst_wl_ids) {
                argsn = MakeExprs (MakeArray (MakeExprs (MakeNum (count++), NULL)),
                                   MakeExprs (DupTree (vectorn, NULL), NULL));
                _ids = MakeIds (IDS_NAME (subst_wl_ids), NULL, ST_regular);
                IDS_VARDEC (_ids) = IDS_VARDEC (subst_wl_ids);
                letn = MakeLet (MakePrf (F_psi, argsn), _ids);
                substn = MakeAssign (letn, substn);

                subst_wl_ids = IDS_NEXT (subst_wl_ids);
            }
            /* add iv = sel (see example above) */
            _ids = MakeIds (IDS_NAME (NPART_VEC (partn)), NULL, ST_regular);
            IDS_VARDEC (_ids) = IDS_VARDEC (NPART_VEC (partn));
            letn = MakeLet (DupTree (vectorn, NULL), _ids);
            substn = MakeAssign (letn, substn);

            /* we dont' need the old _SUBST info anymore so we return the
               new subst assign chain here. WLFassign uses this information to
               melt both chains. */
            INFO_WLI_SUBST (arg_info) = substn;
        }
        break;

    case wlfm_rename:

        break;

    default:
        DBUG_ASSERT (0, ("Not expected"));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFlet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLFlet (node *arg_node, node *arg_info)
{
    node *prfn, *idn;

    DBUG_ENTER ("WLFlet");

    switch (wlf_mode) {
    case wlfm_search_WL:
    case wlfm_replace:
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        break;

    case wlfm_search_ref:
        /* is this a prf psi() which can be folded? */
        prfn = LET_EXPR (arg_node);
        if (N_prf == NODE_TYPE (prfn) && F_psi == PRF_PRF (prfn)
            && ID_WL (PRF_ARG2 (prfn))) { /* second arg of psi() references a WL. */

            idn = PRF_ARG2 (prfn);

            /* These assignments are only done to transfer information
               to CreateCode (wlfm_replace) */
            INFO_WLI_ID (arg_info) = idn;
            INFO_WLI_NCA (arg_info) = ID_WL (idn);

            ref_mode_arg_info = arg_info; /* needed in CreateCode() */
            Fold (idn, INFO_WLI_WL (arg_info));
            wlf_expr++;

            /* unused here */
            INFO_WLI_ID (arg_info) = NULL;
            INFO_WLI_NCA (arg_info) = NULL;
        }

        break;

    case wlfm_rename:
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        break;

    default:
        DBUG_ASSERT (0, ("Not expected"));
    }

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
    node *tmpn;

    DBUG_ENTER ("WLFNwith");

    switch (wlf_mode) {
    case wlfm_search_WL:
        /* inside the body of this WL we may find another WL. So we better
         save the old arg_info information. */
        tmpn = MakeInfo ();
        tmpn->mask[0] = INFO_DEF; /* DEF and USE information have */
        tmpn->mask[1] = INFO_USE; /* to be identical. */
        tmpn->varno = INFO_VARNO;
        INFO_WLI_FUNDEF (tmpn) = INFO_WLI_FUNDEF (arg_info);
        INFO_WLI_ASSIGN (tmpn) = INFO_WLI_ASSIGN (arg_info);
        INFO_WLI_NEXT (tmpn) = arg_info;
        arg_info = tmpn;

        INFO_WLI_WL (arg_info) = arg_node; /* store the current node for later */

        /* It's faster to
           1. traverse into bodies to find WL within and fold them first
           2. and then try to fold references to other WLs.
           */
        INFO_WLI_FLAG (arg_info) = 0;
        arg_node = TravSons (arg_node, arg_info);

        if (INFO_WLI_FLAG (arg_info)) {
            /* traverse bodies of this WL again and fold now.
               Do not traverse WithOp (Id would confuse us). */
            wlf_mode = wlfm_search_ref; /* we do not need to save arg_info here */

            all_new_ig = NULL;
            new_codes = NULL;

            intersect_grids_ot
              = Malloc (sizeof (int) * IDS_SHAPE (NPART_VEC (NWITH_PART (arg_node)), 0));
            intersect_grids_os
              = Malloc (sizeof (int) * IDS_SHAPE (NPART_VEC (NWITH_PART (arg_node)), 0));

            NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

            FREE (intersect_grids_ot);
            FREE (intersect_grids_os);

            /* all codes have been traversed. Now append new_codes to WL and
             exchange old generators with all_new_ig. */
            if (new_codes) {
                /* We have new codes. This means at least one folding action has been
                   done and we have to replace the N_Nparts, too. */
                tmpn = NWITH_CODE (arg_node);
                while (NCODE_NEXT (tmpn))
                    tmpn = NCODE_NEXT (tmpn);
                NCODE_NEXT (tmpn) = new_codes;

                arg_node = InternGen2Tree (arg_node, all_new_ig);
                all_new_ig = FreeInternGenChain (all_new_ig);
                arg_node = CheckForSuperfuousCodes (arg_node);

                /* After successful folding every modarray-WL has to be transformed
                   into a genarray-WL. DCR has no chance to remove the subst WL with
                   references from N_Nwithop nodes to it. */
                if (WO_modarray == NWITH_TYPE (arg_node))
                    arg_node = Modarray2Genarray (arg_node);
            }

            /* this WL is finisched. Search other WLs on same level. */
            wlf_mode = wlfm_search_WL;
        }

        /* restore arg_info */
        tmpn = arg_info;
        arg_info = INFO_WLI_NEXT (arg_info);
        INFO_DEF = tmpn->mask[0];
        INFO_USE = tmpn->mask[1];
        FREE (tmpn);
        break;

    case wlfm_replace:
        /* Don't walk further into WL because we know that we do not
           find the Id we search for there. */
        break;

    case wlfm_rename:
        /* No renaming in generator or withop necessary. */
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
        break;

    default:
        DBUG_ASSERT (0, ("Not expected"));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFNcode(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLFNcode (node *arg_node, node *arg_info)
{
    intern_gen *ig;

    DBUG_ENTER ("WLFNcode");
    DBUG_ASSERT (0 != NCODE_USED (arg_node), ("traversing unusd code"));

    switch (wlf_mode) {
    case wlfm_search_WL:
    case wlfm_rename:
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
        if (NCODE_NEXT (arg_node))
            NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
        break;

    case wlfm_search_ref:
        /* create generator list orig_ig. Copy all generators of the target-WL
           to this list which point to the current N_Ncode node. Don't use
           the traversal mechanism because it's slow. */
        orig_ig = Tree2InternGen (INFO_WLI_WL (arg_info), arg_node);
        new_ig = NULL;

        /* traverse Code, create new_ig, fold. */
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

        /* Have we done at least one folding operation in this code block? If
           not, the new_ig list is empty and we have to keep the original parts. */
        if (!new_ig) {
            new_ig = orig_ig;
            orig_ig = NULL;
        }

        /* copy new generators to all_new_ig and clear new_ig. */
        if (!all_new_ig)
            all_new_ig = new_ig;
        else {
            ig = all_new_ig;
            while (ig->next)
                ig = ig->next;
            ig->next = new_ig;
        }
        new_ig = NULL;

        FreeInternGenChain (orig_ig);

        /* traverse next code block. */
        if (NCODE_NEXT (arg_node))
            NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
        break;

    default:
        DBUG_ASSERT (0, ("Not expected"));
    }

    DBUG_RETURN (arg_node);
}
