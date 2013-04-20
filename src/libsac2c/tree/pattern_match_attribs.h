#ifndef _SAC_PATTERN_MATCH_ATTRIBS_H_
#define _SAC_PATTERN_MATCH_ATTRIBS_H_

#include "types.h"

/**
 * generic attrib matchers:
 */
extern attrib *PMAgetNode (node **match);
extern attrib *PMAisNode (node **match);
extern attrib *PMAgetNodeOrAvis (node **match);
extern attrib *PMAisNodeOrAvis (node **match);

/**
 * attrib matchers for PMvar / PMparam:
 */
extern attrib *PMAisVar (node **var);
extern attrib *PMAgetSaaShape (node **shp);
extern attrib *PMAgetAvis (node **avis);
extern attrib *PMAhasAvis (node **avis);

/**
 * attrib matchers for PMconst:
 */
extern attrib *PMAgetVal (constant **c);
extern attrib *PMAisVal (constant **c);
extern attrib *PMAanyLeVal (constant **c);

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
extern attrib *PMAgetPrf (prf *fun);
extern attrib *PMAisPrf (prf fun);
extern attrib *PMAisNotPrf (prf fun);

/**
 * attrib matchers for with3:
 */
extern attrib *PMAhasCountRange (int *v);

/**
 * utils:
 */
extern attrib *PMAfree (attrib *p);
extern bool PMAmatch (attrib *p, node *arg);

#endif /* _SAC_PATTERN_MATCH_ATTRIBS_H_ */
