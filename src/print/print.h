/*
 *
 * $Log$
 * Revision 1.4  1994/12/14 10:18:39  sbs
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

extern node *PrintAssign (node *, node *);
extern node *PrintBlock (node *, node *);
extern node *PrintLet (node *, node *);
extern node *PrintFundef (node *, node *);
extern node *PrintTypedef (node *, node *);
extern node *PrintModul (node *, node *);
extern node *PrintImplist (node *, node *);
extern node *PrintPrf (node *, node *);
extern node *PrintId (node *, node *);
extern node *PrintNum (node *, node *);
extern node *PrintFloat (node *, node *);
extern node *PrintBool (node *, node *);
extern node *PrintReturn (node *, node *);
extern node *PrintAp (node *, node *);
extern node *PrintExprs (node *, node *);
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
extern node *PrintArray (node *, node *);
extern node *PrintInc (node *, node *);
extern node *PrintDec (node *, node *);
extern node *PrintPost (node *, node *);
extern node *PrintPre (node *, node *);

extern node *Print (node *);

#endif /* _sac_print_h */
