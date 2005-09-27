/*
 * $Log$
 * Revision 1.4  2005/09/27 14:25:06  sah
 * quick fix for bug Ã#120
 *
 * Revision 1.3  2005/09/27 02:52:11  sah
 * hopefully, IVE is now back to its own power. Due to
 * lots of other compiler bugs, this code is not fully
 * tested, yet.
 *
 * Revision 1.2  2005/09/15 11:08:52  sah
 * now, wloffsets are used whenever possible
 *
 * Revision 1.1  2005/09/14 21:26:36  sah
 * Initial revision
 *
 *
 */

#include "index_optimize.h"

#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "free.h"
#include "internal_lib.h"
#include "type_utils.h"
#include "new_types.h"
#include "shape.h"

/**
 * forward declarations
 */
typedef struct OFFSETINFO offsetinfo;
typedef struct IVINFO ivinfo;

/**
 * INFO structure
 */
struct INFO {
    ivinfo *ivinfo;
    node *lhs;
    node *vardecs;
    node *preassigns;
};

/**
 * INFO macros
 */
#define INFO_IVINFO(n) ((n)->ivinfo)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_IVINFO (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**
 * WITHIV information structure.
 *
 * this structure is used to store withid x shape -> offset
 * information to ease the lookup of matching withloop
 * offsets for a given vect2offset operation.
 *
 * IV       stores the avis of the indexvector
 * OFFSETS  contains all offsetvar avis and their corresponding shape
 * SCALARS  contains the N_ids chain of scalars that can be used instead
 *          of the index vector
 */

struct OFFSETINFO {
    node *avis;
    shape *shape;
    offsetinfo *next;
};

struct IVINFO {
    node *iv;
    offsetinfo *offsets;
    node *scalars;
    ivinfo *next;
};

/**
 * WITHIV macros
 */
#define WITHIV_IV(n) ((n)->iv)
#define WITHIV_OFFSETS(n) ((n)->offsets)
#define WITHIV_SCALARS(n) ((n)->scalars)
#define WITHIV_NEXT(n) ((n)->next)
#define WITHOFFSET_AVIS(n) ((n)->avis)
#define WITHOFFSET_SHAPE(n) ((n)->shape)
#define WITHOFFSET_NEXT(n) ((n)->next)

/**
 * WITHIV functions
 */
static offsetinfo *
GenOffsetInfo (node *lhs, node *withops)
{
    offsetinfo *result;
    offsetinfo *next;

    DBUG_ENTER ("GenOffsetInfo");

    if (lhs != NULL) {
        DBUG_ASSERT ((withops != NULL), "# withops does not match # lhs ids");

        next = GenOffsetInfo (IDS_NEXT (lhs), WITHOP_NEXT (withops));

        if (((NODE_TYPE (withops) == N_genarray) || (NODE_TYPE (withops) == N_modarray))
            && (TUshapeKnown (IDS_NTYPE (lhs)))) {
            /*
             * only genarray and modarray wls have a built in index.
             * furthermore, the shape of the result must be known
             * to be able to decide whether the offset does match
             */
            result = ILIBmalloc (sizeof (offsetinfo));

            WITHOFFSET_SHAPE (result) = TYgetShape (IDS_NTYPE (lhs));
            WITHOFFSET_AVIS (result) = WITHOP_IDX (withops);
            WITHOFFSET_NEXT (result) = next;
        } else {
            result = next;
        }
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

static offsetinfo *
FreeOffsetInfo (offsetinfo *info)
{
    DBUG_ENTER ("FreeOffsetInfo");

    if (info != NULL) {
        WITHOFFSET_NEXT (info) = FreeOffsetInfo (WITHOFFSET_NEXT (info));

        info = ILIBfree (info);
    }

    DBUG_RETURN (info);
}

static ivinfo *
PushIV (ivinfo *info, node *withid, node *lhs, node *withops)
{
    ivinfo *result;

    DBUG_ENTER ("PushIV");

    result = ILIBmalloc (sizeof (ivinfo));

    WITHIV_IV (result) = IDS_AVIS (WITHID_VEC (withid));
    WITHIV_OFFSETS (result) = GenOffsetInfo (lhs, withops);
    WITHIV_SCALARS (result) = WITHID_IDS (withid);
    WITHIV_NEXT (result) = info;

    DBUG_RETURN (result);
}

static ivinfo *
PopIV (ivinfo *info)
{
    ivinfo *result;

    DBUG_ENTER ("PopIV");

    DBUG_ASSERT ((info != NULL), "IVINFO stack already empty!");

    result = WITHIV_NEXT (info);

    WITHIV_OFFSETS (info) = FreeOffsetInfo (WITHIV_OFFSETS (info));

    info = ILIBfree (info);

    DBUG_RETURN (result);
}

static node *
FindIVOffset (ivinfo *info, node *iv, shape *shape)
{
    node *result = NULL;
    offsetinfo *oinfo;

    DBUG_ENTER ("FindIVOffset");

    while ((info != NULL) && (WITHIV_IV (info) != iv)) {
        info = WITHIV_NEXT (info);
    }

    if (info != NULL) {
        oinfo = WITHIV_OFFSETS (info);

        while ((oinfo != NULL) && (!SHcompareShapes (shape, WITHOFFSET_SHAPE (oinfo)))) {
            oinfo = WITHOFFSET_NEXT (oinfo);
        }

        if (oinfo != NULL) {
            result = WITHOFFSET_AVIS (oinfo);
        }
    }

    DBUG_RETURN (result);
}

static node *
FindIVScalars (ivinfo *info, node *iv)
{
    node *result = NULL;

    DBUG_ENTER ("FindIVScalars");

    while ((info != NULL) && (WITHIV_IV (info) != iv)) {
        info = WITHIV_NEXT (info);
    }

    if (info != NULL) {
        result = WITHIV_SCALARS (info);
    }

    DBUG_RETURN (result);
}

static node *
GetAvis4Shape (node *avis, shape *shape)
{
    node *result = NULL;
    node *types;
    node *ids;

    DBUG_ENTER ("GetAvis4Shape");

    types = AVIS_IDXTYPES (avis);
    ids = AVIS_IDXIDS (avis);

    /*
     * we have to be robust w.r.t. missing idxids, as during
     * optimisation, the idxids for withids have been deleted
     * again! see bug #120
     */
    while ((types != NULL) && (ids != NULL)) {

        if (SHcompareShapes (shape, TYgetShape (TYPE_TYPE (EXPRS_EXPR (types))))) {
            result = IDS_AVIS (ids);
            break;
        }

        types = EXPRS_NEXT (types);
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (result);
}

static node *
Scalar2Offset (node *scalar, int dims, shape *shape, info *arg_info)
{
    node *args = NULL;
    node *avis;
    node *let;
    int cnt;

    DBUG_ENTER ("Scalar2Offset");

    avis
      = TBmakeAvis (ILIBtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    /*
     * create a list of dims scalar arguments
     */
    for (cnt = 0; cnt < dims; cnt++) {
        args = TBmakeExprs (DUPdoDupNode (scalar), args);
    }

    /*
     * add shape array
     */
    args = TBmakeExprs (SHshape2Array (shape), args);

    /*
     * tmpvar = _idxs2offset( args)
     */
    let = TBmakeLet (TBmakeIds (avis, NULL), TBmakePrf (F_idxs2offset, args));

    INFO_PREASSIGNS (arg_info) = TBmakeAssign (let, INFO_PREASSIGNS (arg_info));

    DBUG_RETURN (avis);
}

/**
 * optimizer functions
 */
static node *
ReplaceByWithOffset (node *arg_node, info *arg_info)
{
    shape *shape;
    node *offset;
    node *scalars;

    DBUG_ENTER ("ReplaceByWithOffset");

    shape = SHarray2Shape (PRF_ARG1 (arg_node));

    offset = FindIVOffset (INFO_IVINFO (arg_info), ID_AVIS (PRF_ARG2 (arg_node)), shape);

    shape = SHfreeShape (shape);

    if (offset != NULL) {
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TBmakeId (offset);
    } else {
        scalars = FindIVScalars (INFO_IVINFO (arg_info), ID_AVIS (PRF_ARG2 (arg_node)));

        if (scalars != NULL) {
            offset
              = TBmakePrf (F_idxs2offset, TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                                       TCids2Exprs (scalars)));
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = offset;
        }
    }

    DBUG_RETURN (arg_node);
}

static node *
ReplaceByIdx2Offset (node *arg_node, info *arg_info)
{
    node *result;
    node *ivassign;
    node *idxs;

    DBUG_ENTER ("ReplaceByIdx2Offset");

    DBUG_ASSERT ((NODE_TYPE (PRF_ARG2 (arg_node)) == N_id),
                 "ReplaceByIdx2Offset called with iv being non N_id node");
    DBUG_ASSERT ((AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node))) != NULL),
                 "ReplaceByIdx2Offset with AVIS_SSAASSIGN of iv being NULL");

    ivassign = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)));

    DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (ivassign)) == N_array),
                 "ReplaceByIdx2Offset with non N_array AVIS_SSAASSIGN");

    idxs = ARRAY_AELEMS (ASSIGN_RHS (ivassign));

    result = TBmakePrf (F_idxs2offset, TBmakeExprs (DUPdoDupNode (PRF_ARG1 (arg_node)),
                                                    DUPdoDupTree (idxs)));

    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (result);
}

static node *
OptimizeComputation (node *arg_node, info *arg_info)
{
    node *iv;
    node *prf;
    node *arg1;
    node *arg2;
    shape *shape;

    DBUG_ENTER ("OptimizeComputation");

    iv = PRF_ARG2 (arg_node);
    prf = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (iv)));
    shape = SHarray2Shape (PRF_ARG1 (arg_node));

    if (AVIS_NEEDCOUNT (IDS_AVIS (INFO_LHS (arg_info))) == 0) {
        /*
         * this vector is only used as index vector!
         */
        switch (PRF_PRF (prf)) {
        case F_add_AxA:
        case F_sub_AxA:
            /*
             * look up the matching offsets and replace computation
             */
            arg1 = GetAvis4Shape (ID_AVIS (PRF_ARG1 (prf)), shape);
            arg2 = GetAvis4Shape (ID_AVIS (PRF_ARG2 (prf)), shape);

            if ((arg1 != NULL) && (arg2 != NULL)) {
                arg_node = FREEdoFreeNode (arg_node);
                arg_node
                  = TCmakePrf2 ((PRF_PRF (prf) == F_add_AxA) ? F_add_SxS : F_sub_SxS,
                                TBmakeId (arg1), TBmakeId (arg2));
            }
            break;

        case F_add_SxA:
        case F_sub_SxA:
            if (TUdimKnown (ID_NTYPE (PRF_ARG2 (prf)))) {
                arg2 = GetAvis4Shape (ID_AVIS (PRF_ARG2 (prf)), shape);

                if (arg2 != NULL) {
                    arg1 = Scalar2Offset (PRF_ARG1 (prf),
                                          TYgetDim (ID_NTYPE (PRF_ARG2 (prf))), shape,
                                          arg_info);

                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node
                      = TCmakePrf2 ((PRF_PRF (prf) == F_add_SxA) ? F_add_SxA : F_sub_SxA,
                                    TBmakeId (arg1), TBmakeId (arg2));
                }
            }
            break;

        case F_add_AxS:
        case F_sub_AxS:
            if (TUdimKnown (ID_NTYPE (PRF_ARG1 (prf)))) {
                arg1 = GetAvis4Shape (ID_AVIS (PRF_ARG1 (prf)), shape);

                if (arg1 != NULL) {
                    arg2 = Scalar2Offset (PRF_ARG1 (prf),
                                          TYgetDim (ID_NTYPE (PRF_ARG1 (prf))), shape,
                                          arg_info);

                    arg_node = FREEdoFreeNode (arg_node);
                    arg_node
                      = TCmakePrf2 ((PRF_PRF (prf) == F_add_AxS) ? F_add_AxS : F_sub_AxS,
                                    TBmakeId (arg1), TBmakeId (arg2));
                }
            }
            break;

        case F_mul_SxA:
            arg2 = GetAvis4Shape (ID_AVIS (PRF_ARG2 (prf)), shape);

            if (arg2 != NULL) {
                arg1 = PRF_ARG1 (prf);
                PRF_ARG1 (prf) = NULL;

                arg_node = FREEdoFreeNode (arg_node);
                arg_node = TCmakePrf2 (F_mul_SxS, arg1, arg2);
            }
            break;

        case F_mul_AxS:
            arg1 = GetAvis4Shape (ID_AVIS (PRF_ARG1 (prf)), shape);

            if (arg1 != NULL) {
                arg2 = PRF_ARG2 (prf);
                PRF_ARG2 (prf) = NULL;

                arg_node = FREEdoFreeNode (arg_node);
                arg_node = TCmakePrf2 (F_mul_SxS, arg1, arg2);
            }
            break;

        default:
            break;
        }
    }

    shape = SHfreeShape (shape);

    DBUG_RETURN (arg_node);
}

/**
 * traversal functions
 */
node *
IVEOlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEOlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
IVEOwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEOwith");

    INFO_IVINFO (arg_info) = PushIV (INFO_IVINFO (arg_info), WITH_WITHID (arg_node),
                                     INFO_LHS (arg_info), WITH_WITHOP (arg_node));

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_IVINFO (arg_info) = PopIV (INFO_IVINFO (arg_info));

    DBUG_RETURN (arg_node);
}

node *
IVEOprf (node *arg_node, info *arg_info)
{
    node *ivarg;
    node *ivassign;

    DBUG_ENTER ("IVEOprf");

    if (PRF_PRF (arg_node) == F_vect2offset) {
        ivarg = PRF_ARG2 (arg_node);
        ivassign = AVIS_SSAASSIGN (ID_AVIS (ivarg));

        if (ivassign != NULL) {
            if (NODE_TYPE (ASSIGN_RHS (ivassign)) == N_array) {
                /*
                 * this index vector is defined as a array of
                 * scalars.
                 */
                arg_node = ReplaceByIdx2Offset (arg_node, arg_info);
            } else if (NODE_TYPE (ASSIGN_RHS (ivassign)) == N_prf) {
                /*
                 * this index vector is defined as a computation
                 * on (maybe) other index vectors or constants
                 */
                arg_node = OptimizeComputation (arg_node, arg_info);
            }
        } else {
            if (NODE_TYPE (AVIS_DECL (ID_AVIS (ivarg))) != N_arg) {
                /*
                 * this id has no defining assignment and is no argument.
                 * the only possible reason for this is, that this id is a
                 * withloop index vector.
                 */
                arg_node = ReplaceByWithOffset (arg_node, arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

node *
IVEOfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEOblock");

    INFO_VARDECS (arg_info) = NULL;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (INFO_VARDECS (arg_info) != NULL) {
        FUNDEF_VARDEC (arg_node)
          = TCappendVardec (INFO_VARDECS (arg_info), FUNDEF_VARDEC (arg_node));
        INFO_VARDECS (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IVEOassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEOassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);

        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
IVEOdoIndexVectorEliminationOptimisation (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IVEIdoIndexVectorEliminationOptimisation");

    TRAVpush (TR_iveo);

    info = MakeInfo ();

    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
