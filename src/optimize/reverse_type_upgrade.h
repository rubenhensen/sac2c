/*
 *
 * $Log$
 *
 */

#ifndef _SAC_REVERSE_TYPE_UPGRADE_H_
#define _SAC_REVERSE_TYPE_UPGRADE_H_

#include "types.h"

extern node *RTUPdoReverseTypeUpgrade (node *module);
extern node *RTUPdoReverseTypeUpgradeOneFundef (node *fundef);

extern node *RTUPfundef (node *arg_node, info *arg_info);
extern node *RTUPassign (node *arg_node, info *arg_info);
extern node *RTUPlet (node *arg_node, info *arg_info);
extern node *RTUPids (node *arg_node, info *arg_info);
extern node *RTUPid (node *arg_node, info *arg_info);
extern node *RTUPwith (node *arg_node, info *arg_info);
extern node *RTUPpart (node *arg_node, info *arg_info);
extern node *RTUPgenerator (node *arg_node, info *arg_info);

#endif
