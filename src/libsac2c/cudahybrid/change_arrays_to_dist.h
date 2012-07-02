#ifndef _SAC_CHANGEARRAYSTODIST_H_
#define _SAC_CHANGEARRAYSTODIST_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Change Arrays to Distributed Type (cadt)
 *
 * Prefix: CADT
 *
 *****************************************************************************/
extern node *CADTdoChangeArraysToDistributedType (node *syntax_tree);

extern node *CADTfundef (node *arg_node, info *arg_info);
extern node *CADTwith (node *arg_node, info *arg_info);
extern node *CADTret (node *arg_node, info *arg_info);
extern node *CADTavis (node *arg_node, info *arg_info);

#endif /* _SAC_CHANGEARRAYSTODIST_H_ */
