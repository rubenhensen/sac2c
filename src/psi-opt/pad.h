/*
 * $Log$
 * Revision 3.2  2002/10/02 14:53:07  cg
 * External declaration of apdiag_file removed since it is
 * declared static in pad.c
 *
 * Revision 3.1  2000/11/20 18:01:48  sacbase
 * new release made
 *
 * Revision 1.5  2000/08/03 15:29:49  mab
 * added apdiag_file, APprintDiag
 * removed all dummies
 *
 * Revision 1.4  2000/06/14 10:41:31  mab
 * comments added
 *
 * Revision 1.3  2000/06/08 11:14:49  mab
 * pad_info added
 *
 * Revision 1.2  2000/05/26 14:24:29  sbs
 * dummy function ArrayPadding added.
 *
 * Revision 1.1  2000/05/26 13:41:40  sbs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   pad.h
 *
 * prefix: AP
 *
 * description:
 *
 *   This compiler module infers new array shapes and applies array padding
 *   to improve cache performance.
 *
 *
 *****************************************************************************/

#ifndef sac_pad_h

#define sac_pad_h

extern void APprintDiag (char *format, ...);

extern node *ArrayPadding (node *arg_node);

#endif /* sac_pad_h */
