/*
 *
 * $Log$
 * Revision 3.9  2002/07/24 13:19:26  dkr
 * macros MUST_... renamed
 *
 * Revision 3.8  2002/06/03 08:16:37  dkr
 * TAGGED_ARRAYS: objects of *all* types are refcounted.
 * (The *backend* decides whether code for RC is generated or not)
 *
 * Revision 3.7  2002/03/01 15:18:07  dkr
 * macro RC_INIT added
 *
 * Revision 3.6  2002/02/22 13:48:54  dkr
 * minor changes done
 *
 * Revision 3.5  2001/05/08 13:14:44  dkr
 * new macros for RC added
 *
 * Revision 3.4  2001/04/04 16:47:20  dkr
 * FUN_DOES_REFCOUNT moved to tree_compound.h
 *
 * Revision 3.3  2000/12/12 15:33:25  dkr
 * macro FUN_DOES_REFCOUNT corrected
 *
 * Revision 3.2  2000/12/12 12:17:40  dkr
 * macros MUST_REFCOUNT, FUN_DOES_REFCOUNT added
 *
 * Revision 3.1  2000/11/20 18:01:36  sacbase
 * new release made
 *
 * Revision 2.6  2000/06/08 12:13:54  jhs
 * abstraction of InferWithDFM, used to infer DFMs of with-loops,
 * can be used by other phases now
 *
 * Revision 2.5  2000/03/31 14:10:39  dkr
 * N_Nwith2 added
 *
 * Revision 2.4  2000/02/24 15:55:28  dkr
 * RC functions for old with-loop removed
 *
 * Revision 2.3  2000/02/23 17:49:22  cg
 * Type property functions IsUnique(<type>), IsBoxed(<type>)
 * moved from refcount.c to tree_compound.c.
 *
 * Revision 2.2  2000/01/25 13:38:33  dkr
 * function FindVardec renamed to FindVardec_Varno and moved to
 * tree_compound.h
 *
 * [...]
 *
 */

#ifndef _refcount_h_
#define _refcount_h_

/* value, representing an undefined reference counter */
#define RC_UNDEF (-2)
/* value, representing an inactive reference counter */
#define RC_INACTIVE (-1)

/*
 * macros for testing the RC status
 */
#define RC_IS_UNDEF(rc) ((rc) == RC_UNDEF)
#define RC_IS_INACTIVE(rc) ((rc) == RC_INACTIVE)
#define RC_IS_ACTIVE(rc) ((rc) >= 0) /* == (RC_IS_ZERO(rc) || RC_IS_VITAL(rc)) */

#define RC_INIT(rc) (RC_IS_ACTIVE (rc) ? 1 : (rc))

#define RC_IS_LEGAL(rc) ((RC_IS_INACTIVE (rc)) || (RC_IS_ACTIVE (rc)))

#define RC_IS_ZERO(rc) ((rc) == 0)
#define RC_IS_VITAL(rc) ((rc) > 0)

/*
 *  Steering which variables to be refcounted.
 */
#ifdef TAGGED_ARRAYS
#define DECL_MUST_REFCOUNT(vardec_or_arg)                                                \
    (VARDEC_OR_ARG_STATUS (vardec_or_arg) != ST_artificial)
#else /* TAGGED_ARRAYS */
#define DECL_MUST_REFCOUNT(vardec_or_arg)                                                \
    TYPE_MUST_REFCOUNT (VARDEC_OR_ARG_TYPE (vardec_or_arg))
#endif /* TAGGED_ARRAYS */
#define DECL_MUST_NAIVEREFCOUNT(vardec_or_arg)                                           \
    (VARDEC_OR_ARG_STATUS (vardec_or_arg) != ST_artificial)

#ifdef TAGGED_ARRAYS
#define TYPE_MUST_REFCOUNT(type) (TRUE)
#else /* TAGGED_ARRAYS */
#define TYPE_MUST_REFCOUNT(type) (IsArray (type) || IsNonUniqueHidden (type))
#endif /* TAGGED_ARRAYS */

extern node *Refcount (node *arg_node);

extern node *RCblock (node *arg_node, node *arg_info);
extern node *RCvardec (node *arg_node, node *arg_info);
extern node *RCassign (node *arg_node, node *arg_info);
extern node *RCloop (node *arg_node, node *arg_info);
extern node *RCprf (node *arg_node, node *arg_info);
extern node *RCid (node *arg_node, node *arg_info);
extern node *RClet (node *arg_node, node *arg_info);
extern node *RCcond (node *arg_node, node *arg_info);
extern node *RCfundef (node *arg_node, node *arg_info);
extern node *RCarg (node *arg_node, node *arg_info);
extern node *RCprepost (node *arg_node, node *arg_info);
extern node *RCicm (node *arg_node, node *arg_info);
extern node *RCNwith (node *arg_node, node *arg_info);
extern node *RCNpart (node *arg_node, node *arg_info);
extern node *RCNcode (node *arg_node, node *arg_info);
extern node *RCNgen (node *arg_node, node *arg_info);
extern node *RCNwithid (node *arg_node, node *arg_info);
extern node *RCNwithop (node *arg_node, node *arg_info);
extern node *RCNwith2 (node *arg_node, node *arg_info);

#endif /* _refcount_h_ */
