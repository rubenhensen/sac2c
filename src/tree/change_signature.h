/*
 * $Log$
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
node *CSRemoveArg (node *fundef, node *arg, nodelist *letlist, bool freearg);
node *CSRemoveResult (node *fundef, int position, nodelist *letlist);

#endif /* SAC_CHANGE_SIGNATURE_H */
