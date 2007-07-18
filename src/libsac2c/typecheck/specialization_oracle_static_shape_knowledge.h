#ifndef _SPECIALIZATION_ORACLE_STATIC_SHAPE_KNOWLEDGE_H_
#define _SPECIALIZATION_ORACLE_STATIC_SHAPE_KNOWLEDGE_H_

#include "types.h"

extern node *SOSSKap (node *arg_node, info *arg_info);
extern node *SOSSKarg (node *arg_node, info *arg_info);
extern node *SOSSKconst (node *arg_node, info *arg_info);
extern node *SOSSKid (node *arg_node, info *arg_info);
extern node *SOSSKids (node *arg_node, info *arg_info);
extern node *SOSSKprf (node *arg_node, info *arg_info);
extern node *SOSSKfundef (node *arg_node, info *arg_info);
extern node *SOSSKlet (node *arg_node, info *arg_info);
extern node *SOSSKwith (node *arg_node, info *arg_info);
extern node *SOSSKdoSpecializationOracleSSK (node *syntax_tree);

#endif /*_SPECIALIZATION_ORACLE_STATIC_SHAPE_KNOWLEDGE_H_*/
