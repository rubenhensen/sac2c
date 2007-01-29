/*
 *
 * $Log$
 * Revision 3.11  2004/11/22 17:59:48  skt
 * code brushing in SACDevCampDK 2004
 *
 * Revision 3.10  2004/11/21 14:22:39  skt
 * uncomment some old stuff to make it run this newast
 *
 * Revision 3.9  2004/09/28 13:22:48  ktr
 * Removed generatemasks.
 *
 * Revision 3.8  2004/08/26 17:01:36  skt
 * moved MUTHDecodeExecmode from multithread to multithread_lib
 *
 * Revision 3.7  2004/08/18 13:24:31  skt
 * switch to mtexecmode_t done
 *
 * Revision 3.6  2004/08/05 17:42:19  skt
 * TagAllocs added
 *
 * Revision 3.5  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 3.4  2004/07/26 16:53:07  skt
 * added support for exclusive cells
 *
 * Revision 3.3  2004/06/08 14:39:11  skt
 * MUTHGetLastExpression added
 *
 * Revision 3.2  2001/05/08 12:27:57  dkr
 * new macros for RC used
 *
 * Revision 3.1  2000/11/20 18:03:12  sacbase
 * new release made
 *
 * Revision 1.6  2000/07/13 08:24:24  jhs
 * Moved DupMask_ InsertBlock, InsertMT and InsertST from blocks_init.[ch]
 * Renamed InsertXX to MUTHInsertXX.
 *
 * Revision 1.5  2000/04/10 15:45:08  jhs
 * Added Reduce
 *
 * Revision 1.4  2000/03/28 09:48:44  jhs
 * Added refcunt-macros
 *
 * Revision 1.3  2000/03/09 18:34:47  jhs
 * Additional features.
 *
 * Revision 1.2  2000/03/02 13:01:02  jhs
 * Added MUTHExchangeApplication,
 * added MUTHExpandFundefName,
 * added DBUG_PRINTS and DBUG_ASSERTS.
 *
 * Revision 1.1  2000/02/21 17:48:22  jhs
 * Initial revision
 *
 */

#ifndef _SAC_MULTITHREAD_LIB_H_
#define _SAC_MULTITHREAD_LIB_H_

#include "types.h"

#define MUTH_SPLITPHASE_ENABLED TRUE

extern node *MUTHLIBexpandFundefName (node *fundef, char *prefix);
extern void MUTHLIBtagAllocs (node *withloop, mtexecmode_t executionmode);
extern char *MUTHLIBdecodeExecmode (mtexecmode_t execmode);

#endif /* _SAC_MULTITHREAD_LIB_H_ */
