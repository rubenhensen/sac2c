/*
 *
 * $Log$
 * Revision 1.4  2004/11/22 18:33:19  ktr
 * SACDevCamp 04 Ismop
 *
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

#ifndef _SAC_SSAWLUNROLL_H_
#define _SAC_SSAWLUNROLL_H_

#include "types."

/*****************************************************************************
 *
 * SSAWLUnroll
 *
 * prefix: WLU
 *
 *****************************************************************************/
extern int WLUcheckUnrollModarray (node *wln);
extern node *WLUdoUnrollModarray (node *wln, info *arg_info);

extern int WLUcheckUnrollGenarray (node *wln, info *arg_info);
extern node *WLUdoUnrollGenarray (node *wln, info *arg_info);

extern int WLUcheckUnrollFold (node *wln);
extern node *WLUdoUnrollFold (node *wln, info *arg_info);

#endif /* _SAC_SSAWLUNROLL_H_ */
