/*
 *
 * $Log$
 * Revision 3.5  2003/03/14 15:41:35  dkr
 * sac_wl.tagged.h added
 *
 * Revision 3.4  2002/10/29 20:29:45  dkr
 * include of stdlib.h removed again...
 *
 * Revision 3.3  2002/10/29 19:07:57  dkr
 * stdlib.h included
 *
 * Revision 3.2  2002/07/03 13:59:16  dkr
 * sac_boundcheck.h renamed into sac_runtimecheck.h
 *
 * Revision 3.1  2000/11/20 18:02:07  sacbase
 * new release made
 *
 * Revision 2.11  2000/08/18 13:56:15  dkr
 * sac_icm.h always included (the cat?-macros are usefull even without
 * TAGGED_ARRAYS)
 *
 * Revision 2.10  2000/08/01 14:02:26  nmw
 * sac_free_interface_handler.h removed
 *
 * Revision 2.9  2000/07/20 12:46:07  dkr
 * include of sac_free_interface_handler.h added
 *
 * Revision 2.8  2000/07/06 08:15:38  dkr
 * include for TAGGED_ARRAYS changed
 *
 * Revision 2.7  2000/07/05 12:51:41  nmw
 * sac_arg.h added
 *
 * Revision 2.6  2000/01/17 16:25:58  cg
 * Exchanged sequence of #include "sac_heapmgr.h" and nclude "sac_mt.h"
 *
 * Revision 2.5  1999/07/08 12:36:33  cg
 * Include of sac_malloc.h replaced by include of mew file sac_heapmgr.h
 *
 * Revision 2.4  1999/06/24 14:01:27  sbs
 * sac_icm.h only included iff TAGGED_ARRAYS.
 *
 * Revision 2.3  1999/06/16 17:36:03  rob
 * Include ifdef to pick different sac_std.h file to ease
 * mental anguish and angry programmers over transition to
 * TAGGED_ARRAYS.
 *
 * Revision 2.2  1999/04/06 13:43:12  cg
 * added include of sac_cachesim.h
 *
 * Revision 2.1  1999/02/23 12:43:43  sacbase
 * new release made
 *
 * Revision 1.6  1998/06/29 08:48:59  cg
 * sequence of including header files modified
 *
 * Revision 1.5  1998/05/07 08:26:08  cg
 * sac_rc.h renamed to sac_std.h
 *
 * Revision 1.4  1998/05/07 08:17:51  cg
 * SAC header files converted to new naming conventions.
 *
 * Revision 1.2  1998/03/24 13:52:05  cg
 * #include "sac_bool"  added
 *
 * Revision 1.1  1998/03/19 16:36:57  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   sac.h
 *
 * description:
 *
 *   SAC standard header file.
 *
 *   This header file is included by each compiled SAC code.
 *
 *****************************************************************************/

#ifndef _SAC_H_
#define _SAC_H_

#include "sac_icm.h"

#include "sac_message.h"
#include "sac_runtimecheck.h"
#include "sac_misc.h"
#include "sac_bool.h"

#ifdef TAGGED_ARRAYS
#include "sac_std.tagged.h"
#include "sac_wl.tagged.h"
#else /* TAGGED_ARRAYS */
#include "sac_std.h"
#include "sac_wl.h"
#endif /* TAGGED_ARRAYS */

#include "sac_idx.h"
#include "sac_prf.h"
#include "sac_mt.h"
#include "sac_heapmgr.h"
#include "sac_trace.h"
#include "sac_profile.h"
#include "sac_cachesim.h"
#include "sac_arg.h"

#endif /* _SAC_H_ */
