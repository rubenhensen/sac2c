/*
 * $Log$
 * Revision 1.1  2001/05/14 15:55:08  nmw
 * Initial revision
 *
 *
 * created from WLT.h, Revision 3.2 on 2001/05/14 by nmw
 */

#ifndef _SSAWLT_h
#define _SSAWLT_h

extern node *SSAWLTfundef (node *, node *);
extern node *SSAWLTassign (node *, node *);
extern node *SSAWLTcond (node *, node *);
extern node *SSAWLTNwith (node *, node *);
extern node *SSAWLTlet (node *, node *);
extern node *SSAWLTap (node *, node *);

extern node *SSAWLTNpart (node *, node *);
extern node *SSAWLTNgenerator (node *, node *);
extern node *SSAWLTNcode (node *, node *);

#endif
