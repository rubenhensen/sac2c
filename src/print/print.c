/*
 *
 * $Log$
 * Revision 3.147  2003/12/23 10:54:48  khf
 * Old syntax (output format 2) in PrintNwith, PrintNcode, PrintNpart removed.
 * Now only multigenerator syntax (internal syntax) available.
 * PrintNcode modified for printing NCODE_CEXPRS.
 * PrintNwithop modified for printing NWITHOP_NEXT.
 *
 * Revision 3.146  2003/11/14 15:41:17  sbs
 * minor bug eliminated.
 *
 * Revision 3.145  2003/11/12 11:58:05  sbs
 * optional default expression now is printed correctly 8-)
 *
 * Revision 3.144  2003/11/12 08:47:08  sbs
 * optional default expression is now printed as well
 *
 * Revision 3.143  2003/11/04 17:26:38  cg
 * Fixed a bug in printing/scanning of individual escape characters.
 * Escape characters are stored in OCTAL representation !!
 * So, scanning using atoi is NOT promising.
 *
 * Revision 3.142  2003/10/19 21:39:45  dkrHH
 * prf_string moved from print.[ch] to globals.[ch] (for BEtest)
 *
 * Revision 3.141  2003/10/15 12:25:48  dkrHH
 * indent warning deactivated for SPMD-funs only
 *
 * Revision 3.140  2003/09/25 10:54:39  dkr
 * to_unq() and from_unq() are prfs now
 *
 * Revision 3.139  2003/06/11 21:41:40  ktr
 * added support for multidimensional arrays
 *
 * Revision 3.138  2003/06/11 15:07:12  ktr
 * Quick-Fix of the bug by commenting out critical lines.
 *
 * Revision 3.137  2003/05/27 18:56:24  ktr
 * removed bug in PrintExprs. It was assumed that arg_info would always contain
 * an arg_info node.
 *
 * Revision 3.136  2003/05/23 14:51:34  ktr
 * Moved variable declarations before the DBUG_ENTER Macro.
 *
 * Revision 3.135  2003/05/22 01:10:32  ktr
 * printArray now prints multidimensional N_array nodes correctly :)
 *
 * Revision 3.134  2003/05/21 16:38:54  ktr
 * printArray now prints multidimensional N_array nodes.
 *
 * Revision 3.133  2003/04/04 17:01:02  sbs
 * prf_node_info.mac extended
 *
 * Revision 3.132  2002/10/31 19:46:47  dkr
 * DBUG-string PRINT_PTR added
 *
 * Revision 3.131  2002/10/30 16:25:36  dkr
 * no changes done
 *
 * Revision 3.130  2002/10/30 16:10:02  dkr
 * PrintAp(): wrapper annotation added
 *
 * Revision 3.129  2002/10/29 19:51:31  dkr
 * comment added
 *
 * Revision 3.128  2002/10/29 19:50:09  dkr
 * PrintAp() prints name of AP_FUNDEF if present!
 *
 * Revision 3.127  2002/10/21 18:43:30  sah
 * added debug flag PRINT_SYMBOL to disable printing
 * long prf identifiers and enable printing prf symbols
 * instead
 *
 * Revision 3.126  2002/10/21 17:43:13  sah
 * fixed missing { [ when printing index vector in PrintSetWL
 *
 * Revision 3.125  2002/10/18 13:32:50  sbs
 * accesses to ID_ATTRIB replaced by FLAGS.
 * Function PrintFlag added
 *
 * Revision 3.124  2002/10/10 23:50:56  dkr
 * ICM_STR added
 *
 * Revision 3.123  2002/10/10 11:21:35  sbs
 * Now, sel applied to other than two args will be printed correctly as well.
 *
 * Revision 3.122  2002/10/09 02:04:26  dkr
 * PrintNgen(): correct N_Nwithid used now 8-)
 *
 * Revision 3.121  2002/10/07 04:48:07  dkr
 * PrintAST(): NWITHOP_TYPE added
 *
 * Revision 3.120  2002/09/13 23:22:50  dkr
 * PrintLet(): bug fixed
 *
 * Revision 3.119  2002/09/13 22:11:01  dkr
 * ->lineno replaced by NODE_LINE
 *
 * Revision 3.118  2002/09/11 23:06:55  dkr
 * printing of PRFs modified, prf_node_info.mac modified.
 *
 * Revision 3.117  2002/09/09 19:31:40  dkr
 * prf_name_string added
 *
 * Revision 3.116  2002/09/09 11:52:11  dkr
 * PrintPrf(): PRF_ARGS may be NULL...
 *
 * Revision 3.115  2002/09/06 12:20:06  sah
 * handling of N_setwl nodes modified.
 *
 * Revision 3.114  2002/09/06 10:35:43  sah
 * added PrintSetWL
 *
 * Revision 3.113  2002/08/30 15:48:18  dkr
 * DoPrintAST: FUNDEF_RETURN added
 *
 * Revision 3.112  2002/08/30 10:50:05  dkr
 * PrintPrf: F_type_error added
 *
 * Revision 3.111  2002/08/12 14:59:50  sbs
 *  N_mop representation changed
 *
 * Revision 3.110  2002/08/09 16:36:21  sbs
 * basic support for N_mop written.
 *
 * Revision 3.109  2002/08/09 14:11:03  dkr
 * printing of wrapper functions with non-empty body added
 *
 * Revision 3.108  2002/08/09 10:00:52  dkr
 * DBUG-string PRINT_NTY added
 *
 * Revision 3.107  2002/08/06 07:51:47  sbs
 * PrintNtype and PrintDebugNtype made public 8-))
 *
 * Revision 3.106  2002/07/29 12:12:53  sbs
 * PRF_IF macro extended by z.
 *
 * Revision 3.105  2002/07/24 10:07:16  sah
 * changed printing of F_sel
 *
 * Revision 3.104  2002/07/12 18:29:23  dkr
 * indentation of ICMs corrected
 *
 * Revision 3.103  2002/07/10 19:25:07  dkr
 * several ICM_... types added and renamed
 *
 * Revision 3.102  2002/07/10 16:23:21  dkr
 * ICM_ANY added, ICM_VAR renamed into ICM_VARANY
 *
 * Revision 3.101  2002/07/04 11:15:43  dkr
 * PrintId: DBUG-string PRINT_NT added
 *
 * Revision 3.100  2002/07/03 17:11:58  dkr
 * ID_UNQCONV added again ...
 *
 * Revision 3.98  2002/07/03 12:04:31  sbs
 * when breaking after typecheck, the ntype attached to AVIS nodes
 * is printed as acomment to every vardec.
 *
 * Revision 3.96  2002/07/01 19:56:41  sah
 * PrintNgenerator now prints ". (NULL)" instead of "."
 * on NULL entries for generator boundaries in withloops.
 * Needed to distinguish between new N_dot nodes and NULL
 * entries.
 *
 * Revision 3.95  2002/07/01 12:54:09  dkr
 * ICM_ICM added
 *
 * Revision 3.94  2002/06/28 13:23:53  dkr
 * now, NOOP is no c-icm but a h-icm
 *
 * Revision 3.93  2002/06/28 12:53:25  sbs
 * PrintNtype and PrintDebugNtype included.
 * For the time being disabled, though 8-)
 *
 * Revision 3.92  2002/06/25 14:07:03  sbs
 * PrintDot added .
 *
 * Revision 3.91  2002/06/21 12:26:45  dkr
 * PrintAST: AVIS_SSACONST for N_id nodes added
 *
 * Revision 3.90  2002/06/07 15:49:49  dkr
 * printing of FUNDEF_ICM: \n added
 *
 * Revision 3.89  2002/06/07 15:46:12  dkr
 * printing of TYPEDEF_ICM: semicolon is now part of the ICM itself
 *
 * Revision 3.88  2002/06/07 14:39:04  dkr
 * mdb_argtag renamed into ATG_string
 *
 * Revision 3.87  2002/06/06 18:21:47  dkr
 * Now, ID_NT_TAG is printed in phase PH_genccode only
 *
 * Revision 3.84  2002/05/31 14:53:15  sbs
 * Now the wrapper functions are printed as well
 *
 * Revision 3.83  2002/04/16 21:19:35  dkr
 * Handling of main() function modified:
 * The main() function defined by the programmer is renamed in the same
 * way as a normal user-defined function.
 * GSCPrintMain() prints a new main() function which contains all the
 * initialization stuff.
 *
 * Revision 3.82  2002/04/16 11:05:42  dkr
 * PrintArgtab() and PrintAssign() modified
 *
 * Revision 3.81  2002/04/15 16:08:34  dkr
 * bug in PrintArgtab() fixed
 *
 * Revision 3.80  2002/04/09 16:34:55  dkr
 * debug output for FUNDEF_ARGTAB and AP_ARGTAB added
 *
 * Revision 3.79  2002/04/08 19:59:51  dkr
 * PrintAvis() and PrintSSAcnt() modified.
 * DBUG-strings PRINT_AVIS, PRINT_SSA added.
 *
 * Revision 3.78  2002/04/08 14:11:58  dkr
 * PrintFundef(): prototype of zombie funs in no longer printed
 *
 * Revision 3.77  2002/03/08 10:12:54  sbs
 * printing of xombie functions prevented from declaration section
 * printing of wrapper functions insrerted
 *
 * Revision 3.76  2002/03/07 21:27:55  dkr
 * - return statement is printed even for void-functions now
 * - some brushing done
 *
 * Revision 3.74  2002/03/01 03:20:50  dkr
 * support for ARGTABs added
 *
 * Revision 3.72  2002/02/12 15:45:02  dkr
 * DoPrintAST(): N_avis added
 *
 * [ eliminated ]
 *
 */

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "new_types.h"
#include "DupTree.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "DataFlowMask.h"
#include "generatemasks.h"
#include "filemgr.h"
#include "globals.h"
#include "gen_startup_code.h"
#include "WithloopFolding.h"
#include "scheduling.h"
#include "refcount.h"
#include "wl_bounds.h"
#include "wltransform.h"
#include "constants.h"
#include "print_interface.h"
#include "print.h"

#define WARN_INDENT

/*
 * This global variable is used to detect inside of PrintIcm() whether
 * the ICM is in assignment position or not.
 */
static node *last_assignment_icm = NULL;

/*
 * PrintNode(): INFO_PRINT_CONT(arg_info) contains the root of syntaxtree.
 *  -> traverses next-node if and only if its parent is not the root.
 * Print(): INFO_PRINT_CONT(arg_info) is NULL.
 *  -> traverses all next-nodes.
 *
 * This behaviour is implemented with the macro PRINT_CONT.
 */

#define PRINT_CONT(code_then, code_else)                                                 \
    if ((arg_info != NULL) && (INFO_PRINT_CONT (arg_info) == arg_node)) {                \
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
 * The selection of when to write a line pragma is actually not correctly
 * implemented because pragmas are now printed not only to the SIB but also
 * when stopping the compilation phase after having generated the SIB.
 * However, for the time being we keep this implementation for debugging
 * purposes.
 */
#define PRINT_LINE_PRAGMA_IN_SIB(file_handle, node)                                      \
    if (compiler_phase == PH_writesib) {                                                 \
        fprintf (file_handle, "# %d \"%s\"\n", NODE_LINE (node), NODE_FILE (node));      \
    }

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
#define ICM_DEF(prf, trf) extern void Print##prf (node *exprs, node *arg_info);
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

char *prf_symbol[] = {
#define PRF_IF(a, b, c, d, e, f, g, h) d
#include "prf_node_info.mac"
#undef PRF_IF
};

bool prf_is_infix[] = {
#define PRF_IF(a, b, c, d, e, f, g, h) e
#include "prf_node_info.mac"
#undef PRF_IF
};

#ifndef DBUG_OFF

/******************************************************************************
 *
 * function:
 *   void DbugPrintArray( node *arg_node)
 *
 * description:
 *   This function is only used for printing of debugging informations in
 *   PrintArray and PrintId.
 *
 ******************************************************************************/

static void
DbugPrintArray (node *arg_node)
{
    int *intptr, veclen, i;
    simpletype vectype;
    void *constvec;
    char *chrptr;
    float *fltptr;
    double *dblptr;

    DBUG_ENTER ("DbugPrintArray");

    if (NODE_TYPE (arg_node) == N_array) {
        veclen = ARRAY_VECLEN (arg_node);
        vectype = ARRAY_VECTYPE (arg_node);
        constvec = ARRAY_CONSTVEC (arg_node);
    } else {
        DBUG_ASSERT ((NODE_TYPE (arg_node) == N_id),
                     "argument is neither a N_array nor a N_id node");
        veclen = ID_VECLEN (arg_node);
        vectype = ID_VECTYPE (arg_node);
        constvec = ID_CONSTVEC (arg_node);
    }

    if (constvec != NULL) {
        fprintf (outfile, ":[");
    }

    if ((constvec != NULL) && (veclen >= 1)) {
        switch (vectype) {
        case T_nothing:
            break;

        case T_int:
            intptr = (int *)constvec;
            fprintf (outfile, "%d", intptr[0]);
            for (i = 1; i < ((veclen < 10) ? (veclen) : (10)); i++) {
                fprintf (outfile, ",%d", intptr[i]);
            }
            break;

        case T_float:
            fltptr = (float *)constvec;
            fprintf (outfile, "%f", fltptr[0]);
            for (i = 1; i < ((veclen < 10) ? (veclen) : (10)); i++) {
                fprintf (outfile, ",%f", fltptr[i]);
            }
            break;

        case T_double:
            dblptr = (double *)constvec;
            fprintf (outfile, "%f", dblptr[0]);
            for (i = 1; i < ((veclen < 10) ? (veclen) : (10)); i++) {
                fprintf (outfile, ",%f", dblptr[i]);
            }
            break;

        case T_bool:
            intptr = (int *)constvec;
            fprintf (outfile, "%s", ((intptr[0] == 0) ? ("false") : ("true")));
            for (i = 1; i < ((veclen < 10) ? (veclen) : (10)); i++) {
                fprintf (outfile, ",%s", ((intptr[i] == 0) ? ("false") : ("true")));
            }
            break;

        case T_char:
            chrptr = (char *)constvec;
            if ((chrptr[0] >= ' ') && (chrptr[0] <= '~') && (chrptr[0] != '\'')) {
                fprintf (outfile, ",'%c'", chrptr[0]);
            } else {
                fprintf (outfile, ",'\\%o'", chrptr[0]);
            }
            for (i = 1; i < ((veclen < 10) ? (veclen) : (10)); i++) {
                if ((chrptr[i] >= ' ') && (chrptr[i] <= '~') && (chrptr[i] != '\'')) {
                    fprintf (outfile, ",'%c'", chrptr[i]);
                } else {
                    fprintf (outfile, ",'\\%o'", chrptr[i]);
                }
            }
            break;

        default:
            DBUG_ASSERT ((0), "illegal type found!");
            break;
        }
    }

    if (constvec != NULL) {
        if (veclen > 10) {
            fprintf (outfile, ",..");
        }
        fprintf (outfile, "]");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void WLAAprintAccesses( node* arg_node, node* arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
WLAAprintAccesses (node *arg_node, node *arg_info)
{
    feature_t feature;
    int i, dim, iv;
    access_t *access;
    shpseg *offset;

    DBUG_ENTER ("WLAAprintAccesses");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Ncode), "Wrong node-type: N_Ncode exspected");

    feature = NCODE_WLAA_FEATURE (arg_node);
    fprintf (outfile, "/*\n");
    INDENT;
    fprintf (outfile, " * WITH-LOOP features:\n");

    INDENT;
    fprintf (outfile, " *   array: %s\n", VARDEC_NAME (NCODE_WLAA_WLARRAY (arg_node)));

    INDENT;
    fprintf (outfile, " *   index-var: %s\n",
             VARDEC_NAME (NCODE_WLAA_INDEXVAR (arg_node)));

    if (feature == FEATURE_NONE) {
        INDENT;
        fprintf (outfile, " *   no special features\n");
    }
    if ((feature & FEATURE_WL) == FEATURE_WL) {
        INDENT;
        fprintf (outfile, " *   with-loop containing array access(es)\n");
    }
    if ((feature & FEATURE_LOOP) == FEATURE_LOOP) {
        INDENT;
        fprintf (outfile, " *   while-/do-/for-loop containing array access(es)\n");
    }
    if ((feature & FEATURE_TAKE) == FEATURE_TAKE) {
        INDENT;
        fprintf (outfile, " *   primitive function take\n");
    }
    if ((feature & FEATURE_DROP) == FEATURE_DROP) {
        INDENT;
        fprintf (outfile, " *   primitive function drop\n");
    }
    if ((feature & FEATURE_AP) == FEATURE_AP) {
        INDENT;
        fprintf (outfile, " *   function aplication\n");
    }
    if ((feature & FEATURE_ASEL) == FEATURE_ASEL) {
        INDENT;
        fprintf (outfile, " *   primitive function sel with array return value\n");
    }
    if ((feature & FEATURE_MODA) == FEATURE_MODA) {
        INDENT;
        fprintf (outfile, " *   primitive function modarray\n");
    }
    if ((feature & FEATURE_CAT) == FEATURE_CAT) {
        INDENT;
        fprintf (outfile, " *   primitive function cat\n");
    }
    if ((feature & FEATURE_ROT) == FEATURE_ROT) {
        INDENT;
        fprintf (outfile, " *   primitive function rotate\n");
    }
    if ((feature & FEATURE_COND) == FEATURE_COND) {
        INDENT;
        fprintf (outfile, " *   conditional containing array access(es)\n");
    }
    if ((feature & FEATURE_AARI) == FEATURE_AARI) {
        INDENT;
        fprintf (outfile, " *   primitive arithmetic operation on arrays "
                          "(without index vector access)\n");
    }
    if ((feature & FEATURE_UNKNOWN) == FEATURE_UNKNOWN) {
        INDENT;
        fprintf (outfile, " *   primitive function sel with unknown indexvector\n");
    }

    INDENT;
    fprintf (outfile, " *\n");

    dim = SHP_SEG_SIZE;
    access = NCODE_WLAA_ACCESS (arg_node);
    INDENT;
    fprintf (outfile, " * WLAA:\n");
    do {
        if (access == NULL) {
            INDENT;
            fprintf (outfile, " *   No accesses! \n");
        } else {
            dim = VARDEC_OR_ARG_DIM (ACCESS_ARRAY (access));
            iv = 0;
            offset = ACCESS_OFFSET (access);
            INDENT;
            fprintf (outfile, " *   %s ", ACLT (ACCESS_CLASS (access)));

            switch (ACCESS_CLASS (access)) {
            case ACL_irregular:
            /* here's no break missing ! */
            case ACL_unknown:
                fprintf (outfile, "sel( %s ", VARDEC_NAME (ACCESS_IV (access)));
                fprintf (outfile, ", %s)", VARDEC_NAME (ACCESS_ARRAY (access)));
                fprintf (outfile, "\n");
                access = ACCESS_NEXT (access);
                break;

            case ACL_offset:
                if (offset == NULL)
                    fprintf (outfile, "no offset\n");
                else {
                    do {
                        if (ACCESS_DIR (access) == ADIR_read) {
                            fprintf (outfile, "read ( %s + [ %d",
                                     VARDEC_NAME (ACCESS_IV (access)),
                                     SHPSEG_SHAPE (offset, 0));
                        } else {
                            fprintf (outfile, "write( %s + [ %d",
                                     VARDEC_NAME (ACCESS_IV (access)),
                                     SHPSEG_SHAPE (offset, 0));
                        }
                        for (i = 1; i < dim; i++)
                            fprintf (outfile, ",%d", SHPSEG_SHAPE (offset, i));
                        fprintf (outfile, " ], %s)\n",
                                 STR_OR_UNKNOWN (VARDEC_NAME (ACCESS_ARRAY (access))));
                        offset = SHPSEG_NEXT (offset);
                    } while (offset != NULL);
                }
                access = ACCESS_NEXT (access);
                break;

            case ACL_const:
                if (offset == NULL)
                    fprintf (outfile, "no offset\n");
                else {
                    do {
                        if (ACCESS_DIR (access) == ADIR_read) {
                            fprintf (outfile, "read ( [ %d", SHPSEG_SHAPE (offset, 0));
                        } else {
                            fprintf (outfile, "write( [ %d", SHPSEG_SHAPE (offset, 0));
                        }
                        for (i = 1; i < dim; i++) {
                            fprintf (outfile, ",%d", SHPSEG_SHAPE (offset, i));
                        }
                        fprintf (outfile, " ], %s)\n",
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
    fprintf (outfile, " */\n");
    INDENT;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void TSIprintInfo( node* arg_node, node* arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
TSIprintInfo (node *arg_node, node *arg_info)
{
    int count, iter, dim, i, tilesize;
    node *pragma, *aelems;
    char *ap_name;

    DBUG_ENTER ("TSIprintInfo");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Ncode), "Wrong node-type: N_Ncode exspected");

    count = 0;
    iter = 0;
    dim = NCODE_WLAA_ARRAYDIM (arg_node);
    aelems = NULL;
    if (NCODE_TSI_TILESHP (arg_node) == NULL) {
        pragma = NULL;
    } else {
        pragma = MakePragma ();
        for (i = dim - 1; i >= 0; i--) {
            tilesize = SHPSEG_SHAPE (NCODE_TSI_TILESHP (arg_node), i);
            aelems = MakeExprs (MakeNum (tilesize), aelems);
        }
        ap_name = Malloc (6 * sizeof (char));
        ap_name = strcpy (ap_name, "BvL0");
        PRAGMA_WLCOMP_APS (pragma)
          = MakeExprs (MakeAp (ap_name, NULL, MakeExprs (MakeFlatArray (aelems), NULL)),
                       NULL);
    }

    fprintf (outfile, "/*\n");
    INDENT;
    fprintf (outfile, " * TSI-Infos:\n");
    INDENT;
    fprintf (outfile, " *   Number of relevant accesses: %d\n", count);
    INDENT;
    fprintf (outfile, " *   Number of iterations: %d\n", iter);
    INDENT;
    fprintf (outfile, " *\n");
    INDENT;
    fprintf (outfile, " * TSI-proposal:\n");
    INDENT;
    if (pragma != NULL) {
        fprintf (outfile, " *    ");
        FreePragma (pragma, NULL);
        fprintf (outfile, "\n");
        INDENT;
    } else {
        fprintf (outfile, " *   No proposal possible!\n");
        INDENT;
    }
    fprintf (outfile, " */\n");
    INDENT;

    DBUG_VOID_RETURN;
}

#endif /* ! DBUG_OFF */

/******************************************************************************
 *
 * Function:
 *   void PrintFlag( char *flag, bool do_it)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrintFlag (char *flag, bool do_it)
{
    DBUG_ENTER ("PrintFlag");

    DBUG_EXECUTE ("PRINT_FLAGS", do_it = TRUE;);

    if (do_it) {
        fprintf (outfile, ": %s", flag);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PrintStatus( statustype status, bool do_it)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrintStatus (statustype status, bool do_it)
{
    DBUG_ENTER ("PrintStatus");

    DBUG_EXECUTE ("PRINT_STATUS", do_it = TRUE;);

    if (do_it) {
        fprintf (outfile, ":");
        if (status != ST_regular) {
            fprintf (outfile, "%s", mdb_statustype[status]);
        } else {
            fprintf (outfile, "-");
        }
    }

    DBUG_VOID_RETURN;
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
PrintArgtab (argtab_t *argtab, bool is_def)
{
    int i;

    DBUG_ENTER ("PrintArgtab");

    if (argtab != NULL) {
        fprintf (outfile, "[");
        for (i = 0; i < argtab->size; i++) {
            if (argtab->tag[i] != ATG_notag) {
                fprintf (outfile, " %s:", ATG_string[argtab->tag[i]]);

                if (argtab->ptr_in[i] != NULL) {
                    PRINT_POINTER_BRACKETS (outfile, argtab->ptr_in[i]);
                    if (is_def) {
                        DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_arg),
                                     "illegal argtab entry found!");

                        if (ARG_NAME (argtab->ptr_in[i]) != NULL) {
                            fprintf (outfile, "%s", ARG_NAME (argtab->ptr_in[i]));
                        }
                    } else {
                        DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_exprs),
                                     "illegal argtab entry found!");

                        fprintf (outfile, "%s",
                                 mdb_nodetype[NODE_TYPE (
                                   EXPRS_EXPR (argtab->ptr_in[i]))]);
                    }
                } else {
                    fprintf (outfile, "-");
                }

                fprintf (outfile, "/");

                if (argtab->ptr_out[i] != NULL) {
                    PRINT_POINTER_BRACKETS (outfile, argtab->ptr_out[i]);
                    if (is_def) {
                    } else {
                        fprintf (outfile, "%s",
                                 STR_OR_EMPTY (IDS_NAME (((ids *)argtab->ptr_out[i]))));
                    }
                } else {
                    fprintf (outfile, "-");
                }
            } else {
                DBUG_ASSERT ((argtab->ptr_in[i] == NULL), "illegal argtab entry found!");
                DBUG_ASSERT ((argtab->ptr_out[i] == NULL), "illegal argtab entry found!");

                fprintf (outfile, " ---");
            }

            if (i < argtab->size - 1) {
                fprintf (outfile, ",");
            }
        }
        fprintf (outfile, " ]");
    } else {
        fprintf (outfile, "-");
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
    types *rettypes = NULL;
    node *args = NULL;

    DBUG_ENTER ("Argtab2Fundef");

    argtab = FUNDEF_ARGTAB (fundef);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent");

    if (argtab->ptr_out[0] == NULL) {
        rettypes = MakeTypes1 (T_void);
    } else {
        rettypes = DupOneTypes (argtab->ptr_out[0]);
    }

    for (i = argtab->size - 1; i >= 1; i--) {
        if (argtab->ptr_in[i] != NULL) {
            node *arg = DupNode (argtab->ptr_in[i]);
            ARG_NEXT (arg) = args;
            args = arg;
        } else if (argtab->ptr_out[i] != NULL) {
            args = MakeArg (NULL, DupOneTypes (argtab->ptr_out[i]), ST_regular,
                            ST_regular, args);
        }
    }

    new_fundef = MakeFundef (StringCopy (FUNDEF_NAME (fundef)), FUNDEF_MOD (fundef),
                             rettypes, args, NULL, NULL);

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
    ids *_ids = NULL;
    node *exprs = NULL;

    DBUG_ENTER ("Argtab2Let");

    argtab = AP_ARGTAB (ap);
    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent");

    if (argtab->ptr_out[0] != NULL) {
        _ids = DupOneIds (argtab->ptr_out[0]);
    }

    for (i = argtab->size - 1; i >= 1; i--) {
        if (argtab->ptr_out[i] != NULL) {
            exprs = MakeExprs (DupIds_Id (argtab->ptr_out[i]), exprs);
        } else if (argtab->ptr_in[i] != NULL) {
            node *expr = DupNode (argtab->ptr_in[i]);
            EXPRS_NEXT (expr) = exprs;
            exprs = expr;
        }
    }

    new_ap = MakeAp (StringCopy (AP_NAME (ap)), AP_MOD (ap), exprs);
    AP_FUNDEF (new_ap) = AP_FUNDEF (ap);

    new_let = MakeLet (new_ap, _ids);

    DBUG_RETURN (new_let);
}

/******************************************************************************
 *
 * Function:
 *   void PrintArgtags( argtab_t *argtab)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrintArgtags (argtab_t *argtab)
{
    int i;

    DBUG_ENTER ("PrintArgtags");

    fprintf (outfile, " /*");

    /* return value */
    if (argtab->tag[0] != ATG_notag) {
        DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent");
        fprintf (outfile, " %s", ATG_string[argtab->tag[0]]);
    }

    fprintf (outfile, " <-");

    /* arguments */
    for (i = 1; i < argtab->size; i++) {
        DBUG_ASSERT ((argtab->tag[i] != ATG_notag), "argtab is uncompressed");
        fprintf (outfile, " %s", ATG_string[argtab->tag[i]]);
    }

    fprintf (outfile, " */ ");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PrintRC( int rc, int nrc, bool do_it)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrintRC (int rc, int nrc, bool do_it)
{
    DBUG_ENTER ("PrintRC");

    DBUG_EXECUTE ("PRINT_RC", do_it = TRUE;);

    if (do_it) {
        if (rc >= 0) {
            fprintf (outfile, ":%d", rc);
        } else if (rc == RC_UNDEF) {
            fprintf (outfile, ":?");
        } else if (rc == RC_INACTIVE) {
            /* no output */
        } else {
            DBUG_ASSERT ((0), "illegal RC value found!");
        }
    }

    DBUG_EXECUTE ("PRINT_NRC",
                  if ((nrc >= 0) && do_it) { fprintf (outfile, "::%d", nrc); });

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PrintIds( ids *arg, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrintIds (ids *arg, node *arg_info)
{
    DBUG_ENTER ("PrintIds");

    if (arg != NULL) {
        DBUG_PRINT ("PRINT", ("%s", IDS_NAME (arg)));

        if (IDS_MOD (arg) != NULL) {
            fprintf (outfile, "%s:", IDS_MOD (arg));
        }

        fprintf (outfile, "%s", IDS_NAME (arg));

        PrintStatus (IDS_ATTRIB (arg), FALSE);
        PrintStatus (IDS_STATUS (arg), FALSE);

        PrintRC (IDS_REFCNT (arg), IDS_NAIVE_REFCNT (arg), show_refcnt);

        if (show_idx && IDS_USE (arg)) {
            Trav (IDS_USE (arg), arg_info);
        }
        if (NULL != IDS_NEXT (arg)) {
            fprintf (outfile, ", ");
        }

        PrintIds (IDS_NEXT (arg), arg_info);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *PrintModul( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintModul (node *arg_node, node *arg_info)
{
    char *type_str;

    DBUG_ENTER ("PrintModul");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if (INFO_PRINT_SEPARATE (arg_info)) {
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
        outfile = WriteOpen ("%s/header.h", tmp_dirname);
        GSCPrintFileHeader (arg_node);

        if (NULL != MODUL_TYPES (arg_node)) {
            fprintf (outfile, "\n\n");
            /* print typedefs */
            Trav (MODUL_TYPES (arg_node), arg_info);
        }

        GSCPrintDefines ();

        if (NULL != MODUL_FUNS (arg_node)) {
            fprintf (outfile, "\n\n");
            INFO_PRINT_PROTOTYPE (arg_info) = TRUE;
            /* print function declarations */
            Trav (MODUL_FUNS (arg_node), arg_info);
            INFO_PRINT_PROTOTYPE (arg_info) = FALSE;
        }

        if (NULL != MODUL_OBJS (arg_node)) {
            fprintf (outfile, "\n\n");
            print_objdef_for_header_file = TRUE;
            /* print object declarations */
            Trav (MODUL_OBJS (arg_node), arg_info);
        }

        fclose (outfile);

        outfile = WriteOpen ("%s/globals.c", tmp_dirname);
        fprintf (outfile, "#include \"header.h\"\n\n");
        fprintf (outfile,
                 "int SAC__%s__dummy_value_which_is_completely_useless"
                 " = 0;\n\n",
                 MODUL_NAME (arg_node));

        if (NULL != MODUL_OBJS (arg_node)) {
            fprintf (outfile, "\n\n");
            print_objdef_for_header_file = FALSE;
            /* print object definitions */
            Trav (MODUL_OBJS (arg_node), arg_info);
        }

        /*
         *  Maybe there's nothing to compile in this module because all functions
         *  deal with shape-independent arrays which are removed after writing
         *  the SIB.
         *
         *  Unfortunately, things are not as easy as they should be.
         *  If there is no symbol declared in this file then gcc creates an
         *  object file which does not contain any objects. This causes ranlib
         *  not (!!) to produce an empty symbol table, but to produce no symbol
         *  table at all. Finding no symbol table lets the linker give some
         *  nasty warnings. These are suppressed by the above dummy symbol.
         */

        fclose (outfile);

        if (NULL != MODUL_FUNS (arg_node)) {
            Trav (MODUL_FUNS (arg_node), arg_info); /* print function definitions */
                                                    /*
                                                     * Note that in this case a separate file is created for each function.
                                                     * These files are opened and closed in PrintFundef().
                                                     */
        }
    } else {
        switch (MODUL_FILETYPE (arg_node)) {
        case F_modimp:
            fprintf (outfile,
                     "\n"
                     "/*\n"
                     " *  Module %s :\n"
                     " */\n",
                     MODUL_NAME (arg_node));
            break;
        case F_classimp:
            fprintf (outfile,
                     "\n"
                     "/*\n"
                     " *  Class %s :\n",
                     MODUL_NAME (arg_node));
            if (MODUL_CLASSTYPE (arg_node) != NULL) {
                type_str = Type2String (MODUL_CLASSTYPE (arg_node), 0, TRUE);
                fprintf (outfile, " *  classtype %s;\n", type_str);
                type_str = Free (type_str);
            }
            fprintf (outfile, " */\n");
            break;
        case F_prog:
            fprintf (outfile,
                     "\n"
                     "/*\n"
                     " *  SAC-Program %s :\n"
                     " */\n",
                     puresacfilename);
            break;
        default:
            break;
        }

        if (MODUL_IMPORTS (arg_node) != NULL) {
            fprintf (outfile, "\n");
            /* print import-list */
            Trav (MODUL_IMPORTS (arg_node), arg_info);
        }

        if (MODUL_TYPES (arg_node) != NULL) {
            fprintf (outfile, "\n\n"
                              "/*\n"
                              " *  type definitions\n"
                              " */\n\n");
            /* print typedefs */
            Trav (MODUL_TYPES (arg_node), arg_info);
        }

        GSCPrintDefines ();

        if (MODUL_FUNS (arg_node) != NULL) {
            fprintf (outfile, "\n\n"
                              "/*\n"
                              " *  function declarations\n"
                              " */\n\n");
            INFO_PRINT_PROTOTYPE (arg_info) = TRUE;
            /* print function declarations */
            Trav (MODUL_FUNS (arg_node), arg_info);
            INFO_PRINT_PROTOTYPE (arg_info) = FALSE;
        }

        if (MODUL_OBJS (arg_node) != NULL) {
            fprintf (outfile, "\n\n"
                              "/*\n"
                              " *  global objects\n"
                              " */\n\n");
            /* print objdefs */
            Trav (MODUL_OBJS (arg_node), arg_info);
        }

        if (MODUL_FUNS (arg_node) != NULL) {
            fprintf (outfile, "\n\n"
                              "/*\n"
                              " *  function definitions\n"
                              " */\n\n");
            /* print function definitions */
            Trav (MODUL_FUNS (arg_node), arg_info);
        }

        DBUG_EXECUTE ("PRINT_CWRAPPER", if (MODUL_CWRAPPER (arg_node) != NULL) {
            fprintf (outfile, "\n\n"
                              "/*\n"
                              " *  c wrapper functions\n"
                              " */\n\n");
            /* print wrapper mappings */
            Trav (MODUL_CWRAPPER (arg_node), arg_info);
        });
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintImplist( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintImplist (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintImplist");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    fprintf (outfile, "import %s: ", IMPLIST_NAME (arg_node));

    if ((IMPLIST_ITYPES (arg_node) == NULL) && (IMPLIST_ETYPES (arg_node) == NULL)
        && (IMPLIST_FUNS (arg_node) == NULL) && (IMPLIST_OBJS (arg_node) == NULL)) {
        fprintf (outfile, "all;\n");
    } else {
        fprintf (outfile, "{");
        if (IMPLIST_ITYPES (arg_node) != NULL) {
            fprintf (outfile, "\n  implicit types: ");
            PrintIds (IMPLIST_ITYPES (arg_node), arg_info);
            fprintf (outfile, "; ");
        }
        if (IMPLIST_ETYPES (arg_node) != NULL) {
            fprintf (outfile, "\n  explicit types: ");
            PrintIds (IMPLIST_ETYPES (arg_node), arg_info);
            fprintf (outfile, "; ");
        }
        if (IMPLIST_OBJS (arg_node) != NULL) {
            fprintf (outfile, "\n  global objects: ");
            PrintIds (IMPLIST_OBJS (arg_node), arg_info);
            fprintf (outfile, "; ");
        }
        if (IMPLIST_FUNS (arg_node) != NULL) {
            fprintf (outfile, "\n  funs: ");
            PrintIds (IMPLIST_FUNS (arg_node), arg_info);
            fprintf (outfile, "; ");
        }
        fprintf (outfile, "\n}\n");
    }

    if (IMPLIST_NEXT (arg_node) != NULL) {
        /* print further imports */
        PRINT_CONT (Trav (IMPLIST_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintTypedef( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintTypedef (node *arg_node, node *arg_info)
{
    char *type_str;

    DBUG_ENTER ("PrintTypedef");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if ((TYPEDEF_ICM (arg_node) == NULL)
        || (NODE_TYPE (TYPEDEF_ICM (arg_node)) != N_icm)) {
        type_str = Type2String (TYPEDEF_TYPE (arg_node), 0, TRUE);
        fprintf (outfile, "typedef %s ", type_str);
        type_str = Free (type_str);

        if (TYPEDEF_MOD (arg_node) != NULL) {
            fprintf (outfile, "%s:", TYPEDEF_MOD (arg_node));
        }
        fprintf (outfile, "%s", TYPEDEF_NAME (arg_node));

        PrintStatus (TYPEDEF_ATTRIB (arg_node), FALSE);
        PrintStatus (TYPEDEF_STATUS (arg_node), FALSE);

        fprintf (outfile, ";\n");
    } else {
        Trav (TYPEDEF_ICM (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    if (TYPEDEF_COPYFUN (arg_node) != NULL) {
        fprintf (outfile, "\nextern %s %s( %s);\n", TYPEDEF_NAME (arg_node),
                 TYPEDEF_COPYFUN (arg_node), TYPEDEF_NAME (arg_node));
    }
    if (TYPEDEF_FREEFUN (arg_node) != NULL) {
        fprintf (outfile, "extern void %s( %s);\n\n", TYPEDEF_FREEFUN (arg_node),
                 TYPEDEF_NAME (arg_node));
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (TYPEDEF_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintObjdef( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintObjdef (node *arg_node, node *arg_info)
{
    char *type_str;

    DBUG_ENTER ("PrintObjdef");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    /* print objinit flag declaration in header.file
     * this has to placed before the if switch, because the ICM_Trav seems
     * to leave with no return...
     */
    if ((generatelibrary & GENERATELIBRARY_C) && (print_objdef_for_header_file)) {
        fprintf (outfile,
                 "/* flag, if object has been initialized */\n"
                 "extern bool SAC_INIT_FLAG_%s;\n",
                 OBJDEF_NAME (arg_node));
    }

    if ((OBJDEF_ICM (arg_node) == NULL) || (NODE_TYPE (OBJDEF_ICM (arg_node)) != N_icm)) {
        if ((OBJDEF_STATUS (arg_node) == ST_imported_mod)
            || (OBJDEF_STATUS (arg_node) == ST_imported_class)
            || print_objdef_for_header_file) {
            fprintf (outfile, "extern ");
        }

        type_str = Type2String (OBJDEF_TYPE (arg_node), 0, TRUE);
        fprintf (outfile, "%s ", type_str);
        type_str = Free (type_str);

        if (OBJDEF_MOD (arg_node) != NULL) {
            fprintf (outfile, "%s:", OBJDEF_MOD (arg_node));
        }

        fprintf (outfile, "%s", OBJDEF_NAME (arg_node));

        PrintStatus (OBJDEF_ATTRIB (arg_node), FALSE);
        PrintStatus (OBJDEF_STATUS (arg_node), FALSE);

        if (OBJDEF_EXPR (arg_node) != NULL) {
            fprintf (outfile, " = ");
            Trav (OBJDEF_EXPR (arg_node), arg_info);
        }

        fprintf (outfile, ";\n");

        if (OBJDEF_PRAGMA (arg_node) != NULL) {
            Trav (OBJDEF_PRAGMA (arg_node), arg_info);
        }

        fprintf (outfile, "\n");
    } else {
        Trav (OBJDEF_ICM (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (OBJDEF_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   void PrintFunctionHeader( node *arg_node, node *arg_info, bool in_comment)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
PrintFunctionHeader (node *arg_node, node *arg_info, bool in_comment)
{
    types *ret_types;
    char *type_str;
    bool print_sac = TRUE;
    bool print_c = FALSE;
    bool print_argtab = FALSE;

    DBUG_ENTER ("PrintFunctionHeader");

    DBUG_EXECUTE ("PRINT_PTR", fprintf (outfile, "/* " F_PTR " */\n", arg_node););

    PRINT_LINE_PRAGMA_IN_SIB (outfile, arg_node);

    if (FUNDEF_ARGTAB (arg_node) != NULL) {
        print_sac = FALSE;
        print_c = TRUE;

        DBUG_EXECUTE ("PRINT_ARGTAB", fprintf (outfile, "/* \n"); INDENT;
                      fprintf (outfile, " *  ");
                      PrintArgtab (FUNDEF_ARGTAB (arg_node), TRUE);
                      fprintf (outfile, "  */\n"); INDENT; print_sac = TRUE;
                      print_argtab = TRUE;);
    }

    if (FUNDEF_INLINE (arg_node)) {
        fprintf (outfile, "inline ");
    }

    if (print_c) {
        node *tmp = Argtab2Fundef (arg_node);

        PrintFunctionHeader (tmp, arg_info, in_comment);
        tmp = FreeZombie (FreeTree (tmp));

        fprintf (outfile, " ");
        if (!print_argtab) {
            PrintArgtags (FUNDEF_ARGTAB (arg_node));
        }
    }

    if (print_sac) {
        if (print_c) {
            fprintf (outfile, "\n");
            INDENT;
            fprintf (outfile, "/*  ");
        }

        ret_types = FUNDEF_TYPES (arg_node);
        while (ret_types != NULL) {
            type_str = Type2String (ret_types, 0, FALSE);
            fprintf (outfile, "%s", type_str);
            type_str = Free (type_str);

            PrintStatus (TYPES_STATUS (FUNDEF_TYPES (arg_node)), FALSE);

            ret_types = TYPES_NEXT (ret_types);
            if (ret_types != NULL) {
                fprintf (outfile, ", ");
            }
        }

        fprintf (outfile, " ");

        if (FUNDEF_MOD (arg_node) != NULL) {
            fprintf (outfile, "%s:", FUNDEF_MOD (arg_node));
        }

        if (FUNDEF_NAME (arg_node) != NULL) {
            fprintf (outfile, "%s", FUNDEF_NAME (arg_node));
        }

        PrintStatus (FUNDEF_ATTRIB (arg_node), FALSE);
        PrintStatus (FUNDEF_STATUS (arg_node), FALSE);

        fprintf (outfile, "(");

        if (FUNDEF_ARGS (arg_node) != NULL) {
            Trav (FUNDEF_ARGS (arg_node), arg_info); /* print args of function */
        }

        fprintf (outfile, ")");

        if (print_c) {
            fprintf (outfile, "\n");
            INDENT;
            fprintf (outfile, " */ ");
        }
    }

    /* Now, we print the new type signature, iff present */
    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, (in_comment) ? " *\n" : "/*\n");
    fprintf (outfile, " *  ");
    if (FUNDEF_NAME (arg_node) != NULL) {
        fprintf (outfile, "%s :: ", FUNDEF_NAME (arg_node));
        if (FUNDEF_TYPE (arg_node) != NULL) {
            char *(*t2s_fun) (ntype *, bool, int);
            t2s_fun = TYType2String;
            DBUG_EXECUTE ("PRINT_NTY", t2s_fun = TYType2DebugString;);

            fprintf (outfile, "%s\n",
                     t2s_fun (FUNDEF_TYPE (arg_node), TRUE,
                              indent + strlen (FUNDEF_NAME (arg_node)) + 8));
        }
    }
    INDENT;
    fprintf (outfile, (in_comment) ? " *" : " */");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *PrintFundef( node *arg_node, node *arg_info)
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
PrintFundef (node *arg_node, node *arg_info)
{
    int old_indent = indent;

    DBUG_ENTER ("PrintFundef");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    INFO_PRINT_VARNO (arg_info) = FUNDEF_VARNO (arg_node);

    /*
     * needed for the introduction of PROFILE_... MACROS in the
     *  function body.
     */
    INFO_PRINT_FUNDEF (arg_info) = arg_node;

    if (INFO_PRINT_PROTOTYPE (arg_info)) {
        /*
         * print function declaration
         */

        if ((FUNDEF_STATUS (arg_node) != ST_zombiefun)
            && (FUNDEF_STATUS (arg_node) != ST_wrapperfun)
            && (FUNDEF_STATUS (arg_node) != ST_spmdfun)) {
            if ((FUNDEF_BODY (arg_node) == NULL)
                || ((FUNDEF_RETURN (arg_node) != NULL)
                    && (NODE_TYPE (FUNDEF_RETURN (arg_node)) == N_icm))) {
                fprintf (outfile, "extern ");

                if ((FUNDEF_ICM (arg_node) == NULL)
                    || (NODE_TYPE (FUNDEF_ICM (arg_node)) != N_icm)) {
                    PrintFunctionHeader (arg_node, arg_info, FALSE);
                } else {
                    /* print N_icm ND_FUN_DEC */
                    fprintf (outfile, "\n");
                    Trav (FUNDEF_ICM (arg_node), arg_info);
                }

                fprintf (outfile, ";\n");

                if (FUNDEF_PRAGMA (arg_node) != NULL) {
                    Trav (FUNDEF_PRAGMA (arg_node), arg_info);
                }

                fprintf (outfile, "\n");
            }
        }
    } else {
        /*
         * print function definition
         */

        if (FUNDEF_STATUS (arg_node) == ST_zombiefun) {
            if (compiler_phase < PH_genccode) {
                fprintf (outfile, "/*\n");
                INDENT;
                fprintf (outfile, " * zombie function:\n");
                INDENT;
                fprintf (outfile, " *   ");
                PrintFunctionHeader (arg_node, arg_info, FALSE);
                fprintf (outfile, "\n");
                INDENT;
                fprintf (outfile, " */\n\n");
            } else {
                /*
                 * do not print zombie code in header files,
                 * do not generate separate files
                 */
            }
        } else {

            if (FUNDEF_STATUS (arg_node) == ST_wrapperfun) {
                if (FUNDEF_BODY (arg_node) == NULL) {
                    fprintf (outfile, "/*\n");
                    INDENT;
                    fprintf (outfile, " * wrapper function:\n");
                    INDENT;
                    fprintf (outfile, " *   ");
                    PrintFunctionHeader (arg_node, arg_info, TRUE);
                    fprintf (outfile, "\n");
                    INDENT;
                    fprintf (outfile, " */\n\n");
                } else {
                    fprintf (outfile, "/* wrapper function */\n");
                }
            } else if ((FUNDEF_STATUS (arg_node) == ST_condfun)
                       || (FUNDEF_STATUS (arg_node) == ST_dofun)
                       || (FUNDEF_STATUS (arg_node) == ST_whilefun)) {
                fprintf (outfile, "/* LaC function */\n");
            }

            if (FUNDEF_BODY (arg_node) != NULL) {
                if (INFO_PRINT_SEPARATE (arg_info)) {
                    outfile = WriteOpen ("%s/fun%d.c", tmp_dirname, function_counter);
                    fprintf (outfile, "#include \"header.h\"\n\n");
                }

                if ((FUNDEF_STATUS (arg_node) == ST_spmdfun)
                    && (compiler_phase == PH_genccode)) {
                    fprintf (outfile, "#if SAC_DO_MULTITHREAD\n\n");
                }

                DBUG_EXECUTE ("PRINT_MASKS",
                              fprintf (outfile, "\n**MASKS - function: \n");
                              PrintDefMask (outfile, FUNDEF_DEFMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info));
                              PrintUseMask (outfile, FUNDEF_USEMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info)););

                if ((FUNDEF_ICM (arg_node) == NULL)
                    || (NODE_TYPE (FUNDEF_ICM (arg_node)) != N_icm)) {
                    PrintFunctionHeader (arg_node, arg_info, FALSE);
                } else {
                    /* print N_icm ND_FUN_DEC */
                    Trav (FUNDEF_ICM (arg_node), arg_info);
                }

                fprintf (outfile, "\n");

                /* traverse function body */
                Trav (FUNDEF_BODY (arg_node), arg_info);

                if (FUNDEF_PRAGMA (arg_node) != NULL) {
                    Trav (FUNDEF_PRAGMA (arg_node), arg_info);
                }

                if ((FUNDEF_STATUS (arg_node) == ST_spmdfun)
                    && (compiler_phase == PH_genccode)) {
                    fprintf (outfile, "\n#endif  /* SAC_DO_MULTITHREAD */\n\n");
                } else {
                    fprintf (outfile, "\n\n");
                }

                if (INFO_PRINT_SEPARATE (arg_info)) {
                    fclose (outfile);
                    function_counter++;
                }
            }
        }
    }

    if (indent != old_indent) {
#ifdef WARN_INDENT
        if (FUNDEF_STATUS (INFO_PRINT_FUNDEF (arg_info)) != ST_spmdfun) {
            /*
             * for the time being, indentation of MT funs is always unbalanced :-(
             */
            SYSWARN (("Indentation unbalanced while printing function %s."
                      " Indentation at beginning of function: %i."
                      " Indentation at end of function: %i",
                      FUNDEF_NAME (arg_node), old_indent, indent));
        }
#endif
        indent = old_indent;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) { /* traverse next function */
        PRINT_CONT (Trav (FUNDEF_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintAnnotate( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintAnnotate (node *arg_node, node *arg_info)
{
    static char strbuffer1[256];
    static char strbuffer2[256];

    DBUG_ENTER ("PrintAnnotate");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

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

    fprintf (outfile, "%s;", strbuffer1);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintArg( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintArg (node *arg_node, node *arg_info)
{
    char *type_str;

    DBUG_ENTER ("PrintArg");

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, " **%d:", ARG_VARNO (arg_node)););

    type_str = Type2String (ARG_TYPE (arg_node), 0, TRUE);
    fprintf (outfile, " %s", type_str);
    type_str = Free (type_str);

    if (ARG_ATTRIB (arg_node) == ST_reference) {
        fprintf (outfile, " &");
    } else if (ARG_ATTRIB (arg_node) == ST_readonly_reference) {
        fprintf (outfile, " (&)");
    }

    if ((!INFO_PRINT_OMIT_FORMAL_PARAMS (arg_info)) && (ARG_NAME (arg_node) != NULL)) {
        fprintf (outfile, " %s", ARG_NAME (arg_node));
    }

    PrintStatus (ARG_ATTRIB (arg_node), FALSE);
    PrintStatus (ARG_STATUS (arg_node), FALSE);

    PrintRC (ARG_REFCNT (arg_node), ARG_NAIVE_REFCNT (arg_node), show_refcnt);

    if (ARG_COLCHN (arg_node) && show_idx) {
        Trav (ARG_COLCHN (arg_node), arg_info);
    }

    Trav (ARG_AVIS (arg_node), arg_info);

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",");
        PRINT_CONT (Trav (ARG_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintVardec( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintVardec (node *arg_node, node *arg_info)
{
    char *type_str;

    DBUG_ENTER ("PrintVardec");

    INDENT;

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**%d: ", VARDEC_VARNO (arg_node)););

    if ((VARDEC_ICM (arg_node) == NULL) || (NODE_TYPE (VARDEC_ICM (arg_node)) != N_icm)) {
        type_str = Type2String (VARDEC_TYPE (arg_node), 0, TRUE);
        fprintf (outfile, "%s ", type_str);
        type_str = Free (type_str);

        fprintf (outfile, "%s", VARDEC_NAME (arg_node));

        PrintStatus (VARDEC_ATTRIB (arg_node), FALSE);
        PrintStatus (VARDEC_STATUS (arg_node), FALSE);

        if (VARDEC_COLCHN (arg_node) && show_idx) {
            Trav (VARDEC_COLCHN (arg_node), arg_info);
        }

        fprintf (outfile, "; ");

        if (compiler_phase == PH_typecheck) {
            type_str = TYType2String (AVIS_TYPE (VARDEC_AVIS (arg_node)), FALSE, 0);
            fprintf (outfile, "/* %s */", type_str);
            type_str = Free (type_str);
        }

        Trav (VARDEC_AVIS (arg_node), arg_info);

        fprintf (outfile, "\n");
    } else {
        Trav (VARDEC_ICM (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    if (VARDEC_NEXT (arg_node)) {
        PRINT_CONT (Trav (VARDEC_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintBlock( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintBlock (node *arg_node, node *arg_info)
{
    int old_indent = indent;

    DBUG_ENTER ("PrintBlock");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    INDENT;
    fprintf (outfile, "{ \n");
    indent++;

    if (BLOCK_CACHESIM (arg_node) != NULL) {
        INDENT;
        fprintf (outfile, "#pragma cachesim \"%s\"\n\n", BLOCK_CACHESIM (arg_node));
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        Trav (BLOCK_VARDEC (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    DBUG_EXECUTE ("PRINT_SSA", if (BLOCK_SSACOUNTER (arg_node) != NULL) {
        INDENT;
        fprintf (outfile, "/* SSAcnt:\n");
        Trav (BLOCK_SSACOUNTER (arg_node), arg_info);
        INDENT;
        fprintf (outfile, " */\n");
    });

    if (BLOCK_INSTR (arg_node)) {
        Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    indent--;
    INDENT;
    fprintf (outfile, "}");

    if (indent != old_indent) {
#ifdef WARN_INDENT
        if (FUNDEF_STATUS (INFO_PRINT_FUNDEF (arg_info)) != ST_spmdfun) {
            /*
             * for the time being, indentation of MT funs is always unbalanced :-(
             */
            SYSWARN (("Indentation unbalanced while printing block of function %s."
                      " Indentation at beginning of block: %i."
                      " Indentation at end of block: %i",
                      FUNDEF_NAME (INFO_PRINT_FUNDEF (arg_info)), old_indent, indent));
        }
#endif
        indent = old_indent;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintReturn( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintReturn");

    if (RETURN_USEMASK (arg_node) != NULL) {
        fprintf (outfile, "/* use:");
        DFMPrintMask (outfile, " %s", RETURN_USEMASK (arg_node));
        fprintf (outfile, " */\n");
        INDENT;
    }
    if (RETURN_DEFMASK (arg_node) != NULL) {
        fprintf (outfile, "/* def:");
        DFMPrintMask (outfile, " %s", RETURN_DEFMASK (arg_node));
        fprintf (outfile, " */\n");
        INDENT;
    }

    fprintf (outfile, "return");
    if (RETURN_EXPRS (arg_node) != NULL) {
        fprintf (outfile, "( ");
        Trav (RETURN_EXPRS (arg_node), arg_info);
        fprintf (outfile, ")");
    }
    fprintf (outfile, "; ");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintAssign( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintAssign (node *arg_node, node *arg_info)
{
    node *instr;
    bool trav_instr;

    DBUG_ENTER ("PrintAssign");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "\n"); INDENT;
                  fprintf (outfile, "**MASKS - assign: \n"); INDENT;
                  PrintDefMask (outfile, ASSIGN_DEFMASK (arg_node),
                                INFO_PRINT_VARNO (arg_info));
                  INDENT; PrintUseMask (outfile, ASSIGN_USEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info));
                  INDENT; PrintMrdMask (outfile, ASSIGN_MRDMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info)););

    instr = ASSIGN_INSTR (arg_node);
    DBUG_ASSERT ((instr != NULL), "instruction of N_assign is NULL");

    DBUG_EXECUTE ("WLI", if ((NODE_TYPE (instr) == N_let)
                             && (PRF_PRF (LET_EXPR (instr)) == F_sel)) {
        DbugIndexInfo (ASSIGN_INDEX (arg_node));
    });

    PRINT_LINE_PRAGMA_IN_SIB (outfile, arg_node);

    DBUG_EXECUTE ("LINE", fprintf (outfile, "/*%03d*/", NODE_LINE (arg_node)););

    trav_instr = TRUE;
    if (NODE_TYPE (instr) == N_annotate) {
        if (compiler_phase < PH_compile) {
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
        Trav (instr, arg_info);
        fprintf (outfile, "\n");
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (ASSIGN_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintDo( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintDo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintDo");

    fprintf (outfile, "do \n");

    if (DO_BODY (arg_node) != NULL) {
        DBUG_EXECUTE ("PRINT_MASKS", INDENT; fprintf (outfile, "**MASKS - do body: \n");
                      INDENT; PrintDefMask (outfile, DO_DEFMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info));
                      INDENT; PrintUseMask (outfile, DO_USEMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info));
                      INDENT; PrintMrdMask (outfile, DO_MRDMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info)););

        Trav (DO_BODY (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    INDENT;
    fprintf (outfile, "while (");
    Trav (DO_COND (arg_node), arg_info);
    fprintf (outfile, ");");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintWhile( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintWhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWhile");

    fprintf (outfile, "while (");
    Trav (WHILE_COND (arg_node), arg_info);
    fprintf (outfile, ") \n");

    if (WHILE_BODY (arg_node) != NULL) {
        DBUG_EXECUTE ("PRINT_MASKS", INDENT;
                      fprintf (outfile, "**MASKS - while body: \n"); INDENT;
                      PrintDefMask (outfile, WHILE_DEFMASK (arg_node),
                                    INFO_PRINT_VARNO (arg_info));
                      INDENT; PrintUseMask (outfile, WHILE_USEMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info));
                      INDENT; PrintMrdMask (outfile, WHILE_MRDMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info)););

        Trav (WHILE_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintCond( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintCond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCond");

    fprintf (outfile, "if ");

    fprintf (outfile, "(");
    Trav (COND_COND (arg_node), arg_info);
    fprintf (outfile, ") \n");

    if (COND_THEN (arg_node) != NULL) {
        DBUG_EXECUTE ("PRINT_MASKS", INDENT; fprintf (outfile, "**MASKS - then: \n");
                      INDENT; PrintDefMask (outfile, COND_THENDEFMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info));
                      INDENT; PrintUseMask (outfile, COND_THENUSEMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info)););

        Trav (COND_THEN (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    INDENT;
    fprintf (outfile, "else\n");

    if (COND_ELSE (arg_node) != NULL) {
        DBUG_EXECUTE ("PRINT_MASKS", INDENT; fprintf (outfile, "**MASKS - else: \n");
                      INDENT; PrintDefMask (outfile, COND_ELSEDEFMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info));
                      INDENT; PrintUseMask (outfile, COND_ELSEUSEMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info)););

        Trav (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintCast( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintCast (node *arg_node, node *arg_info)
{
    char *type_str;

    DBUG_ENTER ("PrintCast");

    type_str = Type2String (CAST_TYPE (arg_node), 0, TRUE);
    fprintf (outfile, "(: %s) ", type_str);
    type_str = Free (type_str);

    Trav (CAST_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintLet( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintLet (node *arg_node, node *arg_info)
{
    node *expr;
    bool print_sac = TRUE;
    bool print_c = FALSE;
    bool print_argtab = FALSE;

    DBUG_ENTER ("PrintLet");

    DBUG_PRINT ("PRINT", ("%s " F_PTR, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if (LET_USEMASK (arg_node) != NULL) {
        fprintf (outfile, "/* use:");
        DFMPrintMask (outfile, " %s", LET_USEMASK (arg_node));
        fprintf (outfile, " */\n");
        INDENT;
    }
    if (LET_DEFMASK (arg_node) != NULL) {
        fprintf (outfile, "/* def:");
        DFMPrintMask (outfile, " %s", LET_DEFMASK (arg_node));
        fprintf (outfile, " */\n");
        INDENT;
    }

    expr = LET_EXPR (arg_node);
    if ((NODE_TYPE (expr) == N_ap) && (AP_ARGTAB (expr) != NULL)) {
        print_sac = FALSE;
        print_c = TRUE;

        DBUG_EXECUTE ("PRINT_ARGTAB", fprintf (outfile, "/* \n"); INDENT;
                      fprintf (outfile, " *  "); PrintArgtab (AP_ARGTAB (expr), FALSE);
                      fprintf (outfile, "  */\n"); INDENT; print_sac = TRUE;
                      print_argtab = TRUE;);
    }

    if (print_c) {
        node *tmp = Argtab2Let (expr);

        Trav (tmp, arg_info);

        /*
         * we have to clean AP_FUNDEF in order to fool FreeAp()!!!
         */
        AP_FUNDEF (LET_EXPR (tmp)) = NULL;
        tmp = FreeTree (tmp);

        if (!print_argtab) {
            PrintArgtags (AP_ARGTAB (expr));
        }
    }

    if (print_sac) {
        if (print_c) {
            fprintf (outfile, "\n");
            INDENT;
            fprintf (outfile, "/*  ");
        }

        if (LET_IDS (arg_node) != NULL) {
            PrintIds (LET_IDS (arg_node), arg_info);
            fprintf (outfile, " = ");
        }
        Trav (expr, arg_info);

        fprintf (outfile, "; ");

        if (print_c) {
            fprintf (outfile, "\n");
            INDENT;
            fprintf (outfile, " */ ");
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintPrf( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintPrf (node *arg_node, node *arg_info)
{
    prf prf;
    char *prf_str;

    DBUG_ENTER ("PrintPrf");

    prf = PRF_PRF (arg_node);
    if (sbs) {
        prf_str = (compiler_phase < PH_genccode) ? prf_string[prf] : prf_symbol[prf];
        DBUG_EXECUTE ("PRINT_SYMBOL", prf_str = prf_symbol[prf];);
    } else {
        prf_str = prf_symbol[prf];
    }

    DBUG_PRINT ("PRINT", ("%s (%s)" F_PTR, mdb_nodetype[NODE_TYPE (arg_node)],
                          mdb_prf[prf], arg_node));

    if ((prf == F_sel) && (CountExprs (PRF_ARGS (arg_node)) == 2)) {
        /*
         * F_sel is printed with special [] notation:
         * first the array argument is printed, then a leading '[' followed by
         * the selection vector and finally the closing ']'.
         */
        Trav (PRF_ARG2 (arg_node), arg_info);
        fprintf (outfile, "[");
        Trav (PRF_ARG1 (arg_node), arg_info);
        fprintf (outfile, "]");
    } else if (prf_is_infix[prf]) {
        /* primitive functions in infix notation */
        fprintf (outfile, "(");
        if (PRF_EXPRS2 (arg_node) == NULL) {
            fprintf (outfile, "%s ", prf_str);
        }
        Trav (PRF_ARG1 (arg_node), arg_info);
        if (PRF_EXPRS2 (arg_node) != NULL) {
            fprintf (outfile, " %s ", prf_str);
            DBUG_ASSERT ((PRF_EXPRS3 (arg_node) == NULL), "more than two args found");
            Trav (PRF_ARG2 (arg_node), arg_info);
        }
        fprintf (outfile, ")");
    } else {
        /* primitive functions that are printed as function application */
        fprintf (outfile, "%s(", prf_str);
        if (PRF_ARGS (arg_node) != NULL) {
            fprintf (outfile, " ");
            Trav (PRF_ARGS (arg_node), arg_info);
        }
        fprintf (outfile, ")");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintAp( node  *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintAp (node *arg_node, node *arg_info)
{
    node *fundef;

    DBUG_ENTER ("PrintAp");

    fundef = AP_FUNDEF (arg_node);
    if (compiler_phase >= PH_precompile) {
        /*
         * Precompile() renames N_fundef names only
         *  -> we *must* find an AP_FUNDEF in order to print the correct name!
         */
        DBUG_ASSERT ((fundef != NULL), "no AP_FUNDEF found!");
    }

    if (fundef != NULL) {
        /*
         * print name of 'AP_FUNDEF(arg_node)'
         */
        if ((FUNDEF_STATUS (fundef) == ST_wrapperfun)) {
            fprintf (outfile, "wrapper:");
        }
        if (FUNDEF_MOD (fundef) != NULL) {
            fprintf (outfile, "%s:", FUNDEF_MOD (fundef));
        }
        fprintf (outfile, "%s", FUNDEF_NAME (fundef));
    } else {
        /*
         * print name of 'arg_node'
         */
        if (AP_MOD (arg_node) != NULL) {
            fprintf (outfile, "%s:", AP_MOD (arg_node));
        }
        fprintf (outfile, "%s", AP_NAME (arg_node));
    }

    fprintf (outfile, "(");
    if (AP_ARGS (arg_node) != NULL) {
        fprintf (outfile, " ");
        Trav (AP_ARGS (arg_node), arg_info);
    }
    fprintf (outfile, ")");

    DBUG_EXECUTE ("PRINT_PTR", fprintf (outfile, " /* "); PRINT_POINTER (outfile, fundef);
                  fprintf (outfile, " */ "););

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintMop( node  *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintMop (node *arg_node, node *arg_info)
{
    node *exprs;
    ids *fun_ids;

    DBUG_ENTER ("PrintMop");

    if (MOP_FIX (arg_node)) {
        fprintf (outfile, "(");
    }

    exprs = MOP_EXPRS (arg_node);
    fun_ids = MOP_OPS (arg_node);

    while (fun_ids) {
        Trav (EXPRS_EXPR (exprs), arg_info);

        if (IDS_MOD (fun_ids) != NULL) {
            fprintf (outfile, " %s:%s ", IDS_MOD (fun_ids), IDS_NAME (fun_ids));
        } else {
            fprintf (outfile, " %s ", IDS_NAME (fun_ids));
        }

        exprs = EXPRS_NEXT (exprs);
        fun_ids = IDS_NEXT (fun_ids);
    }
    if (exprs) {
        Trav (EXPRS_EXPR (exprs), arg_info);
    }

    if (MOP_FIX (arg_node)) {
        fprintf (outfile, ")");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintEmpty( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintEmpty (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintEmpty");

    INDENT;
    fprintf (outfile, "/* empty */\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintArray( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintArray (node *arg_node, node *arg_info)
{
    int i;

    int old_print_dim = INFO_PRINT_DIM (arg_info);
    shpseg *old_print_shape = INFO_PRINT_SHAPE (arg_info);
    shpseg *old_print_shape_counter = INFO_PRINT_SHAPE_COUNTER (arg_info);

    DBUG_ENTER ("PrintArray");

    if (ARRAY_AELEMS (arg_node) != NULL) {

        INFO_PRINT_DIM (arg_info) = ARRAY_DIM (arg_node);
        INFO_PRINT_SHAPE (arg_info) = SHShape2OldShpseg (ARRAY_SHAPE (arg_node));
        INFO_PRINT_SHAPE_COUNTER (arg_info)
          = Array2Shpseg (CreateZeroVector (ARRAY_DIM (arg_node), T_int), NULL);

        for (i = 0; i < INFO_PRINT_DIM (arg_info); i++)
            fprintf (outfile, "[ ");

        Trav (ARRAY_AELEMS (arg_node), arg_info);

        for (i = 0; i < INFO_PRINT_DIM (arg_info); i++)
            fprintf (outfile, " ]");

    } else {
        fprintf (outfile, "[]");
    }

    if (INFO_PRINT_SHAPE (arg_info) != NULL)
        FreeShpseg (INFO_PRINT_SHAPE (arg_info));

    if (INFO_PRINT_SHAPE_COUNTER (arg_info) != NULL)
        FreeShpseg (INFO_PRINT_SHAPE_COUNTER (arg_info));

    INFO_PRINT_DIM (arg_info) = old_print_dim;
    INFO_PRINT_SHAPE (arg_info) = old_print_shape;
    INFO_PRINT_SHAPE_COUNTER (arg_info) = old_print_shape_counter;

    if (compiler_phase != PH_genccode) {
        DBUG_EXECUTE ("PRINT_CAR", DbugPrintArray (arg_node););
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintExprs( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintExprs (node *arg_node, node *arg_info)
{
    int i;
    int j;

    DBUG_ENTER ("PrintExprs");

    Trav (EXPRS_EXPR (arg_node), arg_info);

    if (EXPRS_NEXT (arg_node) != NULL) {
        if (arg_info != NULL) {
            for (i = INFO_PRINT_DIM (arg_info) - 1;
                 (i >= 0)
                 && (++SHPSEG_SHAPE (INFO_PRINT_SHAPE_COUNTER (arg_info), i)
                     >= SHPSEG_SHAPE (INFO_PRINT_SHAPE (arg_info), i));
                 i--)
                SHPSEG_SHAPE (INFO_PRINT_SHAPE_COUNTER (arg_info), i) = 0;
            for (j = INFO_PRINT_DIM (arg_info) - 1; j > i; j--)
                fprintf (outfile, " ]");
            fprintf (outfile, ", ");
            for (j = INFO_PRINT_DIM (arg_info) - 1; j > i; j--)
                fprintf (outfile, "[ ");
            PRINT_CONT (Trav (EXPRS_NEXT (arg_node), arg_info), ;);
        } else {
            fprintf (outfile, ", ");
            PRINT_CONT (Trav (EXPRS_NEXT (arg_node), arg_info), ;);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintId( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintId (node *arg_node, node *arg_info)
{
    bool print_nt = FALSE;

    DBUG_ENTER ("PrintId");

    if (ID_MOD (arg_node) != NULL) {
        fprintf (outfile, "%s:", ID_MOD (arg_node));
    }

    DBUG_EXECUTE ("PRINT_NT", print_nt = TRUE;);

    fprintf (outfile, "%s",
             (((compiler_phase == PH_genccode) || print_nt)
              && (ID_NT_TAG (arg_node) != NULL))
               ? ID_NT_TAG (arg_node)
               : ID_NAME (arg_node));

    PrintFlag (FLAG2STRING (ID, arg_node, IS_GLOBAL), FALSE);
    PrintFlag (FLAG2STRING (ID, arg_node, IS_REFERENCE), FALSE);
    PrintFlag (FLAG2STRING (ID, arg_node, IS_READ_ONLY), FALSE);
    PrintStatus (ID_STATUS (arg_node), FALSE);

    PrintRC (ID_REFCNT (arg_node), ID_NAIVE_REFCNT (arg_node), show_refcnt);

    if (compiler_phase != PH_genccode) {
        DBUG_EXECUTE ("PRINT_CAR", DbugPrintArray (arg_node););
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintNum( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintNum (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNum");

    fprintf (outfile, "%d", NUM_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintFloat( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintFloat (node *arg_node, node *arg_info)
{
    char *tmp_string;

    DBUG_ENTER ("PrintFloat");

    tmp_string = Float2String (FLOAT_VAL (arg_node));
    fprintf (outfile, "%s", tmp_string);
    tmp_string = Free (tmp_string);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintDouble( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintDouble (node *arg_node, node *arg_info)
{
    char *tmp_string;

    DBUG_ENTER ("PrintDouble");

    tmp_string = Double2String (DOUBLE_VAL (arg_node));
    fprintf (outfile, "%s", tmp_string);
    tmp_string = Free (tmp_string);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintBool( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintBool (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintBool");

    if (0 == BOOL_VAL (arg_node)) {
        fprintf (outfile, "false");
    } else {
        fprintf (outfile, "true");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintStr( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintStr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintStr");

    fprintf (outfile, "\"%s\"", STR_STRING (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *  node *PrintDot( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintDot (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("PrintDot");

    for (i = 0; i < DOT_NUM (arg_node); i++) {
        fprintf (outfile, ".");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *  node *PrintSetWL( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintSetWL (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintSetWL");

    if (NODE_TYPE (SETWL_IDS (arg_node)) == N_exprs) {
        fprintf (outfile, "{ [");
        Trav (SETWL_IDS (arg_node), arg_info);
        fprintf (outfile, "] -> ");
    } else {
        Trav (SETWL_IDS (arg_node), arg_info);
        fprintf (outfile, " -> ");
    }

    Trav (SETWL_EXPR (arg_node), arg_info);
    fprintf (outfile, " }");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintChar( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintChar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintChar");

    if ((CHAR_VAL (arg_node) >= ' ') && (CHAR_VAL (arg_node) <= '~')
        && (CHAR_VAL (arg_node) != '\'')) {
        fprintf (outfile, "'%c'", CHAR_VAL (arg_node));
    } else {
        switch (CHAR_VAL (arg_node)) {
        case '\n':
            fprintf (outfile, "'\\n'");
            break;
        case '\t':
            fprintf (outfile, "'\\t'");
            break;
        case '\v':
            fprintf (outfile, "'\\v'");
            break;
        case '\b':
            fprintf (outfile, "'\\b'");
            break;
        case '\r':
            fprintf (outfile, "'\\r'");
            break;
        case '\f':
            fprintf (outfile, "'\\f'");
            break;
        case '\a':
            fprintf (outfile, "'\\a'");
            break;
        default:
            fprintf (outfile, "'\\%o'", CHAR_VAL (arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintVectInfo( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintVectInfo (node *arg_node, node *arg_info)
{
    char *type_str;

    DBUG_ENTER ("PrintVectInfo");

    if (show_idx) {
        switch (VINFO_FLAG (arg_node)) {
        case DOLLAR:
            fprintf (outfile, ":$");
            break;
        case VECT:
            fprintf (outfile, ":VECT");
            break;
        case IDX:
            type_str = Type2String (VINFO_TYPE (arg_node), 0, TRUE);
            fprintf (outfile, ":IDX(%s)", type_str);
            type_str = Free (type_str);
            break;
        default:
            DBUG_ASSERT (0, "illegal N_vinfo-flag!");
            break;
        }

        if (VINFO_NEXT (arg_node) != NULL) {
            PRINT_CONT (Trav (VINFO_NEXT (arg_node), arg_info), ;);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintIcm( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintIcm (node *arg_node, node *arg_info)
{
    bool compiled_icm = FALSE;

    DBUG_ENTER ("PrintIcm");

    DBUG_PRINT ("PRINT", ("icm-node %s\n", ICM_NAME (arg_node)));

    if (compiler_phase == PH_genccode) {
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
            indent += ICM_INDENT_BEFORE (arg_node);
            INDENT;
        }
        fprintf (outfile, "SAC_%s( ", ICM_NAME (arg_node));
        if (ICM_ARGS (arg_node) != NULL) {
            Trav (ICM_ARGS (arg_node), arg_info);
        }
        fprintf (outfile, ")");
        if (last_assignment_icm == arg_node) {
            indent += ICM_INDENT_AFTER (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintPragma( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintPragma (node *arg_node, node *arg_info)
{
    int i, first;

    DBUG_ENTER ("PrintPragma");

    if (PRAGMA_LINKNAME (arg_node) != NULL) {
        fprintf (outfile, "#pragma linkname \"%s\"\n", PRAGMA_LINKNAME (arg_node));
    }

    if (PRAGMA_LINKSIGN (arg_node) != NULL) {
        fprintf (outfile, "#pragma linksign [");
        if (PRAGMA_NUMPARAMS (arg_node) > 0) {
            fprintf (outfile, "%d", PRAGMA_LS (arg_node, 0));
        }
        for (i = 1; i < PRAGMA_NUMPARAMS (arg_node); i++) {
            fprintf (outfile, ", %d", PRAGMA_LS (arg_node, i));
        }
        fprintf (outfile, "]\n");
    }

    if (PRAGMA_REFCOUNTING (arg_node) != NULL) {
        fprintf (outfile, "#pragma refcounting [");
        first = 1;
        for (i = 0; i < PRAGMA_NUMPARAMS (arg_node); i++) {
            if (PRAGMA_RC (arg_node, i)) {
                if (first) {
                    fprintf (outfile, "%d", i);
                    first = 0;
                } else {
                    fprintf (outfile, ", %d", i);
                }
            }
        }
        fprintf (outfile, "]\n");
    }

    if (PRAGMA_READONLY (arg_node) != NULL) {
        fprintf (outfile, "#pragma readonly [");
        first = 1;
        for (i = 0; i < PRAGMA_NUMPARAMS (arg_node); i++) {
            if (PRAGMA_RO (arg_node, i)) {
                if (first) {
                    fprintf (outfile, "%d", i);
                    first = 0;
                } else {
                    fprintf (outfile, ", %d", i);
                }
            }
        }
        fprintf (outfile, "]\n");
    }

    if (PRAGMA_EFFECT (arg_node) != NULL) {
        fprintf (outfile, "#pragma effect ");
        PrintIds (PRAGMA_EFFECT (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    if (PRAGMA_TOUCH (arg_node) != NULL) {
        fprintf (outfile, "#pragma touch ");
        PrintIds (PRAGMA_TOUCH (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    if (PRAGMA_COPYFUN (arg_node) != NULL) {
        fprintf (outfile, "#pragma copyfun \"%s\"\n", PRAGMA_COPYFUN (arg_node));
    }

    if (PRAGMA_FREEFUN (arg_node) != NULL) {
        fprintf (outfile, "#pragma freefun \"%s\"\n", PRAGMA_FREEFUN (arg_node));
    }

    if (PRAGMA_INITFUN (arg_node) != NULL) {
        fprintf (outfile, "#pragma initfun \"%s\"\n", PRAGMA_INITFUN (arg_node));
    }

    if (PRAGMA_WLCOMP_APS (arg_node) != NULL) {
        fprintf (outfile, "#pragma wlcomp ");
        Trav (PRAGMA_WLCOMP_APS (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    if (PRAGMA_APL (arg_node) != NULL) {
        fprintf (outfile, "#pragma wlcomp ");
        Trav (PRAGMA_APL (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintSpmd( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintSpmd (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintSpmd");

    fprintf (outfile, "\n");

    if (SPMD_ICM_BEGIN (arg_node) == NULL) {
        /*
         * The SPMD-block has not yet been compiled.
         */

        INDENT;
        fprintf (outfile, "/*** begin of SPMD region ***\n");

        INDENT;
        fprintf (outfile, " * in:");
        DFMPrintMask (outfile, " %s", SPMD_IN (arg_node));
        fprintf (outfile, "\n");

        INDENT;
        fprintf (outfile, " * inout:");
        DFMPrintMask (outfile, " %s", SPMD_INOUT (arg_node));
        fprintf (outfile, "\n");

        INDENT;
        fprintf (outfile, " * out:");
        DFMPrintMask (outfile, " %s", SPMD_OUT (arg_node));
        fprintf (outfile, "\n");

        INDENT;
        fprintf (outfile, " * shared:");
        DFMPrintMask (outfile, " %s", SPMD_SHARED (arg_node));
        fprintf (outfile, "\n");

        INDENT;
        fprintf (outfile, " * local:");
        DFMPrintMask (outfile, " %s", SPMD_LOCAL (arg_node));
        fprintf (outfile, "\n");

        INDENT;
        fprintf (outfile, " */\n");

        Trav (SPMD_REGION (arg_node), arg_info);

        fprintf (outfile, "\n");
        INDENT;
        fprintf (outfile, "/*** end of SPMD region ***/\n");
    } else {
        /*
         * print ICMs
         */
        INDENT;
        Trav (SPMD_ICM_BEGIN (arg_node), arg_info);
        fprintf (outfile, "\n");
        Trav (SPMD_ICM_PARALLEL (arg_node), arg_info);
        INDENT;
        fprintf (outfile, "\n");
        INDENT;
        Trav (SPMD_ICM_ALTSEQ (arg_node), arg_info);
        fprintf (outfile, "\n");
        Trav (SPMD_ICM_SEQUENTIAL (arg_node), arg_info);
        fprintf (outfile, "\n");
        INDENT;
        Trav (SPMD_ICM_END (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintSync( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintSync (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintSync");

    fprintf (outfile, "\n");
    INDENT;

    fprintf (outfile, "/*** begin of sync region ***\n");

    INDENT;
    fprintf (outfile, " * in:");
    DFMPrintMask (outfile, " %s", SYNC_IN (arg_node));
    fprintf (outfile, "\n");

    INDENT;
    fprintf (outfile, " * inout:");
    DFMPrintMask (outfile, " %s", SYNC_INOUT (arg_node));
    fprintf (outfile, "\n");

    INDENT;
    fprintf (outfile, " * out:");
    DFMPrintMask (outfile, " %s", SYNC_OUT (arg_node));
    fprintf (outfile, "\n");

    INDENT;
    fprintf (outfile, " * outrep:");
    DFMPrintMask (outfile, " %s", SYNC_OUTREP (arg_node));
    fprintf (outfile, "\n");

    INDENT;
    fprintf (outfile, " * local:");
    DFMPrintMask (outfile, " %s", SYNC_LOCAL (arg_node));
    fprintf (outfile, "\n");

    INDENT;
    fprintf (outfile, " */\n");

    Trav (SYNC_REGION (arg_node), arg_info);

    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, "/*** end of sync region ***/\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintMT(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintMT (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintMT");

    /* PrintAssign already indents */
    fprintf (outfile, "MT(%i) {", MT_IDENTIFIER (arg_node));
    fprintf (outfile, " /*** begin of mt region ***/\n");

    if (MT_USEMASK (arg_node) != NULL) {
        INDENT;
        fprintf (outfile, "/* use:");
        DFMPrintMask (outfile, " %s", MT_USEMASK (arg_node));
        fprintf (outfile, " */\n");
    }
    if (MT_DEFMASK (arg_node) != NULL) {
        INDENT;
        fprintf (outfile, "/* def:");
        DFMPrintMask (outfile, " %s", MT_DEFMASK (arg_node));
        fprintf (outfile, " */\n");
    }
    if (MT_NEEDLATER (arg_node) != NULL) {
        INDENT;
        fprintf (outfile, "/* needlater:");
        DFMPrintMask (outfile, " %s", MT_NEEDLATER (arg_node));
        fprintf (outfile, " */\n");
    }

    indent++;
    if (MT_REGION (arg_node) != NULL) {
        Trav (MT_REGION (arg_node), arg_info);
    } else {
        INDENT;
        fprintf (outfile, "/* ... Empty ... */");
    }
    indent--;

    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, "} /*** end of mt region ***/\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintST(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintST (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintST");

    /* PrintAssign already indents */
    fprintf (outfile, "ST(%i) {", ST_IDENTIFIER (arg_node));
    fprintf (outfile, " /*** begin of st region ***/\n");

    if (ST_USEMASK (arg_node) != NULL) {
        INDENT;
        fprintf (outfile, "/* use:");
        DFMPrintMask (outfile, " %s", ST_USEMASK (arg_node));
        fprintf (outfile, " */\n");
    }
    if (ST_DEFMASK (arg_node) != NULL) {
        INDENT;
        fprintf (outfile, "/* def:");
        DFMPrintMask (outfile, " %s", ST_DEFMASK (arg_node));
        fprintf (outfile, " */\n");
    }
    if (ST_NEEDLATER_ST (arg_node) != NULL) {
        INDENT;
        fprintf (outfile, "/* needlater_st:");
        DFMPrintMask (outfile, " %s", ST_NEEDLATER_ST (arg_node));
        fprintf (outfile, " */\n");
    }
    if (ST_NEEDLATER_MT (arg_node) != NULL) {
        INDENT;
        fprintf (outfile, "/* needlater_mt");
        DFMPrintMask (outfile, " %s", ST_NEEDLATER_MT (arg_node));
        fprintf (outfile, " */\n");
    }

    indent++;
    if (ST_REGION (arg_node) != NULL) {
        Trav (ST_REGION (arg_node), arg_info);
    } else {
        INDENT;
        fprintf (outfile, "/* ... Empty ... */");
    }
    indent--;

    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, "} /*** end of st region ***/\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintMTsignal( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintMTsignal (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintMTsignal");

    /* PrintAssign already indents and prints \n */
    fprintf (outfile, "MTsignal (");

    DFMPrintMask (outfile, " %s", MTSIGNAL_IDSET (arg_node));

    fprintf (outfile, ");");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintMTsync( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintMTsync (node *arg_node, node *arg_info)
{
    DFMfoldmask_t *foldmask;

    DBUG_ENTER ("PrintMTsync");

    /* PrintAssign already indents and prints \n */
    fprintf (outfile, "MTsync (");

    if (MTSYNC_WAIT (arg_node) != NULL) {
        DFMPrintMask (outfile, " %s", MTSYNC_WAIT (arg_node));
    } else {
        fprintf (outfile, " NULL");
    }
    fprintf (outfile, ",");

    if (MTSYNC_FOLD (arg_node) != NULL) {
        fprintf (outfile, " [");
        foldmask = MTSYNC_FOLD (arg_node);
        while (foldmask != NULL) {
            fprintf (outfile, " %s:%s", VARDEC_OR_ARG_NAME (DFMFM_VARDEC (foldmask)),
                     FUNDEF_NAME (NWITHOP_FUNDEF (DFMFM_FOLDOP (foldmask))));

            foldmask = DFMFM_NEXT (foldmask);
        }
        fprintf (outfile, "]");
    } else {
        fprintf (outfile, " NULL");
    }
    fprintf (outfile, ",");
    if (MTSYNC_ALLOC (arg_node) != NULL) {
        DFMPrintMask (outfile, " %s", MTSYNC_ALLOC (arg_node));
    } else {
        fprintf (outfile, " NULL");
    }
    fprintf (outfile, ");");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintMTalloc( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintMTalloc (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintMTalloc");

    /* PrintAssign already indents */
    fprintf (outfile, "MTalloc (");

    DFMPrintMask (outfile, " %s", MTALLOC_IDSET (arg_node));

    fprintf (outfile, ");");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNwith( node *arg_node, node *arg_info)
 *
 * description:
 *   prints Nwith node.
 *
 ******************************************************************************/

node *
PrintNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNwith");

    DBUG_EXECUTE ("WLI",
                  fprintf (outfile,
                           "\n** WLI N_Nwith : "
                           "(PARTS %d, REF %d(%d,%d), FOLDABLE %d, NO_CHANCE %d)\n",
                           NWITH_PARTS (arg_node), NWITH_REFERENCED (arg_node),
                           NWITH_REFERENCED_FOLD (arg_node),
                           NWITH_REFERENCES_FOLDED (arg_node), NWITH_FOLDABLE (arg_node),
                           NWITH_NO_CHANCE (arg_node)););

    indent++;

    if (NWITH_PRAGMA (arg_node) != NULL) {
        Trav (NWITH_PRAGMA (arg_node), arg_info);
        INDENT;
    }

    indent++;

    fprintf (outfile, "with");
    fprintf (outfile, " ( ");
    PrintIds (NWITHID_VEC (NPART_WITHID (NWITH_PART (arg_node))), arg_info);
    fprintf (outfile, " )\n");
    indent++;
    Trav (NWITH_PART (arg_node), arg_info);
    indent--;

    Trav (NWITH_WITHOP (arg_node), arg_info);

    indent--;

    DBUG_EXECUTE ("PRINT_RC",
                  if (NWITH_PRAGMA (arg_node) == NULL) {
                      fprintf (outfile, "\n");
                      INDENT;
                  } fprintf (outfile, "/* DEC_RC: ");
                  if (
                    NWITH_DEC_RC_IDS (arg_node)
                    != NULL) { PrintIds (NWITH_DEC_RC_IDS (arg_node), arg_info); } else {
                      fprintf (outfile, "-");
                  } fprintf (outfile, " */\n");
                  INDENT;);

    indent--;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNwithid(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Nwithid-nodes
 *
 ******************************************************************************/

node *
PrintNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNwithid");

    if (NWITHID_VEC (arg_node)) {
        PrintIds (NWITHID_VEC (arg_node), arg_info);
        if (NWITHID_IDS (arg_node)) {
            fprintf (outfile, "=");
        }
    }

    if (NWITHID_IDS (arg_node)) {
        fprintf (outfile, "[");
        PrintIds (NWITHID_IDS (arg_node), arg_info);
        fprintf (outfile, "]");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNgenerator(node *arg_node, node *arg_info)
 *
 * description:
 *   prints a generator.
 *
 *   The index variable is found in NPART_WITHID( INFO_PRINT_NPART( arg_info)).
 *
 ******************************************************************************/

node *
PrintNgenerator (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNgenerator");

    fprintf (outfile, "(");

    /* print upper bound */
    if (NGEN_BOUND1 (arg_node)) {
        Trav (NGEN_BOUND1 (arg_node), arg_info);
    } else {
        fprintf (outfile, ". (NULL)");
    }
    /* print first operator */
    fprintf (outfile, " %s ", prf_symbol[NGEN_OP1 (arg_node)]);

    /* print indices */
    if (INFO_PRINT_NPART (arg_info) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (INFO_PRINT_NPART (arg_info)) == N_Npart),
                     "INFO_PRINT_NPART is no N_Npart node");

        DBUG_ASSERT ((NPART_WITHID (INFO_PRINT_NPART (arg_info)) != NULL),
                     "NPART_WITHID not found!");
        Trav (NPART_WITHID (INFO_PRINT_NPART (arg_info)), arg_info);
    } else {
        fprintf (outfile, "?");
    }

    /* print second operator */
    fprintf (outfile, " %s ", prf_symbol[NGEN_OP2 (arg_node)]);
    /* print lower bound */
    if (NGEN_BOUND2 (arg_node)) {
        Trav (NGEN_BOUND2 (arg_node), arg_info);
    } else {
        fprintf (outfile, ". (NULL)");
    }

    /* print step and width */
    if (NGEN_STEP (arg_node)) {
        fprintf (outfile, " step ");
        Trav (NGEN_STEP (arg_node), arg_info);
    }
    if (NGEN_WIDTH (arg_node)) {
        fprintf (outfile, " width ");
        Trav (NGEN_WIDTH (arg_node), arg_info);
    }

    fprintf (outfile, ")\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNcode( node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Ncode-nodes
 *
 ******************************************************************************/

node *
PrintNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNcode");

    DBUG_ASSERT ((NCODE_USED (arg_node) >= 0), "illegal NCODE_USED value!");

    DBUG_EXECUTE ("PRINT_MASKS", INDENT; fprintf (outfile, "**MASKS - Ncode: \n"); INDENT;
                  PrintDefMask (outfile, NCODE_DEFMASK (arg_node),
                                INFO_PRINT_VARNO (arg_info));
                  INDENT; PrintUseMask (outfile, NCODE_USEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info)););

    DBUG_EXECUTE ("PRINT_RC", INDENT; fprintf (outfile, "/* INC_RC: "); if (
                    NCODE_INC_RC_IDS (arg_node)
                    != NULL) { PrintIds (NCODE_INC_RC_IDS (arg_node), arg_info); } else {
        fprintf (outfile, "-");
    } fprintf (outfile, " */\n"););

    /* print the code section; the body first */
    Trav (NCODE_CBLOCK (arg_node), arg_info);

    /*
     *  NCODE_WLAA_INFO(arg_node) is set to NULL by initializing the N_Ncode node.
     */
    DBUG_EXECUTE ("PRINT_WLAA", if ((compiler_phase == PH_sacopt)
                                    && (NCODE_WLAA_INFO (arg_node) != NULL)) {
        WLAAprintAccesses (arg_node, arg_info);
    });

    DBUG_EXECUTE ("PRINT_TSI", if ((compiler_phase == PH_sacopt)
                                   && (NCODE_WLAA_INFO (arg_node) != NULL)) {
        TSIprintInfo (arg_node, arg_info);
    });

    fprintf (outfile, " : ");
    Trav (NCODE_CEXPRS (arg_node), arg_info);
    fprintf (outfile, " ; ");

    if (NCODE_AP_DUMMY_CODE (arg_node)) {
        fprintf (outfile, " /* dummy code for AP */");
    }
    if (NCODE_USED (arg_node) == 0) {
        fprintf (outfile, " /* unused! */");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNpart( node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Npart nodes
 *
 ******************************************************************************/

node *
PrintNpart (node *arg_node, node *arg_info)
{
    node *tmp_npart;

    DBUG_ENTER ("PrintNpart");

    tmp_npart = INFO_PRINT_NPART (arg_info);
    INFO_PRINT_NPART (arg_info) = arg_node;

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "\n"); INDENT;
                  fprintf (outfile, "**MASKS - Npart: \n"); INDENT;
                  PrintDefMask (outfile, NPART_DEFMASK (arg_node),
                                INFO_PRINT_VARNO (arg_info));
                  INDENT; PrintUseMask (outfile, NPART_USEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info));
                  INDENT;);

    /* print generator */
    INDENT; /* each gen in a new line. */
    Trav (NPART_GEN (arg_node), arg_info);

    DBUG_ASSERT ((NPART_CODE (arg_node) != NULL),
                 "part within WL without pointer to N_Ncode");

    Trav (NPART_CODE (arg_node), arg_info);

    if (NPART_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",\n");
        /*
         * continue with other parts
         */
        PRINT_CONT (Trav (NPART_NEXT (arg_node), arg_info), ;);
    } else {
        fprintf (outfile, "\n");
    }

    INFO_PRINT_NPART (arg_info) = tmp_npart;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNwithop( node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Nwithop-nodes
 *
 *   ATTENTION: the closed bracket ) is not printed,
 *              because PrintNwith must append the last expr.
 *
 ******************************************************************************/

node *
PrintNwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNwithop");

    DBUG_EXECUTE ("PRINT_MASKS", INDENT; fprintf (outfile, "**MASKS - Nwithop: \n");
                  INDENT; PrintDefMask (outfile, NWITHOP_DEFMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info));
                  INDENT; PrintUseMask (outfile, NWITHOP_USEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info)););

    INDENT;
    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        fprintf (outfile, "genarray( ");
        Trav (NWITHOP_SHAPE (arg_node), arg_info);
        if (NWITHOP_DEFAULT (arg_node) != NULL) {
            fprintf (outfile, " , ");
            Trav (NWITHOP_DEFAULT (arg_node), arg_info);
        }
        break;

    case WO_modarray:
        fprintf (outfile, "modarray( ");
        Trav (NWITHOP_ARRAY (arg_node), arg_info);
        break;

    case WO_foldfun:
        if (NWITHOP_MOD (arg_node) == NULL) {
            fprintf (outfile, "fold/*fun*/( %s, ", NWITHOP_FUN (arg_node));
        } else {
            fprintf (outfile, "fold/*fun*/( %s:%s, ", NWITHOP_MOD (arg_node),
                     NWITHOP_FUN (arg_node));
        }
        Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        break;

    case WO_foldprf:
        fprintf (outfile, "fold/*prf*/( %s", prf_string[NWITHOP_PRF (arg_node)]);
        if (NWITHOP_NEUTRAL (arg_node)) {
            fprintf (outfile, ", ");
            Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        }
        break;

    default:
        DBUG_ASSERT ((0), "illegal WL type found!");
        break;
    }

    fprintf (outfile, ")");

    if (NWITHOP_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",\n");
        /*
         * continue with other withops
         */
        PRINT_CONT (Trav (NWITHOP_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNwith2( node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Nwith2-nodes
 *
 ******************************************************************************/

node *
PrintNwith2 (node *arg_node, node *arg_info)
{
    node *code, *tmp_nwith2;
    int id;

    DBUG_ENTER ("PrintNwith2");

    tmp_nwith2 = INFO_PRINT_NWITH2 (arg_info);
    INFO_PRINT_NWITH2 (arg_info) = arg_node;

    indent++;

    if (NWITH2_PRAGMA (arg_node) != NULL) {
        Trav (NWITH2_PRAGMA (arg_node), arg_info);
        INDENT;
    }

    indent++;

    fprintf (outfile, "with2 (");
    Trav (NWITH2_WITHID (arg_node), arg_info);
    fprintf (outfile, ")\n");

    INDENT;
    fprintf (outfile, "/********** operators: **********/\n");
    code = NWITH2_CODE (arg_node);
    id = 0;
    while (code != NULL) {
        INDENT;
        fprintf (outfile, "op_%d =\n", id);
        /*
         * store code-id before printing NWITH2_SEGS!!
         */
        NCODE_ID (code) = id++;
        indent++;
        Trav (code, arg_info);
        indent--;

        if (NCODE_NEXT (code) != NULL) {
            fprintf (outfile, ",\n");
        } else {
            fprintf (outfile, "\n");
        }

        code = NCODE_NEXT (code);
    }

    if (NWITH2_SEGS (arg_node) != NULL) {
        Trav (NWITH2_SEGS (arg_node), arg_info);
    }

    INDENT;
    fprintf (outfile, "/********** conexpr: **********/\n");
    Trav (NWITH2_WITHOP (arg_node), arg_info);
    fprintf (outfile, ")");

    indent--;

    DBUG_EXECUTE ("PRINT_RC",
                  if (NWITH2_PRAGMA (arg_node) == NULL) {
                      fprintf (outfile, "\n");
                      INDENT;
                  } fprintf (outfile, "/* DEC_RC: ");
                  if (
                    NWITH2_DEC_RC_IDS (arg_node)
                    != NULL) { PrintIds (NWITH2_DEC_RC_IDS (arg_node), arg_info); } else {
                      fprintf (outfile, "-");
                  } fprintf (outfile, " */\n");
                  INDENT;);

    indent--;

    INFO_PRINT_NWITH2 (arg_info) = tmp_nwith2;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLsegx( node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLseg- and N_WLsegVar-nodes.
 *
 ******************************************************************************/

node *
PrintWLsegx (node *arg_node, node *arg_info)
{
    node *seg;
    int id;

    DBUG_ENTER ("PrintWLsegx");

    seg = arg_node;
    id = 0;
    while (seg != NULL) {
        INDENT;
        fprintf (outfile, "/**********%s segment %d: **********",
                 (NODE_TYPE (seg) == N_WLseg) ? "" : " (var.)", id++);

        fprintf (outfile, "\n");
        INDENT;
        fprintf (outfile, " * index domain: ");
        WLSEGX_IDX_PRINT (outfile, seg, IDX_MIN);
        fprintf (outfile, " -> ");
        WLSEGX_IDX_PRINT (outfile, seg, IDX_MAX);
        fprintf (outfile, "\n");
        INDENT;

        if (NODE_TYPE (arg_node) == N_WLseg) {
            fprintf (outfile, " * sv: ");
            PRINT_VECT (outfile, WLSEG_SV (arg_node), WLSEG_DIMS (arg_node), "%i");
            fprintf (outfile, "\n");
            INDENT;
            fprintf (outfile, " * homsv: ");
            PRINT_HOMSV (outfile, WLSEG_HOMSV (arg_node), WLSEG_DIMS (arg_node));
            fprintf (outfile, "\n");
            INDENT;
        }

        if (WLSEGX_SCHEDULING (seg) != NULL) {
            fprintf (outfile, " * scheduling: ");
            SCHPrintScheduling (outfile, WLSEGX_SCHEDULING (seg));
            fprintf (outfile, "\n");
            INDENT;
        }

        if (WLSEGX_TASKSEL (seg) != NULL) {
            fprintf (outfile, " * taskselector: ");
            SCHPrintTasksel (outfile, WLSEGX_TASKSEL (seg));
            fprintf (outfile, "\n");
            INDENT;
        }

        fprintf (outfile, " */\n");

        Trav (WLSEGX_CONTENTS (seg), arg_info);
        PRINT_CONT (seg = WLSEGX_NEXT (seg), seg = NULL)
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLxblock( node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLblock- and N_WLublock-nodes
 *
 ******************************************************************************/

node *
PrintWLxblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLxblock");

    INDENT;
    fprintf (outfile, "(");

    NodeOrInt_Print (outfile, NODE_TYPE (arg_node), &(WLXBLOCK_BOUND1 (arg_node)),
                     WLXBLOCK_DIM (arg_node));
    fprintf (outfile, " -> ");
    NodeOrInt_Print (outfile, NODE_TYPE (arg_node), &(WLXBLOCK_BOUND2 (arg_node)),
                     WLXBLOCK_DIM (arg_node));
    fprintf (outfile, "), ");

    fprintf (outfile, "%sblock%d[%d] %d:", (NODE_TYPE (arg_node) == N_WLblock) ? "" : "u",
             WLXBLOCK_LEVEL (arg_node), WLXBLOCK_DIM (arg_node),
             WLXBLOCK_STEP (arg_node));

    if (WLXBLOCK_NOOP (arg_node)) {
        fprintf (outfile, " /* noop */");
    }

    fprintf (outfile, "\n");

    if (WLXBLOCK_CONTENTS (arg_node) != NULL) {
        indent++;
        Trav (WLXBLOCK_CONTENTS (arg_node), arg_info);
        indent--;
    }

    if (WLXBLOCK_NEXTDIM (arg_node) != NULL) {
        indent++;
        Trav (WLXBLOCK_NEXTDIM (arg_node), arg_info);
        indent--;
    }

    if (WLXBLOCK_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (WLXBLOCK_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLstridex( node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLstride- and N_WLstrideVar-nodes.
 *
 * remark:
 *   N_WLstride-nodes    are printed as '->',
 *   N_WLstrideVar-nodes are printed as '=>'.
 *
 ******************************************************************************/

node *
PrintWLstridex (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLstridex");

    INDENT;
    fprintf (outfile, "(");
    NodeOrInt_Print (outfile, NODE_TYPE (arg_node), WLSTRIDEX_GET_ADDR (arg_node, BOUND1),
                     WLSTRIDEX_DIM (arg_node));
    fprintf (outfile, " %s> ", (NODE_TYPE (arg_node) == N_WLstride) ? "-" : "=");
    NodeOrInt_Print (outfile, NODE_TYPE (arg_node), WLSTRIDEX_GET_ADDR (arg_node, BOUND2),
                     WLSTRIDEX_DIM (arg_node));
    fprintf (outfile, "), step%d[%d] ", WLSTRIDEX_LEVEL (arg_node),
             WLSTRIDEX_DIM (arg_node));
    NodeOrInt_Print (outfile, NODE_TYPE (arg_node), WLSTRIDEX_GET_ADDR (arg_node, STEP),
                     WLSTRIDEX_DIM (arg_node));

    if (WLSTRIDEX_NOOP (arg_node)) {
        fprintf (outfile, ": /* noop */");
    }

    fprintf (outfile, "\n");

    if (WLSTRIDEX_CONTENTS (arg_node) != NULL) {
        indent++;
        Trav (WLSTRIDEX_CONTENTS (arg_node), arg_info);
        indent--;
    }

    if (WLSTRIDEX_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (WLSTRIDEX_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintWLcode( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
PrintWLcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLcode");

    fprintf (outfile, " ");
    if (arg_node != NULL) {
        DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Ncode), "illegal code node found!");

        /*
         * we use the code here, therefore the counter USED should be >0 !!
         */
        DBUG_ASSERT ((NCODE_USED (arg_node) > 0), "illegal NCODE_USED value!");

        fprintf (outfile, "op_%d", NCODE_ID (arg_node));
    } else {
        if (INFO_PRINT_NWITH2 (arg_info) != NULL) {
            DBUG_ASSERT ((NODE_TYPE (INFO_PRINT_NWITH2 (arg_info)) == N_Nwith2),
                         "INFO_PRINT_NWITH2 is no N_Nwith2 node");

            switch (NWITH2_TYPE (INFO_PRINT_NWITH2 (arg_info))) {
            case WO_genarray:
                fprintf (outfile, "init");
                break;

            case WO_modarray:
                fprintf (outfile, "copy");
                break;

            case WO_foldfun:
                /* here is no break missing! */
            case WO_foldprf:
                fprintf (outfile, "noop");
                break;

            default:
                DBUG_ASSERT ((0), "illegal with-loop type found");
                break;
            }
        } else {
            fprintf (outfile, "?");
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLgridx( node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLgrid- and N_WLgridVar-nodes.
 *
 * remark:
 *   N_WLgrid-nodes    are printed as '-->' (fitted) and '->>' (unfitted),
 *   N_WLgridVar-nodes are printed as '==>' (fitted) and '=>>' (unfitted)
 *   respectively.
 *
 ******************************************************************************/

node *
PrintWLgridx (node *arg_node, node *arg_info)
{
    char *str = (NODE_TYPE (arg_node) == N_WLgrid) ? "-" : "=";

    DBUG_ENTER ("PrintWLgridx");

    INDENT;
    fprintf (outfile, "(");
    NodeOrInt_Print (outfile, NODE_TYPE (arg_node), WLGRIDX_GET_ADDR (arg_node, BOUND1),
                     WLGRIDX_DIM (arg_node));
    fprintf (outfile, " %s%s> ", str, WLGRIDX_FITTED (arg_node) ? str : ">");
    NodeOrInt_Print (outfile, NODE_TYPE (arg_node), WLGRIDX_GET_ADDR (arg_node, BOUND2),
                     WLGRIDX_DIM (arg_node));
    fprintf (outfile, "):");

    if (WLGRIDX_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        Trav (WLGRIDX_NEXTDIM (arg_node), arg_info);
        indent--;
    } else {
        if ((WLGRIDX_CODE (arg_node) != NULL) || (!WLGRIDX_NOOP (arg_node))) {
            PrintWLcode (WLGRIDX_CODE (arg_node), arg_info);
        }
        if (WLGRIDX_NOOP (arg_node)) {
            fprintf (outfile, " /* noop */");
        }
        fprintf (outfile, "\n");
    }

    if (WLGRIDX_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (WLGRIDX_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintCWrapper( node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_cwrapper nodes to generate C interface files.
 *
 ******************************************************************************/

node *
PrintCWrapper (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCWrapper");

    if (compiler_phase != PH_genccode) {
        /* internal debug output of mapping-tree of wrappers */
        DBUG_EXECUTE ("PRINT_CWRAPPER", {
            nodelist *funlist;
            node *fundef;
            char *type_str;

            fprintf (outfile, "CWrapper %s with %d arg(s) and %d result(s)\n",
                     CWRAPPER_NAME (arg_node), CWRAPPER_ARGCOUNT (arg_node),
                     CWRAPPER_RESCOUNT (arg_node));

            funlist = CWRAPPER_FUNS (arg_node);
            while (funlist != NULL) {
                fundef = NODELIST_NODE (funlist);

                fprintf (outfile, "  overloaded for (");
                /* print args of function */
                if (FUNDEF_ARGS (fundef) != NULL) {
                    Trav (FUNDEF_ARGS (fundef), arg_info);
                }

                fprintf (outfile, ") -> (");

                /* print results of function */
                type_str = Type2String (FUNDEF_TYPES (fundef), 0, TRUE);
                fprintf (outfile, "%s", type_str);
                type_str = Free (type_str);

                fprintf (outfile, ")\n");
                funlist = NODELIST_NEXT (funlist);
            }

            fprintf (outfile, "\n\n");

            if (CWRAPPER_NEXT (arg_node) != NULL) {
                PRINT_CONT (Trav (CWRAPPER_NEXT (arg_node), arg_info), ;);
            }
        });
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintSSAcnt( node *arg_node, node *arg_info)
 *
 * description:
 *   Prints list of SSA rename counters (for debug only).
 *
 ******************************************************************************/

node *
PrintSSAcnt (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintSSAcnt");

    INDENT;
    fprintf (outfile, " *  ");
    PRINT_POINTER_BRACKETS (outfile, arg_node);
    fprintf (outfile, " baseid = %s, counter = %d\n", SSACNT_BASEID (arg_node),
             SSACNT_COUNT (arg_node));

    if (SSACNT_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (SSACNT_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintCSEinfo( node *arg_node, node *arg_info);
 *
 * description:
 *   Prints sets of available common subexpressions (debug only).
 *
 ******************************************************************************/

node *
PrintCSEinfo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCSEinfo");

    /* to be implemented */

    if (CSEINFO_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (CSEINFO_NEXT (arg_node), arg_info), ;);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintAvis( node *arg_node, node *arg_info)
 *
 * description:
 *   Prints elements of avis node connected to vardec or arg.
 *
 ******************************************************************************/

node *
PrintAvis (node *arg_node, node *arg_info)
{
    bool do_it = FALSE;

    DBUG_ENTER ("PrintAvis");

    /* to be implemented */

    DBUG_EXECUTE ("PRINT_AVIS", do_it = TRUE;);

    if (do_it) {
        fprintf (outfile, " /* AVIS:");

        fprintf (outfile, " TYPE   = %s,",
                 TYType2String (AVIS_TYPE (arg_node), FALSE, 0));
        fprintf (outfile, " SSACNT = ");
        PRINT_POINTER_BRACKETS (outfile, AVIS_SSACOUNT (arg_node));
#if 1
        if (valid_ssaform && (AVIS_SSACOUNT (arg_node) != NULL)) {
            node *cnt = AVIS_SSACOUNT (arg_node);

            fprintf (outfile, " (baseid = %s, counter = %d)", SSACNT_BASEID (cnt),
                     SSACNT_COUNT (cnt));
        }
#endif

        fprintf (outfile, " */ ");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintInfo( node *arg_node, node *arg_info)
 *
 * description:
 *   N_info node found -> ERROR!!
 *
 ******************************************************************************/

node *
PrintInfo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintInfo");

    DBUG_ASSERT ((0), "N_info node found for printing!!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintTrav( node *syntax_tree, node *arg_info)
 *
 * description:
 *   initiates print of (sub-)tree
 *
 ******************************************************************************/

static node *
PrintTrav (node *syntax_tree, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("PrintTrav");

    /*
     * Save act_tab to restore it afterwards.
     * This could be useful if Print() is called from inside a debugger.
     */
    old_tab = act_tab;
    act_tab = print_tab;
    indent = 0;

    if (compiler_phase == PH_genccode) {
        switch (linkstyle) {
        case 0:
            /*
             * The current file is a SAC program.
             * Therefore, the C file is generated within the target directory.
             */
            outfile = WriteOpen ("%s%s", targetdir, cfilename);
            NOTE (("Writing file \"%s%s\"", targetdir, cfilename));
            GSCPrintFileHeader (syntax_tree);
            Trav (syntax_tree, arg_info);
            GSCPrintMain ();
            fclose (outfile);
            break;

        case 1:
            /*
             * The current file is a module/class implementation, but the functions
             * are nevertheless not compiled separatly to the archive.
             * Therefore, the C file is generated within the temprorary directory.
             */
            outfile = WriteOpen ("%s/%s", tmp_dirname, cfilename);
            NOTE (("Writing file \"%s%s\"", targetdir, cfilename));
            GSCPrintFileHeader (syntax_tree);
            Trav (syntax_tree, arg_info);
            fclose (outfile);
            break;

        case 2:
            /*
             * The current file is a module/class implementation. The functions and
             * global objects are all printed to separate files allowing for
             * separate compilation and the building of an archive. An additional
             * header file is generated for global variable and type declarations
             * as well as function prototypes.
             */
            INFO_PRINT_SEPARATE (arg_info) = 1;
            Trav (syntax_tree, arg_info);
            INFO_PRINT_SEPARATE (arg_info) = 0;
            break;

        default:
            DBUG_ASSERT ((0), "illegal link style found!");
            break;
        }
    } else {
        outfile = stdout;
        fprintf (outfile, "\n-----------------------------------------------\n");
        Trav (syntax_tree, arg_info);
        fprintf (outfile, "\n-----------------------------------------------\n");
    }

    act_tab = old_tab;

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *PrintTravPre( node *arg_node, node *arg_info)
 *
 * description:
 *   This function is called before the traversal of each node.
 *
 ******************************************************************************/

node *
PrintTravPre (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintTravPre");

    DBUG_PRINT ("PRINT_LINE", ("line (%s) %s:%i\n", mdb_nodetype[NODE_TYPE (arg_node)],
                               NODE_FILE (arg_node), NODE_LINE (arg_node)));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintTravPost( node *arg_node, node *arg_info)
 *
 * description:
 *   This function is called after the traversal of each node.
 *
 ******************************************************************************/

node *
PrintTravPost (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintTravPost");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *Print( node *syntax_tree)
 *
 * description:
 *   Prints the whole (sub-) tree behind the given node.
 *
 ******************************************************************************/

node *
Print (node *syntax_tree)
{
    node *arg_info;

    DBUG_ENTER ("Print");

    arg_info = MakeInfo ();
    /* we want to duplicate all sons */
    INFO_PRINT_CONT (arg_info) = NULL;

    INFO_PRINT_DIM (arg_info) = 0;

    syntax_tree = PrintTrav (syntax_tree, arg_info);

    arg_info = Free (arg_info);

    /* if generating c library, invoke the headerfile generator */
    if ((generatelibrary & GENERATELIBRARY_C) && (compiler_phase == PH_genccode)) {
        PrintInterface (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNode( node *syntax_tree)
 *
 * description:
 *   Prints the given node without next node.
 *
 ******************************************************************************/

node *
PrintNode (node *syntax_tree)
{
    node *arg_info;

    DBUG_ENTER ("PrintNode");

    arg_info = MakeInfo ();
    /* we want to duplicate all sons */
    INFO_PRINT_CONT (arg_info) = syntax_tree;

    INFO_PRINT_DIM (arg_info) = 0;

    syntax_tree = PrintTrav (syntax_tree, arg_info);

    Free (arg_info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 *  debug output
 */

/******************************************************************************
 *
 * Function:
 *   void DoIndentAST( void);
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
DoIndentAST (void)
{
    int j;

    DBUG_ENTER ("DoIndentAST");

    for (j = 0; j < indent; j++) {
        if (j % 4) {
            fprintf (outfile, "  ");
        } else {
            fprintf (outfile, "| ");
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void DoPrintDFMaskAST( DFMmask_t mask)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
DoPrintDFMaskAST (DFMmask_t mask)
{
    DBUG_ENTER ("DoPrintDFMaskAST");

    PRINT_POINTER_BRACKETS (outfile, mask);
    if (mask != NULL) {
        fprintf (outfile, "/");
        PRINT_POINTER_BRACKETS (outfile, DFMGetMaskBase (mask));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void DoPrintShapeAST( int dim, shpseg *shape)
 *
 * description:
 *
 *
 ******************************************************************************/

static void
DoPrintShapeAST (int dim, shpseg *shape)
{
    int d;

    if (dim != SCALAR) {
        fprintf (outfile, "[");
        if (dim == ARRAY_OR_SCALAR) {
            fprintf (outfile, "?");
        } else if (KNOWN_SHAPE (dim)) {
            fprintf (outfile, "%i", SHPSEG_SHAPE (shape, 0));
            for (d = 1; d < dim; d++) {
                fprintf (outfile, ",%i", SHPSEG_SHAPE (shape, d));
            }
        } else if (KNOWN_DIMENSION (dim)) {
            fprintf (outfile, ".");
            for (d = 1; d < dim; d++) {
                fprintf (outfile, ",.");
            }
        }
        fprintf (outfile, "]");
    }
}

/******************************************************************************
 *
 * function:
 *   void DoPrintBasicTypeAST( types *type)
 *
 * description:
 *   Prints a basic types-structure.
 *   This function is called from 'DoPrintAllTypesAST' only.
 *
 ******************************************************************************/

static void
DoPrintBasicTypeAST (types *type)
{
    shpseg *shape;
    int dim;

    DBUG_ENTER ("DoPrintBasicTypeAST");

    fprintf (outfile, "%s", type_string[GetBasetype (type)]);
    shape = Type2Shpseg (type, &dim);
    DoPrintShapeAST (dim, shape);
    if (shape != NULL) {
        shape = FreeShpseg (shape);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void DoPrintAllTypesAST( types *type, bool print_status, bool print_addr)
 *
 * Description:
 *   Prints a single types-structure.
 *   This function is called from 'DoPrintTypesAST' only.
 *
 ******************************************************************************/

static void
DoPrintAllTypesAST (types *type, bool print_status, bool print_addr)
{
    DBUG_ENTER ("DoPrintAllTypesAST");

    if (print_addr) {
        PRINT_POINTER_BRACKETS (outfile, type);
    }

    if (TYPES_BASETYPE (type) == T_user) {
        PRINT_STRING (outfile, TYPES_NAME (type));
        DoPrintShapeAST (TYPES_DIM (type), TYPES_SHPSEG (type));

        fprintf (outfile, "/");

        if (TYPES_TDEF (type) != NULL) {
            DoPrintBasicTypeAST (type);
        }
        PRINT_POINTER_BRACKETS (outfile, TYPES_TDEF (type));
    } else {
        DoPrintBasicTypeAST (type);
    }

    if (print_status) {
        PrintStatus (TYPES_STATUS (type), TRUE);
    }

    if (TYPES_NEXT (type) != NULL) {
        fprintf (outfile, " ");
        DoPrintAllTypesAST (TYPES_NEXT (type), print_status, print_addr);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void DoPrintTypesAST( types *type, bool print_status, bool print_addr)
 *
 * Description:
 *   Prints all the data of a types-structure.
 *
 ******************************************************************************/

static void
DoPrintTypesAST (types *type, bool print_status, bool print_addr)
{
    DBUG_ENTER ("DoPrintTypesAST");

    if (type != NULL) {
        if (TYPES_NEXT (type) != NULL) {
            fprintf (outfile, "{ ");
        }
        DoPrintAllTypesAST (type, print_status, print_addr);
        if (TYPES_NEXT (type) != NULL) {
            fprintf (outfile, " }");
        }
    } else {
        fprintf (outfile, "-");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void DoPrintIdsAST( ids *vars, bool print_status)
 *
 * description:
 *   Prints a 'ids'-chain.
 *   This function is called from 'DoPrintAST' only.
 *
 ******************************************************************************/

static void
DoPrintIdsAST (ids *vars, bool print_status)
{
    DBUG_ENTER ("DoPrintIdsAST");

    fprintf (outfile, "{ ");
    while (vars != NULL) {
        if (IDS_VARDEC (vars) != NULL) {
            DoPrintTypesAST (IDS_TYPE (vars), TRUE, FALSE);
            PRINT_POINTER_BRACKETS (outfile, IDS_VARDEC (vars));
        } else {
            fprintf (outfile, "/*no decl*/");
        }
        fprintf (outfile, " ");

        PRINT_STRING (outfile, IDS_NAME (vars));

        if (print_status) {
            PrintStatus (IDS_ATTRIB (vars), TRUE);
            PrintStatus (IDS_STATUS (vars), TRUE);
        }

        PrintRC (IDS_REFCNT (vars), IDS_NAIVE_REFCNT (vars), TRUE);

        fprintf (outfile, " ");
        vars = IDS_NEXT (vars);
        if (vars != NULL) {
            fprintf (outfile, ", ");
        }
    }
    fprintf (outfile, "}");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void DoPrintNodelistAST( nodelist *nl)
 *
 * description:
 *   Prints a Nodelist.
 *   This function is called from 'DoPrintAST' only.
 *
 ******************************************************************************/

static void
DoPrintNodelistAST (nodelist *nl)
{
    DBUG_ENTER ("DoPrintNodelistAST");

    fprintf (outfile, "{ ");
    while (nl != NULL) {
        PRINT_POINTER_BRACKETS (outfile, NODELIST_NODE (nl));

        fprintf (outfile, " ");
        nl = NODELIST_NEXT (nl);
        if (nl != NULL) {
            fprintf (outfile, ", ");
        }
    }
    fprintf (outfile, "}");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void DoPrintAttrAST( int num, node *arg_node)
 *
 * description:
 *   Prints an attribute containing a node-pointer.
 *   This function is called from 'DoPrintAST' only.
 *
 ******************************************************************************/

static void
DoPrintAttrAST (int num, node *arg_node)
{
    DBUG_ENTER ("DoPrintAttrAST");

    DoIndentAST ();

    if (num >= 0) {
        fprintf (outfile, "%i-", num);
    } else {
        fprintf (outfile, "+-");
    }

    PRINT_POINTER (outfile, arg_node);
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/* forward declaration */
static void DoPrintAST (node *arg_node, bool skip_next, bool print_attr);

/******************************************************************************
 *
 * function:
 *   void DoPrintSonAST( int num, node *arg_node,
 *                       bool next_node, bool print_attr)
 *
 * description:
 *   Prints a son containing a hole sub-tree.
 *   This function is called from 'DoPrintAST' only.
 *
 ******************************************************************************/

static void
DoPrintSonAST (int num, node *arg_node, bool next_node, bool print_attr)
{
    DBUG_ENTER ("DoPrintSonAST");

    DoIndentAST ();

    if (num >= 0) {
        fprintf (outfile, "%i=", num);
    } else {
        fprintf (outfile, "+=");
    }

    if (next_node) {
        fprintf (outfile, "(NEXT)\n");
    } else {
        /* inner node -> do not skip NEXT-nodes */
        DoPrintAST (arg_node, FALSE, print_attr);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void DoPrintAST( node *arg_node, bool skip_next, bool print_attr)
 *
 * description:
 *   This function prints the syntax tree without any interpretation.
 *   Some attribues of interest are printed within parentheses behind
 *   the node name.
 *
 ******************************************************************************/

static void
DoPrintAST (node *arg_node, bool skip_next, bool print_attr)
{
    node *next_node;
    bool new_line;
    int i;

    DBUG_ENTER ("DoPrintAST");

    next_node = NULL;

    outfile = stdout;

    if (arg_node) {
        new_line = TRUE;

        /* print node name */
        fprintf (outfile, "%s", mdb_nodetype[NODE_TYPE (arg_node)]);
        PRINT_POINTER_BRACKETS (outfile, arg_node);
        fprintf (outfile, "  ");

        /* print additional information to nodes */
        switch (NODE_TYPE (arg_node)) {
        case N_typedef:
            fprintf (outfile, "(");

            DoPrintTypesAST (TYPEDEF_TYPE (arg_node), TRUE, FALSE);
            fprintf (outfile, " ");

            PRINT_STRING (outfile, TYPEDEF_NAME (arg_node));

            PrintStatus (TYPEDEF_ATTRIB (arg_node), TRUE);
            PrintStatus (TYPEDEF_STATUS (arg_node), TRUE);

            fprintf (outfile, ")");

            next_node = TYPEDEF_NEXT (arg_node);
            break;

        case N_objdef:
            fprintf (outfile, "(");

            DoPrintTypesAST (OBJDEF_TYPE (arg_node), TRUE, FALSE);
            fprintf (outfile, " ");

            PRINT_STRING (outfile, OBJDEF_NAME (arg_node));

            PrintStatus (OBJDEF_ATTRIB (arg_node), TRUE);
            PrintStatus (OBJDEF_STATUS (arg_node), TRUE);

            fprintf (outfile, ")");

            next_node = OBJDEF_NEXT (arg_node);
            break;

        case N_fundef:
            fprintf (outfile, "(");

            DoPrintTypesAST (FUNDEF_TYPES (arg_node), TRUE, TRUE);
            fprintf (outfile, " ");

            PRINT_STRING (outfile, FUNDEF_MOD (arg_node));
            fprintf (outfile, ":");
            PRINT_STRING (outfile, FUNDEF_NAME (arg_node));

            PrintStatus (FUNDEF_ATTRIB (arg_node), TRUE);
            PrintStatus (FUNDEF_STATUS (arg_node), TRUE);

            fprintf (outfile, ", ");
            fprintf (outfile, "argtab: ");
            PrintArgtab (FUNDEF_ARGTAB (arg_node), TRUE);

            fprintf (outfile, ", ");
            fprintf (outfile, "return: ");
            PRINT_POINTER_BRACKETS (outfile, FUNDEF_RETURN (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "mask base: ");
            PRINT_POINTER_BRACKETS (outfile, FUNDEF_DFM_BASE (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "used: ");
            if (FUNDEF_USED (arg_node) != USED_INACTIVE) {
                fprintf (outfile, "%d", FUNDEF_USED (arg_node));
            } else {
                fprintf (outfile, "-");
            }

            if (FUNDEF_IS_LACFUN (arg_node)) {
                fprintf (outfile, ", ");
                fprintf (outfile, "int assign: ");
                PRINT_POINTER_BRACKETS (outfile, FUNDEF_INT_ASSIGN (arg_node));

                fprintf (outfile, ", ");
                fprintf (outfile, "ext assigns: ");
                DoPrintNodelistAST (FUNDEF_EXT_ASSIGNS (arg_node));
            }

            fprintf (outfile, ")");

            next_node = FUNDEF_NEXT (arg_node);
            break;

        case N_pragma:
            if (PRAGMA_WLCOMP_APS (arg_node) != NULL) {
                fprintf (outfile, "(wlcomp)\n");
                indent++;
                DoPrintSonAST (0, PRAGMA_WLCOMP_APS (arg_node), FALSE, FALSE);
                indent--;
                fprintf (outfile, "/* end of pragma */");
            }
            break;

        case N_avis:
            fprintf (outfile, "(");

            fprintf (outfile, ")");
            break;

        case N_arg:
            fprintf (outfile, "(");

            DoPrintTypesAST (ARG_TYPE (arg_node), TRUE, FALSE);
            fprintf (outfile, " ");

            PRINT_STRING (outfile, ARG_NAME (arg_node));

            PrintStatus (ARG_ATTRIB (arg_node), TRUE);
            PrintStatus (ARG_STATUS (arg_node), TRUE);

            PrintRC (ARG_REFCNT (arg_node), ARG_NAIVE_REFCNT (arg_node), TRUE);

            fprintf (outfile, ", ");
            fprintf (outfile, "varno: %i", ARG_VARNO (arg_node));

            fprintf (outfile, ")");

            next_node = ARG_NEXT (arg_node);
            break;

        case N_vardec:
            fprintf (outfile, "(");

            DoPrintTypesAST (VARDEC_TYPE (arg_node), TRUE, FALSE);
            fprintf (outfile, " ");

            PRINT_STRING (outfile, VARDEC_NAME (arg_node));

            PrintStatus (VARDEC_ATTRIB (arg_node), TRUE);
            PrintStatus (VARDEC_STATUS (arg_node), TRUE);

            PrintRC (VARDEC_REFCNT (arg_node), VARDEC_NAIVE_REFCNT (arg_node), TRUE);

            fprintf (outfile, ", ");
            fprintf (outfile, "varno: %i", VARDEC_VARNO (arg_node));

            fprintf (outfile, ")");

            next_node = VARDEC_NEXT (arg_node);
            break;

        case N_exprs:
            next_node = EXPRS_NEXT (arg_node);
            break;

        case N_vinfo:
            next_node = VINFO_NEXT (arg_node);
            break;

        case N_assign:
            next_node = ASSIGN_NEXT (arg_node);
            break;

        case N_cond:
            fprintf (outfile, "(");

            fprintf (outfile, "in-mask: ");
            DoPrintDFMaskAST (COND_IN_MASK (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "out-mask: ");
            DoPrintDFMaskAST (COND_OUT_MASK (arg_node));
            fprintf (outfile, "/");
            DoPrintDFMaskAST (COND_OUT_MASK (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "local-mask: ");
            DoPrintDFMaskAST (COND_LOCAL_MASK (arg_node));
            fprintf (outfile, "/");
            DoPrintDFMaskAST (COND_LOCAL_MASK (arg_node));

            fprintf (outfile, ")");
            break;

        case N_do:
            fprintf (outfile, "(");

            fprintf (outfile, "in-mask: ");
            DoPrintDFMaskAST (DO_IN_MASK (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "out-mask: ");
            DoPrintDFMaskAST (DO_OUT_MASK (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "local-mask: ");
            DoPrintDFMaskAST (DO_LOCAL_MASK (arg_node));

            fprintf (outfile, ")");
            break;

        case N_while:
            fprintf (outfile, "(");

            fprintf (outfile, "in-mask: ");
            DoPrintDFMaskAST (WHILE_IN_MASK (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "out-mask: ");
            DoPrintDFMaskAST (WHILE_OUT_MASK (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "local-mask: ");
            DoPrintDFMaskAST (WHILE_LOCAL_MASK (arg_node));

            fprintf (outfile, ")");
            break;

        case N_let:
            DoPrintIdsAST (LET_IDS (arg_node), TRUE);
            break;

        case N_ap:
            fprintf (outfile, "(");

            PRINT_STRING (outfile, AP_NAME (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "fundef: ");
            PRINT_POINTER_BRACKETS (outfile, AP_FUNDEF (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "argtab: ");
            PrintArgtab (AP_ARGTAB (arg_node), FALSE);

            fprintf (outfile, ")");
            break;

        case N_prf:
            fprintf (outfile, "(");

            fprintf (outfile, "%s", mdb_prf[PRF_PRF (arg_node)]);

            fprintf (outfile, ")");
            break;

        case N_id:
            fprintf (outfile, "(");

            if (ID_VARDEC (arg_node) != NULL) {
                DoPrintTypesAST (ID_TYPE (arg_node), TRUE, FALSE);
                PRINT_POINTER_BRACKETS (outfile, ID_VARDEC (arg_node));
            } else {
                fprintf (outfile, "/*no decl*/");
            }
            fprintf (outfile, " ");

            PRINT_STRING (outfile, ID_NAME (arg_node));

            PrintFlag (FLAG2STRING (ID, arg_node, IS_GLOBAL), TRUE);
            PrintFlag (FLAG2STRING (ID, arg_node, IS_REFERENCE), TRUE);
            PrintFlag (FLAG2STRING (ID, arg_node, IS_READ_ONLY), TRUE);
            PrintStatus (ID_STATUS (arg_node), TRUE);

            PrintRC (ID_REFCNT (arg_node), ID_NAIVE_REFCNT (arg_node), TRUE);

            if ((ID_AVIS (arg_node) != NULL)
                && (AVIS_SSACONST (ID_AVIS (arg_node)) != NULL)) {
                fprintf (outfile, " ");
                COPrintConstant (outfile, AVIS_SSACONST (ID_AVIS (arg_node)));
            }

            fprintf (outfile, ")");
            break;

        case N_num:
            fprintf (outfile, "(");

            fprintf (outfile, "%i", NUM_VAL (arg_node));

            fprintf (outfile, ")");
            break;

        case N_array:
            fprintf (outfile, "(");

            DoPrintTypesAST (ARRAY_TYPE (arg_node), TRUE, FALSE);

            fprintf (outfile, ")");
            break;

        case N_icm:
            fprintf (outfile, "(");

            fprintf (outfile, "%s", ICM_NAME (arg_node));

            fprintf (outfile, ")");
            break;

        case N_Nwith:
            fprintf (outfile, "(");

            fprintf (outfile, "in-mask: ");
            DoPrintDFMaskAST (NWITH_IN_MASK (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "out-mask: ");
            DoPrintDFMaskAST (NWITH_OUT_MASK (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "local-mask: ");
            DoPrintDFMaskAST (NWITH_LOCAL_MASK (arg_node));

            fprintf (outfile, ")");

            fprintf (outfile, "\n");
            if (NWITH_PRAGMA (arg_node) != NULL) {
                indent++;
                DoPrintSonAST (-1, NWITH_PRAGMA (arg_node), FALSE, FALSE);
                indent--;

                new_line = FALSE;
            }
            break;

        case N_Nwithid:
            fprintf (outfile, "(");

            if (NWITHID_VEC (arg_node) != NULL) {
                char *withid = IDS_NAME (NWITHID_VEC (arg_node));

                PRINT_STRING (outfile, withid);

                PrintRC (IDS_REFCNT (NWITHID_VEC (arg_node)),
                         IDS_NAIVE_REFCNT (NWITHID_VEC (arg_node)), TRUE);
            } else {
                fprintf (outfile, "?");
            }
            fprintf (outfile, " = ");
            if (NWITHID_IDS (arg_node) != NULL) {
                DoPrintIdsAST (NWITHID_IDS (arg_node), TRUE);
            } else {
                fprintf (outfile, "?");
            }

            fprintf (outfile, ")");
            break;

        case N_Nwithop:
            fprintf (outfile, "(");

            if (NWITHOP_TYPE (arg_node) == WO_foldprf) {
                fprintf (outfile, "foldprf: %s", prf_string[NWITHOP_PRF (arg_node)]);
            } else if (NWITHOP_TYPE (arg_node) == WO_foldfun) {
                fprintf (outfile, "foldfun: ");
                if (NWITHOP_FUNDEF (arg_node) != NULL) {
                    fprintf (outfile, "%s", FUNDEF_NAME (NWITHOP_FUNDEF (arg_node)));
                    PRINT_POINTER_BRACKETS (outfile, NWITHOP_FUNDEF (arg_node));
                } else {
                    fprintf (outfile, "?");
                }
            } else if (NWITHOP_TYPE (arg_node) == WO_genarray) {
                fprintf (outfile, "genarray");
            } else {
                DBUG_ASSERT ((NWITHOP_TYPE (arg_node) == WO_modarray),
                             "illegal NWITHOP_TYPE found!");
                fprintf (outfile, "modarray");
            }

            fprintf (outfile, ")");
            break;

        case N_Npart:
            fprintf (outfile, "(");

            fprintf (outfile, "code: ");
            PRINT_POINTER_BRACKETS (outfile, NPART_CODE (arg_node));

            fprintf (outfile, ")");

            next_node = NPART_NEXT (arg_node);
            break;

        case N_Ncode:
            fprintf (outfile, "(");

            PRINT_POINTER_BRACKETS (outfile, arg_node);

            fprintf (outfile, ", ");
            fprintf (outfile, "used: %d", NCODE_USED (arg_node));

            fprintf (outfile, ")");

            next_node = NCODE_NEXT (arg_node);
            break;

        case N_Nwith2:
            fprintf (outfile, "(");

            fprintf (outfile, "in-mask: ");
            DoPrintDFMaskAST (NWITH2_IN_MASK (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "out-mask: ");
            DoPrintDFMaskAST (NWITH2_OUT_MASK (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "local-mask: ");
            DoPrintDFMaskAST (NWITH2_LOCAL_MASK (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "offset needed: %i", NWITH2_OFFSET_NEEDED (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "mt: %i", NWITH2_MT (arg_node));

            fprintf (outfile, ")");
            break;

        case N_WLseg:
            fprintf (outfile, "(");

            for (i = 0; i < WLSEG_BLOCKS (arg_node); i++) {
                fprintf (outfile, "bv%i: ", i);
                PRINT_VECT (outfile, WLSEG_BV (arg_node, i), WLSEG_DIMS (arg_node), "%i");
                fprintf (outfile, ", ");
            }

            fprintf (outfile, "ubv: ");
            PRINT_VECT (outfile, WLSEG_UBV (arg_node), WLSEG_DIMS (arg_node), "%i");

            fprintf (outfile, ", ");
            fprintf (outfile, "idx min: ");
            WLSEG_IDX_PRINT (outfile, arg_node, IDX_MIN);

            fprintf (outfile, ", ");
            fprintf (outfile, "idx max: ");
            WLSEG_IDX_PRINT (outfile, arg_node, IDX_MAX);

            fprintf (outfile, ", ");
            fprintf (outfile, "sv: ");
            PRINT_VECT (outfile, WLSEG_SV (arg_node), WLSEG_DIMS (arg_node), "%i");

            fprintf (outfile, ", ");
            fprintf (outfile, "homsv: ");
            PRINT_VECT (outfile, WLSEG_HOMSV (arg_node), WLSEG_DIMS (arg_node), "%i");

            fprintf (outfile, ", ");
            fprintf (outfile, "scheduler: ");
            SCHPrintScheduling (outfile, WLSEG_SCHEDULING (arg_node));

            fprintf (outfile, ")");

            next_node = WLSEG_NEXT (arg_node);
            break;

        case N_WLsegVar:
            fprintf (outfile, "(");

            fprintf (outfile, "idx min: ");
            WLSEGVAR_IDX_PRINT (outfile, arg_node, IDX_MIN);

            fprintf (outfile, ", ");
            fprintf (outfile, "idx max: ");
            WLSEGVAR_IDX_PRINT (outfile, arg_node, IDX_MAX);

            fprintf (outfile, ", ");
            fprintf (outfile, "scheduler: ");
            SCHPrintScheduling (outfile, WLSEGVAR_SCHEDULING (arg_node));

            fprintf (outfile, ")");

            next_node = WLSEGVAR_NEXT (arg_node);
            break;

        case N_WLblock:
            fprintf (outfile, "(");

            fprintf (outfile, "%d->%d block%d[%d] %d", WLBLOCK_BOUND1 (arg_node),
                     WLBLOCK_BOUND2 (arg_node), WLBLOCK_LEVEL (arg_node),
                     WLBLOCK_DIM (arg_node), WLBLOCK_STEP (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "noop: %i", WLBLOCK_NOOP (arg_node));

            fprintf (outfile, ")");

            next_node = WLBLOCK_NEXT (arg_node);
            break;

        case N_WLublock:
            fprintf (outfile, "(");

            fprintf (outfile, "%d->%d ublock%d[%d] %d", WLUBLOCK_BOUND1 (arg_node),
                     WLUBLOCK_BOUND2 (arg_node), WLUBLOCK_LEVEL (arg_node),
                     WLUBLOCK_DIM (arg_node), WLUBLOCK_STEP (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "noop: %i", WLUBLOCK_NOOP (arg_node));

            fprintf (outfile, ")");

            next_node = WLUBLOCK_NEXT (arg_node);
            break;

        case N_WLstride:
            fprintf (outfile, "(");

            fprintf (outfile, "%d->%d step%d[%d] %d", WLSTRIDE_BOUND1 (arg_node),
                     WLSTRIDE_BOUND2 (arg_node), WLSTRIDE_LEVEL (arg_node),
                     WLSTRIDE_DIM (arg_node), WLSTRIDE_STEP (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "noop: %i", WLSTRIDE_NOOP (arg_node));

            fprintf (outfile, ")");

            next_node = WLSTRIDE_NEXT (arg_node);
            break;

        case N_WLstrideVar:
            fprintf (outfile, "(");

            NodeOrInt_Print (outfile, NODE_TYPE (arg_node),
                             &(WLSTRIDEVAR_BOUND1 (arg_node)),
                             WLSTRIDEVAR_DIM (arg_node));
            fprintf (outfile, "->");
            NodeOrInt_Print (outfile, NODE_TYPE (arg_node),
                             &(WLSTRIDEVAR_BOUND2 (arg_node)),
                             WLSTRIDEVAR_DIM (arg_node));
            fprintf (outfile, ", ");
            fprintf (outfile, "step%d[%d] ", WLSTRIDEVAR_LEVEL (arg_node),
                     WLSTRIDEVAR_DIM (arg_node));
            NodeOrInt_Print (outfile, NODE_TYPE (arg_node),
                             &(WLSTRIDEVAR_STEP (arg_node)), WLSTRIDEVAR_DIM (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "noop: %i", WLSTRIDEVAR_NOOP (arg_node));

            fprintf (outfile, ")");

            next_node = WLSTRIDEVAR_NEXT (arg_node);
            break;

        case N_WLgrid:
            fprintf (outfile, "(");

            fprintf (outfile, "%d->%d [%d]", WLGRID_BOUND1 (arg_node),
                     WLGRID_BOUND2 (arg_node), WLGRID_DIM (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "fitted: %i", WLGRID_FITTED (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "noop: %i", WLGRID_NOOP (arg_node));

            fprintf (outfile, ")");

            if (WLGRID_CODE (arg_node) != NULL) {
                fprintf (outfile, ": op");
            } else {
                fprintf (outfile, ": ..");
            }

            next_node = WLGRID_NEXT (arg_node);
            break;

        case N_WLgridVar:
            fprintf (outfile, "(");

            NodeOrInt_Print (outfile, NODE_TYPE (arg_node),
                             &(WLGRIDVAR_BOUND1 (arg_node)), WLGRIDVAR_DIM (arg_node));
            fprintf (outfile, "->");
            NodeOrInt_Print (outfile, NODE_TYPE (arg_node),
                             &(WLGRIDVAR_BOUND2 (arg_node)), WLGRIDVAR_DIM (arg_node));
            fprintf (outfile, " [%d]", WLGRIDVAR_DIM (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "fitted: %i", WLGRIDVAR_FITTED (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "noop: %i", WLGRIDVAR_NOOP (arg_node));

            fprintf (outfile, ")");

            next_node = WLGRIDVAR_NEXT (arg_node);
            break;

        default:
            break;
        }

        if (new_line) {
            fprintf (outfile, "\n");
        }

        indent++;
        for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++) {
            DoPrintSonAST (i, arg_node->node[i],
                           ((arg_node->node[i] != NULL) && skip_next
                            && (arg_node->node[i] == next_node)),
                           TRUE);
        }
        if (print_attr) {
            for (i = nnode[NODE_TYPE (arg_node)]; i < MAX_SONS; i++) {
                if (arg_node->node[i] != NULL) {
                    DoPrintAttrAST (i, arg_node->node[i]);
                }
            }
        }
        indent--;
    } else {
        fprintf (outfile, "NULL\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintAST( node *arg_node)
 *   void PrintNodeAST( node *arg_node)
 *   void PrintNtype( ntype *type)
 *   void PrintDebugNtype( ntype *type)
 *
 * description:
 *   These functions are for debug assistance.
 *   The first two print the syntax tree without any interpretation.
 *   Some attribues of interest are printed inside of parenthesizes behind
 *   the node name.
 *   PrintAST prints the whole chain, PrintNodeAST skips NEXT-sons.
 *
 *   The other two functions are used for printing types (ntype * nodes).
 *   PrintNtype prints the types as they would be printed for code generation,
 *   whereas PrintDebugNtype prints all internal details.
 *
 ******************************************************************************/

void
PrintAST (node *arg_node)
{
    DBUG_ENTER ("PrintAST");

    DoPrintAST (arg_node, FALSE, TRUE);

    DBUG_VOID_RETURN;
}

void
PrintNodeAST (node *arg_node)
{
    DBUG_ENTER ("PrintNodeAST");

    DoPrintAST (arg_node, TRUE, TRUE);

    DBUG_VOID_RETURN;
}

void
PrintNtype (ntype *type)
{
    char *tmp;

    DBUG_ENTER ("PrintNtype");

    outfile = stdout;

    tmp = TYType2String (type, TRUE, 0);
    fprintf (outfile, "%s\n", tmp);
    tmp = Free (tmp);

    DBUG_VOID_RETURN;
}

void
PrintDebugNtype (ntype *type)
{
    char *tmp;

    DBUG_ENTER ("PrintDebugNtype");

    outfile = stdout;

    tmp = TYType2DebugString (type, TRUE, 0);
    fprintf (outfile, "%s\n", tmp);
    tmp = Free (tmp);

    DBUG_VOID_RETURN;
}
