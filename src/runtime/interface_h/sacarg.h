/*
 *
 * $Id$
 *
 */

#ifndef _SAC_SACARG_H_
#define _SAC_SACARG_H_

/* constants */
#define SAC_CI_SIMPLETYPE 1
#define SAC_CI_ARRAYTYPE 2

/* basetypes (same as in sac2c internally used) */
typedef int SAC_ARG_simpletype;

/* SAC integer array */
typedef struct SAC_ARG_STRUCT {
    int type;       /* type of argument */
    int *shpvec;    /* shapevector of argument */
    int dim;        /* dim of shape */
    void *elems;    /* pointer to data elements */
    int *rc;        /* pointer to revcounter */
    int lrc;        /* local refcounter copy */
    char *typename; /* name of usertype or NULL */
} SAC_ARG_STRUCT;

#define SAC_ARG_LRC(a) (a->lrc)
#define SAC_ARG_RC(a) (a->rc)
#define SAC_ARG_ELEMS(a) (a->elems)
#define SAC_ARG_TYPE(a) (a->type)
#define SAC_ARG_SHPVEC(a) (a->shpvec)
#define SAC_ARG_DIM(a) (a->dim)
#define SAC_ARG_TNAME(a) (a->typename)

extern SAC_arg SAC_CI_NewSACArg (SAC_ARG_simpletype basetype, char *tname, int dim,
                                 int *shpvec);
extern SAC_arg SAC_CI_CreateSACArg (SAC_ARG_simpletype basetype, char *tname, int dim,
                                    ...);
extern bool SAC_CI_CmpSACArgType (SAC_arg sa, SAC_ARG_simpletype basetype, char *tname,
                                  int dim, ...);
extern SAC_arg SAC_CI_InitRefcounter (SAC_arg sa, int initvalue);
extern void SAC_CI_ExitOnInvalidArg (SAC_arg sa, SAC_ARG_simpletype basetype, char *tname,
                                     int flag);

extern void SAC_CI_InitSACArgDirectory ();
extern void SAC_CI_FreeSACArgDirectory ();

extern char *SAC_CI_SACArg2string (SAC_arg sa, char *buffer);

#endif /* _SAC_SACARG_H_ */
