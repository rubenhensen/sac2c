/*
 * $Id: index vector extrema cleanup.h 15657 2007-11-13 13:57:30Z cg $
 */
#ifndef _SAC_INDEX_VECTOR_EXTREMA_CLEANUP_H_
#define _SAC_INDEX_VECTOR_EXTREMA_CLEANUP_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Template traversal ( temp_tab)
 *
 * Prefix: IVEXC
 *
 *****************************************************************************/
extern node *IVEXCdoIndexVectorExtremaCleanup (node *arg_node);
extern node *IVEXCdoIndexVectorExtremaCleanupPartition (node *arg_node, info *arg_info);

extern node *IVEXCpart (node *arg_node, info *arg_info);
extern node *IVEXCcode (node *arg_node, info *arg_info);
extern node *IVEXClet (node *arg_node, info *arg_info);
extern node *IVEXCavis (node *arg_node, info *arg_info);
extern node *IVEXCprf (node *arg_node, info *arg_info);
extern node *IVEXCid (node *arg_node, info *arg_info);
extern node *IVEXCids (node *arg_node, info *arg_info);

#endif // _SAC_INDEX_VECTOR_EXTREMA_CLEANUP_H_
