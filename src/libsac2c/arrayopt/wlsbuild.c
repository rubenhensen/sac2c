/*
 * $Id$
 */

/** <!--*******************************************************************-->
 *
 * @defgroup wlsb WLSBuild
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
 * </pre>
 *   is transformed into
 *
 * <pre>
 *   A = with ( lb_1++lb_2 <= kv < ub_1++ub_2) {
 *         iv = take( shape( lb_1), kv);
 *         jv = drop( shape( lb_1), kv);
 *         val = expr( iv, jv);
 *       } : val
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
#include "dbug.h"
#include "new_types.h"
#include "print.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "SSAConstantFolding.h"
#include "internal_lib.h"
#include "shape.h"
#include "free.h"
#include "constants.h"

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
    lut_t *codelut;
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
#define INFO_CODELUT(n) (n->codelut)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

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
    INFO_CODELUT (result) = LUTgenerateLut ();

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_CODELUT (info) = LUTremoveLut (INFO_CODELUT (info));
    info = ILIBfree (info);

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
 * @fn node *WLSBdoBuild( node *with, node *fundef)
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
WLSBdoBuild (node *with, node *fundef)
{
    info *info;

    DBUG_ENTER ("WLSBdoBuild");

    DBUG_ASSERT (NODE_TYPE (with) == N_with, "First parameter must be a with-loop");

    DBUG_ASSERT (NODE_TYPE (fundef) = N_fundef, "Second parameter must a fundef");

    info = MakeInfo (fundef);

    DBUG_PRINT ("WLS", ("Building new with-loop..."));

    TRAVpush (TR_wlsb);
    with = TRAVdo (with, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("WLS", ("Scalarization complete!"));
    DBUG_EXECUTE ("WLS", PRTdoPrintNode (with););

    DBUG_RETURN (with);
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
CreateOneVector (int nr)
{
    node *res;
    node *temp;

    DBUG_ENTER ("CreateOneVector");

    res = TCcreateZeroVector (nr, T_int);

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
 * @param vec1   Either an AKV N_id OR a N_array vector
 * @param vec2   Either an AKV N_id OR a N_array vector
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

    DBUG_ENTER ("ConcatVectors");

    /*
     * In case the arguments are constant N_id nodes, constant arrays are
     * generated to be able to merge the elements,
     */
    if (NODE_TYPE (vec1) == N_id) {
        DBUG_ASSERT (TYisAKV (ID_NTYPE (vec1)),
                     "Bounds vectors must be AKV or structural constants!");
        t1 = COconstant2AST (TYgetValue (ID_NTYPE (vec1)));
    } else {
        DBUG_ASSERT (NODE_TYPE (vec1) == N_array,
                     "Bounds vectors must be AKV or structural constants!");
        t1 = vec1;
    }

    if (NODE_TYPE (vec2) == N_id) {
        DBUG_ASSERT (TYisAKV (ID_NTYPE (vec2)),
                     "Bounds vectors must be AKV or structural constants!");
        t2 = COconstant2AST (TYgetValue (ID_NTYPE (vec2)));
    } else {
        DBUG_ASSERT (NODE_TYPE (vec1) == N_array,
                     "Bounds vectors must be AKV or structural constants!");
        t2 = vec2;
    }

    /*
     * concatenate the elements,
     */
    res = TCmakeIntVector (
      TCappendExprs (DUPdoDupTree (ARRAY_AELEMS (t1)), DUPdoDupTree (ARRAY_AELEMS (t2))));

    /*
     * and deallocate the newly created constant vectors must be freed.
     */
    if (NODE_TYPE (vec1) == N_id) {
        t1 = FREEdoFreeTree (t1);
    }

    if (NODE_TYPE (vec2) == N_id) {
        t2 = FREEdoFreeTree (t2);
    }

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
    DBUG_ENTER ("WLSBcode");

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
            DBUG_PRINT ("WLS", ("Code can be reused!"));
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
            avis = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (oldavis)),
                               TYcopyType (AVIS_TYPE (oldavis)));

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

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
            avis = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (oldavis)),
                               TYcopyType (AVIS_TYPE (oldavis)));

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

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
            if (NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (new_code))) == N_empty) {
                BLOCK_INSTR (CODE_CBLOCK (new_code))
                  = TCappendAssign (BLOCK_INSTR (CODE_CBLOCK (new_code)), prefix);
            } else {
                BLOCK_INSTR (CODE_CBLOCK (new_code))
                  = TCappendAssign (prefix, BLOCK_INSTR (CODE_CBLOCK (new_code)));
            }

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

    DBUG_ENTER ("WLSBgen");

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
            GENERATOR_STEP (INFO_OUTERGEN (arg_info)) = CreateOneVector (outerdim);
        }

        if (GENERATOR_STEP (arg_node) == NULL) {
            GENERATOR_STEP (arg_node) = CreateOneVector (innerdim);
        }

        newstep = ConcatVectors (GENERATOR_STEP (INFO_OUTERGEN (arg_info)),
                                 GENERATOR_STEP (arg_node), arg_info);
    }

    if ((GENERATOR_WIDTH (INFO_OUTERGEN (arg_info)) == NULL)
        && (GENERATOR_WIDTH (arg_node) == NULL)) {
        newwidth = NULL;
    } else {
        if (GENERATOR_WIDTH (INFO_OUTERGEN (arg_info)) == NULL) {
            GENERATOR_WIDTH (INFO_OUTERGEN (arg_info)) = CreateOneVector (outerdim);
        }

        if (GENERATOR_WIDTH (arg_node) == NULL) {
            GENERATOR_WIDTH (arg_node) = CreateOneVector (innerdim);
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
    DBUG_ENTER ("WLSBpart");

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
        PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

        /*
         * Traverse next part
         */
        if (PART_NEXT (arg_node) != NULL) {
            PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
        }
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
        if (PART_NEXT (arg_node) != NULL) {
            PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ("WLSBwithid");

    DBUG_ASSERT (INFO_INNERTRAV (arg_info) == TRUE,
                 "WLSBwithid only applicable in inner with-loop");

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
        avis = TBmakeAvis (ILIBtmpVar (), vectype);

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

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

    DBUG_ENTER ("WLSBgenarray");

    DBUG_ASSERT (INFO_INNERTRAV (arg_info) == FALSE,
                 "WLSBgenarray only applicable for outer with-loop");

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
    DBUG_ENTER ("WLSBmodarray");

    DBUG_ASSERT (INFO_INNERTRAV (arg_info) == FALSE,
                 "WLSBmodarray only applicable for outer with-loop");

    INFO_NEWWITHOP (arg_info) = DUPdoDupNode (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- WLSB -->
 *****************************************************************************/
