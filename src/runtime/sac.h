/*
 *
 * $Log$
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
 *
 *****************************************************************************/

#ifndef SAC_H

#define SAC_H

#include "sac_boundcheck.h"
#include "sac_heapmgr.h"
#include "sac_misc.h"
#include "sac_message.h"
#include "sac_bool.h"

#ifdef TAGGED_ARRAYS
#include "sac_icm.h"
#include "sac_std.tagged.h"
#else /* TAGGED_ARRAYS */
#include "sac_std.h"
#endif /* TAGGED_ARRAYS */

#include "sac_idx.h"
#include "sac_prf.h"
#include "sac_mt.h"
#include "sac_wl.h"
#include "sac_trace.h"
#include "sac_profile.h"
#include "sac_cachesim.h"

#endif /* SAC_H */
