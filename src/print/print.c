/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "type_utils.h"
#include "DupTree.h"
#include "dbug.h"
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
#include "print_interface.h"
#include "internal_lib.h"
#include "namespaces.h"
#include "shape.h"

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
struct INFO {
    /* print */
    node *cont;
    node *fundef;
    node *npart;
    node *nwith2;
    int sib;
    int ofp;
    int prototype;
    int separate;
    int dim;
    shpseg *shape;
    shpseg *shapecnt;
    /* writesib */
    nodelist *etypes;
    nodelist *efuns;
    nodelist *eobjs;
    bool firstError;
};

/* access macros print */
#define INFO_CONT(n) (n->cont)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_NPART(n) (n->npart)
#define INFO_NWITH2(n) (n->nwith2)
#define INFO_OMIT_FORMAL_PARAMS(n) (n->ofp)
#define INFO_PROTOTYPE(n) (n->prototype)
#define INFO_SEPARATE(n) (n->separate)
#define INFO_DIM(n) (n->dim)
#define INFO_SHAPE(n) (n->shape)
#define INFO_SHAPE_COUNTER(n) (n->shapecnt)
#define INFO_FIRSTERROR(n) (n->firstError)

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
    fprintf (file, "%s", STR_OR_UNKNOWN (str));                                          \
    PRINT_POINTER_BRACKETS (file, str);

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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    /* initialise own fields. remember to update dependent phases
     * as well!
     */
    INFO_CONT (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_NPART (result) = NULL;
    INFO_NWITH2 (result) = NULL;
    INFO_OMIT_FORMAL_PARAMS (result) = 0;
    INFO_PROTOTYPE (result) = 0;
    INFO_SEPARATE (result) = 0;
    INFO_DIM (result) = 0;
    INFO_SHAPE (result) = NULL;
    INFO_SHAPE_COUNTER (result) = NULL;
    INFO_FIRSTERROR (result) = TRUE;

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
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

static void
PrintSimdBegin ()
{
    DBUG_ENTER ("PrintSimdBegin");

    INDENT;
    fprintf (global.outfile, "( /* SIMD-Block begin */\n");
    global.indent++;

    DBUG_VOID_RETURN;
}

static void
PrintSimdEnd ()
{
    DBUG_ENTER ("PrintSimdEnd");

    fprintf (global.outfile, "\n");
    global.indent--;
    INDENT;
    fprintf (global.outfile, ") /* SIMD-Block end */\n");

    DBUG_VOID_RETURN;
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
    shpseg *offset;

    DBUG_ENTER ("WLAAprintAccesses");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_code), "Wrong node-type: N_code exspected");

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
                                     SHPSEG_SHAPE (offset, 0));
                        } else {
                            fprintf (global.outfile, "write( %s + [ %d",
                                     VARDEC_NAME (ACCESS_IV (access)),
                                     SHPSEG_SHAPE (offset, 0));
                        }
                        for (i = 1; i < dim; i++)
                            fprintf (global.outfile, ",%d", SHPSEG_SHAPE (offset, i));
                        fprintf (global.outfile, " ], %s)\n",
                                 STR_OR_UNKNOWN (VARDEC_NAME (ACCESS_ARRAY (access))));
                        offset = SHPSEG_NEXT (offset);
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
                                     SHPSEG_SHAPE (offset, 0));
                        } else {
                            fprintf (global.outfile, "write( [ %d",
                                     SHPSEG_SHAPE (offset, 0));
                        }
                        for (i = 1; i < dim; i++) {
                            fprintf (global.outfile, ",%d", SHPSEG_SHAPE (offset, i));
                        }
                        fprintf (global.outfile, " ], %s)\n",
                                 STR_OR_UNKNOWN (VARDEC_NAME (ACCESS_ARRAY (access))));
                        offset = SHPSEG_NEXT (offset);
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

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("TSIprintInfo");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_code), "Wrong node-type: N_code exspected");

    count = 0;
    iter = 0;
    dim = CODE_WLAA_ARRAYDIM (arg_node);
    aelems = NULL;
    if (CODE_TSI_TILESHP (arg_node) == NULL) {
        pragma = NULL;
    } else {
        pragma = MakePragma ();
        for (i = dim - 1; i >= 0; i--) {
            tilesize = SHPSEG_SHAPE (CODE_TSI_TILESHP (arg_node), i);
            aelems = TBmakeExprs (MakeNum (tilesize), aelems);
        }
        ap_name = ILIBmalloc (6 * sizeof (char));
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

    DBUG_VOID_RETURN;
}
#endif /* ! TSI_DEACTIVATED */

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
    int i;

    DBUG_ENTER ("PrintArgtab");

    if (argtab != NULL) {
        fprintf (global.outfile, "[");
        for (i = 0; i < argtab->size; i++) {
            if (argtab->tag[i] != ATG_notag) {
                fprintf (global.outfile, " %s:", global.argtag_string[argtab->tag[i]]);

                if (argtab->ptr_in[i] != NULL) {
                    PRINT_POINTER_BRACKETS (global.outfile, argtab->ptr_in[i]);
                    if (is_def) {
                        DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_arg),
                                     "illegal argtab entry found!");

                        if (ARG_NAME (argtab->ptr_in[i]) != NULL) {
                            fprintf (global.outfile, "%s", ARG_NAME (argtab->ptr_in[i]));
                        }
                    } else {
                        DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_exprs),
                                     "illegal argtab entry found!");

                        fprintf (global.outfile, "%s",
                                 NODE_TEXT (EXPRS_EXPR (argtab->ptr_in[i])));
                    }
                } else {
                    fprintf (global.outfile, "-");
                }

                fprintf (global.outfile, "/");

                if (argtab->ptr_out[i] != NULL) {
                    PRINT_POINTER_BRACKETS (global.outfile, argtab->ptr_out[i]);
                    if (is_def) {
                    } else {
                        fprintf (global.outfile, "%s",
                                 STR_OR_EMPTY (IDS_NAME (((node *)argtab->ptr_out[i]))));
                    }
                } else {
                    fprintf (global.outfile, "-");
                }
            } else {
                DBUG_ASSERT ((argtab->ptr_in[i] == NULL), "illegal argtab entry found!");
                DBUG_ASSERT ((argtab->ptr_out[i] == NULL), "illegal argtab entry found!");

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

    DBUG_VOID_RETURN;
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
    int i;
    node *rets = NULL;
    node *args = NULL;

    DBUG_ENTER ("Argtab2Fundef");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent");

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
      = TBmakeFundef (ILIBstringCopy (FUNDEF_NAME (fundef)),
                      NSdupNamespace (FUNDEF_NS (fundef)), rets, args, NULL, NULL);

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
    int i;
    node *ids = NULL;
    node *exprs = NULL;
    node *expr = NULL;

    DBUG_ENTER ("Argtab2Let");

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent");

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
    int i;

    DBUG_ENTER ("PrintArgtags");

    if (!in_comment) {
        fprintf (global.outfile, " /*");
    }

    /* return value */
    if (argtab->tag[0] != ATG_notag) {
        DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent");
        fprintf (global.outfile, " %s", global.argtag_string[argtab->tag[0]]);
    }

    fprintf (global.outfile, " <-");

    /* arguments */
    for (i = 1; i < argtab->size; i++) {
        DBUG_ASSERT ((argtab->tag[i] != ATG_notag), "argtab is uncompressed");
        fprintf (global.outfile, " %s", global.argtag_string[argtab->tag[i]]);
    }

    if (!in_comment) {
        fprintf (global.outfile, " */ ");
    }

    DBUG_VOID_RETURN;
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
    DBUG_ENTER ("PRTids");

    if (arg_node != NULL) {

        if (NODE_ERROR (arg_node) != NULL) {
            NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
        }

        fprintf (global.outfile, "%s", IDS_NAME (arg_node));

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
    DBUG_ENTER ("PRTids");

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

node *
PRTmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTmodule");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, NODE_TEXT (arg_node), arg_node));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (INFO_SEPARATE (arg_info)) {
        /*
         * In this case, we print a module or class implementation and we want
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
        global.outfile = FMGRwriteOpen ("%s/header.h", global.tmp_dirname);
        GSCprintFileHeader (arg_node);

        if (NULL != MODULE_TYPES (arg_node)) {
            fprintf (global.outfile, "\n\n");
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

        global.outfile = FMGRwriteOpen ("%s/globals.c", global.tmp_dirname);
        fprintf (global.outfile, "#include \"header.h\"\n\n");
        fprintf (global.outfile,
                 "int SAC__%s__dummy_value_which_is_completely_useless"
                 " = 0;\n\n",
                 NSgetName (MODULE_NAMESPACE (arg_node)));

        if (NULL != MODULE_OBJS (arg_node)) {
            fprintf (global.outfile, "\n\n");
            global.print_objdef_for_header_file = FALSE;
            /* print object definitions */
            TRAVdo (MODULE_OBJS (arg_node), arg_info);
        }

        fclose (global.outfile);

        if (NULL != MODULE_FUNS (arg_node)) {
            TRAVdo (MODULE_FUNS (arg_node), arg_info); /* print function definitions */
                                                       /*
                                                        * Note that in this case a separate file is created for each function.
                                                        * These files are opened and closed in PRTfundef().
                                                        */
        }

        /*
         * finally create a dummy funfile, so we can rely on one to exist
         * when using wildcards like fun*.c
         */

        global.outfile = FMGRwriteOpen ("%s/fundummy.c", global.tmp_dirname);
        fprintf (global.outfile, "#include \"header.h\"\n\n");
        fprintf (global.outfile,
                 "int SAC__%s__another_dummy_value_which_is_completely_useless"
                 " = 0;\n\n",
                 NSgetName (MODULE_NAMESPACE (arg_node)));
        fclose (global.outfile);

    } else {
        switch (global.filetype) {
        case F_modimp:
            fprintf (global.outfile,
                     "\n"
                     "/*\n"
                     " *  Module %s :\n"
                     " */\n",
                     NSgetName (MODULE_NAMESPACE (arg_node)));
            break;
        case F_classimp:
            fprintf (global.outfile,
                     "\n"
                     "/*\n"
                     " *  Class %s :\n",
                     NSgetName (MODULE_NAMESPACE (arg_node)));
            fprintf (global.outfile, " */\n");
            break;
        case F_prog:
            fprintf (global.outfile,
                     "\n"
                     "/*\n"
                     " *  SAC-Program %s :\n"
                     " */\n",
                     global.puresacfilename);
            break;
        default:
            break;
        }

        if (MODULE_IMPORTS (arg_node) != NULL) {
            fprintf (global.outfile, "\n");
            /* print import-list */
            TRAVdo (MODULE_IMPORTS (arg_node), arg_info);
        }

        if (MODULE_TYPES (arg_node) != NULL) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  type definitions\n"
                                     " */\n\n");
            /* print typedefs */
            TRAVdo (MODULE_TYPES (arg_node), arg_info);
        }

        GSCprintDefines ();

        if (MODULE_FUNDECS (arg_node) != NULL) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  prototypes for externals (FUNDECS)\n"
                                     " */\n\n");
            INFO_PROTOTYPE (arg_info) = TRUE;
            /* print function declarations */
            TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
            INFO_PROTOTYPE (arg_info) = FALSE;
        }

        if (MODULE_FUNS (arg_node) != NULL) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  prototypes for locals (FUNDEFS)\n"
                                     " */\n\n");
            INFO_PROTOTYPE (arg_info) = TRUE;
            /* print function declarations */
            TRAVdo (MODULE_FUNS (arg_node), arg_info);
            INFO_PROTOTYPE (arg_info) = FALSE;
        }

        if (MODULE_OBJS (arg_node) != NULL) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  global objects\n"
                                     " */\n\n");
            /* print objdefs */
            TRAVdo (MODULE_OBJS (arg_node), arg_info);
        }

        if (MODULE_SPMDSTORE (arg_node) != NULL) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  SPMD infrastructure\n"
                                     " */\n\n");
            TRAVdo (MODULE_SPMDSTORE (arg_node), arg_info);
        }

        if (MODULE_FUNS (arg_node) != NULL) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  function definitions (FUNDEFS)\n"
                                     " */\n\n");
            /* print function definitions */
            TRAVdo (MODULE_FUNS (arg_node), arg_info);
        }

        DBUG_EXECUTE ("PRINT_CWRAPPER", if (MODULE_CWRAPPER (arg_node) != NULL) {
            fprintf (global.outfile, "\n\n"
                                     "/*\n"
                                     " *  c wrapper functions\n"
                                     " */\n\n");
            /* print wrapper mappings */
            TRAVdo (MODULE_CWRAPPER (arg_node), arg_info);
        });
    }

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

    DBUG_ENTER ("PRTtypedef");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, NODE_TEXT (arg_node), arg_node));

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
        } else {
            fprintf (global.outfile, "typedef ");
        }

        if (!ishidden) {
            type_str = TYtype2String (TYPEDEF_NTYPE (arg_node), 0, TRUE);
            fprintf (global.outfile, "%s ", type_str);
            type_str = ILIBfree (type_str);
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

    if (TYPEDEF_COPYFUN (arg_node) != NULL) {
        fprintf (global.outfile, "\nextern %s %s( %s);\n", TYPEDEF_NAME (arg_node),
                 TYPEDEF_COPYFUN (arg_node), TYPEDEF_NAME (arg_node));
    }
    if (TYPEDEF_FREEFUN (arg_node) != NULL) {
        fprintf (global.outfile, "extern void %s( %s);\n\n", TYPEDEF_FREEFUN (arg_node),
                 TYPEDEF_NAME (arg_node));
    }

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

    DBUG_ENTER ("PRTobjdef");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, NODE_TEXT (arg_node), arg_node));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    /* print objinit flag declaration in header.file
     * this has to placed before the if switch, because the ICM_Trav seems
     * to leave with no return...
     */
    if (global.genlib.c && global.print_objdef_for_header_file) {
        fprintf (global.outfile,
                 "/* flag, if object has been initialized */\n"
                 "extern bool SAC_INIT_FLAG_%s;\n",
                 OBJDEF_NAME (arg_node));
    }

    if ((OBJDEF_ICM (arg_node) == NULL) || (NODE_TYPE (OBJDEF_ICM (arg_node)) != N_icm)) {
        if (!OBJDEF_ISLOCAL (arg_node) || global.print_objdef_for_header_file) {
            fprintf (global.outfile, "extern ");
        }

        type_str = TYtype2String (OBJDEF_TYPE (arg_node), FALSE, 0);
        fprintf (global.outfile, "%s ", type_str);
        type_str = ILIBfree (type_str);

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

    DBUG_ENTER ("PRTret");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, NODE_TEXT (arg_node), arg_node));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (RET_TYPE (arg_node) != NULL) {
        type_str = TYtype2String (RET_TYPE (arg_node), FALSE, 0);
        fprintf (global.outfile, "%s", type_str);
        type_str = ILIBfree (type_str);

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

static void
PrintFunctionHeader (node *arg_node, info *arg_info, bool in_comment)
{
    types *ret_types;
    char *type_str;
    bool print_sac = TRUE;
    bool print_c = FALSE;
    bool print_argtab = FALSE;

    DBUG_ENTER ("PrintFunctionHeader");

    DBUG_EXECUTE ("PRINT_PTR", fprintf (global.outfile, "/* " F_PTR " */\n", arg_node););

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (FUNDEF_ARGTAB (arg_node) != NULL) {
        print_sac = FALSE;
        print_c = TRUE;

        DBUG_EXECUTE ("PRINT_ARGTAB", fprintf (global.outfile, "/* \n"); INDENT;
                      fprintf (global.outfile, " *  ");
                      PRTprintArgtab (FUNDEF_ARGTAB (arg_node), TRUE);
                      fprintf (global.outfile, "  */\n"); INDENT; print_sac = TRUE;
                      print_argtab = TRUE;);
    }

    if (FUNDEF_ISINLINE (arg_node)) {
        fprintf (global.outfile, "inline\n");
    }

    if (FUNDEF_ISLACINLINE (arg_node)) {
        fprintf (global.outfile, "/* lacinline */\n");
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
            if (RET_TYPE (FUNDEF_RETS (arg_node)) != NULL) {
                /*
                 * We do have new types !
                 */
                TRAVdo (FUNDEF_RETS (arg_node), arg_info);
            } else {
                /*
                 *  Print old types.
                 */
                ret_types = FUNDEF_TYPES (arg_node);
                while (ret_types != NULL) {
                    type_str = CVtype2String (ret_types, 0, FALSE);
                    fprintf (global.outfile, "%s", type_str);
                    type_str = ILIBfree (type_str);

                    ret_types = TYPES_NEXT (ret_types);
                    if (ret_types != NULL) {
                        fprintf (global.outfile, ", ");
                    }
                }
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

        if (FUNDEF_HASDOTARGS (arg_node) || FUNDEF_HASDOTRETS (arg_node)) {
            fprintf (global.outfile, ", ...");
        }

        fprintf (global.outfile, ")");

        if (print_c) {
            fprintf (global.outfile, "\n");
            INDENT;
            fprintf (global.outfile, " */ ");
        }
    }

    /* Now, we print the new type signature, iff present */
    fprintf (global.outfile, "\n");
    INDENT;
    fprintf (global.outfile, (in_comment) ? " *\n" : "/*\n");
    fprintf (global.outfile, " *  ");
    if (FUNDEF_NAME (arg_node) != NULL) {
        fprintf (global.outfile, "%s :: ", FUNDEF_NAME (arg_node));
        if (FUNDEF_WRAPPERTYPE (arg_node) != NULL) {
            char *(*t2s_fun) (ntype *, bool, int);
            t2s_fun = TYtype2String;
            DBUG_EXECUTE ("PRINT_NTY", t2s_fun = TYtype2DebugString;);

            fprintf (global.outfile, "%s\n",
                     t2s_fun (FUNDEF_WRAPPERTYPE (arg_node), TRUE,
                              global.indent + strlen (FUNDEF_NAME (arg_node)) + 8));
        } else {
            fprintf (global.outfile, " ---\n");
        }
    }
    INDENT;
    fprintf (global.outfile, (in_comment) ? " *" : " */");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PRTprintHomsv( FILE *handle, int *vect, int dims)
 *
 * Description:
 *
 *
 ******************************************************************************/

extern void
PRTprintHomsv (FILE *handle, int *vect, int dims)
{
    int d;

    DBUG_ENTER ("PRTprintHomsv");

    if ((vect) != NULL) {
        fprintf (handle, "[ ");
        for (d = 0; d < (dims); d++) {
            if ((vect)[d] > 0) {
                fprintf (handle, "%i", (vect)[d]);
            } else {
                fprintf (handle, "?");
            }
            fprintf (handle, " ");
        }
        fprintf (handle, "]");
    } else {
        fprintf (handle, "NULL");
    }

    DBUG_VOID_RETURN;
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
    int old_indent = global.indent;

    DBUG_ENTER ("PRTfundef");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, NODE_TEXT (arg_node), arg_node));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    /*
     * needed for the introduction of PROFILE_... MACROS in the
     *  function body.
     */
    INFO_FUNDEF (arg_info) = arg_node;

    if (INFO_PROTOTYPE (arg_info)) {
        /*
         * print function declaration
         */

        if ((!FUNDEF_ISZOMBIE (arg_node)) && (!FUNDEF_ISSPMDFUN (arg_node))) {
            if ((FUNDEF_BODY (arg_node) == NULL)
                || ((FUNDEF_RETURN (arg_node) != NULL)
                    && (NODE_TYPE (FUNDEF_RETURN (arg_node)) == N_icm))) {
                fprintf (global.outfile, "extern ");

                if ((FUNDEF_ICM (arg_node) == NULL)
                    || (NODE_TYPE (FUNDEF_ICM (arg_node)) != N_icm)) {
                    PrintFunctionHeader (arg_node, arg_info, FALSE);
                } else {
                    /* print N_icm ND_FUN_DEC */
                    fprintf (global.outfile, "\n");
                    TRAVdo (FUNDEF_ICM (arg_node), arg_info);
                }

                fprintf (global.outfile, ";\n");

                if (global.compiler_phase < PH_genccode) {
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
            if (global.compiler_phase < PH_genccode) {
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

            if (FUNDEF_BODY (arg_node) != NULL) {
                if (INFO_SEPARATE (arg_info)) {
                    global.outfile = FMGRwriteOpen ("%s/fun%d.c", global.tmp_dirname,
                                                    global.function_counter);
                    fprintf (global.outfile, "#include \"header.h\"\n\n");
                }

                if (FUNDEF_ISWRAPPERFUN (arg_node)) {
                    fprintf (global.outfile, "/* wrapper function */\n");
                } else if (FUNDEF_ISCONDFUN (arg_node)) {
                    fprintf (global.outfile, "/* Cond function */\n");

                } else if (FUNDEF_ISDOFUN (arg_node)) {
                    fprintf (global.outfile, "/* Loop function */\n");
                }

                if ((FUNDEF_ICM (arg_node) == NULL)
                    || (NODE_TYPE (FUNDEF_ICM (arg_node)) != N_icm)) {
                    PrintFunctionHeader (arg_node, arg_info, FALSE);
                } else {
                    /* print N_icm ND_FUN_DEC */
                    TRAVdo (FUNDEF_ICM (arg_node), arg_info);
                }

                fprintf (global.outfile, "\n");

                /* traverse function body */
                TRAVdo (FUNDEF_BODY (arg_node), arg_info);

                if (global.compiler_phase < PH_genccode) {
                    if (FUNDEF_PRAGMA (arg_node) != NULL) {
                        TRAVdo (FUNDEF_PRAGMA (arg_node), arg_info);
                    }
                }

                fprintf (global.outfile, "\n\n");

                if (INFO_SEPARATE (arg_info)) {
                    fclose (global.outfile);
                    global.function_counter++;
                }
            }
        }
    }

    DBUG_ASSERTF (global.indent == old_indent,
                  ("Indentation unbalanced while printing function '%s`.\n"
                   " Indentation at beginning of function: %i.\n"
                   " Indentation at end of function: %i\n",
                   FUNDEF_NAME (arg_node), old_indent, global.indent));

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
    static char strbuffer1[256];

    DBUG_ENTER ("PRTannotate");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, NODE_TEXT (arg_node), arg_node));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (ANNOTATE_TAG (arg_node) & CALL_FUN) {
        sprintf (strbuffer1, "PROFILE_BEGIN_UDF( %d, %d)", ANNOTATE_FUNNUMBER (arg_node),
                 ANNOTATE_FUNAPNUMBER (arg_node));
    } else {
        if (ANNOTATE_TAG (arg_node) & RETURN_FROM_FUN) {
            sprintf (strbuffer1, "PROFILE_END_UDF( %d, %d)",
                     ANNOTATE_FUNNUMBER (arg_node), ANNOTATE_FUNAPNUMBER (arg_node));
        } else {
            DBUG_ASSERT ((0), "wrong tag at N_annotate");
        }
    }

#if 0 /**** TODO ****/
  if (ANNOTATE_TAG (arg_node) & INL_FUN) {
    sprintf (strbuffer2, "PROFILE_INLINE( %s)", strbuffer1);
  } else {
    strcpy (strbuffer2, strbuffer1);
  }

  if (ANNOTATE_TAG (arg_node) & LIB_FUN) {
    sprintf (strbuffer1, "PROFILE_LIBRARY( %s)", strbuffer2);
  } else {
    strcpy (strbuffer1, strbuffer2);
  }

#endif

    fprintf (global.outfile, "%s;", strbuffer1);

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

    DBUG_ENTER ("PRTarg");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (ARG_NTYPE (arg_node) != NULL) {
        type_str = TYtype2String (ARG_NTYPE (arg_node), FALSE, 0);
    } else {
        type_str = CVtype2String (ARG_TYPE (arg_node), 0, TRUE);
    }
    fprintf (global.outfile, " %s ", type_str);
    type_str = ILIBfree (type_str);

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

    DBUG_ENTER ("PRTvardec");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    if ((VARDEC_ICM (arg_node) == NULL) || (NODE_TYPE (VARDEC_ICM (arg_node)) != N_icm)) {
        type_str = TYtype2String (VARDEC_NTYPE (arg_node), FALSE, 0);
        fprintf (global.outfile, "%s ", type_str);
        type_str = ILIBfree (type_str);

        if (AVIS_ISUNIQUE (VARDEC_AVIS (arg_node))) {
            fprintf (global.outfile, "*");
        }

        fprintf (global.outfile, "%s", VARDEC_NAME (arg_node));

        fprintf (global.outfile, "; ");

        type_str = CVtype2String (VARDEC_TYPE (arg_node), 0, TRUE);
        fprintf (global.outfile, "/* %s */", type_str);
        type_str = ILIBfree (type_str);

        TRAVdo (VARDEC_AVIS (arg_node), arg_info);

        fprintf (global.outfile, "\n");
    } else {
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
    int old_indent = global.indent;

    DBUG_ENTER ("PRTblock");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, NODE_TEXT (arg_node), arg_node));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, "{ \n");
    global.indent++;

    if (BLOCK_CACHESIM (arg_node) != NULL) {
        INDENT;
        fprintf (global.outfile, "#pragma cachesim \"%s\"\n\n",
                 BLOCK_CACHESIM (arg_node));
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
        fprintf (global.outfile, "\n");
    }

    DBUG_EXECUTE ("PRINT_SSA", if (BLOCK_SSACOUNTER (arg_node) != NULL) {
        INDENT;
        fprintf (global.outfile, "/* SSAcnt:\n");
        TRAVdo (BLOCK_SSACOUNTER (arg_node), arg_info);
        INDENT;
        fprintf (global.outfile, " */\n");
    });

    if (BLOCK_INSTR (arg_node)) {
        TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    global.indent--;
    INDENT;
    fprintf (global.outfile, "}");

    if (FUNDEF_ISSPMDFUN (INFO_FUNDEF (arg_info)) && (BLOCK_VARDEC (arg_node) != NULL)) {
        DBUG_ASSERTF (
          global.indent - old_indent == -1,
          ("Indentation unbalanced while printing block of SPMD function '%s`.\n"
           " Indentation at beginning of block: %i.\n"
           " Indentation at end of block: %i.\n"
           " Indentation at end should be one less than at beginning.",
           FUNDEF_NAME (INFO_FUNDEF (arg_info)), old_indent, global.indent));
    } else {
        DBUG_ASSERTF (global.indent == old_indent,
                      ("Indentation unbalanced while printing function '%s`.\n"
                       " Indentation at beginning of function: %i.\n"
                       " Indentation at end of function: %i\n",
                       FUNDEF_NAME (arg_node), old_indent, global.indent));
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
    DBUG_ENTER ("PRTreturn");

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

    DBUG_ENTER ("PRTassign");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, NODE_TEXT (arg_node), arg_node));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    instr = ASSIGN_INSTR (arg_node);
    DBUG_ASSERT ((instr != NULL), "instruction of N_assign is NULL");

    trav_instr = TRUE;
    if (NODE_TYPE (instr) == N_annotate) {
        if (global.compiler_phase < PH_compile) {
            trav_instr = FALSE;
        }
        DBUG_EXECUTE ("PRINT_PROFILE", trav_instr = TRUE;);
    }

    if (trav_instr) {
        if (NODE_TYPE (instr) != N_icm) {
            INDENT;
        } else {
            last_assignment_icm = instr;
        }
        TRAVdo (instr, arg_info);
        fprintf (global.outfile, "\n");
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
    DBUG_ENTER ("PRTdo");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
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
    DBUG_ENTER ("PRTwhile");

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
    DBUG_ENTER ("PRTcond");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "if ");

    fprintf (global.outfile, "(");
    TRAVdo (COND_COND (arg_node), arg_info);
    fprintf (global.outfile, ") \n");

    if (COND_THEN (arg_node) != NULL) {
        TRAVdo (COND_THEN (arg_node), arg_info);
        fprintf (global.outfile, "\n");
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

    DBUG_ENTER ("PRTcast");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    type_str = TYtype2String (CAST_NTYPE (arg_node), FALSE, 0);
    fprintf (global.outfile, "(: %s) ", type_str);
    type_str = ILIBfree (type_str);

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

    DBUG_ENTER ("PRTlet");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, NODE_TEXT (arg_node), arg_node));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    expr = LET_EXPR (arg_node);
    if ((NODE_TYPE (expr) == N_ap) && (AP_ARGTAB (expr) != NULL)) {
        print_sac = FALSE;
        print_c = TRUE;

        DBUG_EXECUTE ("PRINT_ARGTAB", fprintf (global.outfile, "/* \n"); INDENT;
                      fprintf (global.outfile, " *  ");
                      PRTprintArgtab (AP_ARGTAB (expr), FALSE);
                      fprintf (global.outfile, "  */\n"); INDENT; print_sac = TRUE;
                      print_argtab = TRUE;);
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
    prf prf;
    char *prf_str;

    DBUG_ENTER ("PRTprf");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    prf = PRF_PRF (arg_node);
    prf_str = global.prf_symbol[prf];

    DBUG_PRINT ("PRINT",
                ("%s (%s)" F_PTR, NODE_TEXT (arg_node), global.mdb_prf[prf], arg_node));

    if (global.prf_is_infix[prf] && (TCcountExprs (PRF_ARGS (arg_node)) < 3)) {
        /* primitive functions in infix notation */
        fprintf (global.outfile, "(");
        if (PRF_EXPRS2 (arg_node) == NULL) {
            fprintf (global.outfile, "%s ", prf_str);
            PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        }
        if (PRF_EXPRS2 (arg_node) != NULL) {
            PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
            fprintf (global.outfile, " %s ", prf_str);
            PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        }
        fprintf (global.outfile, ")");
    } else {
        /**
         * print prf in prefix format
         */
        fprintf (global.outfile, "%s(", prf_str);
        if (PRF_ARGS (arg_node) != NULL) {
            fprintf (global.outfile, " ");
            TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
        fprintf (global.outfile, ")");
    }

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

    DBUG_ENTER ("PRTfuncond");

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

    DBUG_ENTER ("PRTap");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fundef = AP_FUNDEF (arg_node);

    DBUG_ASSERT ((fundef != NULL), "no AP_FUNDEF found!");

    /*
     * print name of 'AP_FUNDEF(arg_node)'
     */
    if ((FUNDEF_ISWRAPPERFUN (fundef))) {
        fprintf (global.outfile, "wrapper:");
    }
    if (FUNDEF_NS (fundef) != NULL) {
        fprintf (global.outfile, "%s::", NSgetName (FUNDEF_NS (fundef)));
    }
    fprintf (global.outfile, "%s", FUNDEF_NAME (fundef));

    fprintf (global.outfile, "(");
    if (AP_ARGS (arg_node) != NULL) {
        fprintf (global.outfile, " ");
        TRAVdo (AP_ARGS (arg_node), arg_info);
    }
    fprintf (global.outfile, ")");

    DBUG_EXECUTE ("PRINT_PTR", fprintf (global.outfile, " /* ");
                  PRINT_POINTER (global.outfile, fundef);
                  fprintf (global.outfile, " */ "););

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
    DBUG_ENTER ("PRTspap");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    SPAP_ID (arg_node) = TRAVdo (SPAP_ID (arg_node), arg_info);

    fprintf (global.outfile, "(");
    if (SPAP_ARGS (arg_node) != NULL) {
        fprintf (global.outfile, " ");
        TRAVdo (SPAP_ARGS (arg_node), arg_info);
    }
    fprintf (global.outfile, ")");

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

    DBUG_ENTER ("PRTspmop");

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
 *   node *PRTempty( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTempty (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTempty");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, "/* empty */\n");

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
    shpseg *old_print_shape = INFO_SHAPE (arg_info);
    shpseg *old_print_shape_counter = INFO_SHAPE_COUNTER (arg_info);

    DBUG_ENTER ("PRTarray");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (ARRAY_AELEMS (arg_node) != NULL) {

        INFO_DIM (arg_info) = ARRAY_DIM (arg_node);
        INFO_SHAPE (arg_info) = SHshape2OldShpseg (ARRAY_SHAPE (arg_node));
        INFO_SHAPE_COUNTER (arg_info)
          = TCarray2Shpseg (TCcreateZeroVector (ARRAY_DIM (arg_node), T_int), NULL);

        for (i = 0; i < INFO_DIM (arg_info); i++)
            fprintf (global.outfile, "[ ");

        TRAVdo (ARRAY_AELEMS (arg_node), arg_info);

        for (i = 0; i < INFO_DIM (arg_info); i++)
            fprintf (global.outfile, " ]");

    } else {
        type_str = TYtype2String (ARRAY_ELEMTYPE (arg_node), FALSE, 0);
        fprintf (global.outfile, "[:%s]", type_str);
        type_str = ILIBfree (type_str);
    }

    if (INFO_SHAPE (arg_info) != NULL)
        FREEfreeShpseg (INFO_SHAPE (arg_info));

    if (INFO_SHAPE_COUNTER (arg_info) != NULL)
        FREEfreeShpseg (INFO_SHAPE_COUNTER (arg_info));

    INFO_DIM (arg_info) = old_print_dim;
    INFO_SHAPE (arg_info) = old_print_shape;
    INFO_SHAPE_COUNTER (arg_info) = old_print_shape_counter;

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

node *
PRTexprs (node *arg_node, info *arg_info)
{
    int i;
    int j;

    DBUG_ENTER ("PRTexprs");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    TRAVdo (EXPRS_EXPR (arg_node), arg_info);

    if (EXPRS_NEXT (arg_node) != NULL) {
        if (arg_info != NULL) {
            for (i = INFO_DIM (arg_info) - 1;
                 (i >= 0)
                 && (++SHPSEG_SHAPE (INFO_SHAPE_COUNTER (arg_info), i)
                     >= SHPSEG_SHAPE (INFO_SHAPE (arg_info), i));
                 i--)
                SHPSEG_SHAPE (INFO_SHAPE_COUNTER (arg_info), i) = 0;
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

    DBUG_ENTER ("PRTid");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (global.compiler_phase == PH_genccode) {
        if (ID_NT_TAG (arg_node) != NULL) {
            text = ID_NT_TAG (arg_node);
        } else if (ID_ICMTEXT (arg_node) != NULL) {
            text = ID_ICMTEXT (arg_node);
        } else if (ID_AVIS (arg_node) != NULL) {
            text = ID_NAME (arg_node);
        } else {
            text = "";
            DBUG_ASSERT (FALSE,
                         "Found an Id node with neither NTtag nor ICMText nor Name");
        }
    } else {
        if (ID_AVIS (arg_node) != NULL) {
            text = ID_NAME (arg_node);
        } else if (ID_ICMTEXT (arg_node) != NULL) {
            text = ID_ICMTEXT (arg_node);
        } else {
            text = "";
            DBUG_ASSERT (FALSE, "Found an Id node with neither Avis nor ICMText");
        }
    }

    fprintf (global.outfile, "%s", text);

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
    DBUG_ENTER ("PRTspid");

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
    DBUG_ENTER ("PRTglobobj");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (global.compiler_phase == PH_genccode) {
        DBUG_ASSERT ((OBJDEF_NT_TAG (GLOBOBJ_OBJDEF (arg_node)) != NULL),
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
    DBUG_ENTER ("PRTnum");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "%d", NUM_VAL (arg_node));

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

    DBUG_ENTER ("PRTfloat");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    tmp_string = CVfloat2String (FLOAT_VAL (arg_node));
    fprintf (global.outfile, "%s", tmp_string);
    tmp_string = ILIBfree (tmp_string);

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

    DBUG_ENTER ("PRTdouble");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    tmp_string = CVdouble2String (DOUBLE_VAL (arg_node));
    fprintf (global.outfile, "%s", tmp_string);
    tmp_string = ILIBfree (tmp_string);

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
    DBUG_ENTER ("PRTbool");

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
    char *s;

    DBUG_ENTER ("PRTstr");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    s = STR_STRING (arg_node);

    putc ('\"', global.outfile);
    while (s[0] != '\0') {
        if (s[0] == '\"') {
            fprintf (global.outfile, "\\\"");
        } else {
            putc (s[0], global.outfile);
        }
        s++;
    }
    putc ('\"', global.outfile);

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

    DBUG_ENTER ("PRTtype");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, NODE_TEXT (arg_node), arg_node));

    if (TYPE_TYPE (arg_node) != NULL) {
        type_str = TYtype2String (TYPE_TYPE (arg_node), FALSE, 0);
        fprintf (global.outfile, "%s", type_str);
        type_str = ILIBfree (type_str);
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

    DBUG_ENTER ("PRTdot");

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
    DBUG_ENTER ("PRTsetwl");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (NODE_TYPE (SETWL_VEC (arg_node)) == N_exprs) {
        fprintf (global.outfile, "{ [");
        TRAVdo (SETWL_VEC (arg_node), arg_info);
        fprintf (global.outfile, "] -> ");
    } else {
        TRAVdo (SETWL_VEC (arg_node), arg_info);
        fprintf (global.outfile, " -> ");
    }

    TRAVdo (SETWL_EXPR (arg_node), arg_info);
    fprintf (global.outfile, " }");

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
    DBUG_ENTER ("PRTchar");

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

    DBUG_ENTER ("PRTicm");

    DBUG_PRINT ("PRINT", ("icm-node %s\n", ICM_NAME (arg_node)));

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (global.compiler_phase == PH_genccode) {
#define ICM_ALL
#define ICM_DEF(prf, trf)                                                                \
    if (strcmp (ICM_NAME (arg_node), #prf) == 0) {                                       \
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
#undef ICM_VARANY
#undef ICM_VARNT
#undef ICM_VARID
#undef ICM_VARINT
#undef ICM_END
        ;
    }

    if (!compiled_icm) {
        if (last_assignment_icm == arg_node) {
            global.indent += ICM_INDENT_BEFORE (arg_node);
            INDENT;
        }
        fprintf (global.outfile, "SAC_%s( ", ICM_NAME (arg_node));
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

node *
PRTpragma (node *arg_node, info *arg_info)
{
    int i;
    node *nums;

    DBUG_ENTER ("PRTpragma");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (PRAGMA_LINKNAME (arg_node) != NULL) {
        fprintf (global.outfile, "#pragma linkname \"%s\"\n", PRAGMA_LINKNAME (arg_node));
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

    if (PRAGMA_TOUCH (arg_node) != NULL) {
        fprintf (global.outfile, "#pragma touch ");
        TRAVdo (PRAGMA_TOUCH (arg_node), arg_info);
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
 *   node *PRTspmd( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PRTspmd (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTspmd");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "\n");

    if (SPMD_ICM_BEGIN (arg_node) == NULL) {
        /*
         * The SPMD-block has not yet been compiled.
         */

        INDENT;
        fprintf (global.outfile, "/*** begin of SPMD region ***\n");

        INDENT;
        fprintf (global.outfile, " * in:");
        DFMprintMask (global.outfile, " %s", SPMD_IN (arg_node));
        fprintf (global.outfile, "\n");

        INDENT;
        fprintf (global.outfile, " * inout:");
        DFMprintMask (global.outfile, " %s", SPMD_INOUT (arg_node));
        fprintf (global.outfile, "\n");

        INDENT;
        fprintf (global.outfile, " * out:");
        DFMprintMask (global.outfile, " %s", SPMD_OUT (arg_node));
        fprintf (global.outfile, "\n");

        INDENT;
        fprintf (global.outfile, " * shared:");
        DFMprintMask (global.outfile, " %s", SPMD_SHARED (arg_node));
        fprintf (global.outfile, "\n");

        INDENT;
        fprintf (global.outfile, " * local:");
        DFMprintMask (global.outfile, " %s", SPMD_LOCAL (arg_node));
        fprintf (global.outfile, "\n");

        INDENT;
        fprintf (global.outfile, " */\n");

        TRAVdo (SPMD_REGION (arg_node), arg_info);

        fprintf (global.outfile, "\n");
        INDENT;
        fprintf (global.outfile, "/*** end of SPMD region ***/\n");
    } else {
        /*
         * print ICMs
         */
        INDENT;
        TRAVdo (SPMD_ICM_BEGIN (arg_node), arg_info);
        fprintf (global.outfile, "\n");
        TRAVdo (SPMD_ICM_PARALLEL (arg_node), arg_info);
        INDENT;
        fprintf (global.outfile, "\n");
        INDENT;
        TRAVdo (SPMD_ICM_ALTSEQ (arg_node), arg_info);
        fprintf (global.outfile, "\n");
        TRAVdo (SPMD_ICM_SEQUENTIAL (arg_node), arg_info);
        fprintf (global.outfile, "\n");
        INDENT;
        TRAVdo (SPMD_ICM_END (arg_node), arg_info);
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
    DBUG_ENTER ("PRTmt");

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
    DBUG_ENTER ("PRTex");

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
    DBUG_ENTER ("PRTst");

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
 * function:
 *   node *PRTwith( node *arg_node, info *arg_info)
 *
 * description:
 *   prints Nwith node.
 *
 ******************************************************************************/

node *
PRTwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTwith");

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

    TRAVdo (WITH_WITHOP (arg_node), arg_info);

    global.indent--;

    DBUG_EXECUTE ("PRINT_RC", if (WITH_PRAGMA (arg_node) == NULL) {
        fprintf (global.outfile, "\n");
        INDENT;
    } INDENT;);

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
    DBUG_ENTER ("PRTwithid");

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
    DBUG_ENTER ("PRTgenerator");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "(");

    /* print upper bound */
    if (GENERATOR_BOUND1 (arg_node)) {
        TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);
    } else {
        fprintf (global.outfile, ". (NULL)");
    }
    /* print first operator */
    fprintf (global.outfile, " %s ", global.prf_symbol[GENERATOR_OP1 (arg_node)]);

    /* print indices */
    if (INFO_NPART (arg_info) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (INFO_NPART (arg_info)) == N_part),
                     "INFO_NPART is no N_part node");

        DBUG_ASSERT ((PART_WITHID (INFO_NPART (arg_info)) != NULL),
                     "PART_WITHID not found!");
        TRAVdo (PART_WITHID (INFO_NPART (arg_info)), arg_info);
    } else {
        fprintf (global.outfile, "?");
    }

    /* print second operator */
    fprintf (global.outfile, " %s ", global.prf_symbol[GENERATOR_OP2 (arg_node)]);
    /* print lower bound */
    if (GENERATOR_BOUND2 (arg_node)) {
        TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);
    } else {
        fprintf (global.outfile, ". (NULL)");
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
    fprintf (global.outfile, ")\n");

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
    DBUG_ENTER ("PRTdefault");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "default partition( ");

    /* print indices */
    if (INFO_NPART (arg_info) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (INFO_NPART (arg_info)) == N_part),
                     "INFO_NPART is no N_part node");

        DBUG_ASSERT ((PART_WITHID (INFO_NPART (arg_info)) != NULL),
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
    DBUG_ENTER ("PRTcode");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    DBUG_ASSERT ((CODE_USED (arg_node) >= 0), "illegal CODE_USED value!");

    if (CODE_ISSIMDSUITABLE (arg_node)) {
        PrintSimdBegin ();
    }

    /* print the code section; the body first */
    TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    fprintf (global.outfile, " : ");
    TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    fprintf (global.outfile, " ; ");

    if (CODE_ISSIMDSUITABLE (arg_node)) {
        PrintSimdEnd ();
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

    DBUG_ENTER ("PRTpart");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    tmp_npart = INFO_NPART (arg_info);
    INFO_NPART (arg_info) = arg_node;

    /* print generator */
    INDENT; /* each gen in a new line. */
    TRAVdo (PART_GENERATOR (arg_node), arg_info);

    DBUG_ASSERT ((PART_CODE (arg_node) != NULL),
                 "part within WL without pointer to N_code");

    TRAVdo (PART_CODE (arg_node), arg_info);

    if (PART_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, ",\n");
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
    DBUG_ENTER ("PRTgenarray");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    fprintf (global.outfile, "genarray( ");
    TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        fprintf (global.outfile, " , ");
        TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    if (GENARRAY_MEM (arg_node) != NULL) {
        fprintf (global.outfile, " , ");
        TRAVdo (GENARRAY_MEM (arg_node), arg_info);
    }

    if (GENARRAY_RC (arg_node) != NULL) {
        fprintf (global.outfile, " ,RC(");
        TRAVdo (GENARRAY_RC (arg_node), arg_info);
        fprintf (global.outfile, ")");
    }

    if (GENARRAY_IDX (arg_node) != NULL) {
        fprintf (global.outfile, " ,IDX(%s)", AVIS_NAME (GENARRAY_IDX (arg_node)));
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
    DBUG_ENTER ("PRTmodarray");

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

    if (MODARRAY_RC (arg_node) != NULL) {
        fprintf (global.outfile, " ,RC(");
        TRAVdo (MODARRAY_RC (arg_node), arg_info);
        fprintf (global.outfile, ")");
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
    DBUG_ENTER ("PRTspfold");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    DBUG_ASSERT ((SPFOLD_FUN (arg_node) != NULL), "Missing fold function symbol");
    /**
     * udf-case prior to TC!
     */
    if (SPFOLD_NS (arg_node) == NULL) {
        fprintf (global.outfile, "fold/*udf-symb*/( %s, ", SPFOLD_FUN (arg_node));
    } else {
        fprintf (global.outfile, "fold/*udf-symb*/( %s::%s, ",
                 NSgetName (SPFOLD_NS (arg_node)), SPFOLD_FUN (arg_node));
    }
    TRAVdo (SPFOLD_NEUTRAL (arg_node), arg_info);

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

    DBUG_ENTER ("PRTfold");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;

    DBUG_ASSERT (FOLD_FUNDEF (arg_node) != NULL, "Missing fold function link");
    /**
     * * udf-case after TC!
     */
    fundef = FOLD_FUNDEF (arg_node);
    fprintf (global.outfile, "fold(");
    if (FUNDEF_NS (fundef) != NULL) {
        fprintf (global.outfile, " %s::", NSgetName (FUNDEF_NS (fundef)));
    }
    fprintf (global.outfile, "%s, ", FUNDEF_NAME (fundef));
    TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    fprintf (global.outfile, ")");

    if (FOLD_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, ",\n");
        /*
         * continue with other withops
         */
        PRINT_CONT (TRAVdo (FOLD_NEXT (arg_node), arg_info), ;);
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

    DBUG_ENTER ("PRTwith2");

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
    TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    global.indent--;

    DBUG_EXECUTE ("PRINT_RC", if (WITH2_PRAGMA (arg_node) == NULL) {
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
 *   node *PrintWLsegx( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_wlseg- and N_wlsegVar-nodes.
 *
 ******************************************************************************/

static node *
PrintWLsegx (node *arg_node, info *arg_info)
{
    node *seg;
    int id;

    DBUG_ENTER ("PrintWLsegx");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    seg = arg_node;
    id = 0;
    while (seg != NULL) {
        INDENT;
        fprintf (global.outfile, "/**********%s segment %d: **********",
                 (NODE_TYPE (seg) == N_wlseg) ? "" : " (var.)", id++);

        fprintf (global.outfile, "\n");
        INDENT;
        fprintf (global.outfile, " * index domain: ");
        WLSEGX_IDX_PRINT (global.outfile, seg, IDX_MIN);
        fprintf (global.outfile, " -> ");
        WLSEGX_IDX_PRINT (global.outfile, seg, IDX_MAX);
        fprintf (global.outfile, "\n");
        INDENT;

        if (NODE_TYPE (arg_node) == N_wlseg) {
            fprintf (global.outfile, " * sv: ");
            PRINT_VECT (global.outfile, WLSEG_SV (arg_node), WLSEG_DIMS (arg_node), "%i");
            fprintf (global.outfile, "\n");
            INDENT;
            fprintf (global.outfile, " * homsv: ");
            PRTprintHomsv (global.outfile, WLSEG_HOMSV (arg_node), WLSEG_DIMS (arg_node));
            fprintf (global.outfile, "\n");
            INDENT;
        }

        if (WLSEGX_SCHEDULING (seg) != NULL) {
            fprintf (global.outfile, " * scheduling: ");
            SCHprintScheduling (global.outfile, WLSEGX_SCHEDULING (seg));
            fprintf (global.outfile, "\n");
            INDENT;
        }

        if (WLSEGX_TASKSEL (seg) != NULL) {
            fprintf (global.outfile, " * taskselector: ");
            SCHprintTasksel (global.outfile, WLSEGX_TASKSEL (seg));
            fprintf (global.outfile, "\n");
            INDENT;
        }

        fprintf (global.outfile, " */\n");

        TRAVdo (WLSEGX_CONTENTS (seg), arg_info);
        PRINT_CONT (seg = WLSEGX_NEXT (seg), seg = NULL)
    }

    DBUG_RETURN (arg_node);
}

node *
PRTwlseg (node *arg_node, info *arg_info)
{
    return (PrintWLsegx (arg_node, arg_info));
}

node *
PRTwlsegvar (node *arg_node, info *arg_info)
{
    return (PrintWLsegx (arg_node, arg_info));
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
    int bound1, bound2;

    DBUG_ENTER ("PrintWLxblock");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, "(");

    bound1 = WLXBLOCK_BOUND1 (arg_node);
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node), &bound1,
                       WLXBLOCK_DIM (arg_node));
    fprintf (global.outfile, " -> ");
    bound2 = WLXBLOCK_BOUND2 (arg_node);
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node), &bound2,
                       WLXBLOCK_DIM (arg_node));
    fprintf (global.outfile, "), ");

    fprintf (global.outfile,
             "%sblock%d[%d] %d:", (NODE_TYPE (arg_node) == N_wlblock) ? "" : "u",
             WLXBLOCK_LEVEL (arg_node), WLXBLOCK_DIM (arg_node),
             WLXBLOCK_STEP (arg_node));

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
 *   node *PRTwlsimd( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_wlsimd node
 *
 ******************************************************************************/

node *
PRTwlsimd (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTwlsimd");

    PrintSimdBegin ();

    TRAVcont (arg_node, arg_info);

    PrintSimdEnd ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTwlstridex( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_wlstride- and N_wlstrideVar-nodes.
 *
 * remark:
 *   N_wlstride-nodes    are printed as '->',
 *   N_wlstrideVar-nodes are printed as '=>'.
 *
 ******************************************************************************/

node *
PRTwlstride (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTwlstridex");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    if (WLSTRIDE_ISSIMDSUITABLE (arg_node)) {
        PrintSimdBegin ();
    }

    INDENT;
    fprintf (global.outfile, "(");
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node),
                       WLSTRIDEX_GET_ADDR (arg_node, BOUND1), WLSTRIDEX_DIM (arg_node));
    fprintf (global.outfile, " %s> ", (NODE_TYPE (arg_node) == N_wlstride) ? "-" : "=");
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node),
                       WLSTRIDEX_GET_ADDR (arg_node, BOUND2), WLSTRIDEX_DIM (arg_node));
    fprintf (global.outfile, "), step%d[%d] ", WLSTRIDEX_LEVEL (arg_node),
             WLSTRIDEX_DIM (arg_node));
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node),
                       WLSTRIDEX_GET_ADDR (arg_node, STEP), WLSTRIDEX_DIM (arg_node));

    if (WLSTRIDEX_ISNOOP (arg_node)) {
        fprintf (global.outfile, ": /* noop */");
    }

    fprintf (global.outfile, "\n");

    if (WLSTRIDEX_CONTENTS (arg_node) != NULL) {
        global.indent++;
        TRAVdo (WLSTRIDEX_CONTENTS (arg_node), arg_info);
        global.indent--;
    }

    if (WLSTRIDE_ISSIMDSUITABLE (arg_node)) {
        PrintSimdEnd ();
    }

    if (WLSTRIDEX_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (WLSTRIDEX_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

node *
PRTwlstridevar (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTwlstridevar");

    INDENT;
    fprintf (global.outfile, "(");
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node),
                       WLSTRIDEX_GET_ADDR (arg_node, BOUND1), WLSTRIDEX_DIM (arg_node));
    fprintf (global.outfile, " %s> ", (NODE_TYPE (arg_node) == N_wlstride) ? "-" : "=");
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node),
                       WLSTRIDEX_GET_ADDR (arg_node, BOUND2), WLSTRIDEX_DIM (arg_node));
    fprintf (global.outfile, "), step%d[%d] ", WLSTRIDEX_LEVEL (arg_node),
             WLSTRIDEX_DIM (arg_node));
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node),
                       WLSTRIDEX_GET_ADDR (arg_node, STEP), WLSTRIDEX_DIM (arg_node));

    if (WLSTRIDEX_ISNOOP (arg_node)) {
        fprintf (global.outfile, ": /* noop */");
    }

    fprintf (global.outfile, "\n");

    if (WLSTRIDEX_CONTENTS (arg_node) != NULL) {
        global.indent++;
        TRAVdo (WLSTRIDEX_CONTENTS (arg_node), arg_info);
        global.indent--;
    }

    if (WLSTRIDEX_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (WLSTRIDEX_NEXT (arg_node), arg_info), ;);
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
    DBUG_ENTER ("PRTwlcode");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, " ");
    if (arg_node != NULL) {
        DBUG_ASSERT ((NODE_TYPE (arg_node) == N_code), "illegal code node found!");

        /*
         * we use the code here, therefore the counter USED should be >0 !!
         */
        DBUG_ASSERT ((CODE_USED (arg_node) > 0), "illegal CODE_USED value!");

        fprintf (global.outfile, "op_%d", CODE_ID (arg_node));
    } else {
        if (INFO_NWITH2 (arg_info) != NULL) {
            DBUG_ASSERT ((NODE_TYPE (INFO_NWITH2 (arg_info)) == N_with2),
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
                DBUG_ASSERT ((0), "illegal with-loop type found");
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
 *   node *PrintWLgridx( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_wlgrid- and N_wlgridVar-nodes.
 *
 * remark:
 *   N_wlgrid-nodes    are printed as '-->' (fitted) and '->>' (unfitted),
 *   N_wlgridVar-nodes are printed as '==>' (fitted) and '=>>' (unfitted)
 *   respectively.
 *
 ******************************************************************************/

node *
PRTwlgrid (node *arg_node, info *arg_info)
{
    char *str = (NODE_TYPE (arg_node) == N_wlgrid) ? "-" : "=";

    DBUG_ENTER ("PrintWLgridx");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, "(");
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node),
                       WLGRIDX_GET_ADDR (arg_node, BOUND1), WLGRIDX_DIM (arg_node));
    fprintf (global.outfile, " %s%s> ", str, WLGRIDX_ISFITTED (arg_node) ? str : ">");
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node),
                       WLGRIDX_GET_ADDR (arg_node, BOUND2), WLGRIDX_DIM (arg_node));
    fprintf (global.outfile, "):");

    if (WLGRIDX_NEXTDIM (arg_node) != NULL) {
        fprintf (global.outfile, "\n");
        global.indent++;
        TRAVdo (WLGRIDX_NEXTDIM (arg_node), arg_info);
        global.indent--;
    } else {
        if ((WLGRIDX_CODE (arg_node) != NULL) || (!WLGRIDX_ISNOOP (arg_node))) {
            PRTwlcode (WLGRIDX_CODE (arg_node), arg_info);
        }
        if (WLGRIDX_ISNOOP (arg_node)) {
            fprintf (global.outfile, " /* noop */");
        }
        fprintf (global.outfile, "\n");
    }

    if (WLGRIDX_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (WLGRIDX_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

node *
PRTwlgridvar (node *arg_node, info *arg_info)
{
    char *str = (NODE_TYPE (arg_node) == N_wlgrid) ? "-" : "=";

    DBUG_ENTER ("PrintWLgridx");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, "(");
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node),
                       WLGRIDX_GET_ADDR (arg_node, BOUND1), WLGRIDX_DIM (arg_node));
    fprintf (global.outfile, " %s%s> ", str, WLGRIDX_ISFITTED (arg_node) ? str : ">");
    WLBnodeOrIntPrint (global.outfile, NODE_TYPE (arg_node),
                       WLGRIDX_GET_ADDR (arg_node, BOUND2), WLGRIDX_DIM (arg_node));
    fprintf (global.outfile, "):");

    if (WLGRIDX_NEXTDIM (arg_node) != NULL) {
        fprintf (global.outfile, "\n");
        global.indent++;
        TRAVdo (WLGRIDX_NEXTDIM (arg_node), arg_info);
        global.indent--;
    } else {
        if ((WLGRIDX_CODE (arg_node) != NULL) || (!WLGRIDX_ISNOOP (arg_node))) {
            PRTwlcode (WLGRIDX_CODE (arg_node), arg_info);
        }
        if (WLGRIDX_ISNOOP (arg_node)) {
            fprintf (global.outfile, " /* noop */");
        }
        fprintf (global.outfile, "\n");
    }

    if (WLGRIDX_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (WLGRIDX_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

#if 0 /**** TODO ****/

/******************************************************************************
 *
 * function:
 *   node *PrintCWrapper( node *arg_node, info *arg_info)
 *
 * description:
 *   prints N_cwrapper nodes to generate C interface files.
 *
 ******************************************************************************/

node *
PRTcwrapper (node * arg_node, info * arg_info)
{
  DBUG_ENTER ("PrintCWrapper");

  if (NODE_ERROR (arg_node) != NULL) {
    NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
  }

  if (global.compiler_phase != PH_genccode) {
    /* internal debug output of mapping-tree of wrappers */
    DBUG_EXECUTE ("PRINT_CWRAPPER", {
                  nodelist * funlist;
                  node * fundef;
                  char *type_str;
                  fprintf (global.outfile,
                           "CWrapper %s with %d arg(s) and %d result(s)\n",
                           CWRAPPER_NAME (arg_node),
                           CWRAPPER_ARGCOUNT (arg_node),
                           CWRAPPER_RESCOUNT (arg_node));
                  funlist = CWRAPPER_FUNS (arg_node);
                  while (funlist != NULL) {
                  fundef = NODELIST_NODE (funlist);
                  fprintf (global.outfile, "  overloaded for (");
                  /* print args of function */
                  if (FUNDEF_ARGS (fundef) != NULL) {
                  TRAVdo (FUNDEF_ARGS (fundef), arg_info);}

                  fprintf (global.outfile, ") -> (");
                  /* print results of function */
                  type_str =
                  Type2String (FUNDEF_WRAPPERTYPES (fundef), 0, TRUE);
                  fprintf (global.outfile, "%s", type_str);
                  type_str = ILIBfree (type_str);
                  fprintf (global.outfile, ")\n");
                  funlist = NODELIST_NEXT (funlist);}

                  fprintf (global.outfile, "\n\n");
                  if (CWRAPPER_NEXT (arg_node) != NULL) {
                  PRINT_CONT (TRAVdo (CWRAPPER_NEXT (arg_node), arg_info),;);}
                  }
    );
  }

  DBUG_RETURN (arg_node);
}

#endif

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
    DBUG_ENTER ("PRTssacnt");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    INDENT;
    fprintf (global.outfile, " *  ");
    PRINT_POINTER_BRACKETS (global.outfile, arg_node);
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
 *   node *PRTcseinfo( node *arg_node, info *arg_info);
 *
 * description:
 *   Prints sets of available common subexpressions (debug only).
 *
 ******************************************************************************/

node *
PRTcseinfo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTcseinfo");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    /* to be implemented */

    if (CSEINFO_NEXT (arg_node) != NULL) {
        PRINT_CONT (TRAVdo (CSEINFO_NEXT (arg_node), arg_info), ;);
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
    bool do_it = FALSE;

    DBUG_ENTER ("PRTavis");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    /* to be implemented */

    DBUG_EXECUTE ("PRINT_AVIS", do_it = TRUE;);

    if (do_it) {
        fprintf (global.outfile, " /* AVIS:");

        fprintf (global.outfile, " TYPE   = %s,",
                 TYtype2String (AVIS_TYPE (arg_node), FALSE, 0));
        fprintf (global.outfile, " SSACNT = ");
        PRINT_POINTER_BRACKETS (global.outfile, AVIS_SSACOUNT (arg_node));
#if 1
        if (global.valid_ssaform && (AVIS_SSACOUNT (arg_node) != NULL)) {
            node *cnt = AVIS_SSACOUNT (arg_node);

            fprintf (global.outfile, " (baseid = %s, counter = %d)", SSACNT_BASEID (cnt),
                     SSACNT_COUNT (cnt));
        }
#endif

        fprintf (global.outfile, " */ ");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PRTinfo( node *arg_node, info *arg_info)
 *
 * description:
 *   N_info node found -> ERROR!!
 *
 ******************************************************************************/

node *
PRTinfo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTinfo");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    DBUG_ASSERT ((0), "N_info node found for printing!!");

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
    DBUG_ENTER ("PrintTrav");

    TRAVpush (TR_prt);
    global.indent = 0;

    if (global.compiler_phase == PH_genccode) {
        if (global.filetype == F_prog) {
            /*
             * The current file is a SAC program.
             * Therefore, the C file is generated within the target directory.
             */
            global.outfile = FMGRwriteOpen ("%s%s", global.targetdir, global.cfilename);
            CTInote ("Writing file \"%s%s\"", global.targetdir, global.cfilename);

            GSCprintFileHeader (syntax_tree);
            syntax_tree = TRAVdo (syntax_tree, arg_info);
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
        global.outfile = stdout;
        fprintf (global.outfile, "\n-----------------------------------------------\n");
        TRAVdo (syntax_tree, arg_info);
        fprintf (global.outfile, "\n-----------------------------------------------\n");
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
    DBUG_ENTER ("PrintTravPre");

    DBUG_PRINT ("PRINT_LINE", ("line (%s) %s:%i\n", NODE_TEXT (arg_node),
                               NODE_FILE (arg_node), NODE_LINE (arg_node)));

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
    DBUG_ENTER ("PrintTravPost");

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

    DBUG_ENTER ("Print");

    arg_info = MakeInfo ();
    /* we want to duplicate all sons */
    INFO_CONT (arg_info) = NULL;

    syntax_tree = PrintTRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

#if 0
  /* if generating c library, invoke the headerfile generator */
  if (global.genlib.c && (global.compiler_phase == PH_genccode)) {
    PrintInterface (syntax_tree);
  }
#endif

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

    DBUG_ENTER ("PRTdoPrintNode");

    arg_info = MakeInfo ();
    /* we want to duplicate all sons */
    INFO_CONT (arg_info) = syntax_tree;

    syntax_tree = PrintTRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
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
    DBUG_ENTER ("PRTdataflowgraph");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    global.outfile = stdout;

    fprintf (global.outfile, "****** Dataflowgraph begin ******\n");

    if (arg_node != NULL) {
        DBUG_ASSERT ((NODE_TYPE (arg_node) == N_dataflowgraph),
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
    DBUG_ENTER ("PRTdataflownode");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    global.outfile = stdout;

    if (arg_node != NULL) {
        DBUG_ASSERT ((NODE_TYPE (arg_node) == N_dataflownode),
                     "PrintDataflownode expects a N_dataflownode");

        fprintf (global.outfile, "%s: %s, REFCOUNT: %i", DATAFLOWNODE_NAME (arg_node),
                 MUTHLIBdecodeExecmode (DATAFLOWNODE_EXECMODE (arg_node)),
                 DATAFLOWNODE_REFCOUNT (arg_node));
        if (global.break_after == PH_multithread) {
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
 *   node *PRTerror( node *arg_node, info *arg_info)
 *
 * description:
 *   This function print all the errors
 *
 ******************************************************************************/

node *
PRTerror (node *arg_node, info *arg_info)
{
    bool firstError;

    DBUG_ENTER ("PRTerror");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    firstError = INFO_FIRSTERROR (arg_info);

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

    DBUG_RETURN (arg_node);
}

node *
PRTimport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTimport");

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

node *
PRTexport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTexport");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "export ");

    if (EXPORT_ALL (arg_node)) {
        fprintf (global.outfile, "all");
    } else {
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

node *
PRTuse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTuse");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "use %s : ", USE_MOD (arg_node));

    if (USE_ALL (arg_node)) {
        fprintf (global.outfile, "all");
    } else {
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

node *
PRTprovide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTprovide");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "provide ");

    if (PROVIDE_ALL (arg_node)) {
        fprintf (global.outfile, "all");
    } else {
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

node *
PRTsymbol (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRTsymbol");

    if (NODE_ERROR (arg_node) != NULL) {
        NODE_ERROR (arg_node) = TRAVdo (NODE_ERROR (arg_node), arg_info);
    }

    fprintf (global.outfile, "%s", SYMBOL_ID (arg_node));

    if (SYMBOL_NEXT (arg_node) != NULL) {
        fprintf (global.outfile, ", ");
        SYMBOL_NEXT (arg_node) = TRAVdo (SYMBOL_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
