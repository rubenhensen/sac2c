#ifndef _SAC_CHANGE_SIGNATURE_H_
#define _SAC_CHANGE_SIGNATURE_H_

#include "types.h"

/*****************************************************************************
 *
 * change_signature.h
 *
 * prefix: CS
 *
 * description:
 *  thismodule implements some functions to change the functions signature
 *  (add/remove args or results) of a given funtion for a given list of
 *  functions applications.
 *
 *
 *****************************************************************************/
extern node *CSremoveArg (node *fundef, node *arg, nodelist *letlist, bool freearg);
extern node *CSremoveResult (node *fundef, int position, nodelist *letlist);

extern node *CSaddArg (node *fundef, node *arg, nodelist *letlist);
extern node *CSaddResult (node *fundef, node *vardec, nodelist *letlist);

#endif /* _SAC_CHANGE_SIGNATURE_H_ */
