/*
 *
 * $Log$
 * Revision 1.1  2004/10/07 12:35:57  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup wlsb WLSBuild
 * @ingroup wls
 *
 * @brief replaces a with-loop with an equivalent scalarized with-loop.
 *
 * <pre>
 *
 *   A = with ( lb_1 <= iv < ub_1 ) {
 *         B = with ( lb_2 <= jv < ub_2 ) {
 *               val = expr( iv, jv);
 *             } : val
 *             genarray( shp_2);
 *       } : B
 *       genarray( shp_1);
 *
 *   is transformed into
 *
 *   A = with ( lb_1++lb_2 <= kv < ub_1++ub_2) {
 *         iv = take( shape( lb_1), kv);
 *         jv = drop( shape( lb_1), kv);
 *         val = expr( iv, jv);
 *       } : val
 *       genarray( shp_1++shp_2);
 *
 * </pre>
 *
 * @{
 */

/**
 *
 * @file wlscheck.c
 *
 * Implements a traversal to build a new, scalarized with-loop.
 */
#define NEW_INFO

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "new_types.h"
#include "print.h"
#include "optimize.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "SSAConstantFolding.h"
#include "shape.h"

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *cexpr;
    bool innertrav;
    node *newwithid;
    node *innerwithid;
    node *outerwithid;
    node *outergen;
    node *newgen;
    node *newcode;
    node *newcodes;
    node *newparts;
    node *newwithop;
    LUT_t codelut;
};

/**
 * INFO macros
 */
#define INFO_WLSB_FUNDEF(n) (n->fundef)
#define INFO_WLSB_CEXPR(n) (n->cexpr)
#define INFO_WLSB_INNERTRAV(n) (n->innertrav)
#define INFO_WLSB_NEWWITHID(n) (n->newwithid)
#define INFO_WLSB_INNERWITHID(n) (n->innerwithid)
#define INFO_WLSB_OUTERWITHID(n) (n->outerwithid)
#define INFO_WLSB_OUTERGEN(n) (n->outergen)
#define INFO_WLSB_NEWGEN(n) (n->newgen)
#define INFO_WLSB_NEWCODE(n) (n->newcode)
#define INFO_WLSB_NEWCODES(n) (n->newcodes)
#define INFO_WLSB_NEWPARTS(n) (n->newparts)
#define INFO_WLSB_NEWWITHOP(n) (n->newwithop)
#define INFO_WLSB_CODELUT(n) (n->codelut)

/**
 * INFO functions
 */
static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_WLSB_FUNDEF (result) = fundef;
    INFO_WLSB_INNERTRAV (result) = FALSE;
    INFO_WLSB_CEXPR (result) = NULL;
    INFO_WLSB_NEWWITHID (result) = NULL;
    INFO_WLSB_INNERWITHID (result) = NULL;
    INFO_WLSB_OUTERWITHID (result) = NULL;
    INFO_WLSB_OUTERGEN (result) = NULL;
    INFO_WLSB_NEWGEN (result) = NULL;
    INFO_WLSB_NEWCODE (result) = NULL;
    INFO_WLSB_NEWCODES (result) = NULL;
    INFO_WLSB_NEWPARTS (result) = NULL;
    INFO_WLSB_NEWWITHOP (result) = NULL;
    INFO_WLSB_CODELUT (result) = GenerateLUT ();

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_WLSB_CODELUT (info) = RemoveLUT (INFO_WLSB_CODELUT (info));
    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSBuild( node *with, node *fundef)
 *
 * @brief starting function of the WLSBuild traversal.
 *
 * @param with the with-loop to be scalarized
 * @param fundef
 *
 * @return a new, scalarized with-loop
 *
 *****************************************************************************/
node *
WLSBuild (node *with, node *fundef)
{
    funtab *old_tab;
    info *info;

    DBUG_ENTER ("WLSBuild");

    DBUG_ASSERT (NODE_TYPE (with) == N_Nwith, "First parameter must be a with-loop");

    DBUG_ASSERT (NODE_TYPE (fundef) = N_fundef, "Second parameter must a fundef");

    info = MakeInfo (fundef);

    old_tab = act_tab;
    act_tab = wlsb_tab;

    DBUG_PRINT ("WLS", ("Building new with-loop..."));

    with = Trav (with, info);
    act_tab = old_tab;

    info = FreeInfo (info);

    DBUG_PRINT ("WLS", ("Scalarization complete!"));
    DBUG_EXECUTE ("WLS", PrintNode (with););

    DBUG_RETURN (with);
}

/******************************************************************************
 *
 * Helper function
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CreateOneVector( int l)
 *
 * @brief Creates a vector of ones of length l
 *
 * @param l length of vector
 *
 * @return
 *
 *****************************************************************************/
static node *
CreateOneVector (int nr)
{
    node *res;
    node *temp;

    DBUG_ENTER ("CreateOneVector");

    res = CreateZeroVector (nr, T_int);

    temp = ARRAY_AELEMS (res);

    while (temp != NULL) {
        NUM_VAL (EXPRS_EXPR (temp)) = 1;
        temp = EXPRS_NEXT (temp);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *ConcatVector(node *vec1, node *vec2, info *arg_info)
 *
 * @brief Concatenates vectors vec1 and vec2.
 *        IMPORTANT: Both vectors are inspected only!
 *
 * @param vec1
 * @param vec2
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
static node *
ConcatVectors (node *vec1, node *vec2, info *arg_info)
{
    node *res;
    node *arg_expr_mem[2];
    node **arg_expr = &arg_expr_mem[0];

    DBUG_ENTER ("ConcatVectors");

    arg_expr[0] = vec1;
    arg_expr[1] = vec2;
    res = SSACFFoldPrfExpr (F_cat_VxV, arg_expr);

    if (NODE_TYPE (res) != N_array) {
        DBUG_ASSERT ((0), "Not yet implemented");
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn int CountParts( node *parts)
 *
 * @brief Counts the number of parts in a given chain of parts
 *
 * @param parts
 *
 * @return number of parts
 *
 *****************************************************************************/
static int
CountParts (node *parts)
{
    int counter = 0;

    DBUG_ENTER ("CountParts");

    while (parts != NULL) {
        counter += 1;
        parts = NPART_NEXT (parts);
    }

    DBUG_RETURN (counter);
}

/******************************************************************************
 *
 * WLS build traversal (wlsb_tab)
 *
 * prefix: WLSB
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *WLSBcode(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSBcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSBcode");

    if (!INFO_WLSB_INNERTRAV (arg_info)) {
        /*
         * Outer code traversal
         */
        INFO_WLSB_INNERTRAV (arg_info) = TRUE;
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
        INFO_WLSB_INNERTRAV (arg_info) = FALSE;

        /*
         * Remember an old CEXPR in order to be able to build a new withop
         */
        INFO_WLSB_CEXPR (arg_info) = NCODE_CEXPR (arg_node);
    } else {
        /*
         * Inner code traversal
         */

        /*
         * Try to find a hashed version of the needed code
         */
        INFO_WLSB_NEWCODE (arg_info)
          = SearchInLUT_PP (INFO_WLSB_CODELUT (arg_info), arg_node);
        if (INFO_WLSB_NEWCODE (arg_info) != arg_node) {
            DBUG_PRINT ("WLS", ("Code can be reused!"));
        } else {
            /*
             * Create a new code
             */
            node *new_code;
            node *prefix;
            node *array;
            ids *newids, *oldids;
            ids *new_iv;
            LUT_t lut;

            /*
             * The new code consists of:
             *
             * - a redefinition of the outer index vector
             */
            array = Ids2Array (NWITHID_IDS (INFO_WLSB_OUTERWITHID (arg_info)));
            new_iv = DupOneIds (NWITHID_VEC (INFO_WLSB_OUTERWITHID (arg_info)));
            prefix = MakeAssignLet (IDS_NAME (new_iv), IDS_VARDEC (new_iv), array);
            AVIS_WITHID (IDS_AVIS (new_iv)) = NULL;
            AVIS_SSAASSIGN (IDS_AVIS (new_iv)) = prefix;

            /*
             * - a redefinition of the inner index vector
             *
             * Its index scalars are the suffix of the new index scalars
             */
            newids = NWITHID_IDS (INFO_WLSB_NEWWITHID (arg_info));
            oldids = NWITHID_IDS (INFO_WLSB_OUTERWITHID (arg_info));
            while (oldids != NULL) {
                newids = IDS_NEXT (newids);
                oldids = IDS_NEXT (oldids);
            }

            array = Ids2Array (newids);
            new_iv = DupOneIds (NWITHID_VEC (INFO_WLSB_INNERWITHID (arg_info)));
            ASSIGN_NEXT (prefix)
              = MakeAssignLet (IDS_NAME (new_iv), IDS_VARDEC (new_iv), array);
            AVIS_WITHID (IDS_AVIS (new_iv)) = NULL;
            AVIS_SSAASSIGN (IDS_AVIS (new_iv)) = ASSIGN_NEXT (prefix);

            /*
             * The old inner part's code with replaced references to old ids
             */
            lut = GenerateLUT ();

            /*
             * Rename all occurences of old ids
             */
            oldids = NWITHID_IDS (INFO_WLSB_INNERWITHID (arg_info));

            while (oldids != NULL) {
                InsertIntoLUT_S (lut, IDS_NAME (oldids), IDS_NAME (newids));
                InsertIntoLUT_P (lut, IDS_VARDEC (oldids), IDS_VARDEC (newids));
                InsertIntoLUT_P (lut, IDS_AVIS (oldids), IDS_AVIS (newids));

                oldids = IDS_NEXT (oldids);
                newids = IDS_NEXT (newids);
            }

            new_code = DupNodeLUT (arg_node, lut);

            /*
             * The new code block must be prepended with the prefix
             */
            if (NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (new_code))) == N_empty) {
                BLOCK_INSTR (NCODE_CBLOCK (new_code))
                  = AppendAssign (BLOCK_INSTR (NCODE_CBLOCK (new_code)), prefix);
            } else {
                BLOCK_INSTR (NCODE_CBLOCK (new_code))
                  = AppendAssign (prefix, BLOCK_INSTR (NCODE_CBLOCK (new_code)));
            }

            lut = RemoveLUT (lut);

            /*
             * Put this code into CODELUT
             */
            InsertIntoLUT_P (INFO_WLSB_CODELUT (arg_info), arg_node, new_code);

            /*
             * Put it into NEWCODES
             */
            NCODE_NEXT (new_code) = INFO_WLSB_NEWCODES (arg_info);
            INFO_WLSB_NEWCODES (arg_info) = new_code;

            /*
             * return the new code
             */
            INFO_WLSB_NEWCODE (arg_info) = new_code;
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSBgen(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSBgen (node *arg_node, info *arg_info)
{
    int outerdim, innerdim;
    node *newlb, *newub, *newstep, *newwidth;

    DBUG_ENTER ("WLSBgen");

    outerdim = CountIds (NWITHID_IDS (INFO_WLSB_OUTERWITHID (arg_info)));
    innerdim = CountIds (NWITHID_IDS (INFO_WLSB_INNERWITHID (arg_info)));

    /*
     * The new bounding vectors are simply the concatenation of the
     * old bounding vectors
     */
    newlb = ConcatVectors (NGEN_BOUND1 (INFO_WLSB_OUTERGEN (arg_info)),
                           NGEN_BOUND1 (arg_node), arg_info);

    newub = ConcatVectors (NGEN_BOUND2 (INFO_WLSB_OUTERGEN (arg_info)),
                           NGEN_BOUND2 (arg_node), arg_info);

    /*
     * Step and width vectors may be NULL. They must be created if needed.
     */
    if ((NGEN_STEP (INFO_WLSB_OUTERGEN (arg_info)) == NULL)
        && (NGEN_STEP (arg_node) == NULL)) {
        newstep = NULL;
    } else {
        if (NGEN_STEP (INFO_WLSB_OUTERGEN (arg_info)) == NULL) {
            NGEN_STEP (INFO_WLSB_OUTERGEN (arg_info)) = CreateOneVector (outerdim);
        }

        if (NGEN_STEP (arg_node) == NULL) {
            NGEN_STEP (arg_node) = CreateOneVector (innerdim);
        }

        newstep = ConcatVectors (NGEN_STEP (INFO_WLSB_OUTERGEN (arg_info)),
                                 NGEN_STEP (arg_node), arg_info);
    }

    if ((NGEN_WIDTH (INFO_WLSB_OUTERGEN (arg_info)) == NULL)
        && (NGEN_WIDTH (arg_node) == NULL)) {
        newwidth = NULL;
    } else {
        if (NGEN_WIDTH (INFO_WLSB_OUTERGEN (arg_info)) == NULL) {
            NGEN_WIDTH (INFO_WLSB_OUTERGEN (arg_info)) = CreateOneVector (outerdim);
        }

        if (NGEN_WIDTH (arg_node) == NULL) {
            NGEN_WIDTH (arg_node) = CreateOneVector (innerdim);
        }

        newwidth = ConcatVectors (NGEN_WIDTH (INFO_WLSB_OUTERGEN (arg_info)),
                                  NGEN_WIDTH (arg_node), arg_info);
    }

    /*
     * Create the new generator
     */
    INFO_WLSB_NEWGEN (arg_info) = MakeNGenerator (newlb, newub, NGEN_OP1 (arg_node),
                                                  NGEN_OP2 (arg_node), newstep, newwidth);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSBpart(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSBpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSBpart");

    if (!INFO_WLSB_INNERTRAV (arg_info)) {
        /*
         * Traversal of outer part
         *
         * Remember outer withid and outer generator
         */
        INFO_WLSB_OUTERWITHID (arg_info) = NPART_WITHID (arg_node);
        INFO_WLSB_OUTERGEN (arg_info) = NPART_GEN (arg_node);

        /*
         * Traverse into code block
         */
        NPART_CODE (arg_node) = Trav (NPART_CODE (arg_node), arg_info);

        /*
         * Traverse next part
         */
        if (NPART_NEXT (arg_node) != NULL) {
            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        }
    } else {
        /*
         * Traversal of inner part
         */
        node *newpart;

        /*
         * Traverse withid
         */
        NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);
        INFO_WLSB_INNERWITHID (arg_info) = NPART_WITHID (arg_node);

        /*
         * Traverse generator
         */
        NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

        /*
         * Traverse code
         */
        NPART_CODE (arg_node) = Trav (NPART_CODE (arg_node), arg_info);

        /*
         * Create new part and store it into INFO_WLSB_NEWPARTS
         */
        newpart = MakeNPart (INFO_WLSB_NEWWITHID (arg_info), INFO_WLSB_NEWGEN (arg_info),
                             INFO_WLSB_NEWCODE (arg_info));

        /*
         * Increase code usage
         */
        NCODE_USED (NPART_CODE (newpart)) += 1;

        /*
         * Put part into NEWPARTS chain
         */
        NPART_NEXT (newpart) = INFO_WLSB_NEWPARTS (arg_info);
        INFO_WLSB_NEWPARTS (arg_info) = newpart;

        /*
         * Traverse next part
         */
        if (NPART_NEXT (arg_node) != NULL) {
            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSBwith(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSBwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSBwith");

    if (!INFO_WLSB_INNERTRAV (arg_info)) {
        /*
         * Traversal of outer with-loop
         */
        node *new_with;

        /*
         * Traverse into parts
         */
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

        /*
         * Traverse into withop
         */
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

        /*
         * Finally, create a new with-loop
         */
        new_with
          = MakeNWith (INFO_WLSB_NEWPARTS (arg_info), INFO_WLSB_NEWCODES (arg_info),
                       INFO_WLSB_NEWWITHOP (arg_info));

        NWITH_PARTS (new_with) = CountParts (NWITH_PART (new_with));
        NWITH_PRAGMA (new_with) = DupNode (NWITH_PRAGMA (arg_node));

        /*
         * replace the old with-loop
         */
        arg_node = FreeTree (arg_node);
        arg_node = new_with;
        /*
         * Increment WLS counter
         */
        wls_expr += 1;
    } else {
        /*
         * Traversal of inner with-loop
         */
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *WLSBwithid(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSBwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSBwithid");

    DBUG_ASSERT (INFO_WLSB_INNERTRAV (arg_info) == TRUE,
                 "WLSBwithid only applicable in inner with-loop");

    if (INFO_WLSB_NEWWITHID (arg_info) == NULL) {
        /*
         * Create new joint withid
         */
        int dim;
        ntype *vectype;
        ids *vec, *scalars;
        node *vardec;

        /*
         * Create a new type for the new index vector
         */
        dim = CountIds (NWITHID_IDS (INFO_WLSB_OUTERWITHID (arg_info)))
              + CountIds (NWITHID_IDS (arg_node));

        vectype = TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (1, dim));

        /*
         * Create a vardec for the new index vector
         */
        vardec = MakeVardec (TmpVar (), TYType2OldType (vectype),
                             FUNDEF_VARDEC (INFO_WLSB_FUNDEF (arg_info)));
        FUNDEF_VARDEC (INFO_WLSB_FUNDEF (arg_info)) = vardec;
        AVIS_TYPE (VARDEC_AVIS (vardec)) = vectype;

        /*
         * Create the ids for the new index vector
         */
        vec = MakeIds (StringCopy (VARDEC_NAME (vardec)), NULL, ST_regular);
        IDS_VARDEC (vec) = vardec;
        IDS_AVIS (vec) = VARDEC_AVIS (vardec);

        /*
         * recycle the old index scalars
         */
        scalars = AppendIds (DupAllIds (NWITHID_IDS (INFO_WLSB_OUTERWITHID (arg_info))),
                             DupAllIds (NWITHID_IDS (arg_node)));

        /*
         * Build the new withid
         */
        INFO_WLSB_NEWWITHID (arg_info) = MakeNWithid (vec, scalars);
    } else {
        /*
         * Copy existing withid
         */
        INFO_WLSB_NEWWITHID (arg_info) = DupNode (INFO_WLSB_NEWWITHID (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSBwithop(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSBwithop (node *arg_node, info *arg_info)
{
    node *innershape;

    DBUG_ENTER ("WLSBwithop");

    DBUG_ASSERT (INFO_WLSB_INNERTRAV (arg_info) == FALSE,
                 "WLSBwithop only applicable for outer with-loop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        /*
         * Compute inner index space
         *
         * iis = take( shape( inner_iv), shape( CEXPR))
         */
        if (INFO_WLSB_INNERWITHID (arg_info) == NULL) {
            innershape
              = SHShape2Array (TYGetShape (ID_NTYPE (INFO_WLSB_CEXPR (arg_info))));
        } else {
            shape *tmp;
            tmp = SHTakeFromShape (CountIds (
                                     NWITHID_IDS (INFO_WLSB_INNERWITHID (arg_info))),
                                   TYGetShape (ID_NTYPE (INFO_WLSB_CEXPR (arg_info))));
            innershape = SHShape2Array (tmp);
            tmp = SHFreeShape (tmp);
        }

        INFO_WLSB_NEWWITHOP (arg_info)
          = MakeNWithOp (WO_genarray,
                         ConcatVectors (NWITHOP_SHAPE (arg_node), innershape, arg_info));
        break;

    case WO_modarray:
        INFO_WLSB_NEWWITHOP (arg_info) = DupNode (arg_node);
        break;

    default:
        DBUG_ASSERT ((0), "Illegal withop-type!");
        break;
    }

    DBUG_RETURN (arg_node);
}

/*@}*/ /* defgroup wlsb */
