/*
 *
 * $Log$
 * Revision 2.6  1999/08/04 14:36:45  bs
 *  reinitial revision
 *
 * Revision 2.5  1999/05/10 11:12:59  bs
 * All functions of the tsi moved to wl_access_analyze.c
 *
 * Revision 2.4  1999/04/12 18:01:46  bs
 * Two functions added: TSIprintAccesses and TSIprintFestures.
 *
 * Revision 2.3  1999/04/08 12:50:48  bs
 * The TSI is analysing withloops now.
 *
 * Revision 2.2  1999/03/15 15:58:23  bs
 * access macros changed, declaration of TileSizeInference modified.
 *
 * Revision 2.1  1999/02/23 12:43:18  sacbase
 * new release made
 *
 * Revision 1.1  1999/01/15 15:31:06  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   tile_size_inference.h
 *
 * prefix: TSI
 *
 * description:
 *
 *   This compiler module realizes an inference scheme for the selection
 *   of appropriate tile sizes. This is used by the code generation in
 *   order to create tiled target code.
 *
 *
 *****************************************************************************/

#ifndef _tile_size_inference_h

#define _tile_size_inference_h

#define NUM_OF_CACHEPARAM 3

extern node *TileSizeInference (node *arg_node);
extern node *TSIfundef (node *arg_node, node *arg_info);
extern node *TSIblock (node *arg_node, node *arg_info);
extern node *TSInwith (node *arg_node, node *arg_info);
extern node *TSIncode (node *arg_node, node *arg_info);

#endif /* _tile_size_inference_h  */
