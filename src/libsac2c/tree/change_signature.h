/*
 * $Log$
 * Revision 1.4  2004/11/22 21:29:55  ktr
 * Big Switch Header! SacDevCamp 04
 *
 * Revision 1.3  2001/04/17 15:48:35  nmw
 * AddResult implemented
 *
 * Revision 1.2  2001/04/09 15:54:19  nmw
 * CSAddArg implemented
 *
 * Revision 1.1  2001/03/02 15:46:04  nmw
 * Initial revision
 *
 *
 */

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
