/*
 *
 * $Log$
 * Revision 3.38  2001/04/02 11:41:53  dkr
 * include of wltransform.h replaced by wl_bounds.h
 *
 * Revision 3.37  2001/03/29 14:46:29  dkr
 * NWITH2_SCHEDULING removed
 *
 * Revision 3.36  2001/03/29 01:35:51  dkr
 * WLSEGVAR_IDX_MIN, WLSEGVAR_IDX_MAX are now node-vectors
 *
 * Revision 3.35  2001/03/22 19:26:15  dkr
 * include of tree.h eliminated
 *
 * Revision 3.34  2001/03/22 14:07:35  sbs
 * F_abs now printed prefix rather than infix 8-)
 *
 * Revision 3.33  2001/03/21 17:12:08  dkr
 * PrintAST: address of ID_VARDEC, IDS_VARDEC printed
 *
 * Revision 3.32  2001/03/20 16:00:29  ben
 * DoPrintAST: WLSEGX_SCHEDULER is printed now
 *
 * Revision 3.31  2001/03/19 16:43:58  dkr
 * WLSEG_HOMSV removed (WLSEG_SV used instead)
 *
 * Revision 3.30  2001/03/15 20:35:16  dkr
 * PrintAssign: DBUG-string PRINT_PROFILE is handled correctly now
 *
 * Revision 3.29  2001/03/15 16:49:46  dkr
 * PrintArg: the '&' for reference objects is no longer printed by
 * Type2String().
 * Note, that types->attrib is *not* part of the virtual TYPES types.
 *
 * Revision 3.28  2001/03/14 17:27:08  nmw
 * DoPrintIdsAST: order of attributes modified
 *
 * Revision 3.27  2001/03/08 12:26:53  dkr
 * DBUG-string PRINT_PROFILE added:
 * prints profiling instructions in *all* phases
 *
 * Revision 3.26  2001/03/07 14:24:19  dkr
 * PrintAssign modified:
 * N_annotate nodes are printed after phase 20 only
 *
 * Revision 3.25  2001/03/05 15:11:12  dkr
 * NCODE_NO renamed into NCODE_ID
 * handling of NCODE_ID modified
 *
 * Revision 3.24  2001/03/05 13:32:50  dkr
 * NGEN_OP?_GEN removed
 *
 * Revision 3.23  2001/03/05 12:52:17  dkr
 * some debug output for PRINT_RC added
 *
 * Revision 3.22  2001/02/14 14:39:58  dkr
 * DoPrintStatusAST modified
 *
 * Revision 3.21  2001/02/14 10:22:13  dkr
 * PrintAST extended: FUNDEF_TYPES, ..._STATUS, ..._ATTRIB are printed
 * now, too
 *
 * Revision 3.20  2001/02/12 16:56:07  nmw
 * warning corrected
 *
 * Revision 3.19  2001/02/12 15:57:34  nmw
 * Print functions for N_cseinfo, N_ssacnt and N_avis added
 *
 * Revision 3.18  2001/02/07 21:14:52  dkr
 * space after keyword 'extern' added
 *
 * Revision 3.17  2001/02/07 20:15:03  dkr
 * fixed a bug in PrintWLgridx()
 *
 * Revision 3.16  2001/02/06 01:43:28  dkr
 * output for new with-loop modified
 * PrintAST... updated
 *
 * Revision 3.15  2001/02/02 09:45:02  dkr
 * minor changes in output for with-loop done
 *
 * Revision 3.14  2001/01/30 09:56:00  dkr
 * output for N_WLgrid(Var)-nodes modified
 *
 * Revision 3.13  2001/01/29 18:34:03  dkr
 * some superfluous attributes of N_WLsegVar removed
 *
 * Revision 3.12  2001/01/29 16:07:21  dkr
 * PrintWLstrideVar and PrintWLstride replaced by PrintWLstridex
 * PrintWLgridVar and PrintWLgrid replaced by PrintWLgridx
 *
 * Revision 3.11  2001/01/24 23:40:07  dkr
 * PrintWLseg and PrintWLsegVar replaced by PrintWLsegx
 * PrintWLblock and PrintWLublock replaced by PrintWLxblock
 *
 * Revision 3.10  2001/01/17 17:38:37  dkr
 * printing of dummy code changed (AP, naive compilation)
 *
 * Revision 3.9  2001/01/09 17:25:45  dkr
 * N_WLstriVar renamed into N_WLstrideVar
 *
 * Revision 3.8  2000/12/14 17:42:24  dkr
 * indentation warning deactivated for old MT
 *
 * Revision 3.7  2000/12/12 15:33:55  dkr
 * handling of ..._ICM sons corrected
 *
 * Revision 3.6  2000/12/12 12:29:00  dkr
 * nodes N_pre, N_post, N_inc, N_dec removed
 *
 * Revision 3.5  2000/12/06 11:42:51  dkr
 * PrintTravPre, PrintTravPost added
 * DBUG-string PRINT_LINE added: prints NODE_LINE for each node
 *
 * Revision 3.4  2000/12/04 13:32:58  dkr
 * DoPrintTypesAST extended
 *
 * Revision 3.3  2000/11/27 21:05:17  cg
 * Added print support for APL pragma in Nwith2 nodes.
 *
 * Revision 3.2  2000/11/23 16:08:29  sbs
 * definitions of DbugPrintArray, WLAAprintAccesses, and TSIprintInfo
 * enclosed in ifndef DBUG_OFF to avoid warnings in product version!
 *
 * Revision 3.1  2000/11/20 17:59:45  sacbase
 * new release made
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
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "print.h"
#include "print_interface.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "DataFlowMask.h"
#include "optimize.h"
#include "generatemasks.h"
#include "filemgr.h"
#include "globals.h"
#include "gen_startup_code.h"
#include "WithloopFolding.h"
#include "scheduling.h"
#include "wl_access_analyze.h"
#include "tile_size_inference.h"
#include "NameTuples.h"
#include "wl_bounds.h"

#define WARN_INDENT

/* format string for pointers */
#ifdef SAC_FOR_SOLARIS_SPARC
#define F_PTR "0x%p"
#else
#define F_PTR "%p"
#endif

/*
 * PrintNode(): INFO_PRINT_CONT(arg_info) contains the root of syntaxtree.
 *  -> traverses next-node if and only if its parent is not the root.
 * Print(): INFO_PRINT_CONT(arg_info) is NULL.
 *  -> traverses all next-nodes.
 *
 * This behaviour is implemented with the macro PRINT_CONT.
 */

#define PRINT_CONT(code_then, code_else)                                                 \
    if (arg_info != NULL) {                                                              \
        if (INFO_PRINT_CONT (arg_info) != arg_node) {                                    \
            code_then;                                                                   \
        } else {                                                                         \
            code_else;                                                                   \
        }                                                                                \
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

#define PRINT_LINE_PRAGMA_IN_SIB(file_handle, node)                                      \
    if (compiler_phase == PH_writesib) {                                                 \
        fprintf (file_handle, "# %d \"%s\"\n", NODE_LINE (node), NODE_FILE (node));      \
    }
/*
 * The selection of when to write a line pragma is actually not correctly
 * implemented because pragmas are now printed not only to the SIB but also
 * when stopping the compilation phase after having generated the SIB.
 * However, for the time being we keep this implementation for debugging
 * purposes.
 */

/******************************************************************************/

/*
 * First, we generate the external declarations for all functions that
 * expand ICMs to C.
 */

#define ICM_ALL
#define ICM_DEF(prf, trf) extern void Print##prf (node *ex, node *arg_info);
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_VAR(dim, name)
#define ICM_VARINT(dim, name)
#define ICM_END(prf, args)
#include "icm.data"
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_VARINT
#undef ICM_END
#undef ICM_ALL

/******************************************************************************/

#define PRF_IF(n, s, x, y) x

char *prf_string[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

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
 *   void WLAAprintAccesses(node* arg_node, node* arg_info)
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
    if ((feature & FEATURE_APSI) == FEATURE_APSI) {
        INDENT;
        fprintf (outfile, " *   primitive function psi with array return value\n");
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
        fprintf (outfile, " *   primitive function psi with unknown indexvector\n");
    }

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
                fprintf (outfile, "psi( %s ", VARDEC_NAME (ACCESS_IV (access)));
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
                                 STR_OR_NULL (VARDEC_NAME (ACCESS_ARRAY (access)), "?"));
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
                                 STR_OR_NULL (VARDEC_NAME (ACCESS_ARRAY (access)), "?"));
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
 *   void TSIprintInfo(node* arg_node, node* arg_info)
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
          = MakeExprs (MakeAp (ap_name, NULL, MakeExprs (MakeArray (aelems), NULL)),
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
    bool print_nt = FALSE;

    DBUG_ENTER ("PrintIds");

    if (arg != NULL) {
        DBUG_PRINT ("PRINT", ("%s", IDS_NAME (arg)));

        if (IDS_MOD (arg) != NULL) {
            fprintf (outfile, "%s:", IDS_MOD (arg));
        }

        DBUG_EXECUTE ("PRINT_NT", if (IDS_VARDEC (arg) != NULL) {
            PrintNT (outfile, IDS_NAME (arg), IDS_TYPE (arg));
            print_nt = TRUE;
        });

        if (!print_nt) {
            fprintf (outfile, "%s", IDS_NAME (arg));
        }

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

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

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
            Trav (MODUL_TYPES (arg_node), arg_info); /* print typedefs */
        }

        if (NULL != MODUL_FUNS (arg_node)) {
            fprintf (outfile, "\n\n");
            INFO_PRINT_PROTOTYPE (arg_info) = 1;
            Trav (MODUL_FUNS (arg_node), arg_info); /* print function declarations */
            INFO_PRINT_PROTOTYPE (arg_info) = 0;
        }

        if (NULL != MODUL_OBJS (arg_node)) {
            fprintf (outfile, "\n\n");
            print_objdef_for_header_file = 1;
            Trav (MODUL_OBJS (arg_node), arg_info); /* print object declarations */
        }

        fclose (outfile);

        outfile = WriteOpen ("%s/globals.c", tmp_dirname);
        fprintf (outfile, "#include \"header.h\"\n\n");
        fprintf (outfile, "int SAC__%s__dummy_value_which_is_completely_useless = 0;\n\n",
                 MODUL_NAME (arg_node));

        if (NULL != MODUL_OBJS (arg_node)) {
            fprintf (outfile, "\n\n");
            print_objdef_for_header_file = 0;
            Trav (MODUL_OBJS (arg_node), arg_info); /* print object definitions */
        }

        fclose (outfile);

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
            fprintf (outfile, "\n/*\n *  Module %s :\n */\n", MODUL_NAME (arg_node));
            break;
        case F_classimp:
            fprintf (outfile, "\n/*\n *  Class %s :\n", MODUL_NAME (arg_node));
            if (MODUL_CLASSTYPE (arg_node) != NULL) {
                type_str = Type2String (MODUL_CLASSTYPE (arg_node), 0, TRUE);
                fprintf (outfile, " *  classtype %s;\n", type_str);
                FREE (type_str);
            }
            fprintf (outfile, " */\n");
            break;
        case F_prog:
            fprintf (outfile, "\n/*\n *  SAC-Program %s :\n */\n", puresacfilename);
            break;
        default:
            break;
        }

        if (MODUL_IMPORTS (arg_node) != NULL) {
            fprintf (outfile, "\n");
            Trav (MODUL_IMPORTS (arg_node), arg_info); /* print import-list */
        }

        if (MODUL_TYPES (arg_node) != NULL) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  type definitions\n");
            fprintf (outfile, " */\n\n");
            Trav (MODUL_TYPES (arg_node), arg_info); /* print typedefs */
        }

        if (MODUL_FUNS (arg_node) != NULL) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  function declarations\n");
            fprintf (outfile, " */\n\n");
            INFO_PRINT_PROTOTYPE (arg_info) = 1;
            Trav (MODUL_FUNS (arg_node), arg_info); /* print function declarations*/
            INFO_PRINT_PROTOTYPE (arg_info) = 0;
        }

        if (MODUL_OBJS (arg_node) != NULL) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  global objects\n");
            fprintf (outfile, " */\n\n");
            Trav (MODUL_OBJS (arg_node), arg_info); /* print objdefs */
        }

        if (MODUL_FUNS (arg_node) != NULL) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  function definitions\n");
            fprintf (outfile, " */\n");
            Trav (MODUL_FUNS (arg_node), arg_info); /* print function definitions */
        }

        DBUG_EXECUTE ("PRINT_CWRAPPER", if (MODUL_CWRAPPER (arg_node) != NULL) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  c wrapper functions\n");
            fprintf (outfile, " */\n");
            Trav (MODUL_CWRAPPER (arg_node), arg_info); /* print wrapper mappings */
        });
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintImplist(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintImplist (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintImplist");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    fprintf (outfile, "import %s: ", IMPLIST_NAME (arg_node));

    if ((IMPLIST_ITYPES (arg_node) == NULL) && (IMPLIST_ETYPES (arg_node) == NULL)
        && (IMPLIST_FUNS (arg_node) == NULL) && (IMPLIST_OBJS (arg_node) == NULL)) {
        fprintf (outfile, "all;\n");
    } else {
        fprintf (outfile, "{");
        if (IMPLIST_ITYPES (arg_node) != NULL) {
            fprintf (outfile, "\n  implicit types: ");
            PrintIds (IMPLIST_ITYPES (arg_node),
                      arg_info); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (IMPLIST_ETYPES (arg_node) != NULL) {
            fprintf (outfile, "\n  explicit types: ");
            PrintIds (IMPLIST_ETYPES (arg_node),
                      arg_info); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (IMPLIST_OBJS (arg_node) != NULL) {
            fprintf (outfile, "\n  global objects: ");
            PrintIds (IMPLIST_OBJS (arg_node),
                      arg_info); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (IMPLIST_FUNS (arg_node) != NULL) {
            fprintf (outfile, "\n  funs: ");
            PrintIds (IMPLIST_FUNS (arg_node),
                      arg_info); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        fprintf (outfile, "\n}\n");
    }

    if (IMPLIST_NEXT (arg_node) != NULL) { /* print further imports */
        PRINT_CONT (Trav (IMPLIST_NEXT (arg_node), arg_info), );
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintTypedef(node *arg_node, node *arg_info)
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

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if ((TYPEDEF_ICM (arg_node) == NULL)
        || (NODE_TYPE (TYPEDEF_ICM (arg_node)) != N_icm)) {
        type_str = Type2String (TYPEDEF_TYPE (arg_node), 0, TRUE);
        fprintf (outfile, "typedef %s ", type_str);
        FREE (type_str);

        if (TYPEDEF_MOD (arg_node) != NULL) {
            fprintf (outfile, "%s:", TYPEDEF_MOD (arg_node));
        }
        fprintf (outfile, "%s", TYPEDEF_NAME (arg_node));

        PrintStatus (TYPEDEF_ATTRIB (arg_node), FALSE);
        PrintStatus (TYPEDEF_STATUS (arg_node), FALSE);

        fprintf (outfile, ";\n");
    } else {
        Trav (TYPEDEF_ICM (arg_node), arg_info);
        fprintf (outfile, ";\n");
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
        PRINT_CONT (Trav (TYPEDEF_NEXT (arg_node), arg_info), );
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintObjdef(node *arg_node, node *arg_info)
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

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

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
        FREE (type_str);

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
        PRINT_CONT (Trav (OBJDEF_NEXT (arg_node), arg_info), );
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   void PrintFunctionHeader( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
PrintFunctionHeader (node *arg_node, node *arg_info)
{
    types *ret_types;
    char *type_str;

    DBUG_ENTER ("PrintFunctionHeader");

    PRINT_LINE_PRAGMA_IN_SIB (outfile, arg_node);

    if (FUNDEF_INLINE (arg_node)) {
        fprintf (outfile, "inline ");
    }

    ret_types = FUNDEF_TYPES (arg_node);
    while (ret_types != NULL) {
        type_str = Type2String (ret_types, 0, FALSE);
        fprintf (outfile, "%s", type_str);
        FREE (type_str);

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

    fprintf (outfile, "%s", FUNDEF_NAME (arg_node));

    PrintStatus (FUNDEF_ATTRIB (arg_node), FALSE);
    PrintStatus (FUNDEF_STATUS (arg_node), FALSE);

    fprintf (outfile, "(");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        Trav (FUNDEF_ARGS (arg_node), arg_info); /* print args of function */
    }

    fprintf (outfile, ")");

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

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

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

        if (FUNDEF_STATUS (arg_node) != ST_spmdfun) {
            if ((FUNDEF_BODY (arg_node) == NULL)
                || ((FUNDEF_RETURN (arg_node) != NULL)
                    && (NODE_TYPE (FUNDEF_RETURN (arg_node)) == N_icm)
                    && (strcmp (FUNDEF_NAME (arg_node), "main") != 0))) {
                fprintf (outfile, "extern ");

                if ((FUNDEF_ICM (arg_node) == NULL)
                    || (NODE_TYPE (FUNDEF_ICM (arg_node)) != N_icm)) {
                    PrintFunctionHeader (arg_node, arg_info);
                } else {
                    Trav (FUNDEF_ICM (arg_node), arg_info); /* print N_icm ND_FUN_DEC */
                }

                fprintf (outfile, ";\n");
                DBUG_EXECUTE ("PRINT_FUNATR",
                              fprintf (outfile, "/* ATTRIB = %s */\n",
                                       mdb_statustype[FUNDEF_ATTRIB (arg_node)]););

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

        if (FUNDEF_BODY (arg_node) != NULL) {

            if (INFO_PRINT_SEPARATE (arg_info)) {
                outfile = WriteOpen ("%s/fun%d.c", tmp_dirname, function_counter);

                fprintf (outfile, "#include \"header.h\"\n");
            }

            fprintf (outfile, "\n");

            if ((FUNDEF_STATUS (arg_node) == ST_spmdfun)
                && (compiler_phase == PH_genccode)) {
                fprintf (outfile, "#if SAC_DO_MULTITHREAD\n\n");
            }

            DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "\n**MASKS - function: \n");
                          PrintDefMask (outfile, FUNDEF_DEFMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info));
                          PrintUseMask (outfile, FUNDEF_USEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info)););

            if ((FUNDEF_ICM (arg_node) == NULL)
                || (NODE_TYPE (FUNDEF_ICM (arg_node)) != N_icm)) {
                PrintFunctionHeader (arg_node, arg_info);
            } else {
                Trav (FUNDEF_ICM (arg_node), arg_info); /* print N_icm ND_FUN_DEC */
            }

            fprintf (outfile, "\n");
            DBUG_EXECUTE ("PRINT_FUNATR",
                          fprintf (outfile, "/* ATTRIB = %s */\n",
                                   mdb_statustype[FUNDEF_ATTRIB (arg_node)]););

            Trav (FUNDEF_BODY (arg_node), arg_info); /* traverse function body */

            if (FUNDEF_PRAGMA (arg_node) != NULL) {
                Trav (FUNDEF_PRAGMA (arg_node), arg_info);
            }

            if ((FUNDEF_STATUS (arg_node) == ST_spmdfun)
                && (compiler_phase == PH_genccode)) {
                fprintf (outfile, "\n#endif  /* SAC_DO_MULTITHREAD */\n\n");
            } else {
                fprintf (outfile, "\n");
            }

            if (INFO_PRINT_SEPARATE (arg_info)) {
                fclose (outfile);
                function_counter++;
            }
        }
    }

    if (indent != old_indent) {
#ifdef WARN_INDENT
        if (gen_mt_code != GEN_MT_OLD) {
            /*
             * for the time being (old) code for MT is always unbalanced :-(
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
        PRINT_CONT (Trav (FUNDEF_NEXT (arg_node), arg_info), );
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintAnnotate(node *arg_node, node *arg_info)
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

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if (ANNOTATE_TAG (arg_node) & CALL_FUN) {
        sprintf (strbuffer1, "PROFILE_BEGIN_UDF( %d, %d)", ANNOTATE_FUNNUMBER (arg_node),
                 ANNOTATE_FUNAPNUMBER (arg_node));
    } else {
        if (ANNOTATE_TAG (arg_node) & RETURN_FROM_FUN) {
            sprintf (strbuffer1, "PROFILE_END_UDF( %d, %d)",
                     ANNOTATE_FUNNUMBER (arg_node), ANNOTATE_FUNAPNUMBER (arg_node));
        } else {
            DBUG_ASSERT ((1 == 0), "wrong tag at N_annotate");
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
    bool print_nt = FALSE;

    DBUG_ENTER ("PrintArg");

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, " **%d:", ARG_VARNO (arg_node)););

    type_str = Type2String (ARG_TYPE (arg_node), 0, TRUE);
    fprintf (outfile, " %s", type_str);
    FREE (type_str);

    if (ARG_ATTRIB (arg_node) == ST_reference) {
        fprintf (outfile, " &");
    } else if (ARG_ATTRIB (arg_node) == ST_readonly_reference) {
        fprintf (outfile, " (&)");
    }

    if ((!INFO_PRINT_OMIT_FORMAL_PARAMS (arg_info)) && (ARG_NAME (arg_node) != NULL)) {

        DBUG_EXECUTE ("PRINT_NT",
                      PrintNT (outfile, ARG_NAME (arg_node), ARG_TYPE (arg_node));
                      print_nt = TRUE;);

        if (!print_nt) {
            fprintf (outfile, " %s", ARG_NAME (arg_node));
        }
    }

    PrintStatus (ARG_ATTRIB (arg_node), FALSE);
    PrintStatus (ARG_STATUS (arg_node), FALSE);

    PrintRC (ARG_REFCNT (arg_node), ARG_NAIVE_REFCNT (arg_node), show_refcnt);

    if (ARG_COLCHN (arg_node) && show_idx) {
        Trav (ARG_COLCHN (arg_node), arg_info);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",");
        PRINT_CONT (Trav (ARG_NEXT (arg_node), arg_info), );
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
    bool print_nt = FALSE;

    DBUG_ENTER ("PrintVardec");

    INDENT;

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**%d: ", VARDEC_VARNO (arg_node)););

    if ((VARDEC_ICM (arg_node) == NULL) || (NODE_TYPE (VARDEC_ICM (arg_node)) != N_icm)) {
        type_str = Type2String (VARDEC_TYPE (arg_node), 0, TRUE);
        fprintf (outfile, "%s ", type_str);
        FREE (type_str);

        DBUG_EXECUTE ("PRINT_NT",
                      PrintNT (outfile, VARDEC_NAME (arg_node), VARDEC_TYPE (arg_node));
                      print_nt = TRUE;);

        if (!print_nt) {
            fprintf (outfile, "%s", VARDEC_NAME (arg_node));
        }

        PrintStatus (VARDEC_ATTRIB (arg_node), FALSE);
        PrintStatus (VARDEC_STATUS (arg_node), FALSE);

        if (VARDEC_COLCHN (arg_node) && show_idx) {
            Trav (VARDEC_COLCHN (arg_node), arg_info);
        }

        fprintf (outfile, ";\n");
    } else {
        Trav (VARDEC_ICM (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    if (VARDEC_NEXT (arg_node)) {
        PRINT_CONT (Trav (VARDEC_NEXT (arg_node), arg_info), );
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintBlock(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintBlock (node *arg_node, node *arg_info)
{
    static int not_yet_done_print_main_begin = 1;
    /*
     * This static variable assures that only once for the outer block of
     * the main() function initialization code is generated, but not for
     * subsequent blocks of perhaps loops or conditionals.
     */

    int old_indent = indent;

    DBUG_ENTER ("PrintBlock");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

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

    if (not_yet_done_print_main_begin && (NODE_TYPE (arg_info) == N_info)
        && (INFO_PRINT_FUNDEF (arg_info) != NULL)
        && (strcmp (FUNDEF_NAME (INFO_PRINT_FUNDEF (arg_info)), "main") == 0)
        && (compiler_phase == PH_genccode)) {
        GSCPrintMainBegin ();
        not_yet_done_print_main_begin = 0;
    }

    if (BLOCK_INSTR (arg_node)) {
        Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    indent--;
    INDENT;
    fprintf (outfile, "}");

    if (indent != old_indent) {
#ifdef WARN_INDENT
        if (gen_mt_code != GEN_MT_OLD) {
            /*
             * for the time being (old) code for MT is always unbalanced :-(
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
 *   node *PrintReturn(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintReturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        if ((NODE_TYPE (arg_info) == N_info) && (compiler_phase == PH_genccode)
            && (INFO_PRINT_FUNDEF (arg_info) != NULL)
            && (strcmp (FUNDEF_NAME (INFO_PRINT_FUNDEF (arg_info)), "main") == 0)) {
            GSCPrintMainEnd ();
            INDENT;
        }

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

        fprintf (outfile, "return (");
        Trav (RETURN_EXPRS (arg_node), arg_info);
        fprintf (outfile, ");");
    }

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

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "\n"); INDENT;
                  fprintf (outfile, "**MASKS - assign: \n"); INDENT;
                  PrintDefMask (outfile, ASSIGN_DEFMASK (arg_node),
                                INFO_PRINT_VARNO (arg_info));
                  INDENT; PrintUseMask (outfile, ASSIGN_USEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info));
                  INDENT; PrintMrdMask (outfile, ASSIGN_MRDMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info)););

    DBUG_EXECUTE ("WLI", if ((N_let == NODE_TYPE (ASSIGN_INSTR (arg_node)))
                             && (F_psi == PRF_PRF (LET_EXPR (ASSIGN_INSTR (arg_node))))) {
        DbugIndexInfo (ASSIGN_INDEX (arg_node));
    });

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "instruction of N_assign is NULL");

    if (N_icm == NODE_TYPE (ASSIGN_INSTR (arg_node))) {
        indent += ICM_INDENT_BEFORE (ASSIGN_INSTR (arg_node));
        INDENT;
        Trav (ASSIGN_INSTR (arg_node), arg_info);
        fprintf (outfile, "\n");
        indent += ICM_INDENT_AFTER (ASSIGN_INSTR (arg_node));
        if (ASSIGN_NEXT (arg_node) != NULL) {
            PRINT_CONT (Trav (ASSIGN_NEXT (arg_node), arg_info), );
        }
    } else {
        PRINT_LINE_PRAGMA_IN_SIB (outfile, arg_node);

        DBUG_EXECUTE ("LINE", fprintf (outfile, "/*%03d*/", arg_node->lineno););

        instr = ASSIGN_INSTR (arg_node);
        if (((NODE_TYPE (instr) != N_return) && (NODE_TYPE (instr) != N_annotate))
            || ((NODE_TYPE (instr) == N_return) && (RETURN_EXPRS (instr) != NULL))
            || ((NODE_TYPE (instr) == N_annotate) && (compiler_phase >= PH_compile))) {
            trav_instr = TRUE;
        } else {
            trav_instr = FALSE;
        }

        if (NODE_TYPE (instr) == N_annotate) {
            DBUG_EXECUTE ("PRINT_PROFILE", trav_instr = TRUE;);
        }

        if (trav_instr) {
            INDENT;
            Trav (instr, arg_info);
            fprintf (outfile, "\n");
        }

        if (ASSIGN_NEXT (arg_node) != NULL) {
            PRINT_CONT (Trav (ASSIGN_NEXT (arg_node), arg_info), );
        }
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

    fprintf (outfile, "\n");
    INDENT;

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**used vars - do loop: ");
                  PrintDefUseMask (outfile, DO_TERMMASK (arg_node),
                                   INFO_PRINT_VARNO (arg_info));
                  INDENT;);

    fprintf (outfile, "do \n");

    DBUG_EXECUTE ("PRINT_VARS", INDENT; fprintf (outfile, "**(NAIVE)DEFVARS: ");
                  PrintIds (DO_DEFVARS (arg_node), arg_info); fprintf (outfile, " | ");
                  PrintIds (DO_NAIVE_DEFVARS (arg_node), arg_info);
                  fprintf (outfile, "\n"); INDENT;
                  fprintf (outfile, "**(NAIVE)USEVARS: ");
                  PrintIds (DO_USEVARS (arg_node), arg_info); fprintf (outfile, " | ");
                  PrintIds (DO_NAIVE_USEVARS (arg_node), arg_info);
                  fprintf (outfile, "\n"););

    if (DO_BODY (arg_node) != NULL) {
        DBUG_EXECUTE ("PRINT_MASKS", INDENT; fprintf (outfile, "**MASKS - do body: \n");
                      INDENT; PrintDefMask (outfile, DO_DEFMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info));
                      INDENT; PrintUseMask (outfile, DO_USEMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info));
                      INDENT; PrintMrdMask (outfile, DO_MRDMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info)););

        Trav (DO_BODY (arg_node), arg_info); /* traverse body of loop */
        fprintf (outfile, "\n");
    }

    INDENT;
    fprintf (outfile, "while (");
    Trav (DO_COND (arg_node), arg_info);
    fprintf (outfile, ");\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintWhile(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintWhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWhile");

    fprintf (outfile, "\n");
    INDENT;

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**used vars - while loop: ");
                  PrintDefUseMask (outfile, WHILE_TERMMASK (arg_node),
                                   INFO_PRINT_VARNO (arg_info));
                  INDENT;);

    fprintf (outfile, "while (");
    Trav (WHILE_COND (arg_node), arg_info);
    fprintf (outfile, ") \n");

    DBUG_EXECUTE ("PRINT_VARS", INDENT; fprintf (outfile, "**(NAIVE)DEFVARS: ");
                  PrintIds (WHILE_DEFVARS (arg_node), arg_info); fprintf (outfile, " | ");
                  PrintIds (WHILE_NAIVE_DEFVARS (arg_node), arg_info);
                  fprintf (outfile, "\n"); INDENT;
                  fprintf (outfile, "**(NAIVE)USEVARS: ");
                  PrintIds (WHILE_USEVARS (arg_node), arg_info); fprintf (outfile, " | ");
                  PrintIds (WHILE_NAIVE_USEVARS (arg_node), arg_info);
                  fprintf (outfile, "\n"););

    if (WHILE_BODY (arg_node) != NULL) {
        DBUG_EXECUTE ("PRINT_MASKS", INDENT;
                      fprintf (outfile, "**MASKS - while body: \n"); INDENT;
                      PrintDefMask (outfile, WHILE_DEFMASK (arg_node),
                                    INFO_PRINT_VARNO (arg_info));
                      INDENT; PrintUseMask (outfile, WHILE_USEMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info));
                      INDENT; PrintMrdMask (outfile, WHILE_MRDMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info)););

        Trav (WHILE_BODY (arg_node), arg_info); /* traverse body of loop */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintCond(node *arg_node, node *arg_info)
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

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**used vars - cond: ");
                  PrintDefUseMask (outfile, COND_CONDUSEMASK (arg_node),
                                   INFO_PRINT_VARNO (arg_info)););

    fprintf (outfile, "(");
    Trav (COND_COND (arg_node), arg_info);
    fprintf (outfile, ") \n");

    if (COND_THEN (arg_node) != NULL) {

        DBUG_EXECUTE ("PRINT_MASKS", INDENT; fprintf (outfile, "**MASKS - then: \n");
                      INDENT; PrintDefMask (outfile, COND_THENDEFMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info));
                      INDENT; PrintUseMask (outfile, COND_THENUSEMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info)););

        DBUG_EXECUTE ("PRINT_VARS", INDENT; fprintf (outfile, "**(NAIVE)VARS - then: ");
                      PrintIds (COND_THENVARS (arg_node), arg_info);
                      fprintf (outfile, " | ");
                      PrintIds (COND_NAIVE_THENVARS (arg_node), arg_info);
                      fprintf (outfile, "\n"););

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

        DBUG_EXECUTE ("PRINT_VARS", INDENT; fprintf (outfile, "**(NAIVE)VARS - else: ");
                      PrintIds (COND_ELSEVARS (arg_node), arg_info);
                      fprintf (outfile, " | ");
                      PrintIds (COND_NAIVE_ELSEVARS (arg_node), arg_info);
                      fprintf (outfile, "\n"););

        Trav (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintCast(node  *arg_node, node *arg_info)
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
    FREE (type_str);

    Trav (CAST_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintLet(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintLet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintLet");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

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

    if (LET_IDS (arg_node)) {
        PrintIds (LET_IDS (arg_node), arg_info);
        fprintf (outfile, " = ");
    }
    Trav (LET_EXPR (arg_node), arg_info);
    fprintf (outfile, "; ");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintPrf(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintPrf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPrf");

    DBUG_PRINT ("PRINT", ("%s (%s)" P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)],
                          mdb_prf[PRF_PRF (arg_node)], arg_node));

    switch (PRF_PRF (arg_node)) {
    case F_psi:
    case F_take:
    case F_drop:
    case F_shape:
    case F_reshape:
    case F_cat:
    case F_dim:
    case F_rotate:
    case F_not:
    case F_toi:
    case F_toi_A:
    case F_tof:
    case F_tof_A:
    case F_tod:
    case F_tod_A:
    case F_idx_psi:
    case F_modarray:
    case F_genarray:
    case F_idx_modarray:
    case F_min:
    case F_max:
    case F_abs:
        /* primitive functions that are printed as function application */
        DBUG_EXECUTE ("PRINT_PRF", fprintf (outfile, "PRF:"););

        fprintf (outfile, "%s( ", prf_string[PRF_PRF (arg_node)]);
        Trav (PRF_ARGS (arg_node), arg_info);
        fprintf (outfile, ")");
        break;

    default:
        /* primitive functions in infix notation */
        fprintf (outfile, "(");
        Trav (EXPRS_EXPR (PRF_ARGS (arg_node)), arg_info);
        fprintf (outfile, " %s ", prf_string[PRF_PRF (arg_node)]);
        if (NULL != EXPRS_NEXT (PRF_ARGS (arg_node))) {
            DBUG_ASSERT ((EXPRS_NEXT (EXPRS_NEXT (PRF_ARGS (arg_node))) == NULL),
                         "more than two args found");
            Trav (EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node))), arg_info);
        }
        fprintf (outfile, ")");
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintAp(node  *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintAp (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAp");

    if (AP_MOD (arg_node) != NULL) {
        fprintf (outfile, "%s:", AP_MOD (arg_node));
    }
    fprintf (outfile, "%s(", AP_NAME (arg_node));

    if (AP_ARGS (arg_node) != NULL) {
        fprintf (outfile, " ");
        Trav (AP_ARGS (arg_node), arg_info);
    }

    fprintf (outfile, ")");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintEmpty(node *arg_node, node *arg_info)
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
 *   node *PrintArray(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PrintArray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintArray");

    if (ARRAY_AELEMS (arg_node) != NULL) {
        fprintf (outfile, "[ ");
        Trav (ARRAY_AELEMS (arg_node), arg_info);
        fprintf (outfile, " ]");
    } else {
        fprintf (outfile, "[]");
    }

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
    DBUG_ENTER ("PrintExprs");

    Trav (EXPRS_EXPR (arg_node), arg_info);

    if (EXPRS_NEXT (arg_node) != NULL) {
        fprintf (outfile, ", ");
        PRINT_CONT (Trav (EXPRS_NEXT (arg_node), arg_info), );
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

    if ((ID_ATTRIB (arg_node) == ST_global) && (ID_MOD (arg_node) != NULL)) {
        fprintf (outfile, "%s:", ID_MOD (arg_node));
    }

    DBUG_EXECUTE ("PRINT_NT", if (ID_VARDEC (arg_node) != NULL) {
        PrintNT (outfile, ID_NAME (arg_node), ID_TYPE (arg_node));
        print_nt = TRUE;
    });

    if (!print_nt) {
        fprintf (outfile, "%s", ID_NAME (arg_node));
    }

    PrintStatus (ID_ATTRIB (arg_node), FALSE);
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
 *   node *PrintNum(node *arg_node, node *arg_info)
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
 *   node *PrintFloat(node *arg_node, node *arg_info)
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
    FREE (tmp_string);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintDouble(node *arg_node, node *arg_info)
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
    FREE (tmp_string);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintBool(node *arg_node, node *arg_info)
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
 *   node *PrintStr(node *arg_node, node *arg_info)
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
 *   node *PrintChar(node *arg_node, node *arg_info)
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
        fprintf (outfile, "'\\%o'", CHAR_VAL (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintVectInfo(node *arg_node, node *arg_info)
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
            FREE (type_str);
            break;
        default:
            DBUG_ASSERT (0, "illegal N_vinfo-flag!");
            break;
        }

        if (VINFO_NEXT (arg_node) != NULL) {
            PRINT_CONT (Trav (VINFO_NEXT (arg_node), arg_info), );
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
    int compiled_icm = 0;

    DBUG_ENTER ("PrintIcm");

    DBUG_PRINT ("PRINT", ("icm-node %s\n", ICM_NAME (arg_node)));

    if (compiler_phase == PH_genccode) {
        /*
         * For expanded C-ICMs we have to undo the indentation via ICM_INDENT_...
         * in order to prevent superfluous indentation!
         */
#define ICM_ALL
#define ICM_DEF(prf, trf)                                                                \
    if (strcmp (ICM_NAME (arg_node), #prf) == 0) {                                       \
        indent -= ICM_INDENT_BEFORE (arg_node);                                          \
        Print##prf (ICM_ARGS (arg_node), arg_info);                                      \
        indent -= ICM_INDENT_AFTER (arg_node);                                           \
        compiled_icm = 1;                                                                \
    } else
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_VAR(dim, name)
#define ICM_VARINT(dim, name)
#define ICM_END(prf, args)
#include "icm.data"
#undef ICM_ALL
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_VARINT
#undef ICM_END
        if (strcmp (ICM_NAME (arg_node), "NOOP") == 0) {
            fprintf (outfile, "/* noop */");
            compiled_icm = 1;
        }
    }

    if ((compiler_phase != PH_genccode) || (compiled_icm == 0)) {
        if ((strcmp (ICM_NAME (arg_node), "ND_FUN_RET") == 0)
            && (INFO_PRINT_FUNDEF (arg_info) != NULL)
            && (strcmp (FUNDEF_NAME (INFO_PRINT_FUNDEF (arg_info)), "main") == 0)
            && (compiler_phase == PH_genccode)) {
            GSCPrintMainEnd ();
        }

        fprintf (outfile, "SAC_%s( ", ICM_NAME (arg_node));
        if (ICM_ARGS (arg_node) != NULL) {
            Trav (ICM_ARGS (arg_node), arg_info);
        }
        fprintf (outfile, ")");

        if (ICM_END_OF_STATEMENT (arg_node)) {
            fprintf (outfile, ";");
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PrintPragma(node *arg_node, node *arg_info)
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
        fprintf (outfile, "#pragma linksign [%d", PRAGMA_LS (arg_node, 0));

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
 * remarks: there are syntactic alternatives to print the new WLs.
 * If only one Npart node exists the WL is printed in the way the
 * scanner can handle it. This is essential because the SIBs (which are
 * written with this code) have to be scanned again.
 * If a complete partition exists (more than one Npart) an internal
 * syntax is used.
 *
 * INFO_PRINT_INT_SYN(arg_info) is NULL for the internal syntax or != NULL
 * if 'PrintNpart' shall return the last expr.
 *
 ******************************************************************************/

node *
PrintNwith (node *arg_node, node *arg_info)
{
    node *buffer, *tmp_nwith;

    DBUG_ENTER ("PrintNwith");

    buffer = INFO_PRINT_INT_SYN (arg_info);

    tmp_nwith = INFO_PRINT_NWITH (arg_info);
    INFO_PRINT_NWITH (arg_info) = arg_node;

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

    DBUG_EXECUTE ("PRINT_RC", fprintf (outfile, "\n"); INDENT;
                  fprintf (outfile, "/* DEC_RC: "); if (
                    NWITH_DEC_RC_IDS (arg_node)
                    != NULL) { PrintIds (NWITH_DEC_RC_IDS (arg_node), arg_info); } else {
                      fprintf (outfile, "-");
                  } fprintf (outfile, " */\n");
                  INDENT;);

    indent++;

    /*
     * check wether to use output format 1 (multiple NParts)
     * or 2 (only one NPart) and use INFO_PRINT_INT_SYN(arg_info)
     * as flag for traversal.
     */
    if (NPART_NEXT (NWITH_PART (arg_node)) != NULL) {
        /* output format 1 */
        INFO_PRINT_INT_SYN (arg_info) = NULL;
        fprintf (outfile, "with\n");
        indent++;
        Trav (NWITH_PART (arg_node), arg_info);
        indent--;
    } else {
        /* output format 2 */
        INFO_PRINT_INT_SYN (arg_info) = arg_node; /* set != NULL */
        fprintf (outfile, "with ");
        Trav (NWITH_PART (arg_node), arg_info);
    }

    Trav (NWITH_WITHOP (arg_node), arg_info);

    if (NPART_NEXT (NWITH_PART (arg_node)) == NULL) {
        /*
         * output format 2:
         * now, INFO_PRINT_INT_SYN(arg_info) contains the last expr.
         */
        if (WO_modarray == NWITH_TYPE (arg_node)) {
            fprintf (outfile, ", dummy, ");
        } else {
            fprintf (outfile, ", ");
        }
        Trav (INFO_PRINT_INT_SYN (arg_info), arg_info);
    }
    fprintf (outfile, ")");

    indent -= 2;

    INFO_PRINT_NWITH (arg_info) = tmp_nwith;
    INFO_PRINT_INT_SYN (arg_info) = buffer;

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
 *   The index variable is found in NWITH_WITHID( INFO_PRINT_NWITH( arg_info)).
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
        fprintf (outfile, ".");
    }
    /* print first operator */
    fprintf (outfile, " %s ", prf_string[NGEN_OP1 (arg_node)]);

    /* print indices */
    if (INFO_PRINT_NWITH (arg_info) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (INFO_PRINT_NWITH (arg_info)) == N_Nwith),
                     "INFO_PRINT_NWITH is no N_Nwith node");

        Trav (NWITH_WITHID (INFO_PRINT_NWITH (arg_info)), arg_info);
    } else {
        fprintf (outfile, "?");
    }

    /* print second operator */
    fprintf (outfile, " %s ", prf_string[NGEN_OP2 (arg_node)]);
    /* print lower bound */
    if (NGEN_BOUND2 (arg_node)) {
        Trav (NGEN_BOUND2 (arg_node), arg_info);
    } else {
        fprintf (outfile, ".");
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

    DBUG_EXECUTE ("PRINT_RC", fprintf (outfile, "\n"); INDENT;
                  fprintf (outfile, "/* INC_RC: "); if (
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

    /*
     * print the expression if internal syntax should be used.
     * else return expr in INFO_PRINT_INT_SYN( arg_info)
     */
    if (NCODE_CEXPR (arg_node) != NULL) {
        if (INFO_PRINT_INT_SYN (arg_info) != NULL) {
            INFO_PRINT_INT_SYN (arg_info) = NCODE_CEXPR (arg_node);
        } else {
            fprintf (outfile, " : ");
            Trav (NCODE_CEXPR (arg_node), arg_info);
        }
    }
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
    DBUG_ENTER ("PrintNpart");

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "\n"); INDENT;
                  fprintf (outfile, "**MASKS - Npart: \n"); INDENT;
                  PrintDefMask (outfile, NPART_DEFMASK (arg_node),
                                INFO_PRINT_VARNO (arg_info));
                  INDENT; PrintUseMask (outfile, NPART_USEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info));
                  INDENT;);

    /* print generator */
    if (INFO_PRINT_INT_SYN (arg_info) == NULL) {
        INDENT; /* each gen in a new line. */
    }
    Trav (NPART_GEN (arg_node), arg_info);

    DBUG_ASSERT ((NPART_CODE (arg_node) != NULL),
                 "part within WL without pointer to N_Ncode");

    Trav (NPART_CODE (arg_node), arg_info);

    if (NPART_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",\n");
        /*
         * continue with other parts
         */
        PRINT_CONT (Trav (NPART_NEXT (arg_node), arg_info), );
    } else {
        fprintf (outfile, "\n");
    }

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

    tmp_nwith2 = INFO_PRINT_NWITH (arg_info);
    INFO_PRINT_NWITH (arg_info) = arg_node;

    indent++;

    if (NWITH2_PRAGMA (arg_node) != NULL) {
        Trav (NWITH2_PRAGMA (arg_node), arg_info);
        INDENT;
    }

    DBUG_EXECUTE ("PRINT_RC", fprintf (outfile, "\n"); INDENT;
                  fprintf (outfile, "/* DEC_RC: "); if (
                    NWITH2_DEC_RC_IDS (arg_node)
                    != NULL) { PrintIds (NWITH2_DEC_RC_IDS (arg_node), arg_info); } else {
                      fprintf (outfile, "-");
                  } fprintf (outfile, " */\n");
                  INDENT;);

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

    indent -= 2;

    INFO_PRINT_NWITH (arg_info) = tmp_nwith2;

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
    int i = 0;

    DBUG_ENTER ("PrintWLsegx");

    seg = arg_node;
    while (seg != NULL) {
        INDENT;
        fprintf (outfile, "/**********%s segment %d: **********",
                 (NODE_TYPE (arg_node) == N_WLseg) ? "" : " (var.)", i++);

        fprintf (outfile, "\n");
        INDENT;
        fprintf (outfile, " * index domain: ");
        WLSEGX_IDX_PRINT (outfile, arg_node, IDX_MIN);
        fprintf (outfile, " -> ");
        WLSEGX_IDX_PRINT (outfile, arg_node, IDX_MAX);
        fprintf (outfile, "\n");
        INDENT;

        if (WLSEGX_SCHEDULING (seg) != NULL) {
            fprintf (outfile, " * scheduling: ");
            SCHPrintScheduling (outfile, WLSEGX_SCHEDULING (seg));
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
 *   void PrintWLboundInt( int val)
 *
 * description:
 *   prints a parameter of a N_WLstride- or N_WLgrid-node.
 *
 ******************************************************************************/

static void
PrintWLboundInt (int val)
{
    DBUG_ENTER ("PrintWLboundInt");

    if (val == IDX_OTHER) {
        fprintf (outfile, "?");
    } else if (val == IDX_SHAPE) {
        fprintf (outfile, ".");
    } else {
        fprintf (outfile, "%d", val);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintWLboundVar( node *wlnode, int dim)
 *
 * description:
 *   prints a parameter of a N_WLstrideVar- or N_WLgridVar-node.
 *
 ******************************************************************************/

static void
PrintWLboundVar (node *wlnode, int dim)
{
    DBUG_ENTER ("PrintWLboundVar");

    if (NODE_TYPE (wlnode) == N_num) {
        PrintWLboundInt (NUM_VAL (wlnode));
    } else {
        DBUG_ASSERT ((NODE_TYPE (wlnode) == N_id), "illegal node type found!");

        fprintf (outfile, "%s[%d]", ID_NAME (wlnode), dim);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintWLbound( nodetype nt, void *node_or_int, int dim)
 *
 * description:
 *   prints a parameter of a N_WLstride(Var)- or N_WLgrid(Var)-node.
 *
 ******************************************************************************/

static void
PrintWLbound (nodetype nt, void *node_or_int, int dim)
{
    char *name;
    int val;

    DBUG_ENTER ("PrintWLbound");

    NodeOrInt_GetNameOrVal (&name, &val, nt, node_or_int);

    if (val == IDX_OTHER) {
        if (name == NULL) {
            fprintf (outfile, "?");
        } else {
            PrintWLboundVar ((*(node **)node_or_int), dim);
        }
    } else {
        PrintWLboundInt (val);
    }

    DBUG_VOID_RETURN;
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
    PrintWLboundInt (WLXBLOCK_BOUND1 (arg_node));
    fprintf (outfile, " -> ");
    PrintWLboundInt (WLXBLOCK_BOUND2 (arg_node));
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
        PRINT_CONT (Trav (WLXBLOCK_NEXT (arg_node), arg_info), );
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
    PrintWLbound (NODE_TYPE (arg_node), WLSTRIDEX_GET_ADDR (arg_node, BOUND1),
                  WLSTRIDEX_DIM (arg_node));
    fprintf (outfile, " %s> ", (NODE_TYPE (arg_node) == N_WLstride) ? "-" : "=");
    PrintWLbound (NODE_TYPE (arg_node), WLSTRIDEX_GET_ADDR (arg_node, BOUND2),
                  WLSTRIDEX_DIM (arg_node));
    fprintf (outfile, "), step%d[%d] ", WLSTRIDEX_LEVEL (arg_node),
             WLSTRIDEX_DIM (arg_node));
    PrintWLbound (NODE_TYPE (arg_node), WLSTRIDEX_GET_ADDR (arg_node, STEP),
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
        PRINT_CONT (Trav (WLSTRIDEX_NEXT (arg_node), arg_info), );
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
        if (INFO_PRINT_NWITH (arg_info) != NULL) {
            DBUG_ASSERT ((NODE_TYPE (INFO_PRINT_NWITH (arg_info)) == N_Nwith2),
                         "INFO_PRINT_NWITH( arg_info) contains no N_Nwith2 node");

            switch (NWITH2_TYPE (INFO_PRINT_NWITH (arg_info))) {
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
    PrintWLbound (NODE_TYPE (arg_node), WLGRIDX_GET_ADDR (arg_node, BOUND1),
                  WLGRIDX_DIM (arg_node));
    fprintf (outfile, " %s%s> ", str, WLGRIDX_FITTED (arg_node) ? str : ">");
    PrintWLbound (NODE_TYPE (arg_node), WLGRIDX_GET_ADDR (arg_node, BOUND2),
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
        PRINT_CONT (Trav (WLGRIDX_NEXT (arg_node), arg_info), );
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
                FREE (type_str);

                fprintf (outfile, ")\n");
                funlist = NODELIST_NEXT (funlist);
            }

            fprintf (outfile, "\n\n");

            if (CWRAPPER_NEXT (arg_node) != NULL) {
                PRINT_CONT (Trav (CWRAPPER_NEXT (arg_node), arg_info), );
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

    fprintf (outfile, "baseid: %s, counter: %d\n", SSACNT_BASEID (arg_node),
             SSACNT_COUNT (arg_node));

    if (SSACNT_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (SSACNT_NEXT (arg_node), arg_info), );
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
        PRINT_CONT (Trav (CSEINFO_NEXT (arg_node), arg_info), );
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
    DBUG_ENTER ("PrintAvis");

    /* to be implemented */

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
             * global objects are all printed to separate files allowing for separate
             * compilation and the building of an archive. An additional header file
             * is generated for global variable and type declarations as well as
             * function prototypes.
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

    DBUG_PRINT ("PRINT_LINE", ("line (%s) %i\n", mdb_nodetype[NODE_TYPE (arg_node)],
                               NODE_LINE (arg_node)));

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

    syntax_tree = PrintTrav (syntax_tree, arg_info);

    FREE (arg_info);

    /* if generating c library, invoke the headerfile generator */
    if ((generatelibrary & GENERATELIBRARY_C) && (compiler_phase == PH_genccode)) {
        PrintInterface (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNode(node *syntax_tree)
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

    syntax_tree = PrintTrav (syntax_tree, arg_info);

    FREE (arg_info);

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
 * function:
 *   void DoPrintShapeAST( int dim, shpseg *shape)
 *
 * description:
 *   This function is called from 'DoPrintAST' only.
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
 *   This function is called from 'DoPrintAllTypeAST' only.
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
 *   void DoPrintAllTypeAST( types *type, bool print_status)
 *
 * Description:
 *   Prints a single types-structure.
 *   This function is called from 'DoPrintTypesAST' only.
 *
 ******************************************************************************/

static void
DoPrintAllTypeAST (types *type, bool print_status)
{
    node *tdef;

    DBUG_ENTER ("DoPrintAllTypeAST");

    if (TYPES_BASETYPE (type) == T_user) {
        if (TYPES_NAME (type) != NULL) {
            fprintf (outfile, "%s", TYPES_NAME (type));
        } else {
            fprintf (outfile, "?");
        }
        DoPrintShapeAST (TYPES_DIM (type), TYPES_SHPSEG (type));
        if (TYPES_NAME (type) != NULL) {
            fprintf (outfile, "<" F_PTR ">", TYPES_NAME (type));
        }

        fprintf (outfile, "/");

        tdef = TYPES_TDEF (type);
        if (tdef != NULL) {
            DoPrintBasicTypeAST (type);
            fprintf (outfile, "<" F_PTR ">", tdef);
        } else {
            fprintf (outfile, "NULL");
        }
    } else {
        DoPrintBasicTypeAST (type);
    }

    if (print_status) {
        PrintStatus (TYPES_STATUS (type), TRUE);
    }

    if (TYPES_NEXT (type) != NULL) {
        fprintf (outfile, " ");
        DoPrintAllTypeAST (TYPES_NEXT (type), print_status);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void DoPrintTypesAST( types *type, bool print_status)
 *
 * Description:
 *   Prints all the data of a types-structure.
 *
 ******************************************************************************/

static void
DoPrintTypesAST (types *type, bool print_status)
{
    DBUG_ENTER ("DoPrintTypesAST");

    if (type != NULL) {
        if (TYPES_NEXT (type) != NULL) {
            fprintf (outfile, "{ ");
        }
        DoPrintAllTypeAST (type, print_status);
        if (TYPES_NEXT (type) != NULL) {
            fprintf (outfile, " }");
        }
    } else {
        fprintf (outfile, "NULL");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void DoPrintIdsAST( ids *vars, bool print_status)
 *
 * description:
 *   This function is called from 'DoPrintAST' only.
 *   Prints a 'ids'-chain.
 *
 ******************************************************************************/

static void
DoPrintIdsAST (ids *vars, bool print_status)
{
    DBUG_ENTER ("DoPrintIdsAST");

    fprintf (outfile, "{ ");
    while (vars != NULL) {
        if (IDS_VARDEC (vars) != NULL) {
            DoPrintTypesAST (IDS_TYPE (vars), TRUE);
            fprintf (outfile, "<" F_PTR ">", IDS_VARDEC (vars));

            fprintf (outfile, " ");
        }

        fprintf (outfile, "%s<" F_PTR ">", IDS_NAME (vars), IDS_NAME (vars));

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
 *   void DoPrintAttrAST( int num, node *arg_node)
 *
 * description:
 *   This function is called from 'DoPrintAST' only.
 *   Prints an attribute containing a node-pointer.
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

    if (arg_node != NULL) {
        fprintf (outfile, F_PTR "\n", arg_node);
    } else {
        fprintf (outfile, "NULL\n");
    }

    DBUG_VOID_RETURN;
}

/* forward declaration */
static void DoPrintAST (node *arg_node, bool skip_next, bool print_attr);

/******************************************************************************
 *
 * function:
 *   void DoPrintSonAST( int num, node *arg_node,
 *                       bool skip_node, bool print_attr)
 *
 * description:
 *   This function is called from 'DoPrintAST' only.
 *   Prints a son containing a hole sub-tree.
 *
 ******************************************************************************/

static void
DoPrintSonAST (int num, node *arg_node, bool skip_node, bool print_attr)
{
    DBUG_ENTER ("DoPrintSonAST");

    DoIndentAST ();

    if (num >= 0) {
        fprintf (outfile, "%i=", num);
    } else {
        fprintf (outfile, "+=");
    }

    if (skip_node) {
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
 *   void DoPrintAST( node *arg_node, bool skip_next)
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
    node *skip;
    bool new_line;
    int i;

    DBUG_ENTER ("DoPrintAST");

    skip = NULL;

    outfile = stdout;

    if (arg_node) {
        new_line = TRUE;

        /* print node name */
        fprintf (outfile, "%s<" F_PTR ">  ", mdb_nodetype[NODE_TYPE (arg_node)],
                 arg_node);

        /* print additional information to nodes */
        switch (NODE_TYPE (arg_node)) {
        case N_typedef:
            fprintf (outfile, "(");

            DoPrintTypesAST (TYPEDEF_TYPE (arg_node), TRUE);
            fprintf (outfile, " ");

            fprintf (outfile, "%s<" F_PTR ">", STR_OR_NULL (TYPEDEF_NAME (arg_node), "?"),
                     TYPEDEF_NAME (arg_node));

            PrintStatus (TYPEDEF_ATTRIB (arg_node), TRUE);
            PrintStatus (TYPEDEF_STATUS (arg_node), TRUE);

            fprintf (outfile, ")");

            skip = TYPEDEF_NEXT (arg_node);
            break;

        case N_objdef:
            fprintf (outfile, "(");

            DoPrintTypesAST (OBJDEF_TYPE (arg_node), TRUE);
            fprintf (outfile, " ");

            fprintf (outfile, "%s<" F_PTR ">", STR_OR_NULL (OBJDEF_NAME (arg_node), "?"),
                     OBJDEF_NAME (arg_node));

            PrintStatus (OBJDEF_ATTRIB (arg_node), TRUE);
            PrintStatus (OBJDEF_STATUS (arg_node), TRUE);

            fprintf (outfile, ")");

            skip = OBJDEF_NEXT (arg_node);
            break;

        case N_fundef:
            fprintf (outfile, "(");

            DoPrintTypesAST (FUNDEF_TYPES (arg_node), TRUE);
            fprintf (outfile, " ");

            fprintf (outfile, "%s<" F_PTR ">", FUNDEF_NAME (arg_node),
                     FUNDEF_NAME (arg_node));

            PrintStatus (OBJDEF_ATTRIB (arg_node), TRUE);
            PrintStatus (OBJDEF_STATUS (arg_node), TRUE);

            fprintf (outfile, ")");

            skip = FUNDEF_NEXT (arg_node);
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

        case N_arg:
            fprintf (outfile, "(");

            DoPrintTypesAST (ARG_TYPE (arg_node), TRUE);
            fprintf (outfile, " ");

            fprintf (outfile, "%s<" F_PTR ">", STR_OR_NULL (ARG_NAME (arg_node), "?"),
                     ARG_NAME (arg_node));

            PrintStatus (ARG_ATTRIB (arg_node), TRUE);
            PrintStatus (ARG_STATUS (arg_node), TRUE);

            PrintRC (ARG_REFCNT (arg_node), ARG_NAIVE_REFCNT (arg_node), TRUE);

            fprintf (outfile, ", ");
            fprintf (outfile, "varno: %i", ARG_VARNO (arg_node));

            fprintf (outfile, ")");

            skip = ARG_NEXT (arg_node);
            break;

        case N_exprs:
            skip = EXPRS_NEXT (arg_node);
            break;

        case N_vardec:
            fprintf (outfile, "(");

            DoPrintTypesAST (VARDEC_TYPE (arg_node), TRUE);
            fprintf (outfile, " ");

            fprintf (outfile, "%s<" F_PTR ">", STR_OR_NULL (VARDEC_NAME (arg_node), "?"),
                     VARDEC_NAME (arg_node));

            PrintStatus (VARDEC_ATTRIB (arg_node), TRUE);
            PrintStatus (VARDEC_STATUS (arg_node), TRUE);

            PrintRC (VARDEC_REFCNT (arg_node), VARDEC_NAIVE_REFCNT (arg_node), TRUE);

            fprintf (outfile, ", ");
            fprintf (outfile, "varno: %i", VARDEC_VARNO (arg_node));

            fprintf (outfile, ")");

            skip = VARDEC_NEXT (arg_node);
            break;

        case N_vinfo:
            skip = VINFO_NEXT (arg_node);
            break;

        case N_assign:
            skip = ASSIGN_NEXT (arg_node);
            break;

        case N_let:
            DoPrintIdsAST (LET_IDS (arg_node), TRUE);
            break;

        case N_ap:
            fprintf (outfile, "(");

            fprintf (outfile, "%s<" F_PTR ">", AP_NAME (arg_node), AP_NAME (arg_node));

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
                DoPrintTypesAST (ID_TYPE (arg_node), TRUE);
                fprintf (outfile, "<" F_PTR ">", ID_VARDEC (arg_node));

                fprintf (outfile, " ");
            }

            fprintf (outfile, "%s<" F_PTR ">", ID_NAME (arg_node), ID_NAME (arg_node));

            PrintStatus (ID_ATTRIB (arg_node), TRUE);
            PrintStatus (ID_STATUS (arg_node), TRUE);

            PrintRC (ID_REFCNT (arg_node), ID_NAIVE_REFCNT (arg_node), TRUE);

            fprintf (outfile, ")");
            break;

        case N_num:
            fprintf (outfile, "(");

            fprintf (outfile, "%i", NUM_VAL (arg_node));

            fprintf (outfile, ")");
            break;

        case N_array:
            fprintf (outfile, "(");

            DoPrintTypesAST (ARRAY_TYPE (arg_node), TRUE);

            fprintf (outfile, ")");
            break;

        case N_icm:
            fprintf (outfile, "(");

            fprintf (outfile, "%s", ICM_NAME (arg_node));

            fprintf (outfile, ")");
            break;

        case N_Nwith:
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

                fprintf (outfile, "%s<" F_PTR ">", withid, withid);

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

            if (NWITHOP_FUNDEF (arg_node) != NULL) {
                fprintf (outfile, "%s", FUNDEF_NAME (NWITHOP_FUNDEF (arg_node)));
            }

            fprintf (outfile, ")");
            break;

        case N_Npart:
            fprintf (outfile, "(");

            if (NPART_CODE (arg_node) != NULL) {
                fprintf (outfile, "code used: " F_PTR, NPART_CODE (arg_node));
            } else {
                fprintf (outfile, "no code");
            }

            fprintf (outfile, ")");

            skip = NPART_NEXT (arg_node);
            break;

        case N_Ncode:
            fprintf (outfile, "(");

            fprintf (outfile, "<" F_PTR ">, used: %d", arg_node, NCODE_USED (arg_node));

            fprintf (outfile, ")");

            skip = NCODE_NEXT (arg_node);
            break;

        case N_Nwith2:
            fprintf (outfile, "(");

            fprintf (outfile, "offset_needed: %i", NWITH2_OFFSET_NEEDED (arg_node));

            fprintf (outfile, ")");
            break;

        case N_WLseg:
            fprintf (outfile, "(");

            fprintf (outfile, "idx_min: ");
            WLSEG_IDX_PRINT (outfile, arg_node, IDX_MIN);

            fprintf (outfile, ", ");
            fprintf (outfile, "idx_max: ");
            WLSEG_IDX_PRINT (outfile, arg_node, IDX_MAX);

            fprintf (outfile, ", ");
            fprintf (outfile, "sv: ");
            PRINT_VECT (outfile, WLSEG_SV (arg_node), WLSEG_DIMS (arg_node), "%i");

            for (i = 0; i < WLSEG_BLOCKS (arg_node); i++) {
                fprintf (outfile, ", ");
                fprintf (outfile, "bv%i: ", i);
                PRINT_VECT (outfile, WLSEG_BV (arg_node, i), WLSEG_DIMS (arg_node), "%i");
            }

            fprintf (outfile, ", ");
            fprintf (outfile, "ubv: ");
            PRINT_VECT (outfile, WLSEG_UBV (arg_node), WLSEG_DIMS (arg_node), "%i");

            fprintf (outfile, ", ");
            fprintf (outfile, "maxhomdim: %i", WLSEG_MAXHOMDIM (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "scheduler: ");
            SCHPrintScheduling (outfile, WLSEG_SCHEDULING (arg_node));

            fprintf (outfile, ")");

            skip = WLSEG_NEXT (arg_node);
            break;

        case N_WLsegVar:
            fprintf (outfile, "(");

            fprintf (outfile, "idx_min: ");
            WLSEGVAR_IDX_PRINT (outfile, arg_node, IDX_MIN);

            fprintf (outfile, ", ");
            fprintf (outfile, "idx_max: ");
            WLSEGVAR_IDX_PRINT (outfile, arg_node, IDX_MAX);

            fprintf (outfile, ", ");
            fprintf (outfile, "scheduler: ");
            SCHPrintScheduling (outfile, WLSEGVAR_SCHEDULING (arg_node));

            fprintf (outfile, ")");

            skip = WLSEGVAR_NEXT (arg_node);
            break;

        case N_WLblock:
            fprintf (outfile, "(");

            fprintf (outfile, "%d->%d block%d[%d] %d", WLBLOCK_BOUND1 (arg_node),
                     WLBLOCK_BOUND2 (arg_node), WLBLOCK_LEVEL (arg_node),
                     WLBLOCK_DIM (arg_node), WLBLOCK_STEP (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "noop: %i", WLBLOCK_NOOP (arg_node));

            fprintf (outfile, ")");

            skip = WLBLOCK_NEXT (arg_node);
            break;

        case N_WLublock:
            fprintf (outfile, "(");

            fprintf (outfile, "%d->%d ublock%d[%d] %d", WLUBLOCK_BOUND1 (arg_node),
                     WLUBLOCK_BOUND2 (arg_node), WLUBLOCK_LEVEL (arg_node),
                     WLUBLOCK_DIM (arg_node), WLUBLOCK_STEP (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "noop: %i", WLUBLOCK_NOOP (arg_node));

            fprintf (outfile, ")");

            skip = WLUBLOCK_NEXT (arg_node);
            break;

        case N_WLstride:
            fprintf (outfile, "(");

            fprintf (outfile, "%d->%d step%d[%d] %d", WLSTRIDE_BOUND1 (arg_node),
                     WLSTRIDE_BOUND2 (arg_node), WLSTRIDE_LEVEL (arg_node),
                     WLSTRIDE_DIM (arg_node), WLSTRIDE_STEP (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "noop: %i", WLSTRIDE_NOOP (arg_node));

            fprintf (outfile, ")");

            skip = WLSTRIDE_NEXT (arg_node);
            break;

        case N_WLstrideVar:
            fprintf (outfile, "(");

            PrintWLboundVar (WLSTRIDEVAR_BOUND1 (arg_node), WLSTRIDEVAR_DIM (arg_node));
            fprintf (outfile, "->");
            PrintWLboundVar (WLSTRIDEVAR_BOUND2 (arg_node), WLSTRIDEVAR_DIM (arg_node));
            fprintf (outfile, ", ");
            fprintf (outfile, "step%d[%d] ", WLSTRIDEVAR_LEVEL (arg_node),
                     WLSTRIDEVAR_DIM (arg_node));
            PrintWLboundVar (WLSTRIDEVAR_STEP (arg_node), WLSTRIDEVAR_DIM (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "noop: %i", WLSTRIDEVAR_NOOP (arg_node));

            fprintf (outfile, ")");

            skip = WLSTRIDEVAR_NEXT (arg_node);
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

            skip = WLGRID_NEXT (arg_node);
            break;

        case N_WLgridVar:
            fprintf (outfile, "(");

            PrintWLboundVar (WLGRIDVAR_BOUND1 (arg_node), WLGRIDVAR_DIM (arg_node));
            fprintf (outfile, "->");
            PrintWLboundVar (WLGRIDVAR_BOUND2 (arg_node), WLGRIDVAR_DIM (arg_node));
            fprintf (outfile, " [%d]", WLGRIDVAR_DIM (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "fitted: %i", WLGRIDVAR_FITTED (arg_node));

            fprintf (outfile, ", ");
            fprintf (outfile, "noop: %i", WLGRIDVAR_NOOP (arg_node));

            fprintf (outfile, ")");

            skip = WLGRIDVAR_NEXT (arg_node);
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
                            && (arg_node->node[i] == skip)),
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
    } else
        fprintf (outfile, "NULL\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintAST( node *arg_node)
 *   void PrintNodeAST( node *arg_node)
 *
 * description:
 *   These functions are for debug assistance.
 *   They print the syntax tree without any interpretation.
 *   Some attribues of interest are printed inside of parenthesizes behind
 *   the node name.
 *   PrintAST prints the whole chain, PrintNodeAST skips NEXT-sons.
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
