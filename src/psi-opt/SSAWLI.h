/*
 * $Log$
 * Revision 1.3  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.2  2004/03/26 13:02:12  khf
 * SSAWLINgenerator added
 *
 * Revision 1.1  2001/05/15 15:41:11  nmw
 * Initial revision
 *
 *
 * created from WLI.h, Revision 3.2  on 2001/05/15 by nmw
 */

#ifndef _SSAWLI_h
#define _SSAWLI_h

extern node *SSAWLIfundef (node *, info *);
extern node *SSAWLIid (node *, info *);
extern node *SSAWLIassign (node *, info *);
extern node *SSAWLIcond (node *, info *);
extern node *SSAWLINwith (node *, info *);
extern node *SSAWLIlet (node *, info *);
extern node *SSAWLIap (node *, info *);

extern node *SSAWLINwithop (node *, info *);
extern node *SSAWLINpart (node *, info *);
extern node *SSAWLINgenerator (node *, info *);
extern node *SSAWLINcode (node *, info *);

extern int ssawli_phase;

#endif
