/*
 *
 * $Log$
 * Revision 2.73  2000/05/12 17:17:46  dkr
 * implementation of PrintNodeAST completed
 *
 * Revision 2.72  2000/04/27 13:01:59  dkr
 * system warning about unbalanced indentation deactivated
 *
 * Revision 2.71  2000/04/27 11:35:18  dkr
 * system warning about unbalanced indentation active in DBUG-mode only now
 *
 * Revision 2.70  2000/03/31 09:48:30  jhs
 * PrintMT and PrintST now handle REGIONS==NULL.
 *
 * Revision 2.69  2000/03/27 14:53:12  dkr
 * indentation of ICM-assigns via ICM_INDENT works correctly now
 *
 * Revision 2.68  2000/03/23 14:05:55  jhs
 * Added printing of all attributes of N_MTsync.
 *
 * Revision 2.67  2000/03/22 17:35:18  jhs
 * Added PrintMT(sync|alloc|signal).
 *
 * Revision 2.66  2000/03/21 17:55:55  dkr
 * PRINT_CONT modified: arg_info might be NULL
 *
 * Revision 2.65  2000/03/21 17:21:02  dkr
 * I really hope that the indentation is now *always* correct ... %-]
 *
 * Revision 2.64  2000/03/21 16:04:14  dkr
 * indentation of ICMs corrected
 *
 * Revision 2.63  2000/03/21 15:52:04  jhs
 * Prints some masks for DFA now.
 *
 * Revision 2.62  2000/03/21 13:41:22  dkr
 * macro PRINT_VECT used
 *
 * Revision 2.61  2000/03/20 19:28:48  dkr
 * added a second SYSWARN for unbalanced indentation in PrintFundef
 *
 * Revision 2.60  2000/03/20 18:43:32  dkr
 * SYSWARN added for unbalanced indentation
 *
 * Revision 2.59  2000/03/20 10:33:17  dkr
 * indentation of ICMs reactivated
 * WHY THE HELL IS THE OUTPUT OF THE PRINT MODUL CORRUPTED AFTER EACH
 * UPDATE??
 *
 * Revision 2.58  2000/03/16 16:19:06  dkr
 * some output layout errors fixed
 *
 * Revision 2.57  2000/03/15 14:56:56  dkr
 * PrintNodeTree renamed to PrintAST
 * PrintNodeAST added (but not yet implemented)
 *
 * Revision 2.56  2000/03/09 18:31:53  jhs
 * Added print of [LET|RETURN][USE|DEF]MASK.
 *
 * Revision 2.55  2000/03/02 15:56:00  jhs
 * Added printing of FUNDEF_ATTRIB at N_fundef.
 *
 * Revision 2.54  2000/03/02 13:08:48  jhs
 * Added some \n.
 *
 * Revision 2.53  2000/02/24 17:59:35  dkr
 * Fixed a bug: PrintIds() has an arg_info now
 *
 * Revision 2.52  2000/02/24 16:52:08  dkr
 * some brushing done
 * functions for old with-loop removed
 *
 * Revision 2.51  2000/02/23 23:05:29  dkr
 * spacing in PrintAp, PrintArgs, PrintFundef, ... slightly changed
 *
 * Revision 2.49  2000/02/23 20:16:34  cg
 * Node status ST_imported replaced by ST_imported_mod and
 * ST_imported_class in order to allow distinction between enteties
 * that are imported from a module and those that are imported from a
 * class.
 *
 * Revision 2.48  2000/02/23 14:06:32  dkr
 * superfluous indentation of loop- and cond-bodies removed
 *
 * Revision 2.47  2000/02/21 12:49:48  jhs
 * Removed superfluous variable nametab and replaced it's only usage with
 * mdb_nodetype, already used at other places within this file
 *
 * [...]
 *
 * Revision 2.1  1999/02/23 12:40:24  sacbase
 * new release made
 *
 * Revision 1.261  1999/02/15 15:11:50  cg
 * Defines with function name before each function definition
 * moved into ND_FUNDEC ICM.
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

/*
 * PrintNode(): INFO_PRINT_CONT(arg_info) contains the root of syntaxtree.
 *  -> traverses next-node if and only if its parent is not the root.
 * Print(): INFO_PRINT_CONT(arg_info) is NULL.
 *  -> traverses all next-nodes.
 *
 * This behaviour is implemented with the macro PRINT_CONT.
 */

#if 0 /* Trav() is used in icm2c_wl.c -> (arg_info == NULL) !! */
#define PRINT_CONT(code_then, code_else)                                                 \
    DBUG_ASSERT ((arg_info != NULL), "Print(): arg_info is NULL!");                      \
    if (INFO_PRINT_CONT (arg_info) != arg_node) {                                        \
        code_then;                                                                       \
    } else {                                                                             \
        code_else;                                                                       \
    }
#else
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
#endif

#define ACLT(arg)                                                                        \
    (arg == ACL_unknown)                                                                 \
      ? ("ACL_unknown")                                                                  \
      : ((arg == ACL_irregular)                                                          \
           ? ("ACL_irregular")                                                           \
           : ((arg == ACL_offset) ? ("ACL_offset:")                                      \
                                  : ((arg == ACL_const) ? ("ACL_const :") : (""))))

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
            /* here's no break missing ! */
            case ACL_unknown:
#if 0
        fprintf(outfile,"ACCESS_IV: %s", mdb_nodetype[NODE_TYPE(ACCESS_IV(access))]);
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
PrintIds (ids *arg, node *arg_info)
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
            Trav (IDS_USE (arg), arg_info);
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
        if (ICM_INDENT (ASSIGN_INSTR (arg_node)) < 0) {
            indent += ICM_INDENT (ASSIGN_INSTR (arg_node));
        }
        INDENT;
        PrintIcm (ASSIGN_INSTR (arg_node), arg_info);
        fprintf (outfile, "\n");
        if (ICM_INDENT (ASSIGN_INSTR (arg_node)) > 0) {
            indent += ICM_INDENT (ASSIGN_INSTR (arg_node));
        }
        if (ASSIGN_NEXT (arg_node) != NULL) {
            PRINT_CONT (Trav (ASSIGN_NEXT (arg_node), arg_info), );
        }
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

        if (ASSIGN_NEXT (arg_node) != NULL) {
            PRINT_CONT (Trav (ASSIGN_NEXT (arg_node), arg_info), );
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
#if 0
    SYSWARN( ("Indentation unbalanced while printing block of function %s."
              " Indentation at beginning of block: %i."
              " Indentation at end of block: %i",
              FUNDEF_NAME( INFO_PRINT_FUNDEF( arg_info)), old_indent, indent));
#endif
        indent = old_indent;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

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

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (TYPEDEF_NEXT (arg_node), arg_info), );
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
        if ((OBJDEF_STATUS (arg_node) == ST_imported_mod)
            || (OBJDEF_STATUS (arg_node) == ST_imported_class)
            || print_objdef_for_header_file) {
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

        fprintf (outfile, "\n");
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (OBJDEF_NEXT (arg_node), arg_info), );
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
#if 0
    SYSWARN( ("Indentation unbalanced while printing function %s."
              " Indentation at beginning of function: %i."
              " Indentation at end of function: %i",
              FUNDEF_NAME( arg_node), old_indent, indent));
#endif
        indent = old_indent;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) { /* traverse next function */
        PRINT_CONT (Trav (FUNDEF_NEXT (arg_node), arg_info), );
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
    case F_max:
        /* primitive functions that are printed as function application */
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
    } else {
        fprintf (outfile, "true");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

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

/******************************************************************************/

node *
PrintAp (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAp");

    if (AP_MOD (arg_node) != NULL) {
        fprintf (outfile, "%s:", AP_MOD (arg_node));
    }
    fprintf (outfile, "%s(", AP_NAME (arg_node));

    if (INFO_PRINT_PRAGMA_WLCOMP (arg_info)) {
        /*
         * Here, we are printing a wlcomp pragma.
         */
        DBUG_ASSERT ((AP_ARGS (arg_node) != NULL),
                     "No parameter of wlcomp-pragma found!");
        fprintf (outfile, " ");
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
        if (AP_ARGS (arg_node) != NULL) {
            fprintf (outfile, " ");
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

    EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);

    if (EXPRS_NEXT (arg_node) != NULL) {
        fprintf (outfile, ", ");
        PRINT_CONT (Trav (EXPRS_NEXT (arg_node), arg_info), );
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintArg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintArg");

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**%d: ", ARG_VARNO (arg_node)););

    fprintf (outfile, " %s",
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
        fprintf (outfile, ",");
        PRINT_CONT (Trav (ARG_NEXT (arg_node), arg_info), );
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
        PRINT_CONT (Trav (VARDEC_NEXT (arg_node), arg_info), );
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

    fprintf (outfile, "do \n");

    DBUG_EXECUTE ("VARS", fprintf (outfile, "**(NAIVE)DEFVARS\n");
                  PrintIds (DO_DEFVARS (arg_node), arg_info); fprintf (outfile, "\n");
                  PrintIds (DO_NAIVE_DEFVARS (arg_node), arg_info);
                  fprintf (outfile, "\n**(NAIVE)USEVARS\n");
                  PrintIds (DO_USEVARS (arg_node), arg_info); fprintf (outfile, "\n");
                  PrintIds (DO_NAIVE_USEVARS (arg_node), arg_info);
                  fprintf (outfile, "\n"););

    if (DO_BODY (arg_node) != NULL) {

        DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**MASKS - do body: \n");
                      PrintDefUseMasks (outfile, DO_DEFMASK (arg_node),
                                        DO_USEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info));
                      PrintMrdMask (outfile, DO_MRDMASK (arg_node),
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

    fprintf (outfile, "while (");
    Trav (WHILE_COND (arg_node), arg_info);
    fprintf (outfile, ") \n");

    DBUG_EXECUTE ("VARS", fprintf (outfile, "**(NAIVE)DEFVARS\n");
                  PrintIds (WHILE_DEFVARS (arg_node), arg_info); fprintf (outfile, "\n");
                  PrintIds (WHILE_NAIVE_DEFVARS (arg_node), arg_info);
                  fprintf (outfile, "\n**(NAIVE)USEVARS\n");
                  PrintIds (WHILE_USEVARS (arg_node), arg_info); fprintf (outfile, "\n");
                  PrintIds (WHILE_NAIVE_USEVARS (arg_node), arg_info);
                  fprintf (outfile, "\n"););

    if (WHILE_BODY (arg_node) != NULL) {

        DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**MASKS - while body: \n");
                      PrintDefUseMasks (outfile, WHILE_DEFMASK (arg_node),
                                        WHILE_USEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info));
                      PrintMrdMask (outfile, WHILE_MRDMASK (arg_node),
                                    INFO_PRINT_VARNO (arg_info)););

        Trav (WHILE_BODY (arg_node), arg_info); /* traverse body of loop */
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
    fprintf (outfile, ") \n");

    if (COND_THEN (arg_node) != NULL) {

        DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**MASKS - then: \n");
                      PrintDefUseMasks (outfile, COND_THENDEFMASK (arg_node),
                                        COND_THENUSEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info)););

        DBUG_EXECUTE ("VARS", fprintf (outfile, "\n**(NAIVE)VARS - then\n");
                      PrintIds (COND_THENVARS (arg_node), arg_info);
                      fprintf (outfile, "\n");
                      PrintIds (COND_NAIVE_THENVARS (arg_node), arg_info);
                      fprintf (outfile, "\n"););

        Trav (COND_THEN (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    INDENT;
    fprintf (outfile, "else\n");

    if (COND_ELSE (arg_node) != NULL) {

        DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**MASKS - else: \n");
                      PrintDefUseMasks (outfile, COND_ELSEDEFMASK (arg_node),
                                        COND_ELSEUSEMASK (arg_node),
                                        INFO_PRINT_VARNO (arg_info)););

        DBUG_EXECUTE ("VARS", fprintf (outfile, "\n**(NAIVE)VARS - else\n");
                      PrintIds (COND_ELSEVARS (arg_node), arg_info);
                      fprintf (outfile, "\n");
                      PrintIds (COND_NAIVE_ELSEVARS (arg_node), arg_info);
                      fprintf (outfile, "\n"););

        Trav (COND_ELSE (arg_node), arg_info);
    }

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

    PrintIds (arg_node->info.ids, arg_info);
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
    PrintIds (arg_node->info.ids, arg_info);
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

        if (VINFO_NEXT (arg_node) != NULL) {
            PRINT_CONT (Trav (VINFO_NEXT (arg_node), arg_info), );
        }
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
        }

        fprintf (outfile, "SAC_%s( ", ICM_NAME (arg_node));
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

            PRINT_CONT (Trav (ICM_NEXT (arg_node), arg_info), );
        }
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
        INFO_PRINT_PRAGMA_WLCOMP (arg_info) = 1;
        Trav (PRAGMA_WLCOMP_APS (arg_node), arg_info);
        INFO_PRINT_PRAGMA_WLCOMP (arg_info) = 0;
        fprintf (outfile, "\n");
    }

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

/******************************************************************************/

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

    SYNC_REGION (arg_node) = Trav (SYNC_REGION (arg_node), arg_info);

    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, "/*** end of sync region ***/\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintMT (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintMT");

    /* PrintAssign already indents */
    fprintf (outfile, "/*** begin of mt region ***/\n");

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
        MT_REGION (arg_node) = Trav (MT_REGION (arg_node), arg_info);
    } else {
        INDENT;
        fprintf (outfile, "/* ... Empty ... */");
    }
    indent--;

    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, "/*** end of mt region ***/\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintST (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintST");

    /* PrintAssign already indents */
    fprintf (outfile, "/*** begin of st region ***/\n");

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
        ST_REGION (arg_node) = Trav (ST_REGION (arg_node), arg_info);
    } else {
        INDENT;
        fprintf (outfile, "/* ... Empty ... */");
    }
    indent--;

    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, "/*** end of st region ***/\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintMTsignal (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintMTsignal");

    /* PrintAssign already indents and prints \n*/
    fprintf (outfile, "MTsignal (");

    DFMPrintMask (outfile, " %s", MTSIGNAL_IDSET (arg_node));

    fprintf (outfile, ");");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

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

            fprintf (outfile, " %s:%s", DFMFM_NAME (foldmask),
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

/******************************************************************************/

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
        PRINT_CONT (Trav (NPART_NEXT (arg_node), arg_info), );
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
        INDENT;
        fprintf (outfile, "/* scheduling :");
        SCHPrintScheduling (outfile, NWITH2_SCHEDULING (arg_node));
        fprintf (outfile, " */\n");
    }

    INDENT;
    fprintf (outfile, "/********** operators: **********/\n");
    code = NWITH2_CODE (arg_node);
    while (code != NULL) {
        INDENT;
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

    INDENT;
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
            INDENT;
            fprintf (outfile, "/********** segment %d: **********/\n", i++);
        } else {
            INDENT;
            fprintf (outfile, "/********** segment %d: **********\n", i++);
            INDENT;
            fprintf (outfile, " * scheduling: ");
            SCHPrintScheduling (outfile, WLSEG_SCHEDULING (seg));
            fprintf (outfile, "\n");
            INDENT;
            fprintf (outfile, " */\n");
        }

        WLSEG_CONTENTS (seg) = Trav (WLSEG_CONTENTS (seg), arg_info);
        PRINT_CONT (seg = WLSEG_NEXT (seg), seg = NULL)
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

    INDENT;
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
        PRINT_CONT (Trav (WLBLOCK_NEXT (arg_node), arg_info), );
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

    INDENT;
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
        PRINT_CONT (Trav (WLUBLOCK_NEXT (arg_node), arg_info), );
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

    INDENT;
    fprintf (outfile, "(%d -> %d), step%d[%d] %d\n", WLSTRIDE_BOUND1 (arg_node),
             WLSTRIDE_BOUND2 (arg_node), WLSTRIDE_LEVEL (arg_node),
             WLSTRIDE_DIM (arg_node), WLSTRIDE_STEP (arg_node));

    indent++;
    WLSTRIDE_CONTENTS (arg_node) = Trav (WLSTRIDE_CONTENTS (arg_node), arg_info);
    indent--;

    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        PRINT_CONT (Trav (WLSTRIDE_NEXT (arg_node), arg_info), );
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

    INDENT;
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
        PRINT_CONT (Trav (WLGRID_NEXT (arg_node), arg_info), );
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
            INDENT;
            fprintf (outfile, "/********** (var.) segment %d: **********/\n", i++);
        } else {
            INDENT;
            fprintf (outfile, "/********** (var.) segment %d: **********\n", i++);
            INDENT;
            fprintf (outfile, " * scheduling: ");
            SCHPrintScheduling (outfile, WLSEGVAR_SCHEDULING (seg));
            fprintf (outfile, "\n");
            INDENT;
            fprintf (outfile, " */\n");
        }

        WLSEGVAR_CONTENTS (seg) = Trav (WLSEGVAR_CONTENTS (seg), arg_info);
        PRINT_CONT (seg = WLSEGVAR_NEXT (seg), seg = NULL);
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

    INDENT;
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
        PRINT_CONT (Trav (WLSTRIVAR_NEXT (arg_node), arg_info), );
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

    INDENT;
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
        PRINT_CONT (Trav (WLGRIDVAR_NEXT (arg_node), arg_info), );
    }

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

    act_tab = old_tab;

    DBUG_RETURN (syntax_tree);
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

/* forward declaration */
static void DoPrintAST (node *arg_node, int skip_next);

/******************************************************************************
 *
 * function:
 *   void DoPrintSonAST( int num, node *arg_node, int skip_node)
 *
 * description:
 *   This function is called from 'DoPrintAST' only.
 *   Prints a son or attribute containing a hole sub-tree.
 *
 ******************************************************************************/

static void
DoPrintSonAST (int num, node *arg_node, int skip_node)
{
    int j;

    DBUG_ENTER ("DoPrintSonAST");

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

    if (skip_node) {
        fprintf (outfile, "(NEXT)\n");
    } else {
        DoPrintAST (arg_node, 0); /* inner node -> do not skip NEXT-nodes */
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void DoPrintIdsAST( ids *vars)
 *
 * description:
 *   This function is called from 'DoPrintAST' only.
 *   Prints a 'ids'-chain.
 *
 ******************************************************************************/

static void
DoPrintIdsAST (ids *vars)
{
    DBUG_ENTER ("DoPrintIdsAST");

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
 *   void DoPrintAST( node *arg_node, int skip_next)
 *
 * description:
 *   This function prints the syntax tree without any interpretation.
 *   Some attribues of interest are printed inside of parenthesizes behind
 *   the node name.
 *
 ******************************************************************************/

static void
DoPrintAST (node *arg_node, int skip_next)
{
    node *skip;
    int i;

    DBUG_ENTER ("DoPrintAST");

    outfile = stdout;

    if (arg_node) {
        /* print node name */
        fprintf (outfile, "%s  ", mdb_nodetype[NODE_TYPE (arg_node)]);

        /* print additional information to nodes */
        switch (NODE_TYPE (arg_node)) {
        case N_typedef:
            fprintf (outfile, "\n");
            skip = TYPEDEF_NEXT (arg_node);
            break;

        case N_objdef:
            fprintf (outfile, "\n");
            skip = OBJDEF_NEXT (arg_node);
            break;

        case N_assign:
            fprintf (outfile, "\n");
            skip = ASSIGN_NEXT (arg_node);
            break;

        case N_exprs:
            fprintf (outfile, "\n");
            skip = EXPRS_NEXT (arg_node);
            break;

        case N_vinfo:
            fprintf (outfile, "\n");
            skip = VINFO_NEXT (arg_node);
            break;

        case N_pragma:
            if (PRAGMA_WLCOMP_APS (arg_node) != NULL) {
                fprintf (outfile, "(wlcomp)\n");
                indent++;
                DoPrintSonAST (0, PRAGMA_WLCOMP_APS (arg_node), 0);
                indent--;
            } else {
                fprintf (outfile, "\n");
            }
            break;

        case N_let:
            DoPrintIdsAST (LET_IDS (arg_node));
            fprintf (outfile, "\n");
            break;

        case N_id:
            fprintf (outfile, "(%s:%d::%d)\n", ID_NAME (arg_node), ID_REFCNT (arg_node),
                     ID_NAIVE_REFCNT (arg_node));
            if ((!(optimize & OPT_RCO)) && show_refcnt && (ID_REFCNT (arg_node) != -1)
                && (ID_REFCNT (arg_node) != ID_NAIVE_REFCNT (arg_node))) {
                fprintf (outfile, "**");
            }
            break;

        case N_num:
            fprintf (outfile, "(%i)\n", NUM_VAL (arg_node));
            break;

        case N_prf:
            fprintf (outfile, "(%s)\n", mdb_prf[PRF_PRF (arg_node)]);
            break;

        case N_ap:
            fprintf (outfile, "(%s)\n", AP_NAME (arg_node));
            break;

        case N_arg:
            fprintf (outfile, "(%s %s:%d::%d)\n",
                     mdb_type[TYPES_BASETYPE (ARG_TYPE (arg_node))],
                     (ARG_NAME (arg_node) != NULL) ? ARG_NAME (arg_node) : "?",
                     ARG_REFCNT (arg_node), ARG_NAIVE_REFCNT (arg_node));
            if ((!(optimize & OPT_RCO)) && show_refcnt && (ARG_REFCNT (arg_node) != -1)
                && (ARG_REFCNT (arg_node) != ARG_NAIVE_REFCNT (arg_node))) {
                fprintf (outfile, "**");
            }

            skip = ARG_NEXT (arg_node);
            break;

        case N_vardec:
            fprintf (outfile, "(%s %s:%d::%d)\n",
                     mdb_type[TYPES_BASETYPE (VARDEC_TYPE (arg_node))],
                     VARDEC_NAME (arg_node), VARDEC_REFCNT (arg_node),
                     VARDEC_NAIVE_REFCNT (arg_node));
            if ((!(optimize & OPT_RCO)) && show_refcnt && (VARDEC_REFCNT (arg_node) != -1)
                && (VARDEC_REFCNT (arg_node) != VARDEC_NAIVE_REFCNT (arg_node))) {
                fprintf (outfile, "**");
            }

            skip = VARDEC_NEXT (arg_node);
            break;

        case N_fundef:
            fprintf (outfile, "(%s)\n", FUNDEF_NAME (arg_node));

            skip = FUNDEF_NEXT (arg_node);
            break;

        case N_Nwith:
            fprintf (outfile, "\n");
            if (NWITH_PRAGMA (arg_node) != NULL) {
                indent++;
                DoPrintSonAST (-1, NWITH_PRAGMA (arg_node), 0);
                indent--;
            }
            break;

        case N_Nwithid:
            fprintf (outfile, "( ");
            if (NWITHID_VEC (arg_node) != NULL) {
                fprintf (outfile, "%s:%d::%d", IDS_NAME (NWITHID_VEC (arg_node)),
                         IDS_REFCNT (NWITHID_VEC (arg_node)),
                         IDS_NAIVE_REFCNT (NWITHID_VEC (arg_node)));
                if ((!(optimize & OPT_RCO)) && show_refcnt
                    && (IDS_REFCNT (NWITHID_VEC (arg_node)) != -1)
                    && (IDS_REFCNT (NWITHID_VEC (arg_node))
                        != IDS_NAIVE_REFCNT (NWITHID_VEC (arg_node)))) {
                    fprintf (outfile, "**");
                }
            } else {
                fprintf (outfile, "?");
            }
            fprintf (outfile, " = ");
            if (NWITHID_IDS (arg_node) != NULL) {
                DoPrintIdsAST (NWITHID_IDS (arg_node));
            } else {
                fprintf (outfile, "?");
            }
            fprintf (outfile, " )\n");
            break;

        case N_Nwithop:
            fprintf (outfile, "( ");
            if (NWITHOP_FUNDEF (arg_node) != NULL) {
                fprintf (outfile, "%s", FUNDEF_NAME (NWITHOP_FUNDEF (arg_node)));
            }
            fprintf (outfile, " )\n");
            break;

        case N_Npart:
            if (NPART_CODE (arg_node) != NULL) {
                fprintf (outfile, "(code used: 0x%p)\n", NPART_CODE (arg_node));
            } else {
                fprintf (outfile, "(no code)\n");
            }

            skip = NPART_NEXT (arg_node);
            break;

        case N_Ncode:
            fprintf (outfile, "(adr: 0x%p, used: %d)\n", arg_node, NCODE_USED (arg_node));

            skip = NCODE_NEXT (arg_node);
            break;

        case N_WLseg:
            fprintf (outfile, "(sv: ");
            PRINT_VECT (outfile, WLSEG_SV (arg_node), WLSEG_DIMS (arg_node), "%i");

            for (i = 0; i < WLSEG_BLOCKS (arg_node); i++) {
                fprintf (outfile, ", bv%i: ", i);
                PRINT_VECT (outfile, WLSEG_BV (arg_node, i), WLSEG_DIMS (arg_node), "%i");
            }

            fprintf (outfile, ", ubv: ");
            PRINT_VECT (outfile, WLSEG_UBV (arg_node), WLSEG_DIMS (arg_node), "%i");

            fprintf (outfile, ", homsv: ");
            PRINT_VECT (outfile, WLSEG_HOMSV (arg_node), WLSEG_DIMS (arg_node), "%i");
            fprintf (outfile, ", maxhomdim: %i)\n", WLSEG_MAXHOMDIM (arg_node));
            PRINT_VECT (outfile, WLSEG_HOMSV (arg_node), WLSEG_DIMS (arg_node), "%i");

            skip = WLSEG_NEXT (arg_node);
            break;

        case N_WLsegVar:
            fprintf (outfile, "(sv: ");
            PRINT_VECT (outfile, WLSEGVAR_SV (arg_node), WLSEGVAR_DIMS (arg_node), "%i");

            for (i = 0; i < WLSEGVAR_BLOCKS (arg_node); i++) {
                fprintf (outfile, ", bv%i: ", i);
                PRINT_VECT (outfile, WLSEGVAR_BV (arg_node, i), WLSEGVAR_DIMS (arg_node),
                            "%i");
            }

            fprintf (outfile, ", ubv: ");
            PRINT_VECT (outfile, WLSEGVAR_UBV (arg_node), WLSEGVAR_DIMS (arg_node), "%i");

            fprintf (outfile, ")\n");

            skip = WLSEGVAR_NEXT (arg_node);
            break;

        case N_WLblock:
            fprintf (outfile, "(%d->%d block%d[%d] %d)\n", WLBLOCK_BOUND1 (arg_node),
                     WLBLOCK_BOUND2 (arg_node), WLBLOCK_LEVEL (arg_node),
                     WLBLOCK_DIM (arg_node), WLBLOCK_STEP (arg_node));

            skip = WLBLOCK_NEXT (arg_node);
            break;

        case N_WLublock:
            fprintf (outfile, "(%d->%d ublock%d[%d] %d)\n", WLUBLOCK_BOUND1 (arg_node),
                     WLUBLOCK_BOUND2 (arg_node), WLUBLOCK_LEVEL (arg_node),
                     WLUBLOCK_DIM (arg_node), WLUBLOCK_STEP (arg_node));

            skip = WLUBLOCK_NEXT (arg_node);
            break;

        case N_WLstride:
            fprintf (outfile, "(%d->%d step%d[%d] %d)\n", WLSTRIDE_BOUND1 (arg_node),
                     WLSTRIDE_BOUND2 (arg_node), WLSTRIDE_LEVEL (arg_node),
                     WLSTRIDE_DIM (arg_node), WLSTRIDE_STEP (arg_node));

            skip = WLSTRIDE_NEXT (arg_node);
            break;

        case N_WLgrid:
            fprintf (outfile, "(%d->%d [%d])", WLUBLOCK_BOUND1 (arg_node),
                     WLUBLOCK_BOUND2 (arg_node), WLUBLOCK_DIM (arg_node));
            if (WLGRID_CODE (arg_node) != NULL) {
                fprintf (outfile, ": op\n");
            } else {
                fprintf (outfile, ": ..\n");
            }

            skip = WLGRID_NEXT (arg_node);
            break;

        case N_WLstriVar:
            fprintf (outfile, "(");
            PrintWLvar (WLSTRIVAR_BOUND1 (arg_node), WLSTRIVAR_DIM (arg_node));
            fprintf (outfile, "->");
            PrintWLvar (WLSTRIVAR_BOUND2 (arg_node), WLSTRIVAR_DIM (arg_node));
            fprintf (outfile, ", step%d[%d] ", WLSTRIVAR_LEVEL (arg_node),
                     WLSTRIVAR_DIM (arg_node));
            PrintWLvar (WLSTRIVAR_STEP (arg_node), WLSTRIVAR_DIM (arg_node));
            fprintf (outfile, ")\n");

            skip = WLSTRIVAR_NEXT (arg_node);
            break;

        case N_WLgridVar:
            fprintf (outfile, "(");
            PrintWLvar (WLGRIDVAR_BOUND1 (arg_node), WLGRIDVAR_DIM (arg_node));
            fprintf (outfile, "->");
            PrintWLvar (WLGRIDVAR_BOUND2 (arg_node), WLGRIDVAR_DIM (arg_node));
            fprintf (outfile, " [%d])\n", WLGRIDVAR_DIM (arg_node));

            skip = WLGRIDVAR_NEXT (arg_node);
            break;

        case N_icm:
            fprintf (outfile, "(%s)\n", ICM_NAME (arg_node));

            skip = ICM_NEXT (arg_node);
            break;

        default:
            fprintf (outfile, "\n");
        }

        indent++;
        for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++) {
            DoPrintSonAST (i, arg_node->node[i],
                           ((arg_node->node[i] != NULL) && (skip_next == 1)
                            && (arg_node->node[i] == skip)));
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

    DoPrintAST (arg_node, 0);

    DBUG_VOID_RETURN;
}

void
PrintNodeAST (node *arg_node)
{
    DBUG_ENTER ("PrintNodeAST");

    DoPrintAST (arg_node, 1);

    DBUG_VOID_RETURN;
}
