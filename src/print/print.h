/*
 *
 * $Log$
 * Revision 2.9  2000/06/23 15:34:13  nmw
 * PrintCWrapper added
 *
 * Revision 2.8  2000/03/22 17:35:42  jhs
 * Added PrintMT(sync|alloc|signal).
 *
 * Revision 2.7  2000/03/21 13:34:30  dkr
 * macro PRINT_VECT added
 *
 * Revision 2.6  2000/03/15 14:54:16  dkr
 * PrintNodeTree renamed to PrintAST
 * PrintNodeAST added
 *
 * Revision 2.5  2000/02/24 15:56:12  dkr
 * Print functions for old with-loop removed
 *
 * Revision 2.4  2000/02/11 18:31:00  dkr
 * PrintNode() added
 *
 * Revision 2.3  2000/02/03 15:20:31  jhs
 * Added PrintMT and PrintSt.
 *
 * Revision 2.2  1999/11/11 18:26:32  dkr
 * PrintNgenerator is now called by Trav only :))
 *
 * Revision 2.1  1999/02/23 12:40:27  sacbase
 * new release made
 *
 * Revision 1.32  1998/08/07 14:38:26  dkr
 * PrintWLsegVar added
 *
 * Revision 1.31  1998/04/26 21:50:18  dkr
 * PrintSPMD renamed to PrintSpmd
 *
 * Revision 1.30  1998/04/25 11:45:56  sbs
 * indent moved into globals!
 *
 * Revision 1.29  1998/04/23 17:32:50  dkr
 * added PrintSync
 *
 * Revision 1.28  1998/04/17 17:27:51  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.27  1998/04/02 17:41:21  dkr
 * added PrintConc
 *
 * Revision 1.26  1998/04/01 23:56:54  dkr
 * added PrintWLstriVar, PrintWLgridVar
 *
 * Revision 1.25  1998/03/27 18:39:38  dkr
 * PrintWLproj -> PrintWLstride
 *
 * Revision 1.24  1998/03/13 16:22:04  dkr
 * new nodes added:
 *   N_WLblock, N_WLublock
 *
 * Revision 1.23  1998/03/03 23:54:21  dkr
 * added PrintNwithid, PrintNwithop, PrintNcode, ...
 * added print-routines for N_Nwith2-nodes
 *
 * Revision 1.22  1997/11/24 16:04:55  srs
 * print routines for N_Nwith node and subnodes
 *
 * Revision 1.21  1997/11/20 13:39:15  srs
 * added PrintNPart and PrintNWith
 *
 * Revision 1.20  1997/11/07 13:32:08  srs
 * removed unused function PrintLeton
 *
 * Revision 1.19  1997/05/14 08:14:41  sbs
 * PrintAnnotate added
 *
 * Revision 1.18  1996/01/05  13:12:55  cg
 * added function PrintStr.
 *
 * Revision 1.17  1995/12/29  10:36:39  cg
 * new functions PrintFoldfun, PrintFoldprf, PrintGenarray, PrintModarray
 *
 * Revision 1.16  1995/12/20  08:18:38  cg
 * added PrintChar
 *
 * Revision 1.15  1995/12/01  17:14:59  cg
 * added function PrintPragma.
 *
 * Revision 1.14  1995/07/11  09:02:10  cg
 * declaration of PrintConstdef deleted.
 *
 * Revision 1.13  1995/07/04  08:38:17  hw
 * PrintDouble inserted
 *
 * Revision 1.12  1995/06/06  14:04:15  cg
 * PrintConstdef and PrintObjdef added.
 *
 * Revision 1.11  1995/06/02  16:53:33  sbs
 * PrintVectInfo inser5ed.
 *
 * Revision 1.10  1995/03/29  12:01:34  hw
 * PrintIcm added
 *
 * Revision 1.9  1995/03/28  12:37:39  hw
 * char *prf_string[] inserted
 *
 * Revision 1.8  1995/03/08  14:40:10  sbs
 * include "tree.h" moved from tree.c to tree.h
 *
 * Revision 1.7  1995/03/08  14:03:00  sbs
 * INDENT & indent exported!
 *
 * Revision 1.6  1995/02/14  12:22:37  sbs
 * PrintFold inserted
 *
 * Revision 1.5  1994/12/15  17:14:03  sbs
 * PrintCast inserted
 *
 * Revision 1.4  1994/12/14  10:18:39  sbs
 * PrintModul & PrintImportlist & PrintTypedef inserted
 *
 * Revision 1.3  1994/11/11  13:50:54  hw
 * added new Print-functions: PrintInc PrintDec PrintPost PrintPre
 *
 * Revision 1.2  1994/11/10  15:35:18  sbs
 * RCS-header inserted
 *
 */

#ifndef _sac_print_h
#define _sac_print_h

#include "tree.h"
#include "globals.h"

#define INDENT                                                                           \
    {                                                                                    \
        int j;                                                                           \
        for (j = 0; j < indent; j++) {                                                   \
            fprintf (outfile, "  ");                                                     \
        }                                                                                \
    }

#define PRINT_VECT(file, vect, dims, format)                                             \
    {                                                                                    \
        int d;                                                                           \
        if (vect != NULL) {                                                              \
            fprintf (file, "[ ");                                                        \
            for (d = 0; d < dims; d++) {                                                 \
                fprintf (file, format, (vect)[d]);                                       \
                fprintf (file, " ");                                                     \
            }                                                                            \
            fprintf (file, "]");                                                         \
        } else {                                                                         \
            fprintf (file, "NULL");                                                      \
        }                                                                                \
    }

extern char *prf_string[];

extern node *PrintAssign (node *arg_node, node *arg_info);
extern node *PrintBlock (node *arg_node, node *arg_info);
extern node *PrintLet (node *arg_node, node *arg_info);
extern node *PrintAnnotate (node *arg_node, node *arg_info);
extern node *PrintFundef (node *arg_node, node *arg_info);
extern node *PrintTypedef (node *arg_node, node *arg_info);
extern node *PrintObjdef (node *arg_node, node *arg_info);
extern node *PrintModul (node *arg_node, node *arg_info);
extern node *PrintImplist (node *arg_node, node *arg_info);
extern node *PrintPrf (node *arg_node, node *arg_info);
extern node *PrintId (node *arg_node, node *arg_info);
extern node *PrintStr (node *arg_node, node *arg_info);
extern node *PrintNum (node *arg_node, node *arg_info);
extern node *PrintChar (node *arg_node, node *arg_info);
extern node *PrintFloat (node *arg_node, node *arg_info);
extern node *PrintDouble (node *arg_node, node *arg_info);
extern node *PrintBool (node *arg_node, node *arg_info);
extern node *PrintReturn (node *arg_node, node *arg_info);
extern node *PrintAp (node *arg_node, node *arg_info);
extern node *PrintExprs (node *arg_node, node *arg_info);
extern node *PrintCast (node *arg_node, node *arg_info);
extern node *PrintAssign (node *arg_node, node *arg_info);
extern node *PrintArg (node *arg_node, node *arg_info);
extern node *PrintVardec (node *arg_node, node *arg_info);
extern node *PrintDo (node *arg_node, node *arg_info);
extern node *PrintWhile (node *arg_node, node *arg_info);
extern node *PrintFor (node *arg_node, node *arg_info);
extern node *PrintEmpty (node *arg_node, node *arg_info);
extern node *PrintCond (node *arg_node, node *arg_info);
extern node *PrintArray (node *arg_node, node *arg_info);
extern node *PrintInc (node *arg_node, node *arg_info);
extern node *PrintDec (node *arg_node, node *arg_info);
extern node *PrintPost (node *arg_node, node *arg_info);
extern node *PrintPre (node *arg_node, node *arg_info);
extern node *PrintIcm (node *arg_node, node *arg_info);
extern node *PrintVectInfo (node *arg_node, node *arg_info);
extern node *PrintPragma (node *arg_node, node *arg_info);
extern node *PrintSpmd (node *arg_node, node *arg_info);
extern node *PrintSync (node *arg_node, node *arg_info);
extern node *PrintMT (node *arg_node, node *arg_info);
extern node *PrintST (node *arg_node, node *arg_info);
extern node *PrintMTsignal (node *arg_node, node *arg_info);
extern node *PrintMTsync (node *arg_node, node *arg_info);
extern node *PrintMTalloc (node *arg_node, node *arg_info);

/* new with-loop */
extern node *PrintNwith (node *arg_node, node *arg_info);
extern node *PrintNwithid (node *arg_node, node *arg_info);
extern node *PrintNcode (node *arg_node, node *arg_info);
extern node *PrintNpart (node *arg_node, node *arg_info);
extern node *PrintNgenerator (node *arg_node, node *arg_info);
extern node *PrintNwithop (node *arg_node, node *arg_info);

extern node *PrintNwith2 (node *arg_node, node *arg_info);
extern node *PrintWLseg (node *arg_node, node *arg_info);
extern node *PrintWLblock (node *arg_node, node *arg_info);
extern node *PrintWLublock (node *arg_node, node *arg_info);
extern node *PrintWLstride (node *arg_node, node *arg_info);
extern node *PrintWLgrid (node *arg_node, node *arg_info);
extern node *PrintWLsegVar (node *arg_node, node *arg_info);
extern node *PrintWLstriVar (node *arg_node, node *arg_info);
extern node *PrintWLgridVar (node *arg_node, node *arg_info);

extern node *PrintCWrapper (node *arg_node, node *arg_info);

extern void PrintFunctionHeader (node *arg_node, node *arg_info);

extern node *Print (node *syntax_tree);
extern node *PrintNode (node *node);

extern void PrintAST (node *node);     /* debug output */
extern void PrintNodeAST (node *node); /* debug output */

#endif /* _sac_print_h */
