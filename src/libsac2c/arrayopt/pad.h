/*
 * $Id$
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

#ifndef _SAC_PAD_H_
#define _SAC_PAD_H_

#include "types.h"

extern void APprintDiag (char *format, ...);
extern node *APdoArrayPadding (node *arg_node);

#endif /* _SAC_PAD_H_  */
