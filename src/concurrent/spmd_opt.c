/*
 *
 * $Log$
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   spmd_opt.c
 *
 * prefix: SPMDO
 *
 * description:
 *
 *   This file implements the traversal of a function body in order to
 *   optimize spmd-blocks, i.e. adjacent compatible spmd-blocks are merged
 *   to single ones.
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
