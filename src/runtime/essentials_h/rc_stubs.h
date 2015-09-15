#if defined(SAC_BACKEND_C99) || defined(SAC_BACKEND_DISTMEM)

#ifndef SAC_HAS_RC_NORC
/* Stub SAC_ND_RC_TO_NORC / SAC_ND_RC_FROM_NORC
 * The selected RC method cannot do the NORC mode to save us copying the descriptors
 * in SPMD functions.
 * Define the missing macros as empty, and copy descriptors in SPMD frame code. */

#define SAC_HAS_RC_NORC 0

#define SAC_ND_RC_TO_NORC__NODESC(var_NT)   /* stub */
#define SAC_ND_RC_TO_NORC__DESC(var_NT)     /* stub */
#define SAC_ND_RC_FROM_NORC__NODESC(var_NT) /* stub */
#define SAC_ND_RC_FROM_NORC__DESC(var_NT)   /* stub */

#endif /* not defined SAC_HAS_RC_NORC */

#endif /* defined(SAC_BACKEND_C99) || defined(SAC_BACKEND_DISTMEM) */
