/*
 *
 * $Id$
 */

#ifndef _SAC_PATTERN_MATCH_ATTRIBS_H_
#define _SAC_PATTERN_MATCH_ATTRIBS_H_

#include "types.h"

/** needs to be moved into types.h: */
typedef struct PATTR attrib;

/**
 * generic attrib matchers:
 */
extern attrib *PMAgetNode (node **match);

/**
 * attrib matchers for PMvar:
 */
extern attrib *PMAisVar (node **var);

/**
 * attrib matchers for PMconst:
 */
extern attrib *PMAgetVal (constant **c);
extern attrib *PMAisVal (constant **c);

/**
 * attrib matchers for PMint:
 */
extern attrib *PMAgetIVal (int *v);
extern attrib *PMAisIVal (int *v);
extern attrib *PMAleIVal (int *v);

/**
 * attrib matchers for PMarray:
 */
extern attrib *PMAgetLen (int *l);
extern attrib *PMAhasLen (int *l);
extern attrib *PMAgetFS (constant **fs);
extern attrib *PMAhasFS (constant **fs);

/**
 * attrib matchers for PMprf:
 */
extern attrib *PMAisPrf (prf fun);

/**
 * utils:
 */
extern attrib *PMAfree (attrib *p);
extern bool PMAmatch (attrib *p, node *arg);

#endif /* _SAC_PATTERN_MATCH_ATTRIBS_H_ */
