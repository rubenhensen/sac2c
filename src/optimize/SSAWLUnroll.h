/*
 *
 * $Log$
 * Revision 1.3  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.2  2002/10/09 02:06:25  dkr
 * no changes done
 *
 * Revision 1.1  2002/10/08 22:10:05  dkr
 * Initial revision
 *
 *
 * created from WLUnroll.h, Revision 3.1 on 2002/10/10 by dkr
 *
 */

#ifndef _SSAWLUnroll_h_
#define _SSAWLUnroll_h_

extern int SSACheckUnrollModarray (node *wln);
extern node *SSADoUnrollModarray (node *wln, info *arg_info);

extern int SSACheckUnrollGenarray (node *wln, info *arg_info);
extern node *SSADoUnrollGenarray (node *wln, info *arg_info);

extern int SSACheckUnrollFold (node *wln);
extern node *SSADoUnrollFold (node *wln, info *arg_info);

#endif /* _SSAWLUnroll_h_ */
