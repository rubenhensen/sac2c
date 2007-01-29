/*
 *
 * $Log$
 * Revision 1.1  2005/02/02 18:13:34  mwe
 * Initial revision
 *
 *
 */

#ifndef _SAC_SIGNATURE_SIMPLIFICATION_H_
#define _SAC_SIGNATURE_SIMPLIFICATION_H_

#include "types.h"

node *SISIdoSignatureSimplification (node *module);

node *SISImodule (node *arg_node, info *arg_info);
node *SISIfundef (node *arg_node, info *arg_info);
node *SISIarg (node *arg_node, info *arg_info);
node *SISIblock (node *arg_node, info *arg_info);
node *SISIassign (node *arg_node, info *arg_info);
node *SISIlet (node *arg_node, info *arg_info);
node *SISIap (node *arg_node, info *arg_info);
node *SISIret (node *arg_node, info *arg_info);
node *SISIreturn (node *arg_node, info *arg_info);
node *SISIids (node *arg_node, info *arg_info);
node *SISIexprs (node *arg_node, info *arg_info);
node *SISIid (node *arg_node, info *arg_info);

#endif /*_SAC_SIGNATURE_SIMPLIFICATION_H_*/
