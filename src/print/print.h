#ifndef _sac_print_h

#define _sac_print_h

extern node *PrintAssign (node *, node *);
extern node *PrintBlock (node *, node *);
extern node *PrintLet (node *, node *);
extern node *PrintFundef (node *, node *);
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

extern node *Print (node *);

#endif /* _sac_print_h */
