/*
 * $Log$
 * Revision 1.1  2001/05/15 15:41:11  nmw
 * Initial revision
 *
 *
 * created from WLI.h, Revision 3.2  on 2001/05/15 by nmw
 */

#ifndef _SSAWLI_h
#define _SSAWLI_h

extern node *SSAWLIfundef (node *, node *);
extern node *SSAWLIid (node *, node *);
extern node *SSAWLIassign (node *, node *);
extern node *SSAWLIcond (node *, node *);
extern node *SSAWLINwith (node *, node *);
extern node *SSAWLIlet (node *, node *);
extern node *SSAWLIap (node *, node *);

extern node *SSAWLINwithop (node *, node *);
extern node *SSAWLINpart (node *, node *);
extern node *SSAWLINcode (node *, node *);

extern int ssawli_phase;

#endif
