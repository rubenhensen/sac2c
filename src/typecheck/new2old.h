/*
 * $Log$
 * Revision 1.1  2002/08/05 16:58:33  sbs
 * Initial revision
 *
 *
 */

#ifndef _new2old_h
#define _new2old_h

#include "types.h"

extern node *NT2OTTransform (node *arg_node);

extern node *NT2OTfundef (node *arg_node, node *arg_info);
extern node *NT2OTvardec (node *arg_node, node *arg_info);

#endif /* _new2old_h */
