/*
 *
 * $Log$
 * Revision 3.8  2001/02/12 15:57:34  nmw
 * Print functions for N_cseinfo, N_ssacnt and N_avis added
 *
 * Revision 3.7  2001/02/06 01:21:35  dkr
 * macros INDENT_STR and DO_INDENT added
 *
 * Revision 3.6  2001/01/29 16:07:10  dkr
 * PrintWLstrideVar and PrintWLstride replaced by PrintWLstridex
 * PrintWLgridVar and PrintWLgrid replaced by PrintWLgridx
 *
 * Revision 3.5  2001/01/24 23:41:20  dkr
 * PrintWLseg and PrintWLsegVar replaced by PrintWLsegx
 * PrintWLblock and PrintWLublock replaced by PrintWLxblock
 *
 * Revision 3.4  2001/01/09 17:22:13  dkr
 * PrintWLstriVar renamed into PrintWLstrideVar
 *
 * Revision 3.3  2000/12/12 12:16:54  dkr
 * nodes N_pre, N_post, N_inc, N_dec removed
 *
 * Revision 3.2  2000/12/06 11:43:11  dkr
 * PrintTravPre, PrintTravPost added
 *
 * Revision 3.1  2000/11/20 17:59:47  sacbase
 * new release made
 *
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
 * [ eliminated ]
 *
 */

#ifndef _sac_print_h
#define _sac_print_h

#include "tree.h"
#include "globals.h"

#define INDENT_STR "  "

#define DO_INDENT(cnt)                                                                   \
    {                                                                                    \
        int j;                                                                           \
        for (j = 0; j < cnt; j++) {                                                      \
            fprintf (outfile, INDENT_STR);                                               \
        }                                                                                \
    }

#define INDENT DO_INDENT (indent)

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

extern node *PrintModul (node *arg_node, node *arg_info);
extern node *PrintImplist (node *arg_node, node *arg_info);
extern node *PrintTypedef (node *arg_node, node *arg_info);
extern node *PrintObjdef (node *arg_node, node *arg_info);
extern node *PrintFundef (node *arg_node, node *arg_info);
extern node *PrintAnnotate (node *arg_node, node *arg_info);
extern node *PrintArg (node *arg_node, node *arg_info);
extern node *PrintVardec (node *arg_node, node *arg_info);
extern node *PrintBlock (node *arg_node, node *arg_info);
extern node *PrintReturn (node *arg_node, node *arg_info);
extern node *PrintAssign (node *arg_node, node *arg_info);
extern node *PrintDo (node *arg_node, node *arg_info);
extern node *PrintWhile (node *arg_node, node *arg_info);
extern node *PrintCond (node *arg_node, node *arg_info);
extern node *PrintCast (node *arg_node, node *arg_info);
extern node *PrintLet (node *arg_node, node *arg_info);
extern node *PrintPrf (node *arg_node, node *arg_info);
extern node *PrintAp (node *arg_node, node *arg_info);
extern node *PrintEmpty (node *arg_node, node *arg_info);
extern node *PrintArray (node *arg_node, node *arg_info);
extern node *PrintExprs (node *arg_node, node *arg_info);
extern node *PrintId (node *arg_node, node *arg_info);
extern node *PrintNum (node *arg_node, node *arg_info);
extern node *PrintFloat (node *arg_node, node *arg_info);
extern node *PrintDouble (node *arg_node, node *arg_info);
extern node *PrintBool (node *arg_node, node *arg_info);
extern node *PrintStr (node *arg_node, node *arg_info);
extern node *PrintChar (node *arg_node, node *arg_info);
extern node *PrintVectInfo (node *arg_node, node *arg_info);
extern node *PrintIcm (node *arg_node, node *arg_info);
extern node *PrintPragma (node *arg_node, node *arg_info);
extern node *PrintSpmd (node *arg_node, node *arg_info);
extern node *PrintSync (node *arg_node, node *arg_info);
extern node *PrintMT (node *arg_node, node *arg_info);
extern node *PrintST (node *arg_node, node *arg_info);
extern node *PrintMTsignal (node *arg_node, node *arg_info);
extern node *PrintMTsync (node *arg_node, node *arg_info);
extern node *PrintMTalloc (node *arg_node, node *arg_info);
extern node *PrintSSAcnt (node *arg_node, node *arg_info);
extern node *PrintCSEinfo (node *arg_node, node *arg_info);
extern node *PrintAvis (node *arg_node, node *arg_info);

/* new with-loop */
extern node *PrintNwith (node *arg_node, node *arg_info);
extern node *PrintNwithid (node *arg_node, node *arg_info);
extern node *PrintNcode (node *arg_node, node *arg_info);
extern node *PrintNpart (node *arg_node, node *arg_info);
extern node *PrintNgenerator (node *arg_node, node *arg_info);
extern node *PrintNwithop (node *arg_node, node *arg_info);

extern node *PrintNwith2 (node *arg_node, node *arg_info);
extern node *PrintWLsegx (node *arg_node, node *arg_info);
extern node *PrintWLxblock (node *arg_node, node *arg_info);
extern node *PrintWLstridex (node *arg_node, node *arg_info);
extern node *PrintWLgridx (node *arg_node, node *arg_info);

extern node *PrintCWrapper (node *arg_node, node *arg_info);

extern void PrintFunctionHeader (node *arg_node, node *arg_info);

extern node *Print (node *syntax_tree);
extern node *PrintNode (node *node);

extern void PrintAST (node *node);     /* debug output */
extern void PrintNodeAST (node *node); /* debug output */

extern node *PrintTravPre (node *arg_node, node *arg_info);
extern node *PrintTravPost (node *arg_node, node *arg_info);

#endif /* _sac_print_h */
