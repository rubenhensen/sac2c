/*
 * $Id: ivextram.h 15815 2008-10-24 18:04:47Z rbe $
 */
#ifndef _SAC_INDEX_VECTOR_EXTREMA_INSERTION_H_
#define _SAC_INDEX_VECTOR_EXTREMA_INSERTION_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Template traversal ( ivext_tab)
 *
 * Prefix: IVEXI
 *
 *****************************************************************************/
extern node *IVEXIdoInsertIndexVectorExtrema (node *arg_node);

extern node *IVEXImodule (node *arg_node, info *arg_info);
extern node *IVEXIfundef (node *arg_node, info *arg_info);
extern node *IVEXIblock (node *arg_node, info *arg_info);
extern node *IVEXIcode (node *arg_node, info *arg_info);
extern node *IVEXIlet (node *arg_node, info *arg_info);
extern node *IVEXIwith (node *arg_node, info *arg_info);
extern node *IVEXIgenerator (node *arg_node, info *arg_info);
extern node *IVEXIpart (node *arg_node, info *arg_info);
extern node *IVEXIassign (node *arg_node, info *arg_info);

#endif // _SAC_INDEX_VECTOR_EXTREMA_INSERTION_H_
