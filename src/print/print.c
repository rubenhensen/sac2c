/*
 *
 * $Log$
 * Revision 2.42  2000/02/02 16:10:10  bs
 * *** empty log message ***
 *
 * Revision 2.41  2000/01/31 19:15:29  bs
 * Function WLAAprintAccesse modified.
 * Function TSIprintInfo added.
 *
 * Revision 2.40  2000/01/26 17:29:30  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.38  1999/11/18 12:52:28  bs
 *  FEATURE_UNKNOWN added.
 *
 * Revision 2.37  1999/11/11 18:26:03  dkr
 * PrintNgenerator is now called by Trav only :))
 *
 * Revision 2.36  1999/10/28 19:58:45  sbs
 * ARRAY_FLAT changed to PRINT_CAR
 * PRINT_WLAA added.
 *
 * Revision 2.35  1999/10/28 17:09:12  dkr
 * output of use/def/mrd masks changed
 *
 * Revision 2.34  1999/10/28 09:43:42  sbs
 * printing of RC made optional via DBUG_FLAG "PRINT_RC" and
 * "PRINT_NRC".
 *
 * Revision 2.33  1999/10/19 08:09:11  dkr
 * comment corrected
 *
 * Revision 2.32  1999/09/20 11:37:12  jhs
 * Added USE/DEFVARS to do-loop.
 *
 * Revision 2.31  1999/09/10 14:26:03  jhs
 * Added printing of NAIVE_VARS of loops and conds.
 *
 * Revision 2.30  1999/09/01 17:10:03  jhs
 * SYNC_SCHEDULING
 *
 * Revision 2.29  1999/08/27 11:09:40  jhs
 * Delete the long message for different refcounters, instead only "**"
 * will be printed (keep in mind, naive-refcounters are done one
 * variables that will normally not be refcounted, e.g. scalars).
 *
 * Revision 2.28  1999/08/25 15:26:41  bs
 * Bug fixed in PrintNcode.
 *
 * Revision 2.27  1999/08/04 14:30:39  bs
 * WLAAprintAccesses modified, using new access macros now.
 *
 * Revision 2.26  1999/07/28 10:41:51  bs
 * Function WLAAprintAccesses modified.
 *
 * Revision 2.25  1999/07/08 16:00:52  bs
 * Bug fixed in WLAAprintAccesses.
 *
 * Revision 2.24  1999/07/07 05:57:44  sbs
 * adjust the function for printing Vinfo-Nodes for the new Dollar separated chains
 *
 * Revision 2.23  1999/06/15 12:32:14  jhs
 * Added code to print out the naive refcounters, when the normal refcounters
 * are printed. The naive one is introduced by "::" in front, an error message is
 * printed as comment, when refcounters are NOT optimized and normal and naive
 * refcounters differ.
 *
 * Revision 2.22  1999/06/08 08:32:42  cg
 * The print phase now always carries an N_info node with it in order
 * to distinguish between different layouts. The distinction between
 * arg_info ==NULL and arg_info !=NULL is no longer used.
 *
 * Revision 2.21  1999/06/03 08:45:00  cg
 * Added correct printing of wlcomp pragmas.
 *
 * Revision 2.20  1999/05/27 08:50:04  cg
 * global variable show_icm made obsolete and removed.
 *
 * Revision 2.19  1999/05/12 15:29:35  jhs
 * DbugPrintArray dbugged.
 *
 * Revision 2.18  1999/05/12 08:51:52  jhs
 * Changed attribute names to access constatnt vectors.
 *
 * Revision 2.17  1999/05/11 08:57:43  sbs
 * PRINT_LINE_PRAGMA_IN_SIB added. It allows for identifying errorneous
 * code from modules that is imported via the sib!.
 *
 * Revision 2.16  1999/05/10 15:52:30  bs
 * Bug fixed in WLAAprintAccesses
 *
 * Revision 2.15  1999/05/10 13:24:36  bs
 * WLAA infos will be printed if compilation breaks after phase 15 (sacopt).
 *
 * Revision 2.14  1999/05/10 12:02:28  bs
 * WLAA printfunction(s) modified.
 *
 * Revision 2.13  1999/05/05 12:20:31  jhs
 * PrintNGenerators enhanced, with ORIG_GENERATORS it is possible to
 * see the original operators of any generator. These values are not
 * modified by flatten or elsewhere.
 *
 * Revision 2.12  1999/04/19 17:08:56  jhs
 * DbugPrintArray now fit for empty arrays.
 *
 * Revision 2.11  1999/04/15 15:00:56  cg
 * ICMs are no longer printed with ';' behind. This was a bug.
 *
 * Revision 2.10  1999/04/14 09:24:31  cg
 * ICMs are now printed with a following ';'
 *
 * Revision 2.9  1999/04/13 14:02:27  cg
 * added printing of #pragma cachesim.
 *
 * Revision 2.8  1999/04/12 17:58:43  bs
 * PrintNpart and PrintNcode modified: now there is a possibility to print
 * TSI informations using the dbug flag 'TSI_INFO'
 *
 * Revision 2.7  1999/04/08 17:18:05  jhs
 * Handling for empty arrays added.
 *
 * Revision 2.6  1999/03/17 22:22:22  bs
 * DbugPrintArray modified. Printing of character- and boolean arrays possible.
 *
 * Revision 2.5  1999/03/17 15:37:09  bs
 * DbugPrintArray modified.
 *
 * Revision 2.4  1999/03/15 15:57:29  sbs
 * braces for if's inserted which might cause ambiguities.
 *
 * Revision 2.3  1999/03/15 14:19:19  bs
 * Access macros renamed (take a look at tree_basic.h).
 *
 * Revision 2.2  1999/03/09 11:21:23  bs
 * Debugging-information in PrintArray and PrintId added. Using the debug flag
 * "ARRAY_FLAT" the compact form of integer vectors will be printed.
 *
 * Revision 2.1  1999/02/23 12:40:24  sacbase
 * new release made
 *
 * Revision 1.261  1999/02/15 15:11:50  cg
 * Defines with function name before each function definition
 * moved into ND_FUNDEC ICM.
 *
 * Revision 1.260  1999/02/10 10:00:20  srs
 * bugfix in PrintAssign()
 *
 * Revision 1.259  1999/02/10 09:19:41  srs
 * added DBUG output for WLI
 *
 * Revision 1.258  1999/02/10 08:44:22  cg
 * bug fixed in PrintModul(), added some useful comments.
 *
 * Revision 1.257  1999/02/09 17:27:57  dkr
 * fixed a bug in PrintModul:
 *   no printf-comands on closed files ... :^)
 *
 * Revision 1.255  1999/02/04 17:23:37  srs
 * added check if arg_info is not NULL.
 *
 * Revision 1.254  1999/01/07 14:00:18  sbs
 * MRD list printing inserted.
 *
 * Revision 1.253  1998/12/21 10:52:25  sbs
 * MRD lists now can be printed by using #d,MRD,MASK !
 *
 * Revision 1.252  1998/12/03 09:21:31  sbs
 * ftoi, ftod, and friends eliminated
 *
 * Revision 1.251  1998/10/30 09:51:32  cg
 * primtive functions min and max or no longer printed
 * in infix notation.
 *
 * ... [eliminated] ...
 *
 * Revision 1.6  1994/11/10  15:34:26  sbs
 * RCS-header inserted
 *
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

#include "tree.h"

#include "print.h"
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

#define ACLT(arg)                                                                        \
    (arg == ACL_unknown)                                                                 \
      ? ("ACL_unknown")                                                                  \
      : ((arg == ACL_irregular)                                                          \
           ? ("ACL_irregular")                                                           \
           : ((arg == ACL_offset) ? ("ACL_offset:")                                      \
                                  : ((arg == ACL_const) ? ("ACL_const :") : (""))))

#define NIFmdb_nodetype(mdb_nodetype) mdb_nodetype
char *nametab[] = {
#include "node_info.mac"
};

#define IV(a) ((a) == 0) ? ("") : ("%s + ", VARDEC_NAME ())

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

/******************************************************************************
 *
 * function:
 *  void DbugPrintArray(node *arg_node)
 *
 * description:
 *   - this function is only used for printing of debugging informations in
 *     PrintArray and PrintId.
 *
 ******************************************************************************/

static void
DbugPrintArray (node *arg_node)
{
    int *intptr, i;
    void *constvec;
    simpletype vectype;
    int veclen;
    char *chrptr;
    float *fltptr;
    double *dblptr;

    if (NODE_TYPE (arg_node) == N_array) {
        veclen = ARRAY_VECLEN (arg_node);
        vectype = ARRAY_VECTYPE (arg_node);
        constvec = ARRAY_CONSTVEC (arg_node);
    } else /* (NODE_TYPE(arg_node) == N_id) */ {
        veclen = ID_VECLEN (arg_node);
        vectype = ID_VECTYPE (arg_node);
        constvec = ID_CONSTVEC (arg_node);
    }
    switch (vectype) {
    case T_nothing:
        fprintf (outfile, ":[");
        break;
    case T_int:
        intptr = (int *)constvec;
        if ((intptr == NULL) || (veclen < 1))
            return;
        else {
            fprintf (outfile, ":[%d", intptr[0]);
            for (i = 1; i < ((veclen < 10) ? (veclen) : (10)); i++)
                fprintf (outfile, ",%d", intptr[i]);
        }
        break;
    case T_float:
        fltptr = (float *)constvec;
        if ((fltptr == NULL) || (veclen < 1))
            return;
        else {
            fprintf (outfile, ":[%f", fltptr[0]);
            for (i = 1; i < ((veclen < 10) ? (veclen) : (10)); i++)
                fprintf (outfile, ",%f", fltptr[i]);
        }
        break;
    case T_double:
        dblptr = (double *)constvec;
        if ((dblptr == NULL) || (veclen < 1))
            return;
        else {
            fprintf (outfile, ":[%f", dblptr[0]);
            for (i = 1; i < ((veclen < 10) ? (veclen) : (10)); i++)
                fprintf (outfile, ",%f", dblptr[i]);
        }
        break;
    case T_bool:
        intptr = (int *)constvec;
        if ((intptr == NULL) || (veclen < 1))
            return;
        else {
            fprintf (outfile, ":[%s", ((intptr[0] == 0) ? ("false") : ("true")));
            for (i = 1; i < ((veclen < 10) ? (veclen) : (10)); i++)
                fprintf (outfile, ",%s", ((intptr[i] == 0) ? ("false") : ("true")));
        }
        break;
    case T_char:
        chrptr = (char *)constvec;
        if ((chrptr == NULL) || (veclen < 1))
            return;
        else {
            fprintf (outfile, ":[");
            if ((chrptr[0] >= ' ') && (chrptr[0] <= '~') && (chrptr[0] != '\'')) {
                fprintf (outfile, ",'%c'", chrptr[0]);
            } else {
                fprintf (outfile, ",'\\%o'", chrptr[0]);
            }
            for (i = 1; i < ((veclen < 10) ? (veclen) : (10)); i++)
                if ((chrptr[i] >= ' ') && (chrptr[i] <= '~') && (chrptr[i] != '\'')) {
                    fprintf (outfile, ",'%c'", chrptr[i]);
                } else {
                    fprintf (outfile, ",'\\%o'", chrptr[i]);
                }
        }
        break;
    default:
        return;
    }
    if (veclen > 10)
        fprintf (outfile, ",..]");
    else
        fprintf (outfile, "]");
    return;
}

/******************************************************************************/

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
                /*
                 * here's no break missing !
                 */
            case ACL_unknown:
#if 0
        fprintf(outfile,"ACCESS_IV: %s", nametab[NODE_TYPE(ACCESS_IV(access))]);
#else
                fprintf (outfile, "psi  ( %s ", VARDEC_NAME (ACCESS_IV (access)));
                fprintf (outfile, ", %s)", VARDEC_NAME (ACCESS_ARRAY (access)));
#endif
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
                        if (VARDEC_NAME (ACCESS_ARRAY (access)) != NULL)
                            fprintf (outfile, " ], %s)\n",
                                     VARDEC_NAME (ACCESS_ARRAY (access)));
                        else
                            fprintf (outfile, " ], ?)\n");
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
                        if (VARDEC_NAME (ACCESS_ARRAY (access)) != NULL) {
                            fprintf (outfile, " ], %s)\n",
                                     VARDEC_NAME (ACCESS_ARRAY (access)));
                        } else {
                            fprintf (outfile, " ], ?)\n");
                        }
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

/******************************************************************************/

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
#if 0
    fprintf(outfile," *");
    Print( pragma);
    FreePragma(pragma, NULL);
    INDENT;
#else
        fprintf (outfile, " *    ");
        FreePragma (pragma, NULL);
        fprintf (outfile, "\n");
        INDENT;
#endif
    } else {
        fprintf (outfile, " *   No proposal possible!\n");
        INDENT;
    }
    fprintf (outfile, " */\n");
    INDENT;

    DBUG_VOID_RETURN;
}

/******************************************************************************/

void
PrintIds (ids *arg)
{
    DBUG_ENTER ("PrintIds");

    while (arg != NULL) {
        DBUG_PRINT ("PRINT", ("%s", IDS_NAME (arg)));

        if (IDS_MOD (arg) != NULL) {
            fprintf (outfile, "%s:", IDS_MOD (arg));
        }
        fprintf (outfile, "%s", IDS_NAME (arg));

        DBUG_EXECUTE ("PRINT_RC", if ((IDS_REFCNT (arg) != -1) && show_refcnt) {
            fprintf (outfile, ":%d", IDS_REFCNT (arg));
        });
        DBUG_EXECUTE ("PRINT_NRC", if ((IDS_NAIVE_REFCNT (arg) != -1) && show_refcnt) {
            fprintf (outfile, "::%d", IDS_NAIVE_REFCNT (arg));
        });

        if ((!(optimize & OPT_RCO)) && show_refcnt && (IDS_REFCNT (arg) != -1)
            && (IDS_REFCNT (arg) != IDS_NAIVE_REFCNT (arg))) {
            fprintf (outfile, "**");
        }
        if (show_idx && IDS_USE (arg)) {
            Trav (IDS_USE (arg), NULL);
        }
        if (NULL != IDS_NEXT (arg)) {
            fprintf (outfile, ", ");
        }
        arg = IDS_NEXT (arg);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************/

void
PrintNums (nums *n)
{
    DBUG_ENTER ("PrintNums");

    while (n != NULL) {
        fprintf (outfile, "%d", NUMS_NUM (n));

        if (NUMS_NEXT (n) != NULL) {
            fprintf (outfile, ", ");
        }

        n = NUMS_NEXT (n);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************/

node *
PrintAssign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAssign");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "\n**MASKS - assign: \n");
                  PrintDefUseMasks (outfile, ASSIGN_DEFMASK (arg_node),
                                    ASSIGN_USEMASK (arg_node),
                                    INFO_PRINT_VARNO (arg_info));
                  PrintMrdMask (outfile, ASSIGN_MRDMASK (arg_node),
                                INFO_PRINT_VARNO (arg_info)););

    DBUG_EXECUTE ("WLI", if (N_let == NODE_TYPE (ASSIGN_INSTR (arg_node))
                             && F_psi == PRF_PRF (LET_EXPR (ASSIGN_INSTR (arg_node))))
                           DbugIndexInfo (ASSIGN_INDEX (arg_node)););

    if (N_icm == NODE_TYPE (ASSIGN_INSTR (arg_node))) {
        PrintIcm (ASSIGN_INSTR (arg_node), arg_info);
        fprintf (outfile, "\n");
        if (ASSIGN_NEXT (arg_node))
            Trav (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        PRINT_LINE_PRAGMA_IN_SIB (outfile, arg_node);
        DBUG_EXECUTE ("LINE", fprintf (outfile, "/*%03d*/", arg_node->lineno););

        if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_return)
            || ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_return)
                && (RETURN_EXPRS (ASSIGN_INSTR (arg_node)) != NULL))) {
            INDENT;
            Trav (ASSIGN_INSTR (arg_node), arg_info);
            fprintf (outfile, "\n");
        }

        if (ASSIGN_NEXT (arg_node)) {
            Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintBlock (node *arg_node, node *arg_info)
{
    static int not_yet_done_print_main_begin = 1;
    /*
     * This static variable assures that only once for the outer block of
     * the main() function initialization code is generated, but not for
     * subsequent blocks of perhaps loops or conditionals.
     */

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

    if (arg_info && /* arg_info may be NULL if Print() is called */
                    /* from within a debugger (PrintFundef did */
                    /* not create an info node). */
        not_yet_done_print_main_begin && (NODE_TYPE (arg_info) == N_info)
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
    fprintf (outfile, "}\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintLet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintLet");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if (LET_IDS (arg_node)) {
        PrintIds (LET_IDS (arg_node));
        fprintf (outfile, " = ");
    }
    Trav (LET_EXPR (arg_node), arg_info);
    fprintf (outfile, "; ");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintAnnotate (node *arg_node, node *arg_info)
{
    static char strbuffer1[256];
    static char strbuffer2[256];

    DBUG_ENTER ("PrintAnnotate");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if (ANNOTATE_TAG (arg_node) & CALL_FUN) {
        sprintf (strbuffer1, "PROFILE_BEGIN_UDF( %d ,%d )", ANNOTATE_FUNNUMBER (arg_node),
                 ANNOTATE_FUNAPNUMBER (arg_node));
    } else {
        if (ANNOTATE_TAG (arg_node) & RETURN_FROM_FUN) {
            sprintf (strbuffer1, "PROFILE_END_UDF( %d ,%d )",
                     ANNOTATE_FUNNUMBER (arg_node), ANNOTATE_FUNAPNUMBER (arg_node));
        } else {
            DBUG_ASSERT ((1 == 0), "wrong tag at N_annotate");
        }
    }

    if (ANNOTATE_TAG (arg_node) & INL_FUN) {
        sprintf (strbuffer2, "PROFILE_INLINE( %s )", strbuffer1);
    } else {
        strcpy (strbuffer2, strbuffer1);
    }

    if (ANNOTATE_TAG (arg_node) & LIB_FUN) {
        sprintf (strbuffer1, "PROFILE_LIBRARY( %s )", strbuffer2);
    } else {
        strcpy (strbuffer1, strbuffer2);
    }

    fprintf (outfile, "%s;", strbuffer1);

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintModul (node *arg_node, node *arg_info)
{
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
        fprintf (outfile, "int __dummy_value_which_is_completely_useless=0;\n\n");

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
                fprintf (outfile, " *  classtype %s;\n",
                         Type2String (MODUL_CLASSTYPE (arg_node), 0));
            }
            fprintf (outfile, " */\n");
            break;
        case F_prog:
            fprintf (outfile, "\n/*\n *  SAC-Program %s :\n */\n", puresacfilename);
            break;
        default:;
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
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

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
            PrintIds (IMPLIST_ITYPES (arg_node)); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (IMPLIST_ETYPES (arg_node) != NULL) {
            fprintf (outfile, "\n  explicit types: ");
            PrintIds (IMPLIST_ETYPES (arg_node)); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (IMPLIST_OBJS (arg_node) != NULL) {
            fprintf (outfile, "\n  global objects: ");
            PrintIds (IMPLIST_OBJS (arg_node)); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (IMPLIST_FUNS (arg_node) != NULL) {
            fprintf (outfile, "\n  funs: ");
            PrintIds (IMPLIST_FUNS (arg_node)); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        fprintf (outfile, "\n}\n");
    }

    if (IMPLIST_NEXT (arg_node))
        Trav (IMPLIST_NEXT (arg_node), arg_info); /* print further imports */

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintTypedef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintTypedef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    fprintf (outfile, "typedef %s ", Type2String (TYPEDEF_TYPE (arg_node), 0));
    if (TYPEDEF_MOD (arg_node) != NULL) {
        fprintf (outfile, "%s:", TYPEDEF_MOD (arg_node));
    }
    fprintf (outfile, "%s;\n", TYPEDEF_NAME (arg_node));

    if (TYPEDEF_COPYFUN (arg_node) != NULL) {
        fprintf (outfile, "\nextern void *%s(void *);\n", TYPEDEF_COPYFUN (arg_node));
        fprintf (outfile, "extern void %s(void *);\n\n", TYPEDEF_FREEFUN (arg_node));
    }

    if (TYPEDEF_NEXT (arg_node)) {
        Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintObjdef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintObjdef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    if ((OBJDEF_ICM (arg_node) != NULL) && (NODE_TYPE (OBJDEF_ICM (arg_node)) == N_icm)) {
        Trav (OBJDEF_ICM (arg_node), arg_info);
        fprintf (outfile, "\n");
    } else {
        if ((OBJDEF_STATUS (arg_node) == ST_imported) || print_objdef_for_header_file) {
            fprintf (outfile, "extern ");
        }

        fprintf (outfile, "%s ", Type2String (OBJDEF_TYPE (arg_node), 0));

        if (OBJDEF_MOD (arg_node) != NULL) {
            fprintf (outfile, "%s:", OBJDEF_MOD (arg_node));
        }

        fprintf (outfile, "%s", OBJDEF_NAME (arg_node));

        if (OBJDEF_EXPR (arg_node) != NULL) {
            fprintf (outfile, " = ");
            Trav (OBJDEF_EXPR (arg_node), arg_info);
        }

        fprintf (outfile, ";\n");

        if (OBJDEF_PRAGMA (arg_node) != NULL) {
            Trav (OBJDEF_PRAGMA (arg_node), arg_info);
        }
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

void
PrintFunctionHeader (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintFunctionHeader");

    PRINT_LINE_PRAGMA_IN_SIB (outfile, arg_node);

    if (0 != FUNDEF_INLINE (arg_node)) {
        fprintf (outfile, "inline ");
    }

    fprintf (outfile, "%s ", Type2String (FUNDEF_TYPES (arg_node), 0));

    if (FUNDEF_MOD (arg_node) != NULL) {
        fprintf (outfile, "%s:", FUNDEF_MOD (arg_node));
    }

    fprintf (outfile, "%s(", FUNDEF_NAME (arg_node));

    if (FUNDEF_ARGS (arg_node) != NULL) {
        Trav (FUNDEF_ARGS (arg_node), arg_info); /* print args of function */
    }

    fprintf (outfile, ")");

    DBUG_VOID_RETURN;
}

/******************************************************************************/

/*
 * Remark for PrintFundef:
 *
 *  If C-code is to be generated, which means that an N_icm node already
 *  hangs on node[3], additional extern declarations for function
 *  definitions are printed.
 */

node *
PrintFundef (node *arg_node, node *arg_info)
{
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
                || ((NULL != FUNDEF_RETURN (arg_node))
                    && (N_icm == NODE_TYPE (FUNDEF_RETURN (arg_node)))
                    && (0 != strcmp (FUNDEF_NAME (arg_node), "main")))) {

                fprintf (outfile, "extern ");

                if ((NULL != FUNDEF_ICM (arg_node))
                    && (N_icm == NODE_TYPE (FUNDEF_ICM (arg_node)))
                    && (FUNDEF_STATUS (arg_node) != ST_spmdfun)) {
                    Trav (FUNDEF_ICM (arg_node), arg_info); /* print N_icm ND_FUN_DEC */
                } else {
                    PrintFunctionHeader (arg_node, arg_info);
                }

                fprintf (outfile, ";\n");

                if (FUNDEF_PRAGMA (arg_node) != NULL) {
                    Trav (FUNDEF_PRAGMA (arg_node), arg_info);
                }
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
                          PrintDefUseMasks (outfile, FUNDEF_DEFMASK (arg_node),
                                            FUNDEF_USEMASK (arg_node),
                                            INFO_PRINT_VARNO (arg_info)););

            if ((FUNDEF_ICM (arg_node) != NULL)
                && (N_icm == NODE_TYPE (FUNDEF_ICM (arg_node)))) {
                Trav (FUNDEF_ICM (arg_node), arg_info); /* print N_icm ND_FUN_DEC */
            } else {
                PrintFunctionHeader (arg_node, arg_info);
            }

            fprintf (outfile, "\n");
            Trav (FUNDEF_BODY (arg_node), arg_info); /* traverse function body */

            if (FUNDEF_PRAGMA (arg_node) != NULL) {
                Trav (FUNDEF_PRAGMA (arg_node), arg_info);
            }

            if ((FUNDEF_STATUS (arg_node) == ST_spmdfun)
                && (compiler_phase == PH_genccode)) {
                fprintf (outfile, "\n#endif  /* SAC_DO_MULTITHREAD */\n\n");
            }

            if (INFO_PRINT_SEPARATE (arg_info)) {
                fclose (outfile);
                function_counter++;
            }
        }
    }

    if (FUNDEF_NEXT (arg_node)) {
        Trav (FUNDEF_NEXT (arg_node), arg_info); /* traverse next function */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

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
    case F_max: {
        /* primitive functions that are printed as function application */
        fprintf (outfile, "%s( ", prf_string[PRF_PRF (arg_node)]);
        Trav (PRF_ARGS (arg_node), arg_info);
        fprintf (outfile, " )");
        break;
    }
    default: {
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
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintStr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintStr");

    DBUG_ASSERT ((N_str == NODE_TYPE (arg_node)), "wrong node type");

    fprintf (outfile, "\"%s\"", STR_STRING (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintId (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintId");

    DBUG_ASSERT ((N_id == NODE_TYPE (arg_node)), "wrong node type");

    if ((ID_ATTRIB (arg_node) == ST_global) && (ID_MOD (arg_node) != NULL)) {
        fprintf (outfile, "%s:", ID_MOD (arg_node));
    }

    fprintf (outfile, "%s", ID_NAME (arg_node));

    DBUG_EXECUTE ("PRINT_RC", if ((ID_REFCNT (arg_node) != -1) && show_refcnt) {
        fprintf (outfile, ":%d", ID_REFCNT (arg_node));
    });
    DBUG_EXECUTE ("PRINT_NRC", if ((ID_NAIVE_REFCNT (arg_node) != -1) && show_refcnt) {
        fprintf (outfile, "::%d", ID_NAIVE_REFCNT (arg_node));
    });

    if ((!(optimize & OPT_RCO)) && show_refcnt && (ID_REFCNT (arg_node) != -1)
        && (ID_REFCNT (arg_node) != ID_NAIVE_REFCNT (arg_node))) {
        fprintf (outfile, "**");
    }

    if (compiler_phase != PH_genccode) {
        DBUG_EXECUTE ("PRINT_CAR", DbugPrintArray (arg_node););
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintNum (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNum");

    fprintf (outfile, "%d", NUM_VAL (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

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

/******************************************************************************/

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

/******************************************************************************/

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

/******************************************************************************/

node *
PrintBool (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintBool");

    if (0 == BOOL_VAL (arg_node)) {
        fprintf (outfile, "false");
    } else
        fprintf (outfile, "true");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintReturn");

    if (RETURN_EXPRS (arg_node) && (!RETURN_INWITH (arg_node))) {
        if (arg_info && /* may be NULL if call from within debugger */
            (NODE_TYPE (arg_info) = N_info) && (compiler_phase == PH_genccode)
            && (INFO_PRINT_FUNDEF (arg_info) != NULL)
            && (strcmp (FUNDEF_NAME (INFO_PRINT_FUNDEF (arg_info)), "main") == 0)) {
            GSCPrintMainEnd ();
            INDENT;
        }

        fprintf (outfile, "return( ");
        Trav (RETURN_EXPRS (arg_node), arg_info);
        fprintf (outfile, " );");
    }

    if (RETURN_INWITH (arg_node)) {
        INFO_PRINT_WITH_RET (arg_info) = arg_node;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintAp (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAp");

    if (AP_MOD (arg_node) != NULL) {
        fprintf (outfile, "%s:", AP_MOD (arg_node));
    }
    fprintf (outfile, "%s(", AP_NAME (arg_node));

    DBUG_ASSERT ((arg_info != NULL), "PrintAp() called with arg_info==NULL");

    if (INFO_PRINT_PRAGMA_WLCOMP (arg_info)) {
        /*
         * Here, we are printing a wlcomp pragma.
         */
        DBUG_ASSERT ((AP_ARGS (arg_node) != NULL), "Illegal wlcomp pragma specification");
        Trav (EXPRS_EXPR (AP_ARGS (arg_node)), arg_info);
        if (EXPRS_NEXT (AP_ARGS (arg_node)) == NULL) {
            fprintf (outfile, ", Default");
        } else {
            fprintf (outfile, ", ");
            Trav (EXPRS_EXPR (EXPRS_NEXT (AP_ARGS (arg_node))), arg_info);
        }
    } else {
        /*
         * Here, we are printing a regular function application.
         */
        if (AP_ARGS (arg_node)) {
            Trav (AP_ARGS (arg_node), arg_info);
        }
    }

    fprintf (outfile, ")");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintCast (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCast");

    fprintf (outfile, "(: %s) ", Type2String (CAST_TYPE (arg_node), 0));
    Trav (CAST_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintExprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintExprs");

    Trav (EXPRS_EXPR (arg_node), arg_info);

    if (EXPRS_NEXT (arg_node) != NULL) {
        fprintf (outfile, ", ");
        Trav (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintArg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintArg");

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**%d: ", ARG_VARNO (arg_node)););

    fprintf (outfile, "%s",
             Type2String (ARG_TYPE (arg_node),
                          INFO_PRINT_OMIT_FORMAL_PARAMS (arg_info) ? 0 : 1));

    DBUG_EXECUTE ("PRINT_RC", if ((1 == show_refcnt) && (-1 != ARG_REFCNT (arg_node))) {
        fprintf (outfile, ":%d", ARG_REFCNT (arg_node));
    });
    DBUG_EXECUTE ("PRINT_NRC",
                  if ((1 == show_refcnt) && (-1 != ARG_NAIVE_REFCNT (arg_node))) {
                      fprintf (outfile, "::%d", ARG_NAIVE_REFCNT (arg_node));
                  });

    if ((!(optimize & OPT_RCO)) && show_refcnt && (ARG_REFCNT (arg_node) != -1)
        && (ARG_REFCNT (arg_node) != ARG_NAIVE_REFCNT (arg_node))) {
        fprintf (outfile, "**");
    }

    if (ARG_COLCHN (arg_node) && show_idx) {
        Trav (ARG_COLCHN (arg_node), arg_info);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (outfile, ", ");
        Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintVardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintVardec");

    INDENT;

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**%d: ", VARDEC_VARNO (arg_node)););

    fprintf (outfile, "%s", Type2String (VARDEC_TYPE (arg_node), 1));
    if (VARDEC_COLCHN (arg_node) && show_idx) {
        Trav (VARDEC_COLCHN (arg_node), arg_info);
    }
    fprintf (outfile, ";\n");
    if (VARDEC_NEXT (arg_node)) {
        Trav (VARDEC_NEXT (arg_node), arg_info);
    } else {
        fprintf (outfile, "\n");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintDo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintDo");

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "\n**used vars - do loop: ");
                  PrintDefUseMask (outfile, DO_TERMMASK (arg_node),
                                   INFO_PRINT_VARNO (arg_info)););

    fprintf (outfile, "do\n");

    DBUG_EXECUTE ("VARS", fprintf (outfile, "**(NAIVE)DEFVARS\n");
                  PrintIds (DO_DEFVARS (arg_node)); fprintf (outfile, "\n");
                  PrintIds (DO_NAIVE_DEFVARS (arg_node));
                  fprintf (outfile, "\n**(NAIVE)USEVARS\n");
                  PrintIds (DO_USEVARS (arg_node)); fprintf (outfile, "\n");
                  PrintIds (DO_NAIVE_USEVARS (arg_node)); fprintf (outfile, "\n"););

    if (DO_BODY (arg_node) != NULL) {

        DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**MASKS - do body: \n");
                      PrintDefUseMasks (outfile, DO_DEFMASK (arg_node),
                                        DO_USEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info));
                      PrintMrdMask (outfile, DO_MRDMASK (arg_node),
                                    INFO_PRINT_VARNO (arg_info)););

        indent++;
        Trav (DO_BODY (arg_node), arg_info); /* traverse body of loop */
        indent--;
    }

    INDENT;
    fprintf (outfile, "while( ");
    Trav (DO_COND (arg_node), arg_info);
    fprintf (outfile, " );\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintEmpty (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintEmpty");

    INDENT;
    fprintf (outfile, "/* empty */\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintWhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWhile");

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "\n**used vars - while loop: ");
                  PrintDefUseMask (outfile, WHILE_TERMMASK (arg_node),
                                   INFO_PRINT_VARNO (arg_info)););

    fprintf (outfile, "while( ");
    Trav (WHILE_COND (arg_node), arg_info);
    fprintf (outfile, " )\n");

    DBUG_EXECUTE ("VARS", fprintf (outfile, "**(NAIVE)DEFVARS\n");
                  PrintIds (WHILE_DEFVARS (arg_node)); fprintf (outfile, "\n");
                  PrintIds (WHILE_NAIVE_DEFVARS (arg_node));
                  fprintf (outfile, "\n**(NAIVE)USEVARS\n");
                  PrintIds (WHILE_USEVARS (arg_node)); fprintf (outfile, "\n");
                  PrintIds (WHILE_NAIVE_USEVARS (arg_node)); fprintf (outfile, "\n"););

    if (WHILE_BODY (arg_node) != NULL) {

        DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**MASKS - while body: \n");
                      PrintDefUseMasks (outfile, WHILE_DEFMASK (arg_node),
                                        WHILE_USEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info));
                      PrintMrdMask (outfile, WHILE_MRDMASK (arg_node),
                                    INFO_PRINT_VARNO (arg_info)););

        indent++;
        Trav (WHILE_BODY (arg_node), arg_info); /* traverse body of loop */
        indent--;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintCond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCond");

    fprintf (outfile, "if (");

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "\n**used vars - cond: ");
                  PrintDefUseMask (outfile, COND_CONDUSEMASK (arg_node),
                                   INFO_PRINT_VARNO (arg_info)););

    Trav (COND_COND (arg_node), arg_info);
    fprintf (outfile, ")\n");

    if (COND_THEN (arg_node) != NULL) {

        DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**MASKS - then: \n");
                      PrintDefUseMasks (outfile, COND_THENDEFMASK (arg_node),
                                        COND_THENUSEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info)););

        DBUG_EXECUTE ("VARS", fprintf (outfile, "\n**(NAIVE)VARS - then\n");
                      PrintIds (COND_THENVARS (arg_node)); fprintf (outfile, "\n");
                      PrintIds (COND_NAIVE_THENVARS (arg_node));
                      fprintf (outfile, "\n"););

        Trav (COND_THEN (arg_node), arg_info);
    }

    INDENT;
    fprintf (outfile, "else\n");

    if (COND_ELSE (arg_node) != NULL) {

        DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**MASKS - else: \n");
                      PrintDefUseMasks (outfile, COND_ELSEDEFMASK (arg_node),
                                        COND_ELSEUSEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info)););

        DBUG_EXECUTE ("VARS", fprintf (outfile, "\n**(NAIVE)VARS - else\n");
                      PrintIds (COND_ELSEVARS (arg_node)); fprintf (outfile, "\n");
                      PrintIds (COND_NAIVE_ELSEVARS (arg_node));
                      fprintf (outfile, "\n"););

        Trav (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintWith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWith");

    fprintf (outfile, "old_with (");
    Trav (WITH_GEN (arg_node), arg_info);
    fprintf (outfile, ") ");

    Trav (WITH_OPERATOR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintGenator (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintGenator");

    Trav (GEN_LEFT (arg_node), arg_info);
    if ((-1 == arg_node->info.ids->refcnt) || (0 == show_refcnt)) {
        fprintf (outfile, " <= %s <= ", arg_node->info.ids->id);
    } else {
        fprintf (outfile, " <= %s:%d <= ", arg_node->info.ids->id,
                 arg_node->info.ids->refcnt);
    }
    Trav (GEN_RIGHT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintGenarray (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintGenarray");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (GENARRAY_BODY (arg_node)))) != N_return) {
        /* right now INFO_PRINT_WITH_RET(arg_info) is NULL, but in PrintReturn it
           will be replaced by a pointer to an N_return node instead of
           printing it. */
        fprintf (outfile, "\n");
        Trav (GENARRAY_BODY (arg_node), arg_info);
        ret_node = INFO_PRINT_WITH_RET (arg_info);

        INDENT;
    } else {
        fprintf (outfile, "\n");
        INDENT;
        ret_node = ASSIGN_INSTR (BLOCK_INSTR (GENARRAY_BODY (arg_node)));
    }

    DBUG_ASSERT (ret_node != NULL, "genarray without return-statement");

    fprintf (outfile, "genarray( ");
    Trav (GENARRAY_ARRAY (arg_node), arg_info);
    fprintf (outfile, ", ");
    Trav (RETURN_EXPRS (ret_node), arg_info);
    fprintf (outfile, ")");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintModarray (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintModarray");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (MODARRAY_BODY (arg_node)))) != N_return) {
        fprintf (outfile, "\n");
        Trav (MODARRAY_BODY (arg_node), arg_info);
        ret_node = INFO_PRINT_WITH_RET (arg_info);

        INDENT;
    } else {
        fprintf (outfile, "\n");
        INDENT;
        ret_node = ASSIGN_INSTR (BLOCK_INSTR (MODARRAY_BODY (arg_node)));
    }

    DBUG_ASSERT (ret_node != NULL, "modarray without return-statement");

    fprintf (outfile, "modarray( ");
    Trav (MODARRAY_ARRAY (arg_node), arg_info);
    fprintf (outfile, ", %s, ", MODARRAY_ID (arg_node));
    Trav (RETURN_EXPRS (ret_node), arg_info);
    fprintf (outfile, ")");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintFoldfun (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintFold");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (FOLDFUN_BODY (arg_node)))) != N_return) {
        fprintf (outfile, "\n");
        Trav (FOLDFUN_BODY (arg_node), arg_info);
        ret_node = INFO_PRINT_WITH_RET (arg_info);

        INDENT;
    } else {
        fprintf (outfile, "\n");
        INDENT;
        ret_node = ASSIGN_INSTR (BLOCK_INSTR (FOLDFUN_BODY (arg_node)));
    }

    DBUG_ASSERT (ret_node != NULL, "foldfun without return-statement");

    fprintf (outfile, "fold( ");
    if (NULL != FOLDFUN_MOD (arg_node)) {
        fprintf (outfile, "%s:", FOLDFUN_MOD (arg_node));
    }
    fprintf (outfile, "%s, ", FOLDFUN_NAME (arg_node));

    Trav (FOLDFUN_NEUTRAL (arg_node), arg_info);
    fprintf (outfile, ", ");
    Trav (RETURN_EXPRS (ret_node), arg_info);
    fprintf (outfile, " )");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintFoldprf (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintFold");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (FOLDPRF_BODY (arg_node)))) != N_return) {
        fprintf (outfile, "\n");
        Trav (FOLDPRF_BODY (arg_node), arg_info);
        ret_node = INFO_PRINT_WITH_RET (arg_info);

        INDENT;
    } else {
        fprintf (outfile, "\n");
        INDENT;
        ret_node = ASSIGN_INSTR (BLOCK_INSTR (FOLDPRF_BODY (arg_node)));
    }

    DBUG_ASSERT (ret_node != NULL, "foldprf without return-statement");

    fprintf (outfile, "fold( %s, ", prf_string[FOLDPRF_PRF (arg_node)]);

    if (FOLDPRF_NEUTRAL (arg_node) != NULL) {
        Trav (FOLDPRF_NEUTRAL (arg_node), arg_info);
        fprintf (outfile, ", ");
    }

    Trav (RETURN_EXPRS (ret_node), arg_info);
    fprintf (outfile, " )");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

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

/******************************************************************************/

node *
PrintDec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintDec");

    fprintf (outfile, "--");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintInc (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintInc");

    fprintf (outfile, "++");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintPost (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPost");

    PrintIds (arg_node->info.ids);
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, ";");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintPre (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPre");

    Trav (arg_node->node[0], arg_info);
    PrintIds (arg_node->info.ids);
    fprintf (outfile, ";");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintVectInfo (node *arg_node, node *arg_info)
{
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
            fprintf (outfile, ":IDX(%s)", Type2String (VINFO_TYPE (arg_node), 0));
            break;
        default:
            DBUG_ASSERT (0, "illegal N_vinfo-flag!");
        }

        if (VINFO_NEXT (arg_node)) {
            Trav (VINFO_NEXT (arg_node), arg_info);
        }
        fprintf (outfile, " ");
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintIcm (node *arg_node, node *arg_info)
{
    int compiled_icm = 0;

    DBUG_ENTER ("PrintIcm");

    DBUG_PRINT ("PRINT", ("icm-node %s\n", ICM_NAME (arg_node)));

    if (ICM_INDENT (arg_node) < 0) {
        indent += ICM_INDENT (arg_node);
    }

    INDENT
    if (compiler_phase == PH_genccode) {
#define ICM_ALL
#define ICM_DEF(prf, trf)                                                                \
    if (strcmp (ICM_NAME (arg_node), #prf) == 0) {                                       \
        Print##prf (ICM_ARGS (arg_node), arg_info);                                      \
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
            compiled_icm = 1;
        }
    }

    if ((compiler_phase != PH_genccode) || (compiled_icm == 0)) {

        if ((strcmp (ICM_NAME (arg_node), "ND_FUN_RET") == 0)
            && (strcmp (FUNDEF_NAME (INFO_PRINT_FUNDEF (arg_info)), "main") == 0)
            && (compiler_phase == PH_genccode)) {
            GSCPrintMainEnd ();
            INDENT;
        }

        fprintf (outfile, "SAC_%s(", ICM_NAME (arg_node));
        if (NULL != ICM_ARGS (arg_node)) {
            Trav (ICM_ARGS (arg_node), arg_info);
        }
        fprintf (outfile, ")");

        if (NULL != ICM_NEXT (arg_node)) {
            if (0 == strcmp (ICM_NAME (arg_node), "ND_TYPEDEF_ARRAY")) {
                /*
                 * ICM's within the typedef-chain are connected via ICM_NEXT!
                 */
                fprintf (outfile, "\n");
                INDENT;
            } else {
                /*
                 * ICM's that handle the arguments in fun-decl's are linked
                 * via ICM_NEXT as well!! These have to be connected by colons!
                 */
                fprintf (outfile, ", ");
            }

            Trav (ICM_NEXT (arg_node), arg_info);
        }
    }

    if (ICM_INDENT (arg_node) > 0) {
        indent += ICM_INDENT (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

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
        PrintIds (PRAGMA_EFFECT (arg_node));
        fprintf (outfile, "\n");
    }

    if (PRAGMA_TOUCH (arg_node) != NULL) {
        fprintf (outfile, "#pragma touch ");
        PrintIds (PRAGMA_TOUCH (arg_node));
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
        DBUG_ASSERT ((arg_info != NULL), "No arg_info node in print traversal");
        INFO_PRINT_PRAGMA_WLCOMP (arg_info) = 1;
        Trav (PRAGMA_WLCOMP_APS (arg_node), arg_info);
        INFO_PRINT_PRAGMA_WLCOMP (arg_info) = 0;
    }

    fprintf (outfile, "\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

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

        INDENT
        fprintf (outfile, " * in:");
        DFMPrintMask (outfile, " %s", SPMD_IN (arg_node));
        fprintf (outfile, "\n");

        INDENT
        fprintf (outfile, " * inout:");
        DFMPrintMask (outfile, " %s", SPMD_INOUT (arg_node));
        fprintf (outfile, "\n");

        INDENT
        fprintf (outfile, " * out:");
        DFMPrintMask (outfile, " %s", SPMD_OUT (arg_node));
        fprintf (outfile, "\n");

        INDENT
        fprintf (outfile, " * shared:");
        DFMPrintMask (outfile, " %s", SPMD_SHARED (arg_node));
        fprintf (outfile, "\n");

        INDENT
        fprintf (outfile, " * local:");
        DFMPrintMask (outfile, " %s", SPMD_LOCAL (arg_node));
        fprintf (outfile, "\n");

        INDENT
        fprintf (outfile, " */\n");

        Trav (SPMD_REGION (arg_node), arg_info);

        INDENT
        fprintf (outfile, "/*** end of SPMD region ***/\n");
    } else {
        /*
         * print ICMs
         */
        Trav (SPMD_ICM_BEGIN (arg_node), arg_info);
        fprintf (outfile, "\n");
        Trav (SPMD_ICM_PARALLEL (arg_node), arg_info);
        Trav (SPMD_ICM_ALTSEQ (arg_node), arg_info);
        fprintf (outfile, "\n");
        Trav (SPMD_ICM_SEQUENTIAL (arg_node), arg_info);
        Trav (SPMD_ICM_END (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintSync (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintSync");

    fprintf (outfile, "\n");
    INDENT;

    fprintf (outfile, "/*** begin of sync region ***\n");

    INDENT
    fprintf (outfile, " * in:");
    DFMPrintMask (outfile, " %s", SYNC_IN (arg_node));
    fprintf (outfile, "\n");

    INDENT
    fprintf (outfile, " * inout:");
    DFMPrintMask (outfile, " %s", SYNC_INOUT (arg_node));
    fprintf (outfile, "\n");

    INDENT
    fprintf (outfile, " * out:");
    DFMPrintMask (outfile, " %s", SYNC_OUT (arg_node));
    fprintf (outfile, "\n");

    INDENT
    fprintf (outfile, " * outrep:");
    DFMPrintMask (outfile, " %s", SYNC_OUTREP (arg_node));
    fprintf (outfile, "\n");

    INDENT
    fprintf (outfile, " * local:");
    DFMPrintMask (outfile, " %s", SYNC_LOCAL (arg_node));
    fprintf (outfile, "\n");

    INDENT
    fprintf (outfile, " */\n");

    indent++;
    SYNC_REGION (arg_node) = Trav (SYNC_REGION (arg_node), arg_info);
    indent--;

    INDENT
    fprintf (outfile, "/*** end of sync region ***/\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNwith(node *arg_node, node *arg_info)
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

    DBUG_ASSERT (arg_info, "arg_info is NULL");

    buffer = INFO_PRINT_INT_SYN (arg_info);
    tmp_nwith = INFO_PRINT_NWITH (arg_info);
    INFO_PRINT_NWITH (arg_info) = arg_node;

    indent += 2;

    DBUG_EXECUTE ("WLI",
                  fprintf (outfile,
                           "\n** WLI N_Nwith : "
                           "(PARTS %d, REF %d(%d,%d), CPLX %d, FOLDABLE %d, NO_CHANCE "
                           "%d)\n",
                           NWITH_PARTS (arg_node), NWITH_REFERENCED (arg_node),
                           NWITH_REFERENCED_FOLD (arg_node),
                           NWITH_REFERENCES_FOLDED (arg_node), NWITH_COMPLEX (arg_node),
                           NWITH_FOLDABLE (arg_node), NWITH_NO_CHANCE (arg_node)););

    INFO_PRINT_ACCESS (arg_info) = NWITH_WLAA (arg_node);

    if (NWITH_PRAGMA (arg_node) != NULL) {
        Trav (NWITH_PRAGMA (arg_node), arg_info);
        INDENT;
    }

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
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        indent--;
    } else {
        /* output format 2 */
        INFO_PRINT_INT_SYN (arg_info) = (node *)!NULL; /* set != NULL */
        fprintf (outfile, "with ");
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    if (NPART_NEXT (NWITH_PART (arg_node)) == NULL) {
        /*
         * output format 2: now we have in
         * INFO_PRINT_INT_SYN(arg_info) the last expr.
         */
        if (WO_modarray == NWITH_TYPE (arg_node))
            fprintf (outfile, ", dummy, ");
        else
            fprintf (outfile, ", ");
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
        PrintIds (NWITHID_VEC (arg_node));
        if (NWITHID_IDS (arg_node)) {
            fprintf (outfile, "=");
        }
    }

    if (NWITHID_IDS (arg_node)) {
        fprintf (outfile, "[");
        PrintIds (NWITHID_IDS (arg_node));
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
 *   prints a generator
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
    /* print first operator and eventually original first operator */
    fprintf (outfile, " %s", prf_string[NGEN_OP1 (arg_node)]);
    DBUG_EXECUTE ("ORIG_GENS",
                  fprintf (outfile, "::%s", prf_string[NGEN_OP1_ORIG (arg_node)]););
    fprintf (outfile, " ");

    /* print indices */
    if (INFO_PRINT_NWITH (arg_info) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (INFO_PRINT_NWITH (arg_info)) == N_Nwith),
                     "INFO_PRINT_NWITH is no N_Nwith node");
        NWITH_WITHID (INFO_PRINT_NWITH (arg_info))
          = Trav (NWITH_WITHID (INFO_PRINT_NWITH (arg_info)), arg_info);
    } else {
        fprintf (outfile, "?");
    }

    /* print second operator and eventually original operator */
    fprintf (outfile, " %s", prf_string[NGEN_OP2 (arg_node)]);
    DBUG_EXECUTE ("ORIG_GENS",
                  fprintf (outfile, "::%s", prf_string[NGEN_OP2_ORIG (arg_node)]););
    fprintf (outfile, " ");
    /* print lower bound */
    if (NGEN_BOUND2 (arg_node)) {
        Trav (NGEN_BOUND2 (arg_node), arg_info);
    } else {
        fprintf (outfile, " .");
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

    fprintf (outfile, ")");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNcode(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Ncode-nodes
 *
 ******************************************************************************/

node *
PrintNcode (node *arg_node, node *arg_info)
{
    node *block;

    DBUG_ENTER ("PrintNcode");
    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "\n**MASKS - Ncode: \n");
                  PrintDefUseMasks (outfile, NCODE_DEFMASK (arg_node),
                                    NCODE_USEMASK (arg_node),
                                    INFO_PRINT_VARNO (arg_info)););

    /* print the code section; first the body */
    block = NCODE_CBLOCK (arg_node);
    fprintf (outfile, " {");
    if (block != NULL) {
        fprintf (outfile, "\n");
        indent++;

        if (BLOCK_VARDEC (block) != NULL) {
            BLOCK_VARDEC (block) = Trav (BLOCK_VARDEC (block), arg_info);
            fprintf (outfile, "\n");
        }

        if (BLOCK_INSTR (block) != NULL) {
            BLOCK_INSTR (block) = Trav (BLOCK_INSTR (block), arg_info);
        }

        indent--;
        INDENT;
    }
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

    fprintf (outfile, "}");

    /*
     * print the expression if internal syntax should be used.
     * else return expr in INFO_PRINT_INT_SYN(arg_info)
     */
    if (NCODE_CEXPR (arg_node) != NULL) {
        if (INFO_PRINT_INT_SYN (arg_info) != NULL) {
            INFO_PRINT_INT_SYN (arg_info) = NCODE_CEXPR (arg_node);
        } else {
            fprintf (outfile, " : ");
            NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNpart(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Npart nodes
 *
 ******************************************************************************/

node *
PrintNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintNpart");
    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "\n**MASKS - Npart: \n");
                  PrintDefUseMasks (outfile, NPART_DEFMASK (arg_node),
                                    NPART_USEMASK (arg_node),
                                    INFO_PRINT_VARNO (arg_info)););

    /* print generator */
    if (INFO_PRINT_INT_SYN (arg_info) == NULL) {
        INDENT; /* each gen in a new line. */
    }
    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    DBUG_ASSERT ((NPART_CODE (arg_node) != NULL),
                 "part within WL without pointer to N_Ncode");

    NPART_CODE (arg_node) = Trav (NPART_CODE (arg_node), arg_info);

    if (NPART_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",\n");
        /*
         * continue with other parts
         */
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    } else {
        fprintf (outfile, "\n");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNwithop(node *arg_node, node *arg_info)
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

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**MASKS - Nwithop: \n");
                  PrintDefUseMasks (outfile, NWITHOP_DEFMASK (arg_node),
                                    NWITHOP_USEMASK (arg_node),
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
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintNwith2(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_Nwith2-nodes
 *
 ******************************************************************************/

node *
PrintNwith2 (node *arg_node, node *arg_info)
{
    node *code, *tmp_nwith2;

    DBUG_ENTER ("PrintNwith2");

    tmp_nwith2 = INFO_PRINT_NWITH (arg_info);
    INFO_PRINT_NWITH (arg_info) = arg_node;

    indent += 2;

    fprintf (outfile, "with2 (");
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    fprintf (outfile, ")\n");

    if (NWITH2_SCHEDULING (arg_node) != NULL) {
        INDENT
        fprintf (outfile, "/* scheduling :");
        SCHPrintScheduling (outfile, NWITH2_SCHEDULING (arg_node));
        fprintf (outfile, " */\n");
    }

    INDENT
    fprintf (outfile, "/********** operators: **********/\n");
    code = NWITH2_CODE (arg_node);
    while (code != NULL) {
        INDENT
        fprintf (outfile, "op_%d =", NCODE_NO (code));
        indent++;
        code = Trav (code, arg_info);
        indent--;
        code = NCODE_NEXT (code);

        if (code != NULL) {
            fprintf (outfile, ",\n");
        } else {
            fprintf (outfile, "\n");
        }
    }

    if (NWITH2_SEGS (arg_node) != NULL) {
        NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    }

    INDENT
    fprintf (outfile, "/********** conexpr: **********/\n");
    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    fprintf (outfile, ")");

    indent -= 2;

    INFO_PRINT_NWITH (arg_info) = tmp_nwith2;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLseg(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLseg-nodes
 *
 ******************************************************************************/

node *
PrintWLseg (node *arg_node, node *arg_info)
{
    node *seg;
    int i = 0;

    DBUG_ENTER ("PrintWLseg");

    seg = arg_node;
    while (seg != NULL) {
        if (WLSEG_SCHEDULING (seg) == NULL) {
            INDENT
            fprintf (outfile, "/********** segment %d: **********/\n", i++);
        } else {
            INDENT
            fprintf (outfile, "/********** segment %d: **********\n", i++);
            INDENT
            fprintf (outfile, " * scheduling: ");
            SCHPrintScheduling (outfile, WLSEG_SCHEDULING (seg));
            fprintf (outfile, "\n");
            INDENT
            fprintf (outfile, " */\n");
        }

        WLSEG_CONTENTS (seg) = Trav (WLSEG_CONTENTS (seg), arg_info);
        seg = WLSEG_NEXT (seg);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLblock(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLblock-nodes
 *
 ******************************************************************************/

node *
PrintWLblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLblock");

    INDENT
    fprintf (outfile, "(%d -> %d), block%d[%d] %d: ", WLBLOCK_BOUND1 (arg_node),
             WLBLOCK_BOUND2 (arg_node), WLBLOCK_LEVEL (arg_node), WLBLOCK_DIM (arg_node),
             WLBLOCK_STEP (arg_node));

    if (WLBLOCK_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLBLOCK_NEXTDIM (arg_node) = Trav (WLBLOCK_NEXTDIM (arg_node), arg_info);
        indent--;
    }

    if (WLBLOCK_CONTENTS (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLBLOCK_CONTENTS (arg_node) = Trav (WLBLOCK_CONTENTS (arg_node), arg_info);
        indent--;
    }

    if (WLBLOCK_NEXT (arg_node) != NULL) {
        WLBLOCK_NEXT (arg_node) = Trav (WLBLOCK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLublock(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLublock-nodes
 *
 ******************************************************************************/

node *
PrintWLublock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLublock");

    INDENT
    fprintf (outfile, "(%d -> %d), ublock%d[%d] %d: ", WLUBLOCK_BOUND1 (arg_node),
             WLUBLOCK_BOUND2 (arg_node), WLUBLOCK_LEVEL (arg_node),
             WLUBLOCK_DIM (arg_node), WLUBLOCK_STEP (arg_node));

    if (WLUBLOCK_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLUBLOCK_NEXTDIM (arg_node) = Trav (WLUBLOCK_NEXTDIM (arg_node), arg_info);
        indent--;
    }

    if (WLUBLOCK_CONTENTS (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLUBLOCK_CONTENTS (arg_node) = Trav (WLUBLOCK_CONTENTS (arg_node), arg_info);
        indent--;
    }

    if (WLUBLOCK_NEXT (arg_node) != NULL) {
        WLUBLOCK_NEXT (arg_node) = Trav (WLUBLOCK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLstride(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLstride-nodes
 *
 * remark:
 *   N_WLstride, N_WLstriVar differs in output.
 *   The former prints '->', the latter '=>' !!!
 *
 ******************************************************************************/

node *
PrintWLstride (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLstride");

    INDENT
    fprintf (outfile, "(%d -> %d), step%d[%d] %d\n", WLSTRIDE_BOUND1 (arg_node),
             WLSTRIDE_BOUND2 (arg_node), WLSTRIDE_LEVEL (arg_node),
             WLSTRIDE_DIM (arg_node), WLSTRIDE_STEP (arg_node));

    indent++;
    WLSTRIDE_CONTENTS (arg_node) = Trav (WLSTRIDE_CONTENTS (arg_node), arg_info);
    indent--;

    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        WLSTRIDE_NEXT (arg_node) = Trav (WLSTRIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLgrid(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLgrid-nodes
 *
 * remark:
 *   N_WLgrid, N_WLgridVar differs in output.
 *   The former prints '->', the latter '=>' !!!
 *
 ******************************************************************************/

node *
PrintWLgrid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLgrid");

    INDENT
    fprintf (outfile, "(%d -> %d): ", WLGRID_BOUND1 (arg_node), WLGRID_BOUND2 (arg_node));

    indent++;
    if (WLGRID_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        WLGRID_NEXTDIM (arg_node) = Trav (WLGRID_NEXTDIM (arg_node), arg_info);
    } else {
        if (WLGRID_CODE (arg_node) != NULL) {
            fprintf (outfile, "op_%d\n", NCODE_NO (WLGRID_CODE (arg_node)));
        } else {
            if (INFO_PRINT_NWITH (arg_info) != NULL) {
                DBUG_ASSERT ((NODE_TYPE (INFO_PRINT_NWITH (arg_info)) == N_Nwith2),
                             "INFO_PRINT_NWITH( arg_info) contains no N_Nwith2 node");
                switch (NWITH2_TYPE (INFO_PRINT_NWITH (arg_info))) {
                case WO_genarray:
                    fprintf (outfile, "init\n");
                    break;
                case WO_modarray:
                    fprintf (outfile, "copy\n");
                    break;
                case WO_foldfun:
                    /* here is no break missing! */
                case WO_foldprf:
                    fprintf (outfile, "noop\n");
                    break;
                default:
                    DBUG_ASSERT ((0), "wrong with-loop type found");
                }
            } else {
                fprintf (outfile, "?\n");
            }
        }
    }
    indent--;

    if (WLGRID_NEXT (arg_node) != NULL) {
        WLGRID_NEXT (arg_node) = Trav (WLGRID_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLsegVar(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLsegVar-nodes
 *
 ******************************************************************************/

node *
PrintWLsegVar (node *arg_node, node *arg_info)
{
    node *seg;
    int i = 0;

    DBUG_ENTER ("PrintWLsegVar");

    seg = arg_node;
    while (seg != NULL) {
        if (WLSEGVAR_SCHEDULING (seg) == NULL) {
            INDENT
            fprintf (outfile, "/********** (var.) segment %d: **********/\n", i++);
        } else {
            INDENT
            fprintf (outfile, "/********** (var.) segment %d: **********\n", i++);
            INDENT
            fprintf (outfile, " * scheduling: ");
            SCHPrintScheduling (outfile, WLSEGVAR_SCHEDULING (seg));
            fprintf (outfile, "\n");
            INDENT
            fprintf (outfile, " */\n");
        }

        WLSEGVAR_CONTENTS (seg) = Trav (WLSEGVAR_CONTENTS (seg), arg_info);
        seg = WLSEG_NEXT (seg);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   void PrintWLvar(node *arg_node, node *arg_info)
 *
 * description:
 *   prints a son of N_WLstriVar- and N_WLgridVar-nodes.
 *
 ******************************************************************************/

void
PrintWLvar (node *arg_node, int dim)
{
    DBUG_ENTER ("PrintWLvar");

    switch (NODE_TYPE (arg_node)) {
    case N_num:

        fprintf (outfile, "%d", NUM_VAL (arg_node));
        break;

    case N_id:

        fprintf (outfile, "%s[%d]", ID_NAME (arg_node), dim);
        break;

    default:

        DBUG_ASSERT ((0), "wrong node type found");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLstriVar(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLstriVar-nodes
 *
 * remark:
 *   N_WLstride, N_WLstriVar differs in output.
 *   The former prints '->', the latter '=>' !!!
 *
 ******************************************************************************/

node *
PrintWLstriVar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLstriVar");

    INDENT
    fprintf (outfile, "(");
    PrintWLvar (WLSTRIVAR_BOUND1 (arg_node), WLSTRIVAR_DIM (arg_node));
    fprintf (outfile, " => ");
    PrintWLvar (WLSTRIVAR_BOUND2 (arg_node), WLSTRIVAR_DIM (arg_node));
    fprintf (outfile, "), step%d[%d] ", WLSTRIVAR_LEVEL (arg_node),
             WLSTRIVAR_DIM (arg_node));
    PrintWLvar (WLSTRIVAR_STEP (arg_node), WLSTRIVAR_DIM (arg_node));
    fprintf (outfile, "\n");

    indent++;
    WLSTRIVAR_CONTENTS (arg_node) = Trav (WLSTRIVAR_CONTENTS (arg_node), arg_info);
    indent--;

    if (WLSTRIVAR_NEXT (arg_node) != NULL) {
        WLSTRIVAR_NEXT (arg_node) = Trav (WLSTRIVAR_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintWLgridVar( node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLgrid-nodes
 *
 * remark:
 *   N_WLgrid, N_WLgridVar differs in output.
 *   The former prints '->', the latter '=>' !!!
 *
 ******************************************************************************/

node *
PrintWLgridVar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLgridVar");

    INDENT
    fprintf (outfile, "(");
    PrintWLvar (WLGRIDVAR_BOUND1 (arg_node), WLGRIDVAR_DIM (arg_node));
    fprintf (outfile, " => ");
    PrintWLvar (WLGRIDVAR_BOUND2 (arg_node), WLGRIDVAR_DIM (arg_node));
    fprintf (outfile, "): ");

    indent++;
    if (WLGRIDVAR_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        WLGRIDVAR_NEXTDIM (arg_node) = Trav (WLGRIDVAR_NEXTDIM (arg_node), arg_info);
    } else {
        if (WLGRIDVAR_CODE (arg_node) != NULL) {
            fprintf (outfile, "op_%d\n", NCODE_NO (WLGRIDVAR_CODE (arg_node)));
        } else {
            if (INFO_PRINT_NWITH (arg_info) != NULL) {
                DBUG_ASSERT ((NODE_TYPE (INFO_PRINT_NWITH (arg_info)) == N_Nwith2),
                             "INFO_PRINT_NWITH( arg_info) contains no N_Nwith2 node");
                switch (NWITH2_TYPE (INFO_PRINT_NWITH (arg_info))) {
                case WO_genarray:
                    fprintf (outfile, "init\n");
                    break;
                case WO_modarray:
                    fprintf (outfile, "copy\n");
                    break;
                case WO_foldfun:
                    /* here is no break missing! */
                case WO_foldprf:
                    fprintf (outfile, "noop\n");
                    break;
                default:
                    DBUG_ASSERT ((0), "wrong with-loop type found");
                }
            } else {
                fprintf (outfile, "?\n");
            }
        }
    }
    indent--;

    if (WLGRIDVAR_NEXT (arg_node) != NULL) {
        WLGRIDVAR_NEXT (arg_node) = Trav (WLGRIDVAR_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *Print(node *syntax_tree)
 *
 * description:
 *   initiates print of (sub-)tree
 *
 ******************************************************************************/

node *
Print (node *syntax_tree)
{
    funtab *old_tab;
    node *arg_info;

    DBUG_ENTER ("Print");

    /* save act_tab to restore it afterwards. This could be useful if
       Print() is called from inside a debugger. */
    old_tab = act_tab;
    act_tab = print_tab;
    indent = 0;

    arg_info = MakeInfo ();

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
            syntax_tree = Trav (syntax_tree, arg_info);
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
            syntax_tree = Trav (syntax_tree, arg_info);
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
            syntax_tree = Trav (syntax_tree, arg_info);
            INFO_PRINT_SEPARATE (arg_info) = 0;
            break;
        }
    }

    else {
        outfile = stdout;
        fprintf (outfile, "\n-----------------------------------------------\n");
        syntax_tree = Trav (syntax_tree, arg_info);
        fprintf (outfile, "\n-----------------------------------------------\n");
    }

    FREE (arg_info);

    act_tab = old_tab;

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   void PrintNodeTreeSon( int num, node *node)
 *
 * description:
 *   This function is call from 'PrintNodeTree' only.
 *   Prints a son or attribute containing a hole sub-tree.
 *
 ******************************************************************************/

void
PrintNodeTreeSon (int num, node *node)
{
    int j;

    DBUG_ENTER ("PrintNodeTreeSon");

    for (j = 0; j < indent; j++) {
        if (j % 4) {
            fprintf (outfile, "  ");
        } else {
            fprintf (outfile, "| ");
        }
    }

    if (num >= 0) {
        fprintf (outfile, "%i-", num);
    } else {
        fprintf (outfile, "+-");
    }
    PrintNodeTree (node);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintNodeTreeIds( ids *vars)
 *
 * description:
 *   This function is call from 'PrintNodeTree' only.
 *   Prints a 'ids'-chain.
 *
 ******************************************************************************/

void
PrintNodeTreeIds (ids *vars)
{
    DBUG_ENTER ("PrintNodeTreeIds");

    fprintf (outfile, "( ");
    while (vars != NULL) {
        fprintf (outfile, "%s:%d::%d ", IDS_NAME (vars), IDS_REFCNT (vars),
                 IDS_NAIVE_REFCNT (vars));
        if ((!(optimize & OPT_RCO)) && show_refcnt && (IDS_REFCNT (vars) != -1)
            && (IDS_REFCNT (vars) != IDS_NAIVE_REFCNT (vars))) {
            fprintf (outfile, "**");
        }
        vars = IDS_NEXT (vars);
    }
    fprintf (outfile, ")");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void PrintNodeTree( node *node)
 *
 * description:
 *   this function is for debug assistance.
 *   It prints the syntax tree without any interpretation.
 *   Some attribues of interest are printed inside of parenthesizes behind
 *   the node name.
 *
 ******************************************************************************/

void
PrintNodeTree (node *node)
{
    int i, d;

    DBUG_ENTER ("PrintNodeTree");

    outfile = stdout;

    if (node) {
        /* print node name */
        fprintf (outfile, "%s  ", mdb_nodetype[NODE_TYPE (node)]);

        /* print additional information to nodes */
        switch (NODE_TYPE (node)) {
        case N_pragma:
            if (PRAGMA_WLCOMP_APS (node) != NULL) {
                fprintf (outfile, "(wlcomp)\n");
                indent++;
                PrintNodeTreeSon (0, PRAGMA_WLCOMP_APS (node));
                indent--;
            } else {
                fprintf (outfile, "\n");
            }
            break;
        case N_let:
            PrintNodeTreeIds (LET_IDS (node));
            fprintf (outfile, "\n");
            break;
        case N_id:
            fprintf (outfile, "(%s:%d::%d)\n", ID_NAME (node), ID_REFCNT (node),
                     ID_NAIVE_REFCNT (node));
            if ((!(optimize & OPT_RCO)) && show_refcnt && (ID_REFCNT (node) != -1)
                && (ID_REFCNT (node) != ID_NAIVE_REFCNT (node))) {
                fprintf (outfile, "**");
            }
            break;
        case N_num:
            fprintf (outfile, "(%i)\n", NUM_VAL (node));
            break;
        case N_prf:
            fprintf (outfile, "(%s)\n", mdb_prf[PRF_PRF (node)]);
            break;
        case N_ap:
            fprintf (outfile, "(%s)\n", AP_NAME (node));
            break;
        case N_arg:
            fprintf (outfile, "(%s %s:%d::%d)\n",
                     mdb_type[TYPES_BASETYPE (ARG_TYPE (node))],
                     (ARG_NAME (node) != NULL) ? ARG_NAME (node) : "?", ARG_REFCNT (node),
                     ARG_NAIVE_REFCNT (node));
            if ((!(optimize & OPT_RCO)) && show_refcnt && (ARG_REFCNT (node) != -1)
                && (ARG_REFCNT (node) != ARG_NAIVE_REFCNT (node))) {
                fprintf (outfile, "**");
            }
            break;
        case N_vardec:
            fprintf (outfile, "(%s %s:%d::%d)\n",
                     mdb_type[TYPES_BASETYPE (VARDEC_TYPE (node))], VARDEC_NAME (node),
                     VARDEC_REFCNT (node), VARDEC_NAIVE_REFCNT (node));
            if ((!(optimize & OPT_RCO)) && show_refcnt && (VARDEC_REFCNT (node) != -1)
                && (VARDEC_REFCNT (node) != VARDEC_NAIVE_REFCNT (node))) {
                fprintf (outfile, "**");
            }
            break;
        case N_fundef:
            fprintf (outfile, "(%s)\n", FUNDEF_NAME (node));
            break;
        case N_Nwith:
            fprintf (outfile, "\n");
            if (NWITH_PRAGMA (node) != NULL) {
                indent++;
                PrintNodeTreeSon (-1, NWITH_PRAGMA (node));
                indent--;
            }
            break;
        case N_Nwithid:
            fprintf (outfile, "( ");
            if (NWITHID_VEC (node) != NULL) {
                fprintf (outfile, "%s:%d::%d", IDS_NAME (NWITHID_VEC (node)),
                         IDS_REFCNT (NWITHID_VEC (node)),
                         IDS_NAIVE_REFCNT (NWITHID_VEC (node)));
                if ((!(optimize & OPT_RCO)) && show_refcnt
                    && (IDS_REFCNT (NWITHID_VEC (node)) != -1)
                    && (IDS_REFCNT (NWITHID_VEC (node))
                        != IDS_NAIVE_REFCNT (NWITHID_VEC (node)))) {
                    fprintf (outfile, "**");
                }
            } else {
                fprintf (outfile, "?");
            }
            fprintf (outfile, " = ");
            if (NWITHID_IDS (node) != NULL) {
                PrintNodeTreeIds (NWITHID_IDS (node));
            } else {
                fprintf (outfile, "?");
            }
            fprintf (outfile, " )\n");
            break;
        case N_Nwithop:
            fprintf (outfile, "( ");
            if (NWITHOP_FUNDEF (node) != NULL) {
                fprintf (outfile, "%s", FUNDEF_NAME (NWITHOP_FUNDEF (node)));
            }
            fprintf (outfile, " )\n");
            break;
        case N_Npart:
            if (NPART_CODE (node) != NULL) {
                fprintf (outfile, "(code used: 0x%p)\n", NPART_CODE (node));
            } else {
                fprintf (outfile, "(no code)\n");
            }
            break;
        case N_Ncode:
            fprintf (outfile, "(adr: 0x%p, used: %d)\n", node, NCODE_USED (node));
            break;
        case N_WLseg:
            fprintf (outfile, "(sv: [ ");
            for (d = 0; d < WLSEG_DIMS (node); d++) {
                fprintf (outfile, "%li ", (WLSEG_SV (node))[d]);
            }
            for (i = 0; i < WLSEG_BLOCKS (node); i++) {
                fprintf (outfile, "], bv%i: [ ", i);
                for (d = 0; d < WLSEG_DIMS (node); d++) {
                    fprintf (outfile, "%li ", (WLSEG_BV (node, i))[d]);
                }
            }
            fprintf (outfile, "], ubv: [ ");
            for (d = 0; d < WLSEG_DIMS (node); d++) {
                fprintf (outfile, "%li ", (WLSEG_UBV (node))[d]);
            }
            fprintf (outfile, "])\n");
            break;
        case N_WLsegVar:
            fprintf (outfile, "(sv: [ ");
            for (d = 0; d < WLSEGVAR_DIMS (node); d++) {
                fprintf (outfile, "%li ", (WLSEGVAR_SV (node))[d]);
            }
            for (i = 0; i < WLSEGVAR_BLOCKS (node); i++) {
                fprintf (outfile, "], bv%i: [ ", i);
                for (d = 0; d < WLSEGVAR_DIMS (node); d++) {
                    fprintf (outfile, "%li ", (WLSEGVAR_BV (node, i))[d]);
                }
            }
            fprintf (outfile, "], ubv: [ ");
            for (d = 0; d < WLSEGVAR_DIMS (node); d++) {
                fprintf (outfile, "%li ", (WLSEGVAR_UBV (node))[d]);
            }
            fprintf (outfile, "])\n");
            break;
        case N_WLblock:
            fprintf (outfile, "(%d->%d block%d[%d] %d)\n", WLBLOCK_BOUND1 (node),
                     WLBLOCK_BOUND2 (node), WLBLOCK_LEVEL (node), WLBLOCK_DIM (node),
                     WLBLOCK_STEP (node));
            break;
        case N_WLublock:
            fprintf (outfile, "(%d->%d ublock%d[%d] %d)\n", WLUBLOCK_BOUND1 (node),
                     WLUBLOCK_BOUND2 (node), WLUBLOCK_LEVEL (node), WLUBLOCK_DIM (node),
                     WLUBLOCK_STEP (node));
            break;
        case N_WLstride:
            fprintf (outfile, "(%d->%d step%d[%d] %d)\n", WLSTRIDE_BOUND1 (node),
                     WLSTRIDE_BOUND2 (node), WLSTRIDE_LEVEL (node), WLSTRIDE_DIM (node),
                     WLSTRIDE_STEP (node));
            break;
        case N_WLgrid:
            fprintf (outfile, "(%d->%d [%d])", WLUBLOCK_BOUND1 (node),
                     WLUBLOCK_BOUND2 (node), WLUBLOCK_DIM (node));
            if (WLGRID_CODE (node) != NULL) {
                fprintf (outfile, ": op\n");
            } else {
                fprintf (outfile, ": ..\n");
            }
            break;
        case N_WLstriVar:
            fprintf (outfile, "(");
            PrintWLvar (WLSTRIVAR_BOUND1 (node), WLSTRIVAR_DIM (node));
            fprintf (outfile, "->");
            PrintWLvar (WLSTRIVAR_BOUND2 (node), WLSTRIVAR_DIM (node));
            fprintf (outfile, ", step%d[%d] ", WLSTRIVAR_LEVEL (node),
                     WLSTRIVAR_DIM (node));
            PrintWLvar (WLSTRIVAR_STEP (node), WLSTRIVAR_DIM (node));
            fprintf (outfile, ")\n");
            break;
        case N_WLgridVar:
            fprintf (outfile, "(");
            PrintWLvar (WLGRIDVAR_BOUND1 (node), WLGRIDVAR_DIM (node));
            fprintf (outfile, "->");
            PrintWLvar (WLGRIDVAR_BOUND2 (node), WLGRIDVAR_DIM (node));
            fprintf (outfile, " [%d])\n", WLGRIDVAR_DIM (node));
            break;
        case N_icm:
            fprintf (outfile, "(%s)\n", ICM_NAME (node));
            break;
        default:
            fprintf (outfile, "\n");
        }

        indent++;
        for (i = 0; i < nnode[NODE_TYPE (node)]; i++)
            if (node->node[i]) {
                PrintNodeTreeSon (i, node->node[i]);
            }
        indent--;
    } else
        fprintf (outfile, "NULL\n");

    DBUG_VOID_RETURN;
}
