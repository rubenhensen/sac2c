/*
 *
 * $Log$
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
extern node *SSADoUnrollModarray (node *wln, node *arg_info);

extern int SSACheckUnrollGenarray (node *wln, node *arg_info);
extern node *SSADoUnrollGenarray (node *wln, node *arg_info);

extern int SSACheckUnrollFold (node *wln);
extern node *SSADoUnrollFold (node *wln, node *arg_info);

#endif /* _SSAWLUnroll_h_ */
