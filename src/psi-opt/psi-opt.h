/*
 *
 * $Log$
 * Revision 1.2  1996/02/13 10:15:11  sbs
 * counting of eliminations inserted.
 *
 * Revision 1.1  1995/06/02  10:06:56  sbs
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_psi_opt_h

#define _sac_psi_opt_h

extern int psi_optimize; /* imported from main.c */
extern int psi_opt_ive;  /* imported from main.c */
extern int ive_expr;
extern int ive_op;

extern node *PsiOpt (node *);

#endif /* _sac_psi_opt_h */
