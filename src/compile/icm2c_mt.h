/*
 *
 * $Log$
 * Revision 1.1  1998/05/13 07:22:57  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   icm2c_mt.h
 *
 * prefix: ICMCompile
 *
 * description:
 *
 *   This file contains the prototypes of those functions that actually
 *   implement the C implemented ICMs.
 *
 *
 *
 *****************************************************************************/

#ifndef ICM2C_MT_H

#define ICM2C_MT_H

extern void ICMCompileMT_SPMD_FUN_DEC (char *name, char *from, int narg, char **vararg);
extern void ICMCompileMT_SPMD_FUN_RET (int narg, char **vararg);
extern void ICMCompileMT_START_SYNCBLOCK (int narg, char **vararg);
extern void ICMCompileMT_SYNC_FOLD (int narg, char **vararg);
extern void ICMCompileMT_SYNC_FOLD_NONFOLD (int narg, char **vararg);
extern void ICMCompileMT_CONTINUE (int narg, char **vararg);
extern void ICMCompileMT_SPMD_BLOCK (char *name, int narg, char **vararg);
extern void ICMCompileMT_SPMD_PRESET (char *name, int narg, char **vararg);

#endif /* ICM2C_MT_H */
