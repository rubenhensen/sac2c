/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:44:21  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sync_opt.c
 *
 * prefix: SYNCI
 *
 * description:
 *
 *   This file implements the traversal of a function body in order to
 *   optimize synchronisation blocks, i.e. adjacent synchronisation blocks
 *   are merged into a single one where data dependencies do not require
 *   a barrier synchronisation in between.
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
