/*******************************************************************************

 This file realizes the withlop folding for a code in ssa form. it uses no
 masks anymore. Most code is unchanged from the original implementation in
 WLF.c . Due to the ssaform, we can simplify some task in WLF concerning
 renaming operations.

 *******************************************************************************

 Usage of arg_info:

 wlfm_search_WL  :
 wlfm_search_ref :
 - NEXT   : store old information in nested WLs
 - WL     : reference to base node of current WL (N_Nwith)
 - FLAG   : collects information in the wlfm_search phase if there is
            at least one foldable reference in the WL.

 wlfm_replace    :
 wlfm_rename     :
 - SUBST  : Pointer to subst N_Ncode/ pointer to copy of subst N_Ncode
 - NEW_ID : pointer to N_id which shall replace the prf sel()
            in the target Code.
 - ID     : pointer to original N_id node in target WL to replace
            by subst WL.
 - NCA    : points to the assignment of the subst WL. Here new
                     assignments are inserted to define new variables.

 all wlfm phases :
 - ASSIGN : always the last N_assign node
 - FUNDEF : pointer to last fundef node. Needed to access vardecs

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
     Creation of new code is initiated in CreateCode() and the resulting
     N_code nodes are collected in new_codes until they are inserted into the
     syntax tree in WLFNwith(). While a code is in new_codes, ID_WL of every
     Id inside the code points to its original Id (this is the Id which was
     copied by DUPdoDupTree()). DUPdoDupTree() sets this pointer and, if the
     argument of DUPdoDupTree() is a code inside new_codes, copies ID_WL of
     this code (which is a pointer to the original).

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"
#include "ctinfo.h"
#include "print.h"

#define DBUG_PREFIX "WLF"
#include "debug.h"

#include "traverse.h"
#include "SSAWithloopFolding.h"
#include "SSAWLF.h"
#include "shape.h"
#include "type_utils.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "constants.h"
#include "math_utils.h"

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
    info *next;
    node *subst;
    node *wl;
    node *new_id;
    node *assign;
    node *fundef;
    node *id;
    node *nca;
    int flag;
    int mystery;
};

/*
 * INFO macros
 */
#define INFO_NEXT(n) (n->next)
#define INFO_SUBST(n) (n->subst)
#define INFO_WL(n) (n->wl)
#define INFO_NEW_ID(n) (n->new_id)
#define INFO_ASSIGN(n) (n->assign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ID(n) (n->id)
#define INFO_NCA(n) (n->nca)
#define INFO_FLAG(n) (n->flag)
#define INFO_MYSTERY(n) (n->mystery)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_NEXT (result) = NULL;
    INFO_SUBST (result) = NULL;
    INFO_WL (result) = NULL;
    INFO_NEW_ID (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_ID (result) = NULL;
    INFO_NCA (result) = NULL;
    INFO_FLAG (result) = 0;
    INFO_MYSTERY (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

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
    node *mnew;
    struct CODE_CONSTRUCTION *next;
} code_constr_type;

/******************************************************************************
 *
 *   global variables
 *
 ******************************************************************************/

static wlf_mode_type wlf_mode;

static code_constr_type *code_constr; /* list of combinded code blocks */

static node *new_codes; /* list of created N_Ncode nodes */

static info *ref_mode_arg_info; /* saves arg_info for CreateCode(). Else
                                   we would have to pass arg_info through
                                   many functions (which costs time) */

static intern_gen *new_ig; /* new generators derived by a traversel
                              of one Ncode block. */

static intern_gen *all_new_ig; /* new generators derived from all Ncode
                                  nodes. */

/* global vars to speed up function call of IntersectGrids(). They are only used
   to transfer information between IntersectGrids() and IntersectInternGen(). */

static int *intersect_grids_ot; /* grid offsets used by IntersectGrids() */

static int *intersect_grids_os;

static intern_gen *intersect_grids_tig, *intersect_grids_sig, *intersect_grids_baseig;

static intern_gen *intersect_intern_gen; /* resulting igs of IntersectInternGen. */

/******************************************************************************
 *
 * function:
 *   node * FindCExpr (node *cexprs, node *avis, node *wlassign)
 *
 * description:
 *   expects avis to be defined on the LHS of wlassign and searches for the
 *   corresponding position in the exprs-chain cexprs.
 *   This is crucial for folding MO-WLs (see issue 2271 for details)
 *
 ******************************************************************************/

static node *
FindCExpr (node *cexprs, node *avis, node *wlassign)
{
    node *ids;
    DBUG_ENTER ();

    ids = ASSIGN_LHS (wlassign);
    while ((ids!=NULL) && (IDS_AVIS (ids) != avis)) {
        DBUG_ASSERT (cexprs != NULL, "inconsistent multi-operator WL in WLF");
        ids = IDS_NEXT (ids);
        cexprs = EXPRS_NEXT (cexprs);
    }
    DBUG_ASSERT (ids != NULL, "LHS variable in WL assignment not found!");
    DBUG_ASSERT (cexprs != NULL, "inconsistent multi-operator WL in WLF");

    DBUG_RETURN (EXPRS_EXPR (cexprs));
}

/******************************************************************************
 *
 * function:
 *   void AddCC(node *targetn, node *substn, node *resultn)
 *
 * description:
 *   adds entry to the global code_constr list.
 *
 ******************************************************************************/

static void
AddCC (node *targetn, node *substn, node *resultn)
{
    code_constr_type *cc;

    DBUG_ENTER ();

    cc = (code_constr_type *)MEMmalloc (sizeof (code_constr_type));
    cc->target = targetn;
    cc->subst = substn;
    cc->mnew = resultn;
    cc->next = code_constr;
    code_constr = cc;

    DBUG_RETURN ();
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

static code_constr_type *
SearchCC (node *targetn, node *substn)
{
    code_constr_type *cc;

    DBUG_ENTER ();

    cc = code_constr;
    while (cc && (cc->target != targetn || cc->subst != substn)) {
        cc = cc->next;
    }

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
 ******************************************************************************/

static void
FreeCC (code_constr_type *cc)
{
    code_constr_type *tmpcc;

    DBUG_ENTER ();

    while (cc) {
        tmpcc = cc;
        cc = cc->next;
        tmpcc = MEMfree (tmpcc);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   intern_gen *MergeGenerators(intern_gen *ig)
 *
 * description:
 *   tries to merge gererators (grids) to reduce effort of further
 *   intersections.
 *   This optimization can result in better code production of compile
 *   (wltransform.c, OptimizeWL does something similar but it not that
 *    powerful).
 *
 ******************************************************************************/

static intern_gen *
MergeGenerators (intern_gen *ig)
{
    DBUG_ENTER ();

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *LinearTransformationsHelp(intern_gen *ig, int dim, prf prf,
 *                                         int arg_no, int constval)
 *
 * description:
 *   Executes transformation (prf const) on the given ig in dimension dim.
 *   More complex transformations require to split up generators in two new
 *   generators (original ig and newig). If a newig is created, it is returned.
 *
 ******************************************************************************/

static intern_gen *
LinearTransformationsHelp (intern_gen *ig, int dim, prf prf, int arg_no, int constval)
{
    int lbuf, ubuf, cut, buf;
    intern_gen *newig = NULL;

    DBUG_ENTER ();
    DBUG_ASSERT (((1 == arg_no) || (2 == arg_no)), "wrong parameters");

    switch (prf) {
    case F_add_SxS:
        /* addition is commutable, grids are not affected. */
        ig->l[dim] -= constval;
        ig->u[dim] -= constval;
        break;

    case F_sub_SxS:
        if (2 == arg_no) {
            /* index - scalar, grids are not affected */
            ig->l[dim] += constval;
            ig->u[dim] += constval;
        } else { /* arg_no == 1 */
            /* scalar - index */
            lbuf = constval - ig->u[dim] + 1; /* +1 to transform < in <= */
            ubuf = constval - ig->l[dim] + 1; /* +1 to transform <= in < */

            if (ig->step) {
                /*
                 * reverse the grid.
                 * Example:
                 * "xxx  xxx  xx" => "xx  xxx  xxx"
                 * done with 2 gens: "xx   xx   xx" &&
                 *                       "x    x  "
                 */
                cut = (ig->u[dim] - ig->l[dim]) % ig->step[dim];
                if (cut == 0) {
                    lbuf += ig->step[dim] - ig->width[dim];
                } else if (cut > ig->width[dim]) {
                    lbuf += cut - ig->width[dim];
                } else if (cut < ig->width[dim]) {
                    /* make newig */
                    newig = WLFcopyInternGen (ig);
                    newig->l[dim] = ig->l[dim] + cut;
                    newig->width[dim] = ig->width[dim] - cut;
                    ig->width[dim] = cut;
                    /* if u - l is smaller than width, the newig is empty */
                    if (newig->u[dim] <= newig->l[dim]) {
                        newig = WLFfreeInternGen (newig);
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

    case F_mul_SxS:
        /*
         * first try  (srs):  ig->l[dim] = (ig->l[dim] + constval - 1) / constval;
         *                    ig->u[dim] = (ig->u[dim] - 1) / constval +1;
         *
         * second try (sbs):  ig->l[dim] = (ig->l[dim] + constval - 1) / constval;
         *                    ig->u[dim] = (ig->u[dim] + constval - 1) / constval;
         *
         * But this is still incorrect in some cases, e.g. if (constval < 0) ...
         *
         * These conditions must hold for l_neu and u_neu:
         *
         *   constval > 0:      l  >  constval * ( l_neu - 1 )
         *                  &&  l  <= constval * l_neu
         *                  &&  u  >  constval * ( u_neu - 1 )
         *                  &&  u  <= constval * u_neu
         *
         *             ==>      l / constvec  <=  l_neu  <  l / constvec + 1
         *                  &&  u / constvec  <=  u_neu  <  u / constvec + 1
         *
         *   constval < 0:      u  >  constval * l_neu           // bounds reflected, ...
         *                  &&  u  <= constval * ( l_neu - 1 )   // ... <= and < reversed
         *                  &&  l  >  constval * u_neu
         *                  &&  l  <= constval * ( u_neu - 1 )
         *
         *             ==>      u / constvec  <  l_neu  <=  u / constval + 1
         *                  &&  l / constvec  <  u_neu  <=  l / constval + 1
         *
         * Therefore the following code should work correctly:
         */
        if (constval > 0) {
            ig->l[dim] = ((ig->l[dim] % constval == 0) || (ig->l[dim] < 0))
                           ? (ig->l[dim] / constval)
                           : (ig->l[dim] / constval + 1);
            ig->u[dim] = ((ig->u[dim] % constval == 0) || (ig->u[dim] < 0))
                           ? (ig->u[dim] / constval)
                           : (ig->u[dim] / constval + 1);
        }
        if (constval < 0) {
            ig->l[dim] = ((ig->u[dim] % constval == 0) || (ig->u[dim] < 0))
                           ? (ig->u[dim] / constval + 1)
                           : (ig->u[dim] / constval);
            ig->u[dim] = ((ig->l[dim] % constval == 0) || (ig->l[dim] < 0))
                           ? (ig->l[dim] / constval + 1)
                           : (ig->l[dim] / constval);
        }

        if (ig->step) {
            DBUG_UNREACHABLE ("WL folding with transformed index variables "
                              "by multiplication and grids not supported right now.");
        }
        break;

    case F_div_SxS:
        DBUG_ASSERT (arg_no == 2,
                     "WLF transformation (scalar / index) not yet implemented!");

        /*
         * first try  (srs):  ig->l[dim] = ig->l[dim] * constval;
         *                    ig->u[dim] = ig->u[dim] * constval;
         *
         * But this is incorrect in some cases, e.g. if (constval < 0) ...
         *
         * These conditions must hold for l_neu and u_neu:
         *
         *   constval > 0:      l  >  ( l_neu - 1 ) / constval
         *                  &&  l  <= l_neu / constval
         *                  &&  u  >  ( u_neu - 1 ) / constval
         *                  &&  u  <= u_neu / constval
         *
         *             ==>      l * constvec  <=  l_neu  <  l * constvec + 1
         *                  &&  u * constvec  <=  u_neu  <  u * constvec + 1
         *
         *   constval < 0:      u  >  l_neu / constval           // bounds reflected, ...
         *                  &&  u  <= ( l_neu - 1 ) / constval   // ... <= and < reversed
         *                  &&  l  >  u_neu / constval
         *                  &&  l  <= ( u_neu - 1 ) / constval
         *
         *             ==>      u * constvec  <  l_neu  <=  u * constval + 1
         *                  &&  l * constvec  <  u_neu  <=  l * constval + 1
         *
         * Therefore the following code should work correctly:
         */
        if (constval > 0) {
            ig->l[dim] = ig->l[dim] * constval;
            ig->u[dim] = ig->u[dim] * constval;
        }
        if (constval < 0) {
            ig->l[dim] = ig->u[dim] * constval + 1;
            ig->u[dim] = ig->l[dim] * constval + 1;
        }

        if (ig->step) {
            DBUG_UNREACHABLE ("WL folding with transformed index variables "
                              "by division and grids not supported right now.");
        }
        break;

    default:
        DBUG_UNREACHABLE ("Wrong transformation function");
    }

    DBUG_RETURN (newig);
}

/******************************************************************************
 *
 * function:
 *   void LinearTransformationsScalar( intern_gen *ig,
 *                                     index_info *transformations, int dim)
 *
 * description:
 *   like LinearTransformationsVector(), but only transforms one dimension of ig.
 *
 *
 ******************************************************************************/

static intern_gen *
LinearTransformationsScalar (intern_gen *ig, index_info *transformations, int dim)
{
    intern_gen *actig, *newig;

    DBUG_ENTER ();
    DBUG_ASSERT (0 == transformations->vector, "wrong parameters");
    DBUG_ASSERT (!transformations->last[0] || !transformations->last[0]->vector,
                 "scalar points to vector");
    DBUG_ASSERT (transformations->permutation[0], "scalar constant???");

    actig = ig;
    if (transformations->arg_no)
        /* valid prf. */
        while (actig) { /* all igs */
            newig = LinearTransformationsHelp (actig, dim, transformations->mprf,
                                               transformations->arg_no,
                                               transformations->const_arg[0]);

            if (newig) {
                newig->next = ig; /* insert new element before whole list. */
                ig = newig;       /* set new root */
            }
            actig = actig->next;
        }

    if (transformations->last[0]) {
        ig = LinearTransformationsScalar (ig, transformations->last[0], dim);
    }

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   void LinearTransformationsVector(intern_gen *ig,
 *                                    index_info *transformations)
 *
 * description:
 *   realizes transformations on the given list of intern gens.
 *
 *
 ******************************************************************************/

static intern_gen *
LinearTransformationsVector (intern_gen *ig, index_info *transformations)
{
    int dim, act_dim;
    intern_gen *actig, *newig;

    DBUG_ENTER ();
    DBUG_ASSERT (transformations->vector == ig->shape,
                 "Transformations do not fit to generators");

    dim = ig->shape;

    if (transformations->vector && transformations->arg_no) {
        /* vector transformation and a valid prf. */
        for (act_dim = 0; act_dim < dim; act_dim++) { /* all dimensions */
            actig = ig;
            if (transformations->permutation[act_dim]) {
                while (actig) { /* all igs */
                    newig
                      = LinearTransformationsHelp (actig, act_dim, transformations->mprf,
                                                   transformations->arg_no,
                                                   transformations->const_arg[act_dim]);
                    if (newig) {
                        newig->next = ig; /* insert new element before whole list. */
                        ig = newig;       /* set new root */
                    }
                    actig = actig->next;
                }
            }
        }
    }

    if (transformations->last[0] && transformations->last[0]->vector) {
        ig = LinearTransformationsVector (ig, transformations->last[0]);
    } else { /* maybe additional scalar transformations */
        for (act_dim = 0; act_dim < dim; act_dim++) {
            if (transformations->last[act_dim]) {
                ig = LinearTransformationsScalar (ig, transformations->last[act_dim],
                                                  act_dim);
            }
        }
    }

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *FinalTransformations( intern_gen *ig,
 *                                     index_info *transformations,
 *                                     int target_dim);
 *
 * description:
 *   transforms list of ig into cuboids of dimension target_dim.
 *
 *
 ******************************************************************************/

static intern_gen *
FinalTransformations (intern_gen *substig, index_info *transformations, int target_dim)
{
    intern_gen *tmpig, *newig, *rootig;
    int ok, i,*help;
    size_t j;

    DBUG_ENTER ();
    DBUG_ASSERT (transformations->vector == substig->shape, "wrong parameters");

    /* create array to speed up later transformations.
       help[i] is
       - 0 if the ith index scalar is not used to index the subst array
       - d if the ith index scalar addresses the dth component of the subst array.

       Example:
       iv=[i,j,k]
       subst array A[42,i,k]
       help: [2,0,3] */
    j = sizeof (int) * target_dim;
    help = (int *)MEMmalloc (j);
    help = (int *)memset (help, 0, j);
    for (i = 0; i < transformations->vector; i++) {
        if (transformations->permutation[i]) {
            help[transformations->permutation[i] - 1] = i + 1;
        }
    }

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
            newig = WLFcreateInternGen (target_dim, NULL != tmpig->step);
            for (i = 0; i < target_dim; i++) {
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
            }
            DBUG_ASSERT (0 == WLFnormalizeInternGen (newig),
                         "Error while normalizing ig");

            newig->code = tmpig->code;
            newig->next = rootig;
            rootig = newig;
        }

        /* go to next substig */
        tmpig = tmpig->next;
    }

    help = MEMfree (help);
    WLFfreeInternGenChain (substig);

    DBUG_RETURN (rootig);
}

/******************************************************************************
 *
 * function:
 *   node *CreateCode(node *target, node *subst)
 *
 * description:
 *   substitutes the code block subst into target. INFO_ID points
 *   to the N_id node in the target WL which shall be replaced.
 *   New vardecs and assignments are introduced and an N_code node is returned.
 *   Most of the work is done in the usual traversal steps (with mode
 *   wlfm_replace and wlfm_rename).
 *
 ******************************************************************************/

static node *
CreateCode (node *target, node *subst)
{
    node *coden;
    info *new_arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (N_code == NODE_TYPE (target), "wrong target Parameter");
    DBUG_ASSERT (N_code == NODE_TYPE (subst), "wrong subst Parameter");

    DBUG_PRINT ("Creating code now...");
    DBUG_EXECUTE_TAG ("WLF_CC", fprintf (stderr, " target code:");
                                PRTdoPrintFile (stderr, target);
                                fprintf (stderr, " subst code:");
                                PRTdoPrintFile (stderr, subst); );
    wlf_mode = wlfm_replace;

    /* create new arg_info to avoid modifications of the old one and copy
       all relevant information */
    new_arg_info = MakeInfo ();
    INFO_FUNDEF (new_arg_info) = INFO_FUNDEF (ref_mode_arg_info);
    INFO_ID (new_arg_info) = INFO_ID (ref_mode_arg_info);
    INFO_NCA (new_arg_info) = INFO_NCA (ref_mode_arg_info);
    INFO_SUBST (new_arg_info) = subst;
    INFO_NEW_ID (new_arg_info) = NULL;

    /* WHY is this copied? */
    INFO_MYSTERY (new_arg_info) = INFO_MYSTERY (ref_mode_arg_info);

    /*
     * DUPdoDupTree() shall fill ID_WL of Id nodes with special information.
     * So we have to call DUPdoDupTree() with DUP_WLF.
     */
    coden = DUPdoDupTreeType (CODE_CBLOCK (target), DUP_WLF);
    coden = TRAVdo (coden, new_arg_info);
    coden = TBmakeCode (coden, DUPdoDupTreeType (CODE_CEXPRS (target), DUP_WLF));

    new_arg_info = FreeInfo (new_arg_info);

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

static void
IntersectGrids (int dim)
{
    int dc, first, last, d;
    intern_gen *ig;
    code_constr_type *cc;
    node *coden;

    DBUG_ENTER ();

    dc = 0;

    while (dc < intersect_grids_baseig->step[dim]) {
        /* search common dc */
        if (((!intersect_grids_tig->step)
             || ((dc + intersect_grids_ot[dim]) % intersect_grids_tig->step[dim]
                 < intersect_grids_tig->width[dim]))
            && ((!intersect_grids_sig->step)
                || ((dc + intersect_grids_os[dim]) % intersect_grids_sig->step[dim]
                    < intersect_grids_sig->width[dim]))) {
            first = dc;
            /* search first dc where either in tig or sig the element is
               not present. */
            do {
                dc++;
            } while (
              ((!intersect_grids_tig->step)
               || ((dc + intersect_grids_ot[dim]) % intersect_grids_tig->step[dim]
                   < intersect_grids_tig->width[dim]))
              && ((!intersect_grids_sig->step)
                  || ((dc + intersect_grids_os[dim]) % intersect_grids_sig->step[dim]
                      < intersect_grids_sig->width[dim]))
              && (dc < intersect_grids_baseig->step[dim]));
            last = dc;

            if (dim < intersect_grids_baseig->shape - 1) {
                /* process inner dimensions if lower bound is less then upper bound. */
                if (intersect_grids_baseig->l[dim] + first
                    < intersect_grids_baseig->u[dim]) {
                    intersect_grids_baseig->l[dim] += first;
                    intersect_grids_baseig->width[dim] = last - first;
                    IntersectGrids (dim + 1);
                    intersect_grids_baseig->l[dim] -= first; /* restore old value */
                } else {
                    dc = intersect_grids_baseig->step[dim]; /* stop further search */
                }
            } else {
                /* create new ig and add it to structures
                   if lower bound is less then upper bound. */
                if (intersect_grids_baseig->l[dim] + first
                    < intersect_grids_baseig->u[dim]) {
                    ig = WLFcreateInternGen (intersect_grids_baseig->shape, 1);
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
                    if (cc) {
                        ig->code = cc->mnew;
                    } else {
                        coden = CreateCode (intersect_grids_tig->code,
                                            intersect_grids_sig->code);
                        ig->code = coden;
                        AddCC (intersect_grids_tig->code, intersect_grids_sig->code,
                               coden);
                        CODE_NEXT (coden) = new_codes;
                        new_codes = coden;
                    }

                    /* add new generator to intersect_intern_gen list. */
                    ig->next = intersect_intern_gen;
                    intersect_intern_gen = ig;
                } else {
                    dc = intersect_grids_baseig->step[dim]; /* stop further search */
                }
            }
        }

        dc++;
    }

    DBUG_RETURN ();
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

static intern_gen *
IntersectInternGen (intern_gen *target_ig, intern_gen *subst_ig)
{
    intern_gen *sig, *new_gen, *new_gen_step, *new_gen_nostep;
    int d, max_dim, l, u, create_steps;
    code_constr_type *cc;
    node *coden;

    DBUG_ENTER ();
    DBUG_ASSERT (target_ig->shape == subst_ig->shape, "wrong parameters");

    DBUG_EXECUTE_TAG ("WLF_IGEN", fprintf (stderr, "target_ig: ");
                                  WLFprintfInternGen (stderr, target_ig, true, true);
                                  fprintf (stderr, "\n\n subst_ig: ");
                                  WLFprintfInternGen (stderr, subst_ig, true, true); );
    max_dim = target_ig->shape;
    new_gen_step = NULL;
    new_gen_nostep = NULL;
    intersect_intern_gen = NULL;

    while (target_ig) {
        sig = subst_ig;
        while (sig) {
            if (!new_gen_step) {
                new_gen_step = WLFcreateInternGen (target_ig->shape, 1);
            }
            if (!new_gen_nostep) {
                new_gen_nostep = WLFcreateInternGen (target_ig->shape, 0);
            }

            create_steps = target_ig->step || sig->step;
            if (create_steps) {
                new_gen = new_gen_step;
                new_gen_step = NULL;
            } else {
                new_gen = new_gen_nostep;
                new_gen_nostep = NULL;
            }

            /* here we have to intersect target_ig and sig */
            DBUG_EXECUTE_TAG ("WLF_IGEN", fprintf (stderr, "\ntrying to intersect ");
                                          WLFprintfInternGen (stderr, target_ig, false, false);
                                          fprintf (stderr, " with ");
                                          WLFprintfInternGen (stderr, sig, false, false);
                                          fprintf (stderr, "\n"); );
            /* first the bounds, ignore step/width */
            for (d = 0; d < max_dim; d++) {
                l = MATHmax (target_ig->l[d], sig->l[d]);
                u = MATHmin (target_ig->u[d], sig->u[d]);
                if (l >= u) {
                    DBUG_EXECUTE_TAG ("WLF_IGEN", fprintf (stderr, " empty!\n"); );
                    break; /* empty intersection */
                } else {
                    new_gen->l[d] = l;
                    new_gen->u[d] = u;
                }
            }

            if (d == max_dim) {
                /* we have a new ig. Maybe. Because if both generators are
                   grids they may still be disjuct. */
                DBUG_EXECUTE_TAG ("WLF_IGEN", fprintf (stderr, " possible intersection!\n"); );
                if (create_steps) {
                    /* prepare parameters for recursive function IntersectGrids().
                       Global pointers are used to speed up function call. */
                    /* new_gen will be the reference structure for new igs
                       created within IntersectGrids(). */
                    if (!target_ig->step) {
                        for (d = 0; d < max_dim; d++) {
                            new_gen->step[d] = sig->step[d];
                        }
                    } else if (!sig->step) {
                        for (d = 0; d < max_dim; d++) {
                            new_gen->step[d] = target_ig->step[d];
                        }
                    } else {
                        for (d = 0; d < max_dim; d++) {
                            new_gen->step[d] = MATHlcm (target_ig->step[d], sig->step[d]);
                        }
                    }

                    /* compute offsets */
                    if (target_ig->step) {
                        for (d = 0; d < max_dim; d++) {
                            intersect_grids_ot[d]
                              = (new_gen->l[d] - target_ig->l[d]) % target_ig->step[d];
                        }
                    }
                    if (sig->step) {
                        for (d = 0; d < max_dim; d++) {
                            intersect_grids_os[d]
                              = (new_gen->l[d] - sig->l[d]) % sig->step[d];
                        }
                    }

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
                    if (cc) {
                        new_gen->code = cc->mnew;
                    } else {
                        coden = CreateCode (target_ig->code, sig->code);
                        new_gen->code = coden;
                        AddCC (target_ig->code, sig->code, coden);
                        CODE_NEXT (coden) = new_codes;
                        new_codes = coden;
                    }
                    DBUG_EXECUTE_TAG ("WLF_IGEN", fprintf (stderr, " new generator:");
                                                  WLFprintfInternGen (stderr, new_gen, true, true); );

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

    if (new_gen_step) {
        new_gen_step = WLFfreeInternGen (new_gen_step);
    }
    if (new_gen_nostep) {
        new_gen_nostep = WLFfreeInternGen (new_gen_nostep);
    }

    DBUG_RETURN (intersect_intern_gen);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *RemoveDoubleIndexVectors( intern_gen *subst_ig,
 *                                         index_info *transformations)
 *
 * description:
 *   only used if permutations of index scalar variables are enabled.
 *   This is a transformation to reduce the number of components of the ig.
 *
 ******************************************************************************/

static intern_gen *
RemoveDoubleIndexVectors (intern_gen *subst_ig, index_info *transformations)
{
    int *found, i, act_dim, dim, fdim;
    size_t j;
    intern_gen *act_ig;

    DBUG_ENTER ();

    j = sizeof (int) * SHP_SEG_SIZE; /* max number of dimensions */
    found = (int *)MEMmalloc (j);
    found = (int *)memset (found, 0, j);

    for (act_dim = 0; act_dim < transformations->vector; act_dim++)
        if (transformations->permutation[act_dim] != 0) { /* ==0: constant */
            dim = transformations->permutation[act_dim] - 1;
            if (found[dim] != 0) {
                /* dimensions found[dim] and act_dim of subst_ig both are based on
                   the dim-th index scalar. */
                act_ig = subst_ig;
                fdim = found[dim] - 1;
                transformations->vector--;
                while (act_ig) {
                    /* fold both dimensions fdim and act_dim to fdim ... */
                    act_ig->l[fdim] = MATHmax (act_ig->l[fdim], act_ig->l[act_dim]);
                    act_ig->u[fdim] = MATHmin (act_ig->u[fdim], act_ig->u[act_dim]);
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
            } else {
                found[dim] = act_dim + 1;
            }
        }

    found = MEMfree (found);

    DBUG_RETURN (subst_ig);
}

/******************************************************************************
 *
 * function:
 *   TransformationRangeCheck( )
 *
 * description:
 *    if the sel-operation we process right now indexes the subst WL in a way
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

static int
TransformationRangeCheck (index_info *transformations, node *substwln,
                          intern_gen *target_ig)
{
    int result, dim, i;
    shape *s;
    intern_gen *whole_ig;
    ntype *shtype;
    int *shvec;

    DBUG_ENTER ();

    /* create bounds of substwln in whole_ig */
    dim = transformations->vector;
    whole_ig = WLFcreateInternGen (dim, 0);
    switch (NODE_TYPE (WITH_WITHOP (substwln))) {
    case N_genarray:
        shtype = NTCnewTypeCheck_Expr (GENARRAY_SHAPE (WITH_WITHOP (substwln)));
        DBUG_ASSERT (TYisAKV (shtype), "WL shape description must be constant!");
        shvec = (int *)COgetDataVec (TYgetValue (shtype));
        for (i = 0; i < dim; i++) {
            whole_ig->l[i] = 0;
            whole_ig->u[i] = shvec[i];
        }
        shtype = TYfreeType (shtype);
        break;

    case N_modarray:
        s = TYgetShape (ID_NTYPE (MODARRAY_ARRAY (WITH_WITHOP (substwln))));
        for (i = 0; i < dim; i++) {
            whole_ig->l[i] = 0;
            whole_ig->u[i] = SHgetExtent (s, i);
        }
        break;

    default:
        DBUG_UNREACHABLE ("TransformationRangeCheck called with fold-op");
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
        DBUG_ASSERT ((dim - 1 < 0)
                       || (target_ig->l && target_ig->u && whole_ig->l && whole_ig->u),
                     "OOOOOOOPS: I seem to be missing something here!");

        for (i = dim - 1; i >= 0; i--) {
            if (target_ig->l[i] < whole_ig->l[i] || target_ig->u[i] > whole_ig->u[i]) {
                result = i + 1;
            }
        }

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
 ******************************************************************************/

static void
Fold (node *idn, index_info *transformations, node *targetwln, node *substwln)
{
    intern_gen *target_ig; /* transformed igs of target WL */
    intern_gen *subst_ig;  /* transformed igs of subst WL */
    intern_gen *tmpig, *intersect_ig;
    int error;
    index_info *transf2;

    DBUG_ENTER ();

    code_constr = NULL;

    /* create target_ig */
    target_ig = new_ig;
    new_ig = NULL;

    /* the 'old' new_ig is not needed anymore because we create a new list of
       intern gens which represents the same set of index elements as the
       'old' list. */
    new_ig = WLFfreeInternGenChain (new_ig);

    /* check if array access is in range. Don't use the original *transformations
       because RemoveDoubleIndexVectors() modifies it.  */
    DBUG_PRINT ("  ...starting transformations...");
    transf2 = WLFduplicateIndexInfo (transformations);
    error = TransformationRangeCheck (transf2, substwln, target_ig);
    transf2 = FREEfreeIndexInfo (transf2);
    if (error) {
        CTIabort (NODE_LOCATION (idn), "Array access to %s out of range in dimension %i",
                      ID_NAME (idn), error);
    }

    /* create subst_ig */
    subst_ig = WLFtree2InternGen (substwln, NULL);
    subst_ig = LinearTransformationsVector (subst_ig, transformations);
    /* We can use the original *transformations here because we don't need it
       anymore. */
#ifndef TRANSF_TRUE_PERMUTATIONS
    subst_ig = RemoveDoubleIndexVectors (subst_ig, transformations);
#endif
    subst_ig = FinalTransformations (subst_ig, transformations, target_ig->shape);

    if (!subst_ig) {
        CTIabort (NODE_LOCATION (idn), "Constants of index vector out of range");
    }

    /* intersect target_ig and subst_ig
       and create new code blocks */
    DBUG_PRINT ("  ...done. Intersecting generators now...");
    intersect_ig = IntersectInternGen (target_ig, subst_ig);
    DBUG_PRINT ("  ...done...");

    /* results are in intersect_ig. At the moment, just append to new_ig. */
#if 0
  /*
   * Is this assertion actually required???
   * See bug #124 for a program that does not compile with this assertion.
   */
  DBUG_ASSERT (intersect_ig, "No new intersections");
#endif
    tmpig = new_ig;
    if (!tmpig) { /* at the mom: always true */
        new_ig = intersect_ig;
    } else {
        while (tmpig->next) {
            tmpig = tmpig->next;
        }
        tmpig->next = intersect_ig;
    }

    /* free all created lists */
    WLFfreeInternGenChain (target_ig);
    WLFfreeInternGenChain (subst_ig);
    FreeCC (code_constr);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   int FoldDecision(node *target_wl, node* subst_wl)
 *
 * description:
 *   decides whether subst_wl shall be folded into target_wl or not.
 *
 *   All references to subst_wl must lie within the target_wl.
 *   This avoids duplicating the work of creating subst_wl
 *   result elements.
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

static int
FoldDecision (node *target_wl, node *subst_wl)
{
    int result;
    ntype *shtype;

    DBUG_ENTER ();

    subst_wl = LET_EXPR (ASSIGN_STMT (subst_wl));

    result = (!TCcontainsDefaultPartition (WITH_PART (target_wl))
              && !TCcontainsDefaultPartition (WITH_PART (subst_wl))
              && WITH_ISFOLDABLE (target_wl)
              && WITH_REFERENCED (subst_wl) == WITH_REFERENCED_FOLD (subst_wl)
              && (N_genarray == NODE_TYPE (WITH_WITHOP (subst_wl))
                  || N_modarray == NODE_TYPE (WITH_WITHOP (subst_wl))));

    if (N_genarray == NODE_TYPE (WITH_WITHOP (subst_wl))) {
        shtype = NTCnewTypeCheck_Expr (GENARRAY_SHAPE (WITH_WITHOP (subst_wl)));
        result = result && TYisAKV (shtype);
        shtype = TYfreeType (shtype);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *CheckForSuperfluousCodes(node *wln)
 *
 * description:
 *   remove all unused N_code nodes of the given WL.
 *
 *
 ******************************************************************************/

static node *
CheckForSuperfluousCodes (node *wln)
{
    node **tmp;

    DBUG_ENTER ();

    tmp = &WITH_CODE (wln);
    while (*tmp) {
        if (!CODE_USED ((*tmp))) {
            *tmp = FREEdoFreeNode (*tmp);
        } else {
            tmp = &CODE_NEXT ((*tmp));
        }
    }

    DBUG_RETURN (wln);
}

/******************************************************************************
 *
 * function:
 *   node *Modarray2Genarray( node *withop, node *wln, node *substwln)
 *
 * description:
 *   transforms the withops of the given WL. The goal is remove a reference
 *   to a WL which can be folded so that DCR can eleminate the WL.
 *
 * example:
 *   B = WL() genarray([shape],...);
 *   C = WL() modarray(B,...);         =>  C = WL() genarray([newshape],...)
 *     where newshape depends on shape and the given index vector of C.
 *
 ******************************************************************************/
static node *
Modarray2Genarray (node *withop, node *wln, node *substwln)
{
    DBUG_ENTER ();

    DBUG_ASSERT (substwln, "substwln is NULL");
    DBUG_ASSERT (withop != NULL, "withop is NULL");

    /*
     * Replace all further modarrays recursively
     */
    if (WITHOP_NEXT (withop) != NULL) {
        L_WITHOP_NEXT (withop, Modarray2Genarray (WITHOP_NEXT (withop), wln, substwln));
    }

    if ((NODE_TYPE (withop) == N_modarray)
        && (AVIS_SSAASSIGN (ID_AVIS (MODARRAY_ARRAY (withop))) == substwln)) {
        node *shparray, *nextop;
        shape *vecshape, *arrayshape, *prefix;

        /*
         * at the moment, substwln points to the assignment of the WL.
         * remove another reference
         */
        substwln = LET_EXPR (ASSIGN_STMT (substwln));
        (WITH_REFERENCES_FOLDED (substwln))++;

        /*
         * compute shape of WL for NWITHOP_SHAPE()
         */
        vecshape = TYgetShape (IDS_NTYPE (WITH_VEC (wln)));
        arrayshape = TYgetShape (ID_NTYPE (MODARRAY_ARRAY (withop)));

        prefix = SHtakeFromShape (SHgetUnrLen (vecshape), arrayshape);
        shparray = SHshape2Array (prefix);

        prefix = SHfreeShape (prefix);

        /*
         * delete old withop and create new one
         */
        nextop = FREEdoFreeNode (withop);
        withop = TBmakeGenarray (shparray, NULL);
        L_WITHOP_NEXT (withop, nextop);
    }

    DBUG_RETURN (withop);
}

/** <!-- ****************************************************************** -->
 * @fn node *FreeWLIAssignInfo( node *arg_node, info *arg_info)
 *
 * @brief Frees the ASSIGN_INDEX attribute of an N_assign node.
 *
 * @param arg_node N_assign node
 * @param arg_info unused
 *
 * @return modified N_assign node
 ******************************************************************************/
static node *
FreeWLIAssignInfo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (ASSIGN_INDEX (arg_node) != NULL) {
        ASSIGN_INDEX (arg_node) = FREEfreeIndexInfo (ASSIGN_INDEX (arg_node));
    }

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *FreeWLIInformation( node *fundef)
 *
 * @brief Clears the WLI information from all N_assign nodes within a
 *        function.
 *
 * @param fundef N_fundef node of function to be cleared.
 *
 * @return modified N_fundef node.
 ******************************************************************************/
static node *
FreeWLIInformation (node *fundef)
{
    anontrav_t freetrav[2] = {{N_assign, &FreeWLIAssignInfo}, {(nodetype)0, NULL}};

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "FreeWLIInformation called with non-fundef node");

    TRAVpushAnonymous (freetrav, &TRAVsons);
    FUNDEF_BODY (fundef) = TRAVopt (FUNDEF_BODY (fundef), NULL);
    TRAVpop ();

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Functions for node_info.mac (traversal mechanism)
 *
 ******************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *WLFmodule(node *arg_node, info *arg_info)
 *
 * @brief Traverses only functions of the module, skipping all the rest for
 *        performance reasons.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLFfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   initializations to arg_info and traversal of the fundef body.
 *
 ******************************************************************************/
node *
WLFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("entering %s for WLF", FUNDEF_NAME (arg_node));
    INFO_WL (arg_info) = NULL;
    INFO_FUNDEF (arg_info) = arg_node;

    wlf_mode = wlfm_search_WL;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFassign( node *arg_node, info *arg_info)
 *
 * description:
 *   set arg_info to remember this N_assign node.
 *
 ******************************************************************************/
node *
WLFassign (node *arg_node, info *arg_info)
{
    node *tmpn, *last_assign, *substn;

    DBUG_ENTER ();

    INFO_ASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    switch (wlf_mode) {
    case wlfm_search_WL:
        if (ASSIGN_NEXT (arg_node)) {
            /* We must not assign the returned node to ASSIGN_NEXT
               because the chain has been modified in TRAVdo(wlfm_rename).
               This is not very nice and bypassed the traversal mechanism,
               but it is necessary since cross pointers are used as references
               to N_assign nodes and new code is inserted there. */
            TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
        break;

    case wlfm_replace:
        if (INFO_NEW_ID (arg_info)) {
            /* paste the new code (subst WL code) into the assign chain (target WL
               code). The current assign node is appended to the new INFO_SUBST
               chain. Therefor, reuse the current assign node because the
               previous assign node (which cannot be reached from here) points
               to it. */

            substn = INFO_SUBST (arg_info);
            /* ist there something to insert? */
            if (substn) {
                last_assign
                  = TBmakeAssign (ASSIGN_STMT (arg_node), ASSIGN_NEXT (arg_node));
                ASSIGN_STMT (arg_node) = ASSIGN_STMT (substn);
                ASSIGN_NEXT (arg_node) = ASSIGN_NEXT (substn);

                /* first assignment node of substn is not needed anymore. */
                ASSIGN_STMT (substn) = NULL;
                FREEdoFreeNode (substn); /* free only node, not tree! */

                tmpn = arg_node;
                while (ASSIGN_NEXT (tmpn)) {
                    tmpn = ASSIGN_NEXT (tmpn);
                }
                ASSIGN_NEXT (tmpn) = last_assign;
                tmpn = ASSIGN_NEXT (tmpn);
            } else {
                tmpn = arg_node;
            }

            /* transform sel(.,array) into Id. */
            tmpn = ASSIGN_STMT (tmpn);
            LET_EXPR (tmpn) = FREEdoFreeNode (LET_EXPR (tmpn));
            LET_EXPR (tmpn) = INFO_NEW_ID (arg_info);
            INFO_NEW_ID (arg_info) = NULL;
        } else {
            ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);
        }
        break;

    default:
        /* Do not assign ASSIGN_NEXT(arg_node) here!
           Why? See comment in wlfm_search_wl branch of this function. */
        ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLFid(node *arg_node, info *arg_info)
 *
 * description:
 *  In wlfm_replace phase insert variable definitions of index vector
 *  and index scalars. Remove sel operation and replace it with the
 *  body of the subst WL.
 *  In wlfm_rename phase detect name clashes and solve them.
 *
 ******************************************************************************/
node *
WLFid (node *arg_node, info *arg_info)
{
    node *substn, *coden, *vectorn, *argsn, *letn, *subst_wl_partn, *subst_header;
    node *old_arg_info_assign, *arrayn;
    node *subst_wl_ids, *_ids;
    int count;

    DBUG_ENTER ();

    switch (wlf_mode) {
    case wlfm_search_WL:
        /* check if we want to fold this Id.
           If no, clear ID_WL. This is the sign for the following phases to fold.
           If yes, set INFO_FLAG. This is only a speed-up flag. If it is 0,
           we do not enter the wlfm_search_ref phase.
           We used ID_WL in case (1) here (see header of file) */
        if (ID_WL (arg_node)) {       /* a WL is referenced (this is an array Id), */
            if (INFO_WL (arg_info) && /* inside WL, */
                ASSIGN_INDEX (INFO_ASSIGN (arg_info)) && /* valid index transformation */
                FoldDecision (INFO_WL (arg_info), ID_WL (arg_node))) {
                INFO_FLAG (arg_info) = 1;
            } else {
                ID_WL (arg_node) = NULL;
            }
        }
        break;

    case wlfm_search_ref:
        /* conditional of N_do, N_while or N_cond. Do nothing. */
        break;

    case wlfm_replace:
        /* This ID_WL is used in case (2) here (see header of file) */
        if (ID_WL (arg_node) == INFO_ID (arg_info)) {
            /* this is the Id which has to be replaced. */
            /* First store the C_EXPR in INFO_NEW_ID. Usage: see WLFassign.
               It might happen that this C_EXPR in the substn will be renamed when
               loops surround the WL. Then we have to rename the C_EXPR, too. */
            coden = INFO_SUBST (arg_info);

            /* keep original name */
            /* This is the name of the var in CODE_CEXPR (coden) whose position
               corresponds to the position of the N_id arg_node in the LHS of
               the assignment where it is defined (ID_WL here points to the N_id). */
            INFO_NEW_ID (arg_info)
                = DUPdoDupTree (FindCExpr (CODE_CEXPRS (coden),
                                           ID_AVIS (arg_node),
                                           AVIS_SSAASSIGN (ID_AVIS (ID_WL (arg_node)))));

            /* Create substitution code. */
            substn = DUPdoDupTree (CODE_CBLOCK_ASSIGNS (coden));

            /* create assignments to rename variables which index the array we want
               to replace.
               Example: We want to replace sel(sel,A) and A is a WL with a
               generator iv=[i,j,k]. Then we add:
                 i = sel([0],sel);
                 j = sel([1],sel);
                 k = sel([2],sel);
                 iv = sel;
               Remember that iv,i,j,k all are temporary variables, inserted
               in flatten. No name clashes can happen. */
            subst_header = NULL;
            vectorn = PRF_ARG1 (LET_EXPR (ASSIGN_STMT (INFO_ASSIGN (arg_info))));
            count = 0;
            /* This ID_WL is used in case (1) here (see header of file) */
            subst_wl_partn = WITH_PART (ASSIGN_RHS (ID_WL (INFO_ID (arg_info))));
            subst_wl_ids = PART_IDS (subst_wl_partn);
            while (subst_wl_ids) {
                node *arrayavisn;
                /* Here we use masks which have been generated before WLI. These masks
                   are never modified in WLI and WLF (although new VARDECs are inserted),
                   so we can use them for variables which already existed BEFORE WLI.
                   Now let's see if we can use masks for index variables:
                   - yes, if this is no newly created code block (WLF)
                   - no otherwise. We could duplicate masks for new code blocks,
                     but I guess that would cost more time than speculatively inserting
                     (and DC-removing) some new variables. */

                arrayn = TCmakeIntVector (TBmakeExprs (TBmakeNum (count), NULL));
                arrayavisn = TBmakeAvis (TRAVtmpVar (), NTCnewTypeCheck_Expr (arrayn));
                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (arrayavisn, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

                argsn = TBmakeExprs (TBmakeId (arrayavisn),
                                     TBmakeExprs (DUPdoDupTree (vectorn), NULL));

                /* keep original name */
                _ids = DUPdoDupNode (subst_wl_ids);

                letn = TBmakeLet (_ids, TBmakePrf (F_sel_VxA, argsn));
                subst_header = TBmakeAssign (letn, subst_header);

                letn = TBmakeLet (TBmakeIds (arrayavisn, NULL), arrayn);
                subst_header = TBmakeAssign (letn, subst_header);

                count++;
                subst_wl_ids = IDS_NEXT (subst_wl_ids);
            }

            /* now add "iv = sel" (see example above) */
            subst_wl_ids = PART_VEC (subst_wl_partn);

            _ids = DUPdoDupNode (subst_wl_ids);

            letn = TBmakeLet (_ids, DUPdoDupTree (vectorn));
            subst_header = TBmakeAssign (letn, subst_header);

            /* trav subst code with wlfm_rename to solve name clashes. */
            if (substn) {
                wlf_mode = wlfm_rename;
                old_arg_info_assign = INFO_ASSIGN (arg_info); /* save arg_info */
                substn = TRAVdo (substn, arg_info);
                INFO_ASSIGN (arg_info) = old_arg_info_assign;
                wlf_mode = wlfm_replace;
            }

            /* merge subst_header and substn */
            /* we don't need the old _SUBST info anymore, so we return the
               new subst assign chain here. WLFassign uses this information to
               melt both chains. */
            if (subst_header) {
                INFO_SUBST (arg_info) = subst_header;
                while (ASSIGN_NEXT (subst_header)) {
                    subst_header = ASSIGN_NEXT (subst_header);
                }
                ASSIGN_NEXT (subst_header) = substn;
            } else {
                INFO_SUBST (arg_info) = substn;
            }
        }
        break;

    case wlfm_rename:
        break;

    default:
        DBUG_UNREACHABLE ("Not expected");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFlet(node *arg_node, info *arg_info)
 *
 * description:
 *   If in wlfm_search_ref mode, initiate folding.
 *   In wlfm_rename phase detect name clashes and solve them.
 *
 ******************************************************************************/
node *
WLFlet (node *arg_node, info *arg_info)
{
    node *prfn, *idn, *targetwln, *substwln;
    index_info *transformation;

    DBUG_ENTER ();

    switch (wlf_mode) {
    case wlfm_rename:
    case wlfm_search_WL:
    case wlfm_replace:
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
        break;

    case wlfm_search_ref:
        /* is this a prf sel() which can be folded? */
        /* All occurences of ID_WL are used in case (1) here (see header of file) */
        prfn = LET_EXPR (arg_node);
        if (N_prf == NODE_TYPE (prfn) && F_sel_VxA == PRF_PRF (prfn)
            && ID_WL (PRF_ARG2 (prfn))) { /* second arg of sel() references a WL. */

            idn = PRF_ARG2 (prfn);

            /* These assignments are only done to transfer information
               to CreateCode (wlfm_replace) */
            INFO_ID (arg_info) = idn;
            INFO_NCA (arg_info) = ID_WL (idn);

            ref_mode_arg_info = arg_info; /* needed in CreateCode() */
            substwln = LET_EXPR (ASSIGN_STMT (ID_WL (idn)));
            targetwln = INFO_WL (arg_info);

            /* We just traversed the original code in the wlfm_search_ref phase, so
               ASSIGN_INDEX provides the correct index_info.*/
            transformation = ASSIGN_INDEX (INFO_ASSIGN (arg_info));

            DBUG_PRINT ("folding array %s in line %zu now...", ID_NAME (idn),
                        NODE_LINE (arg_node));
            Fold (idn, transformation, targetwln, substwln);
            DBUG_PRINT ("                               ...successful");
            global.optcounters.wlf_expr++;
            /* the WL substwln is now referenced one times less*/
            (WITH_REFERENCES_FOLDED (substwln))++;

            /* unused here */
            INFO_ID (arg_info) = NULL;
            INFO_NCA (arg_info) = NULL;
        }

        break;

    default:
        DBUG_UNREACHABLE ("Not expected");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFwith(node *arg_node, info *arg_info)
 *
 * description:
 *   First, traverse all bodies to fold WLs inside. Afterwards traverse
 *   codes again to find foldable assignments. If folding was successful,
 *   replace old generators with new ones, delete superflous code blocks and
 *   transform any modarray into gerarray.
 *
 ******************************************************************************/
node *
WLFwith (node *arg_node, info *arg_info)
{
    info *tmpi;
    node *tmpn, *substwln = NULL;

    DBUG_ENTER ();

    switch (wlf_mode) {
    case wlfm_search_WL:
        /* inside the body of this WL we may find another WL. So we better
         save the old arg_info information. */
        tmpi = MakeInfo ();
        INFO_FUNDEF (tmpi) = INFO_FUNDEF (arg_info);
        INFO_ASSIGN (tmpi) = INFO_ASSIGN (arg_info);
        INFO_NEXT (tmpi) = arg_info;
        arg_info = tmpi;

        INFO_WL (arg_info) = arg_node; /* store the current node for later */

        /* if WO_modarray, save referenced WL to transform modarray into
           genarray later. */
        if (N_modarray == NODE_TYPE (WITH_WITHOP (arg_node))) {
            /* This ID_WL is used in case (1) here (see header of file) */
            substwln = ID_WL (MODARRAY_ARRAY (WITH_WITHOP (arg_node)));
        }

        /* It's faster to
           1. traverse into bodies to find WL within and fold them first
           2. and then try to fold references to other WLs.
           */
        INFO_FLAG (arg_info) = 0;
        DBUG_PRINT ("traversing body of WL in line %zu", NODE_LINE (arg_node));
        arg_node = TRAVcont (arg_node, arg_info);

        if (INFO_FLAG (arg_info)) {
            int dim;

            /* traverse bodies of this WL again and fold now.
               Do not traverse WithOp (Id would confuse us). */
            wlf_mode = wlfm_search_ref; /* we do not need to save arg_info here */

            all_new_ig = NULL;
            new_codes = NULL;

            dim = SHgetUnrLen (TYgetShape (IDS_NTYPE (WITH_VEC (arg_node))));
            intersect_grids_ot = (int *)MEMmalloc (sizeof (int) * dim);
            intersect_grids_os = (int *)MEMmalloc (sizeof (int) * dim);

            DBUG_PRINT ("=> found something to fold in WL in line %zu",
                        NODE_LINE (arg_node));
            WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

            intersect_grids_ot = MEMfree (intersect_grids_ot);
            intersect_grids_os = MEMfree (intersect_grids_os);

            /* all codes have been traversed. Now append new_codes to WL and
               exchange old generators with all_new_ig. */
            if (new_codes) {
                /* We have new codes. This means at least one folding action has been
                   done and we have to replace the N_parts, too. */
                tmpn = WITH_CODE (arg_node);
                while (CODE_NEXT (tmpn)) {
                    tmpn = CODE_NEXT (tmpn);
                }
                CODE_NEXT (tmpn) = new_codes;

                /* before replacing the old generators with the new ones, we
                   can try to merge several of the new generators. */
                all_new_ig = MergeGenerators (all_new_ig);

                arg_node = WLFinternGen2Tree (arg_node, all_new_ig);
                all_new_ig = WLFfreeInternGenChain (all_new_ig);
                arg_node = CheckForSuperfluousCodes (arg_node);
                DBUG_PRINT ("<= new generators created");
            }

            /* this WL is finished. Search other WLs on same level. */
            wlf_mode = wlfm_search_WL;
        } else {
            DBUG_PRINT ("=> found nothing to fold in WL in line %zu",
                        NODE_LINE (arg_node));
        }

        /*
         * If the current WL has a modarray-operator there is a reference to another
         * array in its operator part. If this array was chosen to be folded
         * (FoldDecision) we have to eliminate the reference to it. Else DCR
         * would not remove the subst WL.
         *
         * substwln can be NULL if array is not in reach (loops, function argument)
         */
        if ((substwln != NULL) && (FoldDecision (arg_node, substwln))) {
            WITH_WITHOP (arg_node)
              = Modarray2Genarray (WITH_WITHOP (arg_node), arg_node, substwln);
        }

        /* restore arg_info */
        tmpi = arg_info;
        arg_info = INFO_NEXT (arg_info);
        tmpi = FreeInfo (tmpi);
        break;

    case wlfm_replace:
        /* Don't walk further into WL because we know that we do not
           find the Id we search for there. */
        break;

    case wlfm_rename:
        /* No renaming in generator or withop necessary. */
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        break;

    default:
        DBUG_UNREACHABLE ("Not expected");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFcode( node *arg_node, info *arg_info)
 *
 * description:
 *   If in wlfm_search_ref phase, create list new_ig of gererators which
 *   point to the current code.
 *
 ******************************************************************************/
node *
WLFcode (node *arg_node, info *arg_info)
{
    intern_gen *ig;

    DBUG_ENTER ();
    DBUG_ASSERT (CODE_USED (arg_node), "traversing unused code");

    switch (wlf_mode) {
    case wlfm_search_WL:
        /* here is no break missing! */
    case wlfm_rename:
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
        CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
        break;

    case wlfm_search_ref:
        /* create generator list. Copy all generators of the target-WL
           to this list which point to the current N_Ncode node. Don't use
           the traversal mechanism because it's slow. */
        new_ig = WLFtree2InternGen (INFO_WL (arg_info), arg_node);

        /* traverse Code, create new_ig, fold. */
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
        CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

        /* copy new generators to all_new_ig and clear new_ig. */
        if (!all_new_ig) {
            all_new_ig = new_ig;
        } else {
            ig = all_new_ig;
            while (ig->next) {
                ig = ig->next;
            }
            ig->next = new_ig;
        }
        new_ig = NULL;
        break;

    default:
        DBUG_UNREACHABLE ("Unexpected WLF mode");
    }

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLFdoWLF( node *arg_node)
 *
 *****************************************************************************/
node *
WLFdoWLF (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_PRINT_TAG ("OPTMEM", "mem currently allocated: %zu bytes",
                    global.current_allocated_mem);

    global.valid_ssaform = FALSE;
    /*
     * Wrapper code is created in non-SSA form and later on transformed into
     * SSA form using the standard transformation modules lac2fun and
     * ssa_transform. Therefore, we adjust the global control flag.
     */

    TRAVpush (TR_wlf);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    DBUG_PRINT_TAG ("OPTMEM", "mem currently allocated: %zu bytes",
                    global.current_allocated_mem);

    info = FreeInfo (info);

    /*
     * we free the information gathered by WLI here as it is no longer
     * used after this transformation
     */
    arg_node = FreeWLIInformation (arg_node);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
