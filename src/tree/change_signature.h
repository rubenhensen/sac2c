/*
 * $Log$
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

/*****************************************************************************
 *
 * file:   change_signature.h
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

#ifndef SAC_CHANGE_SIGNATURE_H

#define SAC_CHANGE_SIGNATURE_H
extern node *CSRemoveArg (node *fundef, node *arg, nodelist *letlist, bool freearg);
extern node *CSRemoveResult (node *fundef, int position, nodelist *letlist);

extern node *CSAddArg (node *fundef, node *arg, nodelist *letlist);
extern node *CSAddResult (node *fundef, node *vardec, nodelist *letlist);

#endif /* SAC_CHANGE_SIGNATURE_H */
