/*
 *
 * $Log$
 * Revision 3.3  2004/11/22 11:00:05  ktr
 * Ismop 2004 SacDevCamp 04
 *
 * Revision 3.2  2001/03/22 18:02:55  dkr
 * tree.h no longer included
 *
 * Revision 3.1  2000/11/20 18:03:40  sacbase
 * new release made
 *
 * Revision 1.1  2000/07/21 08:18:08  nmw
 * Initial revision
 *
 */

#ifndef _SAC_IMPORT_SPECIALIZATION_H_
#define _SAC_IMPORT_SPECIALIZATION_H_

#include "types.h"

/******************************************************************************
 *
 * Import Specialization traversal (impspec_tab)
 *
 * Prefix: IMPSPEC
 *
 *****************************************************************************/
extern node *IMPSPECdoImportSpecialization (node *syntax_tree);

extern node *IMPSPECfundef (node *arg_node, info *arg_info);
extern node *IMPSPECmodspec (node *arg_node, info *arg_info);
extern node *IMPSPECarg (node *arg_node, info *arg_info);

#endif /* _SAC_IMPORT_SPECIALIZATION_H_ */
