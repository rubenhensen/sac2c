/*
 *
 * $Log$
 * Revision 1.3  1998/05/15 09:19:29  cg
 * added ICMs MT_SYNC_NONFOLD and MT_SYNC_ONEFOLD_NONFOLD
 *
 * Revision 1.2  1998/05/13 14:48:03  cg
 * added ICM MT_SYNC_ONEFOLD
 *
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
extern void ICMCompileMT_SYNC_NONFOLD ();
extern void ICMCompileMT_SYNC_ONEFOLD (char *foldtype, char *accu_var, char *tmp_var,
                                       char *foldop);
extern void ICMCompileMT_SYNC_ONEFOLD_NONFOLD (char *foldtype, char *accu_var,
                                               char *tmp_var, char *foldop);
extern void ICMCompileMT_SYNC_FOLD_NONFOLD (int narg, char **vararg);
extern void ICMCompileMT_CONTINUE (int narg, char **vararg);
extern void ICMCompileMT_SPMD_BLOCK (char *name, int narg, char **vararg);
extern void ICMCompileMT_SPMD_PRESET (char *name, int narg, char **vararg);

#endif /* ICM2C_MT_H */
