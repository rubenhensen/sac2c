/*
 *
 * $Log$
 * Revision 1.1  2000/10/12 15:46:27  dkr
 * Initial revision
 *
 */

#ifndef _sac_prf_h_
#define _sac_prf_h_

/*
 * macros for categorising prfs.
 */

/* the bounds of each category */

#define FIRST_LEGAL_PRF F_toi

#define LAST_LEGAL_PRF F_genarray

/* ... */

/* all legal prfs */

#define LEGAL_PRF(prf) ((prf >= FIRST_LEGAL_PRF) && (prf <= LAST_LEGAL_PRF))

/* the four categories */

#define SINGLE_SCALAR_ARG(prf) ((prf >= FIRST_LEGAL_PRF) && (prf <= F_not))

#define MULTI_SCALAR_ARGS(prf) ((prf > F_not) && (prf <= F_neq))

#define ARRAY_ARGS_INTRINSIC(prf) ((prf > F_neq) && (prf <= F_idx_modarray))

#define ARRAY_ARGS_NON_INTRINSIC(prf) ((prf > F_idx_modarray) && (prf <= LAST_LEGAL_PRF))

/* combinations of categories */

#define SCALAR_ARGS(prf) (SINGLE_SCALAR_ARG (prf) || MULTI_SCALAR_ARGS (prf))

#define ARRAY_ARGS(prf) (ARRAY_ARGS_INTRINSIC (prf) || ARRAY_ARGS_NON_INTRINSIC (prf))

#endif
