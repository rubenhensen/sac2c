/*
 * $Id$
 */

/** <!--*******************************************************************-->
 *
 * @defgroup wlsb WLSBuild
 *
 * @brief replaces two nested with-loops with one equivalent scalarized
 *        with-loop. ( I find the term "scalarized" to be a misnomer -
 *        it is misleading or, at best, not evocative of
 *        what this optimization does.)
 *        Rumor has it that the term "scalarized" refers to the
 *        fact that this optimization permits array-valued temps
 *        generated by the inner WLs to be replaced by scalar values,
 *        and it is that replacement which lies behind the optimization's name.
 *
 *        In operation,
 *        the index ranges of the two WLs are merged into a single
 *        range via catenation of their bounds. Take and drop on
 *        the extended index vectors restores the original two
 *        index vectors. Other existing optimizations eliminate those
 *        take and drop operations, so the resulting code actually
 *        performs better than one might think.
 *
 *        Example of WLS using option -ssaiv:
 *
 * <pre>
 *   A = with
 *       ( ivl <= iv < ivu ) {
 *         B = with
 *             ( jvl <= jv < jvu ) {
 *               val = expr( iv, jv);
 *             } : val;
 *
 *             ( kvl <= kv < kvu) {
 *               val' = expr(iv, kv);
 *             } : val';
 *             genarray( shp_2);
 *       } : B
 *       genarray( shp_1);
 * </pre>
 *
 *   is transformed into
 *
 * <pre>
 *   LB1 = [ivl, jv1];
 *   UB1 = [ivu, jvu];
 *   LB2 = [ivl, kvl];
 *   UB2 = [ivu, kvu];
 *   A = with
 *     ( LB1 <= mv < UB1) {
 *         iv' = take( shape( ivl), mv);
 *         jv' = drop( shape( ivl), mv);
 *         val = expr( iv', jv');
 *       } : val;
 *     ( LB2 <= nv < UB2) {
 *         iv'' = take( shape( ivl), nv);
 *         kv'' = drop( shape( ivl), nv);
 *         val = expr( iv'', kv'');
 *       } : val;
 *       genarray( shp_1++shp_2);
 * </pre>
 *
 * @ingroup wls
 *
 * @{
 *
 ****************************************************************************/

/** <!--*******************************************************************-->
 *
 * @file wlscheck.c
 *
 * Implements a traversal to build a new, scalarized with-loop.
 *
 ****************************************************************************/
#include "wls.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "WLS"
#include "debug.h"

#include "new_types.h"
#include "print.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "str.h"
#include "memory.h"
#include "shape.h"
#include "free.h"
#include "constants.h"
#include "pattern_match.h"
#include "flattengenerators.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
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
    node *preassigns;
    lut_t *codelut;
    node *vardecs;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_CEXPR(n) (n->cexpr)
#define INFO_INNERTRAV(n) (n->innertrav)
#define INFO_NEWWITHID(n) (n->newwithid)
#define INFO_INNERWITHID(n) (n->innerwithid)
#define INFO_OUTERWITHID(n) (n->outerwithid)
#define INFO_OUTERGEN(n) (n->outergen)
#define INFO_NEWGEN(n) (n->newgen)
#define INFO_NEWCODE(n) (n->newcode)
#define INFO_NEWCODES(n) (n->newcodes)
#define INFO_NEWPARTS(n) (n->newparts)
#define INFO_NEWWITHOP(n) (n->newwithop)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_CODELUT(n) (n->codelut)
#define INFO_VARDECS(n) (n->vardecs)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_INNERTRAV (result) = FALSE;
    INFO_CEXPR (result) = NULL;
    INFO_NEWWITHID (result) = NULL;
    INFO_INNERWITHID (result) = NULL;
    INFO_OUTERWITHID (result) = NULL;
    INFO_OUTERGEN (result) = NULL;
    INFO_NEWGEN (result) = NULL;
    INFO_NEWCODE (result) = NULL;
    INFO_NEWCODES (result) = NULL;
    INFO_NEWPARTS (result) = NULL;
    INFO_NEWWITHOP (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_CODELUT (result) = LUTgenerateLut ();
    INFO_VARDECS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_CODELUT (info) = LUTremoveLut (INFO_CODELUT (info));
    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *WLSBdoBuild( node *arg_node, node *fundef)
 *
 * @brief starting function of the WLSBuild traversal.
 *
 * @param arg_node: the with-loop to be scalarized
 * @param fundef: the function containing the with-loop
 *
 * @return a new, scalarized with-loop
 *
 *****************************************************************************/
node *
WLSBdoBuild (node *arg_node, node *fundef, node **preassigns)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_with, "First parameter must be a with-loop");

    DBUG_ASSERT (NODE_TYPE (fundef) = N_fundef, "Second parameter must be a fundef");

    arg_info = MakeInfo (fundef);

    DBUG_PRINT ("Building new with-loop...");

    TRAVpush (TR_wlsb);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        *preassigns = TCappendAssign (*preassigns, INFO_PREASSIGNS (arg_info));
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    if (NULL != INFO_VARDECS (arg_info)) {
        FUNDEF_VARDECS (fundef)
          = TCappendVardec (FUNDEF_VARDECS (fundef), INFO_VARDECS (arg_info));
        INFO_VARDECS (arg_info) = NULL;
    }

    arg_info = FreeInfo (arg_info);

    DBUG_PRINT ("Scalarization complete. New with-loop is:");
    DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));
#ifdef FIXME // fundef appears to be corrupt at this point...

    DBUG_PRINT ("New fundef is:");
    DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, fundef));
#endif // FIXME

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
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
CreateOneVector (int nr, info *arg_info)
{
    node *res;
    node *temp;

    DBUG_ENTER ();

    res = TCcreateZeroVector (nr, T_int);

    temp = ARRAY_AELEMS (res);

    while (temp != NULL) {
        NUM_VAL (EXPRS_EXPR (temp)) = 1;
        EXPRS_EXPR (temp) = TBmakeId (
          FLATGflattenExpression (EXPRS_EXPR (temp), &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info),
                                  TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0))));
        temp = EXPRS_NEXT (temp);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *ConcatVector(node *vec1, node *vec2, info *arg_info)
 *
 * @brief Concatenates vectors vec1 and vec2.
 *        IMPORTANT: Both vectors are read-only!
 *
 * @param vec1   Either an AKV N_id OR a N_array vector
 *               OR an N_id that points to an N_array vector.
 * @param vec2   Either an AKV N_id OR a N_array vector
 *               OR an N_id that points to an N_array vector.
 * @param arg_info
 *
 * @return A N_array vector
 *
 *****************************************************************************/
static node *
ConcatVectors (node *vec1, node *vec2, info *arg_info)
{
    node *res;
    node *t1 = NULL;
    node *t2 = NULL;
    node *v1 = NULL;
    node *v2 = NULL;
    constant *v1fs = NULL;
    constant *v2fs = NULL;

    DBUG_ENTER ();

    /* We start by dereferencing any N_id, in hopes of finding an N_array */
    if ((NODE_TYPE (vec1) == N_id) && (PMO (PMOarray (&v1fs, &v1, vec1)))) {
        v1fs = COfreeConstant (v1fs);
    } else {
        v1 = vec1;
    }

    if ((NODE_TYPE (vec2) == N_id) && (PMO (PMOarray (&v2fs, &v2, vec2)))) {
        v2fs = COfreeConstant (v2fs);
    } else {
        v2 = vec2;
    }

    /*
     * In case the arguments are constant N_id nodes, constant arrays are
     * generated to be able to merge the elements,
     */
    if (NODE_TYPE (v1) == N_id) {
        DBUG_ASSERT (TYisAKV (ID_NTYPE (v1)), "BOUND1 N_id vector not AKV!");
        t1 = COconstant2AST (TYgetValue (ID_NTYPE (v1)));
    } else {
        DBUG_ASSERT (NODE_TYPE (v1) == N_array, "BOUND1 not N_array or N_id!");
        t1 = v1;
    }

    if (NODE_TYPE (v2) == N_id) {
        DBUG_ASSERT (TYisAKV (ID_NTYPE (v2)), "BOUND2 N_id vector not AKV!");
        t2 = COconstant2AST (TYgetValue (ID_NTYPE (v2)));
    } else {
        DBUG_ASSERT (NODE_TYPE (v2) == N_array, "BOUND2 not N_array or N_id!");
        t2 = v2;
    }

    /*
     * concatenate the elements,
     */
    res = TCmakeIntVector (
      TCappendExprs (DUPdoDupTree (ARRAY_AELEMS (t1)), DUPdoDupTree (ARRAY_AELEMS (t2))));

    /*
     * then deallocate the constant vectors.
     */
    if (NODE_TYPE (v1) == N_id) {
        t1 = FREEdoFreeTree (t1);
    }

    if (NODE_TYPE (v2) == N_id) {
        t2 = FREEdoFreeTree (t2);
    }

    /*
     * Flatten the result
     */
    res = WLSflattenBound (res, &FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                           &INFO_PREASSIGNS (arg_info));
    res = TBmakeId (res);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
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
    DBUG_ENTER ();

    if (!INFO_INNERTRAV (arg_info)) {
        /*
         * Outer code traversal
         */
        INFO_INNERTRAV (arg_info) = TRUE;
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
        INFO_INNERTRAV (arg_info) = FALSE;

        /*
         * Remember an old CEXPR in order to be able to build a new withop
         */
        INFO_CEXPR (arg_info) = CODE_CEXPR (arg_node);
    } else {
        /*
         * Inner code traversal
         */

        /*
         * Try to find a hashed version of the needed code
         */
        INFO_NEWCODE (arg_info) = LUTsearchInLutPp (INFO_CODELUT (arg_info), arg_node);
        if (INFO_NEWCODE (arg_info) != arg_node) {
            DBUG_PRINT ("Code can be reused!");
        } else {
            /*
             * Create a new code
             */
            node *new_code;
            node *prefix;
            node *array;
            node *newids, *oldids;
            node *oldavis, *avis;
            lut_t *lut;

            lut = LUTgenerateLut ();

            /*
             * The new code consists of:
             *
             * - a redefinition of the outer index vector
             */
            array
              = TCmakeIntVector (TCids2Exprs (WITHID_IDS (INFO_OUTERWITHID (arg_info))));

            oldavis = IDS_AVIS (WITHID_VEC (INFO_OUTERWITHID (arg_info)));
            avis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (oldavis)),
                               TYcopyType (AVIS_TYPE (oldavis)));

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            prefix = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), array), NULL);
            AVIS_SSAASSIGN (avis) = prefix;

            LUTinsertIntoLutP (lut, oldavis, avis);

            /*
             * - a redefinition of the inner index vector
             *
             * Its index scalars are the suffix of the new index scalars
             */
            newids = WITHID_IDS (INFO_NEWWITHID (arg_info));
            oldids = WITHID_IDS (INFO_OUTERWITHID (arg_info));
            while (oldids != NULL) {
                newids = IDS_NEXT (newids);
                oldids = IDS_NEXT (oldids);
            }

            array = TCmakeIntVector (TCids2Exprs (newids));

            oldavis = IDS_AVIS (WITHID_VEC (INFO_INNERWITHID (arg_info)));
            avis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (oldavis)),
                               TYcopyType (AVIS_TYPE (oldavis)));

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            ASSIGN_NEXT (prefix)
              = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), array), NULL);
            AVIS_SSAASSIGN (avis) = ASSIGN_NEXT (prefix);

            LUTinsertIntoLutP (lut, oldavis, avis);

            /*
             * The old inner part's code with replaced references to old ids
             */

            /*
             * Rename all occurences of old ids
             */
            oldids = WITHID_IDS (INFO_INNERWITHID (arg_info));

            while (oldids != NULL) {
                LUTinsertIntoLutP (lut, IDS_AVIS (oldids), IDS_AVIS (newids));

                oldids = IDS_NEXT (oldids);
                newids = IDS_NEXT (newids);
            }

            new_code = DUPdoDupNodeLutSsa (arg_node, lut, INFO_FUNDEF (arg_info));

            /*
             * The new code block must be prepended with the prefix
             */
            BLOCK_ASSIGNS (CODE_CBLOCK (new_code))
              = TCappendAssign (prefix, BLOCK_ASSIGNS (CODE_CBLOCK (new_code)));

            lut = LUTremoveLut (lut);

            /*
             * Put this code into CODELUT
             */
            LUTinsertIntoLutP (INFO_CODELUT (arg_info), arg_node, new_code);

            /*
             * Put it into NEWCODES
             */
            CODE_NEXT (new_code) = INFO_NEWCODES (arg_info);
            INFO_NEWCODES (arg_info) = new_code;

            /*
             * return the new code
             */
            INFO_NEWCODE (arg_info) = new_code;
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSBgenerator(node *arg_node, info *arg_info)
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
WLSBgenerator (node *arg_node, info *arg_info)
{
    int outerdim, innerdim;
    node *newlb, *newub, *newstep, *newwidth;

    DBUG_ENTER ();

    outerdim = TCcountIds (WITHID_IDS (INFO_OUTERWITHID (arg_info)));
    innerdim = TCcountIds (WITHID_IDS (INFO_INNERWITHID (arg_info)));

    /*
     * The new bounding vectors are simply the concatenation of the
     * old bounding vectors
     */
    newlb = ConcatVectors (GENERATOR_BOUND1 (INFO_OUTERGEN (arg_info)),
                           GENERATOR_BOUND1 (arg_node), arg_info);

    newub = ConcatVectors (GENERATOR_BOUND2 (INFO_OUTERGEN (arg_info)),
                           GENERATOR_BOUND2 (arg_node), arg_info);

    /*
     * Step and width vectors may be NULL. They must be created if needed.
     */
    if ((GENERATOR_STEP (INFO_OUTERGEN (arg_info)) == NULL)
        && (GENERATOR_STEP (arg_node) == NULL)) {
        newstep = NULL;
    } else {
        if (GENERATOR_STEP (INFO_OUTERGEN (arg_info)) == NULL) {
            GENERATOR_STEP (INFO_OUTERGEN (arg_info))
              = CreateOneVector (outerdim, arg_info);
        }

        if (GENERATOR_STEP (arg_node) == NULL) {
            GENERATOR_STEP (arg_node) = CreateOneVector (innerdim, arg_info);
        }

        newstep = ConcatVectors (GENERATOR_STEP (INFO_OUTERGEN (arg_info)),
                                 GENERATOR_STEP (arg_node), arg_info);
    }

    if ((GENERATOR_WIDTH (INFO_OUTERGEN (arg_info)) == NULL)
        && (GENERATOR_WIDTH (arg_node) == NULL)) {
        newwidth = NULL;
    } else {
        if (GENERATOR_WIDTH (INFO_OUTERGEN (arg_info)) == NULL) {
            GENERATOR_WIDTH (INFO_OUTERGEN (arg_info))
              = CreateOneVector (outerdim, arg_info);
        }

        if (GENERATOR_WIDTH (arg_node) == NULL) {
            GENERATOR_WIDTH (arg_node) = CreateOneVector (innerdim, arg_info);
        }

        newwidth = ConcatVectors (GENERATOR_WIDTH (INFO_OUTERGEN (arg_info)),
                                  GENERATOR_WIDTH (arg_node), arg_info);
    }

    /*
     * Create the new generator
     */
    INFO_NEWGEN (arg_info)
      = TBmakeGenerator (GENERATOR_OP1 (arg_node), GENERATOR_OP2 (arg_node), newlb, newub,
                         newstep, newwidth);

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
    DBUG_ENTER ();

    if (!INFO_INNERTRAV (arg_info)) {
        /*
         * Traversal of outer part
         *
         * Remember outer withid and outer generator
         */
        INFO_OUTERWITHID (arg_info) = PART_WITHID (arg_node);
        INFO_OUTERGEN (arg_info) = PART_GENERATOR (arg_node);

        /*
         * Traverse into code block
         */
        if (global.ssaiv) { /* in SSA, all partitions get new ids */
            INFO_NEWWITHID (arg_info) = NULL;
        }
        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

        /*
         * Traverse next part
         */
        PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
    } else {
        /*
         * Traversal of inner part
         */
        node *newpart;

        /*
         * Traverse withid
         */
        PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
        INFO_INNERWITHID (arg_info) = PART_WITHID (arg_node);

        /*
         * Traverse generator
         */
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

        /*
         * Traverse code
         */
        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

        /*
         * Create new part and store it into INFO_NEWPARTS
         */
        newpart = TBmakePart (INFO_NEWCODE (arg_info), INFO_NEWWITHID (arg_info),
                              INFO_NEWGEN (arg_info));

        /*
         * Increase code usage
         */
        CODE_USED (PART_CODE (newpart)) += 1;

        /*
         * Put part into NEWPARTS chain
         */
        PART_NEXT (newpart) = INFO_NEWPARTS (arg_info);
        INFO_NEWPARTS (arg_info) = newpart;

        /*
         * Traverse next part
         */
        PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ();

    if (!INFO_INNERTRAV (arg_info)) {
        /*
         * Traversal of outer with-loop
         */
        node *new_with;

        /*
         * Traverse into parts
         */
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

        /*
         * Traverse into withop
         */
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        /*
         * Finally, create a new with-loop
         */
        new_with = TBmakeWith (INFO_NEWPARTS (arg_info), INFO_NEWCODES (arg_info),
                               INFO_NEWWITHOP (arg_info));

        WITH_PARTS (new_with) = TCcountParts (WITH_PART (new_with));
        WITH_PRAGMA (new_with) = DUPdoDupNode (WITH_PRAGMA (arg_node));

        /*
         * replace the old with-loop
         */
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = new_with;
        /*
         * Increment WLS counter
         */
        global.optcounters.wls_expr += 1;
    } else {
        /*
         * Traversal of inner with-loop
         */
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
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
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_INNERTRAV (arg_info) == TRUE, "Only applicable to inner with-loop");

    if (INFO_NEWWITHID (arg_info) == NULL) {
        /*
         * Create new joint withid
         */
        int dim;
        ntype *vectype;
        node *vec, *scalars;
        node *avis;

        /*
         * Create a new type for the new index vector
         */
        dim = TCcountIds (WITHID_IDS (INFO_OUTERWITHID (arg_info)))
              + TCcountIds (WITHID_IDS (arg_node));

        vectype = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, dim));

        /*
         * Create a vardec for the new index vector
         */
        avis = TBmakeAvis (TRAVtmpVar (), vectype);

        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        /*
         * Create the ids for the new index vector
         */
        vec = TBmakeIds (avis, NULL);

        /*
         * recycle the old index scalars
         */
        scalars = TCappendIds (DUPdoDupTree (WITHID_IDS (INFO_OUTERWITHID (arg_info))),
                               DUPdoDupTree (WITHID_IDS (arg_node)));

        /*
         * Build the new withid
         */
        INFO_NEWWITHID (arg_info) = TBmakeWithid (vec, scalars);
    } else {
        /*
         * Copy existing withid
         */
        INFO_NEWWITHID (arg_info) = DUPdoDupNode (INFO_NEWWITHID (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSBgenarray(node *arg_node, info *arg_info)
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
WLSBgenarray (node *arg_node, info *arg_info)
{
    node *innershape;

    DBUG_ENTER ();

    DBUG_ASSERT (INFO_INNERTRAV (arg_info) == FALSE,
                 "Only applicable to outer with-loop");

    /*
     * Compute inner index space
     *
     * iis = take( shape( inner_iv), shape( CEXPR))
     */
    if (INFO_INNERWITHID (arg_info) == NULL) {
        innershape = SHshape2Array (TYgetShape (ID_NTYPE (INFO_CEXPR (arg_info))));
    } else {
        shape *tmp;
        tmp = SHtakeFromShape (TCcountIds (WITHID_IDS (INFO_INNERWITHID (arg_info))),
                               TYgetShape (ID_NTYPE (INFO_CEXPR (arg_info))));
        innershape = SHshape2Array (tmp);
        tmp = SHfreeShape (tmp);
    }

    INFO_NEWWITHOP (arg_info)
      = TBmakeGenarray (ConcatVectors (GENARRAY_SHAPE (arg_node), innershape, arg_info),
                        NULL);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSBmodarray(node *arg_node, info *arg_info)
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
WLSBmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_INNERTRAV (arg_info) == FALSE,
                 "Only applicable to outer with-loop");

    INFO_NEWWITHOP (arg_info) = DUPdoDupNode (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- WLSB -->
 *****************************************************************************/

#undef DBUG_PREFIX
