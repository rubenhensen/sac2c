/*
 *
 * $Log$
 * Revision 1.1  1999/07/08 12:28:56  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:
 *
 * prefix: SAC_HM
 *
 * description:
 *
 *
 *
 *
 *
 *
 *****************************************************************************/

#ifndef LIBSAC_HEAPMGR_H
#define LIBSAC_HEAPMGR_H

#define SAC_DO_PHM 1
#include "sac_heapmgr.h"

#define LARGECHUNK_SIZE(header) (((header) + 1)->data1.size)
#define LARGECHUNK_PREVSIZE(header) (((header) + 0)->data3.prevsize)
#define LARGECHUNK_ARENA(header) (((header) + 1)->data1.arena)
#define LARGECHUNK_PREVFREE(header) (((header) + 2)->data2.prevfree)
#define LARGECHUNK_NEXTFREE(header) (((header) + 2)->data2.nextfree)

#define SMALLCHUNK_SIZE(header) (((header) + 0)->data1.size)
#define SMALLCHUNK_ARENA(header) (((header) + 0)->data1.arena)
#define SMALLCHUNK_PREVFREE(header) (((header) + 1)->data2.prevfree)
#define SMALLCHUNK_NEXTFREE(header) (((header) + 1)->data2.nextfree)

#ifndef NULL
#define NULL ((void *)0)
#endif

extern size_byte_t SAC_HM_pagesize;

#endif /* LIBSAC_HEAPMGR_H */
