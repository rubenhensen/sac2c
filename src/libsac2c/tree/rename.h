/*
 * $Id$
 */

#ifndef _SAC_RENAME_H_
#define _SAC_RENAME_H_

#include "types.h"
#include "LookUpTable.h"

/*****************************************************************************
 *
 * Rename traversal ( rename_tab)
 *
 * prefix: REN
 *
 * description:
 *
 *   header file for rename.c.
 *
 *****************************************************************************/

extern node *RENdoRenameLut (node *arg_node, lut_t *lut);

extern node *RENarg (node *arg_node, info *arg_info);
extern node *RENvardec (node *arg_node, info *arg_info);
extern node *RENid (node *arg_node, info *arg_info);
extern node *RENids (node *arg_node, info *arg_info);
extern node *RENwlsegvar (node *arg_node, info *arg_info);

#endif /* _SAC_RENAME_H_ */
