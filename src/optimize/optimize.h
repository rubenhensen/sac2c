/*
 *
 * $Log$
 * Revision 3.18  2005/09/04 12:52:11  ktr
 * re-engineered the optimization cycle
 *
 * Revision 3.17  2005/08/20 12:06:50  ktr
 * added TypeConvElimination
 *
 * Revision 3.16  2005/02/03 18:28:22  mwe
 * counter added
 *
 * Revision 3.15  2005/02/02 18:09:50  mwe
 * new counter added
 * signature simplification added
 *
 * Revision 3.14  2005/01/27 18:20:30  mwe
 * new counter for type_upgrade added
 *
 * Revision 3.13  2004/12/09 10:59:30  mwe
 * support for type_upgrade added
 *
 * Revision 3.12  2004/11/26 17:54:18  skt
 * renamed OPTmodul into OPTmodule
 *
 * Revision 3.11  2004/11/22 18:10:19  sbs
 * SacDevCamp04
 *
 * Revision 3.10  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 3.9  2004/06/30 12:13:39  khf
 * wlpg_expr removed
 *
 * Revision 3.8  2004/04/08 08:09:55  khf
 * support for wlfs and wlpg added but are currently
 * deactivated in global.c
 *
 * Revision 3.7  2004/03/02 16:49:49  mwe
 * support for CVP added
 *
 * Revision 3.6  2003/08/16 08:44:25  ktr
 * SelectionPropagation added. Must currently be activated with -dosp.
 *
 * Revision 3.5  2003/02/08 16:00:01  mwe
 * dl_expr added
 *
 * Revision 3.4  2002/06/07 17:16:08  mwe
 * al_expr added
 *
 * Revision 3.3  2002/03/13 16:03:20  ktr
 * wls_expr declared
 *
 * Revision 3.2  2001/04/19 16:34:14  nmw
 * statistics for wlir added
 *
 * Revision 3.1  2000/11/20 18:00:45  sacbase
 * new release made
 *
 *
 * ... [eliminated] ...
 *
 * Revision 1.1  1994/12/09  10:48:23  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_OPTIMIZE_H_
#define _SAC_OPTIMIZE_H_

#include "types.h"

extern node *OPTdoOptimize (node *arg_node);
extern node *OPTdoIntraFunctionalOptimizations (node *arg_node);

#endif /* _SAC_OPTIMIZE_H_ */
