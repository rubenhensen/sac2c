/*
 *
 * $Log$
 * Revision 1.16  1995/12/20 08:18:38  cg
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
 *
 */

#ifndef _sac_print_h

#define _sac_print_h

#include "tree.h"

extern int indent;

#define INDENT                                                                           \
    {                                                                                    \
        int j;                                                                           \
        for (j = 0; j < indent; j++)                                                     \
            fprintf (outfile, "  ");                                                     \
    }
extern char *prf_string[];

extern node *PrintAssign (node *, node *);
extern node *PrintBlock (node *, node *);
extern node *PrintLet (node *, node *);
extern node *PrintFundef (node *, node *);
extern node *PrintTypedef (node *, node *);
extern node *PrintObjdef (node *, node *);
extern node *PrintModul (node *, node *);
extern node *PrintImplist (node *, node *);
extern node *PrintPrf (node *, node *);
extern node *PrintId (node *, node *);
extern node *PrintNum (node *, node *);
extern node *PrintChar (node *, node *);
extern node *PrintFloat (node *, node *);
extern node *PrintDouble (node *, node *);
extern node *PrintBool (node *, node *);
extern node *PrintReturn (node *, node *);
extern node *PrintAp (node *, node *);
extern node *PrintExprs (node *, node *);
extern node *PrintCast (node *, node *);
extern node *PrintAssign (node *, node *);
extern node *PrintArg (node *, node *);
extern node *PrintVardec (node *, node *);
extern node *PrintDo (node *, node *);
extern node *PrintWhile (node *, node *);
extern node *PrintFor (node *, node *);
extern node *PrintEmpty (node *, node *);
extern node *PrintLeton (node *, node *);
extern node *PrintCond (node *, node *);
extern node *PrintWith (node *, node *);
extern node *PrintGenator (node *, node *);
extern node *PrintConexpr (node *, node *);
extern node *PrintFold (node *, node *);
extern node *PrintArray (node *, node *);
extern node *PrintInc (node *, node *);
extern node *PrintDec (node *, node *);
extern node *PrintPost (node *, node *);
extern node *PrintPre (node *, node *);
extern node *PrintIcm (node *, node *);
extern node *PrintVectInfo (node *, node *);
extern node *PrintPragma (node *, node *);

extern node *Print (node *);

#endif /* _sac_print_h */
