/*
 *
 * $Log$
 * Revision 1.200  1998/04/24 12:11:48  dkr
 * changed PrintSPMD
 *
 * Revision 1.199  1998/04/24 01:13:09  dkr
 * added PrintSync
 *
 * Revision 1.198  1998/04/21 13:31:06  dkr
 * NWITH2_SEG renamed to NWITH2_SEGS
 *
 * Revision 1.197  1998/04/20 14:21:23  sbs
 * inserted some comments in PrintIcm that relate to the usage of ICM_NEXT!
 *
 * Revision 1.196  1998/04/20 02:37:26  dkr
 * changed PrintNodeTree
 *
 * Revision 1.195  1998/04/19 13:17:02  dkr
 * changed output for N_icm in PrintNodeTree
 *
 * Revision 1.194  1998/04/17 19:15:29  dkr
 * removed trigraph
 *
 * Revision 1.193  1998/04/17 19:04:23  dkr
 * changed output of PrintWLseg, PrintNwith2
 *
 * Revision 1.192  1998/04/17 17:27:46  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.191  1998/04/17 11:02:06  srs
 * changec N_Ncode output of PrintNodeTree()
 *
 * Revision 1.190  1998/04/16 22:50:11  dkr
 * changed PrintNwith2:
 *   NWITH2_SEGS can have the value NULL
 *
 * Revision 1.189  1998/04/16 19:01:05  dkr
 * incremented indent level for N_Nwith, N_Nwith2
 * access macros used
 *
 * Revision 1.188  1998/04/16 15:42:46  dkr
 * access macros used
 *
 * Revision 1.187  1998/04/16 11:41:53  srs
 * fixed bug which resulted from printing the return node of the old WLs.
 * inserted new INFO* macros and renamed the already used ones.
 *
 * Revision 1.186  1998/04/09 20:38:10  dkr
 * changed output in PrintNodeTree
 *
 * Revision 1.185  1998/04/09 09:41:55  dkr
 * changed output in PrintConc
 *
 * Revision 1.184  1998/04/07 17:31:47  dkr
 * PrintNcode:
 *    CODE_CEXPR could be NULL (compile)
 *
 * Revision 1.183  1998/04/07 13:08:27  dkr
 * PrintNodeTree:
 *   added NCODE_USED info for N_part nodes
 *
 * Revision 1.182  1998/04/07 11:32:37  dkr
 * now more access macros are used
 *
 * Revision 1.181  1998/04/04 20:33:54  dkr
 * added an INDENT in PrintIcm
 *
 * Revision 1.180  1998/04/03 19:43:19  dkr
 * changed output for N_conc
 *
 * Revision 1.179  1998/04/02 18:12:01  dkr
 * added PrintConc
 *
 * Revision 1.178  1998/04/01 23:56:46  dkr
 * added PrintWLstriVar, PrintWLgridVar
 *
 * Revision 1.177  1998/04/01 19:17:12  dkr
 * changed output for N_Nwith2...
 *
 * Revision 1.176  1998/04/01 17:22:15  srs
 * changed DBUG output of PrintNwith
 *
 * Revision 1.175  1998/03/27 18:39:28  dkr
 * PrintWLproj -> PrintWLstride
 *
 * Revision 1.174  1998/03/26 18:37:41  srs
 * modified WLI DBUG output in PrintNwith
 *
 * Revision 1.173  1998/03/26 14:01:35  dkr
 * PrintNodeTree now supports the WL... nodes
 *
 * Revision 1.172  1998/03/25 15:01:34  dkr
 * PrintNodeTree:
 *    shows now NCODE_USED
 *
 * Revision 1.171  1998/03/24 15:30:29  cg
 * #include "profile.h" removed since file no longer exists.
 *
 * Revision 1.170  1998/03/24 13:42:01  cg
 * The generation of C code for the declaration and initialization of
 * the runtime system is extracted and moved to gen_startup_code.c
 *
 * Revision 1.169  1998/03/22 15:47:47  dkr
 * N_WLblock: BLOCKING -> STEP
 *
 * Revision 1.168  1998/03/22 15:32:48  dkr
 * N_WLproj: OFFSET, WIDTH -> BOUND1, BOUND2
 *
 * Revision 1.167  1998/03/21 14:16:44  dkr
 * changed output in PrintWLublock
 *
 * Revision 1.165  1998/03/21 13:23:07  dkr
 * changed output in PrintWLproj:
 *   show WLPROJ_LEVEL
 *
 * Revision 1.164  1998/03/20 17:38:45  srs
 * added DBUG information to PrintNwith()
 *
 * Revision 1.163  1998/03/20 17:23:53  dkr
 * in N_WL... nodes: INNER is now called CONTENTS
 *
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
 * - node[0]: is used for storing the current fundef node.
 * - node[1]: profile macros  (?)
 * - node[2]: determines which syntax of the new WLs is printed. If it's
 *   NULL then the intermal syntax is uses which allows to state more than
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
#include "optimize.h"
#include "filemgr.h"
#include "globals.h"
#include "gen_startup_code.h"

/******************************************************************************/

int indent = 0;

static int print_separate = 0;

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
#define ICM_END(prf)
#include "icm.data"
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
#undef ICM_ALL

/******************************************************************************/

#define PRF_IF(n, s, x, y) x

char *prf_string[] = {
#include "prf_node_info.mac"
};

#undef PRF_IF

/******************************************************************************/

void
PrintIds (ids *arg)
{
    DBUG_ENTER ("PrintIds");

    do {
        DBUG_PRINT ("PRINT", ("%s", IDS_NAME (arg)));

        if (IDS_MOD (arg) != NULL) {
            fprintf (outfile, "%s:", IDS_MOD (arg));
        }
        fprintf (outfile, "%s", IDS_NAME (arg));
        if ((IDS_REFCNT (arg) != -1) && show_refcnt) {
            fprintf (outfile, ":%d", IDS_REFCNT (arg));
        }
        if (show_idx && IDS_USE (arg)) {
            Trav (IDS_USE (arg), NULL);
        }
        if (NULL != IDS_NEXT (arg)) {
            fprintf (outfile, ", ");
        }
        arg = IDS_NEXT (arg);
    } while (NULL != arg);

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

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - assign : %s\n",
                                   mdb_nodetype[NODE_TYPE (ASSIGN_INSTR (arg_node))]);
                  PrintMasks (arg_node, arg_info););

    if (N_icm == NODE_TYPE (ASSIGN_INSTR (arg_node))) {
        PrintIcm (ASSIGN_INSTR (arg_node), arg_info);
        fprintf (outfile, "\n");
        if (ASSIGN_NEXT (arg_node))
            Trav (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        DBUG_EXECUTE ("LINE", fprintf (outfile, "/*%03d*/", arg_node->lineno););

        if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_return)
            || (RETURN_EXPRS (ASSIGN_INSTR (arg_node)) != NULL)) {
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

    if (print_separate) {
        outfile = WriteOpen ("%s/header.h", tmp_dirname);
        GSCPrintFileHeader ();

        if (NULL != MODUL_TYPES (arg_node)) {
            fprintf (outfile, "\n\n");
            Trav (MODUL_TYPES (arg_node), arg_info); /* print typedefs */
        }

        if (NULL != MODUL_FUNS (arg_node)) {
            fprintf (outfile, "\n\n");
            Trav (MODUL_FUNS (arg_node), arg_node); /* print function declarations */
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

        if (NULL != MODUL_OBJS (arg_node)) {
            fprintf (outfile, "\n\n");
            print_objdef_for_header_file = 0;
            Trav (MODUL_OBJS (arg_node), arg_info); /* print object definitions */
        }

        fclose (outfile);

        if (NULL != MODUL_FUNS (arg_node)) {
            fprintf (outfile, "\n\n");
            Trav (MODUL_FUNS (arg_node), NULL); /* print function definitions */
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
        fprintf (outfile, "%s%s", TYPEDEF_MOD (arg_node), mod_name_con);
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
            fprintf (outfile, "%s%s", OBJDEF_MOD (arg_node), mod_name_con);
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

    if (0 != FUNDEF_INLINE (arg_node)) {
        fprintf (outfile, "inline ");
    }

    fprintf (outfile, "%s ", Type2String (FUNDEF_TYPES (arg_node), 0));

    if (FUNDEF_MOD (arg_node) != NULL) {
        fprintf (outfile, "%s%s", FUNDEF_MOD (arg_node), mod_name_con);
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
 *  arg_info is used as flag :
 *  arg_info == NULL: print function definitions (with body)
 *  arg_info != NULL: print function declarations (without body)
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

    DBUG_PRINT ("PRINT", ("%s " P_FORMAT, mdb_nodetype[NODE_TYPE (arg_node)], arg_node));

    new_info = MakeInfo ();
    new_info->varno = FUNDEF_VARNO (arg_node);
    /*
     * needed for the introduction of PROFILE_... MACROS in the
     *  function body.
     */
    INFO_PRINT_FUNDEF (new_info) = arg_node;
    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - function\n");
                  PrintMasks (arg_node, new_info););

    if (arg_info == NULL) {
        /*
         * print function definition
         */

        if (FUNDEF_BODY (arg_node)) {
            if (print_separate) {
                outfile = WriteOpen ("%s/fun%d.c", tmp_dirname, function_counter);

                fprintf (outfile, "#include \"header.h\"\n");
            }

            fprintf (outfile, "\n");

            if (FUNDEF_ICM (arg_node) && (N_icm == NODE_TYPE (FUNDEF_ICM (arg_node)))) {
                Trav (FUNDEF_RETURN (arg_node), new_info); /* print N_icm ND_FUN_DEC */
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
         * print function declaration
         */

        if ((FUNDEF_BODY (arg_node) == NULL)
            || ((NULL != FUNDEF_RETURN (arg_node))
                && (N_icm == NODE_TYPE (FUNDEF_RETURN (arg_node)))
                && (strcmp (FUNDEF_NAME (arg_node), "main") != 0))) {
            fprintf (outfile, "extern ");

            if ((NULL != FUNDEF_RETURN (arg_node))
                && (N_icm == NODE_TYPE (FUNDEF_RETURN (arg_node)))) {
                Trav (FUNDEF_RETURN (arg_node), new_info); /* print N_icm ND_FUN_DEC */
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
    case F_take:
    case F_drop:
    case F_psi:
    case F_shape:
    case F_reshape:
    case F_cat:
    case F_dim:
    case F_rotate:
    case F_not:
    case F_ftoi:
    case F_ftoi_A:
    case F_ftod:
    case F_ftod_A:
    case F_itof:
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

    if ((0 == show_refcnt) || (-1 == ID_REFCNT (arg_node))) {
        fprintf (outfile, "%s", ID_NAME (arg_node));
    } else {
        fprintf (outfile, "%s:%d", ID_NAME (arg_node), ID_REFCNT (arg_node));
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
        if ((NODE_TYPE (arg_info) = N_info) && (INFO_PRINT_FUNDEF (arg_info) != NULL)
            && (strcmp (FUNDEF_NAME (INFO_PRINT_FUNDEF (arg_info)), "main") == 0)
            && (compiler_phase == PH_genccode)) {
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
        fprintf (outfile, "%s%s", AP_MOD (arg_node), mod_name_con);
    }
    fprintf (outfile, "%s(", AP_NAME (arg_node));
    if (AP_ARGS (arg_node)) {
        Trav (AP_ARGS (arg_node), arg_info);
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

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**Number %d -> ", ARG_VARNO (arg_node)););

    fprintf (outfile, "%s",
             Type2String (ARG_TYPE (arg_node), (arg_info == NULL) ? 0 : 1));

    if ((1 == show_refcnt) && (-1 != ARG_REFCNT (arg_node))) {
        fprintf (outfile, ":%d", ARG_REFCNT (arg_node));
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

    DBUG_EXECUTE ("MASK", fprintf (outfile, "**Number %d -> ", VARDEC_VARNO (arg_node)););

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

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - do body\n");
                  PrintMasks (DO_BODY (arg_node), arg_info););

    fprintf (outfile, "do\n");
    if (NULL != DO_BODY (arg_node)) {
        indent++;
        Trav (DO_BODY (arg_node), arg_info); /* traverse body of loop */
        indent--;
    }

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (DO_MASK (arg_node, 1), VARNO);
                  fprintf (outfile, "**Used Variables (do-cnd) : %s\n", text);
                  FREE (text););

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

    DBUG_EXECUTE ("MASK", char *text; text = PrintMask (WHILE_MASK (arg_node, 1), VARNO);
                  fprintf (outfile, "**Used Variables (while-cnd) : %s\n", text);
                  FREE (text););

    fprintf (outfile, "while( ");
    Trav (WHILE_COND (arg_node), arg_info);
    fprintf (outfile, " )\n");

    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - while body\n");
                  PrintMasks (WHILE_BODY (arg_node), arg_info););

    Trav (WHILE_BODY (arg_node), arg_info); /* traverse body of loop */

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

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

/******************************************************************************/

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

    fprintf (outfile, "[ ");
    Trav (ARRAY_AELEMS (arg_node), arg_info);
    fprintf (outfile, " ]");

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

/******************************************************************************/

node *
PrintIcm (node *arg_node, node *arg_info)
{
    int compiled_icm = 0;
    DBUG_ENTER ("PrintIcm");
    DBUG_PRINT ("PRINT", ("icm-node %s\n", ICM_NAME (arg_node)));

    INDENT
    if (show_icm == 0)
#define ICM_ALL
#define ICM_DEF(prf, trf)                                                                \
    if (strcmp (ICM_NAME (arg_node), #prf) == 0) {                                       \
        Print##prf (ICM_ARGS (arg_node), arg_info);                                      \
        compiled_icm = 1;                                                                \
    } else
#define ICM_STR(name)
#define ICM_INT(name)
#define ICM_VAR(dim, name)
#define ICM_END(prf)
#include "icm.data"
#undef ICM_ALL
#undef ICM_DEF
#undef ICM_STR
#undef ICM_INT
#undef ICM_VAR
#undef ICM_END
        if (strcmp (ICM_NAME (arg_node), "NOOP") == 0) {
            compiled_icm = 1;
        }

    if ((show_icm == 1) || (compiled_icm == 0)) {
        if ((strcmp (ICM_NAME (arg_node), "ND_FUN_RET") == 0)
            && (strcmp (FUNDEF_NAME (INFO_PRINT_FUNDEF (arg_info)), "main") == 0)
            && (compiler_phase == PH_genccode)) {
            GSCPrintMainEnd ();
            INDENT;
        }

        fprintf (outfile, "%s(", ICM_NAME (arg_node));
        if (NULL != ICM_ARGS (arg_node)) {
            Trav (ICM_ARGS (arg_node), arg_info);
        }
        fprintf (outfile, ")");
    }

    if (NULL != ICM_NEXT (arg_node)) {
        if ((1 == show_icm) || (0 == compiled_icm)) {
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

    fprintf (outfile, "\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintSPMD (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintSPMD");

    fprintf (outfile, "/*** begin of SPMD region ***\n");
    INDENT;
    fprintf (outfile, " ***\n");
    INDENT;
    fprintf (outfile, " *** in:    ");

    if (SPMD_IN (arg_node) != NULL) {
        SPMD_IN (arg_node) = Trav (SPMD_IN (arg_node), arg_info);
    }

    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, " *** out:   ");

    if (SPMD_OUT (arg_node) != NULL) {
        SPMD_OUT (arg_node) = Trav (SPMD_OUT (arg_node), arg_info);
    }

    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, " *** inout: ");

    if (SPMD_INOUT (arg_node) != NULL) {
        SPMD_INOUT (arg_node) = Trav (SPMD_INOUT (arg_node), arg_info);
    }

    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, " *** local: ");

    if (SPMD_LOCAL (arg_node) != NULL) {
        SPMD_LOCAL (arg_node) = Trav (SPMD_LOCAL (arg_node), arg_info);
    }

    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, " ***/\n");

    indent++;
    SPMD_REGION (arg_node) = Trav (SPMD_REGION (arg_node), arg_info);
    indent--;

    INDENT
    fprintf (outfile, "/*** end of SPMD region ***/");

    DBUG_RETURN (arg_node);
}

/******************************************************************************/

node *
PrintSync (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintSync");

    fprintf (outfile, "/*** begin of sync region ***\n");
    INDENT;
    fprintf (outfile, " ***\n");
    INDENT;
    fprintf (outfile, " *** inout: ");

    if (SYNC_INOUT (arg_node) != NULL) {
        SYNC_INOUT (arg_node) = Trav (SYNC_INOUT (arg_node), arg_info);
    }

    fprintf (outfile, "\n");
    INDENT;
    fprintf (outfile, " ***/\n");

    indent++;
    SYNC_REGION (arg_node) = Trav (SYNC_REGION (arg_node), arg_info);
    indent--;

    INDENT
    fprintf (outfile, "/*** end of sync region ***/");

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
    node *buffer;

    DBUG_ENTER ("PrintNwith");

    DBUG_ASSERT (arg_info, "arg_info is NULL");
    buffer = INFO_PRINT_INT_SYN (arg_info);

    DBUG_EXECUTE ("WLI",
                  fprintf (outfile,
                           "\n** WLI N_Nwith : "
                           "(PARTS %d, REF %d(%d), CPLX %d, FOLDABLE %d, NO_CHANCE %d)\n",
                           NWITH_PARTS (arg_node), NWITH_REFERENCED (arg_node),
                           NWITH_REFERENCED_FOLD (arg_node), NWITH_COMPLEX (arg_node),
                           NWITH_FOLDABLE (arg_node), NWITH_NO_CHANCE (arg_node)););

    indent++;

    /*
     * check wether to use output format 1 (multiple NParts)
     * or 2 (only one NPart) and use INFO_PRINT_INT_SYN(arg_info)
     * as flag for traversal.
     */
    if (NPART_NEXT (NWITH_PART (arg_node)) != NULL) {
        /* output format 1 */
        INFO_PRINT_INT_SYN (arg_info) = NULL;
        fprintf (outfile, "new_with\n");
        indent++;
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        indent--;
    } else {
        /* output format 2 */
        INFO_PRINT_INT_SYN (arg_info) = (node *)!NULL; /* set != NULL */
        fprintf (outfile, "new_with ");
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    if (NPART_NEXT (NWITH_PART (arg_node)) == NULL) {
        /*
         * output format 2: now we have in
         * INFO_PRINT_INT_SYN(arg_info) the last expr.
         */
        fprintf (outfile, ", ");
        Trav (INFO_PRINT_INT_SYN (arg_info), arg_info);
    }
    fprintf (outfile, ")");

    indent--;

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
    DBUG_EXECUTE ("MASK", fprintf (outfile, "\n**MASKS - Npart\n");
                  PrintMasks (arg_node, arg_info););

    /* print generator */
    if (INFO_PRINT_INT_SYN (arg_info) == NULL) {
        INDENT; /* each gen in a new line. */
    }
    PrintNgenerator (NPART_GEN (arg_node), NPART_WITHID (arg_node), arg_info);

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

    indent++;

    fprintf (outfile, "new_with2 (");
    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    fprintf (outfile, ")\n");

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

    indent--;

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
        fprintf (outfile, "/********** segment %d: **********/\n", i++);
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
 ******************************************************************************/

node *
PrintWLstriVar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLstriVar");

    INDENT
    fprintf (outfile, "(");
    PrintWLvar (WLSTRIVAR_BOUND1 (arg_node), WLSTRIVAR_DIM (arg_node));
    fprintf (outfile, " -> ");
    PrintWLvar (WLSTRIVAR_BOUND2 (arg_node), WLSTRIVAR_DIM (arg_node));
    fprintf (outfile, "), step[%d] ", WLSTRIVAR_DIM (arg_node));
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
 *   node *PrintWLgridVar(node *arg_node, node *arg_info)
 *
 * description:
 *   prints N_WLgrid-nodes
 *
 ******************************************************************************/

node *
PrintWLgridVar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintWLgridVar");

    INDENT
    fprintf (outfile, "(");
    PrintWLvar (WLGRIDVAR_BOUND1 (arg_node), WLGRIDVAR_DIM (arg_node));
    fprintf (outfile, " -> ");
    PrintWLvar (WLGRIDVAR_BOUND2 (arg_node), WLGRIDVAR_DIM (arg_node));
    fprintf (outfile, "): ");

    indent++;
    if (WLGRIDVAR_NEXTDIM (arg_node) != NULL) {
        fprintf (outfile, "\n");
        WLGRIDVAR_NEXTDIM (arg_node) = Trav (WLGRIDVAR_NEXTDIM (arg_node), arg_info);
    } else {
        DBUG_ASSERT ((WLGRIDVAR_CODE (arg_node) != NULL), "WLGRIDVAR_CODE not found");
        fprintf (outfile, "op_%d\n", NCODE_NO (WLGRIDVAR_CODE (arg_node)));
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
    DBUG_ENTER ("Print");

    act_tab = print_tab;
    mod_name_con = mod_name_con_1;
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
            GSCPrintFileHeader ();
            syntax_tree = Trav (syntax_tree, NULL);
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
            GSCPrintFileHeader ();
            syntax_tree = Trav (syntax_tree, NULL);
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
            print_separate = 1;
            syntax_tree = Trav (syntax_tree, NULL);
            break;
        }
    }

    else {
        outfile = stdout;
        fprintf (outfile, "\n-----------------------------------------------\n");
        syntax_tree = Trav (syntax_tree, NULL);
        fprintf (outfile, "\n-----------------------------------------------\n");
    }

    DBUG_RETURN (syntax_tree);
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
                fprintf (outfile, "%s", IDS_NAME (_ids));
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
            fprintf (outfile, "(%s %s)\n", mdb_type[TYPES_BASETYPE (VARDEC_TYPE (node))],
                     VARDEC_NAME (node));
            break;
        case N_fundef:
            fprintf (outfile, "(%s)\n", FUNDEF_NAME (node));
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
            fprintf (outfile, "(%d->%d [%d])\n", WLUBLOCK_BOUND1 (node),
                     WLUBLOCK_BOUND2 (node), WLUBLOCK_DIM (node));
            break;
        case N_WLstriVar:
            fprintf (outfile, "(");
            PrintWLvar (WLSTRIVAR_BOUND1 (node), WLSTRIVAR_DIM (node));
            fprintf (outfile, "->");
            PrintWLvar (WLSTRIVAR_BOUND2 (node), WLSTRIVAR_DIM (node));
            fprintf (outfile, ", step[%d] ", WLSTRIVAR_DIM (node));
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
                for (j = 0; j < indent; j++) {
                    if (j % 4) {
                        fprintf (outfile, "  ");
                    } else {
                        fprintf (outfile, "| ");
                    }
                }

                fprintf (outfile, "%i-", i);
                PrintNodeTree (node->node[i]);
            }
        indent--;
    } else
        fprintf (outfile, "NULL\n");
}
