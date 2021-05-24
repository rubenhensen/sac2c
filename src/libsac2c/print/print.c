#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "print.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "str_buffer.h"
#include "type_utils.h"
#include "DupTree.h"

#define DBUG_PREFIX "PRINT"
#include "debug.h"

#include "traverse.h"
#include "ctinfo.h"
#include "free.h"
#include "convert.h"
#include "DataFlowMask.h"
#include "filemgr.h"
#include "globals.h"
#include "gen_startup_code.h"
#include "SSAWithloopFolding.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "wltransform.h"
#include "multithread_lib.h"
#include "constants.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"
#include "shape.h"
#include "vector.h"
#include "dynelem.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "graphtypes.h"
#include "tfprintutils.h"
#include "int_matrix.h"
#include "stringset.h"
#include "lex.h"

/*
 * use of arg_info in this file:
 * - node[0]: is used for storing the current fundef node.
 * - node[1]: profile macros  (?)
 * - node[2]: determines which syntax of the new WLs is printed. If it's
 *   NULL then the internal syntax is uses which allows to state more than
 *   one Npart. Else the last (and hopefully only) Npart returns the
 *   last expr in node[2].
 * - node[3]: is used while printing the old WLs to return the main
 *   expr from the block.
 */

/*
 * the INFO structure is shared between all phases that do use functions
 * from print, see print_info.h for details.
 */

/* INFO structure */

typedef struct PRINT_OPTS {
    unsigned int funap_props : 1;
    unsigned int fundef_props : 1;
} print_opts;

void
SetDefaultPrintOps (print_opts *po)
{
    po->funap_props = TRUE;
    po->fundef_props = TRUE;
}

struct INFO {
    /* print */
    node *cont;
    node *fundef;
    node *npart;
    node *nwith2;
    int sib;
    int ofp;
    bool specialization;
    bool prototype;
    bool in_setwl;
    int separate;
    int dim;
    shape *shp;
    shape *shapecnt;
    bool isarray;
    /* writesib */
    bool firstError;
    int filecounter;
    int funcounter;
    node *nonlocalfun;
    node *spmdstore;
    /*type family*/
    node *tfsupernode;
    char *tfstringexpr;
    dot_output_mode dotmode;
    print_opts prtopts;

    /*record name space*/
    char *namesapce;

    /* generic use */
    int count;
};

/* access macros print */
#define INFO_CONT(n) ((n)->cont)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_NPART(n) ((n)->npart)
#define INFO_NWITH2(n) ((n)->nwith2)
#define INFO_OMIT_FORMAL_PARAMS(n) ((n)->ofp)
#define INFO_SPECIALIZATION(n) ((n)->specialization)
#define INFO_PROTOTYPE(n) ((n)->prototype)
#define INFO_INSETWL(n) ((n)->in_setwl)
#define INFO_SEPARATE(n) ((n)->separate)
#define INFO_DIM(n) ((n)->dim)
#define INFO_SHAPE(n) ((n)->shp)
#define INFO_ISARRAY(n) ((n)->isarray)
#define INFO_SHAPE_COUNTER(n) ((n)->shapecnt)
#define INFO_FIRSTERROR(n) ((n)->firstError)
#define INFO_FILECOUNTER(n) ((n)->filecounter)
#define INFO_FUNCOUNTER(n) ((n)->funcounter)
#define INFO_NONLOCCALFUN(n) ((n)->nonlocalfun)
#define INFO_SPMDSTORE(n) ((n)->spmdstore)
#define INFO_TFSUPERNODE(n) ((n)->tfsupernode)
#define INFO_TFSTRINGEXPR(n) ((n)->tfstringexpr)
#define INFO_DOTMODE(n) ((n)->dotmode)
#define INFO_NAMESPACE(n) ((n)->namesapce)
#define INFO_PRTOPTS(n) ((n)->prtopts)
#define INFO_COUNT(n) ((n)->count)

/*
 * This global variable is used to detect inside of PrintIcm() whether
 * the ICM is in assignment position or not.
 */
static node *last_assignment_icm = NULL;

/*
 * PrintNode(): INFO_CONT(arg_info) contains the root of syntaxtree.
 *  -> traverses next-node if and only if its parent is not the root.
 * Print(): INFO_CONT(arg_info) is NULL.
 *  -> traverses all next-nodes.
 *
 * This behaviour is implemented with the macro PRINT_CONT.
 */

#define PRINT_CONT(code_then, code_else)                                                 \
    if ((arg_info != NULL) && (INFO_CONT (arg_info) == arg_node)) {                      \
        code_else;                                                                       \
    } else {                                                                             \
        code_then;                                                                       \
    }

#define ACLT(arg)                                                                        \
    (arg == ACL_unknown)                                                                 \
      ? ("ACL_unknown")                                                                  \
      : ((arg == ACL_irregular)                                                          \
           ? ("ACL_irregular")                                                           \
           : ((arg == ACL_offset) ? ("ACL_offset:")                                      \
                                  : ((arg == ACL_const) ? ("ACL_const :") : (""))))

/*
 * macros for printing debug information
 */

#define PRINT_POINTER(file, p)                                                           \
    if ((p) != NULL) {                                                                   \
        fprintf (file, F_PTR, p);                                                        \
    } else {                                                                             \
        fprintf (file, "NULL");                                                          \
    }

#define PRINT_POINTER_BRACKETS(file, p)                                                  \
    fprintf (file, "<");                                                                 \
    PRINT_POINTER (file, p);                                                             \
    fprintf (file, ">");

#define PRINT_STRING(file, str)                                                          \
    fprintf (file, "%s", STRonNULL ("?", str));                                          \
    PRINT_POINTER_BRACKETS (file, str);

/*
 * macro for printing external or SAC_C_EXTERN, depending on context
 */
#define PRINT_EXTERN                                                                     \
    (((global.compiler_subphase != PH_cg_prt)                                            \
      && (global.compiler_subphase != PH_ccg_prt))                                       \
       ? "external"                                                                      \
       : "SAC_C_EXTERN")

/*
 * First, we generate the external declarations for all functions that
 * expand ICMs to C.
 */

#define ICM_ALL
#define ICM_DEF(prf, trf) extern void Print##prf (node *exprs, info *arg_info);
#define ICM_ANY(name)
#define ICM_ICM(name)
#define ICM_NT(name)
#define ICM_ID(name)
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_UINT(name)
#define ICM_BOOL(name)
#define ICM_VARANY(dim, name)
#define ICM_VARNT(dim, name)
#define ICM_VARID(dim, name)
#define ICM_VARINT(dim, name)
#define ICM_END(prf, args)
#include "icm.data"
#undef ICM_DEF
#undef ICM_ANY
#undef ICM_ICM
#undef ICM_NT
#undef ICM_ID
#undef ICM_STR
#undef ICM_INT
#undef ICM_UINT
#undef ICM_BOOL
#undef ICM_VARANY
#undef ICM_VARNT
#undef ICM_VARID
#undef ICM_VARINT
#undef ICM_END
#undef ICM_ALL

/******************************************************************************
 *
 * function:
 *   static info *MakeInfo()
 *
 * description:
 *   creates a new info structure.
 *
 ******************************************************************************/
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    /* initialise own fields. remember to update dependent phases
     * as well!
     */
    INFO_CONT (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_NPART (result) = NULL;
    INFO_NWITH2 (result) = NULL;
    INFO_OMIT_FORMAL_PARAMS (result) = 0;
    INFO_SPECIALIZATION (result) = FALSE;
    INFO_PROTOTYPE (result) = FALSE;
    INFO_INSETWL (result) = FALSE;
    INFO_SEPARATE (result) = 0;
    INFO_DIM (result) = 0;
    INFO_SHAPE (result) = NULL;
    INFO_SHAPE_COUNTER (result) = NULL;
    INFO_ISARRAY (result) = FALSE;
    INFO_FIRSTERROR (result) = TRUE;
    INFO_FILECOUNTER (result) = 1;
    INFO_FUNCOUNTER (result) = 0;
    INFO_NONLOCCALFUN (result) = NULL;
    INFO_SPMDSTORE (result) = NULL;
    INFO_TFSUPERNODE (result) = NULL;
    INFO_TFSTRINGEXPR (result) = NULL;
    INFO_DOTMODE (result) = vertices;

    INFO_NAMESPACE (result) = NULL;

    INFO_COUNT (result) = 0;

    SetDefaultPrintOps (&INFO_PRTOPTS (result));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   static info *FreeInfo( info* info)
 *
 * description:
 *   frees a given info structure.
 *
 ******************************************************************************/
static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * Function:
 *   void IRAprintRcs( node* arg_node, info* arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
IRAprintRcs (node *arg_node, info *arg_info)
{
    size_t i, dim;
    rc_t *rcs;
    node *array, *sharray;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_code, "Wrong node-type: N_code exspected");

    fprintf (global.outfile, "/*\n");

    dim = SHP_SEG_SIZE;
    rcs = CODE_IRA_RCS (arg_node);
    INDENT;
    fprintf (global.outfile, " * IRA:\n");

    if (rcs == NULL) {
        INDENT;
        fprintf (global.outfile, " * No reuse candidates! \n");
    } else {
        do {
            array = RC_ARRAY (rcs);
            if (RC_REUSABLE (rcs)) {
                sharray = RC_SHARRAY (rcs);

                INDENT;
                fprintf (global.outfile, " * Reusable array: %s\n", AVIS_NAME (array));

                INDENT;
                fprintf (global.outfile, " * Reusable array shape: ");
                if (NODE_TYPE (RC_ARRAYSHP (rcs)) == N_id) {
                    PRTid (RC_ARRAYSHP (rcs), arg_info);
                } else if (NODE_TYPE (RC_ARRAYSHP (rcs)) == N_array) {
                    PRTarray (RC_ARRAYSHP (rcs), arg_info);
                } else {
                    DBUG_UNREACHABLE ("Wrong node type found for resuable array shape!");
                }
                fprintf (global.outfile, "\n");

                INDENT;
                fprintf (global.outfile, " * Shared array: %s\n", AVIS_NAME (sharray));

                INDENT;
                fprintf (global.outfile, " * Shared array shape: ");
                PRTarray (RC_SHARRAYSHP (rcs), arg_info);
                fprintf (global.outfile, "\n");

                INDENT;
                fprintf (global.outfile, " * Self referenced: %d\n", RC_SELFREF (rcs));

                dim = RC_DIM (rcs);

                INDENT;
                fprintf (global.outfile, " * Negative offsets: [");
                for (i = 0; i < dim; i++) {
                    fprintf (global.outfile, "%d ", RC_NEGOFFSET (rcs, i));
                }
                fprintf (global.outfile, "]\n");

                INDENT;
                fprintf (global.outfile, " * Positive offsets: [");
                for (i = 0; i < dim; i++) {
                    fprintf (global.outfile, "%d ", RC_POSOFFSET (rcs, i));
                }
                fprintf (global.outfile, "]\n");
            } else {
                INDENT;
                fprintf (global.outfile, " * Non-reusable candidate: %s\n",
                         AVIS_NAME (array));
            }
            rcs = RC_NEXT (rcs);
        } while (rcs != NULL);
    }
    INDENT;
    fprintf (global.outfile, " */\n");
    INDENT;

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void IRAprintRcs( node* arg_node, info* arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
CUAIprintCudaAccessInfo (node *arg_node, info *arg_info)
{
    int i;
    cuda_index_t *idx;
    const char *CUDA_IDX_NAMES[6]
      = {"CONSTANT", "THREADIDX_X", "THREADIDX_Y", "THREADIDX", "LOOPIDX", "EXTID"};

    const char *ACCESS_TYPE_NAMES[2] = {"REUSE", "COALESCE"};

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign, "Wrong node-type: N_assign exspected");

    INDENT;
    fprintf (global.outfile, "/*\n");

    INDENT;
    fprintf (global.outfile, "   CUDA Data Access Information: \n");

    INDENT;
    fprintf (global.outfile, "     - Coefficient Matrix: \n");
    MatrixDisplay (CUAI_MATRIX (ASSIGN_ACCESS_INFO (arg_node)), global.outfile);

    INDENT;
    fprintf (global.outfile, "     - Access Type: %s\n",
             ACCESS_TYPE_NAMES[CUAI_TYPE (ASSIGN_ACCESS_INFO (arg_node))]);
    INDENT;
    fprintf (global.outfile, "     - Access Array: %s(avis: %p)\n",
             AVIS_NAME (CUAI_ARRAY (ASSIGN_ACCESS_INFO (arg_node))),
             (void *)CUAI_ARRAY (ASSIGN_ACCESS_INFO (arg_node)));

    INDENT;
    fprintf (global.outfile, "     - Access Array Shape: ");
    PRTarray (CUAI_ARRAYSHP (ASSIGN_ACCESS_INFO (arg_node)), arg_info);
    fprintf (global.outfile, "\n");

    INDENT;
    fprintf (global.outfile, "     - Shared Memory: %s\n",
             AVIS_NAME (CUAI_SHARRAY (ASSIGN_ACCESS_INFO (arg_node))));

    INDENT;
    fprintf (global.outfile, "     - Shared Memory Shape (Logical): ");
    PRTarray (CUAI_SHARRAYSHP_LOG (ASSIGN_ACCESS_INFO (arg_node)), arg_info);
    fprintf (global.outfile, "\n");

    INDENT;
    fprintf (global.outfile, "     - Shared Memory Shape (Physical): ");
    PRTarray (CUAI_SHARRAYSHP_PHY (ASSIGN_ACCESS_INFO (arg_node)), arg_info);
    fprintf (global.outfile, "\n");

    INDENT;
    fprintf (global.outfile, "     - Dimension: %d\n",
             CUAI_DIM (ASSIGN_ACCESS_INFO (arg_node)));

    INDENT;
    fprintf (global.outfile, "     - Nest Level: %zu\n",
             CUAI_NESTLEVEL (ASSIGN_ACCESS_INFO (arg_node)));

    INDENT;
    fprintf (global.outfile, "     - Indices:\n");

    for (i = 0; i < CUAI_DIM (ASSIGN_ACCESS_INFO (arg_node)); i++) {
        idx = CUAI_INDICES (ASSIGN_ACCESS_INFO (arg_node), i);
        INDENT;
        fprintf (global.outfile, "       - Dimension %d[const:%d]: ", i,
                 CUAI_ISCONSTANT (ASSIGN_ACCESS_INFO (arg_node), i));

        while (idx != NULL) {
            fprintf (global.outfile, "( ( %d)", CUIDX_COEFFICIENT (idx));
            if (CUIDX_ID (idx) != NULL) {
                fprintf (global.outfile, " * %s(avis: %p))", AVIS_NAME (CUIDX_ID (idx)),
                         (void *)CUIDX_ID (idx));
            } else {
                fprintf (global.outfile, ")");
            }

            fprintf (global.outfile, "[Type:%s, LoopLevel:%zu]",
                     CUDA_IDX_NAMES[CUIDX_TYPE (idx)], CUIDX_LOOPLEVEL (idx));

            if (CUIDX_NEXT (idx) != NULL) {
                fprintf (global.outfile, " + ");
                idx = CUIDX_NEXT (idx);
            } else {
                break;
            }
        }
        fprintf (global.outfile, "\n");
    }

    INDENT;
    fprintf (global.outfile, " */\n");

    DBUG_RETURN ();
}

#ifndef WLAA_DEACTIVATED

/******************************************************************************
 *
 * Function:
 *   void WLAAprintAccesses( node* arg_node, info* arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
WLAAprintAccesses (node *arg_node, info *arg_info)
{
    feature_t feature;
    int i, dim, iv;
    access_t *access;
    shape *offset;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_code, "Wrong node-type: N_code exspected");

    feature = CODE_WLAAFEATURE (arg_node);
    fprintf (global.outfile, "/*\n");
    INDENT;
    fprintf (global.outfile, " * WITH-LOOP features:\n");

    INDENT;
    fprintf (global.outfile, " *   array: %s\n",
             VARDEC_NAME (CODE_WLAA_WLARRAY (arg_node)));

    INDENT;
    fprintf (global.outfile, " *   index-var: %s\n",
             VARDEC_NAME (CODE_WLAA_INDEXVAR (arg_node)));

    if (feature == FEATURE_NONE) {
        INDENT;
        fprintf (global.outfile, " *   no special features\n");
    }
    if ((feature & FEATURE_WL) == FEATURE_WL) {
        INDENT;
        fprintf (global.outfile, " *   with-loop containing array access(es)\n");
    }
    if ((feature & FEATURE_LOOP) == FEATURE_LOOP) {
        INDENT;
        fprintf (global.outfile,
                 " *   while-/do-/for-loop containing array access(es)\n");
    }
    if ((feature & FEATURE_TAKE) == FEATURE_TAKE) {
        INDENT;
        fprintf (global.outfile, " *   primitive function take\n");
    }
    if ((feature & FEATURE_DROP) == FEATURE_DROP) {
        INDENT;
        fprintf (global.outfile, " *   primitive function drop\n");
    }
    if ((feature & FEATURE_AP) == FEATURE_AP) {
        INDENT;
        fprintf (global.outfile, " *   function aplication\n");
    }
    if ((feature & FEATURE_ASEL) == FEATURE_ASEL) {
        INDENT;
        fprintf (global.outfile, " *   primitive function sel with array return value\n");
    }
    if ((feature & FEATURE_MODA) == FEATURE_MODA) {
        INDENT;
        fprintf (global.outfile, " *   primitive function modarray\n");
    }
    if ((feature & FEATURE_CAT) == FEATURE_CAT) {
        INDENT;
        fprintf (global.outfile, " *   primitive function cat\n");
    }
    if ((feature & FEATURE_ROT) == FEATURE_ROT) {
        INDENT;
        fprintf (global.outfile, " *   primitive function rotate\n");
    }
    if ((feature & FEATURE_COND) == FEATURE_COND) {
        INDENT;
        fprintf (global.outfile, " *   conditional containing array access(es)\n");
    }
    if ((feature & FEATURE_AARI) == FEATURE_AARI) {
        INDENT;
        fprintf (global.outfile, " *   primitive arithmetic operation on arrays "
                                 "(without index vector access)\n");
    }
    if ((feature & FEATURE_UNKNOWN) == FEATURE_UNKNOWN) {
        INDENT;
        fprintf (global.outfile,
                 " *   primitive function sel with unknown indexvector\n");
    }

    INDENT;
    fprintf (global.outfile, " *\n");

    dim = SHP_SEG_SIZE;
    access = CODE_WLAA_ACCESS (arg_node);
    INDENT;
    fprintf (global.outfile, " * WLAA:\n");
    do {
        if (access == NULL) {
            INDENT;
            fprintf (global.outfile, " *   No accesses! \n");
        } else {
            dim = VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access));
            iv = 0;
            offset = ACCESS_OFFSET (access);
            INDENT;
            fprintf (global.outfile, " *   %s ", ACLT (ACCESS_CLASS (access)));

            switch (ACCESS_CLASS (access)) {
            case ACL_irregular:
                /* here's no break missing ! */
            case ACL_unknown:
                fprintf (global.outfile, "sel( %s ", VARDEC_NAME (ACCESS_IV (access)));
                fprintf (global.outfile, ", %s)", VARDEC_NAME (ACCESS_ARRAY (access)));
                fprintf (global.outfile, "\n");
                access = ACCESS_NEXT (access);
                break;

            case ACL_offset:
                if (offset == NULL)
                    fprintf (global.outfile, "no offset\n");
                else {
                    do {
                        if (ACCESS_DIR (access) == ADIR_read) {
                            fprintf (global.outfile, "read ( %s + [ %d",
                                     VARDEC_NAME (ACCESS_IV (access)),
                                     SHgetExtent (offset, 0));
                        } else {
                            fprintf (global.outfile, "write( %s + [ %d",
                                     VARDEC_NAME (ACCESS_IV (access)),
                                     SHgetExtent (offset, 0));
                        }
                        for (i = 1; i < dim; i++)
                            fprintf (global.outfile, ",%d", SHgetExtent (offset, i));
                        fprintf (global.outfile, " ], %s)\n",
                                 STRonNULL ("?", VARDEC_NAME (ACCESS_ARRAY (access))));
                        offset = NULL;
                    } while (offset != NULL);
                }
                access = ACCESS_NEXT (access);
                break;

            case ACL_const:
                if (offset == NULL)
                    fprintf (global.outfile, "no offset\n");
                else {
                    do {
                        if (ACCESS_DIR (access) == ADIR_read) {
                            fprintf (global.outfile, "read ( [ %d",
                                     SHgetExtent (offset, 0));
                        } else {
                            fprintf (global.outfile, "write( [ %d",
                                     SHgetExtent (offset, 0));
                        }
                        for (i = 1; i < dim; i++) {
                            fprintf (global.outfile, ",%d", SHgetExtent (offset, i));
                        }
                        fprintf (global.outfile, " ], %s)\n",
                                 STRonNULL ("?", VARDEC_NAME (ACCESS_ARRAY (access))));
                        offset = NULL;
                    } while (offset != NULL);
                }
                access = ACCESS_NEXT (access);
                break;
                break;

            default:
                break;
            }
        }
    } while (access != NULL);

    INDENT;
    fprintf (global.outfile, " */\n");
    INDENT;

    DBUG_RETURN ();
}

#endif

#ifndef TSI_DEACTIVATED

/******************************************************************************
 *
 * Function:
 *   void TSIprintInfo( node* arg_node, info* arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
TSIprintInfo (node *arg_node, info *arg_info)
{
    int count, iter, dim, i, tilesize;
    node *pragma, *aelems;
    char *ap_name;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_code, "Wrong node-type: N_code expected");

    count = 0;
    iter = 0;
    dim = CODE_WLAA_ARRAYDIM (arg_node);
    aelems = NULL;
    if (CODE_TSI_TILESHP (arg_node) == NULL) {
        pragma = NULL;
    } else {
        pragma = MakePragma ();
        for (i = dim - 1; i >= 0; i--) {
            tilesize = SHgetExtent (CODE_TSI_TILESHP (arg_node), i);
            aelems = TBmakeExprs (MakeNum (tilesize), aelems);
        }
        ap_name = MEMmalloc (6 * sizeof (char));
        ap_name = strcpy (ap_name, "BvL0");
        PRAGMA_WLCOMP_APS (pragma)
          = TBmakeExprs (MakeAp (ap_name, NULL,
                                 TBmakeExprs (TCmakeFlatArray (aelems), NULL)),
                         NULL);
    }

    fprintf (global.outfile, "/*\n");
    INDENT;
    fprintf (global.outfile, " * TSI-Infos:\n");
    INDENT;
    fprintf (global.outfile, " *   Number of relevant accesses: %d\n", count);
    INDENT;
    fprintf (global.outfile, " *   Number of iterations: %d\n", iter);
    INDENT;
    fprintf (global.outfile, " *\n");
    INDENT;
    fprintf (global.outfile, " * TSI-proposal:\n");
    INDENT;
    if (pragma != NULL) {
        fprintf (global.outfile, " *    ");
        FreePragma (pragma, NULL);
        fprintf (global.outfile, "\n");
        INDENT;
    } else {
        fprintf (global.outfile, " *   No proposal possible!\n");
        INDENT;
    }
    fprintf (global.outfile, " */\n");
    INDENT;

    DBUG_RETURN ();
}
#endif /* ! TSI_DEACTIVATED */

/** <!-- ****************************************************************** -->
 *
 * @fn void printSOSSKdemand(node *arg_node, info *arg_info)
 *
 *    @brief This function prints the demands of each argument of a fundef if
 *           -print d  is given
 *
 *    @param arg_node the N_fundef node
 *    @param arg_info the info structure
 ******************************************************************************/

static void
printSOSSKdemand (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    node *fundef_current_arg = NULL;
    constant *demand = NULL;
    char *demand_string = NULL;

    /* If the function has no arguments, there is nothing to be printed*/
    fundef_current_arg = FUNDEF_ARGS (arg_node);
    if (fundef_current_arg != NULL) {

        /* First print the name of the function*/
        fprintf (global.outfile, "/*\n");
        fprintf (global.outfile, " * %s\n", FUNDEF_NAME (arg_node));
        fprintf (global.outfile, " * -------------------------\n");

        while (fundef_current_arg != NULL) {
            /* print the name of the argument*/
            demand = AVIS_DEMAND (ARG_AVIS (fundef_current_arg));
            fprintf (global.outfile, " * %s:\n", ARG_NAME (fundef_current_arg));
            if (demand != NULL) {
                /* print the demand*/
                demand_string = COconstant2String (demand);
                fprintf (global.outfile, " *   %s\n", demand_string);
                demand_string = MEMfree (demand_string);
            } else {
                fprintf (global.outfile, " *   -- NO DEMAND --\n");
            }

            /* go to the next argument*/
            fundef_current_arg = ARG_NEXT (fundef_current_arg);
        }
        fprintf (global.outfile, " */\n");
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void PrintArgtab( argtab_t *argtab, bool is_def)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
PRTprintArgtab (argtab_t *argtab, bool is_def)
{
    size_t i;

    DBUG_ENTER ();

    if (argtab != NULL) {
        fprintf (global.outfile, "[");
        for (i = 0; i < argtab->size; i++) {
            if (argtab->tag[i] != ATG_notag) {
                fprintf (global.outfile, " %s:", global.argtag_string[argtab->tag[i]]);

                if (argtab->ptr_in[i] != NULL) {
                    PRINT_POINTER_BRACKETS (global.outfile, (void *)argtab->ptr_in[i]);
                    if (is_def) {
                        DBUG_ASSERT (NODE_TYPE (argtab->ptr_in[i]) == N_arg,
                                     "illegal argtab entry found!");

                        if (ARG_NAME (argtab->ptr_in[i]) != NULL) {
                            fprintf (global.outfile, "%s", ARG_NAME (argtab->ptr_in[i]));
                        }
                    } else {
                        DBUG_ASSERT (NODE_TYPE (argtab->ptr_in[i]) == N_exprs,
                                     "illegal argtab entry found!");

                        fprintf (global.outfile, "%s",
                                 NODE_TEXT (EXPRS_EXPR (argtab->ptr_in[i])));
                    }
                } else {
                    fprintf (global.outfile, "-");
                }

                fprintf (global.outfile, "/");

                if (argtab->ptr_out[i] != NULL) {
                    PRINT_POINTER_BRACKETS (global.outfile, (void *)argtab->ptr_out[i]);
                    if (is_def) {
                    } else {
                        fprintf (global.outfile, "%s",
                                 STRonNull ("", IDS_NAME (((node *)argtab->ptr_out[i]))));
                    }
                } else {
                    fprintf (global.outfile, "-");
                }
            } else {
                DBUG_ASSERT (argtab->ptr_in[i] == NULL, "illegal argtab entry found!");
                DBUG_ASSERT (argtab->ptr_out[i] == NULL, "illegal argtab entry found!");

                fprintf (global.outfile, " ---");
            }

            if (i < argtab->size - 1) {
                fprintf (global.outfile, ",");
            }
        }
        fprintf (global.outfile, " ]");
    } else {
        fprintf (global.outfile, "-");
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   node *Argtab2Fundef( node *fundef)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
Argtab2Fundef (node *fundef)
{
    node *new_fundef;
    argtab_t *argtab;
    size_t i;
    node *rets = NULL;
    node *args = NULL;

    DBUG_ENTER ();

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");

    DBUG_ASSERT (argtab->ptr_in[0] == NULL, "argtab inconsistent");

    /*
     * TODO: The entire idea to generate a fundef
     *       from the argtab just 2 print it is fubar. It
     *       would be much better to print it directly.
     */
    rets = DUPdoDupNode (argtab->ptr_out[0]);

    for (i = argtab->size - 1; i >= 1; i--) {
        if (argtab->ptr_in[i] != NULL) {
            node *arg = DUPdoDupNode (argtab->ptr_in[i]);
            ARG_NEXT (arg) = args;
            args = arg;
        } else if (argtab->ptr_out[i] != NULL) {
            args
              = TBmakeArg (TBmakeAvis (NULL, TYcopyType (RET_TYPE (argtab->ptr_out[i]))),
                           args);
        }
    }

    new_fundef
      = TBmakeFundef (STRcpy (FUNDEF_NAME (fundef)), NSdupNamespace (FUNDEF_NS (fundef)),
                      rets, args, NULL, NULL);

    FUNDEF_HASDOTARGS (new_fundef) = FUNDEF_HASDOTARGS (fundef);
    FUNDEF_HASDOTRETS (new_fundef) = FUNDEF_HASDOTRETS (fundef);

    DBUG_RETURN (new_fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *ArgtabLet( node *ap)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
Argtab2Let (node *ap)
{
    node *new_let, *new_ap;
    argtab_t *argtab;
    size_t i;
    node *ids = NULL;
    node *exprs = NULL;
    node *expr = NULL;

    DBUG_ENTER ();

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT (argtab != NULL, "no argtab found!");

    DBUG_ASSERT (argtab->ptr_in[0] == NULL, "argtab inconsistent");

    if (argtab->ptr_out[0] != NULL) {
        ids = DUPdoDupNode (argtab->ptr_out[0]);
    }

    for (i = argtab->size - 1; i >= 1; i--) {
        if (argtab->ptr_out[i] != NULL) {
            exprs = TBmakeExprs (TBmakeId (IDS_AVIS (argtab->ptr_out[i])), exprs);
        } else if (argtab->ptr_in[i] != NULL) {
            expr = DUPdoDupNode (argtab->ptr_in[i]);
            EXPRS_NEXT (expr) = exprs;
            exprs = expr;
        }
    }

    new_ap = TBmakeAp (AP_FUNDEF (ap), exprs);

    new_let = TBmakeLet (ids, new_ap);

    DBUG_RETURN (new_let);
}

/******************************************************************************
 *
 * Function:
 *   void PrintArgtags( argtab_t *argtab, bool in_comment)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrintArgtags (argtab_t *argtab, bool in_comment)
{
    size_t i;

    DBUG_ENTER ();

    if (!in_comment) {
        fprintf (global.outfile, " /*");
    }

    /* return value */
    if (argtab->tag[0] != ATG_notag) {
        DBUG_ASSERT (argtab->ptr_in[0] == NULL, "argtab inconsistent");
        fprintf (global.outfile, " %s", global.argtag_string[argtab->tag[0]]);
    }

    fprintf (global.outfile, " <-");

    /* arguments */
    for (i = 1; i < argtab->size; i++) {
        DBUG_ASSERT (argtab->tag[i] != ATG_notag, "argtab is uncompressed");
        fprintf (global.outfile, " %s", global.argtag_string[argtab->tag[i]]);
    }

    if (!in_comment) {
        fprintf (global.outfile, " */ ");
    }

    DBUG_RETURN ();
}

void
PrintFunapProps (node *ap, node *spap)
{
    if (ap != NULL) {
        if (AP_ISSPAWNED (ap)) {
            fprintf (global.outfile, "spawn ");
        }
        if (AP_SPAWNPLACE (ap) != NULL) {
            fprintf (global.outfile, "(\"%s\")", AP_SPAWNPLACE (ap));
        }

    } else {
        DBUG_ASSERT (spap != NULL, "PrintFunapProps call with 2 NULL args");
        if (SPAP_ISSPAWNED (spap)) {
            fprintf (global.outfile, "spawn ");
        }
        if (SPAP_SPAWNPLACE (spap) != NULL) {
            fprintf (global.outfile, "(\"%s\")", SPAP_SPAWNPLACE (spap));
        }
    }
}

void
PrintFundefProps (node *fundef)
{
    if ((FUNDEF_ISWRAPPERFUN (fundef))) {
        fprintf (global.outfile, "wrapper:");
    }
    if ((FUNDEF_ISINDIRECTWRAPPERFUN (fundef))) {
        fprintf (global.outfile, "indirect wrapper:");
    }
    if ((FUNDEF_ISWRAPPERENTRYFUN (fundef))) {
        fprintf (global.outfile, "wrapper entry:");
    }
    if ((FUNDEF_ISTYPEERROR (fundef))) {
        fprintf (global.outfile, "typeerror:");
    }
}

void
PrintFunName (node *fundef, info *arg_info)
{
    if (INFO_PRTOPTS (arg_info).fundef_props == TRUE) {
        PrintFundefProps (fundef);
    }

    if (FUNDEF_NS (fundef) != NULL) {
        fprintf (global.outfile, "%s::", NSgetName (FUNDEF_NS (fundef)));
    }
    fprintf (global.outfile, "%s", FUNDEF_NAME (fundef));
}

void
PrintOperatorAp (node *ap, node *spap, info *arg_info)
{
    node *exprs;

    exprs = (ap != NULL ? AP_ARGS (ap) : SPAP_ARGS (spap));

    fprintf (global.outfile, "( ");
    if (TCcountExprs (exprs) == 2) {
        /* binary infix operator */
        TRAVdo (EXPRS_EXPR (exprs), arg_info);
        fprintf (global.outfile, " ");
        exprs = EXPRS_NEXT (exprs);
    }

    if (INFO_PRTOPTS (arg_info).funap_props == TRUE) {
        PrintFunapProps (ap, spap);
    }
    if (ap != NULL) {
        PrintFunName (AP_FUNDEF (ap), arg_info);
    } else {
        SPAP_ID (spap) = TRAVdo (SPAP_ID (spap), arg_info);
    }
    /* we are either dealing with a monadic operation or a binary
       one, whose first argument has been printed already and exprs
       has been progressed accordingly! */
    DBUG_ASSERT (TCcountExprs (exprs) == 1,
                 "operator application with wrong number of arguments encountered!");

    TRAVdo (EXPRS_EXPR (exprs), arg_info);
    fprintf (global.outfile, ") ");
}

void
PrintFunAp (node *ap, node *spap, info *arg_info)
{
    node *exprs;

    exprs = (ap != NULL ? AP_ARGS (ap) : SPAP_ARGS (spap));

    if (INFO_PRTOPTS (arg_info).funap_props == TRUE) {
        PrintFunapProps (ap, spap);
    }
    if (ap != NULL) {
        PrintFunName (AP_FUNDEF (ap), arg_info);
    } else {
        SPAP_ID (spap) = TRAVdo (SPAP_ID (spap), arg_info);
    }
    fprintf (global.outfile, "(");
    if (exprs != NULL) {
        fprintf (global.outfile, " ");
        TRAVdo (exprs, arg_info);
    }
    fprintf (global.outfile, ") ");
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTids( node *arg_node, info *arg_info)
 *
 *   @brief print N_ids node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (arg_node != NULL) {

        if (NODE_ERROR (arg_node) != NULL) {
            NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
        }

        fprintf (global.outfile, "%s", IDS_NAME (arg_node));

        if (global.print.avis) {
            fprintf (global.outfile, " /* DEFDEPTH=%d */ ",
                     AVIS_DEFDEPTH (IDS_AVIS (arg_node)));
            if (IDS_AVIS (arg_node) != NULL) {
                fprintf (global.outfile, "/* avis: %p */", (void *)IDS_AVIS (arg_node));
            }
        }

        if (NULL != IDS_NEXT (arg_node)) {
            fprintf (global.outfile, ", ");
            IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTspids( node *arg_node, info *arg_info)
 *
 *   @brief print N_spids node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTspids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (arg_node != NULL) {

        if (NODE_ERROR (arg_node) != NULL) {
            NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
        }

        fprintf (global.outfile, "%s", SPIDS_NAME (arg_node));

        if (NULL != SPIDS_NEXT (arg_node)) {
            fprintf (global.outfile, ", ");
            SPIDS_NEXT (arg_node) = TRAVdo (SPIDS_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTmodule( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void *
AddHeadersDependency (const char *header, strstype_t kind, void *nil)
{
    DBUG_ENTER ();

    switch (kind) {
    case STRS_headers:
        fprintf (global.outfile, "#include %s\n", header);
        break;
    default:
        CTIerrorInternal ("Unable to print module headers!\n");
        break;
    }

    DBUG_RETURN ((void *)NULL);
}

static void
PrintModuleHeaders (stringset_t *headers)
{
    DBUG_ENTER ();

    fprintf (global.outfile,
             "\n\n/* Additional headers for external function declarations */\n");

    /* print external headers */
    STRSfold (&AddHeadersDependency, headers, NULL);

    DBUG_RETURN ();
}

node *
PRTmodule (node *arg_node, info *arg_info)
{
    bool allow_non_fun = TRUE;

    DBUG_ENTER ();

    DBUG_PRINT ("%s " F_PTR, NODE_TEXT (arg_node), (void *)arg_node);

    INFO_NAMESPACE (arg_info) = STRcpy (NSgetName (MODULE_NAMESPACE (arg_node)));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (INFO_SEPARATE (arg_info)) {
        /*
         * In this case, we print a module or class implementation and we are
         * printing to generate C-code. Therefore, we want
         * each function to appear in a separate file to create a real archive
         * for later linking.
         *
         * So we produce several files in the temporary directory:
         *   header.h   contains all type definitions and an external declaration
         *              for each global object and function. This header file
         *              is included by all other files.
         *   globals.c  contains the definitions of the global objects.
         *   fun<n>.c   contains the definition of the nth function.
         */

        INFO_SPMDSTORE (arg_info) = MODULE_SPMDSTORE (arg_node);

        global.outfile = FMGRwriteOpen ("%s/header.h", global.tmp_dirname);
        GSCprintFileHeader (arg_node);

        if (NULL != MODULE_HEADERS (arg_node)) {
            PrintModuleHeaders (MODULE_HEADERS (arg_node));
        }

        if (NULL != MODULE_STRUCTS (arg_node)) {
            fprintf (global.outfile, "\n/* Structure Declarations */\n");
            /* print structdefs */
            TRAVdo (MODULE_STRUCTS (arg_node), arg_info);
        }

        if (NULL != MODULE_TYPEFAMILIES (arg_node)) {
            fprintf (global.outfile, "\n/* Type Families */\n");
            /* print typefamilies */
            TRAVdo (MODULE_TYPEFAMILIES (arg_node), arg_info);
        }

        if (NULL != MODULE_TYPES (arg_node)) {
            fprintf (global.outfile, "\n/* Type Definitions */\n");
            /* print typedefs */
            TRAVdo (MODULE_TYPES (arg_node), arg_info);
        }

        GSCprintDefines ();

        if (NULL != MODULE_FUNDECS (arg_node)) {
            fprintf (global.outfile, "\n\n");
            INFO_PROTOTYPE (arg_info) = TRUE;
            /* print function declarations */
            TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
            INFO_PROTOTYPE (arg_info) = FALSE;
        }

        if (NULL != MODULE_THREADFUNS (arg_node)) {
            fprintf (global.outfile, "\n\n");
            INFO_PROTOTYPE (arg_info) = TRUE;
            /* print function declarations */
            TRAVdo (MODULE_THREADFUNS (arg_node), arg_info);
            INFO_PROTOTYPE (arg_info) = FALSE;
        }

        if (NULL != MODULE_FUNS (arg_node)) {
            fprintf (global.outfile, "\n\n");
            INFO_PROTOTYPE (arg_info) = TRUE;
            /* print function declarations */
            TRAVdo (MODULE_FUNS (arg_node), arg_info);
            INFO_PROTOTYPE (arg_info) = FALSE;
        }

        if (NULL != MODULE_OBJS (arg_node)) {
            fprintf (global.outfile, "\n\n");
            global.print_objdef_for_header_file = TRUE;
            /* print object declarations */
            TRAVdo (MODULE_OBJS (arg_node), arg_info);
        }

        fclose (global.outfile);

        global.outfile
          = FMGRwriteOpen ("%s/globals%s", global.tmp_dirname, global.config.cext);
        fprintf (global.outfile, "#include \"header.h\"\n\n");
        fprintf (global.outfile,
                 "static int SAC__%s__dummy_value_which_is_completely_useless"
                 " = 0;\n\n",
                 NSgetName (MODULE_NAMESPACE (arg_node)));

        if (NULL != MODULE_OBJS (arg_node)) {
            fprintf (global.outfile, "\n\n");
            global.print_objdef_for_header_file = FALSE;
            /* print object definitions */
            TRAVdo (MODULE_OBJS (arg_node), arg_info);
        }

        fclose (global.outfile);
        global.outfile = NULL;

        if (NULL != MODULE_FUNS (arg_node)) {
            TRAVdo (MODULE_FUNS (arg_node), arg_info); /* print function definitions */
                                                       /*
                                                        * Note that in this case a separate file is created for each function.
                                                        * These files are opened and closed in PRTfundef().
                                                        */
        }
        TRAVopt (MODULE_THREADFUNS (arg_node), arg_info);
        if (global.outfile != NULL) {
            fclose (global.outfile);
            /*
             * Due to the linksetsize feature it may be the case that the lust file
             * created is still open for writing after having written the last function.
             */
        }

    } else {
        switch (global.filetype) {
        case FT_modimp:
            fprintf (global.outfile,
                     "\n"
                     "module %s;\n",
                     NSgetName (MODULE_NAMESPACE (arg_node)));
            break;
        case FT_classimp:
            fprintf (global.outfile,
                     "\n"
                     "class %s;\n",
                     NSgetName (MODULE_NAMESPACE (arg_node)));
            break;
        case FT_prog:
            fprintf (global.outfile,
                     "\n"
                     "/*\n"
                     " *  SAC-Program %s :\n"
                     " */\n",
                     global.puresacfilename);
            break;
        case FT_cmod:
            fprintf (global.outfile, "\n"
                                     "/*\n"
                                     " *  C-Wrapper Module\n"
                                     " */\n");
            break;
        default:
            break;
        }

        if (NULL != MODULE_HEADERS (arg_node)) {
            PrintModuleHeaders (MODULE_HEADERS (arg_node));
        }

        /*
         * if this is false header comments and non fun code is filtered out.
         * This is for the sake of the printstart, printstop, printfun options.
         * (mhs)
         */
        /*    allow_non_fun = (( global.break_fun_name == NULL ||
                                 (global.compiler_phase > global.prtphafun_stop_phase)) ||
                                 (global.break_fun_name != NULL &&
                                 global.break_after_phase == PH_undefined));
        */
        allow_non_fun
          = ((global.break_fun_name == NULL)
             || (global.break_fun_name != NULL && global.break_after_phase == PH_undefined
                 && global.prt_cycle_range == FALSE
                 && (global.compiler_phase > global.prtphafun_stop_phase
                     || global.prtphafun_stop_phase == PH_undefined)));

        if (allow_non_fun && (MODULE_INTERFACE (arg_node) != NULL)) {
            fprintf (global.outfile, "\n");

            /* print import-list */
            TRAVdo (MODULE_INTERFACE (arg_node), arg_info);
        }

        if (allow_non_fun && (NULL != MODULE_TYPEFAMILIES (arg_node))) {
            fprintf (global.outfile, "\n\n");

            /* print typefamilies */
            TRAVdo (MODULE_TYPEFAMILIES (arg_node), arg_info);
        }

        if (allow_non_fun && (MODULE_STRUCTS (arg_node) != NULL)) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  struct definitions\n"
                                     " */\n\n");

            /* print structdefs */
            TRAVdo (MODULE_STRUCTS (arg_node), arg_info);
        }

        if (allow_non_fun && (MODULE_TYPES (arg_node) != NULL)) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  type definitions\n"
                                     " */\n\n");

            /* print typedefs */
            TRAVdo (MODULE_TYPES (arg_node), arg_info);
        }

        if (allow_non_fun && (MODULE_FPFRAMESTORE (arg_node) != NULL)) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  FP Frame infrastructure\n"
                                     " */\n\n");

            TRAVdo (MODULE_FPFRAMESTORE (arg_node), arg_info);
        }

        if (((global.compiler_subphase == PH_cg_prt)
             || (global.compiler_subphase == PH_ccg_prt))
            && allow_non_fun) {
            GSCprintDefines ();
        }

        if (allow_non_fun
            && ((MODULE_FUNDECS (arg_node) != NULL)
                && (global.doprintfunsets || global.printfunsets.imp
                    || global.printfunsets.use))) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  prototypes for externals (FUNDECS)\n"
                                     " */\n\n");

            INFO_PROTOTYPE (arg_info) = TRUE;
            /* print function declarations */
            TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
            INFO_PROTOTYPE (arg_info) = FALSE;
        }

        if (allow_non_fun && (MODULE_FUNSPECS (arg_node) != NULL)) {

            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  user requested specialisations (FUNSPECS)\n"
                                     " */\n\n");

            INFO_SPECIALIZATION (arg_info) = TRUE;
            /* print function declarations */
            TRAVdo (MODULE_FUNSPECS (arg_node), arg_info);
            INFO_SPECIALIZATION (arg_info) = FALSE;
        }

        /*
         * The following conditional is a cruel hack to get sac2tex
         * going; a much better solution should be adopted once print.c
         * is rewritten!!!! (sbs)
         */
        if (allow_non_fun && (global.tool != TOOL_sac2tex)) {
            if (MODULE_THREADFUNS (arg_node) != NULL) {
                fprintf (global.outfile, "\n\n"
                                         "/*\n"
                                         " *  prototypes for threads (THREADFUNS)\n"
                                         " */\n\n");

                INFO_PROTOTYPE (arg_info) = TRUE;
                /* print function declarations */
                TRAVdo (MODULE_THREADFUNS (arg_node), arg_info);
                INFO_PROTOTYPE (arg_info) = FALSE;
            }
            if ((MODULE_FUNS (arg_node) != NULL
                 && (global.doprintfunsets || global.printfunsets.imp
                     || global.printfunsets.use || global.printfunsets.pre))) {

                fprintf (global.outfile, "\n\n"
                                         "/*\n"
                                         " *  prototypes for locals (FUNDEFS)\n"
                                         " */\n\n");

                INFO_PROTOTYPE (arg_info) = TRUE;
                /* print function declarations */
                TRAVdo (MODULE_FUNS (arg_node), arg_info);
                INFO_PROTOTYPE (arg_info) = FALSE;
            }
        }

        if (allow_non_fun
            && ((MODULE_OBJS (arg_node) != NULL) && global.doprintfunsets)) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  global objects\n"
                                     " */\n\n");

            /* print objdefs */
            TRAVdo (MODULE_OBJS (arg_node), arg_info);
        }

        if (allow_non_fun
            && ((MODULE_SPMDSTORE (arg_node) != NULL) && global.doprintfunsets)) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  SPMD infrastructure\n"
                                     " */\n\n");

            TRAVdo (MODULE_SPMDSTORE (arg_node), arg_info);
        }

        if (allow_non_fun
            && ((MODULE_THREADFUNS (arg_node) != NULL) && global.doprintfunsets)) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  function definitions (THREADFUNS)\n"
                                     " */\n\n");

            /* print function definitions */
            TRAVdo (MODULE_THREADFUNS (arg_node), arg_info);
        }

        if ((MODULE_FUNS (arg_node) != NULL)
            && (global.doprintfunsets || global.printfunsets.def
                || global.printfunsets.wrp)) {

            if (allow_non_fun) {
                fprintf (global.outfile, "\n\n"
                                         "/*\n"
                                         " *  function definitions (FUNDEFS)\n"
                                         " */\n\n");
            }

            /* print function definitions */
            TRAVdo (MODULE_FUNS (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTstructdef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTstructdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "struct %s {\n", STRUCTDEF_NAME (arg_node));
    global.indent++;
    STRUCTDEF_STRUCTELEM (arg_node) = TRAVopt (STRUCTDEF_STRUCTELEM (arg_node), arg_info);
    global.indent--;
    fprintf (global.outfile, "};\n\n");

    STRUCTDEF_NEXT (arg_node) = TRAVopt (STRUCTDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTstructelem( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTstructelem (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    fprintf (global.outfile, "%s %s;\n",
             TYtype2String (STRUCTELEM_TYPE (arg_node), FALSE, 0),
             STRUCTELEM_NAME (arg_node));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTtypedef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTtypedef (node *arg_node, info *arg_info)
{
    char *type_str;
    bool ishidden;

    DBUG_ENTER ();

    DBUG_PRINT ("%s " F_PTR, NODE_TEXT (arg_node), (void *)arg_node);

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (TYPEDEF_ICM (arg_node) == NULL) {
        ishidden = TUisHidden (TYPEDEF_NTYPE (arg_node));

        if (ishidden) {
            fprintf (global.outfile, "external ");
        }

        if (TYPEDEF_ISALIAS (arg_node)) {
            fprintf (global.outfile, "typealias ");
        } else if (TYPEDEF_ISUNIQUE (arg_node)) {
            fprintf (global.outfile, "classtype ");
        } else if (TYPEDEF_ISNESTED (arg_node)) {
            fprintf (global.outfile, "nested ");
        } else {
            fprintf (global.outfile, "typedef ");
        }

        if (!ishidden) {
            type_str = TYtype2String (TYPEDEF_NTYPE (arg_node), 0, TRUE);
            fprintf (global.outfile, "%s ", type_str);
            type_str = MEMfree (type_str);
        }

        if (TYPEDEF_NS (arg_node) != NULL) {
            fprintf (global.outfile, "%s::", NSgetName (TYPEDEF_NS (arg_node)));
        }
        fprintf (global.outfile, "%s", TYPEDEF_NAME (arg_node));

        fprintf (global.outfile, ";\n");
    } else {
        TRAVdo (TYPEDEF_ICM (arg_node), arg_info);
        fprintf (global.outfile, "\n");
    }

    /** BEGINNOTE
     * For the CUDA backends (`CUDA' and `CUDAHybrid') both the COPYFUN
     * and the FREEFUN were intentionally being discarded. Why, is not know.
     * I (Hans) have removed the if-block and now allow for the COPY- and
     * FREEFUN to be printed.
     *
     * The following warning was issued when a discard happened:
     *
     *   CTIwarn("Discarding copy fun for %s\n", TYPEDEF_NAME( arg_node));
     *
     */
    if (TYPEDEF_COPYFUN (arg_node) != NULL) {
        fprintf (global.outfile, "\n%s %s %s( %s);\n", PRINT_EXTERN,
                 TYPEDEF_NAME (arg_node), TYPEDEF_COPYFUN (arg_node),
                 TYPEDEF_NAME (arg_node));
    }
    if (TYPEDEF_FREEFUN (arg_node) != NULL) {
        fprintf (global.outfile, "%s void %s( %s);\n\n", PRINT_EXTERN,
                 TYPEDEF_FREEFUN (arg_node), TYPEDEF_NAME (arg_node));
    }
    /* ENDNOTE */

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (TYPEDEF_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTobjdef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTobjdef (node *arg_node, info *arg_info)
{
    char *type_str;

    DBUG_ENTER ();

    DBUG_PRINT ("%s " F_PTR, NODE_TEXT (arg_node), (void *)arg_node);

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if ((OBJDEF_ICM (arg_node) == NULL) || (NODE_TYPE (OBJDEF_ICM (arg_node)) != N_icm)) {
        if (!OBJDEF_ISLOCAL (arg_node) || global.print_objdef_for_header_file) {
            fprintf (global.outfile, "external ");
        }

        if (OBJDEF_ISALIAS (arg_node)) {
            fprintf (global.outfile, "alias ");
        }

        type_str = TYtype2String (OBJDEF_TYPE (arg_node), FALSE, 0);
        fprintf (global.outfile, "%s ", type_str);
        type_str = MEMfree (type_str);

        if (OBJDEF_NS (arg_node) != NULL) {
            fprintf (global.outfile, "%s::", NSgetName (OBJDEF_NS (arg_node)));
        }

        fprintf (global.outfile, "%s", OBJDEF_NAME (arg_node));

        if (OBJDEF_EXPR (arg_node) != NULL) {
            fprintf (global.outfile, " = ");
            TRAVdo (OBJDEF_EXPR (arg_node), arg_info);
        }

        fprintf (global.outfile, ";\n");

        if (OBJDEF_PRAGMA (arg_node) != NULL) {
            TRAVdo (OBJDEF_PRAGMA (arg_node), arg_info);
        }

        fprintf (global.outfile, "\n");
    } else {
        TRAVdo (OBJDEF_ICM (arg_node), arg_info);
        fprintf (global.outfile, "\n");
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (OBJDEF_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTret( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTret (node *arg_node, info *arg_info)
{
    char *type_str;

    DBUG_ENTER ();

    DBUG_PRINT ("%s " F_PTR, NODE_TEXT (arg_node), (void *)arg_node);

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (RET_TYPE (arg_node) != NULL) {
        type_str = TYtype2String (RET_TYPE (arg_node), FALSE, 0);
        fprintf (global.outfile, "%s", type_str);
        type_str = MEMfree (type_str);

        if (RET_ISUNIQUE (arg_node)) {
            fprintf (global.outfile, " *");
        }

        if (RET_NEXT (arg_node) != NULL) {
            fprintf (global.outfile, ", ");
            RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   void PrintFunctionHeader( node *arg_node, info *arg_info, bool in_comment)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void *
PrintDispatchFun (node *fundef, void *arg_info)
{
    if (INFO_COUNT ((info *)arg_info) > 0) {
        fprintf (global.outfile, ",\n *                  ");
    }
    PrintFunName (fundef, (info *)arg_info);
    INFO_COUNT ((info *)arg_info) = INFO_COUNT ((info *)arg_info) + 1;
    return (arg_info);
}

static void
PrintFunctionHeader (node *arg_node, info *arg_info, bool in_comment)
{
    bool print_sac = TRUE;
    bool print_c = FALSE;
    bool print_argtab = FALSE;

    DBUG_ENTER ();

    DBUG_EXECUTE_TAG ("PRINT_PTR",
                      fprintf (global.outfile, "/* " F_PTR " */\n", (void *)arg_node));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }
    /*
     * Print SOSSK demand if wanted
     */
    if (global.print.demand) {
        printSOSSKdemand (arg_node, arg_info);
    }

    if (FUNDEF_ARGTAB (arg_node) != NULL) {
        print_sac = FALSE;
        print_c = TRUE;

        DBUG_EXECUTE_TAG ("PRINT_ARGTAB", fprintf (global.outfile, "/* \n"); INDENT;
                          fprintf (global.outfile, " *  ");
                          PRTprintArgtab (FUNDEF_ARGTAB (arg_node), TRUE);
                          fprintf (global.outfile, "  */\n"); INDENT; print_sac = TRUE;
                          print_argtab = TRUE);
    }

    if (FUNDEF_ISGENERIC (arg_node)) {
        fprintf (global.outfile, "/* generic definition */\n");
    }

    if (FUNDEF_ISLACINLINE (arg_node)) {
        fprintf (global.outfile, "/* lacinline */\n");
    }

    if (FUNDEF_ISSTICKY (arg_node)) {
        fprintf (global.outfile, "/* sticky */\n");
    }

    if (FUNDEF_ISINLINE (arg_node)) {
        fprintf (global.outfile, "inline\n");
    }

    if (print_c) {
        node *tmp = Argtab2Fundef (arg_node);

        PrintFunctionHeader (tmp, arg_info, in_comment);
        tmp = FREEdoFreeTree (tmp);
        /*
         * There is a small space leak here because the N_fundef node is not
         * entirely freed, but zombified. We should look for a different solution
         * here rather than generating the function header from the argtab on the
         * spot.
         */

        fprintf (global.outfile, " ");
        if (!print_argtab) {
            PrintArgtags (FUNDEF_ARGTAB (arg_node), in_comment);
        }
    }

    if (print_sac) {
        if (print_c) {
            fprintf (global.outfile, "\n");
            INDENT;
            fprintf (global.outfile, "/*  ");
        }

        if (FUNDEF_RETS (arg_node) == NULL) {
            fprintf (global.outfile, "void ");
        } else {
            TRAVdo (FUNDEF_RETS (arg_node), arg_info);

            if (FUNDEF_HASDOTRETS (arg_node)) {
                fprintf (global.outfile, ", ...");
            }
        }

        fprintf (global.outfile, " ");

        if (FUNDEF_NS (arg_node) != NULL) {
            fprintf (global.outfile, "%s::", NSgetName (FUNDEF_NS (arg_node)));
        }

        if (FUNDEF_NAME (arg_node) != NULL) {
            fprintf (global.outfile, "%s", FUNDEF_NAME (arg_node));
        }

        fprintf (global.outfile, "(");

        if (FUNDEF_ARGS (arg_node) != NULL) {
            TRAVdo (FUNDEF_ARGS (arg_node), arg_info); /* print args of function */
        }

        if (FUNDEF_HASDOTARGS (arg_node)) {
            fprintf (global.outfile, ", ...");
        }

        fprintf (global.outfile, ")");

        if (FUNDEF_ASSERTS (arg_node) != NULL) {
            fprintf (global.outfile, "\nAssert");
            TRAVdo (FUNDEF_ASSERTS (arg_node), arg_info); /* print args of function */
        }

        if (print_c) {
            fprintf (global.outfile, "\n");
            INDENT;
            fprintf (global.outfile, " */ ");
        }
    }

    /*
     * The following conditional is a cruel hack to get sac2tex
     * going; a much better solution should be adopted once print.c
     * is rewritten!!!! (sbs)
     */
    if (global.tool != TOOL_sac2tex) {

        /* Now, we print the new type signature, iff present */
        fprintf (global.outfile, "\n");
        INDENT;
        fprintf (global.outfile, (in_comment) ? " *\n" : "/*\n");
        fprintf (global.outfile, " *  ");
        if (FUNDEF_NAME (arg_node) != NULL) {
            fprintf (global.outfile, "%s :: ", FUNDEF_NAME (arg_node));
            if (FUNDEF_WRAPPERTYPE (arg_node) != NULL) {
                char *(*t2s_fun) (ntype *, bool, size_t);
                t2s_fun = TYtype2String;
                DBUG_EXECUTE_TAG ("PRINT_NTY", t2s_fun = TYtype2DebugString);

                fprintf (global.outfile, "%s\n",
                         t2s_fun (FUNDEF_WRAPPERTYPE (arg_node), TRUE,
                                  global.indent + STRlen (FUNDEF_NAME (arg_node)) + 8)); 
                fprintf (global.outfile, " *  dispatching to: ");
                if (TYisProd (FUNDEF_WRAPPERTYPE (arg_node))) {
                    PrintFunName (FUNDEF_IMPL (arg_node), arg_info);
                    fprintf (global.outfile, "\n");
                } else {
                    INFO_COUNT (arg_info) = 0;
                    TYfoldFunctionInstances (FUNDEF_WRAPPERTYPE (arg_node),
                                             PrintDispatchFun, arg_info);
                    fprintf (global.outfile, "\n");
                }
            } else {
                fprintf (global.outfile, " ---\n");
            }
        }
        INDENT;
        fprintf (global.outfile, (in_comment) ? " *" : " */");
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   node *PRTfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 * Remark:
 *   If C-code is to be generated, which means that an N_icm node already
 *   hangs on node[3], additional extern declarations for function
 *   definitions are printed.
 *
 ******************************************************************************/

node *
PRTfundef (node *arg_node, info *arg_info)
{
    size_t old_indent = global.indent;

    bool is_userdefined_function = FALSE;

    DBUG_ENTER ();

    /*
      if( INFO_NONLOCCALFUN( arg_info) != NULL){
        printf( "FOUND LOCAL FUN OF %s: %s\n", FUNDEF_NAME( INFO_NONLOCCALFUN( arg_info))
      ,FUNDEF_NAME(arg_node));
      }
    */
    /*
     *if the break_fun_name is null then always print
     *if the break_fun_name equals the current FUNDEF node then print
     *if the break_fun_name is not null (selective printing is used) AND
     *break_after_phase (-b option) is not set AND the PRTfundef function was not
     *called during a prt_cycle_range event, then print.
     *
     *OR if the fundef has local fundefs. (mhs)
     */

    if (global.break_fun_name == NULL
        || STReq (global.break_fun_name, FUNDEF_NAME (arg_node))
        || (global.break_fun_name != NULL && global.break_after_phase == PH_undefined
            && global.prt_cycle_range == FALSE)
        || INFO_NONLOCCALFUN (arg_info) != NULL) {

        DBUG_PRINT ("%s " F_PTR, NODE_TEXT (arg_node), (void *)arg_node);

        //    if( INFO_NONLOCCALFUN( arg_info) != NULL) {
        //    }

        if (NODE_ERROR (arg_node) != NULL) {
            NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
        }

        /*
         * needed for the introduction of PROFILE_... MACROS in the
         *  function body.
         */
        INFO_FUNDEF (arg_info) = arg_node;
        /*
          if (FUNDEF_OBJECTS( arg_node) != NULL) {
            FUNDEF_OBJECTS( arg_node) = TRAVdo( FUNDEF_OBJECTS( arg_node), arg_info);
          }
        */
        if (INFO_SPECIALIZATION (arg_info)) {
            fprintf (global.outfile, "specialize ");
            PrintFunctionHeader (arg_node, arg_info, FALSE);
            fprintf (global.outfile, ";\n\n");
        } else if (INFO_PROTOTYPE (arg_info)
                   && (global.doprintfunsets
                       || (global.printfunsets.pre && FUNDEF_ISSTICKY (arg_node))
                       || (global.printfunsets.imp && FUNDEF_WASIMPORTED (arg_node))
                       || (global.printfunsets.use && FUNDEF_WASUSED (arg_node)))) {

            /*
             * print function declaration
             */

            if (!FUNDEF_ISZOMBIE (arg_node)) {
                if (((FUNDEF_BODY (arg_node) == NULL) && !FUNDEF_HEADER (arg_node))
                    || ((FUNDEF_RETURN (arg_node) != NULL)
                        && (NODE_TYPE (FUNDEF_RETURN (arg_node)) == N_icm))) {
                    fprintf (global.outfile, "%s ", PRINT_EXTERN);

                    if ((FUNDEF_ICMDEFBEGIN (arg_node) == NULL)
                        || (NODE_TYPE (FUNDEF_ICMDEFBEGIN (arg_node)) != N_icm)) {
                        PrintFunctionHeader (arg_node, arg_info, FALSE);
                    } else {
                        /* print N_icm ND_FUN_DEC */
                        fprintf (global.outfile, "\n");
                        TRAVdo (FUNDEF_ICMDECL (arg_node), arg_info);
                    }

#if 0
          if (!((FUNDEF_ICMDEFBEGIN (arg_node) == NULL) ||
               (NODE_TYPE (FUNDEF_ICMDEFBEGIN (arg_node)) != N_icm))) {
            fprintf (global.outfile, ";\n");
          }
#else
                    fprintf (global.outfile, ";\n");
#endif

                    if ((global.compiler_subphase != PH_cg_prt)
                        && (global.compiler_subphase != PH_ccg_prt)) {
                        if (FUNDEF_PRAGMA (arg_node) != NULL) {
                            TRAVdo (FUNDEF_PRAGMA (arg_node), arg_info);
                        }
                    }

                    fprintf (global.outfile, "\n");
                }
            }
        } else {
            /*
             * print function definition
             */

            if (FUNDEF_ISZOMBIE (arg_node)) {
                if ((global.compiler_subphase != PH_cg_prt)
                    && (global.compiler_subphase != PH_ccg_prt)) {
                    fprintf (global.outfile, "/*\n");
                    INDENT;
                    fprintf (global.outfile, " * zombie function:\n");
                    INDENT;
                    fprintf (global.outfile, " *   ");
                    PrintFunctionHeader (arg_node, arg_info, FALSE);
                    fprintf (global.outfile, "\n");
                    INDENT;
                    fprintf (global.outfile, " */\n\n");
                } else {
                    /*
                     * do not print zombie code in header files,
                     * do not generate separate files
                     */
                }
            } else {
                /*
                 * we only print functions with bodies here, as we
                 * already have printed headers for all other functions
                 * earlier!
                 */

                is_userdefined_function
                  = STReq (NSgetName (FUNDEF_NS (arg_node)), INFO_NAMESPACE (arg_info));

                if ((FUNDEF_BODY (arg_node) != NULL)
                    && (global.doprintfunsets
                        || (global.printfunsets.def && FUNDEF_ISLOCAL (arg_node)
                            && is_userdefined_function)
                        || (global.printfunsets.wrp && FUNDEF_ISWRAPPERFUN (arg_node)))) {

                    if (INFO_SEPARATE (arg_info) && (INFO_FUNCOUNTER (arg_info) == 0)) {
                        global.outfile = FMGRwriteOpen ("%s/fun%d%s", global.tmp_dirname,
                                                        INFO_FILECOUNTER (arg_info),
                                                        global.config.cext);
                        INFO_FILECOUNTER (arg_info) += 1;
                        global.num_fun_files += 1;
                        fprintf (global.outfile, "#include \"header.h\"\n\n");
                        TRAVopt (INFO_SPMDSTORE (arg_info), arg_info);
                    }

                    fprintf (global.outfile, "\n\n"
                                             "/******************************************"
                                             "**********************************\n");

                    INFO_FUNCOUNTER (arg_info) += 1;

                    if (FUNDEF_ISWRAPPERFUN (arg_node)) {
                        fprintf (global.outfile, " * Wrapper function:\n");
                    } else if (FUNDEF_ISINDIRECTWRAPPERFUN (arg_node)) {
                        fprintf (global.outfile, " * Indirect wrapper function: ");
                    } else if (FUNDEF_ISWRAPPERENTRYFUN (arg_node)) {
                        fprintf (global.outfile, " * Wrapper entry function: ");
                    } else if (FUNDEF_ISCONDFUN (arg_node)) {
                        if (INFO_NONLOCCALFUN (arg_info) == NULL) {
                            fprintf (global.outfile, " * Cond function:\n");
                        } else {
                            fprintf (global.outfile, " * Cond function of ");
                            if (FUNDEF_NS (INFO_NONLOCCALFUN (arg_info)) != NULL) {
                                fprintf (global.outfile, "%s::",
                                         NSgetName (
                                           FUNDEF_NS (INFO_NONLOCCALFUN (arg_info))));
                            }
                            if (FUNDEF_NAME (arg_node) != NULL) {
                                fprintf (global.outfile, "%s(...):\n",
                                         FUNDEF_NAME (INFO_NONLOCCALFUN (arg_info)));
                            }
                        }
                    } else if (FUNDEF_ISLOOPFUN (arg_node)) {
                        if (INFO_NONLOCCALFUN (arg_info) == NULL) {
                            fprintf (global.outfile,
                                     " * Loop function with Loop Count %d:\n",
                                     FUNDEF_LOOPCOUNT (arg_node));
                        } else {
                            fprintf (global.outfile, " * Loop function of ");
                            if (FUNDEF_NS (INFO_NONLOCCALFUN (arg_info)) != NULL) {
                                fprintf (global.outfile, "%s::",
                                         NSgetName (
                                           FUNDEF_NS (INFO_NONLOCCALFUN (arg_info))));
                            }
                            if (FUNDEF_NAME (arg_node) != NULL) {
                                fprintf (global.outfile, "%s(...): with Loop Count %d\n",
                                         FUNDEF_NAME (INFO_NONLOCCALFUN (arg_info)),
                                         FUNDEF_LOOPCOUNT (arg_node));
                            }
                        }
                    }

                    if (FUNDEF_ISCUDALACFUN (arg_node)) {
                        fprintf (global.outfile, " * CUDA lac function:\n");
                    }

                    if (FUNDEF_ISFORLOOP (arg_node)) {
                        fprintf (global.outfile, " * For loop function:\n");
                    }

                    if (FUNDEF_ISTHREADFUN (arg_node)) {
                        fprintf (global.outfile, " * MUTC thread fun\n");
                    }

                    if (FUNDEF_ISSPAWNFUN (arg_node)) {
                        fprintf (global.outfile, " * MUTC spawn fun\n");
                    }

                    if (FUNDEF_ISMTFUN (arg_node)) {
                        fprintf (global.outfile, " * MT function:\n");
                    } else if (FUNDEF_ISSTFUN (arg_node)) {
                        fprintf (global.outfile, " * ST function:\n");
                    } else if (FUNDEF_ISXTFUN (arg_node)) {
                        fprintf (global.outfile, " * XT function:\n");
                    } else if (FUNDEF_ISXTSPMDFUN (arg_node)) {
                        fprintf (global.outfile, " * XT SPMD function:\n");
                    } else if (FUNDEF_ISSPMDFUN (arg_node)) {
                        fprintf (global.outfile, " * SPMD function:\n");
                    }

                    if (global.backend == BE_cuda || global.backend == BE_cudahybrid) {
                        fprintf (global.outfile, " * WITH-loop Count: %d\n",
                                 FUNDEF_WLCOUNT (arg_node));
                    }

                    fprintf (global.outfile, " * ");
                    if (FUNDEF_NS (arg_node) != NULL) {
                        fprintf (global.outfile,
                                 "%s::", NSgetName (FUNDEF_NS (arg_node)));
                    }

                    fprintf (global.outfile,
                             "%s(...) [ %s ]\n"
                             " **********************************************************"
                             "******************/\n",
                             FUNDEF_NAME (arg_node),
                             (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "body"));

                    if ((FUNDEF_ICMDEFBEGIN (arg_node) == NULL)
                        || (NODE_TYPE (FUNDEF_ICMDEFBEGIN (arg_node)) != N_icm)) {
                        PrintFunctionHeader (arg_node, arg_info, FALSE);
                    } else {
                        TRAVdo (FUNDEF_ICMDEFBEGIN (arg_node), arg_info);
                    }

                    fprintf (global.outfile, "\n");

                    /*
                     * The following conditional is a cruel hack to get sac2tex
                     * going; a much better solution should be adopted once print.c
                     * is rewritten!!!! (sbs)
                     */
                    if (global.tool != TOOL_sac2tex) {
                        /* traverse function body */
                        TRAVdo (FUNDEF_BODY (arg_node), arg_info);
                    }

                    if ((global.compiler_subphase != PH_cg_prt)
                        && (global.compiler_subphase != PH_ccg_prt)) {
                        if (FUNDEF_PRAGMA (arg_node) != NULL) {
                            TRAVdo (FUNDEF_PRAGMA (arg_node), arg_info);
                        }
                    }

                    fprintf (global.outfile, "\n");

                    if (!((FUNDEF_ICMDEFBEGIN (arg_node) == NULL)
                          || (NODE_TYPE (FUNDEF_ICMDEFBEGIN (arg_node)) != N_icm))) {
                        TRAVdo (FUNDEF_ICMDEFEND (arg_node), arg_info);
                    }

                    fprintf (global.outfile, "\n");

                    if (INFO_SEPARATE (arg_info)
                        && (INFO_FUNCOUNTER (arg_info) >= global.linksetsize)
                        && ((FUNDEF_NEXT (arg_node) == NULL)
                            || !FUNDEF_ISSPMDFUN (FUNDEF_NEXT (arg_node)))) {
                        fclose (global.outfile);
                        global.outfile = NULL;
                        INFO_FUNCOUNTER (arg_info) = 0;
                    }

                    if (FUNDEF_LOCALFUNS (arg_node) != NULL) {

                        /*
                         * if the printfun option is specified this will print above the
                         * list of local fundefs. This is related to the -printfun option.
                         */
                        if (global.break_fun_name != NULL) {
                            fprintf (global.outfile,
                                     "\n"
                                     "/**************************************************"
                                     "**************************\n"
                                     " * Begin local fundefs of %s:\n"
                                     " **************************************************"
                                     "**************************/"
                                     "\n",
                                     FUNDEF_NAME (arg_node));
                        }

                        INFO_NONLOCCALFUN (arg_info) = arg_node;
                        TRAVdo (FUNDEF_LOCALFUNS (arg_node), arg_info);
                        INFO_NONLOCCALFUN (arg_info) = NULL;

                        if (global.break_fun_name != NULL) {
                            fprintf (global.outfile,
                                     "\n"
                                     "/**************************************************"
                                     "**************************\n"
                                     " * End local fundefs of %s:\n"
                                     " **************************************************"
                                     "**************************/"
                                     "\n",
                                     FUNDEF_NAME (arg_node));
                        }

                    }
                }
            }
        }
        /**end selective print**/
    }

    DBUG_ASSERT (global.indent == old_indent,
                 "Indentation unbalanced while printing function '%s`.\n"
                 " Indentation at beginning of function: %zu.\n"
                 " Indentation at end of function: %zu\n",
                 FUNDEF_NAME (arg_node), old_indent, global.indent);

    if (FUNDEF_NEXT (arg_node) != NULL) { /* traverse next function */
        PRINT_CONT (TRAVdo (FUNDEF_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTannotate( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTannotate (node *arg_node, info *arg_info)
{
    str_buf *buf1, *buf2;

    DBUG_ENTER ();

    buf1 = SBUFcreate (512);
    buf2 = SBUFcreate (512);

    DBUG_PRINT ("%s " F_PTR, NODE_TEXT (arg_node), (void *)arg_node);

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (ANNOTATE_TAG (arg_node) & CALL_FUN) {
        SBUFprintf (buf1, "PROFILE_BEGIN_UDF( %d, %d)", ANNOTATE_FUNNUMBER (arg_node),
                 ANNOTATE_FUNAPNUMBER (arg_node));
    } else {
        if (ANNOTATE_TAG (arg_node) & RETURN_FROM_FUN) {
            SBUFprintf (buf1, "PROFILE_END_UDF( %d, %d)",
                     ANNOTATE_FUNNUMBER (arg_node), ANNOTATE_FUNAPNUMBER (arg_node));
        } else {
            DBUG_UNREACHABLE ("wrong tag at N_annotate");
        }
    }

    if (ANNOTATE_TAG (arg_node) & INL_FUN) {
        SBUFprintf (buf2, "PROFILE_INLINE( %s)", SBUFgetBuffer (buf1));
        if (ANNOTATE_TAG (arg_node) & LIB_FUN) {
            SBUFprintf (buf1, "PROFILE_LIBRARY( %s)", SBUFgetBuffer (buf2));
        } else {
            SBUFprintf (buf1, "%s", SBUFgetBuffer (buf2));
        }
    } else {
        SBUFprintf (buf2, "%s", SBUFgetBuffer (buf1));
        if (ANNOTATE_TAG (arg_node) & LIB_FUN) {
            SBUFprintf (buf1, "PROFILE_LIBRARY( %s)", SBUFgetBuffer (buf2));
        } else {
            SBUFprintf (buf1, "%s", SBUFgetBuffer (buf2));
        }
    }

    fprintf (global.outfile, "%s;", SBUFgetBuffer (buf1));

    buf1 = SBUFfree (buf1);
    buf2 = SBUFfree (buf2);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTarg( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTarg (node *arg_node, info *arg_info)
{
    char *type_str;
    const char *minmk;
    const char *maxmk;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (ARG_NTYPE (arg_node) != NULL) {
        type_str = TYtype2String (ARG_NTYPE (arg_node), FALSE, 0);
    } else {
        DBUG_ASSERT (FALSE, "encountered old types on args");
        type_str = NULL;
    }
    fprintf (global.outfile, " %s ", type_str);
    type_str = MEMfree (type_str);

    if (ARG_ISREFERENCE (arg_node)) {
        if (ARG_ISREADONLY (arg_node)) {
            fprintf (global.outfile, "(&)");
        } else {
            fprintf (global.outfile, "&");
        }
    }

    if (ARG_ISUNIQUE (arg_node)) {
        fprintf (global.outfile, "*");
    }

    if ((!INFO_OMIT_FORMAL_PARAMS (arg_info)) && (ARG_NAME (arg_node) != NULL)) {
        fprintf (global.outfile, "%s", ARG_NAME (arg_node));
    }

    if (ARG_ISCUDADEFINED (arg_node)) {
        fprintf (global.outfile, "[CD]");
    }

    if ((global.tool != TOOL_sac2tex) && (global.compiler_phase > PH_scp)) {

        fprintf (global.outfile, " { "); /* Start of avis info */
        if (AVIS_DIM (ARG_AVIS (arg_node)) != NULL) {
            fprintf (global.outfile, "dim: ");
            AVIS_DIM (ARG_AVIS (arg_node))
              = TRAVdo (AVIS_DIM (ARG_AVIS (arg_node)), arg_info);
        }
        if (AVIS_SHAPE (ARG_AVIS (arg_node)) != NULL) {
            fprintf (global.outfile, ", shape: ");
            AVIS_SHAPE (ARG_AVIS (arg_node))
              = TRAVdo (AVIS_SHAPE (ARG_AVIS (arg_node)), arg_info);
        }
        /* Print extrema information */
        minmk = AVIS_ISMINHANDLED (ARG_AVIS (arg_node)) ? "Y" : "N";
        maxmk = AVIS_ISMAXHANDLED (ARG_AVIS (arg_node)) ? "Y" : "N";
        fprintf (global.outfile, ",%s%s", minmk, maxmk);
        if (AVIS_MIN (ARG_AVIS (arg_node)) != NULL) {
            fprintf (global.outfile, ", minval: %s",
                     AVIS_NAME (ID_AVIS (AVIS_MIN (ARG_AVIS (arg_node)))));
        }
        if (AVIS_MAX (ARG_AVIS (arg_node)) != NULL) {
            fprintf (global.outfile, ", maxval: %s",
                     AVIS_NAME (ID_AVIS (AVIS_MAX (ARG_AVIS (arg_node)))));
        }

        if (AVIS_SCALARS (ARG_AVIS (arg_node)) != NULL) {
            fprintf (global.outfile, ", scalars: ");
            AVIS_SCALARS (ARG_AVIS (arg_node))
              = TRAVdo (AVIS_SCALARS (ARG_AVIS (arg_node)), arg_info);
        }

        if (AVIS_LACSO (ARG_AVIS (arg_node)) != NULL) {
            fprintf (global.outfile, ", lacso: ");
            AVIS_LACSO (ARG_AVIS (arg_node))
              = TRAVdo (AVIS_LACSO (ARG_AVIS (arg_node)), arg_info);
        }

        if (AVIS_ISDEAD (ARG_AVIS (arg_node)) != 0) {
            fprintf (global.outfile, ", ISDEAD");
        }

        fprintf (global.outfile, " } "); /* end of avis info */
    }
    TRAVdo (ARG_AVIS (arg_node), arg_info);

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, ",");
        PRINT_CONT (TRAVdo (ARG_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTudcs( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTudcs (node *arg_node, info *arg_info)
{
    TRAVdo (UDCS_UDC (arg_node), arg_info);

    if (NULL != UDCS_NEXT (arg_node)) {
        fprintf (global.outfile, ", ");
        TRAVdo (UDCS_NEXT (arg_node), arg_info);
    }
    return (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTvardec( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTvardec (node *arg_node, info *arg_info)
{
    char *type_str;
    const char *minmk;
    const char *maxmk;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    DBUG_EXECUTE_TAG ("PRINT_LINENO", fprintf (global.outfile, "\n#line %zu \"%s\"\n",
                                               global.linenum, global.filename););

    if ((VARDEC_ICM (arg_node) == NULL) || (NODE_TYPE (VARDEC_ICM (arg_node)) != N_icm)) {
        /* print mutc index specifier */
        if (AVIS_ISTHREADINDEX (VARDEC_AVIS (arg_node))) {
            fprintf (global.outfile, "index ");
        }

        type_str = TYtype2String (VARDEC_NTYPE (arg_node), FALSE, 0);
        fprintf (global.outfile, "%s ", type_str);
        type_str = MEMfree (type_str);

        /* Print SAA information */
        fprintf (global.outfile, "%s", VARDEC_NAME (arg_node));

        if (global.compiler_phase > PH_scp) {
            fprintf (global.outfile, " { "); /* Start of avis info */
            if (AVIS_DIM (VARDEC_AVIS (arg_node)) != NULL) {
                fprintf (global.outfile, "dim: ");
                AVIS_DIM (VARDEC_AVIS (arg_node))
                  = TRAVdo (AVIS_DIM (VARDEC_AVIS (arg_node)), arg_info);
            }
            if (AVIS_SHAPE (VARDEC_AVIS (arg_node)) != NULL) {
                fprintf (global.outfile, ", shape: ");
                AVIS_SHAPE (VARDEC_AVIS (arg_node))
                  = TRAVdo (AVIS_SHAPE (VARDEC_AVIS (arg_node)), arg_info);
            }
            /* Print extrema information */
            minmk = AVIS_ISMINHANDLED (VARDEC_AVIS (arg_node)) ? "Y" : "N";
            maxmk = AVIS_ISMAXHANDLED (VARDEC_AVIS (arg_node)) ? "Y" : "N";
            fprintf (global.outfile, ", %s%s", minmk, maxmk);
            if (AVIS_MIN (VARDEC_AVIS (arg_node)) != NULL) {
                fprintf (global.outfile, ", minval: %s",
                         AVIS_NAME (ID_AVIS (AVIS_MIN (VARDEC_AVIS (arg_node)))));
            }
            if (AVIS_MAX (VARDEC_AVIS (arg_node)) != NULL) {
                fprintf (global.outfile, ", maxval: %s",
                         AVIS_NAME (ID_AVIS (AVIS_MAX (VARDEC_AVIS (arg_node)))));
            }

            if (AVIS_SCALARS (VARDEC_AVIS (arg_node)) != NULL) {
                fprintf (global.outfile, ", scalars: ");
                AVIS_SCALARS (VARDEC_AVIS (arg_node))
                  = TRAVdo (AVIS_SCALARS (VARDEC_AVIS (arg_node)), arg_info);
            }

            if (AVIS_LACSO (VARDEC_AVIS (arg_node)) != NULL) {
                fprintf (global.outfile, ", lacso: ");
                AVIS_LACSO (VARDEC_AVIS (arg_node))
                  = TRAVdo (AVIS_LACSO (VARDEC_AVIS (arg_node)), arg_info);
            }

            if (AVIS_ISDEAD (VARDEC_AVIS (arg_node)) != 0) {
                fprintf (global.outfile, ", ISDEAD");
            }

            if (AVIS_SUBALLOC (VARDEC_AVIS (arg_node))) {
                fprintf (global.outfile, ", SUBALLOC");
            }

            if (AVIS_COUNT (VARDEC_AVIS (arg_node)) != 0) {
                fprintf (global.outfile, ", USAGE: %d",
                         AVIS_COUNT (VARDEC_AVIS (arg_node)));
            }

            fprintf (global.outfile, " } "); /* end of avis info */
        }

        if (VARDEC_INIT (arg_node) != NULL) {
            fprintf (global.outfile, " = ");
            VARDEC_INIT (arg_node) = TRAVdo (VARDEC_INIT (arg_node), arg_info);
        }

        fprintf (global.outfile, "; ");

        if (AVIS_DECLTYPE (VARDEC_AVIS (arg_node)) != NULL) {
            type_str = TYtype2String (AVIS_DECLTYPE (VARDEC_AVIS (arg_node)), FALSE, 0);
            fprintf (global.outfile, " /* declared: %s */", type_str);
            type_str = MEMfree (type_str);
        }

        if (global.print.avis) {
            fprintf (global.outfile, "/* avis %p  SSA assign: %p */",
                     (void *)VARDEC_AVIS (arg_node),
                     (void *)AVIS_SSAASSIGN (VARDEC_AVIS (arg_node)));
        }

        TRAVdo (VARDEC_AVIS (arg_node), arg_info);

        fprintf (global.outfile, "\n");
    } else {
        if (global.cc_debug_extra
            && ((global.compiler_subphase == PH_cg_prt)
                || (global.compiler_subphase == PH_ccg_prt))) {
            fprintf (global.outfile, "\n#line %zu \"%s\"\n", global.linenum,
                     global.filename);
        }
        TRAVdo (VARDEC_ICM (arg_node), arg_info);
        fprintf (global.outfile, "\n");
    }

    if (VARDEC_NEXT (arg_node)) {
        PRINT_CONT (TRAVdo (VARDEC_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTblock( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTblock (node *arg_node, info *arg_info)
{
    size_t old_indent = global.indent;

    DBUG_ENTER ();

    DBUG_PRINT ("%s " F_PTR, NODE_TEXT (arg_node), (void *)arg_node);

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, "{ \n");
    global.indent++;

    if (BLOCK_ISMTPARALLELBRANCH (arg_node)) {
        INDENT;
        fprintf (global.outfile, "/* MT parallel branch */\n");
    }

    if (BLOCK_ISMTSEQUENTIALBRANCH (arg_node)) {
        INDENT;
        fprintf (global.outfile, "/* MT sequential branch */\n");
    }

    if (BLOCK_CACHESIM (arg_node) != NULL) {
        INDENT;
        fprintf (global.outfile, "#pragma cachesim \"%s\"\n\n",
                 BLOCK_CACHESIM (arg_node));
    }

    if (BLOCK_VARDECS (arg_node) != NULL) {
        TRAVdo (BLOCK_VARDECS (arg_node), arg_info);
        fprintf (global.outfile, "\n");
    }

    DBUG_EXECUTE_TAG ("PRINT_SSA", if (BLOCK_SSACOUNTER (arg_node) != NULL) {
        INDENT;
        fprintf (global.outfile, "/* SSAcnt:\n");
        TRAVdo (BLOCK_SSACOUNTER (arg_node), arg_info);
        INDENT;
        fprintf (global.outfile, " */\n");
    });

    TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    global.indent--;
    INDENT;
    fprintf (global.outfile, "}");

    if (INFO_FUNDEF (arg_info) != NULL) {
        DBUG_ASSERT (global.indent == old_indent,
                     "Indentation unbalanced while printing function '%s`.\n"
                     " Indentation at beginning of function: %zu.\n"
                     " Indentation at end of function: %zu\n",
                     FUNDEF_NAME (INFO_FUNDEF (arg_info)), old_indent, global.indent);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTreturn( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "return");
    if (RETURN_EXPRS (arg_node) != NULL) {
        fprintf (global.outfile, "( ");
        TRAVdo (RETURN_EXPRS (arg_node), arg_info);
        fprintf (global.outfile, ")");
    }
    fprintf (global.outfile, "; ");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTassign( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTassign (node *arg_node, info *arg_info)
{
    node *instr;
    bool trav_instr;

    DBUG_ENTER ();

    DBUG_PRINT ("%s " F_PTR, NODE_TEXT (arg_node), (void *)arg_node);

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    DBUG_EXECUTE_TAG ("PRINT_LINENO", fprintf (global.outfile, "\n#line %zu \"%s\"\n",
                                               global.linenum, global.filename););

    if (global.cc_debug_extra
        && ((global.compiler_subphase == PH_cg_prt)
            || (global.compiler_subphase == PH_ccg_prt))) {
        fprintf (global.outfile, "\n#line %zu \"%s\"\n", global.linenum, global.filename);
    }

    instr = ASSIGN_STMT (arg_node);
    DBUG_ASSERT (instr != NULL, "instruction of N_assign is NULL");

    trav_instr = TRUE;
    if (NODE_TYPE (instr) == N_annotate) {
        if (global.compiler_phase < PH_cg) {
            trav_instr = FALSE;
            trav_instr = TRUE;
        }
        DBUG_EXECUTE_TAG ("PRINT_PROFILE", trav_instr = TRUE);
    }

    if (NODE_TYPE (instr) == N_let) {
        if (NODE_TYPE (LET_EXPR (instr)) == N_prf) {
            if (PRF_PRF (LET_EXPR (instr)) == F_host2device) {
                if (ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (arg_node)) {
                    fprintf (global.outfile, "/** Is NOT allowed to be moved up **/\n");
                } else {
                    fprintf (global.outfile, "/** Is allowed to be moved up **/\n");
                }
            }
            if (PRF_PRF (LET_EXPR (instr)) == F_device2host) {
                if (PRF_PRF (LET_EXPR (instr)) == F_device2host
                    && ASSIGN_ISNOTALLOWEDTOBEMOVEDDOWN (arg_node)) {
                    fprintf (global.outfile, "/** Is NOT allowed to be moved down **/\n");
                } else {
                    fprintf (global.outfile, "/** Is allowed to be moved down **/\n");
                }
            }
        }
    }

    if (global.backend == BE_cuda || global.backend == BE_cudahybrid) {
        switch (ASSIGN_EXECMODE (arg_node)) {
        case CUDA_HOST_SINGLE:
            // fprintf (global.outfile, "/** Execution Mode: Host Single **/\n");
            break;
        case CUDA_DEVICE_SINGLE:
            fprintf (global.outfile, "/** Execution Mode: Device Single **/\n");
            break;
        case CUDA_DEVICE_MULTI:
            fprintf (global.outfile, "/** Execution Mode: Device Multithreaded **/\n");
            break;
        default:
            fprintf (global.outfile, "/** Execution Mode: Unknown **/\n");
            break;
        }
    }

    if (trav_instr) {
        if (NODE_TYPE (instr) != N_icm) {
            INDENT;
        } else {
            last_assignment_icm = instr;
        }
        TRAVdo (instr, arg_info);

        if (global.print.avis) {
            fprintf (global.outfile, "/* addr: %p */", (void *)arg_node);
        }

        fprintf (global.outfile, "\n");
    }

    if ((global.backend == BE_cuda || global.backend == BE_cudahybrid)
        && ASSIGN_ACCESS_INFO (arg_node) != NULL) {
        CUAIprintCudaAccessInfo (arg_node, arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (ASSIGN_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTdo( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (DO_ISCUDARIZABLE (arg_node)) {
        fprintf (global.outfile, "/********** Cudarizable do loop **********/\n");
        INDENT;
    }

    if (DO_ISFORLOOP (arg_node)) {
        fprintf (global.outfile, "/********** For Loop **********/\n");
        INDENT;
    }

    if (DO_LABEL (arg_node) != NULL) {
        fprintf (global.outfile, "goto %s;\n", DO_LABEL (arg_node));
        INDENT;
    }
    fprintf (global.outfile, "do \n");

    if (DO_SKIP (arg_node) != NULL) {
        INDENT;
        fprintf (global.outfile, "{\n");
        global.indent += 1;
        TRAVdo (DO_SKIP (arg_node), arg_info);
        fprintf (global.outfile, "\n");
        global.indent -= 1;
        INDENT;
        fprintf (global.outfile, "%s:\n", DO_LABEL (arg_node));
        global.indent += 1;
    }

    if (DO_BODY (arg_node) != NULL) {
        TRAVdo (DO_BODY (arg_node), arg_info);
        fprintf (global.outfile, "\n");
    }

    if (DO_SKIP (arg_node) != NULL) {
        global.indent -= 1;
        INDENT;
        fprintf (global.outfile, "}\n");
    }

    INDENT;
    fprintf (global.outfile, "while (");
    TRAVdo (DO_COND (arg_node), arg_info);
    fprintf (global.outfile, ");");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTwhile( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "while (");
    TRAVdo (WHILE_COND (arg_node), arg_info);
    fprintf (global.outfile, ") \n");

    if (WHILE_BODY (arg_node) != NULL) {
        TRAVdo (WHILE_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTcond( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (global.optimize.dopra && COND_ISTHENNOOP (arg_node)) {
        fprintf (global.outfile, "/* Noop branch */\n");
    }

    fprintf (global.outfile, "if ");

    fprintf (global.outfile, "(");
    TRAVdo (COND_COND (arg_node), arg_info);
    fprintf (global.outfile, ") \n");

    if (COND_THEN (arg_node) != NULL) {
        TRAVdo (COND_THEN (arg_node), arg_info);
        fprintf (global.outfile, "\n");
    }

    if (global.optimize.dopra && COND_ISELSENOOP (arg_node)) {
        INDENT;
        fprintf (global.outfile, "/* Noop branch */\n");
    }

    INDENT;
    fprintf (global.outfile, "else\n");

    if (COND_ELSE (arg_node) != NULL) {
        TRAVdo (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTcast( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTcast (node *arg_node, info *arg_info)
{
    char *type_str;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    type_str = TYtype2String (CAST_NTYPE (arg_node), FALSE, 0);
    fprintf (global.outfile, "(%s) ", type_str);
    type_str = MEMfree (type_str);

    TRAVdo (CAST_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTlet( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTlet (node *arg_node, info *arg_info)
{
    node *expr;
    bool print_sac = TRUE;
    bool print_c = FALSE;
    bool print_argtab = FALSE;

    DBUG_ENTER ();

    DBUG_PRINT ("%s " F_PTR, NODE_TEXT (arg_node), (void *)arg_node);

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    expr = LET_EXPR (arg_node);
    if ((NODE_TYPE (expr) == N_ap) && (AP_ARGTAB (expr) != NULL)) {
        print_sac = FALSE;
        print_c = TRUE;

        DBUG_EXECUTE_TAG ("PRINT_ARGTAB", fprintf (global.outfile, "/* \n"); INDENT;
                          fprintf (global.outfile, " *  ");
                          PRTprintArgtab (AP_ARGTAB (expr), FALSE);
                          fprintf (global.outfile, "  */\n"); INDENT; print_sac = TRUE;
                          print_argtab = TRUE);
    }

    if (print_c) {
        node *tmp;

        tmp = Argtab2Let (expr);

        TRAVdo (tmp, arg_info);

        /*
         * we have to clean AP_FUNDEF in order to fool FreeAp()!!!
         */
        AP_FUNDEF (LET_EXPR (tmp)) = NULL;
        tmp = FREEdoFreeTree (tmp);

        if (!print_argtab) {
            PrintArgtags (AP_ARGTAB (expr), FALSE);
        }
    }

    if (print_sac) {
        if (print_c) {
            fprintf (global.outfile, "\n");
            INDENT;
            fprintf (global.outfile, "/*  ");
        }

        if (LET_IDS (arg_node) != NULL) {
            LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
            fprintf (global.outfile, " = ");
        }
        TRAVdo (expr, arg_info);

        fprintf (global.outfile, "; ");

        if (print_c) {
            fprintf (global.outfile, "\n");
            INDENT;
            fprintf (global.outfile, " */ ");
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTprf( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    DBUG_PRINT ("%s (%s)" F_PTR, NODE_TEXT (arg_node),
                global.prf_name[PRF_PRF (arg_node)], (void *)arg_node);

    fprintf (global.outfile, "%s(", global.prf_name[PRF_PRF (arg_node)]);

    if (PRF_ARGS (arg_node) != NULL) {
        fprintf (global.outfile, " ");
        TRAVdo (PRF_ARGS (arg_node), arg_info);
    }

    fprintf (global.outfile, ")");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTfuncond( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTfuncond (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "( ");
    TRAVdo (FUNCOND_IF (arg_node), arg_info);
    fprintf (global.outfile, " ? ");
    TRAVdo (FUNCOND_THEN (arg_node), arg_info);
    fprintf (global.outfile, " : ");
    TRAVdo (FUNCOND_ELSE (arg_node), arg_info);
    fprintf (global.outfile, " )");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTap( node  *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTap (node *arg_node, info *arg_info)
{
    node *fundef;
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fundef = AP_FUNDEF (arg_node);

    if (LEXERisOperator (FUNDEF_NAME (fundef))
        && !(FUNDEF_ISLOOPFUN (fundef) || FUNDEF_ISCONDFUN (fundef))) {
        PrintOperatorAp (arg_node, NULL, arg_info);
    } else {
        PrintFunAp (arg_node, NULL, arg_info);
    }

    DBUG_EXECUTE_TAG ("PRINT_PTR", fprintf (global.outfile, " /* ");
                      PRINT_POINTER (global.outfile, (void *)AP_FUNDEF (arg_node));
                      fprintf (global.outfile, " */ "));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTspap( node  *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTspap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (LEXERisOperator (SPAP_NAME (arg_node))) {
        PrintOperatorAp (NULL, arg_node, arg_info);
    } else {
        PrintFunAp (NULL, arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTspmop( node  *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTspmop (node *arg_node, info *arg_info)
{
    node *exprs;
    node *fun_ids;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (SPMOP_ISFIXED (arg_node)) {
        fprintf (global.outfile, "(");
    }

    exprs = SPMOP_EXPRS (arg_node);
    fun_ids = SPMOP_OPS (arg_node);

    while (fun_ids != NULL) {
        TRAVdo (EXPRS_EXPR (exprs), arg_info);

        if (SPID_NS (EXPRS_EXPR (fun_ids)) != NULL) {
            fprintf (global.outfile, " %s::", NSgetName (SPID_NS (EXPRS_EXPR (fun_ids))));
        } else {
            fprintf (global.outfile, " ");
        }

        fprintf (global.outfile, "%s ", SPID_NAME (EXPRS_EXPR (fun_ids)));

        exprs = EXPRS_NEXT (exprs);
        fun_ids = EXPRS_NEXT (fun_ids);
    }

    if (exprs) {
        TRAVdo (EXPRS_EXPR (exprs), arg_info);
    }

    if (SPMOP_ISFIXED (arg_node)) {
        fprintf (global.outfile, ")");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTarray( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTarray (node *arg_node, info *arg_info)
{
    int i;
    char *type_str;
    int old_print_dim = INFO_DIM (arg_info);
    shape *old_print_shape = INFO_SHAPE (arg_info);
    shape *old_print_shape_counter = INFO_SHAPE_COUNTER (arg_info);
    bool old_isarray = INFO_ISARRAY (arg_info);
    node *shpcounter;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (ARRAY_AELEMS (arg_node) != NULL) {

        INFO_DIM (arg_info) = ARRAY_FRAMEDIM (arg_node);
        INFO_SHAPE (arg_info) = SHcopyShape (ARRAY_FRAMESHAPE (arg_node));
        INFO_ISARRAY (arg_info) = TRUE;

        shpcounter = TCcreateZeroVector (ARRAY_FRAMEDIM (arg_node), T_int);
        INFO_SHAPE_COUNTER (arg_info) = SHarray2Shape (shpcounter);
        shpcounter = FREEdoFreeTree (shpcounter);

        for (i = 0; i < INFO_DIM (arg_info); i++)
            fprintf (global.outfile, "[ ");

        TRAVdo (ARRAY_AELEMS (arg_node), arg_info);

        for (i = 0; i < INFO_DIM (arg_info); i++)
            fprintf (global.outfile, " ]");

        SHfreeShape (INFO_SHAPE (arg_info));
        SHfreeShape (INFO_SHAPE_COUNTER (arg_info));
        INFO_ISARRAY (arg_info) = FALSE;
    } else {
        type_str = TYtype2String (ARRAY_ELEMTYPE (arg_node), FALSE, 0);
        fprintf (global.outfile, "[:%s]", type_str);
        type_str = MEMfree (type_str);
    }

    INFO_DIM (arg_info) = old_print_dim;
    INFO_SHAPE (arg_info) = old_print_shape;
    INFO_SHAPE_COUNTER (arg_info) = old_print_shape_counter;
    INFO_ISARRAY (arg_info) = old_isarray;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTexprs( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static
int ShapeInc (shape *shp, int idx)
{
    int res;
    res = SHgetExtent (shp, idx);
    SHsetExtent (shp, idx, res+1);
    return res+1;
}

node *
PRTexprs (node *arg_node, info *arg_info)
{
    int i;
    int j;
    bool old_isarray;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    /*
     * do not inherit whether we are inside an array
     */
    old_isarray = INFO_ISARRAY (arg_info);
    INFO_ISARRAY (arg_info) = FALSE;
    TRAVdo (EXPRS_EXPR (arg_node), arg_info);
    INFO_ISARRAY (arg_info) = old_isarray;

    if (EXPRS_NEXT (arg_node) != NULL) {
        if (INFO_ISARRAY (arg_info)) {
            for (i = INFO_DIM (arg_info) - 1;
                 (i >= 0)
                 && (ShapeInc (INFO_SHAPE_COUNTER (arg_info), i)
                     >= SHgetExtent (INFO_SHAPE (arg_info), i));
                 i--)
                SHsetExtent (INFO_SHAPE_COUNTER (arg_info), i, 0);
            for (j = INFO_DIM (arg_info) - 1; j > i; j--)
                fprintf (global.outfile, " ]");
            fprintf (global.outfile, ", ");
            for (j = INFO_DIM (arg_info) - 1; j > i; j--)
                fprintf (global.outfile, "[ ");
            PRINT_CONT (TRAVdo (EXPRS_NEXT (arg_node), arg_info), ;);
        } else {
            fprintf (global.outfile, ", ");
            PRINT_CONT (TRAVdo (EXPRS_NEXT (arg_node), arg_info), ;);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTid( node *arg_node, info *arg_info)
 *
 *   @brief print N_id node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTid (node *arg_node, info *arg_info)
{
    char *text;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if ((global.compiler_subphase == PH_cg_prt)
        || (global.compiler_subphase == PH_ccg_prt)) {
        if (ID_NT_TAG (arg_node) != NULL) {
            text = ID_NT_TAG (arg_node);
        } else if (ID_ICMTEXT (arg_node) != NULL) {
            text = ID_ICMTEXT (arg_node);
        } else if (ID_AVIS (arg_node) != NULL) {
            text = ID_NAME (arg_node);
        } else {
            text = "";
            DBUG_UNREACHABLE ("Found an Id node with neither NTtag nor ICMText nor Name");
        }
    } else {
        if (ID_AVIS (arg_node) != NULL) {
            text = ID_NAME (arg_node);
        } else if (ID_ICMTEXT (arg_node) != NULL) {
            text = ID_ICMTEXT (arg_node);
        } else {
            text = "";
            DBUG_UNREACHABLE ("Found an Id node with neither Avis nor ICMText");
        }
    }

    fprintf (global.outfile, "%s", text);

    DBUG_EXECUTE_TAG ("PRINT_TAGS", if (ID_NT_TAG (arg_node) != NULL) {
        fprintf (global.outfile, " /* tag: %s */", ID_NT_TAG (arg_node));
    });

    if (global.print.avis) {
        if (ID_AVIS (arg_node) != NULL) {
            fprintf (global.outfile, "/* avis: %p */", (void *)ID_AVIS (arg_node));
        }
    };

    DBUG_EXECUTE_TAG ("DL", if (ID_ISSCLPRF (arg_node)) {
        fprintf (global.outfile, " /* SCL */ ");
    });

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTspid( node *arg_node, info *arg_info)
 *
 *   @brief print N_spid node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTspid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (SPID_NS (arg_node) != NULL) {
        fprintf (global.outfile, "%s::", NSgetName (SPID_NS (arg_node)));
    }
    fprintf (global.outfile, "%s", SPID_NAME (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTglobobj( node *arg_node, info *arg_info)
 *
 *   @brief print N_globobj node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTglobobj (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if ((global.compiler_subphase == PH_cg_prt)
        || (global.compiler_subphase == PH_ccg_prt)) {
        DBUG_ASSERT (OBJDEF_NT_TAG (GLOBOBJ_OBJDEF (arg_node)) != NULL,
                     "found objdef without NT TAG");

        fprintf (global.outfile, "%s", OBJDEF_NT_TAG (GLOBOBJ_OBJDEF (arg_node)));
    } else {
        if (OBJDEF_NS (GLOBOBJ_OBJDEF (arg_node)) != NULL) {
            fprintf (global.outfile,
                     "%s::", NSgetName (OBJDEF_NS (GLOBOBJ_OBJDEF (arg_node))));
        }

        fprintf (global.outfile, "%s", OBJDEF_NAME (GLOBOBJ_OBJDEF (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTnum( node *arg_node, info *arg_info)
 *
 *   @brief print N_num node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnum (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "%d", NUM_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTnumbyte( node *arg_node, info *arg_info)
 *
 *   @brief print N_numbyte node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnumbyte (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if ((global.compiler_subphase == PH_cg_prt)
        || (global.compiler_subphase == PH_ccg_prt)) {
        fprintf (global.outfile, "%d", NUMBYTE_VAL (arg_node));
    } else {
        fprintf (global.outfile, "%db", NUMBYTE_VAL (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTnumshort( node *arg_node, info *arg_info)
 *
 *   @brief print N_numshort node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnumshort (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if ((global.compiler_subphase == PH_cg_prt)
        || (global.compiler_subphase == PH_ccg_prt)) {
        fprintf (global.outfile, "%hd", NUMSHORT_VAL (arg_node));
    } else {
        fprintf (global.outfile, "%hds", NUMSHORT_VAL (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTnumint( node *arg_node, info *arg_info)
 *
 *   @brief print N_numint node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnumint (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "%d", NUMINT_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTnumlong( node *arg_node, info *arg_info)
 *
 *   @brief print N_numlong node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnumlong (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "%ldl", NUMLONG_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTnumlonglong( node *arg_node, info *arg_info)
 *
 *   @brief print N_numlonglong node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnumlonglong (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "%lldLL", NUMLONGLONG_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTnumubyte( node *arg_node, info *arg_info)
 *
 *   @brief print N_numubyte node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnumubyte (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if ((global.compiler_subphase == PH_cg_prt)
        || (global.compiler_subphase == PH_ccg_prt)) {
        fprintf (global.outfile, "%u", NUMUBYTE_VAL (arg_node));
    } else {
        fprintf (global.outfile, "%uub", NUMUBYTE_VAL (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTnumushort( node *arg_node, info *arg_info)
 *
 *   @brief print N_numushort node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnumushort (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if ((global.compiler_subphase == PH_cg_prt)
        || (global.compiler_subphase == PH_ccg_prt)) {
        fprintf (global.outfile, "%hu", NUMUSHORT_VAL (arg_node));
    } else {
        fprintf (global.outfile, "%huus", NUMUSHORT_VAL (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTnumint( node *arg_node, info *arg_info)
 *
 *   @brief print N_numint node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnumuint (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if ((global.compiler_subphase == PH_cg_prt)
        || (global.compiler_subphase == PH_ccg_prt)) {
        fprintf (global.outfile, "%u", NUMUINT_VAL (arg_node));
    } else {
        fprintf (global.outfile, "%uui", NUMUINT_VAL (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTnumulong( node *arg_node, info *arg_info)
 *
 *   @brief print N_numulong node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnumulong (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "%luul", NUMULONG_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTnumulonglong( node *arg_node, info *arg_info)
 *
 *   @brief print N_numulonglong node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnumulonglong (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "%lluULL", NUMULONGLONG_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTfloat( node *arg_node, info *arg_info)
 *
 *   @brief print N_float node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTfloat (node *arg_node, info *arg_info)
{
    char *tmp_string;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    tmp_string = CVfloat2String (FLOAT_VAL (arg_node));
    fprintf (global.outfile, "%s", tmp_string);
    tmp_string = MEMfree (tmp_string);

    DBUG_RETURN (arg_node);
}

node *
PRTfloatvec (node *arg_node, info *arg_info)
{
    floatvec val;
    float scal;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    val = FLOATVEC_VAL (arg_node);
    fprintf (global.outfile, "((floatvec){");
    for (size_t i = 0; i < sizeof (floatvec) / sizeof (float); i++) {
        scal = ((float *)&val)[i];
        if (i != sizeof (floatvec) / sizeof (float) - 1)
            fprintf (global.outfile, "%f, ", (double)scal);
        else
            fprintf (global.outfile, "%f})", (double)scal);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTdouble( node *arg_node, info *arg_info)
 *
 *   @brief print N_double node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTdouble (node *arg_node, info *arg_info)
{
    char *tmp_string;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    tmp_string = CVdouble2String (DOUBLE_VAL (arg_node));
    fprintf (global.outfile, "%s", tmp_string);
    tmp_string = MEMfree (tmp_string);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTbool( node *arg_node, info *arg_info)
 *
 *   @brief print N_bool node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTbool (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (0 == BOOL_VAL (arg_node)) {
        fprintf (global.outfile, "false");
    } else {
        fprintf (global.outfile, "true");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTneste_init( node *arg_node, info *arg_info)
 *
 *   @brief print N_nested_init node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTnested_init (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "nested_init");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTstr( node *arg_node, info *arg_info)
 *
 *   @brief print N_str node
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
PRTstr (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "\"%s\"", STR_STRING (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * Function:
 *   node *PRTtype( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTtype (node *arg_node, info *arg_info)
{
    char *type_str;

    DBUG_ENTER ();

    DBUG_PRINT ("%s " F_PTR, NODE_TEXT (arg_node), (void *)arg_node);

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (TYPE_TYPE (arg_node) != NULL) {
        type_str = TYtype2String (TYPE_TYPE (arg_node), FALSE, 0);
        fprintf (global.outfile, "%s", type_str);
        type_str = MEMfree (type_str);

#ifndef DBUG_OFF
        if (TYisBottom (TYPE_TYPE (arg_node))) {
            fprintf (global.outfile, " /* %s */",
                     TYgetBottomError (TYPE_TYPE (arg_node)));
        }
#endif
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *  node *PRTdot( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTdot (node *arg_node, info *arg_info)
{
    int i;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    for (i = 0; i < DOT_NUM (arg_node); i++) {
        fprintf (global.outfile, ".");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *  node *PRTsetwl( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTsetwl (node *arg_node, info *arg_info)
{
    bool old_insetwl;
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    old_insetwl = INFO_INSETWL (arg_info);
    if (!INFO_INSETWL (arg_info)) {
        fprintf (global.outfile, "{ ");
    } else {
        INFO_INSETWL (arg_info) = FALSE;
    }
    if (NODE_TYPE (SETWL_VEC (arg_node)) == N_exprs) {
        fprintf (global.outfile, "[");
        TRAVdo (SETWL_VEC (arg_node), arg_info);
        fprintf (global.outfile, "] -> ");
    } else {
        TRAVdo (SETWL_VEC (arg_node), arg_info);
        fprintf (global.outfile, " -> ");
    }

    TRAVdo (SETWL_EXPR (arg_node), arg_info);

    if (SETWL_GENERATOR (arg_node) != NULL) {
        fprintf (global.outfile, "| ");
        INFO_INSETWL (arg_info) = TRUE;
        TRAVdo (SETWL_GENERATOR (arg_node), arg_info);
    }

    if (SETWL_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, ";\n");
        INDENT;
        INFO_INSETWL (arg_info) = TRUE;
        TRAVdo (SETWL_NEXT (arg_node), arg_info);
    }

    if (!old_insetwl) {
        INDENT;
        fprintf (global.outfile, "}");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTchar( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTchar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if ((CHAR_VAL (arg_node) >= ' ') && (CHAR_VAL (arg_node) <= '~')
        && (CHAR_VAL (arg_node) != '\'') && (CHAR_VAL (arg_node) != '\\')) {
        fprintf (global.outfile, "'%c'", CHAR_VAL (arg_node));
    } else {
        switch (CHAR_VAL (arg_node)) {
        case '\n':
            fprintf (global.outfile, "'\\n'");
            break;
        case '\t':
            fprintf (global.outfile, "'\\t'");
            break;
        case '\v':
            fprintf (global.outfile, "'\\v'");
            break;
        case '\b':
            fprintf (global.outfile, "'\\b'");
            break;
        case '\r':
            fprintf (global.outfile, "'\\r'");
            break;
        case '\f':
            fprintf (global.outfile, "'\\f'");
            break;
        case '\a':
            fprintf (global.outfile, "'\\a'");
            break;
        case '\\':
            fprintf (global.outfile, "'\\\\'");
            break;
        default:
            fprintf (global.outfile, "'\\%o'", CHAR_VAL (arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTicm( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTicm (node *arg_node, info *arg_info)
{
    bool compiled_icm = FALSE;

    DBUG_ENTER ();

    DBUG_PRINT ("icm-node %s\n", ICM_NAME (arg_node));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if ((global.compiler_subphase == PH_cg_prt)
        || (global.compiler_subphase == PH_ccg_prt)) {

#define ICM_ALL
#define ICM_DEF(prf, trf)                                                                \
    if (STReq (ICM_NAME (arg_node), #prf)) {                                             \
        if (last_assignment_icm == arg_node) {                                           \
            INDENT;                                                                      \
        }                                                                                \
        Print##prf (ICM_ARGS (arg_node), arg_info);                                      \
        compiled_icm = TRUE;                                                             \
    } else
#define ICM_ANY(name)
#define ICM_ICM(name)
#define ICM_NT(name)
#define ICM_ID(name)
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_UINT(name)
#define ICM_BOOL(name)
#define ICM_VARANY(dim, name)
#define ICM_VARNT(dim, name)
#define ICM_VARID(dim, name)
#define ICM_VARINT(dim, name)
#define ICM_END(prf, args)
#include "icm.data"
#undef ICM_ALL
#undef ICM_DEF
#undef ICM_ANY
#undef ICM_ICM
#undef ICM_NT
#undef ICM_ID
#undef ICM_STR
#undef ICM_INT
#undef ICM_UINT
#undef ICM_BOOL
#undef ICM_VARANY
#undef ICM_VARNT
#undef ICM_VARID
#undef ICM_VARINT
#undef ICM_END
        {
        }
    }

    if (!compiled_icm) {
        if (last_assignment_icm == arg_node) {
            global.indent += ICM_INDENT_BEFORE (arg_node);
            INDENT;
        }

        /*
         * By default we prefix all h-ICMs with SAC_.
         * However, some ICMs are already prefixed for some reason.
         * If so, we avoid a double prefixing.
         */
        if (STRprefix ("SAC_", ICM_NAME (arg_node))) {
            fprintf (global.outfile, "%s", ICM_NAME (arg_node));
        } else {
            fprintf (global.outfile, "SAC_%s", ICM_NAME (arg_node));
        }

        fprintf (global.outfile, "(");

        if (ICM_ARGS (arg_node) != NULL) {
            TRAVdo (ICM_ARGS (arg_node), arg_info);
        }

        fprintf (global.outfile, ")");

        if (last_assignment_icm == arg_node) {
            global.indent += ICM_INDENT_AFTER (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTpragma( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void *
StringSetPrint (const char *entry, strstype_t kind, void *rest)
{
    fprintf (global.outfile, " \"%s\"", entry);
    return ((void *)0);
}

node *
PRTpragma (node *arg_node, info *arg_info)
{
    int i;
    node *nums;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (PRAGMA_LINKNAME (arg_node) != NULL) {
        fprintf (global.outfile, "#pragma linkname \"%s\"\n", PRAGMA_LINKNAME (arg_node));
    }

    if (PRAGMA_LINKOBJ (arg_node) != NULL) {
        fprintf (global.outfile, "#pragma linkobj");
        STRSfold (&StringSetPrint, PRAGMA_LINKOBJ (arg_node), NULL);
        fprintf (global.outfile, "\n");
    }

    if (PRAGMA_LINKSIGN (arg_node) != NULL) {
        fprintf (global.outfile, "#pragma linksign [");
        nums = PRAGMA_LINKSIGN (arg_node);
        if (PRAGMA_NUMPARAMS (arg_node) > 0) {
            fprintf (global.outfile, "%d", NUMS_VAL (nums));
            nums = NUMS_NEXT (nums);
        }
        for (i = 1; i < PRAGMA_NUMPARAMS (arg_node); i++) {
            fprintf (global.outfile, ", %d", NUMS_VAL (nums));
            nums = NUMS_NEXT (nums);
        }
        fprintf (global.outfile, "]\n");
    }
#if 0 /*    TODO    */
  if (PRAGMA_REFCOUNTING (arg_node) != NULL) {
    fprintf (global.outfile, "#pragma refcounting [");
    first = 1;
    for (i = 0; i < PRAGMA_NUMPARAMS (arg_node); i++) {
      if (PRAGMA_RC (arg_node, i)) {
        if (first) {
          fprintf (global.outfile, "%d", i);
          first = 0;
        } else {
          fprintf (global.outfile, ", %d", i);
        }
      }
    }
    fprintf (global.outfile, "]\n");
  }

  if (PRAGMA_READONLY (arg_node) != NULL) {
    fprintf (global.outfile, "#pragma readonly [");
    first = 1;
    for (i = 0; i < PRAGMA_NUMPARAMS (arg_node); i++) {
      if (PRAGMA_RO (arg_node, i)) {
        if (first) {
          fprintf (global.outfile, "%d", i);
          first = 0;
        } else {
          fprintf (global.outfile, ", %d", i);
        }
      }
    }
    fprintf (global.outfile, "]\n");
  }
#endif

    if (PRAGMA_EFFECT (arg_node) != NULL) {
        fprintf (global.outfile, "#pragma effect ");
        TRAVdo (PRAGMA_EFFECT (arg_node), arg_info);
        fprintf (global.outfile, "\n");
    }

    if (PRAGMA_COPYFUN (arg_node) != NULL) {
        fprintf (global.outfile, "#pragma copyfun \"%s\"\n", PRAGMA_COPYFUN (arg_node));
    }

    if (PRAGMA_FREEFUN (arg_node) != NULL) {
        fprintf (global.outfile, "#pragma freefun \"%s\"\n", PRAGMA_FREEFUN (arg_node));
    }

    if (PRAGMA_INITFUN (arg_node) != NULL) {
        fprintf (global.outfile, "#pragma initfun \"%s\"\n", PRAGMA_INITFUN (arg_node));
    }

    if (PRAGMA_WLCOMP_APS (arg_node) != NULL) {
        fprintf (global.outfile, "#pragma wlcomp ");
        TRAVdo (PRAGMA_WLCOMP_APS (arg_node), arg_info);
        fprintf (global.outfile, "\n");
    }

    if (PRAGMA_APL (arg_node) != NULL) {
        fprintf (global.outfile, "#pragma wlcomp ");
        TRAVdo (PRAGMA_APL (arg_node), arg_info);
        fprintf (global.outfile, "\n");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTmt(node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTmt (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }
#if 0 /***** TODO   */
  /* PrintAssign already indents */
  fprintf (global.outfile, "MT {");
  fprintf (global.outfile, " /*** begin of mt cell ***/\n");

  if (MT_USEMASK (arg_node) != NULL) {
    INDENT;
    fprintf (global.outfile, "/* use:");
    DFMPrintMask (global.outfile, " %s", MT_USEMASK (arg_node));
    fprintf (global.outfile, " */\n");
  }
  if (MT_DEFMASK (arg_node) != NULL) {
    INDENT;
    fprintf (global.outfile, "/* def:");
    DFMPrintMask (global.outfile, " %s", MT_DEFMASK (arg_node));
    fprintf (global.outfile, " */\n");
  }
  if (MT_NEEDLATER (arg_node) != NULL) {
    INDENT;
    fprintf (global.outfile, "/* needlater:");
    DFMPrintMask (global.outfile, " %s", MT_NEEDLATER (arg_node));
    fprintf (global.outfile, " */\n");
  }

  global.indent++;
  if (MT_REGION (arg_node) != NULL) {
    TRAVdo (MT_REGION (arg_node), arg_info);
  } else {
    INDENT;
    fprintf (global.outfile, "/* ... Empty ... */");
  }
  global.indent--;

  fprintf (global.outfile, "\n");
  INDENT;
  fprintf (global.outfile, "} /*** end of mt cell ***/\n");

#endif

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTex(node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTex (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    /* PrintAssign already indents */
    fprintf (global.outfile, "EX {");
    fprintf (global.outfile, " /*** begin of exclusive cell ***/\n");

    global.indent++;
    if (EX_REGION (arg_node) != NULL) {
        TRAVdo (EX_REGION (arg_node), arg_info);
    } else {
        INDENT;
        fprintf (global.outfile, "/* ... Empty ... */");
    }
    global.indent--;

    fprintf (global.outfile, "\n");
    INDENT;
    fprintf (global.outfile, "} /*** end of exclusive cell ***/\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTst(node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTst (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    /* PrintAssign already indents */
    fprintf (global.outfile, "ST {");
    fprintf (global.outfile, " /*** begin of st cell ***/\n");

    global.indent++;
    if (ST_REGION (arg_node) != NULL) {
        TRAVdo (ST_REGION (arg_node), arg_info);
    } else {
        INDENT;
        fprintf (global.outfile, "/* ... Empty ... */");
    }
    global.indent--;

    fprintf (global.outfile, "\n");
    INDENT;
    fprintf (global.outfile, "} /*** end of st cell ***/\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTcudast(node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTcudast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    /* PrintAssign already indents */
    fprintf (global.outfile, "CUDAST {");
    fprintf (global.outfile, " /*** begin of cudast cell ***/\n");

    global.indent++;
    if (CUDAST_REGION (arg_node) != NULL) {
        TRAVdo (CUDAST_REGION (arg_node), arg_info);
    } else {
        INDENT;
        fprintf (global.outfile, "/* ... Empty ... */");
    }
    global.indent--;

    fprintf (global.outfile, "\n");
    INDENT;
    fprintf (global.outfile, "} /*** end of cudast cell ***/\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTwith( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_with node.
 *
 ******************************************************************************/

node *
PRTwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    global.indent++;

    if (WITH_PRAGMA (arg_node) != NULL) {
        TRAVdo (WITH_PRAGMA (arg_node), arg_info);
        INDENT;
    }

    global.indent++;

    fprintf (global.outfile, "with");

    if (WITH_CUDARIZABLE (arg_node)) {
        INDENT;
        fprintf (global.outfile, "/** CUDA WL **/\n");
    }

    if (WITH_CUDARIZABLE (arg_node) && WITH_HASRC (arg_node)) {
        INDENT;
        fprintf (global.outfile, "/** WL has reuse candidate **/\n");
    }

    if (WITH_ISFOLDABLE (arg_node)) {
        INDENT;
        fprintf (global.outfile, "/** FOLDABLE (all gen's const) **/\n");
    }

    if (WITH_DIST (arg_node) != NULL) {
        INDENT;
        fprintf (global.outfile, "/** WL DIST = \"%s\" **/\n", WITH_DIST (arg_node));
    }

    if (WITH_REFERENCED (arg_node) > 0) {
        INDENT;
        fprintf (global.outfile, "/** REFERENCED: %d (total num refs) **/\n",
                 WITH_REFERENCED (arg_node));
    }

    if (WITH_REFERENCED_FOLD (arg_node) > 0) {
        INDENT;
        fprintf (global.outfile, "/** REFERENCED_FOLD: %d (num refs in fold pos.) **/\n",
                 WITH_REFERENCED_FOLD (arg_node));
    }

    if (WITH_REFERENCES_FOLDED (arg_node) > 0) {
        INDENT;
        fprintf (global.outfile,
                 "/** REFERENCES_FOLDED: %d (num refs folded already) **/\n",
                 WITH_REFERENCES_FOLDED (arg_node));
    }

    if (WITH_PART (arg_node) != NULL) {
#ifdef OLDWAY
        fprintf (global.outfile, " ( ");
        if (WITHID_VEC (PART_WITHID (WITH_PART (arg_node))) != NULL) {
            TRAVdo (WITHID_VEC (PART_WITHID (WITH_PART (arg_node))), arg_info);
        } else {
            fprintf (global.outfile, "[ ");
            TRAVdo (WITHID_IDS (PART_WITHID (WITH_PART (arg_node))), arg_info);
            fprintf (global.outfile, " ]");
        }
        fprintf (global.outfile, " )\n");
        global.indent++;
        TRAVdo (WITH_PART (arg_node), arg_info);
        global.indent--;
#else  // OLDWAY
        /* All partition definitions appear within curly braces */
        fprintf (global.outfile, " {\n");
        global.indent++;
        TRAVdo (WITH_PART (arg_node), arg_info);
        INDENT
        fprintf (global.outfile, " } :\n");
        global.indent--;
#endif // OLDWAY
    } else {
        fprintf (global.outfile, "\n");
    }

    /* genarray, etc. */
    if (WITH_WITHOP (arg_node) != NULL) {
        TRAVdo (WITH_WITHOP (arg_node), arg_info);
    } else {
        INDENT;
        fprintf (global.outfile, "void ");
    }

    global.indent--;

    DBUG_EXECUTE_TAG ("PRINT_RC", if (WITH_PRAGMA (arg_node) == NULL) {
        fprintf (global.outfile, "\n");
        INDENT;
    } INDENT);

    global.indent--;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTwithid(node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_withid-nodes
 *
 ******************************************************************************/

node *
PRTwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (WITHID_VEC (arg_node) != NULL) {
        TRAVdo (WITHID_VEC (arg_node), arg_info);
        if (WITHID_IDS (arg_node) != NULL) {
            fprintf (global.outfile, "=");
        }
    }

    if (WITHID_IDS (arg_node) != NULL) {
        fprintf (global.outfile, "[");
        TRAVdo (WITHID_IDS (arg_node), arg_info);
        fprintf (global.outfile, "]");
    }

    if (WITHID_IDXS (arg_node) != NULL) {
        fprintf (global.outfile, " (IDXS:");
        TRAVdo (WITHID_IDXS (arg_node), arg_info);
        fprintf (global.outfile, ")");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTgenerator(node *arg_node, info *arg_info)
 *
 * description:
 *   prints a generator.
 *
 *   The index variable is found in NPART_WITHID( INFO_NPART( arg_info)).
 *
 ******************************************************************************/

node *
PRTgenerator (node *arg_node, info *arg_info)
{
    bool insetwl;
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    insetwl = INFO_INSETWL (arg_info);
    INFO_INSETWL (arg_info) = FALSE;

    if (!insetwl) {
        fprintf (global.outfile, "(");
    }

    /* print upper bound */
    if (GENERATOR_BOUND1 (arg_node)) {
        TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);
    } else {
        fprintf (global.outfile, "(NULL)");
    }

    /* print first operator */
    if (GENERATOR_OP1 (arg_node) == F_wl_lt) {
        fprintf (global.outfile, " < ");
    } else {
        fprintf (global.outfile, " <= ");
    }

    /* print indices */
    if (INFO_NPART (arg_info) != NULL) {
        DBUG_ASSERT (NODE_TYPE (INFO_NPART (arg_info)) == N_part,
                     "INFO_NPART is no N_part node");

        DBUG_ASSERT (PART_WITHID (INFO_NPART (arg_info)) != NULL,
                     "PART_WITHID not found!");
        TRAVdo (PART_WITHID (INFO_NPART (arg_info)), arg_info);
    } else {
        fprintf (global.outfile, "?");
    }

    /* print second operator */
    if (GENERATOR_OP2 (arg_node) == F_wl_lt) {
        fprintf (global.outfile, " < ");
    } else {
        fprintf (global.outfile, " <= ");
    }

    /* print lower bound */
    if (GENERATOR_BOUND2 (arg_node)) {
        TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);
    } else {
        fprintf (global.outfile, "(NULL)");
    }

    /* print step and width */
    if (GENERATOR_STEP (arg_node)) {
        fprintf (global.outfile, " step ");
        TRAVdo (GENERATOR_STEP (arg_node), arg_info);
    }
    if (GENERATOR_WIDTH (arg_node)) {
        fprintf (global.outfile, " width ");
        TRAVdo (GENERATOR_WIDTH (arg_node), arg_info);
    }

    if (GENERATOR_GENWIDTH (arg_node) != NULL) {
        fprintf (global.outfile, " genwidth ");
        TRAVdo (GENERATOR_GENWIDTH (arg_node), arg_info);
    }
    if (!insetwl) {
        fprintf (global.outfile, ")\n");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTdefault(node *arg_node, info *arg_info)
 *
 * description:
 *   prints a default node.
 *
 *   The index variable is found in NPART_WITHID( INFO_NPART( arg_info)).
 *
 ******************************************************************************/

node *
PRTdefault (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "default partition( ");

    /* print indices */
    if (INFO_NPART (arg_info) != NULL) {
        DBUG_ASSERT (NODE_TYPE (INFO_NPART (arg_info)) == N_part,
                     "INFO_NPART is no N_part node");

        DBUG_ASSERT (PART_WITHID (INFO_NPART (arg_info)) != NULL,
                     "PART_WITHID not found!");
        TRAVdo (PART_WITHID (INFO_NPART (arg_info)), arg_info);
    } else {
        fprintf (global.outfile, "?");
    }

    fprintf (global.outfile, " ):\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTcode( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_code-nodes
 *
 ******************************************************************************/

node *
PRTcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    DBUG_ASSERT (CODE_USED (arg_node) >= 0, "illegal CODE_USED value!");

    /* print the code section; the body first */
    TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    if (CODE_CEXPRS (arg_node) != NULL) {
        fprintf (global.outfile, " : ");
        TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    }

    fprintf (global.outfile, " ; ");

    if ((global.backend == BE_cuda || global.backend == BE_cudahybrid)
        && CODE_IRA_INFO (arg_node) != NULL) {
        IRAprintRcs (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTpart( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_part nodes
 *
 ******************************************************************************/

node *
PRTpart (node *arg_node, info *arg_info)
{
    node *tmp_npart;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    tmp_npart = INFO_NPART (arg_info);
    INFO_NPART (arg_info) = arg_node;

    // For automated partition counting by unit testing code
    INDENT
    fprintf (global.outfile, "/* Partn */\n");

    if (PART_CUDARIZABLE (arg_node)) {
        INDENT
        fprintf (global.outfile, "/*** CUDA Partition ***/\n");
    } else {
        // fprintf (global.outfile, "/*** Partition ***/\n");
    }

    if (PART_THREADBLOCKSHAPE (arg_node) != NULL) {
        INDENT
        fprintf (global.outfile, "/*** Thread Block Shape: ");
        PRTarray (PART_THREADBLOCKSHAPE (arg_node), arg_info);
        fprintf (global.outfile, " ***/\n");
    }

    if (PART_ISCOPY (arg_node)) {
        INDENT
        fprintf (global.outfile, "/*** Copy Partition ***/\n");
    }

    /* print generator */
    INDENT; /* each gen in a new line. */
    TRAVdo (PART_GENERATOR (arg_node), arg_info);

    DBUG_ASSERT (PART_CODE (arg_node) != NULL,
                 "part within WL without pointer to N_code");

    TRAVdo (PART_CODE (arg_node), arg_info);

    if (PART_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, "\n");
        /*
         * continue with other parts
         */
        PRINT_CONT (TRAVdo (PART_NEXT (arg_node), arg_info), ;);
    } else {
        fprintf (global.outfile, "\n");
    }

    INFO_NPART (arg_info) = tmp_npart;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTgenarray( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/

node *
PRTgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    fprintf (global.outfile, "genarray( ");
    TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        if (GENARRAY_DEFSHAPEEXPR (arg_node) != NULL) {
            fprintf (global.outfile, " , genarray( ");
            TRAVdo (GENARRAY_DEFSHAPEEXPR (arg_node), arg_info);
            fprintf (global.outfile, " ,");
            TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
            fprintf (global.outfile, ")");
        } else {
            fprintf (global.outfile, ", ");
            TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
        }
    }

    if (GENARRAY_MEM (arg_node) != NULL) {
        fprintf (global.outfile, ", ");
        TRAVdo (GENARRAY_MEM (arg_node), arg_info);
    }

    if (GENARRAY_SUB (arg_node) != NULL) {
        fprintf (global.outfile, ", SUB(");
        TRAVdo (GENARRAY_SUB (arg_node), arg_info);
        fprintf (global.outfile, ")");
    }

    if (GENARRAY_IDX (arg_node) != NULL) {
        fprintf (global.outfile, ", IDX(%s)", AVIS_NAME (GENARRAY_IDX (arg_node)));
    }
    fprintf (global.outfile, ")");

    if (GENARRAY_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, ",\n");
        /*
         * continue with other withops
         */
        PRINT_CONT (TRAVdo (GENARRAY_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTmodarray( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/

node *
PRTmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    fprintf (global.outfile, "modarray( ");

    TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    if (MODARRAY_MEM (arg_node) != NULL) {
        fprintf (global.outfile, " , ");
        TRAVdo (MODARRAY_MEM (arg_node), arg_info);
    }

    if (MODARRAY_IDX (arg_node) != NULL) {
        fprintf (global.outfile, " ,IDX(%s)", AVIS_NAME (MODARRAY_IDX (arg_node)));
    }

    fprintf (global.outfile, ")");

    if (MODARRAY_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, ",\n");
        /*
         * continue with other withops
         */
        PRINT_CONT (TRAVdo (MODARRAY_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTspfold( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/

node *
PRTspfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    DBUG_ASSERT (SPFOLD_FUN (arg_node) != NULL, "Missing fold function symbol");
    /**
     * udf-case prior to TC!
     */
    if (SPFOLD_GUARD (arg_node) == NULL) {
        fprintf (global.outfile, "fold/*udf-symb*/( ");
    } else {
        fprintf (global.outfile, "foldfix/*udf-symb*/( ");
    }
    if (SPFOLD_NS (arg_node) == NULL) {
        fprintf (global.outfile, "%s(", SPFOLD_FUN (arg_node));
    } else {
        fprintf (global.outfile, "%s::%s(", NSgetName (SPFOLD_NS (arg_node)),
                 SPFOLD_FUN (arg_node));
    }

    TRAVopt (SPFOLD_ARGS (arg_node), arg_info);
    fprintf (global.outfile, "), ");
    TRAVdo (SPFOLD_NEUTRAL (arg_node), arg_info);

    if (SPFOLD_GUARD (arg_node) != NULL) {
        fprintf (global.outfile, ", ");
        TRAVdo (SPFOLD_GUARD (arg_node), arg_info);
    }

    fprintf (global.outfile, ")");

    if (SPFOLD_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, ",\n");
        /*
         * continue with other withops
         */
        PRINT_CONT (TRAVdo (SPFOLD_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTfold( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/

node *
PRTfold (node *arg_node, info *arg_info)
{
    node *fundef;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    DBUG_ASSERT (FOLD_FUNDEF (arg_node) != NULL, "Missing fold function link");
    /**
     * * udf-case after TC!
     */
    fundef = FOLD_FUNDEF (arg_node);

    if (FOLD_GUARD (arg_node) == NULL) {
        fprintf (global.outfile, "fold(");
    } else {
        fprintf (global.outfile, "foldfix(");
    }

    if (FUNDEF_NS (fundef) != NULL) {
        fprintf (global.outfile, " %s::", NSgetName (FUNDEF_NS (fundef)));
    }
    fprintf (global.outfile, "%s(", FUNDEF_NAME (fundef));
    TRAVopt (FOLD_ARGS (arg_node), arg_info);
    fprintf (global.outfile, "), ");
    TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    if (FOLD_INITIAL (arg_node) != NULL) {
        fprintf (global.outfile, ", ");
        FOLD_INITIAL (arg_node) = TRAVdo (FOLD_INITIAL (arg_node), arg_info);
    }

    if (FOLD_PARTIALMEM (arg_node) != NULL) {
        fprintf (global.outfile, ", ");
        TRAVdo (FOLD_PARTIALMEM (arg_node), arg_info);
    }

    if (FOLD_GUARD (arg_node) != NULL) {
        fprintf (global.outfile, ", ");
        TRAVdo (FOLD_GUARD (arg_node), arg_info);
    }

    fprintf (global.outfile, ")");

    if (FOLD_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, ",\n");
        /*
         * continue with other withops
         */
        PRINT_CONT (TRAVdo (FOLD_NEXT (arg_node), arg_info), ;);
    }

    if (FOLD_ISPARTIALFOLD (arg_node)) {
        fprintf (global.outfile, " /* CUDA partial fold */");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTbreak( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/
node *
PRTbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    fprintf (global.outfile, "break(");

    if (BREAK_MEM (arg_node) != NULL) {
        fprintf (global.outfile, " ");
        TRAVdo (BREAK_MEM (arg_node), arg_info);
    }

    fprintf (global.outfile, ")");

    if (BREAK_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, ",\n");
        /*
         * continue with other withops
         */
        PRINT_CONT (TRAVdo (BREAK_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PRTpropagate( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *
 ******************************************************************************/
node *
PRTpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    fprintf (global.outfile, "propagate( ");

    if (PROPAGATE_DEFAULT (arg_node) != NULL) {
        TRAVdo (PROPAGATE_DEFAULT (arg_node), arg_info);
    }

    fprintf (global.outfile, ")");

    if (PROPAGATE_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, ",\n");
        /*
         * continue with other withops
         */
        PRINT_CONT (TRAVdo (PROPAGATE_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTwith2( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_with2-nodes
 *
 ******************************************************************************/

node *
PRTwith2 (node *arg_node, info *arg_info)
{
    node *code, *tmp_nwith2;
    int id;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    tmp_nwith2 = INFO_NWITH2 (arg_info);
    INFO_NWITH2 (arg_info) = arg_node;

    global.indent++;

    if (WITH2_PRAGMA (arg_node) != NULL) {
        TRAVdo (WITH2_PRAGMA (arg_node), arg_info);
        INDENT;
    }

    global.indent++;

    fprintf (global.outfile, "with2 (");
    TRAVdo (WITH2_WITHID (arg_node), arg_info);
    fprintf (global.outfile, ")\n");

    if (WITH2_PARALLELIZE (arg_node)) {
        INDENT;
        fprintf (global.outfile, "/** MT **/\n");
    }

    if (WITH2_DIST (arg_node) != NULL) {
        INDENT;
        fprintf (global.outfile, "/** WL DIST = \"%s\" **/\n", WITH2_DIST (arg_node));
    }

    if (WITH2_CUDARIZABLE (arg_node)) {
        INDENT;
        fprintf (global.outfile, "/** CUDA WL **/\n");
    }

    INDENT;
    fprintf (global.outfile, "/********** operators: **********/\n");
    code = WITH2_CODE (arg_node);
    id = 0;
    while (code != NULL) {
        INDENT;
        fprintf (global.outfile, "op_%d =\n", id);
        /*
         * store code-id before printing NWITH2_SEGS!!
         */
        CODE_ID (code) = id++;
        global.indent++;
        TRAVdo (code, arg_info);
        global.indent--;

        if (CODE_NEXT (code) != NULL) {
            fprintf (global.outfile, ",\n");
        } else {
            fprintf (global.outfile, "\n");
        }

        code = CODE_NEXT (code);
    }

    if (WITH2_SEGS (arg_node) != NULL) {
        TRAVdo (WITH2_SEGS (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, "/********** conexpr: **********/\n");
    if (WITH2_WITHOP (arg_node) != NULL) {
        TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    } else {
        INDENT;
        fprintf (global.outfile, "void ");
    }

    global.indent--;

    DBUG_EXECUTE_TAG ("PRINT_RC", if (WITH2_PRAGMA (arg_node) == NULL) {
        fprintf (global.outfile, "\n");
        INDENT;
    });

    global.indent--;

    INFO_NWITH2 (arg_info) = tmp_nwith2;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTwlseg( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_wlseg- and N_wlsegVar-nodes.
 *
 ******************************************************************************/

node *
PRTwlseg (node *arg_node, info *arg_info)
{
    node *seg;
    int id;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    seg = arg_node;
    id = 0;
    while (seg != NULL) {
        INDENT;
        fprintf (global.outfile, "/**********%s segment %d: **********",
                 (WLSEG_ISDYNAMIC (seg)) ? " (var.)" : "", id++);

        fprintf (global.outfile, "\n");
        INDENT;
        fprintf (global.outfile, " * index domain: ");
        WLSEG_IDXINF (seg) = TRAVopt (WLSEG_IDXINF (seg), arg_info);
        fprintf (global.outfile, " -> ");
        WLSEG_IDXSUP (seg) = TRAVopt (WLSEG_IDXSUP (seg), arg_info);
        fprintf (global.outfile, "\n");
        INDENT;

        if (!WLSEG_ISDYNAMIC (arg_node)) {
            fprintf (global.outfile, " * bv: ");
            WLSEG_BV (arg_node) = TRAVopt (WLSEG_BV (arg_node), arg_info);
            fprintf (global.outfile, "\n");
            INDENT;
            fprintf (global.outfile, " * ubv: ");
            WLSEG_UBV (arg_node) = TRAVopt (WLSEG_UBV (arg_node), arg_info);
            fprintf (global.outfile, "\n");
            INDENT;
            fprintf (global.outfile, " * sv: ");
            WLSEG_SV (arg_node) = TRAVopt (WLSEG_SV (arg_node), arg_info);
            fprintf (global.outfile, "\n");
            INDENT;
            fprintf (global.outfile, " * homsv: ");
            WLSEG_HOMSV (arg_node) = TRAVopt (WLSEG_HOMSV (arg_node), arg_info);
            fprintf (global.outfile, "\n");
            INDENT;
        }

        DBUG_PRINT( "printing scheduling");
        if (WLSEG_SCHEDULING (seg) != NULL) {
            if (global.compiler_subphase < PH_mt_mtas) {
                fprintf (global.outfile, " * preliminary annotated scheduling: ");
            } else {
                fprintf (global.outfile, " * scheduling: ");
            }
            SCHprintScheduling (global.outfile, WLSEG_SCHEDULING (seg));
            fprintf (global.outfile, "\n");
            INDENT;
        }

        DBUG_PRINT( "printing task slector");
        if (WLSEG_TASKSEL (seg) != NULL) {
            if (global.compiler_subphase < PH_mt_mtas) {
                fprintf (global.outfile, " * preliminary annotated taskselector: ");
            } else {
                fprintf (global.outfile, " * taskselector: ");
            }
            SCHprintTasksel (global.outfile, WLSEG_TASKSEL (seg));
            fprintf (global.outfile, "\n");
            INDENT;
        }

        fprintf (global.outfile, " */\n");

        TRAVopt (WLSEG_CONTENTS (seg), arg_info);
        PRINT_CONT (seg = WLSEG_NEXT (seg), seg = NULL)
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLxblock( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_wlblock- and N_wlublock-nodes
 *
 ******************************************************************************/

static node *
PrintWLxblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, "(");

    TRAVdo (WLXBLOCK_BOUND1 (arg_node), arg_info);
    fprintf (global.outfile, " -> ");
    TRAVdo (WLXBLOCK_BOUND2 (arg_node), arg_info);
    fprintf (global.outfile, "), ");

    fprintf (global.outfile,
             "%sblock%d[%d] %d:", (NODE_TYPE (arg_node) == N_wlblock) ? "" : "u",
             WLXBLOCK_LEVEL (arg_node), WLXBLOCK_DIM (arg_node),
             NUM_VAL (WLXBLOCK_STEP (arg_node)));

    if (WLXBLOCK_ISNOOP (arg_node)) {
        fprintf (global.outfile, " /* noop */");
    }

    fprintf (global.outfile, "\n");

    if (WLXBLOCK_CONTENTS (arg_node) != NULL) {
        global.indent++;
        TRAVdo (WLXBLOCK_CONTENTS (arg_node), arg_info);
        global.indent--;
    }

    if (WLXBLOCK_NEXTDIM (arg_node) != NULL) {
        global.indent++;
        TRAVdo (WLXBLOCK_NEXTDIM (arg_node), arg_info);
        global.indent--;
    }

    if (WLXBLOCK_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (WLXBLOCK_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

node *
PRTwlblock (node *arg_node, info *arg_info)
{
    return (PrintWLxblock (arg_node, arg_info));
}

node *
PRTwlublock (node *arg_node, info *arg_info)
{
    return (PrintWLxblock (arg_node, arg_info));
}

/******************************************************************************
 *
 * function:
 *   node *PRTwlstride( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_wlstride nodes
 *
 * remark:
 *   DYNAMIC wlstride nodes are printed as '=>'
 *
 ******************************************************************************/

node *
PRTwlstride (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, "(");
    TRAVopt (WLSTRIDE_BOUND1 (arg_node), arg_info);
    fprintf (global.outfile, " %s> ", (WLSTRIDE_ISDYNAMIC (arg_node)) ? "=" : "-");
    TRAVopt (WLSTRIDE_BOUND2 (arg_node), arg_info);
    fprintf (global.outfile, "), step%d[%d] ", WLSTRIDE_LEVEL (arg_node),
             WLSTRIDE_DIM (arg_node));
    TRAVopt (WLSTRIDE_STEP (arg_node), arg_info);

    if (WLSTRIDE_ISNOOP (arg_node)) {
        fprintf (global.outfile, ": /* noop */");
    }

    fprintf (global.outfile, "\n");

    if (WLSTRIDE_CONTENTS (arg_node) != NULL) {
        global.indent++;
        TRAVopt (WLSTRIDE_CONTENTS (arg_node), arg_info);
        global.indent--;
    }

    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVopt (WLSTRIDE_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTwlcode( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
PRTwlcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, " ");
    if (arg_node != NULL) {
        DBUG_ASSERT (NODE_TYPE (arg_node) == N_code, "illegal code node found!");

        /*
         * we use the code here, therefore the counter USED should be >0 !!
         */
        DBUG_ASSERT (CODE_USED (arg_node) > 0, "illegal CODE_USED value!");

        fprintf (global.outfile, "op_%d", CODE_ID (arg_node));
    } else {
        if (INFO_NWITH2 (arg_info) != NULL) {
            DBUG_ASSERT (NODE_TYPE (INFO_NWITH2 (arg_info)) == N_with2,
                         "INFO_NWITH2 is no N_with2 node");

            switch (WITH2_TYPE (INFO_NWITH2 (arg_info))) {
            case N_genarray:
                fprintf (global.outfile, "init");
                break;

            case N_modarray:
                fprintf (global.outfile, "copy");
                break;

            case N_fold:
                fprintf (global.outfile, "noop");
                break;

            default:
                DBUG_UNREACHABLE ("illegal with-loop type found");
                break;
            }
        } else {
            fprintf (global.outfile, "?");
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTwlgrid( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_wlgrid nodes
 *
 * remark:
 *   DYNAMIC grids are printed as '==>' (fitted) and '=>>' (unfitted).
 *   STATIC grids are printed as '-->' (fitted) and '->>' (unfitted).
 *
 ******************************************************************************/

node *
PRTwlgrid (node *arg_node, info *arg_info)
{
    const char *str = (WLGRID_ISDYNAMIC (arg_node)) ? "=" : "-";

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVopt (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, "(");
    TRAVopt (WLGRID_BOUND1 (arg_node), arg_info);
    fprintf (global.outfile, " %s%s> ", str, WLGRID_ISFITTED (arg_node) ? str : ">");
    TRAVopt (WLGRID_BOUND2 (arg_node), arg_info);
    fprintf (global.outfile, "):");

    if (WLGRID_NEXTDIM (arg_node) != NULL) {
        fprintf (global.outfile, "\n");
        global.indent++;
        TRAVopt (WLGRID_NEXTDIM (arg_node), arg_info);
        global.indent--;
    } else {
        if ((WLGRID_CODE (arg_node) != NULL) || (!WLGRID_ISNOOP (arg_node))) {
            PRTwlcode (WLGRID_CODE (arg_node), arg_info);
        }
        if (WLGRID_ISNOOP (arg_node)) {
            fprintf (global.outfile, " /* noop */");
        }
        fprintf (global.outfile, "\n");
    }

    if (WLGRID_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (WLGRID_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTssacnt( node *arg_node, info *arg_info)
 *
 * description:
 *   Prints list of SSA rename counters (for debug only).
 *
 ******************************************************************************/

node *
PRTssacnt (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, " *  ");
    PRINT_POINTER_BRACKETS (global.outfile, (void *)arg_node);
    fprintf (global.outfile, " baseid = %s, counter = %d\n", SSACNT_BASEID (arg_node),
             SSACNT_COUNT (arg_node));

    if (SSACNT_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (SSACNT_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTavis( node *arg_node, info *arg_info)
 *
 * description:
 *   Prints elements of avis node connected to vardec or arg.
 *
 ******************************************************************************/

node *
PRTavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    /* to be implemented */

    if (global.print.avis) {
        fprintf (global.outfile, " /* AVIS:");

        fprintf (global.outfile, " TYPE   = %s,",
                 TYtype2String (AVIS_TYPE (arg_node), FALSE, 0));
        fprintf (global.outfile, " SSACNT = ");
        PRINT_POINTER_BRACKETS (global.outfile, (void *)AVIS_SSACOUNT (arg_node));

        if (global.valid_ssaform && (AVIS_SSACOUNT (arg_node) != NULL)) {
            node *cnt = AVIS_SSACOUNT (arg_node);

            fprintf (global.outfile, " (baseid = %s, counter = %d)", SSACNT_BASEID (cnt),
                     SSACNT_COUNT (cnt));
        }

        fprintf (global.outfile, " SSAASSIGN = %p ", (void *)AVIS_SSAASSIGN (arg_node));

        fprintf (global.outfile, " /* DEFDEPTH = %d */ ", AVIS_DEFDEPTH (arg_node));

        fprintf (global.outfile, " */ ");
    }

    if (global.print.demand) {
        fprintf (global.outfile, " /* DEMAND = %s */ ",
                 (AVIS_DEMAND (arg_node) == NULL
                    ? "NO DEMAND"
                    : COconstant2String (AVIS_DEMAND (arg_node))));
    }

    if ((global.backend == BE_cuda || global.backend == BE_cudahybrid)
        && AVIS_ISCUDALOCAL (arg_node)) {
        fprintf (global.outfile, " /* CUDA local */");
    }

#ifdef SOONDEAD
    if (global.optimize.dopwlf) {
        fprintf (global.outfile, " /* AVIS_NPART = %d */ ", (int)AVIS_NPART (arg_node));
    }
#endif // SOONDEAD

    DBUG_RETURN (arg_node);
}

node *
PRTconstraint (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (CONSTRAINT_NEXT (arg_node) != NULL) {
        CONSTRAINT_NEXT (arg_node) = TRAVdo (CONSTRAINT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PRTwiths( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTwiths (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, " /* Hybrid With-Loop */ {\n");
    global.indent++;
    INDENT;
    fprintf (global.outfile, "wl: ");
    TRAVdo (WITHS_WITH (arg_node), arg_info);
    INDENT;
    fprintf (global.outfile, ";\n");
    if (WITHS_NEXT (arg_node) != NULL) {
        INDENT;
        fprintf (global.outfile, "next: ");
        TRAVopt (WITHS_NEXT (arg_node), arg_info);
    }
    global.indent--;
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *PRTwith3( node *arg_node, node *arg_info)
 *
 * @brief prints the 1-dimensional with3.
 *
 * @param arg_node with3 node
 * @param arg_info info structure
 *
 * @return unchanged with3 node
 ******************************************************************************/
node *
PRTwith3 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    global.indent++;
    fprintf (global.outfile, "with3 {\n");

    if (WITH3_USECONCURRENTRANGES (arg_node)) {
        INDENT;
        fprintf (global.outfile, "/* concurrent */\n");
    }

    if (WITH3_DIST (arg_node) != NULL) {
        INDENT;
        fprintf (global.outfile, "/* WL DIST = \"%s\" */\n", WITH3_DIST (arg_node));
    }

    if (WITH3_ISTOPLEVEL (arg_node)) {
        INDENT;
        fprintf (global.outfile, "/* top-level */\n");
    }

    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);

    global.indent--;
    INDENT;
    fprintf (global.outfile, "} : ( \n");
    global.indent++;
    if (WITH3_OPERATIONS (arg_node) != NULL) {
        WITH3_OPERATIONS (arg_node) = TRAVdo (WITH3_OPERATIONS (arg_node), arg_info);
    } else {
        fprintf (global.outfile, "void");
    }
    global.indent--;
    fprintf (global.outfile, ")");

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *PRTrange( node *arg_node, node *arg_info)
 *
 * @brief Prints the range component of the with3.
 *
 * @param arg_node range node
 * @param arg_info info structure
 *
 * @return unchanged range node
 ******************************************************************************/
node *
PRTrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INDENT;

    if (RANGE_ISGLOBAL (arg_node)) {
        fprintf (global.outfile, "/* global */\n");
        INDENT;
    }

    if (RANGE_ISBLOCKED (arg_node)) {
        fprintf (global.outfile, "/* blocked */\n");
        INDENT;
    }

    if (RANGE_ISFITTING (arg_node)) {
        fprintf (global.outfile, "/* fitting */\n");
        INDENT;
    }

    if (RANGE_NEEDCUDAUNROLL (arg_node)) {
        fprintf (global.outfile, "/* unroll */\n");
        INDENT;
    }

    fprintf (global.outfile, "(");
    RANGE_LOWERBOUND (arg_node) = TRAVdo (RANGE_LOWERBOUND (arg_node), arg_info);
    fprintf (global.outfile, " <= ");
    RANGE_INDEX (arg_node) = TRAVdo (RANGE_INDEX (arg_node), arg_info);
    fprintf (global.outfile, " < ");
    RANGE_UPPERBOUND (arg_node) = TRAVdo (RANGE_UPPERBOUND (arg_node), arg_info);

    if (RANGE_CHUNKSIZE (arg_node) != NULL) {
        fprintf (global.outfile, " in ");
        RANGE_CHUNKSIZE (arg_node) = TRAVdo (RANGE_CHUNKSIZE (arg_node), arg_info);
    }

    fprintf (global.outfile, " (IDXS: ");
    RANGE_IDXS (arg_node) = TRAVopt (RANGE_IDXS (arg_node), arg_info);
    if (RANGE_IIRR (arg_node) != NULL) {
        fprintf (global.outfile, ", IIRR: ");
        RANGE_IIRR (arg_node) = TRAVdo (RANGE_IIRR (arg_node), arg_info);
    }

    fprintf (global.outfile, ") ");

    fprintf (global.outfile, ") ");

    fprintf (global.outfile, "/* (BS: %d) */ \n", RANGE_BLOCKSIZE (arg_node));

    if (RANGE_BODY (arg_node) != NULL) {
        RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);
    }

    fprintf (global.outfile, " : ");
    if (RANGE_RESULTS (arg_node) != NULL) {
        RANGE_RESULTS (arg_node) = TRAVdo (RANGE_RESULTS (arg_node), arg_info);
    } else {
        fprintf (global.outfile, "void");
    }
    fprintf (global.outfile, ";\n");

    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintTRAVdo( node *syntax_tree, info *arg_info)
 *
 * description:
 *   initiates print of (sub-)tree
 *
 ******************************************************************************/

static node *
PrintTRAVdo (node *syntax_tree, info *arg_info)
{
    DBUG_ENTER ();

    TRAVpush (TR_prt);
    global.indent = 0;

    if ((global.compiler_subphase == PH_cg_prt)
        || (global.compiler_subphase == PH_ccg_prt)) {
        if (global.filetype == FT_prog) {
            /*
             * The current file is a SAC program.
             * Therefore, the C file is generated within the target directory.
             */
            global.outfile = FMGRwriteOpen ("%s/%s%s", global.targetdir,
                                            global.outfilename, global.config.cext);
            CTInote ("Writing file \"%s/%s%s\"", global.targetdir, global.outfilename,
                     global.config.cext);

            GSCprintFileHeader (syntax_tree);
            syntax_tree = TRAVdo (syntax_tree, arg_info);

            GSCprintSACargCopyFreeStubs ();

            GSCprintMain ();

            fclose (global.outfile);
        } else {
            /*
             * The current file is a module/class implementation. The functions and
             * global objects are all printed to separate files allowing for
             * separate compilation and the building of an archive. An additional
             * header file is generated for global variable and type declarations
             * as well as function prototypes.
             */
            INFO_SEPARATE (arg_info) = 1;
            TRAVdo (syntax_tree, arg_info);
            INFO_SEPARATE (arg_info) = 0;
        }
    } else {
        fprintf (global.outfile,
                 "\n/*-----------------------------------------------*/\n");
        TRAVdo (syntax_tree, arg_info);
        fprintf (global.outfile,
                 "\n/*-----------------------------------------------*/\n");
    }

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *PrintTravPre( node *arg_node, info *arg_info)
 *
 * description:
 *   This function is called before the traversal of each node.
 *
 ******************************************************************************/

node *
PRTtravPre (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("PRINT_LINE", "line (%s) %s:%zu\n", NODE_TEXT (arg_node),
                    NODE_FILE (arg_node), NODE_LINE (arg_node));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintTravPost( node *arg_node, info *arg_info)
 *
 * description:
 *   This function is called after the traversal of each node.
 *
 ******************************************************************************/

node *
PRTtravPost (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTdoPrint( node *syntax_tree)
 *
 * description:
 *   Prints the whole (sub-) tree behind the given node.
 *
 ******************************************************************************/

node *
PRTdoPrint (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    /* we want to duplicate all sons */
    INFO_CONT (arg_info) = NULL;

    if (global.outfile == NULL) {
        global.outfile = stdout;
        syntax_tree = PrintTRAVdo (syntax_tree, arg_info);
        global.outfile = NULL;
    } else {
        syntax_tree = PrintTRAVdo (syntax_tree, arg_info);
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *PRTdoPrintNode( node *syntax_tree)
 *
 * description:
 *   Prints the given node without next node.
 *
 ******************************************************************************/

node *
PRTdoPrintNode (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    /* we want to duplicate all sons */
    INFO_CONT (arg_info) = syntax_tree;

    if (global.outfile == NULL) {
        global.outfile = stdout;
        syntax_tree = PrintTRAVdo (syntax_tree, arg_info);
        global.outfile = NULL;
    } else {
        syntax_tree = PrintTRAVdo (syntax_tree, arg_info);
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *PRTdoPrintNodeFile(FILE *fd, node *arg_node)
 *
 *
 * description:
 *   Prints the given node to fd, typically stderr
 *
 ******************************************************************************/

node *
PRTdoPrintNodeFile (FILE *fd, node *arg_node)
{
    FILE *globalfd;

    DBUG_ENTER ();
    globalfd = global.outfile;
    global.outfile = fd;
    PRTdoPrintNode (arg_node);
    global.outfile = globalfd;
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTdoPrintFile(FILE *fd, node *arg_node)
 *
 *
 * description:
 *   Prints the given arg_node to fd, typically stderr
 *
 ******************************************************************************/

node *
PRTdoPrintFile (FILE *fd, node *arg_node)
{
    FILE *globalfd;

    DBUG_ENTER ();
    globalfd = global.outfile;
    global.outfile = fd;
    PRTdoPrint (arg_node);
    global.outfile = globalfd;
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *PrintDataflowgraph(node *arg_node, info *arg_info)
 *
 * @brief Prints the dataflowgraph and its dataflownodes
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/
node *
PRTdataflowgraph (node *arg_node, info *arg_info)
{
    nodelist *member_iterator;
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    global.outfile = stdout;

    fprintf (global.outfile, "****** Dataflowgraph begin ******\n");

    if (arg_node != NULL) {
        DBUG_ASSERT (NODE_TYPE (arg_node) == N_dataflowgraph,
                     "PrintDataflowgraph expects a N_dataflowgraph");

        member_iterator = DATAFLOWGRAPH_MEMBERS (arg_node);
        while (member_iterator != NULL) {
            PRTdataflownode (NODELIST_NODE (member_iterator), arg_info);
            member_iterator = NODELIST_NEXT (member_iterator);
        }
    }

    fprintf (global.outfile, "****** Dataflowgraph end ******\n\n");
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************->>
 *
 * @fn node *PRTdataflownode(node *arg_node, info *arg_info)
 *
 * @brief Prints the dataflownode
 *
 * @param arg_node
 * @param arg_info
 * @return
 *
 *****************************************************************************/

node *
PRTdataflownode (node *arg_node, info *arg_info)
{
    nodelist *dependent_iterator;
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    global.outfile = stdout;

    if (arg_node != NULL) {
        DBUG_ASSERT (NODE_TYPE (arg_node) == N_dataflownode,
                     "PrintDataflownode expects a N_dataflownode");

        fprintf (global.outfile, "%s: %s, REFCOUNT: %i", DATAFLOWNODE_NAME (arg_node),
                 MUTHLIBdecodeExecmode (DATAFLOWNODE_EXECMODE (arg_node)),
                 DATAFLOWNODE_REFCOUNT (arg_node));
        if (global.break_after_phase == PH_mt) {
            fprintf (global.outfile, ", REFLEFT: %i, USED: %i\n",
                     DATAFLOWNODE_REFLEFT (arg_node), DATAFLOWNODE_ISUSED (arg_node));
        } else {
            fprintf (global.outfile, "\n");
        }

        dependent_iterator = DATAFLOWNODE_DEPENDENT (arg_node);

        if (dependent_iterator != NULL) {
            fprintf (global.outfile, "   ->");

            while (dependent_iterator != NULL) {
                fprintf (global.outfile, " %s,",
                         DATAFLOWNODE_NAME (NODELIST_NODE (dependent_iterator)));
                dependent_iterator = NODELIST_NEXT (dependent_iterator);
            }
            fprintf (global.outfile, "\n");
        } else {
            fprintf (global.outfile, "  -> No dependent nodes\n");
        }
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *prterror( node *arg_node, info *arg_info)
 *
 * description:
 *   this function print all the errors
 *
 ******************************************************************************/

node *
PRTerror (node *arg_node, info *arg_info)
{
    bool firstError;

    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    firstError = INFO_FIRSTERROR (arg_info);

    if ((global.outfile != NULL)
        && (ERROR_ANYPHASE (arg_node) == global.compiler_anyphase)) {

        if (firstError) {
            fprintf (global.outfile, "\n/******* BEGIN TREE CORRUPTION ********\n");
            INFO_FIRSTERROR (arg_info) = FALSE;
        }

        fprintf (global.outfile, "%s\n", ERROR_MESSAGE (arg_node));

        if (ERROR_NEXT (arg_node) != NULL) {
            PRINT_CONT (TRAVdo (ERROR_NEXT (arg_node), arg_info), ;);
        }

        if (firstError) {
            fprintf (global.outfile, "********  END TREE CORRUPTION  *******/\n");
            INFO_FIRSTERROR (arg_info) = TRUE;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *PRTimport (node * arg_node, info * arg_info)
 *
 * @brief prints an import node
 *
 * @param arg_node import node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
PRTimport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "import %s : ", IMPORT_MOD (arg_node));

    if (IMPORT_ALL (arg_node)) {
        fprintf (global.outfile, "all");
    } else {
        fprintf (global.outfile, "{ ");
        IMPORT_SYMBOL (arg_node) = TRAVdo (IMPORT_SYMBOL (arg_node), arg_info);
        fprintf (global.outfile, "}");
    }

    fprintf (global.outfile, ";\n");

    if (IMPORT_NEXT (arg_node) != NULL) {
        IMPORT_NEXT (arg_node) = TRAVdo (IMPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *PRTexport (node * arg_node, info * arg_info)
 *
 * @brief prints an export node
 *
 * @param arg_node export node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
PRTexport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "export ");

    if (EXPORT_ALL (arg_node)) {
        fprintf (global.outfile, "all");
        if (EXPORT_SYMBOL (arg_node) != NULL) {
            fprintf (global.outfile, " except ");
        }
    }

    if (EXPORT_SYMBOL (arg_node) != NULL) {
        fprintf (global.outfile, "{ ");
        EXPORT_SYMBOL (arg_node) = TRAVdo (EXPORT_SYMBOL (arg_node), arg_info);
        fprintf (global.outfile, "}");
    }

    fprintf (global.outfile, ";\n");

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = TRAVdo (EXPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *PRTuse (node * arg_node, info * arg_info)
 *
 * @brief prints an use node
 *
 * @param arg_node use node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
PRTuse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "use %s : ", USE_MOD (arg_node));

    if (USE_ALL (arg_node)) {
        fprintf (global.outfile, "all");
        if (USE_SYMBOL (arg_node) != NULL) {
            fprintf (global.outfile, " except ");
        }
    }

    if (USE_SYMBOL (arg_node) != NULL) {
        fprintf (global.outfile, "{ ");
        USE_SYMBOL (arg_node) = TRAVdo (USE_SYMBOL (arg_node), arg_info);
        fprintf (global.outfile, "}");
    }

    fprintf (global.outfile, ";\n");

    if (USE_NEXT (arg_node) != NULL) {
        USE_NEXT (arg_node) = TRAVdo (USE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *PRTprovide (node * arg_node, info * arg_info)
 *
 * @brief prints an provide node
 *
 * @param arg_node provide node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
PRTprovide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "provide ");

    if (PROVIDE_ALL (arg_node)) {
        fprintf (global.outfile, "all");

        if (PROVIDE_SYMBOL (arg_node) != NULL) {
            fprintf (global.outfile, " except ");
        }
    }

    if (PROVIDE_SYMBOL (arg_node) != NULL) {
        fprintf (global.outfile, "{ ");
        PROVIDE_SYMBOL (arg_node) = TRAVdo (PROVIDE_SYMBOL (arg_node), arg_info);
        fprintf (global.outfile, "}");
    }

    fprintf (global.outfile, ";\n");

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = TRAVdo (PROVIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *PRTsymbol (node * arg_node, info * arg_info)
 *
 * @brief prints an symbol node
 *
 * @param arg_node symbol node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
PRTsymbol (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (SYMBOL_NEXT (arg_node) != NULL) {
        SYMBOL_NEXT (arg_node) = TRAVdo (SYMBOL_NEXT (arg_node), arg_info);
        fprintf (global.outfile, ", ");
    }

    fprintf (global.outfile, "%s", SYMBOL_ID (arg_node));

    DBUG_RETURN (arg_node);
}

node *
PRTset (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "%s\n", CTIitemName (SET_MEMBER (arg_node)));

    if (SET_NEXT (arg_node) != NULL) {
        SET_NEXT (arg_node) = TRAVdo (SET_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *PRTfunbundle (node *arg_node, info *arg_info)
 *
 * @brief prints a function bundle
 *
 * @param arg_node funbundle node
 * @param arg_info info structure
 *
 * @return unmodified node
 ******************************************************************************/
node *
PRTfunbundle (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * we only print functions bundled in the prototype section, as most
     * of them won't contain functions with bodies anyways. Otherwise we
     * would print loads of empty bundles and thereby clutter up the output.
     */
    if (INFO_PROTOTYPE (arg_info)) {
        fprintf (global.outfile,
                 "\n\n "
                 "/**********************************************************************"
                 "*******\n"
                 " * Function Bundle %s::%s (%zu)\n"
                 " **********************************************************************"
                 "*******/\n\n",
                 NSgetName (FUNBUNDLE_NS (arg_node)), FUNBUNDLE_NAME (arg_node),
                 FUNBUNDLE_ARITY (arg_node));
    }

    if (FUNBUNDLE_FUNDEF (arg_node) != NULL) {
        TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);
    }

    if (INFO_PROTOTYPE (arg_info)) {
        fprintf (global.outfile, "\n\n "
                                 "/******************************************************"
                                 "**********************/\n\n");
    }

    if (FUNBUNDLE_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (FUNBUNDLE_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

node *
PRTtfdag (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();
    node *defs;
    int i = 0;

    /*
     * Print the component subtyping graphs
     */

    defs = TFDAG_DEFS (arg_node);

    fprintf (global.outfile, "\n/*\nType family specifications\n");
    fprintf (global.outfile, "The following output is in dot format.\n");
    fprintf (global.outfile, "It can be visualized using graphviz's dot tool.\n");
    fprintf (global.outfile, "\ndigraph typespecs{\n");

    while (defs != NULL) {
        if (TFVERTEX_PARENTS (defs) == NULL) {

            fprintf (global.outfile, "subgraph cluster_%d{\n", i);
            fprintf (global.outfile, "node [shape=record];\n");

            if (TFDAG_INFO (arg_node) != NULL) {

                if (COMPINFO_TLC (TFDAG_INFO (arg_node)) != NULL) {
                    printMatrixInDotFormat (COMPINFO_TLC (TFDAG_INFO (arg_node)));
                }
                if (COMPINFO_DIST (TFDAG_INFO (arg_node)) != NULL) {
                    printMatrixInDotFormat (COMPINFO_DIST (TFDAG_INFO (arg_node)));
                }
            }

            fprintf (global.outfile, "node [shape=box];\n");

            TRAVdo (defs, arg_info);

            fprintf (global.outfile, "};\n");
        }

        defs = TFVERTEX_NEXT (defs);
    }

    fprintf (global.outfile, "};\n");
    fprintf (global.outfile, "*/\n");
    DBUG_RETURN (arg_node);
}

node *
PRTtfspec (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();
    DBUG_RETURN (arg_node);
}

node *
PRTtfvertex (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    node *children;

    /*
     * By default, the first time we output the node info in dot format
     */

    if (arg_node != NULL) {

        fprintf (global.outfile, "pre=[%d,", TFVERTEX_PRE (arg_node));

        fprintf (global.outfile, "%d)\\n", TFVERTEX_PREMAX (arg_node));

        fprintf (global.outfile, "post=%d\\n", TFVERTEX_POST (arg_node));

        fprintf (global.outfile, "topo=%d\\n", TFVERTEX_TOPO (arg_node));

        fprintf (global.outfile, "depth=%d\\n", TFVERTEX_DEPTH (arg_node));

        int reachcola, reachcolb, row;

        reachcola = TFVERTEX_REACHCOLA (arg_node);
        reachcolb = TFVERTEX_REACHCOLB (arg_node);
        row = TFVERTEX_ROW (arg_node);

        if (!TFVERTEX_ISRCHCOLAMARKED (arg_node)) {
            fprintf (global.outfile, "rch=[-,");
        } else {
            fprintf (global.outfile, "rch=[%d,", reachcola);
        }

        if (!TFVERTEX_ISRCHCOLBMARKED (arg_node)) {
            fprintf (global.outfile, "-,");
        } else {
            fprintf (global.outfile, "%d,", reachcolb);
        }

        if (!TFVERTEX_ISROWMARKED (arg_node)) {
            fprintf (global.outfile, "-]");
        } else {
            fprintf (global.outfile, "%d]", row);
        }

        fprintf (global.outfile, "\"];\n");
    }

    children = TFVERTEX_CHILDREN (arg_node);

    while (children != NULL) {

        if (TFEDGE_EDGETYPE (children) == edgetree) {
            TRAVdo (TFEDGE_TARGET (children), arg_info);
        }

        children = TFEDGE_NEXT (children);
    }

    /*
     * After the node information has been output, we can output the
     * edge information. This conforms to the dot language rules.
     */

    INFO_TFSUPERNODE (arg_info) = arg_node;

    if (TFVERTEX_CHILDREN (arg_node) != NULL) {
        TRAVdo (TFVERTEX_CHILDREN (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PRTtfrel (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    fprintf (global.outfile, "\"%s\"->\"%s\";\n", TFREL_SUPERTAG (arg_node),
             TFREL_SUBTAG (arg_node));

    if (TFREL_NEXT (arg_node) != NULL) {
        TRAVdo (TFREL_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PRTtfabs (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    if (INFO_DOTMODE (arg_info) == vertices) {
        // fprintf(global.outfile,"%s\n",TFABS_TAG(arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
PRTtfusr (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    if (INFO_DOTMODE (arg_info) == vertices) {
        // fprintf(global.outfile,"%s\n",TFUSR_TAG(arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
PRTtfbin (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    if (INFO_DOTMODE (arg_info) == vertices) {
        // fprintf(global.outfile,"%s\n",TFBIN_TAG(arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
PRTtfedge (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    if (TFEDGE_EDGETYPE (arg_node) == edgecross) {
        fprintf (global.outfile, "<%d/>-><%d/> [style=dotted]",
                 TFVERTEX_PRE (INFO_TFSUPERNODE (arg_info)),
                 TFVERTEX_PRE (TFEDGE_TARGET (arg_node)));
    } else {
        fprintf (global.outfile, "<%d/>-><%d/>",
                 TFVERTEX_PRE (INFO_TFSUPERNODE (arg_info)),
                 TFVERTEX_PRE (TFEDGE_TARGET (arg_node)));
    }

    fprintf (global.outfile, ";\n");

    if (TFEDGE_NEXT (arg_node) != NULL) {
        TRAVdo (TFEDGE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PRTtypecomponentarg (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();
    /* do nothing  */
    DBUG_RETURN (arg_node);
}

node *
PRTtfexpr (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    INFO_TFSTRINGEXPR (arg_info) = STRcat (INFO_TFSTRINGEXPR (arg_info), "(");

    /*
     * The following 5 cases are possible here:
     * Case 1: id/num op id/num
     * Case 2: id/num op expr
     * Case 3: expr op id/num
     * Case 4: expr op expr
     * Case 5: id/num
     */

    if (TFEXPR_OPERAND1 (arg_node) != NULL) {
        TRAVdo (TFEXPR_OPERAND1 (arg_node), arg_info);
        INFO_TFSTRINGEXPR (arg_info)
          = STRcat (INFO_TFSTRINGEXPR (arg_info), TFEXPR_OPERATOR (arg_node));
    } else {

        if (TFEXPR_ASSIGNEEID (arg_node) != NULL) {
            INFO_TFSTRINGEXPR (arg_info)
              = STRcat (INFO_TFSTRINGEXPR (arg_info), TFEXPR_ASSIGNEEID (arg_node));
        }

        INFO_TFSTRINGEXPR (arg_info)
          = STRcat (INFO_TFSTRINGEXPR (arg_info),
                    STRcat ("[val=", STRcat (STRitoa (TFEXPR_VALUE (arg_node)), "]")));
    }

    if (TFEXPR_OPERAND2 (arg_node) != NULL) {
        TRAVdo (TFEXPR_OPERAND2 (arg_node), arg_info);
    } else {
        /*
         * This is possible if arg_node is a num/id. In that case, we need not print
         * the assignee_id and value again as it has already been printed
         * while inspecting Operand1
         */
    }

    INFO_TFSTRINGEXPR (arg_info) = STRcat (INFO_TFSTRINGEXPR (arg_info), ")");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTlivevars( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_livevars node.
 *
 ******************************************************************************/
node *
PRTlivevars (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LIVEVARS_NEXT (arg_node) = TRAVopt (LIVEVARS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
