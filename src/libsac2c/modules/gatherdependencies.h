/**
 * $Id$
 *
 * @file gatherdependencies.h
 * @brief gathers the dependencies of a sac file
 * @author Stephan Herhut
 * @date 2005-06-01
 */

#ifndef _SAC_GATHERDEPENDENCIES_H_
#define _SAC_GATHERDEPENDENCIES_H_

#include "types.h"

extern node *GDPspid (node *arg_node, info *arg_info);
extern node *GDPspfold (node *arg_node, info *arg_info);
extern node *GDPtypedef (node *arg_node, info *arg_info);
extern node *GDPret (node *arg_node, info *arg_info);
extern node *GDPavis (node *arg_node, info *arg_info);
extern node *GDPcast (node *arg_node, info *arg_info);
extern node *GDPobjdef (node *arg_node, info *arg_info);
extern node *GDPfundef (node *arg_node, info *arg_info);
extern node *GDPuse (node *arg_node, info *arg_info);
extern node *GDPimport (node *arg_node, info *arg_info);
extern node *GDPmodule (node *arg_node, info *arg_info);

extern node *GDPdoGatherDependencies (node *tree);

#endif
