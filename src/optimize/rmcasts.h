/*
 * $Log$
 * Revision 1.1  2001/05/22 09:09:45  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   rmcasts.h
 *
 * prefix: RC
 *
 * description:
 *   this module removes all casts from AST
 *
 *****************************************************************************/

#ifndef SAC_RMCASTS_H

#define SAC_RMCASTS_H

extern node *RemoveCasts (node *ast);

extern node *RCcast (node *arg_node, node *arg_info);
#endif /* SAC_RMCASTS_H */
