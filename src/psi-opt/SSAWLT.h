/*
 * $Log$
 * Revision 1.3  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.2  2003/03/12 23:40:25  dkr
 * SSAWLTNwithop added
 *
 * Revision 1.1  2001/05/14 15:55:08  nmw
 * Initial revision
 *
 * created from WLT.h, Revision 3.2 on 2001/05/14 by nmw
 */

#ifndef _SSAWLT_h
#define _SSAWLT_h

extern node *SSAWLTfundef (node *, info *);
extern node *SSAWLTassign (node *, info *);
extern node *SSAWLTcond (node *, info *);
extern node *SSAWLTlet (node *, info *);
extern node *SSAWLTap (node *, info *);

extern node *SSAWLTNwith (node *, info *);
extern node *SSAWLTNwithop (node *, info *);
extern node *SSAWLTNpart (node *, info *);
extern node *SSAWLTNgenerator (node *, info *);
extern node *SSAWLTNcode (node *, info *);

#endif
