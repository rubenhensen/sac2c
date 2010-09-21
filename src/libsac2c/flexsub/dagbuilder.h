/*****************************************************************************
 *
 * file:   dagbuilder.h
 *
 * description:
 *   header file for dagbuilder.c
 *
 *****************************************************************************/

#ifndef _DAGBUILDER_H_
#define _DAGBUILDER_H_

#include "types.h"

extern node *TFBDGdoBuildTFGraph (node *arg_node);
extern node *TFBDGtfspec (node *arg_node, info *arg_info);
extern node *TFBDGtfvertex (node *arg_node, info *arg_info);

#endif /* _DAGBUILDER_H_ */
