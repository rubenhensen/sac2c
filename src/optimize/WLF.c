/*    $Id$
 *
 * $Log$
 * Revision 1.25  1999/02/15 11:14:17  srs
 * replaced N_empty with prg genarray in WLFassign()
 *
 * Revision 1.24  1999/02/12 13:48:33  srs
 * fixed bug in WLFassign()
 *
 * Revision 1.23  1999/02/10 10:02:57  srs
 * Unreferenced WLs are replaced instantly with an N_empty node.
 * (see version 1.20)
 *
 * Revision 1.22  1999/02/02 19:30:02  srs
 * reverted changes from version 1.20
 *
 * Revision 1.21  1999/01/26 15:22:02  srs
 * modified last change of removing unreferenced WLs.
 *
 * Revision 1.20  1999/01/20 09:08:39  srs
 * in WLFAssign: WLs which are not referenced anymore are removed.
 *
 * Revision 1.19  1999/01/18 15:01:34  srs
 * fixed typo
 *
 * Revision 1.18  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.17  1998/12/02 16:41:13  cg
 * changed #include "limits.h" to #include <limits.h>
 *
 * Revision 1.16  1998/11/18 15:06:48  srs
 * N_empty nodes are supported now
 *
 * Revision 1.15  1998/08/20 12:18:20  srs
 * added comments
 * fixed bug in WLFid()
 *
 * Revision 1.14  1998/08/06 18:34:51  srs
 * bug fix:
 * index vector variable and index scalar variables are now renamed if
 * necessary. Loop unrolling and inlining can lead to same names in
 * different WLs.
 *
 * Revision 1.13  1998/07/14 14:27:19  srs
 * modarray->modarray transformation from version 1.11 reverted
 *
 * Revision 1.12  1998/07/14 12:57:41  srs
 * fixed bug in AddRen()
 *
 * Revision 1.11  1998/07/13 14:04:39  srs
 * fixed a bug in Modarray2Genarray.
 * Now a combination of two modarray-operators is not transformed into
 * a genarray anymore.
 *
 * Revision 1.10  1998/05/16 16:25:25  srs
 * fixed bug in WLFid
 *
 * Revision 1.9  1998/05/15 14:48:47  srs
 * several changed/fixes
 *
 * Revision 1.8  1998/05/07 16:12:39  srs
 * added more transformation functions,
 * fixed a bug in IntersectGrids(),
 * added range check for transformed constant generator bounds
 * fixed bug in WLFNwith()
 *
 * Revision 1.7  1998/04/29 15:17:43  srs
 * WLF and linear transformations (without grids) are working now
 *
 * Revision 1.6  1998/04/24 18:57:48  srs
 * added creation of types for N_array node
 *
 * Revision 1.5  1998/04/24 17:27:27  srs
 * added linear transformations
 *
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
 - node[4]: ID     : pointer to original N_id node in target WL to replace
                     by subst WL.
 - node[5]: NCA    : points to the assignment of the subst WL. Here new
                     assignments are inserted to define new variables.

 all wlfm phases :
 - node[2]: ASSIGN : always the last N_assign node
 - node[3]: FUNDEF : pointer to last fundef node. Needed to access vardecs

 - varno  : number of variables in this function, see optimize.c
 - mask[0]: DEF mask, see optimize.c
 - mask[1]: USE mask, see optimize.c

 ******************************************************************************

 Usage of ID_WL:

 ID_WL is used in 2 ways:
 (1) in WLI phase we store a pointer to (assign node of) a WL in ID_WL if
     the Id references a new WL. This pointer is used in WLF.
 (2) When the decision to fold the Id has been made it is necessary to
     create new code blocks. In this new code blocks ID_WL is used to
     point to it's original Id. So we have to distinguish between the Ids
     which are in the original code (1) and those in newly created code
     blocks (2).
     Creation of new code is initiated in CreateCode() and the resulting N_Ncode
     nodes are collected in new_codes until they are inserted into the syntax
     tree in WLFNwith(). While a code is in new_codes, ID_WL of every Id inside
     the code points to it's original Id (this is the Id which was copied by
     DupTree()). DupTree() sets this pointer and, if the argument of DupTree()
     is a code inside new_codes, copies ID_WL of this code (which is a
     pointer to the original).

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

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
#include "generatemasks.h"
#include "ConstantFolding.h"
#include "tree_compound.h"
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
    /* wlfm_search_WL and wlm_search_ref both traverse the original target code
       block. The first phase searches for other WLs inside the current WL
       body. The most cascated WLs are folded first. The second phase again
       traverses the target code and, if a foldable reference is found, initiates
       folding.  wlfm_replace traverses the copied target code block and tries to
       find the same point (Id which should be folded) here. Then the subst code
       is inserted at this point.  wlfm_rename traverses the subst code and renames
       variables which would lead to name clashes. */
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

/* global vars to speed up function call of IntersectGrids(). They are only used
   to transfer information between IntersectGrids() and IntersectInternGen(). */
int *intersect_grids_ot; /* grid offsets used by IntersectGrids() */
int *intersect_grids_os;
intern_gen *intersect_grids_tig, *intersect_grids_sig, *intersect_grids_baseig;
intern_gen *intersect_intern_gen; /* resulting igs of IntersectInternGen. Global
                                     var to speed up function call.*/

long *target_mask; /* here we store the mrd of the psi function
                      which shall be replaced */
long *subst_mask;  /* mrd mask of subst WL. */

/******************************************************************************
 *
 * function:
 *   renaming_type *AddRen(node *old_name, char *new_name, types *type, node *arg_info)
 *
 * description:
 *   Add new entry to renaiming list. No own mem for strings (old and new) is
 *   allocated. So the char pointers should point to string which will not be
 *   deleted soon.
 *
 *   Additionally, if var insert is != 0, this function inserts the assignment
 *   'new_name = old_name' after! the subst-WL. The N_assign node of the subst
 *   WL has to be untouched because cross pointers to this node exist and
 *   the wlf-traversal expects a WL there.
 *
 *  If new_name does not exist, a vardec is installed (therefor type and
 *   arg_info are necessary).
 *
 *  Attention:
 *    old_name is a vardec node. VARDEC_NAME(old_name) is the real name.
 *
 ******************************************************************************/

renaming_type *
AddRen (node *old_name, char *new_name, types *type, node *arg_info, int insert)
{
    renaming_type *ren;
    node *assignn, *letn, *idn;
    ids *_ids;

    DBUG_ENTER ("AddRen");

    /* alloc new mem, search (or create) vardec for new_name and insert
       new element into renaming list. */
    ren = Malloc (sizeof (renaming_type));
    ren->old = VARDEC_NAME (old_name);
    ren->new = new_name;
    ren->vardec
      = CreateVardec (new_name, type, &FUNDEF_VARDEC (INFO_WLI_FUNDEF (arg_info)));

    ren->next = renaming;
    renaming = ren;

    /* assign old value to new variable */
    if (insert) {
        idn = MakeId (StringCopy (VARDEC_NAME (old_name)), NULL, ST_regular);
        ID_VARDEC (idn) = old_name; /* the vardec node */
        _ids = MakeIds (StringCopy (new_name), NULL, ST_regular);
        IDS_VARDEC (_ids) = ren->vardec;
        letn = MakeLet (idn, _ids);
        assignn = MakeAssign (letn, ASSIGN_NEXT (INFO_WLI_NCA (arg_info)));
        ASSIGN_NEXT (INFO_WLI_NCA (arg_info)) = assignn;
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
 *   void DbugRen()
 *
 * description:
 *   prints whole renaming list
 *
 *
 ******************************************************************************/

void
DbugRen (void)
{
    renaming_type *ren;

    DBUG_ENTER ("DbugRen");

    ren = renaming;
    while (ren) {
        printf ("%s -> %s,    Vardec :%p\n", ren->old, ren->new, ren->vardec);
        ren = ren->next;
    }

    DBUG_VOID_RETURN;
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
 *   intern_gen *MergeGenerators(intern_gen *ig)
 *
 * description:
 *   tries to merge gererators (grids) to reduce effort of further
 *   intersections.
 *   This optimization can result in better codeproduction of compile
 *   (wltransform.c, OptimizeWL does something similar but it not that
 *    powerful).
 *
 ******************************************************************************/

intern_gen *
MergeGenerators (intern_gen *ig)
{
    DBUG_ENTER ("MergeGenerators");

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *LinearTransformationsHelp(intern_gen *ig, int dim, prf prf, int arg_no,
 *int const)
 *
 * description:
 *   Executes transformation (prf const) on the given ig in dimension dim.
 *   More complex transformations require to split up generators in two new
 *   generators (original ig and newig). If a newig is created, it is returned.
 *
 *
 ******************************************************************************/

intern_gen *
LinearTransformationsHelp (intern_gen *ig, int dim, prf prf, int arg_no, int constval)
{
    int lbuf, ubuf, cut, buf;
    intern_gen *newig = NULL;

    DBUG_ENTER ("LinearTransformationsHelp");
    DBUG_ASSERT (1 == arg_no || 2 == arg_no, ("wrong parameters"));

    switch (prf) {
    case F_add:
        /* addition is commutable, grids are not affected. */
        ig->l[dim] -= constval;
        ig->u[dim] -= constval;
        break;

    case F_sub:
        if (2 == arg_no) {
            /* index - scalar, grids are not affected */
            ig->l[dim] += constval;
            ig->u[dim] += constval;
        } else {
            /* scalar - index */
            lbuf = constval - ig->u[dim] + 1; /* +1 to transform < in <= */
            ubuf = constval - ig->l[dim] + 1; /* +1 to transform <= in < */

            if (ig->step) {
                /* reverse the grid.
                   Example:
                   "xxx  xxx  xx" => "xx  xxx  xxx"
                   done with 2 gens: "xx   xx   xx" &&
                   "x    x  " */
                cut = (ig->u[dim] - ig->l[dim]) % ig->step[dim];
                if (cut == 0)
                    lbuf += ig->step[dim] - ig->width[dim];
                else if (cut > ig->width[dim])
                    lbuf += cut - ig->width[dim];
                else if (cut < ig->width[dim]) {
                    /* make newig */
                    newig = CopyInternGen (ig);
                    newig->l[dim] = ig->l[dim] + cut;
                    newig->width[dim] = ig->width[dim] - cut;
                    ig->width[dim] = cut;
                    /* if u - l is smaller than width, the newig is empty */
                    if (newig->u[dim] <= newig->l[dim]) {
                        FREE_INTERN_GEN (newig);
                    } else {
                        buf = (constval - newig->u[dim] + 1 + newig->step[dim]
                               - newig->width[dim]);
                        newig->u[dim] = constval - newig->l[dim] + 1;
                        newig->l[dim] = buf;
                    }
                }
            }

            ig->l[dim] = lbuf;
            ig->u[dim] = ubuf;
        }
        break;

    case F_mul:
        ig->l[dim] = (ig->l[dim] + constval - 1) / constval;
        ig->u[dim] = (ig->u[dim] - 1) / constval + 1;
        if (ig->step)
            DBUG_ASSERT (0, ("WL folding with transformed index variables "
                             "by multiplication and grids not supported right now."));
        break;

    case F_div:
        ig->l[dim] = ig->l[dim] * constval;
        ig->u[dim] = (ig->u[dim] - 1) * constval + constval - 1 + 1;
        if (ig->step)
            DBUG_ASSERT (0, ("WL folding with transformed index variables "
                             "by division  and grids not supported right now."));
        break;

    default:
        DBUG_ASSERT (0, ("Wrong transformation function"));
    }

    DBUG_RETURN (newig);
}

/******************************************************************************
 *
 * function:
 *   void LinearTransformationsScalar(intern_gen *ig, index_info *transformations, dim)
 *
 * description:
 *   like LinearTransformationsVector(), but only transforms one dimension of ig.
 *
 *
 ******************************************************************************/

intern_gen *
LinearTransformationsScalar (intern_gen *ig, index_info *transformations, int dim)
{
    intern_gen *actig, *newig;

    DBUG_ENTER ("LinearTransformationsScalar");
    DBUG_ASSERT (0 == transformations->vector, ("wrong parameters"));
    DBUG_ASSERT (!transformations->last[0] || !transformations->last[0]->vector,
                 ("scalar points to vector"));
    DBUG_ASSERT (transformations->permutation[0], ("scalar constant???"));

    actig = ig;
    if (transformations->arg_no)
        /* valid prf. */
        while (actig) { /* all igs */
            newig = LinearTransformationsHelp (actig, dim, transformations->prf,
                                               transformations->arg_no,
                                               transformations->const_arg[0]);

            if (newig) {
                newig->next = ig; /* insert new element before whole list. */
                ig = newig;       /* set new root */
            }
            actig = actig->next;
        }

    if (transformations->last[0])
        ig = LinearTransformationsScalar (ig, transformations->last[0], dim);

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   void LinearTransformationsVector(intern_gen *ig, index_info *transformations)
 *
 * description:
 *   realizes transformations on the given list of intern gens.
 *
 *
 ******************************************************************************/

intern_gen *
LinearTransformationsVector (intern_gen *ig, index_info *transformations)
{
    int dim, act_dim;
    intern_gen *actig, *newig;

    DBUG_ENTER ("LinearTransformationsVector");
    DBUG_ASSERT (transformations->vector == ig->shape,
                 ("Transformations to not fit to generators"));

    dim = ig->shape;

    if (transformations->vector && transformations->arg_no)
        /* vector transformation and a valid prf. */
        for (act_dim = 0; act_dim < dim; act_dim++) { /* all dimensions */
            actig = ig;
            if (transformations->permutation[act_dim])
                while (actig) { /* all igs */
                    newig
                      = LinearTransformationsHelp (actig, act_dim, transformations->prf,
                                                   transformations->arg_no,
                                                   transformations->const_arg[act_dim]);
                    if (newig) {
                        newig->next = ig; /* insert new element before whole list. */
                        ig = newig;       /* set new root */
                    }
                    actig = actig->next;
                }
        }

    if (transformations->last[0] && transformations->last[0]->vector)
        ig = LinearTransformationsVector (ig, transformations->last[0]);
    else { /* maybe additional scalar transformations */
        for (act_dim = 0; act_dim < dim; act_dim++)
            if (transformations->last[act_dim])
                ig = LinearTransformationsScalar (ig, transformations->last[act_dim],
                                                  act_dim);
    }

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *FinalTransformations(intern_gen *ig, index_info *transformations, int
 *target_dim);
 *
 * description:
 *   transforms list of ig into cuboids of dimension target_dim.
 *
 *
 ******************************************************************************/

intern_gen *
FinalTransformations (intern_gen *substig, index_info *transformations, int target_dim)
{
    intern_gen *tmpig, *newig, *rootig;
    int ok, i, *help;

    DBUG_ENTER ("FinalTransformations");
    DBUG_ASSERT (transformations->vector == substig->shape, ("wrong parameters"));

    /* create array to speed up later transformations.
       help[i] is
       - 0 if the ith index scalar is not used to index the subst array
       - d if the ith index scalar addresses the dth component of the subst array.

       Example:
       iv=[i,j,k]
       subst array A[42,i,k]
       help: [2,0,3] */
    i = sizeof (int) * target_dim;
    help = Malloc (i);
    help = memset (help, 0, i);
    for (i = 0; i < transformations->vector; i++)
        if (transformations->permutation[i])
            help[transformations->permutation[i] - 1] = i + 1;

    /* now process all substig and create new ig list rootig */
    tmpig = substig;
    newig = rootig = NULL;

    while (tmpig) {
        /* Check whether constants in *transformations disqualify this tmpig */
        ok = 1;
        for (i = 0; i < tmpig->shape && ok; i++)
            ok = (transformations->permutation[i] ||                /* not a constant */
                  ((tmpig->l[i] <= transformations->const_arg[i] && /* in range */
                    transformations->const_arg[i] < tmpig->u[i])
                   && (!tmpig->step || /* and no grid */
                       (transformations->const_arg[i] - tmpig->l[i])
                           % /* or in this grid */
                           tmpig->step[i]
                         < tmpig->width[i])));

        if (ok) {
            /* start transformations */
            newig = CreateInternGen (target_dim, NULL != tmpig->step);
            for (i = 0; i < target_dim; i++)
                if (help[i]) {
                    newig->l[i] = tmpig->l[help[i] - 1];
                    newig->u[i] = tmpig->u[help[i] - 1];
                    if (tmpig->step) {
                        newig->step[i] = tmpig->step[help[i] - 1];
                        newig->width[i] = tmpig->width[help[i] - 1];
                    }
                } else {
                    newig->l[i] = 0;
                    newig->u[i] = INT_MAX;
                    if (tmpig->step) {
                        newig->step[i] = 1;
                        newig->width[i] = 1;
                    }
                }
            DBUG_ASSERT (0 == NormalizeInternGen (newig), ("Error while normalizing ig"));

            newig->code = tmpig->code;
            newig->next = rootig;
            rootig = newig;
        }

        /* go to next substig */
        tmpig = tmpig->next;
    }

    FREE (help);
    FreeInternGenChain (substig);

    DBUG_RETURN (rootig);
}

/******************************************************************************
 *
 * function:
 *   node *CreateCode(node *target, node *subst)
 *
 * description:
 *   substitutes the code block subst into target. INFO_WLI_ID points
 *   to the N_id node in the target WL which shall be replaced.
 *   New vardecs and assignments are introduced and an N_Ncode node is returned.
 *   Most of the work is done in the usual traversal steps (with mode
 *   wlfm_replace and wlfm_rename).
 *
 ******************************************************************************/

node *
CreateCode (node *target, node *subst)
{
    node *coden, *new_arg_info, *dup_info;

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
    new_arg_info->varno = ref_mode_arg_info->varno;

    /* DupTree() shall fill ID_WL of Id nodes with special information. So
       we have to call DupTree() with DUP_WLF. */
    dup_info = MakeInfo ();
    dup_info->flag = DUP_WLF; /* compare DUPTYPE in DupTree.h */

    coden = DupTree (NCODE_CBLOCK (target), dup_info);
    coden = Trav (coden, new_arg_info);
    coden = MakeNCode (coden, DupTree (NCODE_CEXPR (target), dup_info));

    FreeNode (new_arg_info);
    FreeNode (dup_info);

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
                /* process inner dimensions if lower bound is less then upper bound. */
                if (intersect_grids_baseig->l[dim] + first
                    < intersect_grids_baseig->u[dim]) {
                    intersect_grids_baseig->l[dim] += first;
                    intersect_grids_baseig->width[dim] = last - first;
                    IntersectGrids (dim + 1);
                    intersect_grids_baseig->l[dim] -= first; /* restore old value */
                } else
                    dc = intersect_grids_baseig->step[dim]; /* stop further search */
            } else {
                /* create new ig and add it to structures
                   if lower bound is less then upper bound. */
                if (intersect_grids_baseig->l[dim] + first
                    < intersect_grids_baseig->u[dim]) {
                    ig = CreateInternGen (intersect_grids_baseig->shape, 1);
                    for (d = 0; d < intersect_grids_baseig->shape; d++) {
                        ig->l[d] = intersect_grids_baseig->l[d];
                        ig->u[d] = intersect_grids_baseig->u[d];
                        ig->step[d] = intersect_grids_baseig->step[d];
                        ig->width[d] = intersect_grids_baseig->width[d];
                    }
                    ig->l[dim] += first;
                    ig->width[dim] = last - first;

                    /* add craetes code to code_constr list and to new_codes. */
                    cc = SearchCC (intersect_grids_tig->code, intersect_grids_sig->code);
                    if (cc)
                        ig->code = cc->new;
                    else {
                        coden = CreateCode (intersect_grids_tig->code,
                                            intersect_grids_sig->code);
                        ig->code = coden;
                        AddCC (intersect_grids_tig->code, intersect_grids_sig->code,
                               coden);
                        NCODE_NEXT (coden) = new_codes;
                        new_codes = coden;
                    }

                    /* add new generator to intersect_intern_gen list. */
                    ig->next = intersect_intern_gen;
                    intersect_intern_gen = ig;
                } else
                    dc = intersect_grids_baseig->step[dim]; /* stop further search */
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
    intern_gen *sig, *new_gen, *new_gen_step, *new_gen_nostep;
    int d, max_dim, l, u, create_steps;
    code_constr_type *cc;
    node *coden;

    DBUG_ENTER ("IntersectInternGen");
    DBUG_ASSERT (target_ig->shape == subst_ig->shape, ("wrong parameters"));

    max_dim = target_ig->shape;
    new_gen_step = NULL;
    new_gen_nostep = NULL;
    intersect_intern_gen = NULL;

    while (target_ig) {
        sig = subst_ig;
        while (sig) {
            if (!new_gen_step)
                new_gen_step = CreateInternGen (target_ig->shape, 1);
            if (!new_gen_nostep)
                new_gen_nostep = CreateInternGen (target_ig->shape, 0);

            create_steps = target_ig->step || sig->step;
            if (create_steps) {
                new_gen = new_gen_step;
                new_gen_step = NULL;
            } else {
                new_gen = new_gen_nostep;
                new_gen_nostep = NULL;
            }

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
                            new_gen->step[d] = lcm (target_ig->step[d], sig->step[d]);

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
                    /* the mem of new_gen has not been erased in IntersectGrids(). It can
                       be used for the next intersection. Don't free it. */
                    new_gen_step = new_gen;
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
                    new_gen = NULL;
                }
            }

            sig = sig->next; /* go on with next subst ig */
        }
        target_ig = target_ig->next; /* go on with next target ig */
    }

    if (new_gen_step)
        FREE_INTERN_GEN (new_gen_step);
    if (new_gen_nostep)
        FREE_INTERN_GEN (new_gen_nostep);

    DBUG_RETURN (intersect_intern_gen);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *RemoveDoubleIndexVectors(intern_gen *subst_ig, index_info
 **transformations)
 *
 * description:
 *   only used if permutations of index scalar variables are enabled.
 *   This is a transformation to reduce the number of components of the ig.
 *
 ******************************************************************************/

intern_gen *
RemoveDoubleIndexVectors (intern_gen *subst_ig, index_info *transformations)
{
    int *found, i, act_dim, dim, fdim;
    intern_gen *act_ig;

    DBUG_ENTER ("RemoveDoubleIndexVectors");

    i = sizeof (int) * transformations->vector;
    found = Malloc (i);
    found = memset (found, 0, i);

    for (act_dim = 0; act_dim < transformations->vector; act_dim++)
        if (transformations->permutation[act_dim] != 0) {
            dim = transformations->permutation[act_dim] - 1;
            if (found[dim] != 0) {
                /* dimensions found[dim] and act_dim of subst_ig both are based on
                   the dim-th index scalar. */
                act_ig = subst_ig;
                fdim = found[dim] - 1;
                transformations->vector--;
                while (act_ig) {
                    /* fold both dimensions fdim and act_dim to fdim ... */
                    act_ig->l[fdim] = MAX (act_ig->l[fdim], act_ig->l[act_dim]);
                    act_ig->u[fdim] = MIN (act_ig->u[fdim], act_ig->u[act_dim]);
                    act_ig->shape--;
                    /* ...and remove unused dim act_dim. */
                    for (i = act_dim; i < transformations->vector; i++) {
                        act_ig->l[i] = act_ig->l[i + 1];
                        act_ig->u[i] = act_ig->u[i + 1];
                        if (act_ig->step) {
                            act_ig->step[i] = act_ig->step[i + 1];
                            act_ig->width[i] = act_ig->width[i + 1];
                        }
                        transformations->permutation[i]
                          = transformations->permutation[i + 1];
                        transformations->last[i] = transformations->last[i + 1];
                        transformations->const_arg[i] = transformations->const_arg[i + 1];
                    }
                    act_ig = act_ig->next;
                }
                act_dim--;
            } else
                found[dim] = act_dim + 1;
        }

    FREE (found);

    DBUG_RETURN (subst_ig);
}

/******************************************************************************
 *
 * function:
 *   TransformationRangeCheck( )
 *
 * description:
 *
 *    if the psi-operation we process right now indexes the subst WL in a way
 *    so that invalid elements are chosen, the resulting partition will not
 *    specify a full partition.
 *
 *    Example: A = WL([0] <= i1 < [10) ...
 *             B = WL([0] <= i2 < [10])
 *               genarray(., A[i2+3])
 *             Here for i2 = 10..12 A[i2] is not defined.
 *
 *    We can detect these errors by transforming the minimal and maximal
 *    bounds of the subst WL and check if this new cuboid covers all
 *    cuboids of target_ig.
 *    If yes, the resulting intersection will cover the same set as target_ig.
 *    Else we return the number of the first dimension in which such a
 *    range violation was detected.
 *
 *
 ******************************************************************************/

int
TransformationRangeCheck (index_info *transformations, node *substwln,
                          intern_gen *target_ig)
{
    int result, dim, i;
    node *tmpn;
    intern_gen *whole_ig;

    DBUG_ENTER ("TransformationRangeCheck");

    /* create bounds of substwln in whole_ig */
    dim = transformations->vector;
    whole_ig = CreateInternGen (dim, 0);
    switch (NWITH_TYPE (substwln)) {
    case WO_genarray:
        tmpn = ARRAY_AELEMS (NWITHOP_SHAPE (NWITH_WITHOP (substwln)));
        for (i = 0; i < dim; i++) {
            whole_ig->l[i] = 0;
            whole_ig->u[i] = NUM_VAL (EXPRS_EXPR (tmpn));
            tmpn = EXPRS_NEXT (tmpn);
        }
        break;

    case WO_modarray:
        for (i = 0; i < dim; i++) {
            whole_ig->l[i] = 0;
            whole_ig->u[i] = ID_SHAPE (NWITHOP_ARRAY (NWITH_WITHOP (substwln)), i);
        }
        break;

    default:
        DBUG_ASSERT (0, ("TransformationRangeCheck called with fold-op"));
    }

    /* transform whole_ig */
    whole_ig = LinearTransformationsVector (whole_ig, transformations);
#ifndef TRANSF_TRUE_PERMUTATIONS
    whole_ig = RemoveDoubleIndexVectors (whole_ig, transformations);
#endif
    whole_ig = FinalTransformations (whole_ig, transformations, target_ig->shape);

    /* whole_ig has to cover all igs in target_ig. */
    result = 0;
    dim = target_ig->shape;
    while (!result && target_ig) {
        for (i = dim - 1; i >= 0; i--)
            if (target_ig->l[i] < whole_ig->l[i] || target_ig->u[i] > whole_ig->u[i])
                result = i + 1;

        target_ig = target_ig->next;
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   void Fold(node *idn, index_info *transformation, node *targetwln, node *substwln)
 *
 * description:
 *   The Id idn in the current (arg_info) WL has to be folded.
 *
 *
 ******************************************************************************/

void
Fold (node *idn, index_info *transformations, node *targetwln, node *substwln)
{
    intern_gen *target_ig; /* transformed igs of target WL */
    intern_gen *subst_ig;  /* transformed igs of subst WL */
    intern_gen *tmpig, *intersect_ig;
    int error;
    index_info *transf2;

    DBUG_ENTER ("Fold");

    code_constr = NULL;

    /* create target_ig */
    target_ig = new_ig;
    new_ig = NULL;

    /* the 'old' new_ig is not needed anymore because we create a new list of
       intern gens which represents the same set of index elements as the
       'old' list. */
    new_ig = FreeInternGenChain (new_ig);

    /* check if array access is in range. Don't use the original *transformations
       because RemoveDoubleIndexVectors() modifies it.  */
    transf2 = DuplicateIndexInfo (transformations);
    error = TransformationRangeCheck (transf2, substwln, target_ig);
    FREE_INDEX (transf2);
    if (error)
        ABORT (NODE_LINE (idn),
               ("array access to %s out of range in dimension %i", ID_NAME (idn), error));

    /* create subst_ig */
    subst_ig = Tree2InternGen (substwln, NULL);
    subst_ig = LinearTransformationsVector (subst_ig, transformations);
    /* We can use the original *transformations here because we don't need it
       anymore. */
#ifndef TRANSF_TRUE_PERMUTATIONS
    subst_ig = RemoveDoubleIndexVectors (subst_ig, transformations);
#endif
    subst_ig = FinalTransformations (subst_ig, transformations, target_ig->shape);

    if (!subst_ig)
        ABORT (NODE_LINE (idn), ("constants of index vector out of range"));

    /* intersect target_ig and subst_ig
       and create new code blocks */
    intersect_ig = IntersectInternGen (target_ig, subst_ig);

    /* results are in intersect_ig. At the moment, just append to new_ig. */
    DBUG_ASSERT (intersect_ig, ("No new intersections"));
    tmpig = new_ig;
    if (!tmpig) /* at the mom: always true */
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
 *   The flag FOLDABLE of every WL is
 *     1 if there are const array bounds
 *     0 else.
 *
 *   PARTS is
 *    -1 for WL which have no full partition
 *    >0 otherwise
 *     1 for fold-WL with constant bounds.
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
         && NWITH_FOLDABLE (target_wl) > 0
         && NWITH_REFERENCED (subst_wl) == NWITH_REFERENCED_FOLD (subst_wl)
         && (WO_genarray == NWITH_TYPE (subst_wl) || WO_modarray == NWITH_TYPE (subst_wl))
         && !NWITH_NO_CHANCE (subst_wl));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *CheckForSuperfluousCodes(node *wln)
 *
 * description:
 *   remove all unused N_Ncode nodes of the given WL.
 *
 *
 ******************************************************************************/

node *
CheckForSuperfluousCodes (node *wln)
{
    node **tmp;

    DBUG_ENTER ("CheckForSuperfluousCodes");

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
 *   node *Modarray2Genarray(node *wln, node *substwln)
 *
 * description:
 *   transforms the withop of the given WL. The goal is remove a reference
 *   to a WL which can be folded so that DCR can eleminate the WL.
 *
 * example:
 *   B = WL() genarray([shape],...);
 *   C = WL() modarray(B,...);         =>  C = WL() genarray([newshape],...)
 *     where newshape depends on shape and the given index vector of C.
 *
 ******************************************************************************/

node *
Modarray2Genarray (node *wln, node *substwln)
{
    node *shape, *eltn;
    types *type;
    int dimensions, i;
    shpseg *shpseg;

    DBUG_ENTER ("Modarray2Genarray");
    DBUG_ASSERT (WO_modarray == NWITH_TYPE (wln), ("wrong withop for Modarray2Genarray"));
    DBUG_ASSERT (substwln, ("substwln ist NULL"));

    /* at the moment, substwln points to the assignment of the WL. */
    substwln = LET_EXPR (ASSIGN_INSTR (substwln));
    (NWITH_REFERENCES_FOLDED (substwln))++; /* removed another reference */

    /* compute shape of WL for NWITHOP_SHAPE() */
    type = ID_TYPE (NWITHOP_ARRAY (NWITH_WITHOP (wln)));
    dimensions = IDS_SHAPE (NPART_VEC (NWITH_PART (wln)), 0);

    eltn = NULL;
    for (i = dimensions - 1; i >= 0; i--)
        eltn = MakeExprs (MakeNum (TYPES_SHAPE (type, i)), eltn);

    shape = MakeArray (eltn);

    shpseg = MakeShpseg (
      MakeNums (dimensions, NULL)); /* nums struct is freed inside MakeShpseg. */
    ARRAY_TYPE (shape) = MakeType (T_int, 1, shpseg, NULL, NULL);

    /* delete old withop and create new one */
    FreeTree (NWITH_WITHOP (wln));
    NWITH_WITHOP (wln) = MakeNWithOp (WO_genarray);
    NWITHOP_SHAPE (NWITH_WITHOP (wln)) = shape;

    DBUG_RETURN (wln);
}

/******************************************************************************
 *
 * Functions for node_info.mac (traversal mechanism)
 *
 ******************************************************************************/

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

    /* copy number of vardec explicitely in this phase because the OPTTrav()
       mechanism is not used. */
    INFO_VARNO = FUNDEF_VARNO (arg_node);

    wlf_mode = wlfm_search_WL;
    if (FUNDEF_BODY (arg_node))
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    if (FUNDEF_ARGS (arg_node))
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);

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
    types *idt;
    int i;

    DBUG_ENTER ("WLFassign");

    INFO_WLI_ASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    switch (wlf_mode) {
    case wlfm_search_WL:
        if (ASSIGN_NEXT (arg_node))
            /* We must not assign the returned node to ASSIGN_NEXT
               because the chain has been modified in Trav(wlfm_rename).
               This is not very nice and bypassed the traversal mechanism,
               but it is necessary since cross pointers are used as references
               to N_assign nodes and new code is inserted there. */
            /*       ASSIGN_NEXT(arg_node)  = Trav(ASSIGN_NEXT(arg_node), arg_info); */
            Trav (ASSIGN_NEXT (arg_node), arg_info);

        /* now it's the time when we run the once traversed path through the
           syntax tree back.
           We apply a speed-optimization here by removing WLs which - after WLF -
           are not referenced anymore. DCR could remove the WLs later, but we
           hope that the mem requirement in GenerateMasks will go down dramatically.
           */
        if (opt_dcr) {
            tmpn = ASSIGN_INSTR (arg_node);
            if (N_let == NODE_TYPE (tmpn)) {
                if (N_Nwith == NODE_TYPE (LET_EXPR (tmpn))
                    && NWITH_REFERENCED (LET_EXPR (tmpn)) > 0
                    && NWITH_REFERENCED (LET_EXPR (tmpn))
                         == NWITH_REFERENCES_FOLDED (LET_EXPR (tmpn))) {
                    LET_EXPR (tmpn) = FreeTree (LET_EXPR (tmpn));
                    /* create genarray([..],0) and replace the WL. This does not change
                       the program symantically because the WL is not referenced
                       anymore.*/
                    idt = IDS_TYPE (LET_IDS (tmpn));
                    tmpn = NULL;
                    for (i = TYPES_DIM (idt); i > 0; i--)
                        tmpn = MakeExprs (MakeNum (TYPES_SHAPE (idt, i - 1)),
                                          tmpn); /* Array elements */
                    tmpn = MakeArray (tmpn);     /* N_Array */
                    tmpn
                      = MakePrf (F_genarray, /* prf N_genarray */
                                 MakeExprs (tmpn,
                                            MakeExprs (MakeNullVec (0,
                                                                    TYPES_BASETYPE (idt)),
                                                       NULL)));
                    LET_EXPR (ASSIGN_INSTR (arg_node)) = tmpn;
                }
            }
        }

        break;

    case wlfm_replace:
        if (INFO_WLI_NEW_ID (arg_info)) {
            /* paste the new code (subst WL code) into the assign chain (target WL
               code). The current assign node is appended to the new INFO_WLI_SUBST
               chain. Therefor, reuse the current assign node because the
               previous assign node (which cannot be reached from here) points
               to it. */

            substn = INFO_WLI_SUBST (arg_info);
            /* ist there something to insert? */
            if (substn) {
                last_assign
                  = MakeAssign (ASSIGN_INSTR (arg_node), ASSIGN_NEXT (arg_node));
                ASSIGN_INSTR (arg_node) = ASSIGN_INSTR (substn);
                ASSIGN_NEXT (arg_node) = ASSIGN_NEXT (substn);

                /* fisrt assignment node of substn is not needed anymore. */
                ASSIGN_INSTR (substn) = NULL;
                FreeNode (substn); /* free only node, not tree! */

                tmpn = arg_node;
                while (ASSIGN_NEXT (tmpn))
                    tmpn = ASSIGN_NEXT (tmpn);
                ASSIGN_NEXT (tmpn) = last_assign;
                tmpn = ASSIGN_NEXT (tmpn);
            } else
                tmpn = arg_node;

            /* transform psi(.,array) into Id. */
            tmpn = ASSIGN_INSTR (tmpn);
            LET_EXPR (tmpn) = FreeTree (LET_EXPR (tmpn));
            LET_EXPR (tmpn) = INFO_WLI_NEW_ID (arg_info);
        } else if (ASSIGN_NEXT (arg_node))
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        break;

    default:
        /* Do not assign ASSIGN_NEXT(arg_node) here.
           Why? See comment in wlfm_search_wl branch of this function. */
        if (ASSIGN_NEXT (arg_node))
            Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFid(node *arg_node, node *arg_info)
 *
 * description:
 *  In wlfm_replace phase insert variable definitions of index vector
 *  and index scalars. Remove psi operation and replace it with the
 *  body of the subst WL.
 *  In wlfm_rename phase detect name clashes and solve them.
 *
 ******************************************************************************/

node *
WLFid (node *arg_node, node *arg_info)
{
    node *substn, *coden, *vectorn, *argsn, *letn, *subst_wl_partn, *subst_header;
    node *old_arg_info_assign, *arrayn;
    ids *subst_wl_ids, *_ids;
    int count, varno;
    char *new_name;
    renaming_type *ren;
    shpseg *shpseg;

    DBUG_ENTER ("WLFid");

    switch (wlf_mode) {
    case wlfm_search_WL:
        /* check if we want to fold this Id.
           If no, clear ID_WL. This is the sign for the following phases to fold.
           If yes, set INFO_WLI_FLAG. This is only a speed-up flag. If it is 0,
           we do not enter the wlfm_search_ref phase.
           We used ID_WL in case (1) here (see header of file) */
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
        /* This ID_WL is used in case (2) here (see header of file) */
        if (ID_WL (arg_node) == INFO_WLI_ID (arg_info)) {
            /* this is the Id which has to be replaced. */
            /* First store the C_EXPR in INFO_WLI_NEW_ID. Usage: see WLFassign.
               It might happen that this C_EXPR in the substn will be renamed when
               loops surround the WL. Then we have to rename the C_EXPR, too. */
            renaming = NULL; /* Used by AddRen(), SearchRen() */
            coden = INFO_WLI_SUBST (arg_info);
            varno = ID_VARNO (NCODE_CEXPR (coden));
            if (target_mask[varno]) { /* target_mask was initialized before Fold() */
                new_name = TmpVarName (ID_NAME (NCODE_CEXPR (coden)));
                ren = AddRen (ID_VARDEC (NCODE_CEXPR (coden)), new_name,
                              ID_TYPE (NCODE_CEXPR (coden)), arg_info, 1);
                INFO_WLI_NEW_ID (arg_info) = MakeId (new_name, NULL, ST_regular);
                ID_VARDEC (INFO_WLI_NEW_ID (arg_info)) = ren->vardec;
            } else
                /* keep original name */
                INFO_WLI_NEW_ID (arg_info) = DupTree (NCODE_CEXPR (coden), NULL);

            /* Create substitution code. */
            if (N_empty == NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (coden))))
                substn = NULL;
            else
                substn = DupTree (BLOCK_INSTR (NCODE_CBLOCK (coden)), NULL);

            /* create assignments to rename variables which index the array we want
               to replace.
               Example: We want to replace psi(sel,A) and A is a WL with a
               generator iv=[i,j,k]. Then we add:
                 i = psi([0],sel);
                 j = psi([1],sel);
                 k = psi([2],sel);
                 iv = sel;
               Remember that iv,i,j,k all are temporary variables, inserted
               in flatten. No name clashes can happen. */
            subst_header = NULL;
            vectorn = PRF_ARG1 (LET_EXPR (ASSIGN_INSTR (INFO_WLI_ASSIGN (arg_info))));
            count = 0;
            /* This ID_WL is used in case (1) here (see header of file) */
            subst_wl_partn
              = NWITH_PART (LET_EXPR (ASSIGN_INSTR (ID_WL (INFO_WLI_ID (arg_info)))));
            subst_wl_ids = NPART_IDS (subst_wl_partn);
            while (subst_wl_ids) {
                /* Here we use masks which have been generated before WLI. These masks
                   are never modified in WLI and WLF (although new VARDECs are inserted),
                   so we can use them for variables which already existed BEFORE WLI.
                   Now let's see if we can use masks for index variables:
                   - yes, if this is no newly created code block (WLF)
                   - no otherwise. We could duplicate masks for new code blocks,
                     but I guess that would cost more time than speculatively inserting
                     (and DC-removing) some new variables. */
                if (!NCODE_MASK (coden, 1 /* USE mask */) /* mask does not exist */
                    || NCODE_MASK (coden, 1)[IDS_VARNO (subst_wl_ids)]) {
                    arrayn = MakeArray (MakeExprs (MakeNum (count), NULL));
                    shpseg = MakeShpseg (
                      MakeNums (1, NULL)); /* nums struct is freed inside MakeShpseg. */
                    ARRAY_TYPE (arrayn) = MakeType (T_int, 1, shpseg, NULL, NULL);
                    argsn = MakeExprs (arrayn, MakeExprs (DupTree (vectorn, NULL), NULL));

                    /* maybe the index scalar variable of the subst wl has to be renamed
                     */
                    if (target_mask[IDS_VARNO (subst_wl_ids)]) {
                        new_name = TmpVarName (IDS_NAME (subst_wl_ids));
                        ren = AddRen (IDS_VARDEC (subst_wl_ids), new_name,
                                      IDS_TYPE (subst_wl_ids), arg_info, 0);
                        _ids = MakeIds (new_name, NULL, ST_regular);
                        IDS_VARDEC (_ids) = ren->vardec;
                    } else
                        /* keep original name */
                        _ids = DupOneIds (subst_wl_ids, NULL);

                    letn = MakeLet (MakePrf (F_psi, argsn), _ids);
                    subst_header = MakeAssign (letn, subst_header);
                }

                count++;
                subst_wl_ids = IDS_NEXT (subst_wl_ids);
            }

            /* now add "iv = sel" (see example above) */
            subst_wl_ids = NPART_VEC (subst_wl_partn);
            if (!NCODE_MASK (coden, 1 /* USE mask */) /* mask does not exist */
                || NCODE_MASK (coden, 1)[IDS_VARNO (subst_wl_ids)]) {
                /* maybe the index vector variable of the subst wl has to be renamed */
                if (target_mask[IDS_VARNO (subst_wl_ids)]) {
                    new_name = TmpVarName (IDS_NAME (subst_wl_ids));
                    ren = AddRen (IDS_VARDEC (subst_wl_ids), new_name,
                                  IDS_TYPE (subst_wl_ids), arg_info, 0);
                    _ids = MakeIds (new_name, NULL, ST_regular);
                    IDS_VARDEC (_ids) = ren->vardec;
                } else
                    /* keep original name */
                    _ids = DupOneIds (subst_wl_ids, NULL);

                letn = MakeLet (DupTree (vectorn, NULL), _ids);
                subst_header = MakeAssign (letn, subst_header);
            }

            /* trav subst code with wlfm_rename to solve name clashes. */
            if (substn) {
                wlf_mode = wlfm_rename;
                old_arg_info_assign = INFO_WLI_ASSIGN (arg_info); /* save arg_info */
                substn = Trav (substn, arg_info);
                INFO_WLI_ASSIGN (arg_info) = old_arg_info_assign;
                FreeRen (); /* set list free (created in wlfm_rename) */
                wlf_mode = wlfm_replace;
            }

            /* merge subst_header and substn */
            /* we dont' need the old _SUBST info anymore so we return the
               new subst assign chain here. WLFassign uses this information to
               melt both chains. */
            if (subst_header) {
                INFO_WLI_SUBST (arg_info) = subst_header;
                while (ASSIGN_NEXT (subst_header))
                    subst_header = ASSIGN_NEXT (subst_header);
                ASSIGN_NEXT (subst_header) = substn;
            } else
                INFO_WLI_SUBST (arg_info) = substn;
        }
        break;

    case wlfm_rename:
        varno = ID_VARNO (arg_node);
        ren = SearchRen (ID_NAME (arg_node));
        if (ren || (varno != -1 && target_mask[varno] != subst_mask[varno])) {
            /* we have to solve a name clash. */
            if (ren)
                new_name = StringCopy (ren->new);
            else {
                new_name = TmpVarName (ID_NAME (arg_node));
                /* Get vardec's name because ID_NAME will be deleted soon. */
                ren = AddRen (ID_VARDEC (arg_node), new_name, ID_TYPE (arg_node),
                              arg_info, 1);
            }
            /* replace old name now. */
            FREE (ID_NAME (arg_node));
            ID_NAME (arg_node) = new_name;
            ID_VARDEC (arg_node) = ren->vardec;
        }

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
 *   If in wlfm_search_ref mode, initiate folding.
 *   In wlfm_rename phase detect name clashes and solve them.
 *
 ******************************************************************************/

node *
WLFlet (node *arg_node, node *arg_info)
{
    node *prfn, *idn, *targetwln, *substwln;
    char *new_name;
    renaming_type *ren;
    int varno;
    index_info *transformation;

    DBUG_ENTER ("WLFlet");

    switch (wlf_mode) {
    case wlfm_search_WL:
    case wlfm_replace:
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        break;

    case wlfm_search_ref:
        /* is this a prf psi() which can be folded? */
        /* All occurences of ID_WL are used in case (1) here (see header of file) */
        prfn = LET_EXPR (arg_node);
        if (N_prf == NODE_TYPE (prfn) && F_psi == PRF_PRF (prfn)
            && ID_WL (PRF_ARG2 (prfn))) { /* second arg of psi() references a WL. */

            idn = PRF_ARG2 (prfn);

            /* These assignments are only done to transfer information
               to CreateCode (wlfm_replace) */
            INFO_WLI_ID (arg_info) = idn;
            INFO_WLI_NCA (arg_info) = ID_WL (idn);

            ref_mode_arg_info = arg_info; /* needed in CreateCode() */
            substwln = LET_EXPR (ASSIGN_INSTR (ID_WL (idn)));
            targetwln = INFO_WLI_WL (arg_info);
            target_mask = ASSIGN_MRDMASK (INFO_WLI_ASSIGN (arg_info));
            subst_mask = ASSIGN_MRDMASK (ID_WL (idn));

            /* We just traversed the original code in the wlfm_search_ref phase, so
               ASSIGN_INDEX provides the correct index_info.*/
            transformation = INDEX (INFO_WLI_ASSIGN (arg_info));

            DBUG_PRINT ("WLF", ("folding array %s in line %d now", ID_NAME (idn),
                                NODE_LINE (arg_node)));
            Fold (idn, transformation, targetwln, substwln);
            wlf_expr++;
            /* the WL substwln is now referenced one times less. */
            (NWITH_REFERENCES_FOLDED (substwln))++;

            /* unused here */
            INFO_WLI_ID (arg_info) = NULL;
            INFO_WLI_NCA (arg_info) = NULL;
        }

        break;

    case wlfm_rename:
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
        varno = IDS_VARNO (LET_IDS (arg_node));
        if (varno != -1 && target_mask[varno]) {
            /* we have to solve a name clash. */
            ren = SearchRen (IDS_NAME (LET_IDS (arg_node)));
            if (ren)
                new_name = StringCopy (ren->new);
            else {
                new_name = TmpVarName (IDS_NAME (LET_IDS (arg_node)));
                ren = AddRen (IDS_VARDEC (LET_IDS (arg_node)), new_name,
                              IDS_TYPE (LET_IDS (arg_node)), arg_info, 0);
            }
            /* replace old name now. */
            FreeAllIds (LET_IDS (arg_node));
            LET_IDS (arg_node) = MakeIds (new_name, NULL, ST_regular);
            IDS_VARDEC (LET_IDS (arg_node)) = ren->vardec;
        }
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
 *   First, traverse all bodies to fold WLs inside. Afterwards traverse
 *   codes again to find foldable assignments. If folding was successful,
 *   replace old generators with new ones, delete superflous code blocks and
 *   transform any modarray into gerarray.
 *
 ******************************************************************************/

node *
WLFNwith (node *arg_node, node *arg_info)
{
    node *tmpn, *substwln;

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

        /* if WO_modarray, save referenced WL to transform modarray into
           genarray later. */
        if (WO_modarray == NWITH_TYPE (arg_node))
            /* This ID_WL is used in case (1) here (see header of file) */
            substwln = ID_WL (NWITHOP_ARRAY (NWITH_WITHOP (arg_node)));

        /* It's faster to
           1. traverse into bodies to find WL within and fold them first
           2. and then try to fold references to other WLs.
           */
        INFO_WLI_FLAG (arg_info) = 0;
        DBUG_PRINT ("WLF", ("traversing body of WL in line %d", NODE_LINE (arg_node)));
        arg_node = TravSons (arg_node, arg_info);

        if (INFO_WLI_FLAG (arg_info)) {
            /* traverse bodies of this WL again and fold now.
               Do not traverse WithOp (Id would confuse us). */
            wlf_mode = wlfm_search_ref; /* we do not need to save arg_info here */

            all_new_ig = NULL;
            new_codes = NULL;

            intersect_grids_ot
              = Malloc (sizeof (int) * IDS_SHAPE (NWITH_VEC (arg_node), 0));
            intersect_grids_os
              = Malloc (sizeof (int) * IDS_SHAPE (NWITH_VEC (arg_node), 0));

            DBUG_PRINT ("WLF",
                        ("something to fold in WL in line %d", NODE_LINE (arg_node)));
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

                /* before replacing the old generators with the new ones, we
                   can try to merge several of the new generators. */
                all_new_ig = MergeGenerators (all_new_ig);

                arg_node = InternGen2Tree (arg_node, all_new_ig);
                all_new_ig = FreeInternGenChain (all_new_ig);
                arg_node = CheckForSuperfluousCodes (arg_node);
                DBUG_PRINT ("WLF", ("new generators created"));
            }
            DBUG_PRINT ("WLF", ("folding of last WL complete"));

            /* this WL is finisched. Search other WLs on same level. */
            wlf_mode = wlfm_search_WL;
        }

        /* If the current WL has a modarray-operator there is a referene to another
           array in its operator part. If this array was chosen to be folded
           (FoldDecision) we have to eleminate the reference to it. Else DCR
           would not remove the subst WL.*/
        if (WO_modarray == NWITH_TYPE (arg_node) && substwln
            && /* can be NULL if array is not in reach (loops, function argument)*/
            FoldDecision (arg_node, substwln))
            arg_node = Modarray2Genarray (arg_node, substwln);

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
 *   If in wlfm_search_ref phase, create list new_ig of gererators which
 *   point to the current code.
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
        /* create generator list. Copy all generators of the target-WL
           to this list which point to the current N_Ncode node. Don't use
           the traversal mechanism because it's slow. */
        new_ig = Tree2InternGen (INFO_WLI_WL (arg_info), arg_node);

        /* traverse Code, create new_ig, fold. */
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

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

        /* traverse next code block. */
        if (NCODE_NEXT (arg_node))
            NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
        break;

    default:
        DBUG_ASSERT (0, ("Not expected"));
    }

    DBUG_RETURN (arg_node);
}
