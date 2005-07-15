/*
 *
 * $Log$
 * Revision 3.159  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 3.158  2005/06/28 16:12:02  sah
 * removed a lot of warning
 * messages
 *
 * Revision 3.157  2005/06/27 21:23:15  sah
 * bugfix
 *
 * Revision 3.156  2005/06/14 10:50:48  ktr
 * changed GRID_MODIFIED and STRIDE_MODIFIED into flags X_ISMODIFIED
 *
 * Revision 3.155  2005/06/06 13:24:31  jhb
 * ccorrected copy behavior for N_ERROR
 *
 * Revision 3.154  2005/06/06 10:19:26  jhb
 * duplication of obsolete attributes
 *
 * Revision 3.153  2005/04/19 17:34:57  ktr
 * removed AVIS_SSAASSIGN2, AVIS_SUBSTUSSA
 *
 * Revision 3.152  2005/04/16 14:19:30  khf
 * DUPdefault added
 *
 * Revision 3.151  2005/03/04 21:21:42  cg
 * FUNDEF_USED counter etc removed.
 * Handling of FUNDEF_EXT_ASSIGNS drastically simplified.
 * LaC functions are now always duplicated along with the
 * corresponding application and silently introduced to the
 * fundef chain at save points.
 *
 * Revision 3.150  2005/02/16 22:29:13  sah
 * fixed DupArg/DupRet
 *
 * Revision 3.149  2005/02/16 14:34:56  jhb
 * added next to TBmakeError
 *
 * Revision 3.148  2005/02/03 14:25:09  jhb
 * changed NODE_ERROR to DUPTRAV
 *
 * Revision 3.147  2005/02/01 15:25:37  mwe
 * bug fixed (corrected pointer for AVIS_SSAASSIGN and AVIS_WITHID)
 *
 * Revision 3.146  2005/01/27 16:48:51  mwe
 * corrected duplication of fungroup in N_fundef
 *
 * Revision 3.145  2005/01/26 10:34:45  mwe
 * just edited last log message...
 *
 * Revision 3.144  2005/01/26 10:24:38  mwe
 * AVIS_SSACONST removed and replaced by usage of akv types
 *
 * Revision 3.143  2005/01/20 14:16:06  ktr
 * some bugfixing
 *
 * Revision 3.142  2005/01/19 14:18:12  jhb
 * chande chk in error
 *
 * Revision 3.141  2005/01/11 15:19:49  mwe
 * support for N_fungroup added
 *
 * Revision 3.140  2005/01/11 12:30:42  jhb
 * node CHK included
 *
 * Revision 3.139  2004/12/20 12:46:33  ktr
 * call to TBmakeWlgridvar corrected.
 *
 * Revision 3.138  2004/12/14 12:54:33  ktr
 * DUPids modified.
 *
 * Revision 3.137  2004/12/13 13:54:05  ktr
 * some changes made regarding DUPdoDupTreeSSA
 *
 * Revision 3.136  2004/12/09 12:26:27  sbs
 * never correct AP_NAMEs anymore pls!!!!
 * corrected severe problem in DUPcheckAndDupSpecialFundef
 *
 * Revision 3.135  2004/12/08 18:02:40  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 3.134  2004/12/07 20:35:53  ktr
 * eliminated CONSTVEC which is superseded by ntypes.
 *
 * Revision 3.133  2004/12/06 13:21:19  sah
 * added copying of flags to all other nodes
 * (hopefully)
 *
 * Revision 3.132  2004/12/06 11:55:56  sah
 * flag test
 *
 * Revision 3.131  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 3.130  2004/12/02 19:29:22  sbs
 * DUP_CONT usage in DUPops fixed *-)
 *
 * Revision 3.129  2004/12/02 15:12:29  sah
 * added support for ops node
 *
 * Revision 3.128  2004/12/01 18:53:15  sah
 * added some more SPxxx support
 *
 * Revision 3.127  2004/12/01 15:30:00  ktr
 * Old types are no longer mandatory
 *
 * Revision 3.126  2004/11/27 00:45:29  mwe
 * interface changes
 *
 * Revision 3.125  2004/11/26 18:22:05  mwe
 * DUPglobobj added
 *
 * Revision 3.123  2004/11/26 12:22:23  mwe
 * moved some macros from .c to .h file (needed by external functions)
 *
 * Revision 3.122  2004/11/26 10:58:25  mwe
 * changes according to changes in ast.xml
 *
 * Revision 3.121  2004/11/25 11:17:17  mwe
 * copy FUNDEF_WRAPPERTYPE
 *
 * Revision 3.120  2004/11/24 18:37:11  mwe
 * new DUP-functions added
 * only few functions implemented, other functions contain DBUG_ASSERT
 *
 * Revision 3.119  2004/11/24 12:27:43  mwe
 * TBmakeLet and N_pragma changed according to ast.xml
 *
 * Revision 3.118  2004/11/23 19:07:39  khf
 * SacDevCampDk: compiles!
 *
 * Revision 3.117  2004/11/22 21:35:54  khf
 * codebrushing part2
 *
 * Revision 3.116  2004/11/22 17:04:55  khf
 * the big 2004 codebrushing session (part1)
 *
 * Revision 3.115  2004/11/19 15:11:10  sah
 * removed NEEDOBJS
 * added OBJECTS
 *
 * Revision 3.114  2004/11/07 18:07:54  sah
 * disabled some code in NEW_AST mode
 *
 * Revision 3.113  2004/10/25 16:50:12  khf
 * added copying of new type in DupAssign and DupNwith
 *
 * Revision 3.112  2004/10/20 08:04:09  khf
 * added NWITH_MTO_OFFSET_NEEDED, NCODE_RESOLVEABLE_DEPEND
 *
 * Revision 3.111  2004/10/19 11:52:18  ktr
 * Added FUNDEF_RETALIAS
 *
 * Revision 3.110  2004/10/15 10:03:58  sah
 * removed old module system nodes in
 * new ast mode
 *
 * Revision 3.109  2004/10/15 09:09:14  ktr
 * added AVIS_ALIAS ARG_ALIAS
 *
 * Revision 3.108  2004/10/14 22:49:13  sbs
 * SHCopyShape in DupVinfo now conditional
 *
 * Revision 3.107  2004/10/14 13:38:34  sbs
 * adjusted DupVinfo
 *
 * Revision 3.106  2004/10/12 09:49:06  khf
 * WLGRID_CODE, WLGRIDVAR_CODE can be NULL!
 *
 * Revision 3.105  2004/10/11 14:57:53  sah
 * made INC/DEC NCODE_USED explicit 
 *
 * Revision 3.104  2004/10/07 12:12:45  sah
 * added NCODE_INC_USED macro
 *
 * Revision 3.103  2004/10/04 15:36:04  skt
 * xT_IDENTIFIER erased
 *
 * Revision 3.102  2004/09/28 14:11:18  ktr
 * removed old refcount and generatemasks
 *
 * Revision 3.101  2004/09/27 09:18:25  sah
 * now, for Avis nodes the new type is copied as well
 * see bug #64
 *
 * Revision 3.100  2004/08/30 14:26:58  skt
 * changed NWITH2_ISSCHEDULED into NWITH2_CALCPARALLEL
 *
 * Revision 3.99  2004/08/13 16:26:47  khf
 * added duplication of NWITHOP_OFFSET_NEEDED
 *
 * Revision 3.98  2004/08/06 13:10:15  ktr
 * NWITHOP_MEM is now duplicated as well.
 *
 * Revision 3.97  2004/08/05 11:37:55  ktr
 * New flag NWITHID_VECNEEDED indicates in EMM whether the index vector
 * must be maintained throughout the with-loop
 *
 * Revision 3.96  2004/07/31 13:51:14  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.95  2004/05/17 09:37:55  mwe
 * ARRAY_NTYPE added
 *
 * Revision 3.94  2004/05/12 12:58:27  ktr
 * DO_LABEL and DO_SKIP inserted.
 *
 * Revision 3.93  2004/05/07 09:58:43  khf
 * Added functions DupTreeSSA, DupTreeLUTSSA, DupNodeSSA and
 * DupNodeLUTSSA to obtain the ssa-form
 *
 * Revision 3.92  2004/05/05 14:35:56  ktr
 * Added support for NCODE_EPILOGUE
 *
 * Revision 3.91  2004/03/09 23:57:59  dkrHH
 * old backend removed
 *
 * Revision 3.90  2004/03/05 19:14:27  mwe
 * support for new node N_funcond added
 *
 * Revision 3.89  2004/03/04 17:36:11  mwe
 * DupCondfun removed
 *
 * Revision 3.88  2004/03/04 13:23:04  mwe
 * DupCondfun added
 *
 * Revision 3.87  2004/02/20 08:22:18  mwe
 * signature of MakeModul changed (new argument for FUNDECS)
 * updated DupTree: now duplicates also FUNDECS
 *
 * Revision 3.86  2004/02/06 14:26:21  mwe
 * PHIASSIGN removed
 *
 * Revision 3.85  2003/12/23 10:43:58  khf
 * NWITHOP_NEXT for more operations for withloop-fusion added. Other NWITHOP attributes
 * shifted. NCODE_CEXPR changed to NCODE_CEXPRS. Macro adjusted. Second MakeNCode
 * MakeNCodeExprs for expr from type N_exprs added.
 *
 * Revision 3.84  2003/11/18 16:46:06  dkr
 * DupNwithop(): NWITHOP_DEFAULT added
 *
 * Revision 3.83  2003/09/25 13:54:54  dkr
 * ID_UNQCONV removed
 *
 * Revision 3.82  2003/09/19 11:54:05  dkr
 * definition of DupNode_NT() simplified
 *
 * Revision 3.81  2003/09/16 18:19:11  ktr
 * Added support for AVIS_WITHID
 *
 * Revision 3.80  2003/06/11 22:03:09  ktr
 * The shape structure in N_array is now copied.
 *
 * Revision 3.79  2002/10/24 13:11:32  ktr
 * removed support for ASSIGN_INDENT
 *
 * Revision 3.78  2002/10/20 13:23:59  ktr
 * Added support for WLS N_assign indentation by adding field ASSIGN_INDENT(n) to N_assign
 * node which is increased on every indentation performed by WLS.
 *
 * Revision 3.77  2002/10/18 13:48:08  sbs
 * handling of ID_ATTRIB replaced by FLAG usages
 *
 * Revision 3.76  2002/10/16 11:42:10  sbs
 * OBJDEF_AVIS handling inserted.
 *
 * Revision 3.75  2002/09/09 11:51:23  dkr
 * DupFundef(): FUNDEF_IMPL added
 *
 * Revision 3.74  2002/09/06 12:17:19  sah
 * handling of N_setwl nodes modified.
 *
 * Revision 3.73  2002/09/06 10:36:09  sah
 * added DupSetWL
 *
 * Revision 3.72  2002/09/05 19:20:52  dkr
 * DupAllTypes(), DupAllIds(), DupNodelist(), DupShpseg() can applied
 * to NULL now
 *
 * Revision 3.71  2002/09/05 16:33:16  dkr
 * call of FreeAvis() replaced by FreeNode() !!!
 *
 * Revision 3.70  2002/08/15 11:47:39  dkr
 * type LUT_t* replaced by LUT_t
 *
 * Revision 3.69  2002/08/13 13:43:28  dkr
 * interface to LookUpTable.[ch] modified
 *
 * Revision 3.68  2002/08/12 14:58:52  sbs
 * N_mop representation changed
 *
 * Revision 3.67  2002/08/09 16:36:21  sbs
 * basic support for N_mop written.
 *
 * Revision 3.66  2002/07/03 16:55:16  dkr
 * ID_UNQCONV removed for new backend
 *
 * Revision 3.65  2002/07/02 09:27:59  dkr
 * DupExprs_NT() moved from compile.tagged.c to DupTree.c
 *
 * Revision 3.64  2002/07/02 09:20:29  dkr
 * DupNode_NT() added
 *
 * Revision 3.63  2002/06/25 14:01:53  sbs
 * DupDot added.
 *
 * Revision 3.62  2002/06/20 15:23:25  dkr
 * signature of MakeNWithOp modified
 *
 * Revision 3.61  2002/06/02 21:47:23  dkr
 * ID_NT_TAG modified
 *
 * Revision 3.60  2002/04/09 16:39:20  dkr
 * okay... *now* DupFundef() should work correctly $%&-(
 *
 * Revision 3.59  2002/04/08 20:03:51  dkr
 * bug in DupFundef() fixed
 *
 * Revision 3.58  2002/03/07 02:20:12  dkr
 * duplication of AP_ARGTAB and FUNDEF_ARGTAB added
 *
 * Revision 3.57  2002/03/01 02:40:09  dkr
 * DupFundef() and DupAp() modified: ARGTAB is duplicated now
 *
 * Revision 3.56  2002/02/22 14:27:53  dkr
 * functions Dup...TypesOnly(), DupOneTypesOnly_Inplace() removed
 *
 * Revision 3.55  2002/02/20 16:09:31  dkr
 * function DupOneTypesOnly_Inplace() added
 *
 * Revision 3.54  2002/02/20 15:02:11  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 * fundef DupTypesOnly() renamed into DupAllTypesOnly()
 * fundefs DupOneTypes() and DupOneTypesOnly() added
 *
 * Revision 3.53  2001/12/12 11:14:39  dkr
 * functions DupIds_Id_NT, DupId_NT added
 *
 * Revision 3.52  2001/06/27 12:38:37  ben
 * SCHCopyTasksel inserted
 *
 * Revision 3.51  2001/05/18 11:40:31  dkr
 * DBUG_ASSERT containing IsEmptyLUT() added
 *
 * Revision 3.50  2001/05/17 11:37:24  dkr
 * InitDupTree() added
 * FREE/MALLOC eliminated
 *
 * Revision 3.49  2001/05/16 13:17:37  dkr
 * handling of NULL arguments streamlined:
 * NULL arguments allowed for static ..._() functions but *not* allowed
 * for exported functions.
 *
 * Revision 3.48  2001/05/03 17:26:20  dkr
 * MAXHOMDIM replaced by HOMSV
 *
 * Revision 3.47  2001/05/03 16:50:08  nmw
 * increment fundef used counter in DupIcm for ND_AP icm
 *
 * Revision 3.46  2001/04/26 21:06:21  dkr
 * DupTypesOnly() added
 *
 * Revision 3.45  2001/04/26 11:55:56  nmw
 * ICM_FUNDEF attribute added to DupIcm
 *
 * Revision 3.44  2001/04/26 01:50:04  dkr
 * - reference counnting on fundefs works correctly now (FUNDEF_USED)
 * - DFMs are never duplicated now!!
 *
 * Revision 3.43  2001/04/24 14:15:01  dkr
 * some DBUG_ASSERTs about FUNDEF_USED added
 *
 * Revision 3.42  2001/04/24 09:16:21  dkr
 * P_FORMAT replaced by F_PTR
 *
 * Revision 3.41  2001/04/06 18:46:17  dkr
 * - DupAvis: call of DupTree replaced by DUPTRAV.
 * - The LUT is no longer allocated and removed for each call of
 *   DupTree/DupNode. Instead, we generate *once* a single LUT and
 *   reuse it for further calls.
 *
 * [ ... ]
 *
 */

/******************************************************************************
 *
 * file  : DupTree.c
 *
 * PREFIX: Dup
 *
 * description:
 *   Traversal for duplication of nodes and trees.
 *
 * flags for some special behaviour ('type'):
 *   DUP_NORMAL : no special behaviour
 *   DUP_INLINE : do not duplicate N_assign nodes which contain a N_return
 *   DUP_WLF    : set ID_WL
 *
 * CAUTION:
 *   Do *NOT* call the external functions Dup...() within a DUP-traversal!!!!
 *   For duplicating nodes DUPTRAV should be used and for duplicating non-node
 *   structures appropriate Dup..._() functions are provided!
 *
 *   When adding new functions: Please name top-level functions Dup...()
 *   *without* trailing '_' and name static functions Dup..._() *with*
 *   trailing '_'.
 *
 ******************************************************************************/

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "shape.h"
#include "free.h"
#include "dbug.h"
#include "new_types.h"
#include "DupTree.h"
#include "NameTuplesUtils.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "LookUpTable.h"
#include "scheduling.h"
#include "constants.h"
#include "stringset.h"
#include "namespaces.h"

/*
 * INFO structure
 */
struct INFO {
    int type;
    node *cont;
    node *fundef;
    lut_t *lut;
    bool inspecial;
    node *assign;
    node *fundefssa;
    node *withid;
};

/*
 * INFO macros
 */
#define INFO_DUP_TYPE(n) (n->type)
#define INFO_DUP_CONT(n) (n->cont)
#define INFO_DUP_FUNDEF(n) (n->fundef)
#define INFO_DUP_LUT(n) (n->lut)
#define INFO_DUP_INSPECIAL(n) (n->inspecial)
#define INFO_DUP_ASSIGN(n) (n->assign)
#define INFO_DUP_FUNDEFSSA(n) (n->fundefssa)
#define INFO_DUP_WITHID(n) (n->withid)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_DUP_TYPE (result) = 0;
    INFO_DUP_CONT (result) = NULL;
    INFO_DUP_FUNDEF (result) = NULL;
    INFO_DUP_LUT (result) = NULL;
    INFO_DUP_INSPECIAL (result) = FALSE;
    INFO_DUP_ASSIGN (result) = NULL;
    INFO_DUP_FUNDEFSSA (result) = NULL;
    INFO_DUP_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * DFMs are *not* duplicated for several reasons:
 *  - Most likely the old DFMs are not suitable for the new context.
 *  - Only valid DFMs can be duplicated. Therefore, duplicating the
 *    DFMs requires them to be valid during all compilation phases.
 *    For the time being this cannot be guaranteed.
 */
#define DUP_DFMS 0

static lut_t *dup_lut = NULL;
static node *store_copied_special_fundefs = NULL;

/*
 * always traverses son 'node'
 *
 * The macro is to be used within traversal functions where arg_node and
 * arg_info exist.
 */
#define DUPTRAV(node) ((node) != NULL) ? TRAVdo (node, arg_info) : NULL

/*
 * If INFO_DUP_CONT contains the root of syntaxtree
 *   -> traverses son 'node' if and only if its parent is not the root
 * If INFO_DUP_CONT is NULL
 *   -> traverses son 'node'
 *
 * The macro is to be used within traversal functions where arg_node and
 * arg_info exist.
 */
#define DUPCONT(node) (INFO_DUP_CONT (arg_info) != arg_node) ? DUPTRAV (node) : NULL

/******************************************************************************
 *
 * Function:
 *   void DUPinitDupTree()
 *
 * Description:
 *
 *
 ******************************************************************************/

void
DUPinitDupTree ()
{
    DBUG_ENTER ("DUPinitDupTree");

    DBUG_ASSERT ((dup_lut == NULL), "DUPinitDupTree() called more than once!");
    dup_lut = LUTgenerateLut ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   static node *DupTreeOrNodeLutType( bool NodeOnly,
 *                                      node *arg_node, lut_t *lut, int type,
 *                                      node *fundef)
 *
 * description:
 *   This routine starts a duplication-traversal, it duplicates a whole sub
 *   tree or one node only (that means all of this node, but not the xxx_NEXT).
 *   The start of the duplication is at arg_node, either the subtree starting
 *   from this node or the node only is copied.
 *
 * parameters:
 *   - NodeOnly:
 *       FALSE : duplicate whole subtree
 *       TRUE  : duplicate node only
 *   - arg_node:
 *       starting point of duplication
 *   - lut:
 *       If you want to use your own LUT you can hand over it here.
 *   - type:
 *       value for INFO_DUP_TYPE.
 *   - fundef:
 *       optional: only necessary if type value == DUP_SSA. It is
 *                 required for new vardecs.
 *                 Else it points to NULL.
 *
 ******************************************************************************/

static node *
DupTreeOrNodeLutType (bool node_only, node *arg_node, lut_t *lut, int type, node *fundef)
{
    info *arg_info;
    node *new_node;

    DBUG_ENTER ("DupTreeOrNodeLutType");

    if (arg_node != NULL) {
        arg_info = MakeInfo ();

        INFO_DUP_TYPE (arg_info) = type;
        INFO_DUP_ASSIGN (arg_info) = NULL;
        INFO_DUP_FUNDEF (arg_info) = NULL;
        INFO_DUP_FUNDEFSSA (arg_info) = fundef;

        /*
         * Via this (ugly) macro DUPCONT the decision to copy the whole tree
         * starting from arg_node or only the node itself (meaning not to
         * traverse and copy xxx_NEXT) is done.
         * DUPCONT compares the actual arg_node of a traversal function with the
         * value in INFO_DUP_CONT. If they are the same the xxx_NEXT will be
         * ignored, otherwise it will be traversed. If the start-node is stored as
         * INFO_DUP_CONT it's xx_NEXT will not be duplicated, but the xxx_NEXT's
         * of all sons are copied, because they differ from INFO_DUP_CONT.
         * If NULL is stored in INFO_DUP_CONT (and in a traversal the arg_node
         * never is NULL) all nodes and their xxx_NEXT's are duplicated.
         * So we set INFO_DUP_CONT with NULL to copy all, arg_node to copy
         * start_node (as decribed above) only.
         */
        if (node_only) {
            INFO_DUP_CONT (arg_info) = arg_node;
        } else {
            INFO_DUP_CONT (arg_info) = NULL;
        }

        if (lut == NULL) {
            /*
             * It would be extremly unefficient to generate a new LUT for each call
             * of DupTree/DupNode.
             * Therefore, we just generate it *once* via InitDupTree() !!
             */
            DBUG_ASSERT ((dup_lut != NULL), "DUPinitDupTree() has not been called!");
            DBUG_ASSERT ((LUTisEmptyLut (dup_lut)), "LUT for DupTree is not empty!");
            INFO_DUP_LUT (arg_info) = dup_lut;
        } else {
            INFO_DUP_LUT (arg_info) = lut;
        }

        TRAVpush (TR_dup);
        new_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();

        if (lut == NULL) {
            /*
             * Here, we just remove the content of the LUT but *not* the LUT itself.
             * Guess what: Most likely we will need the LUT again soon ;-)
             */
            dup_lut = LUTremoveContentLut (dup_lut);
        }

        arg_info = FreeInfo (arg_info);
    } else {
        new_node = NULL;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupTreeTravPre( node *arg_node, info *arg_info)
 *
 * description:
 *   This function is called before the traversal of each node.
 *
 ******************************************************************************/

node *
DUPtreeTravPre (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DUPtreeTravPre");

    DBUG_PRINT ("DUP", ("Duplicating - %s", NODE_TEXT (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DUPdupTreeTravPost( node *arg_node, info *arg_info)
 *
 * description:
 *   This function is called after the traversal of each node.
 *
 ******************************************************************************/

node *
DUPtreeTravPost (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DUPtreeTravPost");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DupFlags( node *new_node, node *old_node)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
DupFlags (node *new_node, node *old_node)
{
    DBUG_ENTER ("DupFlags");

    /* TODO : Copy of flagvector (has to be done by sah) */

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * Function:
 *   void CopyCommonNodeData( node *new_node, node *old_node)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
CopyCommonNodeData (node *new_node, node *old_node)
{
    DBUG_ENTER ("CopyCommonNodeData");

    NODE_LINE (new_node) = NODE_LINE (old_node);
    NODE_FILE (new_node) = NODE_FILE (old_node);

    if (NODE_ERROR (new_node) != NULL) {
        NODE_ERROR (new_node) = DUPerror (NODE_ERROR (old_node), NULL);
    }

    new_node = DupFlags (new_node, old_node);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   dfmask_t* DupDfmask( dfmask_t* mask, info *arg_info)
 *
 * Description:
 *   Duplicates the given dfmask.
 *   The real duplication is done by DFMDuplicateMask:
 *   If a new DFMbase is found in the LUT the new one is used,
 *   otherwise the old one (this is done by the LUTmechanismi, called
 *   within this function).
 *
 ******************************************************************************/

static dfmask_t *
DupDfmask (dfmask_t *mask, info *arg_info)
{
    dfmask_t *new_mask;

    DBUG_ENTER ("DupDfmask");

#if DUP_DFMS
    if (mask != NULL) {
        new_mask = DFMduplicateMask (mask, LUTsearchInLutPp (INFO_DUP_LUT (arg_info),
                                                             DFMgetMaskBase (mask)));
    } else {
        new_mask = NULL;
    }
#else
    new_mask = NULL;
#endif

    DBUG_RETURN (new_mask);
}

/******************************************************************************
 *
 * Function:
 *   shpseg *DupShpseg( shpseg *arg_shpseg, info *arg_info)
 *
 * Remark:
 *   'arg_info' might be NULL, because this function is not only used by
 *   the traversal mechanism but also by DupShpseg()!
 *
 ******************************************************************************/

static shpseg *
DupShpseg (shpseg *arg_shpseg, info *arg_info)
{
    int i;
    shpseg *new_shpseg;

    DBUG_ENTER ("DupShpseg");

    if (arg_shpseg != NULL) {
        new_shpseg = TBmakeShpseg (NULL);
        for (i = 0; i < SHP_SEG_SIZE; i++) {
            SHPSEG_SHAPE (new_shpseg, i) = SHPSEG_SHAPE (arg_shpseg, i);
        }

        SHPSEG_NEXT (new_shpseg) = DupShpseg (SHPSEG_NEXT (arg_shpseg), arg_info);
    } else {
        new_shpseg = NULL;
    }

    DBUG_RETURN (new_shpseg);
}

/******************************************************************************
 *
 * Function:
 *   types *DupTypes( types* source, info *arg_info)
 *
 * Remark:
 *   'arg_info' might be NULL, because this function is not only used by
 *   the traversal mechanism but also by Dup...Types()!
 *
 ******************************************************************************/

static types *
DupTypes (types *arg_types, info *arg_info)
{
    types *new_types;

    DBUG_ENTER ("DupTypes");

    if (arg_types != NULL) {
        new_types = TBmakeTypes (TYPES_BASETYPE (arg_types), TYPES_DIM (arg_types),
                                 DupShpseg (TYPES_SHPSEG (arg_types), arg_info),
                                 ILIBstringCopy (TYPES_NAME (arg_types)),
                                 ILIBstringCopy (TYPES_MOD (arg_types)));

        TYPES_TDEF (new_types) = TYPES_TDEF (arg_types);
        TYPES_STATUS (new_types) = TYPES_STATUS (arg_types);

        DBUG_PRINT ("TYPE", ("new type" F_PTR ",old " F_PTR, new_types, arg_types));
        DBUG_PRINT ("TYPE", ("new name" F_PTR ", old name" F_PTR, TYPES_NAME (new_types),
                             TYPES_NAME (arg_types)));

        TYPES_NEXT (new_types) = DupTypes (TYPES_NEXT (arg_types), arg_info);

        if (arg_info != NULL) {
            INFO_DUP_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_types, new_types);
        }
    } else {
        new_types = NULL;
    }

    DBUG_RETURN (new_types);
}

/******************************************************************************
 *
 * Function:
 *   nodelist *DupNodelist( nodelist *nl, info *arg_info)
 *
 * Remark:
 *   'arg_info' might be NULL, because this function is not only used by
 *   the traversal mechanism but also by DupNodelist()!
 *
 ******************************************************************************/

static nodelist *
DupNodelist (nodelist *nl, info *arg_info)
{
    nodelist *new_nl;

    DBUG_ENTER ("DupNodelist");

    if (nl != NULL) {
        new_nl = TBmakeNodelist (LUTsearchInLutPp (INFO_DUP_LUT (arg_info),
                                                   NODELIST_NODE (nl)),
                                 NODELIST_STATUS (nl),
                                 DupNodelist (NODELIST_NEXT (nl), arg_info));
        NODELIST_ATTRIB (new_nl) = NODELIST_ATTRIB (nl);
    } else {
        new_nl = NULL;
    }

    DBUG_RETURN (new_nl);
}

/******************************************************************************
 *
 * Function:
 *   argtab_t *DupArgtab( argtab_t *argtab, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static argtab_t *
DupArgtab (argtab_t *argtab, info *arg_info)
{
    argtab_t *new_argtab;
    int i;

    DBUG_ENTER ("DupArgtab");

    if (argtab != NULL) {
        new_argtab = TBmakeArgtab (argtab->size);

        for (i = 0; i < argtab->size; i++) {
            new_argtab->tag[i] = argtab->tag[i];
            new_argtab->ptr_in[i]
              = (argtab->ptr_in[i] != NULL)
                  ? LUTsearchInLutPp (INFO_DUP_LUT (arg_info), argtab->ptr_in[i])
                  : NULL;
            new_argtab->ptr_out[i]
              = (argtab->ptr_out[i] != NULL)
                  ? LUTsearchInLutPp (INFO_DUP_LUT (arg_info), argtab->ptr_out[i])
                  : NULL;
        }
    } else {
        new_argtab = NULL;
    }

    DBUG_RETURN (new_argtab);
}

/******************************************************************************/

node *
DUPvinfo (node *arg_node, info *arg_info)
{
    node *new_node, *rest;

    DBUG_ENTER ("DUPvinfo");

    rest = DUPCONT (VINFO_NEXT (arg_node));

    if (VINFO_FLAG (arg_node) == DOLLAR) {
        new_node = TCmakeVinfoDollar (rest);
    } else {
        new_node = TBmakeVinfo (VINFO_FLAG (arg_node),
                                (VINFO_SHAPE (arg_node) == NULL
                                   ? NULL
                                   : SHcopyShape (VINFO_SHAPE (arg_node))),
                                VINFO_DOLLAR (rest), rest);
    }
    VINFO_VARDEC (new_node) = VINFO_VARDEC (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPnum (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPnum");

    new_node = TBmakeNum (NUM_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPbool (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPbool");

    new_node = TBmakeBool (BOOL_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPfloat (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPfloat");

    new_node = TBmakeFloat (FLOAT_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPdouble (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPdouble");

    new_node = TBmakeDouble (DOUBLE_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPchar (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPchar");

    new_node = TBmakeChar (CHAR_VAL (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPstr (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPstr");

    new_node = TBmakeStr (ILIBstringCopy (STR_STRING (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPdot (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPdot");

    new_node = TBmakeDot (DOT_NUM (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPsetwl (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPsetwl");

    new_node = TBmakeSetwl (TRAVdo (SETWL_VEC (arg_node), arg_info),
                            TRAVdo (SETWL_EXPR (arg_node), arg_info));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPid (node *arg_node, info *arg_info)
{
    node *new_node, *avis;

    DBUG_ENTER ("DUPid");

    avis = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), ID_AVIS (arg_node));
    new_node = TBmakeId (avis);

    if (INFO_DUP_TYPE (arg_info) == DUP_WLF) {
        /* Withloop folding (wlf) needs this. */
        if (ID_WL (arg_node) && (NODE_TYPE (ID_WL (arg_node)) == N_id)) {
            /* new code in new_codes, see 'usage of ID_WL' in WLF.c for more infos */
            ID_WL (new_node) = ID_WL (arg_node);
        } else {
            ID_WL (new_node) = arg_node; /* original code */
        }
    }

    if (ID_NT_TAG (arg_node) != NULL) {
        ID_NT_TAG (new_node) = ILIBstringCopy (ID_NT_TAG (arg_node));
    }

    /*
     * furthermore, we have to copy the ICMTEXT attribute
     * if it still exists
     */
    ID_ICMTEXT (new_node) = ILIBstringCopy (ID_ICMTEXT (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPspid (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPspid");

    new_node = TBmakeSpid (NSdupNamespace (SPID_NS (arg_node)),
                           ILIBstringCopy (SPID_NAME (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPcast (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPcast");

    new_node
      = TBmakeCast (TYcopyType (CAST_NTYPE (arg_node)), DUPTRAV (CAST_EXPR (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPmodule (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPmodule");

    new_node
      = TBmakeModule (NSdupNamespace (MODULE_NAMESPACE (arg_node)),
                      MODULE_FILETYPE (arg_node), DUPTRAV (MODULE_IMPORTS (arg_node)),
                      DUPTRAV (MODULE_TYPES (arg_node)), DUPTRAV (MODULE_OBJS (arg_node)),
                      DUPTRAV (MODULE_FUNS (arg_node)),
                      DUPTRAV (MODULE_FUNDECS (arg_node)));

    MODULE_CLASSTYPE (new_node) = TYcopyType (MODULE_CLASSTYPE (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPtypedef (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPtypedef");

    new_node = TBmakeTypedef (ILIBstringCopy (TYPEDEF_NAME (arg_node)),
                              NSdupNamespace (TYPEDEF_NS (arg_node)),
                              TYcopyType (TYPEDEF_NTYPE (arg_node)),
                              DUPCONT (TYPEDEF_NEXT (arg_node)));

    TYPEDEF_FLAGSTRUCTURE (new_node) = TYPEDEF_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPobjdef (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPobjdef");

    new_node
      = TBmakeObjdef (TYcopyType (OBJDEF_TYPE (arg_node)),
                      NSdupNamespace (OBJDEF_NS (arg_node)),
                      ILIBstringCopy (OBJDEF_NAME (arg_node)),
                      DUPTRAV (OBJDEF_EXPR (arg_node)), DUPCONT (OBJDEF_NEXT (arg_node)));

    OBJDEF_FLAGSTRUCTURE (new_node) = OBJDEF_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPfundef (node *arg_node, info *arg_info)
{
    node *new_node, *old_fundef, *new_ssacnt;

    DBUG_ENTER ("DUPfundef");

    /*
     * DUP_INLINE
     *  -> N_return nodes are not duplicated
     *  -> would result in an illegal N_fundef node
     *  -> stop here!
     */
    DBUG_ASSERT ((INFO_DUP_TYPE (arg_info) != DUP_INLINE),
                 "N_fundef node can't be duplicated in DUP_INLINE mode!");

    DBUG_PRINT ("DUP", ("start dubbing of fundef %s", FUNDEF_NAME (arg_node)));

    /*
     * We can't copy the FUNDEF_DFM_BASE and DFMmasks belonging to this base
     * directly!
     * Such DFMmasks are attached to N_with, N_with2, N_sync and N_spmd.
     * All of them can be found in the body of the function.
     * But when we copy the body we must already know the base to create the
     * new masks. On the other hand to create the base we must already have
     * the new FUNDEF_ARGS and FUNDEF_VARDECS available.
     * Therefore we first create the raw function without the body via
     * MakeFundef(), then we create the base while duplicating the body
     * and finally we attach the body to the fundef.
     */

    /*
     * INFO_DUP_FUNDEF is a pointer to the OLD fundef node!!
     * We can get the pointer to the NEW fundef via the LUT!
     */
    old_fundef = INFO_DUP_FUNDEF (arg_info);
    INFO_DUP_FUNDEF (arg_info) = arg_node;

    new_node = TBmakeFundef (ILIBstringCopy (FUNDEF_NAME (arg_node)),
                             NSdupNamespace (FUNDEF_NS (arg_node)),
                             DUPTRAV (FUNDEF_RETS (arg_node)),
                             NULL, /* must be duplicated later on */
                             NULL, /* must be duplicated later on */
                             NULL);

    /* now we copy all the other things ... */
    FUNDEF_FUNNO (new_node) = FUNDEF_FUNNO (arg_node);
    FUNDEF_PRAGMA (new_node) = DUPTRAV (FUNDEF_PRAGMA (arg_node));
    FUNDEF_VARNO (new_node) = FUNDEF_VARNO (arg_node);
    FUNDEF_FLAGSTRUCTURE (new_node) = FUNDEF_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    FUNDEF_NEXT (new_node) = DUPCONT (FUNDEF_NEXT (arg_node));

    /*
     * must be done before traversal of BODY
     */
    if (FUNDEF_ISLACFUN (new_node)) {
        FUNDEF_EXT_ASSIGN (new_node) = NULL;
    }

    /*
     * before duplicating ARGS or VARDEC (in BODY) we have to duplicate
     * SSACOUNTER (located in the top-block, but traversed here)
     */
    if ((FUNDEF_BODY (arg_node) != NULL)
        && (BLOCK_SSACOUNTER (FUNDEF_BODY (arg_node)) != NULL)) {
        new_ssacnt = DUPTRAV (BLOCK_SSACOUNTER (FUNDEF_BODY (arg_node)));
    } else {
        new_ssacnt = NULL;
    }

    INFO_DUP_LUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_node, new_node);

    FUNDEF_ARGS (new_node) = DUPTRAV (FUNDEF_ARGS (arg_node));
    FUNDEF_BODY (new_node) = DUPTRAV (FUNDEF_BODY (arg_node));

    /*
     * ARGTAB must be duplicated *after* TYPES and ARGS!!!
     */
    FUNDEF_ARGTAB (new_node) = DupArgtab (FUNDEF_ARGTAB (arg_node), arg_info);

    if (FUNDEF_BODY (new_node) != NULL) {
        BLOCK_SSACOUNTER (FUNDEF_BODY (new_node)) = new_ssacnt;
    }

    /*
     * must be done after traversal of BODY
     */

#if DUP_DFMS
    FUNDEF_DFM_BASE (new_node)
      = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), FUNDEF_DFM_BASE (arg_node));
#else
    FUNDEF_DFM_BASE (new_node) = NULL;
#endif

    FUNDEF_RETURN (new_node)
      = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), FUNDEF_RETURN (arg_node));

    FUNDEF_IMPL (new_node)
      = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), FUNDEF_IMPL (arg_node));

    if (FUNDEF_ISDOFUN (new_node)) {
        FUNDEF_INT_ASSIGN (new_node)
          = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), FUNDEF_INT_ASSIGN (arg_node));
    }

    if (FUNDEF_WRAPPERTYPE (arg_node) != NULL) {
        FUNDEF_WRAPPERTYPE (new_node) = TYcopyType (FUNDEF_WRAPPERTYPE (arg_node));
    }

    if ((!FUNDEF_ISLACFUN (arg_node)) && (FUNDEF_FUNGROUP (arg_node) != NULL)) {

        FUNDEF_FUNGROUP (new_node) = FUNDEF_FUNGROUP (arg_node);
        /*
         * increse reference counter in fungroup and add new_node to funlist
         */
        (FUNGROUP_REFCOUNTER (FUNDEF_FUNGROUP (new_node))) += 1;

        FUNGROUP_FUNLIST (FUNDEF_FUNGROUP (new_node))
          = TBmakeLinklist (new_node, FUNGROUP_FUNLIST (FUNDEF_FUNGROUP (new_node)));
    } else {
        FUNDEF_FUNGROUP (new_node) = NULL;
    }

    INFO_DUP_FUNDEF (arg_info) = old_fundef;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUParg (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUParg");

    new_node = TBmakeArg (DUPTRAV (ARG_AVIS (arg_node)), DUPCONT (ARG_NEXT (arg_node)));

    ARG_TYPE (new_node) = DupTypes (ARG_TYPE (arg_node), arg_info);

    ARG_VARNO (new_node) = ARG_VARNO (arg_node);
    ARG_OBJDEF (new_node) = ARG_OBJDEF (arg_node);
    ARG_LINKSIGN (new_node) = ARG_LINKSIGN (arg_node);
    ARG_FLAGSTRUCTURE (new_node) = ARG_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_node, new_node);

    /* correct backreference */
    AVIS_DECL (ARG_AVIS (new_node)) = new_node;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPret (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPret");

    new_node
      = TBmakeRet (TYcopyType (RET_TYPE (arg_node)), DUPCONT (RET_NEXT (arg_node)));

    RET_LINKSIGN (new_node) = RET_LINKSIGN (arg_node);

    RET_FLAGSTRUCTURE (new_node) = RET_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPexprs (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPexprs");

    new_node
      = TBmakeExprs (DUPTRAV (EXPRS_EXPR (arg_node)), DUPCONT (EXPRS_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPblock (node *arg_node, info *arg_info)
{
    node *new_vardecs;
    node *new_node;
#if DUP_DFMS
    DFMmask_base_t old_base, new_base;
#endif

    DBUG_ENTER ("DUPblock");

    new_vardecs = DUPTRAV (BLOCK_VARDEC (arg_node));

#if DUP_DFMS
    if (INFO_DUP_FUNDEF (arg_info) != NULL) {
        old_base = FUNDEF_DFM_BASE (INFO_DUP_FUNDEF (arg_info));
    } else {
        old_base = NULL;
    }

    /*
     * If the current block is the top most block of the current fundef
     * we have to copy FUNDEF_DFMBASE.
     * Look at DupFundef() for further comments.
     */
    if ((old_base != NULL) && (arg_node == FUNDEF_BODY (INFO_DUP_FUNDEF (arg_info)))) {
        new_base
          = DFMgenMaskBase (FUNDEF_ARGS (LUTsearchInLutPp (INFO_DUP_LUT (arg_info),
                                                           INFO_DUP_FUNDEF (arg_info))),
                            new_vardecs);

        INFO_DUP_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), old_base, new_base);
    }
#endif

    new_node = TBmakeBlock (DUPTRAV (BLOCK_INSTR (arg_node)), new_vardecs);
    BLOCK_CACHESIM (new_node) = ILIBstringCopy (BLOCK_CACHESIM (arg_node));

    /*
     * BLOCK_SSACOUNTER is adjusted correctly later on by DupFundef()
     */
    BLOCK_SSACOUNTER (new_node) = BLOCK_SSACOUNTER (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPvardec (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPvardec");

    new_node
      = TBmakeVardec (DUPTRAV (VARDEC_AVIS (arg_node)), DUPCONT (VARDEC_NEXT (arg_node)));

    VARDEC_TYPE (new_node) = DupTypes (VARDEC_TYPE (arg_node), arg_info);

    VARDEC_VARNO (new_node) = VARDEC_VARNO (arg_node);
    VARDEC_OBJDEF (new_node) = VARDEC_OBJDEF (arg_node);
    VARDEC_FLAGSTRUCTURE (new_node) = VARDEC_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_node, new_node);
    /* correct backreference */
    AVIS_DECL (VARDEC_AVIS (new_node)) = new_node;

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPreturn (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPreturn");

    new_node = TBmakeReturn (DUPTRAV (RETURN_EXPRS (arg_node)));

    RETURN_REFERENCE (new_node) = DUPTRAV (RETURN_REFERENCE (arg_node));

    RETURN_CRET (new_node)
      = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), RETURN_CRET (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    INFO_DUP_LUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPempty (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPempty");

    new_node = TBmakeEmpty ();

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPassign (node *arg_node, info *arg_info)
{
    node *new_node;
    node *stacked_assign;

    DBUG_ENTER ("DUPassign");

    if ((INFO_DUP_TYPE (arg_info) != DUP_INLINE)
        || (NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_return)) {

        new_node = TBmakeAssign (NULL, NULL);

#if 0
    if (INFO_DUP_TYPE( arg_info) == DUP_SSA){
      /*
       * to keep the ssa-form we have to create new ids
       * and insert them into LUT 
       */

      oldids = ASSIGN_LHS( arg_node);
      while (oldids != NULL) {
        nvarname = ILIBtmpVarName( IDS_NAME( oldids));

        newavis = TBmakeAvis( nvarname, TYcopyType( IDS_NTYPE( oldids)));
        AVIS_SSAASSIGN( newavis) = new_node;

        newids = TBmakeIds( newavis, NULL);
        
        vardec =TBmakeVardec( IDS_AVIS( newids), NULL);

        if ( IDS_TYPE( oldids) != NULL) {
          VARDEC_TYPE( vardec) = DUPdupOneTypes( IDS_TYPE( oldids));
        }

        INFO_DUP_FUNDEFSSA( arg_info) 
          = TCaddVardecs( INFO_DUP_FUNDEFSSA( arg_info), vardec);

        INFO_DUP_LUT( arg_info) = LUTinsertIntoLutS( INFO_DUP_LUT( arg_info),
                                    IDS_NAME(oldids),    IDS_NAME(newids));
        INFO_DUP_LUT( arg_info) = LUTinsertIntoLutP( INFO_DUP_LUT( arg_info),
                                    IDS_DECL(oldids), IDS_DECL(newids));
        INFO_DUP_LUT( arg_info) = LUTinsertIntoLutP( INFO_DUP_LUT( arg_info),
                                    IDS_AVIS(oldids),    IDS_AVIS(newids));
        oldids = IDS_NEXT(oldids);
      }      
    }
#endif

        stacked_assign = INFO_DUP_ASSIGN (arg_info);
        INFO_DUP_ASSIGN (arg_info) = new_node;

        ASSIGN_INSTR (new_node) = DUPTRAV (ASSIGN_INSTR (arg_node));

        INFO_DUP_ASSIGN (arg_info) = stacked_assign;

        ASSIGN_NEXT (new_node) = DUPCONT (ASSIGN_NEXT (arg_node));

        INFO_DUP_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_node, new_node);

        ASSIGN_FLAGSTRUCTURE (new_node) = ASSIGN_FLAGSTRUCTURE (arg_node);

        CopyCommonNodeData (new_node, arg_node);
    } else {
        new_node = NULL;
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPcond (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPcond");

    new_node = TBmakeCond (DUPTRAV (COND_COND (arg_node)), DUPTRAV (COND_THEN (arg_node)),
                           DUPTRAV (COND_ELSE (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPdo (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPdo");

    new_node = TBmakeDo (DUPTRAV (DO_COND (arg_node)), DUPTRAV (DO_BODY (arg_node)));

    DO_SKIP (new_node) = DUPTRAV (DO_SKIP (arg_node));
    DO_LABEL (new_node)
      = (DO_LABEL (arg_node) != NULL ? ILIBtmpVarName (DO_LABEL (arg_node)) : NULL);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwhile (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPwhile");

    new_node
      = TBmakeWhile (DUPTRAV (WHILE_COND (arg_node)), DUPTRAV (WHILE_BODY (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPlet (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPlet");

    new_node = TBmakeLet (DUPTRAV (LET_IDS (arg_node)), NULL);

    /*
     * EXPR must be traversed after IDS (for AP_ARGTAB)
     */
    LET_EXPR (new_node) = DUPTRAV (LET_EXPR (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPids (node *arg_node, info *arg_info)
{
    node *new_node, *avis;

    DBUG_ENTER ("DUPids");

    if ((INFO_DUP_TYPE (arg_info) == DUP_SSA)
        && (LUTsearchInLutPp (INFO_DUP_LUT (arg_info), IDS_AVIS (arg_node))
            == IDS_AVIS (arg_node))) {
        node *newavis;
        /*
         * To maintain SSA form, new variables must be generated
         */
        newavis = TBmakeAvis (ILIBtmpVarName (IDS_NAME (arg_node)),
                              TYcopyType (IDS_NTYPE (arg_node)));

        if (AVIS_SSAASSIGN (IDS_AVIS (arg_node)) != NULL) {
            AVIS_SSAASSIGN (newavis) = INFO_DUP_ASSIGN (arg_info);
        }

        AVIS_ISALIAS (newavis) = AVIS_ISALIAS (IDS_AVIS (arg_node));

        FUNDEF_VARDEC (INFO_DUP_FUNDEFSSA (arg_info))
          = TBmakeVardec (newavis, FUNDEF_VARDEC (INFO_DUP_FUNDEFSSA (arg_info)));

        if (IDS_TYPE (arg_node) != NULL) {
            VARDEC_TYPE (AVIS_DECL (newavis)) = DUPdupOneTypes (IDS_TYPE (arg_node));
        }

        INFO_DUP_LUT (arg_info)
          = LUTinsertIntoLutS (INFO_DUP_LUT (arg_info), IDS_NAME (arg_node),
                               AVIS_NAME (newavis));

        INFO_DUP_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), IDS_DECL (arg_node),
                               AVIS_DECL (newavis));

        INFO_DUP_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), IDS_AVIS (arg_node), newavis);
    }

    avis = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), IDS_AVIS (arg_node));

    if ((INFO_DUP_WITHID (arg_info) != NULL) && (AVIS_WITHID (avis) != NULL)) {
        AVIS_WITHID (avis) = INFO_DUP_WITHID (arg_info);
    } else if ((INFO_DUP_ASSIGN (arg_info) != NULL) && (AVIS_SSAASSIGN (avis) != NULL)) {
        AVIS_SSAASSIGN (avis) = INFO_DUP_ASSIGN (arg_info);
    }

    new_node = TBmakeIds (avis, DUPCONT (IDS_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPspids (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPspids");

    new_node = TBmakeSpids (ILIBstringCopy (SPIDS_NAME (arg_node)),
                            DUPCONT (SPIDS_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPap (node *arg_node, info *arg_info)
{
    node *old_fundef, *new_fundef;
    node *new_node;

    DBUG_ENTER ("DUPap");

    DBUG_PRINT ("DUP",
                ("duplicating application of %s() ...",
                 (AP_FUNDEF (arg_node) != NULL) ? FUNDEF_NAME (AP_FUNDEF (arg_node))
                                                : "?"));

    old_fundef = AP_FUNDEF (arg_node);

    if (old_fundef != NULL) {
        if (FUNDEF_ISCONDFUN (old_fundef)
            || (FUNDEF_ISDOFUN (old_fundef)
                && (arg_node != ASSIGN_RHS (FUNDEF_INT_ASSIGN (old_fundef))))
            || (!FUNDEF_ISDOFUN (old_fundef)
                && (FUNDEF_EXT_ASSIGN (old_fundef) != NULL))) {
            /*
             * Definitions of special functions must be duplicated immediately
             * to retain one-to-one correspondence between application and
             * definition.
             *
             * If there is a link to a unique external assignment, this property
             * is also preserved. This situation applies only when inlining is
             * used after fun2lac.
             *
             * INFO_DUP_CONT must be reset to avoid copying of entire fundef
             * chain.
             */
            node *store_dup_cont;
            int store_dup_type;

            store_dup_cont = INFO_DUP_CONT (arg_info);
            store_dup_type = INFO_DUP_TYPE (arg_info);

            INFO_DUP_CONT (arg_info) = old_fundef;
            INFO_DUP_TYPE (arg_info) = DUP_NORMAL;

            new_fundef = TRAVdo (old_fundef, arg_info);

            INFO_DUP_TYPE (arg_info) = store_dup_type;
            INFO_DUP_CONT (arg_info) = store_dup_cont;

            DBUG_ASSERT (FUNDEF_NEXT (new_fundef) == NULL, "Too many functions copied.");

            FUNDEF_NAME (new_fundef) = ILIBfree (FUNDEF_NAME (new_fundef));
            FUNDEF_NAME (new_fundef) = ILIBtmpVarName (FUNDEF_NAME (old_fundef));

            FUNDEF_EXT_ASSIGN (new_fundef) = INFO_DUP_ASSIGN (arg_info);

            /*
             * Unfortunately, there is no proper way to insert the new fundef
             * into the fundef chain. This is postponed until certain safe places
             * in program execution are reached, e.g. N_module nodes. Meanwhile,
             * the new fundefs are stored in an internal fundef chain of
             * duplicated special functions.
             */
            FUNDEF_NEXT (new_fundef) = store_copied_special_fundefs;
            store_copied_special_fundefs = new_fundef;
        } else {
            new_fundef = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), old_fundef);
        }
    } else {
        /*
         * This case is only (?) used during lac2fun conversion.
         */
        new_fundef = NULL;
    }

    new_node = TBmakeAp (new_fundef, DUPTRAV (AP_ARGS (arg_node)));

    AP_ARGTAB (new_node) = DupArgtab (AP_ARGTAB (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPspap (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPspap");

    new_node = TBmakeSpap (DUPTRAV (SPAP_ID (arg_node)), DUPTRAV (SPAP_ARGS (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPspmop (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPspmop");

    DBUG_PRINT ("DUP", ("duplicating multi operation ..."));

    new_node
      = TBmakeSpmop (DUPTRAV (SPMOP_OPS (arg_node)), DUPTRAV (SPMOP_EXPRS (arg_node)));

    SPMOP_FLAGSTRUCTURE (new_node) = SPMOP_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUParray (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUParray");

    new_node = TBmakeArray (SHcopyShape (ARRAY_SHAPE (arg_node)),
                            DUPTRAV (ARRAY_AELEMS (arg_node)));

    ARRAY_STRING (new_node) = ILIBstringCopy (ARRAY_STRING (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPprf (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPprf");

    new_node = TBmakePrf (PRF_PRF (arg_node), DUPTRAV (PRF_ARGS (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPfuncond (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPfuncond");

    new_node
      = TBmakeFuncond (DUPTRAV (FUNCOND_IF (arg_node)), DUPTRAV (FUNCOND_THEN (arg_node)),
                       DUPTRAV (FUNCOND_ELSE (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPcseinfo (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPcseinfo");

    new_node = TBmakeCseinfo (DUPTRAV (CSEINFO_LAYER (arg_node)),
                              DUPTRAV (CSEINFO_LET (arg_node)),
                              DUPCONT (CSEINFO_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPannotate (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPannotate");

    new_node = TBmakeAnnotate (ANNOTATE_TAG (arg_node), ANNOTATE_FUNNUMBER (arg_node),
                               ANNOTATE_FUNAPNUMBER (arg_node));

    DBUG_ASSERT ((FALSE), "DUPannotate not implemented!! :-(");

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPex (node *arg_node, info *arg_info)
{

    node *new_node;

    DBUG_ENTER ("DUPex");

    new_node = TBmakeEx (DUPTRAV (EX_REGION (arg_node)));

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPcwrapper (node *arg_node, info *arg_info)
{

    node *new_node;

    DBUG_ENTER ("DUPcwrapper");

    new_node = TBmakeCwrapper (CWRAPPER_FUNS (arg_node),
                               ILIBstringCopy (CWRAPPER_NAME (arg_node)),
                               NSdupNamespace (CWRAPPER_NS (arg_node)),
                               CWRAPPER_ARGCOUNT (arg_node), CWRAPPER_RESCOUNT (arg_node),
                               DUPTRAV (CWRAPPER_NEXT (arg_node)));
    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPdataflowgraph (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DUPdataflowgraph");

    DBUG_ASSERT ((FALSE), "DUPdataflowgraph until now not implemented!! :-(");
    DBUG_RETURN ((node *)NULL);
}

/*******************************************************************************/

node *
DUPdataflownode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DUPdataflownode");

    DBUG_ASSERT ((FALSE), "DUPdataflownode until now not implemented!! :-(");

    DBUG_RETURN ((node *)NULL);
}

/*******************************************************************************/

node *
DUPimport (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPimport");

    new_node = TBmakeImport (ILIBstringCopy (IMPORT_MOD (arg_node)),
                             DUPCONT (IMPORT_NEXT (arg_node)),
                             DUPTRAV (IMPORT_SYMBOL (arg_node)));

    IMPORT_FLAGSTRUCTURE (new_node) = IMPORT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPexport (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPexport");

    new_node = TBmakeExport (DUPCONT (EXPORT_NEXT (arg_node)),
                             DUPTRAV (EXPORT_SYMBOL (arg_node)));

    EXPORT_FLAGSTRUCTURE (new_node) = EXPORT_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPuse (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPuse");

    new_node = TBmakeUse (ILIBstringCopy (USE_MOD (arg_node)),
                          DUPCONT (USE_NEXT (arg_node)), DUPTRAV (USE_SYMBOL (arg_node)));

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPprovide (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPprovide");

    new_node = TBmakeProvide (DUPCONT (PROVIDE_NEXT (arg_node)),
                              DUPTRAV (PROVIDE_SYMBOL (arg_node)));

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPlinklist (node *arg_node, info *arg_info)
{
    node *new_node;
    node *link;

    DBUG_ENTER ("DUPlinklist");

    link = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), LINKLIST_LINK (arg_node));

    if (link == NULL) {
        link = LINKLIST_LINK (arg_node);
    }

    new_node = TBmakeLinklist (link, DUPCONT (LINKLIST_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPnums (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPnums");

    new_node = TBmakeNums (NUMS_VAL (arg_node), DUPCONT (NUMS_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/*******************************************************************************/

node *
DUPsymbol (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPsymbol");

    new_node = TBmakeSymbol (ILIBstringCopy (SYMBOL_ID (arg_node)),
                             DUPCONT (SYMBOL_NEXT (arg_node)));

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPglobobj (node *arg_node, info *arg_info)
{
    node *new_node;
    node *link;

    DBUG_ENTER ("DUPglobobj");

    link = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), GLOBOBJ_OBJDEF (arg_node));

    if (link == NULL) {
        link = GLOBOBJ_OBJDEF (arg_node);
    }

    new_node = TBmakeGlobobj (link);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPpragma (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPpragma");

    new_node = TBmakePragma (DUPTRAV (PRAGMA_READONLY (arg_node)),
                             DUPTRAV (PRAGMA_REFCOUNTING (arg_node)),
                             DUPTRAV (PRAGMA_EFFECT (arg_node)),
                             DUPTRAV (PRAGMA_TOUCH (arg_node)),
                             DUPTRAV (PRAGMA_LINKSIGN (arg_node)));

    PRAGMA_LINKNAME (new_node) = ILIBstringCopy (PRAGMA_LINKNAME (arg_node));

    PRAGMA_INITFUN (new_node) = ILIBstringCopy (PRAGMA_INITFUN (arg_node));
    PRAGMA_WLCOMP_APS (new_node) = DUPTRAV (PRAGMA_WLCOMP_APS (arg_node));

    PRAGMA_COPYFUN (new_node) = ILIBstringCopy (PRAGMA_COPYFUN (arg_node));
    PRAGMA_FREEFUN (new_node) = ILIBstringCopy (PRAGMA_FREEFUN (arg_node));
    PRAGMA_LINKMOD (new_node) = STRSduplicate (PRAGMA_LINKMOD (arg_node));
    PRAGMA_LINKOBJ (new_node) = STRSduplicate (PRAGMA_LINKOBJ (arg_node));
    PRAGMA_NUMPARAMS (new_node) = PRAGMA_NUMPARAMS (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPicm (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPicm");

    new_node = TBmakeIcm (ICM_NAME (arg_node), DUPTRAV (ICM_ARGS (arg_node)));

    /*
     * The ICM name is not copied here because ICM names are predominantly static
     * string constants and therefore aren't freed anyway.
     */

    ICM_INDENT_BEFORE (new_node) = ICM_INDENT_BEFORE (arg_node);
    ICM_INDENT_AFTER (new_node) = ICM_INDENT_AFTER (arg_node);
    ICM_FUNDEF (new_node)
      = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), ICM_FUNDEF (arg_node));

    ICM_FLAGSTRUCTURE (new_node) = ICM_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPspmd (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPspmd");

    new_node = TBmakeSpmd (DUPTRAV (SPMD_REGION (arg_node)));

    SPMD_IN (new_node) = DupDfmask (SPMD_IN (arg_node), arg_info);
    SPMD_INOUT (new_node) = DupDfmask (SPMD_INOUT (arg_node), arg_info);
    SPMD_OUT (new_node) = DupDfmask (SPMD_OUT (arg_node), arg_info);
    SPMD_LOCAL (new_node) = DupDfmask (SPMD_LOCAL (arg_node), arg_info);
    SPMD_SHARED (new_node) = DupDfmask (SPMD_SHARED (arg_node), arg_info);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPsync (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPsync");

    new_node = TBmakeSync (DUPTRAV (SYNC_REGION (arg_node)));

    SYNC_IN (new_node) = DupDfmask (SYNC_IN (arg_node), arg_info);
    SYNC_INOUT (new_node) = DupDfmask (SYNC_INOUT (arg_node), arg_info);
    SYNC_OUT (new_node) = DupDfmask (SYNC_OUT (arg_node), arg_info);
    SYNC_OUTREP (new_node) = DupDfmask (SYNC_OUTREP (arg_node), arg_info);
    SYNC_LOCAL (new_node) = DupDfmask (SYNC_LOCAL (arg_node), arg_info);

    SYNC_FLAGSTRUCTURE (new_node) = SYNC_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwith (node *arg_node, info *arg_info)
{
    node *new_node, *partn, *coden, *withopn, *vardec, *oldids;
    node *newavis;

    DBUG_ENTER ("DUPwith");

    if ((INFO_DUP_TYPE (arg_info) == DUP_SSA)
        && (NODE_TYPE (WITH_VEC (arg_node)) == N_ids)) {
        /*
         * to maintain the ssa-form we have to create new ids
         * for the elements of N_Nwithid and insert them into LUT
         */
        oldids = WITH_VEC (arg_node);

        newavis = TBmakeAvis (ILIBtmpVarName (IDS_NAME (oldids)),
                              TYcopyType (IDS_NTYPE (oldids)));

        vardec = TBmakeVardec (newavis, NULL);

        if (IDS_TYPE (oldids) != NULL) {
            VARDEC_TYPE (vardec) = DUPdupOneTypes (IDS_TYPE (oldids));
        }

        INFO_DUP_FUNDEFSSA (arg_info)
          = TCaddVardecs (INFO_DUP_FUNDEFSSA (arg_info), vardec);

        INFO_DUP_LUT (arg_info)
          = LUTinsertIntoLutS (INFO_DUP_LUT (arg_info), IDS_NAME (oldids),
                               AVIS_NAME (newavis));

        INFO_DUP_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), IDS_DECL (oldids),
                               AVIS_DECL (newavis));

        INFO_DUP_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), IDS_AVIS (oldids), newavis);

        oldids = WITH_IDS (arg_node);
        while (oldids != NULL) {

            newavis = TBmakeAvis (ILIBtmpVarName (IDS_NAME (oldids)),
                                  TYcopyType (IDS_NTYPE (oldids)));

            vardec = TBmakeVardec (newavis, NULL);

            if (IDS_TYPE (oldids) != NULL) {
                VARDEC_TYPE (vardec) = DUPdupOneTypes (IDS_TYPE (oldids));
            }

            INFO_DUP_FUNDEFSSA (arg_info)
              = TCaddVardecs (INFO_DUP_FUNDEFSSA (arg_info), vardec);

            INFO_DUP_LUT (arg_info)
              = LUTinsertIntoLutS (INFO_DUP_LUT (arg_info), IDS_NAME (oldids),
                                   AVIS_NAME (newavis));

            INFO_DUP_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), IDS_DECL (oldids),
                                   AVIS_DECL (newavis));

            INFO_DUP_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), IDS_AVIS (oldids), newavis);
        }
    }

    /*
     * very important: duplicate codes before parts! Otherwise the code
     * references of the parts can not be set correctly!
     */
    coden = DUPTRAV (WITH_CODE (arg_node));
    partn = DUPTRAV (WITH_PART (arg_node));
    withopn = DUPTRAV (WITH_WITHOP (arg_node));

    new_node = TBmakeWith (partn, coden, withopn);

    /* copy attributes */
    WITH_PRAGMA (new_node) = DUPTRAV (WITH_PRAGMA (arg_node));
    WITH_PARTS (new_node) = WITH_PARTS (arg_node);
    WITH_REFERENCED (new_node) = WITH_REFERENCED (arg_node);
    WITH_REFERENCED_FOLD (new_node) = WITH_REFERENCED_FOLD (arg_node);
    WITH_REFERENCES_FOLDED (new_node) = WITH_REFERENCES_FOLDED (arg_node);

    WITH_IN_MASK (new_node) = DupDfmask (WITH_IN_MASK (arg_node), arg_info);
    WITH_OUT_MASK (new_node) = DupDfmask (WITH_OUT_MASK (arg_node), arg_info);
    WITH_LOCAL_MASK (new_node) = DupDfmask (WITH_LOCAL_MASK (arg_node), arg_info);

    WITH_FLAGSTRUCTURE (new_node) = WITH_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPgenarray (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPgenarray");

    new_node = TBmakeGenarray (DUPTRAV (GENARRAY_SHAPE (arg_node)),
                               DUPTRAV (GENARRAY_DEFAULT (arg_node)));

    GENARRAY_MEM (new_node) = DUPTRAV (GENARRAY_MEM (arg_node));
    GENARRAY_NEXT (new_node) = DUPCONT (GENARRAY_NEXT (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/
node *
DUPmodarray (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPmodarray");

    new_node = TBmakeModarray (DUPTRAV (MODARRAY_ARRAY (arg_node)));

    MODARRAY_MEM (new_node) = DUPTRAV (MODARRAY_MEM (arg_node));

    MODARRAY_NEXT (new_node) = DUPCONT (MODARRAY_NEXT (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/
node *
DUPfold (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPfold");

    new_node = TBmakeFold (DUPTRAV (FOLD_NEUTRAL (arg_node)));

    FOLD_FUN (new_node) = ILIBstringCopy (FOLD_FUN (arg_node));
    FOLD_NS (new_node) = NSdupNamespace (FOLD_NS (arg_node));

    FOLD_FUNDEF (new_node)
      = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), FOLD_FUNDEF (arg_node));

    FOLD_PRF (new_node) = FOLD_PRF (arg_node);

    FOLD_NEXT (new_node) = DUPCONT (FOLD_NEXT (arg_node));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPpart (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPpart");
    DBUG_ASSERT (PART_CODE (arg_node), "N_part node has no valid PART_CODE");

    new_node
      = TBmakePart (LUTsearchInLutPp (INFO_DUP_LUT (arg_info), PART_CODE (arg_node)),
                    DUPTRAV (PART_WITHID (arg_node)),
                    DUPTRAV (PART_GENERATOR (arg_node)));

    CODE_INC_USED (PART_CODE (new_node));
    PART_NEXT (new_node) = DUPCONT (PART_NEXT (arg_node));

    PART_FLAGSTRUCTURE (new_node) = PART_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPcode (node *arg_node, info *arg_info)
{
    node *new_node, *new_block, *new_cexprs;

    DBUG_ENTER ("DUPcode");

    /*
     * very important: duplicate cblock before cexprs! Otherwise the code
     * references of the cexprs can not set correctly!
     */
    new_block = DUPTRAV (CODE_CBLOCK (arg_node));
    new_cexprs = DUPTRAV (CODE_CEXPRS (arg_node));

    new_node = TBmakeCode (new_block, new_cexprs);

    INFO_DUP_LUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_node, new_node);

    CODE_NEXT (new_node) = DUPCONT (CODE_NEXT (arg_node));

    /*
     * CODE_USED is incremented in DUPpart() via CODE_INC_USED
     *                          in DUPwgrid() via CODE_INC_USED
     */
    CODE_USED (new_node) = 0;
    CODE_FLAGSTRUCTURE (new_node) = CODE_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwithid (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPwithid");

    INFO_DUP_WITHID (arg_info) = arg_node;

    new_node
      = TBmakeWithid (DUPTRAV (WITHID_VEC (arg_node)), DUPTRAV (WITHID_IDS (arg_node)));

    INFO_DUP_WITHID (arg_info) = NULL;

    WITHID_FLAGSTRUCTURE (new_node) = WITHID_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPgenerator (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPgenerator");

    new_node = TBmakeGenerator (GENERATOR_OP1 (arg_node), GENERATOR_OP2 (arg_node),
                                DUPTRAV (GENERATOR_BOUND1 (arg_node)),
                                DUPTRAV (GENERATOR_BOUND2 (arg_node)),
                                DUPTRAV (GENERATOR_STEP (arg_node)),
                                DUPTRAV (GENERATOR_WIDTH (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPdefault (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPdefault");

    new_node = TBmakeDefault ();

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwith2 (node *arg_node, info *arg_info)
{
    node *new_node, *id, *segs, *code, *withop;

    DBUG_ENTER ("DUPwith2");

    /*
     * very important: duplicate codes before parts! Otherwise the code
     * references of the parts can not set correctly!
     */
    id = DUPTRAV (WITH2_WITHID (arg_node));
    code = DUPTRAV (WITH2_CODE (arg_node));
    segs = DUPTRAV (WITH2_SEGS (arg_node));
    withop = DUPTRAV (WITH2_WITHOP (arg_node));

    new_node = TBmakeWith2 (WITH2_DIMS (arg_node), id, segs, code, withop);

    WITH2_REUSE (new_node) = DupDfmask (WITH2_REUSE (arg_node), arg_info);

    WITH2_IN_MASK (new_node) = DupDfmask (WITH2_IN_MASK (arg_node), arg_info);
    WITH2_OUT_MASK (new_node) = DupDfmask (WITH2_OUT_MASK (arg_node), arg_info);
    WITH2_LOCAL_MASK (new_node) = DupDfmask (WITH2_LOCAL_MASK (arg_node), arg_info);

    WITH2_FLAGSTRUCTURE (new_node) = WITH2_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlseg (node *arg_node, info *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DUPwlseg");

    new_node = TBmakeWlseg (WLSEG_DIMS (arg_node), DUPTRAV (WLSEG_CONTENTS (arg_node)),
                            DUPCONT (WLSEG_NEXT (arg_node)));

    DUP_VECT (WLSEG_IDX_MIN (new_node), WLSEG_IDX_MIN (arg_node), WLSEG_DIMS (new_node),
              int);
    DUP_VECT (WLSEG_IDX_MAX (new_node), WLSEG_IDX_MAX (arg_node), WLSEG_DIMS (new_node),
              int);

    DUP_VECT (WLSEG_UBV (new_node), WLSEG_UBV (arg_node), WLSEG_DIMS (new_node), int);

    WLSEG_BLOCKS (new_node) = WLSEG_BLOCKS (arg_node);

    for (i = 0; i < WLSEG_BLOCKS (new_node); i++) {
        DUP_VECT (WLSEG_BV (new_node, i), WLSEG_BV (arg_node, i), WLSEG_DIMS (new_node),
                  int);
    }

    DUP_VECT (WLSEG_SV (new_node), WLSEG_SV (arg_node), WLSEG_DIMS (new_node), int);
    DUP_VECT (WLSEG_HOMSV (new_node), WLSEG_HOMSV (arg_node), WLSEG_DIMS (new_node), int);

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (new_node) = SCHcopyScheduling (WLSEG_SCHEDULING (arg_node));
    }

    if (WLSEG_TASKSEL (arg_node) != NULL) {
        WLSEG_TASKSEL (new_node) = SCHcopyTasksel (WLSEG_TASKSEL (arg_node));
    }

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlsegvar (node *arg_node, info *arg_info)
{
    node *new_node;
    int d;

    DBUG_ENTER ("DUPwlsegvar");

    new_node
      = TBmakeWlsegvar (WLSEGVAR_DIMS (arg_node), DUPTRAV (WLSEGVAR_CONTENTS (arg_node)),
                        DUPCONT (WLSEGVAR_NEXT (arg_node)));

    MALLOC_VECT (WLSEGVAR_IDX_MIN (new_node), WLSEGVAR_DIMS (new_node), node *);
    MALLOC_VECT (WLSEGVAR_IDX_MAX (new_node), WLSEGVAR_DIMS (new_node), node *);
    for (d = 0; d < WLSEGVAR_DIMS (new_node); d++) {
        WLSEGVAR_IDX_MIN (new_node)[d] = DUPTRAV (WLSEGVAR_IDX_MIN (arg_node)[d]);
        WLSEGVAR_IDX_MAX (new_node)[d] = DUPTRAV (WLSEGVAR_IDX_MAX (arg_node)[d]);
    }

    if (WLSEGVAR_SCHEDULING (arg_node) != NULL) {
        WLSEGVAR_SCHEDULING (new_node)
          = SCHcopyScheduling (WLSEGVAR_SCHEDULING (arg_node));
    }

    if (WLSEGVAR_TASKSEL (arg_node) != NULL) {
        WLSEGVAR_TASKSEL (new_node) = SCHcopyTasksel (WLSEGVAR_TASKSEL (arg_node));
    }

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlblock (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPwlblock");

    new_node
      = TBmakeWlblock (WLBLOCK_LEVEL (arg_node), WLBLOCK_DIM (arg_node),
                       WLBLOCK_BOUND1 (arg_node), WLBLOCK_BOUND2 (arg_node),
                       WLBLOCK_STEP (arg_node), DUPTRAV (WLBLOCK_NEXTDIM (arg_node)),
                       DUPTRAV (WLBLOCK_CONTENTS (arg_node)),
                       DUPCONT (WLBLOCK_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlublock (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPwlublock");

    new_node
      = TBmakeWlublock (WLUBLOCK_LEVEL (arg_node), WLUBLOCK_DIM (arg_node),
                        WLUBLOCK_BOUND1 (arg_node), WLUBLOCK_BOUND2 (arg_node),
                        WLUBLOCK_STEP (arg_node), DUPTRAV (WLUBLOCK_NEXTDIM (arg_node)),
                        DUPTRAV (WLUBLOCK_CONTENTS (arg_node)),
                        DUPCONT (WLUBLOCK_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlstride (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPwlstride");

    new_node
      = TBmakeWlstride (WLSTRIDE_LEVEL (arg_node), WLSTRIDE_DIM (arg_node),
                        WLSTRIDE_BOUND1 (arg_node), WLSTRIDE_BOUND2 (arg_node),
                        WLSTRIDE_STEP (arg_node), DUPTRAV (WLSTRIDE_CONTENTS (arg_node)),
                        DUPCONT (WLSTRIDE_NEXT (arg_node)));

    WLSTRIDE_PART (new_node) = WLSTRIDE_PART (arg_node);
    WLSTRIDE_FLAGSTRUCTURE (new_node) = WLSTRIDE_FLAGSTRUCTURE (arg_node);

    /*
     * duplicated strides are not modified yet ;)
     */
    WLSTRIDE_ISMODIFIED (new_node) = FALSE;

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlstridevar (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPwlstridevar");

    new_node
      = TBmakeWlstridevar (WLSTRIDEVAR_LEVEL (arg_node), WLSTRIDEVAR_DIM (arg_node),
                           DUPTRAV (WLSTRIDEVAR_BOUND1 (arg_node)),
                           DUPTRAV (WLSTRIDEVAR_BOUND2 (arg_node)),
                           DUPTRAV (WLSTRIDEVAR_STEP (arg_node)),
                           DUPTRAV (WLSTRIDEVAR_CONTENTS (arg_node)),
                           DUPCONT (WLSTRIDEVAR_NEXT (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlgrid (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPwlgrid");

    new_node
      = TBmakeWlgrid (WLGRID_LEVEL (arg_node), WLGRID_DIM (arg_node),
                      WLGRID_BOUND1 (arg_node), WLGRID_BOUND2 (arg_node),
                      LUTsearchInLutPp (INFO_DUP_LUT (arg_info), WLGRID_CODE (arg_node)),
                      DUPTRAV (WLGRID_NEXTDIM (arg_node)),
                      DUPCONT (WLGRID_NEXT (arg_node)));

    if (WLGRID_CODE (new_node) != NULL) {
        CODE_INC_USED (WLGRID_CODE (new_node));
    }

    WLGRID_FLAGSTRUCTURE (new_node) = WLGRID_FLAGSTRUCTURE (arg_node);

    /*
     * duplicated grids are not modified yet ;)
     */
    WLGRID_ISMODIFIED (new_node) = FALSE;

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

node *
DUPwlgridvar (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPwlgridvar");

    new_node = TBmakeWlgridvar (WLGRIDVAR_LEVEL (arg_node), WLGRIDVAR_DIM (arg_node),
                                LUTsearchInLutPp (INFO_DUP_LUT (arg_info),
                                                  WLGRIDVAR_CODE (arg_node)),
                                DUPTRAV (WLGRIDVAR_BOUND1 (arg_node)),
                                DUPTRAV (WLGRIDVAR_BOUND2 (arg_node)),
                                DUPTRAV (WLGRIDVAR_NEXTDIM (arg_node)),
                                DUPCONT (WLGRIDVAR_NEXT (arg_node)));

    if (WLGRIDVAR_CODE (new_node) != NULL) {
        CODE_INC_USED (WLGRIDVAR_CODE (new_node));
    }

    WLGRIDVAR_FLAGSTRUCTURE (new_node) = WLGRIDVAR_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *DUPmt( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_mt, especially the DFMmasks are copied.
 *
 ******************************************************************************/

node *
DUPmt (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPmt");

    new_node = TBmakeMt (DUPTRAV (MT_REGION (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupSt( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_st, especially the DFMmasks are copied.
 *
 ******************************************************************************/

node *
DUPst (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPst");

    new_node = TBmakeSt (DUPTRAV (ST_REGION (arg_node)));

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupAvis( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_avis node. Does not set AVIS_DECL!!
 *
 ******************************************************************************/

node *
DUPavis (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPavis");

    new_node = TBmakeAvis (ILIBstringCopy (LUTsearchInLutSs (INFO_DUP_LUT (arg_info),
                                                             AVIS_NAME (arg_node))),
                           TYcopyType (AVIS_TYPE (arg_node)));

    INFO_DUP_LUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_node, new_node);

    AVIS_SSACOUNT (new_node)
      = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), AVIS_SSACOUNT (arg_node));

    AVIS_SSAASSIGN (new_node)
      = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), AVIS_SSAASSIGN (arg_node));
    AVIS_WITHID (new_node)
      = LUTsearchInLutPp (INFO_DUP_LUT (arg_info), AVIS_WITHID (arg_node));

    AVIS_SSALPINV (new_node) = AVIS_SSALPINV (arg_node);
    AVIS_SSASTACK (new_node) = DUPTRAV (AVIS_SSASTACK (arg_node));
    AVIS_SSAUNDOFLAG (new_node) = AVIS_SSAUNDOFLAG (arg_node);

    AVIS_SSADEFINED (new_node) = AVIS_SSADEFINED (arg_node);
    AVIS_SSATHEN (new_node) = AVIS_SSATHEN (arg_node);
    AVIS_SSAELSE (new_node) = AVIS_SSAELSE (arg_node);
    AVIS_NEEDCOUNT (new_node) = AVIS_NEEDCOUNT (arg_node);
    AVIS_SUBST (new_node) = AVIS_SUBST (arg_node);

    AVIS_FLAGSTRUCTURE (new_node) = AVIS_FLAGSTRUCTURE (arg_node);

    CopyCommonNodeData (new_node, arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupSSAstack( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_ssastack node.
 *
 ******************************************************************************/

node *
DUPssastack (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPssastack");

    new_node
      = TBmakeSsastack (SSASTACK_AVIS (arg_node), DUPCONT (SSASTACK_NEXT (arg_node)));

    SSASTACK_FLAGSTRUCTURE (new_node) = SSASTACK_FLAGSTRUCTURE (arg_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DupSSAcnt( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_ssacnt node.
 *
 ******************************************************************************/

node *
DUPssacnt (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPssacnt");

    new_node
      = TBmakeSsacnt (SSACNT_COUNT (arg_node), ILIBstringCopy (SSACNT_BASEID (arg_node)),
                      DUPCONT (SSACNT_NEXT (arg_node)));

    INFO_DUP_LUT (arg_info)
      = LUTinsertIntoLutP (INFO_DUP_LUT (arg_info), arg_node, new_node);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *Duperror( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a Error node.
 *
 ******************************************************************************/

node *
DUPerror (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPerror");

    new_node = TBmakeError (ILIBstringCopy (ERROR_MESSAGE (arg_node)), NULL);

    if (ERROR_NEXT (arg_node) != NULL) {
        ERROR_NEXT (new_node) = DUPerror (ERROR_NEXT (arg_node), NULL);
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * function:
 *   node *DUPfungroup( node *arg_node, info *arg_info)
 *
 * description:
 *   Duplicates a N_fungroup node.
 *
 ******************************************************************************/

node *
DUPfungroup (node *arg_node, info *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DUPfungroup");

    new_node = TBmakeFungroup ();

    CopyCommonNodeData (new_node, arg_node);

    FUNGROUP_SPECCOUNTER (new_node) = FUNGROUP_SPECCOUNTER (arg_node);
    FUNGROUP_REFCOUNTER (new_node) = FUNGROUP_REFCOUNTER (arg_node);
    FUNGROUP_INLCOUNTER (new_node) = FUNGROUP_INLCOUNTER (arg_node);
    if (FUNGROUP_FUNLIST (new_node) != NULL) {
        FUNGROUP_FUNLIST (new_node) = DUPTRAV (FUNGROUP_FUNLIST (arg_node));
    }
    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * functions:
 *   node *DupTree( node *arg_node)
 *   node *DupTreeSSA( node *arg_node, node *fundef)
 *   node *DupTree_Type( node *arg_node, lut_t * lut, int type)
 *   node *DupTreeLUT( node *arg_node, lut_t * lut)
 *   node *DupTreeLUTSSA( node *arg_node, lut_t * lut, node *fundef)
 *   node *DupTreeLUT_Type( node *arg_node, lut_t * lut, int type)
 *   node *DupNode( node *arg_node)
 *   node *DupNodeSSA( node *arg_node, node *fundef)
 *   node *DupNode_Type( node *arg_node, lut_t * lut, int type)
 *   node *DupNodeLUT( node *arg_node, lut_t * lut)
 *   node *DupNodeLUTSSA( node *arg_node, lut_t * lut, node *fundef)
 *   node *DupNodeLUT_Type( node *arg_node, lut_t * lut, int type)
 *
 * description:
 *   Copying of trees and nodes ...
 *   The node to be copied is arg_node.
 *
 *   Which function do I use???
 *   - If you want to copy a whole tree use one of the DupTreeXxx() functions.
 *     If you want to copy a node only (that means the node and all it's
 *     attributes but not the xxx_NEXT) use one of the DupNodeXxx() functions.
 *   - If you want to use a special LookUpTable (LUT) use the specific
 *     DupXxxLUT() version, otherwise use DupXxx().
 *     (If you dont't know what a LUT is good for use DupXxx() :-/ )
 *   - If you need some special behaviour triggered by INFO_DUP_TYPE use the
 *     specific DupXxx_Type() version.
 *     Legal values for the parameter 'type' are DUP_INLINE, DUP_WLF, ...
 *   - If you want to have the copy in ssa-form use one of the DupXxxSSA()
 *     functions. This functions may only be used to copy code which will be
 *     used inside the passed N_fundef node fundef. This restriction is caused
 *     by the fact that all new vardecs will be put at the fundef's vardec
 *     chain
 *
 ******************************************************************************/

node *
DUPdoDupTree (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ("DUPdoDupTree");

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, NULL, DUP_NORMAL, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupTreeSsa (node *arg_node, node *fundef)
{
    node *new_node;

    DBUG_ENTER ("DUPdoTreeSsa");

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, NULL, DUP_SSA, fundef);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupTreeType (node *arg_node, int type)
{
    node *new_node;

    DBUG_ENTER ("DUPdoDupTreeType");

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, NULL, type, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupTreeLut (node *arg_node, lut_t *lut)
{
    node *new_node;

    DBUG_ENTER ("DUPdoDupTreeLut");

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, lut, DUP_NORMAL, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupTreeLutSsa (node *arg_node, lut_t *lut, node *fundef)
{
    node *new_node;

    DBUG_ENTER ("DUPdoDupTreeLutSsa");

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, lut, DUP_SSA, fundef);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupTreeLutType (node *arg_node, lut_t *lut, int type)
{
    node *new_node;

    DBUG_ENTER ("DUPdoDupTreeLutType");

    new_node = DupTreeOrNodeLutType (FALSE, arg_node, lut, type, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNode (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ("DUPdoDupNode");

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, NULL, DUP_NORMAL, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNodeSsa (node *arg_node, node *fundef)
{
    node *new_node;

    DBUG_ENTER ("DUPdoDupNodeSsa");

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, NULL, DUP_SSA, fundef);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNodeType (node *arg_node, int type)
{
    node *new_node;

    DBUG_ENTER ("DUPdoDupNodeType");

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, NULL, type, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNodeLut (node *arg_node, lut_t *lut)
{
    node *new_node;

    DBUG_ENTER ("DUPdoDupNodeLut");

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, lut, DUP_NORMAL, NULL);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNodeLutSsa (node *arg_node, lut_t *lut, node *fundef)
{
    node *new_node;

    DBUG_ENTER ("DUPdoDupNodeLutSsa");

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, lut, DUP_SSA, fundef);

    DBUG_RETURN (new_node);
}

/* see comment above */
node *
DUPdoDupNodeLutType (node *arg_node, lut_t *lut, int type)
{
    node *new_node;

    DBUG_ENTER ("DUPdoDupNodeLutType");

    new_node = DupTreeOrNodeLutType (TRUE, arg_node, lut, type, NULL);

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * Function:
 *   shpseg *DUPdupShpseg( shpseg *arg_shpseg)
 *
 * Description:
 *
 *
 ******************************************************************************/

shpseg *
DUPdupShpseg (shpseg *arg_shpseg)
{
    shpseg *new_shpseg;

    DBUG_ENTER ("DUPdupShpseg");

    new_shpseg = DupShpseg (arg_shpseg, NULL);

    DBUG_RETURN (new_shpseg);
}

/******************************************************************************
 *
 * Function:
 *   types *DUPdupOneTypes( types *type)
 *
 * Description:
 *   Duplicates the first TYPES structure of the given TYPES chain.
 *
 *   This function duplicates the (real) types-structure. Unfortunately, it
 *   is *not* identical to the (virtual) TYPES-structure  8-((
 *
 *   For duplicating the (virtual) TYPES-structure only, use DupOneTypesOnly()
 *   or DupOneTypesOnly_Inplace() !!!
 *
 ******************************************************************************/

types *
DUPdupOneTypes (types *arg_types)
{
    types *new_types, *tmp;

    DBUG_ENTER ("DUPdupOneTypes");

    DBUG_ASSERT ((arg_types != NULL), "DUPdupOneTypes: argument is NULL!");

    tmp = TYPES_NEXT (arg_types);
    TYPES_NEXT (arg_types) = NULL;
    new_types = DupTypes (arg_types, NULL);
    TYPES_NEXT (arg_types) = tmp;

    DBUG_RETURN (new_types);
}

/******************************************************************************
 *
 * Function:
 *   types *DUPdupAllTypes( types* type)
 *
 * Description:
 *   This function duplicates the (real) types-structure. Unfortunately, it
 *   is *not* identical to the (virtual) TYPES-structure  8-((
 *
 *   For duplicating the (virtual) TYPES-structure only, use DupAllTypesOnly()
 *   !!!
 *
 ******************************************************************************/

types *
DUPdupAllTypes (types *arg_types)
{
    types *new_types;

    DBUG_ENTER ("DUPdupAllTypes");

    if (arg_types != NULL) {
        new_types = DupTypes (arg_types, NULL);
    } else {
        new_types = NULL;
    }

    DBUG_RETURN (new_types);
}

/******************************************************************************
 *
 * Function:
 *   nodelist *DUPdupNodelist( nodelist *nl)
 *
 * Description:
 *
 *
 ******************************************************************************/

nodelist *
DUPdupNodelist (nodelist *nl)
{
    nodelist *new_nl;

    DBUG_ENTER ("DUPdupNodelist");

    new_nl = DupNodelist (nl, NULL);

    DBUG_RETURN (new_nl);
}

/******************************************************************************
 *
 * Function:
 *   node *DUPdupIdsId( ids *arg_ids)
 *
 * Description:
 *   Duplicates an IDS and returns a *N_id* node.
 *
 ******************************************************************************/

node *
DUPdupIdsId (node *arg_ids)
{
    node *new_id;

    DBUG_ENTER ("DUPdupIdsId");

    new_id = TBmakeId (IDS_AVIS (arg_ids));

    DBUG_RETURN (new_id);
}

/******************************************************************************
 *
 * Function:
 *   ids *DUPdupIdIds( node *old_id)
 *
 * Description:
 *   Duplicates a N_id node and returns an *IDS*.
 *
 ******************************************************************************/

node *
DUPdupIdIds (node *old_id)
{
    node *new_ids;

    DBUG_ENTER ("DUPdupIdIds");

    new_ids = TBmakeIds (ID_AVIS (old_id), NULL);

    DBUG_RETURN (new_ids);
}

/******************************************************************************
 *
 * Function:
 *   node *DUPdupIdsIdNt( ids *arg_ids)
 *
 * Description:
 *   Duplicates an IDS and returns a *N_id* node.
 *   Sets ID_NT_TAG.
 *
 ******************************************************************************/

node *
DUPdupIdsIdNt (node *arg_ids)
{
    node *new_id;

    DBUG_ENTER ("DUPdupIdsIdNt");

    new_id = DUPdupIdsId (arg_ids);

    DBUG_ASSERT ((IDS_TYPE (arg_ids) != NULL), "NT_TAG: no type found!");
    ID_NT_TAG (new_id) = NTUcreateNtTag (IDS_NAME (arg_ids), IDS_TYPE (arg_ids));

    DBUG_RETURN (new_id);
}

/******************************************************************************
 *
 * Function:
 *   node *DUPdupIdNt( info *arg_id)
 *
 * Description:
 *   Duplicates a N_id node.
 *   Sets ID_NT_TAG.
 *
 ******************************************************************************/

node *
DUPdupIdNt (node *arg_id)
{
    node *new_id;

    DBUG_ENTER ("DUPdupIdNt");

    DBUG_ASSERT ((NODE_TYPE (arg_id) == N_id), "DupId_NT: no N_id node found!");
    new_id = DUPdoDupNode (arg_id);

    DBUG_ASSERT ((ID_TYPE (arg_id) != NULL), "NT_TAG: no type found!");
    ID_NT_TAG (new_id) = NTUcreateNtTag (ID_NAME (arg_id), ID_TYPE (arg_id));

    DBUG_RETURN (new_id);
}

/******************************************************************************
 *
 * Function:
 *   node *DUPdupNodeNt( node *arg_node)
 *
 * Description:
 *   Duplicates a node. Sets ID_NT_TAG for N_id nodes.
 *
 ******************************************************************************/

node *
DUPdupNodeNt (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ("DUPdupNodeNt");

    if (NODE_TYPE (arg_node) == N_id) {
        new_node = DUPdupIdNt (arg_node);
    } else {
        new_node = DUPdoDupNode (arg_node);
    }

    DBUG_RETURN (new_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DUPdupExprsNt( node *exprs)
 *
 * Description:
 *   Duplicates a N_exprs chain and transforms all N_id nodes found into
 *   tagged N_id nodes.
 *
 ******************************************************************************/

node *
DUPdupExprsNt (node *exprs)
{
    node *expr;
    node *new_exprs;

    DBUG_ENTER ("DUPdupExprsNT");

    if (exprs != NULL) {
        DBUG_ASSERT ((NODE_TYPE (exprs) == N_exprs), "no N_exprs node found!");

        expr = EXPRS_EXPR (exprs);
        DBUG_ASSERT ((expr != NULL), "N_exprs node contains no data!");

        new_exprs = TBmakeExprs (DUPdupNodeNt (expr), DUPdupExprsNt (EXPRS_NEXT (exprs)));
    } else {
        new_exprs = NULL;
    }

    DBUG_RETURN (new_exprs);
}

/** <!--********************************************************************-->
 *
 * @fn node *DUPgetCopiedSpecialFundefs( )
 *
 *   @brief  provides duplicated special functions for safe introduction
 *           into global fundef chain. By default this function is called
 *           by TRAVdo after having traversed an N_module node.
 *
 *   @return the N_fundef chain of duplicated special functions
 *
 *****************************************************************************/

node *
DUPgetCopiedSpecialFundefs ()
{
    node *store;

    DBUG_ENTER ("DUPgetCopiedSpecialFundefs");

    store = store_copied_special_fundefs;
    store_copied_special_fundefs = NULL;

    DBUG_RETURN (store);
}

#if 0

/******************************************************************************
 *
 * Function:
 *   node *CheckAndDupSpecialFundef( node *module, node *fundef, node *assign)
 *
 * Returns:
 *   modified module node
 *
 * Description:
 *   Checks a special fundef for multiple uses. if there are more than one
 *   external assignment/function application, this given assignment is removed
 *   from the special function list of external assignments and added to
 *   the newly duplicated and renamed version of this fundef. the concerning
 *   assignment/function application is modified to call the new fundef.
 *   the new fundef is added to the global MODUL_FUNS chain of fundefs.
 * 
 * remarks:
 *   because the global fundef chain is modified during the traversal of 
 *   this function chain it is necessary not to overwrite the MODUL_FUNS
 *   attribute in the bottom-up traversal!!!
 *
 *****************************************************************************/

node *DUPcheckAndDupSpecialFundef( node *module, node *fundef, node *assign)
{
  node *new_fundef;
  char *new_name;

  DBUG_ENTER("DUPcheckAndDupSpecialFundef");

  DBUG_ASSERT( (NODE_TYPE(module) == N_module), 
               "given module node is not a module");
  DBUG_ASSERT( (NODE_TYPE(fundef) == N_fundef),
               "given fundef node is not a fundef");
  DBUG_ASSERT( (NODE_TYPE(assign) == N_assign),
               "given assign node is not an assignment");
  DBUG_ASSERT( (FUNDEF_ISLACFUN(fundef)),
               "given fundef is not a special fundef");
  DBUG_ASSERT( (FUNDEF_USED( fundef) != USED_INACTIVE),
               "FUNDEF_USED must be active for special functions!");
  DBUG_ASSERT( (FUNDEF_USED( fundef) > 0),
               "fundef is not used anymore");
  DBUG_ASSERT( (FUNDEF_EXT_ASSIGNS(fundef) != NULL),
               "fundef has no external assignments");
  DBUG_ASSERT( (NODE_TYPE(ASSIGN_INSTR(assign)) == N_let),
               "assignment contains no let");
  DBUG_ASSERT( (NODE_TYPE(ASSIGN_RHS(assign)) == N_ap),
                "assignment is to application");
  DBUG_ASSERT( (AP_FUNDEF(ASSIGN_RHS(assign)) == fundef),
               "application of different fundef than given fundef");
  DBUG_ASSERT( (TCnodeListFind(FUNDEF_EXT_ASSIGNS(fundef), assign) != NULL),
               "given assignment is not element of external assignment list");

  if (FUNDEF_USED( fundef) > 1) {
    /* multiple uses - duplicate special fundef */
    DBUG_PRINT("DUP", ("duplicating multiple fundef %s", FUNDEF_NAME(fundef)));
    
    new_fundef = DUPdoDupNode( fundef);
    
    /* rename fundef */
    new_name = ILIBtmpVarName( FUNDEF_NAME( fundef));
    FUNDEF_NAME( new_fundef) = ILIBfree( FUNDEF_NAME( new_fundef));
    FUNDEF_NAME( new_fundef) = new_name;
    
    /* rename recursive funap (only do/while fundefs */
    if (FUNDEF_ISDOFUN(new_fundef)){      DBUG_ASSERT((FUNDEF_INT_ASSIGN( new_fundef) != NULL),
        "missing link to recursive function call");

      AP_FUNDEF( ASSIGN_RHS( FUNDEF_INT_ASSIGN( new_fundef))) = new_fundef;
    }
    
    /* init external assignment list */
    FUNDEF_EXT_ASSIGNS( new_fundef) = TCnodeListAppend( NULL, assign, NULL);
    FUNDEF_USED( new_fundef) = 1;

    /* rename the external assign/funap */
    AP_FUNDEF( ASSIGN_RHS( assign)) = new_fundef;

    /* add new fundef to global chain of fundefs */
    if (FUNDEF_BODY( new_fundef) != NULL){
      FUNDEF_NEXT( new_fundef) = MODULE_FUNS( module);
      MODULE_FUNS( module) = new_fundef;
    }
    else{
      FUNDEF_NEXT( new_fundef) = MODULE_FUNDECS( module);
      MODULE_FUNDECS( module) = new_fundef;
    }

    DBUG_ASSERT( (TCnodeListFind( FUNDEF_EXT_ASSIGNS( fundef),
                                assign) != NULL),
                 "Assignment not found in FUNDEF_EXT_ASSIGNS!");

    /* remove assignment from old external assignment list */
    FUNDEF_EXT_ASSIGNS( fundef) = TCnodeListDelete( FUNDEF_EXT_ASSIGNS( fundef),
                                                  assign, FALSE);

    (FUNDEF_USED( fundef))--;

    DBUG_ASSERT( (FUNDEF_USED( fundef) >= 0),
                 "FUNDEF_USED dropped below 0");
  }
  else {
    /* only single use - no duplication needed */
  }

  DBUG_RETURN(module);
}

#endif
