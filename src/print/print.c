/*
 *
 * $Log$
 * Revision 1.162  1998/03/20 17:10:42  dkr
 * changed output in PrintWLblock:
 *   show LEVEL
 *
 * Revision 1.161  1998/03/19 20:31:20  dkr
 * changed output in PrintNcode:
 *   prints "{}" if CBLOCK is NULL
 *
 * Revision 1.160  1998/03/19 19:06:27  dkr
 * changed output in PrintWL...
 *
 * Revision 1.159  1998/03/17 20:52:22  dkr
 * removed bugs in PrintWL...()
 *
 * Revision 1.158  1998/03/17 16:28:06  dkr
 * removed bug in PrintWLseg()
 *
 * Revision 1.157  1998/03/17 14:21:58  cg
 * file src/compile/trace.h removed.
 * definition of symbolic values of global variable traceflag moved to globals.h
 *
 * Revision 1.156  1998/03/17 12:19:01  cg
 * PrintId() no longer prints strings. This is now exclusively done by
 * PrintStr().
 *
 * Revision 1.155  1998/03/16 19:28:31  dkr
 * fixed a bug in PrintNwith2
 *
 * Revision 1.154  1998/03/16 18:45:27  dkr
 * changed output of ops in PrintNwith2()
 *
 * Revision 1.153  1998/03/16 15:08:20  dkr
 * changed the output in PrintCond
 *
 * Revision 1.152  1998/03/15 13:07:48  srs
 * added DBUG macros to ass MASK information to N_Ncode, N_Npart and
 * N_Nwithop
 *
 * Revision 1.151  1998/03/13 16:21:55  dkr
 * new nodes added:
 *   N_WLblock, N_WLublock
 *
 * Revision 1.150  1998/03/03 23:53:19  dkr
 * added PrintNwithid, PrintNwithop, PrintNcode, ...
 * added print-routines for N_Nwith2-nodes
 *
 * Revision 1.149  1998/03/03 17:30:09  cg
 * The C file resulting from a module/class implementation compiled with
 * link style 1 (one large object code file) is now printed to the temporary
 * directory for further processing instead of being printed to the target
 * directory as it happens for C files resulting from SAC programs.
 *
 * Revision 1.148  1998/02/26 14:10:45  srs
 * changed output of N_empty node (was "\t;\n").
 *
 * Revision 1.147  1998/02/25 09:14:43  cg
 * switched to new compiler_phase representation using the enumeration type
 * compiler_phase_t
 *
 * Revision 1.146  1998/02/13 12:50:08  srs
 * changed PrintNgenerator. = now between vector and scalars
 *
 * Revision 1.145  1998/02/11 17:15:06  srs
 * changed NPART_IDX to NPART_WITHID
 * removed NEW_TREE
 *
 * Revision 1.144  1998/01/30 17:49:15  srs
 * modified printing of new WL generator
 *
 * Revision 1.143  1998/01/22 12:04:31  srs
 * *** empty log message ***
 *
 * Revision 1.142  1997/12/20 15:49:40  srs
 * enhanced PrintNodeTree and
 * changed some functions headers to new style
 *
 * Revision 1.141  1997/12/10 18:36:27  srs
 * changed output of new WLs
 *
 * Revision 1.140  1997/12/02 18:48:06  srs
 * enhanced output of PrintNodeTree
 *
 * Revision 1.139  1997/11/29 15:49:53  srs
 * the print routine for the new WLs prints the 'user-syntax'
 * if only one Npart node is present. Else an internal syntax
 * (with many Npart nodes) is used.
 *
 * Revision 1.138  1997/11/25 11:05:04  srs
 * modifies output for new WLs
 *
 * Revision 1.137  1997/11/24 16:44:33  sbs
 * changed nodename->mdb_nodetype and
 * adjusted WITHOP_FUN-macro
 *
 * Revision 1.136  1997/11/24 16:04:41  srs
 * print routines for N_Nwith node and subnodes
 *
 * Revision 1.135  1997/11/20 15:32:44  srs
 * print routines for new WLs
 *
 * Revision 1.134  1997/11/07 13:32:24  srs
 * removed unused function PrintLeton
 *
 * Revision 1.133  1997/11/02 13:59:14  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.132  1997/10/29 14:31:59  srs
 * free -> FREE
 *
 * Revision 1.131  1997/10/07 13:30:40  srs
 * PrintPrf: F_min, F_max adjusted
 *
 * Revision 1.130  1997/10/03 17:47:46  dkr
 * added support for F_abs
 *
 * Revision 1.129  1997/08/29 12:03:59  sbs
 * missing }
 *
 * Revision 1.128  1997/08/29 10:12:27  sbs
 * output of #define TRACE_PRF included.
 * It is needed for printing prf's that are implemented via macros only,
 * e.g. IDX-PSI.
 *
 * Revision 1.127  1997/08/04 17:09:45  dkr
 * extended conditionals before printing PROFILE_SETUP
 *
 * Revision 1.126  1997/05/29  13:42:20  sbs
 * F_idx_modarray added
 *
 * Revision 1.125  1997/05/28  12:37:20  sbs
 * Profiling integrated
 *
 * Revision 1.124  1997/05/16  09:54:29  sbs
 * ANALSE-TOOL extended to function-application specific timing
 *
 * Revision 1.123  1997/05/14  08:14:41  sbs
 * ANALYSE-Macros inserted in PrintReturn and PrintIcm;
 * PrintAnnotate added
 *
 * Revision 1.122  1997/05/05  11:53:18  cg
 * Now a dummy symbol is generated in the globals.c file. This prevents the linker
 * from warning in the case of non-existing symbol tables due to "empty" object files.
 *
 * Revision 1.121  1997/03/19  13:42:42  cg
 * converted to single tmp directory tmp_dirname instaed of build_dirname
 * and store_dirname
 *
 * Revision 1.120  1996/09/11  06:16:49  cg
 * Different link styles for modules added.
 * Now, the default is to generate one file for each function and to create
 * an archive using ar afterwards.
 * This includes a new format for lib-files.
 *
 * Revision 1.119  1996/08/09  10:10:11  sbs
 * removed 'fprintf(outfile,"%s",arg_node->info.id);'
 * from PrintPost. This led to errorneous charackters
 * in id++ statements between the idtentifier id and ++
 * since the id is stored in info.ids and NOT in info.id.
 *
 * Revision 1.118  1996/04/02  19:35:46  cg
 * non-printable characters are printed correctly using octal numbers
 *
 * Revision 1.117  1996/03/21  18:01:08  cg
 * improved function PrintChar
 *
 * Revision 1.116  1996/02/21  18:02:09  cg
 * now, special characters such as '\n' or '\0' are accepted for printing
 *
 * Revision 1.115  1996/02/12  14:13:46  asi
 * PrintArg and PrintVardec updated for correct idx-output
 *
 * Revision 1.114  1996/02/06  16:10:20  sbs
 * Double2String and Float2String inserted.
 *
 * Revision 1.113  1996/02/06  13:59:44  sbs
 * PrintDouble and PrintFloat forced to append .0 / .0f for whole numbers
 *
 * Revision 1.112  1996/01/25  18:39:56  cg
 * beautified printing of blocks with and without return
 * class type is printed in comments until typedef is created
 *
 * Revision 1.111  1996/01/23  09:00:58  cg
 * changed printing of objdefs and pragmas
 *
 * Revision 1.110  1996/01/22  17:29:58  cg
 * pragma initfun is now printed
 *
 * Revision 1.109  1996/01/21  13:56:54  cg
 * bugs fixed in PrintArg and PrintIds
 *
 * Revision 1.108  1996/01/16  16:57:00  cg
 * extended macro TYP_IF to 5 entries
 *
 * Revision 1.107  1996/01/10  15:13:48  cg
 * now at least one newline is printed after vardecs
 *
 * Revision 1.106  1996/01/09  08:54:58  cg
 * added new version of function Print in comments
 *
 * Revision 1.105  1996/01/07  16:55:50  cg
 * pragmas of typedefs and objdefs are no longer printed
 *
 * Revision 1.104  1996/01/05  14:34:22  cg
 * modified functions for printing genarray, modarray, foldfun, and
 * foldprf. Corresponding return is now infered.
 *
 * Revision 1.103  1996/01/05  13:13:08  cg
 * added function PrintStr, modified PrintPragma for new storage
 * format of strings.
 *
 * Revision 1.102  1996/01/05  12:32:12  cg
 * Now, function WriteOpen is used to open c-file
 *
 * Revision 1.101  1996/01/02  15:52:59  cg
 * function Print now opens outfile itself.
 * If compilation process is interrupted (break parameter), then the
 * resulting program is written to stdout, otherwise to the file
 * specified by the global variable cfilename.
 *
 * Revision 1.100  1995/12/29  10:35:33  cg
 * modified printing of pragmas which can now be printed within
 * or outside comments. With-loops will now be printed in the new syntax
 *
 * Revision 1.99  1995/12/21  15:03:36  cg
 * bugs fixed in printing of pragmas
 *
 * Revision 1.98  1995/12/20  08:18:14  cg
 * added PrintChar, modified PrintPragma
 *
 * Revision 1.97  1995/12/18  18:27:48  cg
 * Bugs fixed in PrintIcm and PrintObjdef. Now the icms for global arrays
 * will be printed correctly.
 *
 * Revision 1.96  1995/12/18  16:18:48  cg
 * Bug fixed in PrintId
 *
 * Revision 1.95  1995/12/04  16:18:25  hw
 * added primitve functions toi, tof & tod
 *
 * Revision 1.94  1995/12/01  20:27:18  cg
 * now init expressions are only printed if present and not if the
 * object is not imported
 *
 * Revision 1.93  1995/12/01  17:13:20  cg
 * added function PrintPragma.
 * additional external declarations to allow mutual recursive C functions
 * are now generated by PrintFundef instead of compile.c
 *
 * Revision 1.92  1995/11/10  15:02:51  cg
 * new layout when writing to stdout to fit with compile time
 * output sent to stderr.
 *
 * Revision 1.91  1995/11/06  09:22:49  cg
 * moved initialization of global variable indent to function Print
 *
 * Revision 1.90  1995/11/01  16:28:21  cg
 * bug fixed in writing module/class name as comment in beginning of
 * C program. Now, macro MODUL_FILETYPE used for distinction.
 *
 * Revision 1.89  1995/11/01  09:33:56  cg
 * Bug fixed in PrintArg.
 *
 * Revision 1.88  1995/10/31  08:55:23  cg
 * function PrintArg now is able to print arguments with or without
 * identifier name depending on the second parameter.
 *
 * Revision 1.87  1995/10/26  16:04:59  cg
 * Files without any functions can be printed now.
 * Converted from macro MOD_NAME_CON to global variable mod_name_con
 *
 * Revision 1.86  1995/10/12  13:46:35  cg
 * PrintFundef and PrintObjdef now rely on STATUS to distinguish between defined
 * and imported items
 *
 * Revision 1.85  1995/10/12  09:01:30  cg
 * "mod:id" now printed in expressions
 *
 * Revision 1.84  1995/09/04  11:48:33  asi
 * PrintFloat and PrintDouble changed ( %.256g used in printf )
 *
 * Revision 1.83  1995/08/11  17:35:36  hw
 * changed PrintPrf (F_modarray, F_genarray inserted)
 *
 * Revision 1.82  1995/08/05  15:45:38  hw
 * changed output of N_fundef node (real name of function (node[5]) will
 * be printed too)
 *
 * Revision 1.81  1995/07/17  14:21:20  hw
 * inline will be printed in front of the declaration of a function
 *
 * Revision 1.80  1995/07/11  10:02:07  cg
 * Module/Class header now in comments.
 *
 * Revision 1.79  1995/07/11  09:02:37  cg
 * most sac grammar version 0.6 features will be printed.
 *
 * Revision 1.78  1995/07/10  07:33:37  asi
 * removed bblock from structure node
 *
 * Revision 1.77  1995/07/07  14:30:33  hw
 * enlarged macro PRF_IF( there are 4 args now)
 *
 * Revision 1.76  1995/07/04  08:36:53  hw
 * - changed PrintPrf( ftod, itod, dtof, dtoi inserted)
 * - PrintDouble inserted.
 *
 * Revision 1.75  1995/06/30  12:00:30  hw
 * changed PrintPrf
 *  -  renamed F_trunc with F_ftoi
 *  - added F_itof, F_ftoi_A, F_itof_A & F_itof
 *
 * Revision 1.74  1995/06/28  15:54:22  hw
 * changed PrintPrf( trunc & idx_psi added)
 *
 * Revision 1.73  1995/06/19  16:27:41  asi
 * debug option LINE modified
 *
 * Revision 1.72  1995/06/15  16:13:46  hw
 * changed PrintIcm( ND_TYPEDEF_ARRAY will be printed in another way)
 *
 * Revision 1.71  1995/06/13  15:40:35  hw
 * changed PrintId (now N_str will be printed also )
 *
 * Revision 1.70  1995/06/09  13:31:42  asi
 * inline will be printed now
 *
 * Revision 1.69  1995/06/06  14:04:53  cg
 * constants and objects will be printed now.
 *
 * Revision 1.68  1995/06/06  07:53:11  sbs
 * some bugs in PrintVect eliminated
 *
 * Revision 1.67  1995/06/02  17:15:21  sbs
 * PrintVectInfo inserted.
 *
 * Revision 1.66  1995/05/30  07:04:37  hw
 * changed PrintFold , because N_foldfun has now two child nodes
 *
 * Revision 1.65  1995/05/29  09:47:42  sbs
 * braces around cond's predicates inserted.
 *
 * Revision 1.64  1995/05/24  15:25:15  sbs
 * trace.h included
 *
 * Revision 1.63  1995/05/22  18:08:19  sbs
 * __trace_buffer inswerted
 *
 * Revision 1.62  1995/05/22  15:51:12  sbs
 * __trace_mem_cnt inserted
 *
 * Revision 1.61  1995/05/22  15:09:02  sbs
 * TRACE_MEM included
 *
 * Revision 1.60  1995/05/17  14:49:27  hw
 * changed PrintBool (TRUE => true; FALSE =>false )
 *
 * Revision 1.59  1995/05/04  11:41:20  sbs
 * ICM-macros adjusted to trf's
 *
 * Revision 1.58  1995/04/28  15:25:20  hw
 * changed PrintGenator ( refcount of index_vector will be shown )
 *
 * Revision 1.57  1995/04/15  15:11:44  asi
 * debug option LINE added, for linenumber output
 *
 * Revision 1.56  1995/04/11  15:08:50  hw
 * changed PrintFundef
 *
 * Revision 1.55  1995/04/11  11:34:45  asi
 * added 'flag' to struct 'node'
 *
 * Revision 1.54  1995/04/05  07:39:43  hw
 * extended PrintIcm
 *
 * Revision 1.53  1995/04/04  11:33:49  sbs
 * ";" at the end of ICM-Assigns eleminated; include inserted
 *
 * Revision 1.52  1995/04/04  09:34:26  sbs
 * parameter to ICM_END macro inserted
 *
 * Revision 1.51  1995/04/03  14:02:56  sbs
 * show_icm inserted
 *
 * Revision 1.50  1995/03/31  15:45:41  hw
 * changed PrintAssign
 *
 * Revision 1.49  1995/03/29  12:01:34  hw
 * PrintIcm added
 * changed PrintAssign (to use it with N_icm)
 *
 * Revision 1.48  1995/03/16  17:46:20  asi
 * output for arguments and variable numbers changed
 *
 * Revision 1.47  1995/03/16  17:22:19  asi
 * Output for Used Variables (gen-,modarray) modified
 *
 * Revision 1.46  1995/03/15  14:14:25  asi
 * output for masks modified
 *
 * Revision 1.45  1995/03/14  15:46:01  asi
 * printing of basic block numbers
 *
 * Revision 1.44  1995/03/14  10:54:29  hw
 * changed PrintId
 *
 * Revision 1.43  1995/03/13  16:54:00  asi
 * changed PrintPost and PrintPre
 *
 * Revision 1.42  1995/03/13  16:44:07  asi
 * changed PrintId
 *
 * Revision 1.41  1995/03/10  13:10:01  hw
 * - changed PrintId , PrintIds ( now refcounts can be printed)
 *
 * Revision 1.40  1995/03/08  14:40:10  sbs
 * include "tree.h" moved from tree.c to tree.h
 *
 * Revision 1.39  1995/03/08  14:04:01  sbs
 * INDENT & indent exported!
 *
 * Revision 1.38  1995/03/08  14:02:20  hw
 * changed PrintPrf
 *
 * Revision 1.37  1995/03/03  17:22:29  asi
 * debug-output for Generator added
 *
 * Revision 1.36  1995/03/01  16:29:50  hw
 * changed PrintGenerator ( name of index-vector is in info->ids)
 *
 * Revision 1.35  1995/03/01  16:04:41  asi
 * debug-output for with loops added
 *
 * Revision 1.34  1995/02/28  18:26:12  asi
 * added argument to functioncall PrintMask
 * added function PrintMasks
 *
 * Revision 1.33  1995/02/14  12:22:37  sbs
 * PrintFold inserted
 *
 * Revision 1.32  1995/02/14  10:12:05  sbs
 * superfluous "i"-declaration in PrintPrf removed
 *
 * Revision 1.31  1995/02/02  14:56:39  hw
 * changed PrintPrf, because N_prf has been changed
 *
 * Revision 1.30  1995/01/16  17:26:38  asi
 * Added PrintMask to PrintDo and PrintWhile
 *
 * Revision 1.29  1995/01/16  11:00:29  asi
 * decleration of print_tab removed, no longer needed
 *
 * Revision 1.28  1995/01/12  13:16:34  asi
 * DBUG_PRINT("MASK",... for optimizing routines degugging inserted
 *
 * Revision 1.27  1995/01/06  14:57:10  asi
 * changed OPTP to MASK and spezial Variable output added
 *
 * Revision 1.26  1995/01/05  15:28:00  asi
 * DBUG_PRINT( MASK,... added - defined and used variables
 * will be printed
 *
 * Revision 1.25  1995/01/05  11:51:25  sbs
 * MOD_NAME_CON macro inserted for mod-name generation for
 * types and functions.
 *
 * Revision 1.24  1995/01/02  19:45:20  sbs
 * PrintTypedef extended for types->id_mod!
 *
 * Revision 1.23  1994/12/31  14:07:01  sbs
 * types->id_mod prepanded at function declarations.
 * This is needed for external functions only!
 *
 * Revision 1.22  1994/12/31  13:54:17  sbs
 * id_mod for N_ap inserted
 *
 * Revision 1.21  1994/12/21  13:25:39  sbs
 * extern declaration for functions inserted
 *
 * Revision 1.20  1994/12/21  13:18:42  sbs
 * PrintFundef changed (works for empty function bodies too
 *
 * Revision 1.19  1994/12/15  17:14:03  sbs
 * PrintCast inserted
 *
 * Revision 1.18  1994/12/14  10:18:39  sbs
 * PrintModul & PrintImportlist & PrintTypedef inserted
 *
 * Revision 1.17  1994/12/08  14:23:41  hw
 * changed PrintAp & PrintFundef
 *
 * Revision 1.16  1994/12/05  11:17:43  hw
 * moved function Type2String to convert.c
 *
 * Revision 1.15  1994/12/02  11:11:55  hw
 * deleted declaration of char *type_string[], because it is defined in
 * my_debug.c now.
 *
 * Revision 1.14  1994/11/22  13:27:27  sbs
 * Typo in return value (previously void *) of PrintIds corrected
 *
 * Revision 1.13  1994/11/22  11:45:48  hw
 * - deleted PrintFor
 * - changed Type2string
 *
 * Revision 1.12  1994/11/17  16:49:03  hw
 * added ";" to output in PrintPost, PrintPre
 *
 * Revision 1.11  1994/11/15  10:06:15  sbs
 * typo for modarray eliminated
 *
 * Revision 1.10  1994/11/14  16:57:44  hw
 * make nicer output
 *
 * Revision 1.9  1994/11/11  16:43:07  hw
 * embellish output
 *
 * Revision 1.8  1994/11/11  13:55:56  hw
 * added new Print-functions: PrintInc PrintDec PrintPost PrintPre
 * embellish output
 *
 * Revision 1.7  1994/11/10  16:59:34  hw
 * added makro INDENT to have a nice output
 *
 * Revision 1.6  1994/11/10  15:34:26  sbs
 * RCS-header inserted
 *
 *
 */

/*
 * use of arg_info in this file:
 * - node[0] is used while printing the old WLs to return the main
 *   expr from the block.
 * - arg_info is not NULL if only function definitions (without bodies) should
 *   be printed.
 * - node[1]: profile macros
 * - node[2] determines which syntax of the new WLs is printed. If it's
 *   NULL then the intermal syntax is uses which allows to state more than
 *   one Npart. Else the last (and hopefully only) Npart returns the
 *   last expr in node[2].
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "print.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "optimize.h"
#include "profile.h"
#include "filemgr.h"
#include "globals.h"

int indent = 0;

static int print_separate = 0;

/*
 * First, we generate the external declarations for all functions that
 * expand ICMs to C.
 */

#define ICM_ALL
#define ICM_DEF(prf, trf) extern void Print##prf (node *ex, node *arg_info);
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_VAR(dim, name)
#define ICM_END(prf)
#include "icm.data"
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
#undef ICM_ALL

#define PRF_IF(n, s, x, y) x

char *prf_string[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

void
PrintFileHeader ()
{
    DBUG_ENTER ("PrintFileHeader");

    if (show_icm == 0) {
        if (traceflag != 0) {
            if (traceflag & TRACE_MEM) {
                fprintf (outfile, "#define TRACE_MEM\n");
            }

            if (traceflag & TRACE_REF) {
                fprintf (outfile, "#define TRACE_REF\n");
            }

            if (traceflag & TRACE_PRF) {
                fprintf (outfile, "#define TRACE_PRF\n");
            }
        }
        if (profileflag != 0) {
            fprintf (outfile, "#define PROFILE\n");
            if (profileflag & PROFILE_FUN) {
                fprintf (outfile, "#define PROFILE_FUN\n");
            }

            if (profileflag & PROFILE_LIB) {
                fprintf (outfile, "#define PROFILE_LIB\n");
            }

            if (profileflag & PROFILE_INL) {
                fprintf (outfile, "#define PROFILE_INL\n");
            }

            if (profileflag & PROFILE_WITH) {
                fprintf (outfile, "#define PROFILE_WITH\n");
            }
        }

        if (check_malloc) {
            fprintf (outfile, "#define CHECK_MALLOC\n");
        }

        if (profileflag != 0) {
            fprintf (outfile, "#include <sys/time.h>\n");
            fprintf (outfile, "#include <sys/resource.h>\n");
            fprintf (outfile, "extern int getrusage(int who, struct rusage *rusage);\n");
        }
        fprintf (outfile, "#include \"libsac.h\"\n");
        fprintf (outfile, "#include \"icm2c.h\"\n");
    }

    DBUG_VOID_RETURN;
}

/*
 * prints ids-information to outfile
 *
 */
void
PrintIds (ids *arg)
{
    DBUG_ENTER ("PrintIds");

    do {
        DBUG_PRINT ("PRINT", ("%s", arg->id));

        if (arg->mod != NULL)
            fprintf (outfile, "%s:", arg->mod);
        fprintf (outfile, "%s", arg->id);
        if ((arg->refcnt != -1) && show_refcnt)
            fprintf (outfile, ":%d", arg->refcnt);
        if (show_idx && arg->use)
            Trav (arg->use, NULL);
        if (NULL != arg->next)
            fprintf (outfile, ", ");
        arg = arg->next;
    } while (NULL != arg);

    DBUG_VOID_RETURN;
}

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

node *
PrintAssign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAssign");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - assign : %s\n",
                                   mdb_nodetype[arg_node->node[0]->nodetype]);
                  PrintMasks (arg_node, arg_info););

    DBUG_PRINT ("FLAG", ("\nflag = %d", arg_node->flag));

    if (N_icm == ASSIGN_INSTR (arg_node)->nodetype) {
        PrintIcm (ASSIGN_INSTR (arg_node), arg_info);
        fprintf (outfile, "\n");
        if (ASSIGN_NEXT (arg_node))
            Trav (arg_node->node[1], arg_info);
    } else {
        DBUG_EXECUTE ("LINE", fprintf (outfile, "/*%03d*/", arg_node->lineno););

        if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_return)
            || (RETURN_EXPRS (ASSIGN_INSTR (arg_node)) != NULL)) {
            INDENT;
            Trav (ASSIGN_INSTR (arg_node), arg_info);
            fprintf (outfile, "\n");
        }

        if (ASSIGN_NEXT (arg_node))
            Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PrintBlock (node *arg_node, node *arg_info)
{
    static profile_setup_flag = 0;

    DBUG_ENTER ("PrintBlock");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    INDENT;
    fprintf (outfile, "{ \n");
    indent++;

    if (BLOCK_VARDEC (arg_node) != NULL) {
        Trav (BLOCK_VARDEC (arg_node), arg_info);
        fprintf (outfile, "\n");
    }

    if ((INFO_FUNDEF (arg_info) != NULL)
        && (strcmp (FUNDEF_NAME (INFO_FUNDEF (arg_info)), "main") == 0)
        && (profile_setup_flag == 0)) {
        profile_setup_flag = 1;
        INDENT;
        fprintf (outfile, "PROFILE_SETUP( %d );\n", PFfuncntr);
    }

    if (BLOCK_INSTR (arg_node)) {
        Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_RETURN (arg_node);
}

node *
PrintLet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintLet");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    if (LET_IDS (arg_node)) {
        PrintIds (LET_IDS (arg_node));
        fprintf (outfile, " = ");
    }
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, "; ");

    DBUG_RETURN (arg_node);
}

node *
PrintAnnotate (node *arg_node, node *arg_info)
{
    static char strbuffer1[256];
    static char strbuffer2[256];

    DBUG_ENTER ("PrintAnnotate");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

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

    if (ANNOTATE_TAG (arg_node) & INL_FUN)
        sprintf (strbuffer2, "PROFILE_INLINE( %s )", strbuffer1);
    else
        strcpy (strbuffer2, strbuffer1);

    if (ANNOTATE_TAG (arg_node) & LIB_FUN)
        sprintf (strbuffer1, "PROFILE_LIBRARY( %s )", strbuffer2);
    else
        strcpy (strbuffer1, strbuffer2);

    fprintf (outfile, "%s;", strbuffer1);

    DBUG_RETURN (arg_node);
}

node *
PrintModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintModul");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    if (print_separate) {
        outfile = WriteOpen ("%s/header.h", tmp_dirname);
        PrintFileHeader ();

        if (NULL != arg_node->node[1]) {
            fprintf (outfile, "\n\n");
            Trav (MODUL_TYPES (arg_node), arg_info); /* print typedefs */
        }

        if (NULL != arg_node->node[2]) {
            fprintf (outfile, "\n\n");
            Trav (MODUL_FUNS (arg_node), arg_node); /* print function declarations */
        }

        if (NULL != arg_node->node[3]) {
            fprintf (outfile, "\n\n");
            print_objdef_for_header_file = 1;

            Trav (MODUL_OBJS (arg_node), arg_info); /* print object declarations */
        }

        fclose (outfile);

        outfile = WriteOpen ("%s/globals.c", tmp_dirname);
        fprintf (outfile, "#include \"header.h\"\n\n");
        fprintf (outfile, "int __dummy_value_which_is_completely_useless=0;\n\n");

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

        if (NULL != arg_node->node[3]) {
            fprintf (outfile, "\n\n");
            print_objdef_for_header_file = 0;

            Trav (arg_node->node[3], arg_info); /* print object definitions */
        }

        fclose (outfile);

        if (NULL != arg_node->node[2]) {
            fprintf (outfile, "\n\n");
            Trav (arg_node->node[2], NULL); /* print function definitions */
        }
    } else {
        switch (MODUL_FILETYPE (arg_node)) {
        case F_modimp:
            fprintf (outfile, "\n/*\n *  Module %s :\n */\n", arg_node->info.id);
            break;
        case F_classimp:
            fprintf (outfile, "\n/*\n *  Class %s :\n", arg_node->info.id);
            if (MODUL_CLASSTYPE (arg_node) != NULL) {
                fprintf (outfile, " *  classtype %s;\n",
                         Type2String (MODUL_CLASSTYPE (arg_node), 0));
            }
            fprintf (outfile, " */\n");
            break;
        case F_prog:
            fprintf (outfile, "\n/*\n *  SAC-Program %s :\n */\n", filename);
            break;
        default:;
        }

        if (MODUL_IMPORTS (arg_node)) {
            fprintf (outfile, "\n");
            Trav (MODUL_IMPORTS (arg_node), arg_info); /* print import-list */
        }

        if (MODUL_TYPES (arg_node)) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  type definitions\n");
            fprintf (outfile, " */\n\n");
            Trav (MODUL_TYPES (arg_node), arg_info); /* print typedefs */
        }

        if (MODUL_FUNS (arg_node)) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  function declarations\n");
            fprintf (outfile, " */\n\n");
            Trav (MODUL_FUNS (arg_node), arg_node); /* print functions */
        }

        if (MODUL_OBJS (arg_node)) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  global objects\n");
            fprintf (outfile, " */\n\n");
            Trav (MODUL_OBJS (arg_node), arg_info); /* print objdefs */
        }

        if (MODUL_FUNS (arg_node)) {
            fprintf (outfile, "\n\n");
            fprintf (outfile, "/*\n");
            fprintf (outfile, " *  function definitions\n");
            fprintf (outfile, " */\n");
            Trav (MODUL_FUNS (arg_node), NULL); /* print functions */
        }
    }

    DBUG_RETURN (arg_node);
}

node *
PrintImplist (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintImplist");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    fprintf (outfile, "import %s: ", arg_node->info.id);

    if ((arg_node->node[1] == NULL) && (arg_node->node[2] == NULL)
        && (arg_node->node[3] == NULL) && (arg_node->node[4] == NULL))
        fprintf (outfile, "all;\n");
    else {
        fprintf (outfile, "{");
        if (arg_node->node[1] != NULL) {
            fprintf (outfile, "\n  implicit types: ");
            PrintIds ((ids *)arg_node->node[1]); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (arg_node->node[2] != NULL) {
            fprintf (outfile, "\n  explicit types: ");
            PrintIds ((ids *)arg_node->node[2]); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (arg_node->node[4] != NULL) {
            fprintf (outfile, "\n  global objects: ");
            PrintIds ((ids *)arg_node->node[4]); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        if (arg_node->node[3] != NULL) {
            fprintf (outfile, "\n  funs: ");
            PrintIds ((ids *)arg_node->node[3]); /* dirty trick for keeping ids */
            fprintf (outfile, ";");
        }
        fprintf (outfile, "\n}\n");
    }

    if (arg_node->node[0])
        Trav (arg_node->node[0], arg_info); /* print further imports */

    DBUG_RETURN (arg_node);
}

node *
PrintTypedef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintTypedef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    fprintf (outfile, "typedef %s ", Type2String (arg_node->info.types, 0));
    if (arg_node->info.types->id_mod != NULL)
        fprintf (outfile, "%s%s", arg_node->info.types->id_mod, mod_name_con);
    fprintf (outfile, "%s;\n", arg_node->info.types->id);

    if (TYPEDEF_COPYFUN (arg_node) != NULL) {
        fprintf (outfile, "\nextern void *%s(void *);\n", TYPEDEF_COPYFUN (arg_node));
        fprintf (outfile, "extern void %s(void *);\n\n", TYPEDEF_FREEFUN (arg_node));
    }

    if (arg_node->node[0])
        Trav (arg_node->node[0], arg_info); /* traverse next typedef/fundef */

    DBUG_RETURN (arg_node);
}

node *
PrintObjdef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintObjdef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    if ((OBJDEF_ICM (arg_node) != NULL) && (NODE_TYPE (OBJDEF_ICM (arg_node)) == N_icm)) {
        Trav (OBJDEF_ICM (arg_node), arg_info);
        fprintf (outfile, "\n");
    } else {
        if ((arg_node->info.types->status == ST_imported)
            || print_objdef_for_header_file) {
            fprintf (outfile, "extern ");
        }

        fprintf (outfile, "%s ", Type2String (arg_node->info.types, 0));

        if (arg_node->info.types->id_mod != NULL) {
            fprintf (outfile, "%s%s", arg_node->info.types->id_mod, mod_name_con);
        }

        fprintf (outfile, "%s", arg_node->info.types->id);

        if (arg_node->node[1] != NULL) {
            fprintf (outfile, " = ");
            Trav (arg_node->node[1], arg_info);
        }

        fprintf (outfile, ";\n");

        if (OBJDEF_PRAGMA (arg_node) != NULL) {
            Trav (OBJDEF_PRAGMA (arg_node), arg_info);
        }
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        Trav (arg_node->node[0], arg_info); /* traverse next objdef */
    }

    DBUG_RETURN (arg_node);
}

void
PrintFunctionHeader (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintFunctionHeader");

    if (0 != arg_node->flag)
        fprintf (outfile, "inline ");

    fprintf (outfile, "%s ", Type2String (arg_node->info.types, 0));

    if (arg_node->info.types->id_mod != NULL)
        fprintf (outfile, "%s%s", arg_node->info.types->id_mod, mod_name_con);

    fprintf (outfile, "%s(", arg_node->info.types->id);

    if (arg_node->node[2] != NULL)
        Trav (arg_node->node[2], arg_info); /* print args of function */

    fprintf (outfile, ")");

    DBUG_VOID_RETURN;
}

/*
 * Remark for PrintFundef:
 *
 *  arg_info is used as flag :
 *  arg_info==NULL : print function definitions (with body)
 *  arg_info!=NULL : print function declarations (without body)
 *
 *  If C-code is to be generated, which means that an N_icm node already
 *  hangs on node[3], additional extern declarations for function
 *  definitions are printed.
 */

node *
PrintFundef (node *arg_node, node *arg_info)
{
    node *new_info;

    DBUG_ENTER ("PrintFundef");

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[arg_node->nodetype], arg_node));

    new_info = MakeNode (N_info);
    new_info->varno = arg_node->varno;
    INFO_FUNDEF (new_info) = arg_node; /* needed for the introduction
                                        * of PROFILE_... MACROS in the
                                        * function body
                                        */
    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - function\n");
                  PrintMasks (arg_node, new_info););

    if (arg_info == NULL) {
        /*
         *  print function definition
         */

        if (FUNDEF_BODY (arg_node)) {
            if (print_separate) {
                outfile = WriteOpen ("%s/fun%d.c", tmp_dirname, function_counter);

                fprintf (outfile, "#include \"header.h\"\n");
            }

            fprintf (outfile, "\n");

            if (FUNDEF_ICM (arg_node) && (N_icm == FUNDEF_ICM (arg_node)->nodetype)) {
                Trav (arg_node->node[3], new_info); /* print N_icm ND_FUN_DEC */
            } else {
                PrintFunctionHeader (arg_node, new_info);
            }

            fprintf (outfile, "\n");
            Trav (FUNDEF_BODY (arg_node), new_info); /* traverse function body */

            if (FUNDEF_PRAGMA (arg_node) != NULL) {
                Trav (FUNDEF_PRAGMA (arg_node), NULL);
            }

            if (print_separate) {
                fclose (outfile);
                function_counter += 1;
            }
        }
    } else {
        /*
         *  print function declaration
         */

        if ((arg_node->node[0] == NULL)
            || ((NULL != arg_node->node[3]) && (N_icm == arg_node->node[3]->nodetype)
                && (strcmp (arg_node->info.types->id, "main") != 0))) {
            fprintf (outfile, "extern ");

            if ((NULL != arg_node->node[3]) && (N_icm == arg_node->node[3]->nodetype)) {
                Trav (arg_node->node[3], new_info); /* print N_icm ND_FUN_DEC */
            } else {
                PrintFunctionHeader (arg_node, new_info);
            }

            fprintf (outfile, ";\n");

            if (FUNDEF_PRAGMA (arg_node) != NULL) {
                Trav (FUNDEF_PRAGMA (arg_node), NULL);
            }
        }
    }

    FREE (new_info);

    if (FUNDEF_NEXT (arg_node))
        Trav (FUNDEF_NEXT (arg_node), arg_info); /* traverse next function */

    DBUG_RETURN (arg_node);
}

node *
PrintPrf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPrf");

    DBUG_PRINT ("PRINT", ("%s (%s)" P_FORMAT, mdb_nodetype[arg_node->nodetype],
                          mdb_prf[arg_node->info.prf], arg_node));

    switch (arg_node->info.prf) {
    case F_take:
    case F_drop:
    case F_psi:
    case F_shape:
    case F_reshape:
    case F_cat:
    case F_dim:
    case F_rotate:
    case F_not:
    case F_abs:
    case F_ftoi:
    case F_ftoi_A:
    case F_ftod:
    case F_ftod_A:
    case F_itof:
    case F_min:
    case F_max:
    case F_itof_A:
    case F_itod:
    case F_itod_A:
    case F_dtoi:
    case F_dtof:
    case F_dtof_A:
    case F_toi:
    case F_toi_A:
    case F_tof:
    case F_tof_A:
    case F_tod:
    case F_tod_A:
    case F_idx_psi:
    case F_modarray:
    case F_genarray:
    case F_idx_modarray: {
        /* primitive functions that are printed as function application */
        fprintf (outfile, "%s( ", prf_string[arg_node->info.prf]);
        Trav (arg_node->node[0], arg_info);
        fprintf (outfile, " )");
        break;
    }
    default: {
        /* primitive functions in infix notation */
        fprintf (outfile, "(");
        Trav (arg_node->node[0]->node[0], arg_info);
        fprintf (outfile, " %s ", prf_string[arg_node->info.prf]);
        if (NULL != arg_node->node[0]->node[1])
            Trav (arg_node->node[0]->node[1]->node[0], arg_info);
        fprintf (outfile, ")");
        break;
    }
    }

    DBUG_RETURN (arg_node);
}

node *
PrintStr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintStr");

    DBUG_ASSERT ((N_str == arg_node->nodetype), "wrong arg_node->nodetype ");

    fprintf (outfile, "\"%s\"", STR_STRING (arg_node));

    DBUG_RETURN (arg_node);
}

node *
PrintId (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintId");

    DBUG_ASSERT ((N_id == arg_node->nodetype), "wrong arg_node->nodetype ");

    if ((ID_ATTRIB (arg_node) == ST_global) && (ID_MOD (arg_node) != NULL)) {
        fprintf (outfile, "%s:", ID_MOD (arg_node));
    }

    if ((0 == show_refcnt) || (-1 == arg_node->refcnt))
        fprintf (outfile, "%s", arg_node->info.ids->id);
    else
        fprintf (outfile, "%s:%d", arg_node->info.ids->id, arg_node->refcnt);

    DBUG_RETURN (arg_node);
}

node *
PrintNum (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("PrintNum");

    fprintf (outfile, "%d", arg_node->info.cint);

    DBUG_RETURN (arg_node);
}

node *
PrintChar (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("PrintChar");

    if ((arg_node->info.cchar >= ' ') && (arg_node->info.cchar <= '~')
        && (arg_node->info.cchar != '\'')) {
        fprintf (outfile, "'%c'", arg_node->info.cchar);
    } else {
        fprintf (outfile, "'\\%o'", arg_node->info.cchar);
    }

    DBUG_RETURN (arg_node);
}

node *
PrintFloat (node *arg_node, node *arg_info)
{
    char *tmp_string;

    DBUG_ENTER ("PrintFloat");

    tmp_string = Float2String (arg_node->info.cfloat);
    fprintf (outfile, "%s", tmp_string);
    FREE (tmp_string);

    DBUG_RETURN (arg_node);
}

node *
PrintDouble (node *arg_node, node *arg_info)
{

    char *tmp_string;

    DBUG_ENTER ("PrintDouble");

    tmp_string = Double2String (arg_node->info.cdbl);
    fprintf (outfile, "%s", tmp_string);
    FREE (tmp_string);

    DBUG_RETURN (arg_node);
}

node *
PrintBool (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("PrintBool");

    if (0 == arg_node->info.cint)
        fprintf (outfile, "false");
    else
        fprintf (outfile, "true");

    DBUG_RETURN (arg_node);
}

node *
PrintReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintReturn");

    if (RETURN_EXPRS (arg_node) && (!RETURN_INWITH (arg_node))) {
        if ((INFO_FUNDEF (arg_info) != NULL)
            && (strcmp (FUNDEF_NAME (INFO_FUNDEF (arg_info)), "main") == 0)) {
            fprintf (outfile, "PROFILE_PRINT();\n");
            INDENT;
        }
        fprintf (outfile, "return( ");
        Trav (arg_node->node[0], arg_info);
        fprintf (outfile, " );");
    }

    if (RETURN_INWITH (arg_node)) {
        arg_info->node[0] = arg_node;
    }

    DBUG_RETURN (arg_node);
}

node *
PrintAp (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAp");

    if (arg_node->info.fun_name.id_mod != NULL)
        fprintf (outfile, "%s%s", arg_node->info.fun_name.id_mod, mod_name_con);
    fprintf (outfile, "%s(", arg_node->info.fun_name.id);
    if (arg_node->node[0])
        Trav (arg_node->node[0], arg_info);
    fprintf (outfile, ")");

    DBUG_RETURN (arg_node);
}

node *
PrintCast (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCast");

    fprintf (outfile, "(: %s) ", Type2String (arg_node->info.types, 0));
    Trav (arg_node->node[0], arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintExprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintExprs");

    Trav (arg_node->node[0], arg_info);

    if (arg_node->node[1]) {
        fprintf (outfile, ", ");
        Trav (arg_node->node[1], arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PrintArg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintArg");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**Number %d -> ", arg_node->varno););

    fprintf (outfile, "%s",
             Type2String (arg_node->info.types, (arg_info == NULL) ? 0 : 1));

    if ((1 == show_refcnt) && (-1 != ARG_REFCNT (arg_node))) {
        fprintf (outfile, ":%d", ARG_REFCNT (arg_node));
    }

    if (ARG_COLCHN (arg_node) && show_idx) {
        Trav (ARG_COLCHN (arg_node), arg_info);
    }

    if (arg_node->node[0]) {
        fprintf (outfile, ", ");
        Trav (arg_node->node[0], arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PrintVardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintVardec");

    INDENT;

    DBUG_EXECUTE ("MASK", fprintf (outfile, "**Number %d -> ", arg_node->varno););

    fprintf (outfile, "%s", Type2String (arg_node->info.types, 1));
    if (VARDEC_COLCHN (arg_node) && show_idx)
        Trav (VARDEC_COLCHN (arg_node), arg_info);
    fprintf (outfile, ";\n");
    if (arg_node->node[0])
        Trav (arg_node->node[0], arg_info);
    else
        fprintf (outfile, "\n");

    DBUG_RETURN (arg_node);
}

node *
PrintDo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintDo");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - do body\n");
                  PrintMasks (arg_node->node[1], arg_info););

    fprintf (outfile, "do\n");
    if (NULL != arg_node->node[1]) {
        indent++;
        Trav (arg_node->node[1], arg_info); /* traverse body of loop */
        indent--;
    }

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[1], VARNO);
                  fprintf (outfile, "**Used Variables (do-cnd) : %s\n", text);
                  FREE (text););

    INDENT;
    fprintf (outfile, "while( ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " );\n");

    DBUG_RETURN (arg_node);
}

node *
PrintEmpty (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintEmpty");

    INDENT;
    fprintf (outfile, "/* empty */\n");

    DBUG_RETURN (arg_node);
}

node *
PrintWhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWhile");

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (arg_node->mask[1], VARNO);
                  fprintf (outfile, "**Used Variables (while-cnd) : %s\n", text);
                  FREE (text););

    fprintf (outfile, "while( ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " )\n");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - while body\n");
                  PrintMasks (arg_node->node[1], arg_info););

    Trav (arg_node->node[1], arg_info); /* traverse body of loop */

    DBUG_RETURN (arg_node);
}

node *
PrintCond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintCond");

    fprintf (outfile, "if (");

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (COND_MASK (arg_node, 1), VARNO);
                  fprintf (outfile, "**Used Variables (Cond) : %s\n", text);
                  FREE (text););

    Trav (COND_COND (arg_node), arg_info);
    fprintf (outfile, ")\n");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - then\n");
                  PrintMasks (COND_THEN (arg_node), arg_info););

    Trav (COND_THEN (arg_node), arg_info);

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - else\n");
                  PrintMasks (COND_ELSE (arg_node), arg_info););

    INDENT;
    fprintf (outfile, "else\n");
    Trav (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintWith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWith");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - generator\n");
                  PrintMasks (arg_node->node[0], arg_info););
    fprintf (outfile, "with (");
    Trav (WITH_GEN (arg_node), arg_info);
    fprintf (outfile, ") ");

    DBUG_EXECUTE ("MASK", char *text;
                  text = PrintMask (arg_node->node[1]->mask[1], VARNO);
                  fprintf (outfile, "**Used Variables (gen-,modarray) : %s\n", text);
                  FREE (text););

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - with body\n");
                  PrintMasks (arg_node, arg_info););
    Trav (WITH_OPERATOR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintGenator (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintGenator");

    Trav (GEN_LEFT (arg_node), arg_info);
    if ((-1 == arg_node->info.ids->refcnt) || (0 == show_refcnt))
        fprintf (outfile, " <= %s <= ", arg_node->info.ids->id);
    else
        fprintf (outfile, " <= %s:%d <= ", arg_node->info.ids->id,
                 arg_node->info.ids->refcnt);
    Trav (GEN_RIGHT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
PrintGenarray (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintGenarray");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (GENARRAY_BODY (arg_node)))) != N_return) {
        /* right now arg_info->node[0] is NULL, but in PrintReturn it
           will be replaced by a pointer to an N_return node instead of
           printing it. */
        fprintf (outfile, "\n");
        Trav (GENARRAY_BODY (arg_node), arg_info);
        ret_node = arg_info->node[0];

        INDENT;
    } else {
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

node *
PrintModarray (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintModarray");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (MODARRAY_BODY (arg_node)))) != N_return) {
        fprintf (outfile, "\n");
        Trav (MODARRAY_BODY (arg_node), arg_info);
        ret_node = arg_info->node[0];

        INDENT;
    } else {
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

node *
PrintFoldfun (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintFold");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (FOLDFUN_BODY (arg_node)))) != N_return) {
        fprintf (outfile, "\n");
        Trav (FOLDFUN_BODY (arg_node), arg_info);
        ret_node = arg_info->node[0];

        INDENT;
    } else {
        ret_node = ASSIGN_INSTR (BLOCK_INSTR (FOLDFUN_BODY (arg_node)));
    }

    DBUG_ASSERT (ret_node != NULL, "foldfun without return-statement");

    if (NULL != FOLDFUN_MOD (arg_node)) {
        fprintf (outfile, "fold( %s%s%s, ", FOLDFUN_MOD (arg_node), mod_name_con,
                 FOLDFUN_NAME (arg_node));
    } else {
        fprintf (outfile, "fold( %s, ", FOLDFUN_NAME (arg_node));
    }

    Trav (FOLDFUN_NEUTRAL (arg_node), arg_info);
    fprintf (outfile, ", ");
    Trav (RETURN_EXPRS (ret_node), arg_info);
    fprintf (outfile, " )");

    DBUG_RETURN (arg_node);
}

node *
PrintFoldprf (node *arg_node, node *arg_info)
{
    node *ret_node;

    DBUG_ENTER ("PrintFold");

    INDENT;

    if (NODE_TYPE (ASSIGN_INSTR (BLOCK_INSTR (FOLDPRF_BODY (arg_node)))) != N_return) {
        fprintf (outfile, "\n");
        Trav (FOLDPRF_BODY (arg_node), arg_info);
        ret_node = arg_info->node[0];

        INDENT;
    } else {
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

node *
PrintArray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintArray");

    fprintf (outfile, "[ ");
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, " ]");

    DBUG_RETURN (arg_node);
}

node *
PrintDec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintDec");

    fprintf (outfile, "--");

    DBUG_RETURN (arg_node);
}

node *
PrintInc (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintInc");

    fprintf (outfile, "++");

    DBUG_RETURN (arg_node);
}

node *
PrintPost (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPost");

    PrintIds (arg_node->info.ids);
    Trav (arg_node->node[0], arg_info);
    fprintf (outfile, ";");

    DBUG_RETURN (arg_node);
}

node *
PrintPre (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintPre");

    Trav (arg_node->node[0], arg_info);
    PrintIds (arg_node->info.ids);
    fprintf (outfile, ";");

    DBUG_RETURN (arg_node);
}

node *
PrintVectInfo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintVectInfo");
    if (show_idx) {
        if (arg_node->info.use == VECT)
            fprintf (outfile, ":VECT");
        else {
            fprintf (outfile, ":IDX(%s)", Type2String ((types *)arg_node->node[1], 0));
        }
        if (arg_node->node[0]) {
            Trav (arg_node->node[0], arg_info);
        }
        fprintf (outfile, " ");
    }
    DBUG_RETURN (arg_node);
}

node *
PrintIcm (node *arg_node, node *arg_info)
{
    int compiled_icm = 0;
    DBUG_ENTER ("PrintIcm");
    DBUG_PRINT ("PRINT", ("icm-node %s\n", arg_node->info.fun_name.id));

    if (show_icm == 0)
#define ICM_ALL
#define ICM_DEF(prf, trf)                                                                \
    if (strcmp (arg_node->info.fun_name.id, #prf) == 0) {                                \
        Print##prf (arg_node->node[0], arg_info);                                        \
        compiled_icm = 1;                                                                \
    } else
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_VAR(dim, name)
#define ICM_END(prf)
#include "icm.data"
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
#undef ICM_ALL
        if (strcmp (ICM_NAME (arg_node), "NOOP") == 0)
            compiled_icm = 1;

    if ((show_icm == 1) || (compiled_icm == 0)) {
        if ((strcmp (ICM_NAME (arg_node), "ND_FUN_RET") == 0)
            && (strcmp (FUNDEF_NAME (INFO_FUNDEF (arg_info)), "main") == 0)) {
            INDENT;
            fprintf (outfile, "PROFILE_PRINT();\n");
        }

        INDENT;
        fprintf (outfile, "%s(", ICM_NAME (arg_node));
        if (NULL != arg_node->node[0])
            Trav (arg_node->node[0], arg_info);
        fprintf (outfile, ")");
    }

    if (NULL != arg_node->node[1]) {
        if ((1 == show_icm) || (0 == compiled_icm)) {
            if (0 == strcmp (ICM_NAME (arg_node), "ND_TYPEDEF_ARRAY")) {
                fprintf (outfile, "\n");
                INDENT;
            } else {
                fprintf (outfile, ", ");
            }

            Trav (arg_node->node[1], arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

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

    fprintf (outfile, "\n");

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
 * node[2] of arg_info is NULL for the internal syntax or != NULL if
 * PrintNpart shall return the last expr.
 *
 ******************************************************************************/

node *
PrintNwith (node *arg_node, node *arg_info)
{
    node *buffer;

    DBUG_ENTER ("PrintNwith");

    DBUG_ASSERT (arg_info, "arg_info is NULL");
    buffer = arg_info->node[2];

    /* check wether to use output format 1 (multiple
       NParts) or 2 (only one NPart) and use
       arg_info->node[2] as flag for traversal. */
    if (NPART_NEXT (NWITH_PART (arg_node))) {
        /* output format 1 */
        arg_info->node[2] = NULL;
        fprintf (outfile, "new_with\n");
        indent++;
        Trav (NWITH_PART (arg_node), arg_info);
        indent--;
    } else {
        /* output format 2 */
        arg_info->node[2] = (node *)!NULL; /* set != NULL */
        fprintf (outfile, "new_with ");
        Trav (NWITH_PART (arg_node), arg_info);
    }

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    if (!NPART_NEXT (NWITH_PART (arg_node))) {
        /* output format 2: now we have in
           arg_info->node[2] the last expr. */
        fprintf (outfile, ", ");
        Trav (arg_info->node[2], arg_info);
    }
    fprintf (outfile, ")");

    arg_info->node[2] = buffer;
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
 *   node *PrintNgenerator(node *gen, node *idx, node *arg_info)
 *
 * description:
 *   prints a generator
 *
 *   ATTENTION: this function is not being used by the conventional
 *   traversation algorithm but from within PrintNPart.
 *
 ******************************************************************************/

node *
PrintNgenerator (node *gen, node *idx, node *arg_info)
{
    DBUG_ENTER ("PrintNgenerator");

    fprintf (outfile, "(");

    /* print upper bound and first operator*/
    if (NGEN_BOUND1 (gen))
        Trav (NGEN_BOUND1 (gen), arg_info);
    else
        fprintf (outfile, ".");
    fprintf (outfile, " %s ", prf_string[NGEN_OP1 (gen)]);

    /* print indices */
    idx = Trav (idx, arg_info);

    /* print second operator and lower bound */
    fprintf (outfile, " %s ", prf_string[NGEN_OP2 (gen)]);
    if (NGEN_BOUND2 (gen))
        Trav (NGEN_BOUND2 (gen), arg_info);
    else
        fprintf (outfile, " .");

    /* print step and width */
    if (NGEN_STEP (gen)) {
        fprintf (outfile, " step ");
        Trav (NGEN_STEP (gen), arg_info);
    }
    if (NGEN_WIDTH (gen)) {
        fprintf (outfile, " width ");
        Trav (NGEN_WIDTH (gen), arg_info);
    }

    fprintf (outfile, ")");

    DBUG_RETURN ((node *)NULL);
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
    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - Ncode\n");
                  PrintMasks (arg_node, arg_info););

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
    fprintf (outfile, "}");

    /* print the expression if internal syntax should be used.
       else return expr in arg_info->node[2] */
    DBUG_ASSERT (NCODE_CEXPR (arg_node), "no expression at N_Ncode");
    if (arg_info->node[2] != NULL) {
        arg_info->node[2] = NCODE_CEXPR (arg_node);
    } else {
        fprintf (outfile, " : ");
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
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
    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - Npart\n");
                  PrintMasks (arg_node, arg_info););

    /* print generator */
    if (!arg_info->node[2])
        INDENT; /* each gen in a new line. */
    PrintNgenerator (NPART_GEN (arg_node), NPART_WITHID (arg_node), arg_info);

    DBUG_ASSERT ((NPART_CODE (arg_node) != NULL),
                 "part within WL without pointer to N_Ncode");
    NPART_CODE (arg_node) = Trav (NPART_CODE (arg_node), arg_info);

    if (NPART_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",\n");
        NPART_NEXT (arg_node)
          = Trav (NPART_NEXT (arg_node), arg_info); /* continue with other parts */
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

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - Nwithop\n");
                  PrintMasks (arg_node, arg_info););

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
    node *code;

    DBUG_ENTER ("PrintNwith2");

    fprintf (outfile, "new_with2 (");
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    fprintf (outfile, ")\n");

    indent++;
    INDENT
    fprintf (outfile, "/* operators: */\n");
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
    indent--;

    indent++;
    NWITH2_SEG (arg_node) = Trav (NWITH2_SEG (arg_node), arg_info);
    indent--;

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    fprintf (outfile, ")");

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
        INDENT
        fprintf (outfile, "/* segment %d: */\n", i++);
        WLSEG_INNER (seg) = Trav (WLSEG_INNER (seg), arg_info);
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
             WLBLOCK_BLOCKING (arg_node));

    if (WLBLOCK_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLBLOCK_NEXTDIM (arg_node) = Trav (WLBLOCK_NEXTDIM (arg_node), arg_info);
        indent--;
    }

    if (WLBLOCK_INNER (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLBLOCK_INNER (arg_node) = Trav (WLBLOCK_INNER (arg_node), arg_info);
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
    fprintf (outfile, "(%d -> %d), ublock[%d] %d\n", WLUBLOCK_BOUND1 (arg_node),
             WLUBLOCK_BOUND2 (arg_node), WLUBLOCK_DIM (arg_node),
             WLUBLOCK_BLOCKING (arg_node));

    if (WLUBLOCK_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLUBLOCK_NEXTDIM (arg_node) = Trav (WLUBLOCK_NEXTDIM (arg_node), arg_info);
        indent--;
    }

    if (WLUBLOCK_INNER (arg_node) != NULL) {
        fprintf (outfile, "\n");
        indent++;
        WLUBLOCK_INNER (arg_node) = Trav (WLUBLOCK_INNER (arg_node), arg_info);
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
 *   node *PrintWLproj(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLproj-nodes
 *
 ******************************************************************************/

node *
PrintWLproj (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLproj");

    INDENT
    fprintf (outfile, "(%d -> %d), step[%d] %d\n", WLPROJ_BOUND1 (arg_node),
             WLPROJ_BOUND2 (arg_node), WLPROJ_DIM (arg_node), WLPROJ_STEP (arg_node));

    indent++;
    WLPROJ_INNER (arg_node) = Trav (WLPROJ_INNER (arg_node), arg_info);
    indent--;

    if (WLPROJ_NEXT (arg_node) != NULL) {
        WLPROJ_NEXT (arg_node) = Trav (WLPROJ_NEXT (arg_node), arg_info);
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
 ******************************************************************************/

node *
PrintWLgrid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLgrid");

    INDENT
    fprintf (outfile, "(%d -> %d): ", WLGRID_OFFSET (arg_node),
             WLGRID_OFFSET (arg_node) + WLGRID_WIDTH (arg_node));

    indent++;
    if (WLGRID_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        WLGRID_NEXTDIM (arg_node) = Trav (WLGRID_NEXTDIM (arg_node), arg_info);
    } else {
        DBUG_ASSERT ((WLGRID_CODE (arg_node) != NULL), "WLGRID_CODE not found");
        fprintf (outfile, "op_%d\n", NCODE_NO (WLGRID_CODE (arg_node)));
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
 *   node *Print(node *arg_node)
 *
 * description:
 *   initiates print of (sub-)tree
 *
 ******************************************************************************/

node *
Print (node *arg_node)
{
    DBUG_ENTER ("Print");

    act_tab = print_tab;
    mod_name_con = mod_name_con_1;
    indent = 0;

    if ((linkstyle > 1) && (break_after == PH_final)) {
        print_separate = 1;
        arg_node = Trav (arg_node, NULL);

        /*
         * If a full compilation is performed, the object of the compilation
         * is a module/class implementation and the link style is 1 or 2,
         * then all functions are printed into separate files into the
         * tmp directory tmp_dirname.
         * In this case no C-file is generated in the current directory.
         */
    } else {
        print_separate = 0;

        if (break_after < PH_genccode) {
            outfile = stdout;
            fprintf (outfile, "\n-----------------------------------------------\n");
        } else {
            if (linkstyle == 1) {
                /*
                 * The current file is a module/class implementation, but the functions
                 * are nevertheless not compiled separatly to the archive.
                 * Therefore, the C file is generated within the temprorary directory.
                 */

                outfile = WriteOpen ("%s/%s", tmp_dirname, cfilename);
            } else {
                /*
                 * The current file is a SAC program.
                 * Therefore, the C file is generated within the target directory.
                 */

                outfile = WriteOpen ("%s%s", targetdir, cfilename);
            }

            NOTE (("Writing file \"%s%s\"", targetdir, cfilename));
        }

        PrintFileHeader ();
        PFprintInitGlobals ();

        arg_node = Trav (arg_node, NULL);

        if (outfile == stdout) {
            fprintf (outfile, "\n------------------------------------------------\n");
        } else {
            fclose (outfile);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   void PrintNodeTree(node *node)
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
    int i, j;
    ids *_ids;

    outfile = stdout;

    if (node) {
        /* print node name */
        fprintf (outfile, "%s  ", mdb_nodetype[NODE_TYPE (node)]);

        /* print additional information to nodes */
        switch (NODE_TYPE (node)) {
        case N_let:
            _ids = LET_IDS (node);
            fprintf (outfile, "(");
            while (_ids) {
                fprintf (outfile, "%s ", IDS_NAME (_ids));
                _ids = IDS_NEXT (_ids);
            }
            fprintf (outfile, ")\n");
            break;
        case N_id:
            fprintf (outfile, "(%s)\n", ID_NAME (node));
            break;
        case N_num:
            fprintf (outfile, "(%i)\n", NUM_VAL (node));
            break;
        case N_prf:
            fprintf (outfile, "(%s)\n", mdb_prf[PRF_PRF (node)]);
            break;
        case N_vardec:
            fprintf (outfile, "(%s %s)\n", mdb_type[VARDEC_TYPE (node)->simpletype],
                     VARDEC_NAME (node));
            break;
        case N_fundef:
            fprintf (outfile, "(%s)\n", FUNDEF_NAME (node));
            break;
        default:
            fprintf (outfile, "\n");
        }

        indent++;
        for (i = 0; i < nnode[NODE_TYPE (node)]; i++)
            if (node->node[i]) {
                for (j = 0; j < indent; j++)
                    if (j % 4)
                        fprintf (outfile, "  ");
                    else
                        fprintf (outfile, "| ");

                fprintf (outfile, "%i-", i);
                PrintNodeTree (node->node[i]);
            }
        indent--;
    } else
        fprintf (outfile, "NULL\n");
}
