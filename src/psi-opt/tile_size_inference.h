/*
 *
 * $Log$
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

#ifndef TRUE

#define TRUE 1
#define FALSE 0

#endif

extern node *TileSizeInference (node *arg_node);

extern node *TSIfundef (node *arg_node, node *arg_info);
extern node *TSIblock (node *arg_node, node *arg_info);
extern node *TSInwith (node *arg_node, node *arg_info);
extern node *TSIncode (node *arg_node, node *arg_info);
extern node *TSIassign (node *arg_node, node *arg_info);
extern node *TSIlet (node *arg_node, node *arg_info);
extern node *TSIap (node *arg_node, node *arg_info);
extern node *TSIid (node *arg_node, node *arg_info);
extern node *TSIwhile (node *arg_node, node *arg_info);
extern node *TSIdo (node *arg_node, node *arg_info);
extern node *TSIcond (node *arg_node, node *arg_info);
extern node *TSIprf (node *arg_node, node *arg_info);

#endif /* _tile_size_inference_h  */
