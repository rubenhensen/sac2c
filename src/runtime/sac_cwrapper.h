/*
 *
 * $Log$
 * Revision 3.3  2002/04/30 08:38:09  dkr
 * no changes done
 *
 * Revision 3.2  2000/12/05 14:31:28  nmw
 * refcounter handling for T_hidden added to SAC_IW_CHECKDEC_RC
 * and SAC_IW_INC_RC.
 *
 * Revision 3.1  2000/11/20 18:02:13  sacbase
 * new release made
 *
 * Revision 1.9  2000/08/03 14:16:53  nmw
 * handling macro for T_hidden added
 * ,
 *
 * Revision 1.8  2000/07/28 14:42:59  nmw
 * macros changed to handle T_user types
 *
 * Revision 1.7  2000/07/24 14:53:41  nmw
 * macros changed for refcounter check
 *
 * Revision 1.6  2000/07/20 11:35:46  nmw
 * added macro for runtimecheck of initialized module
 *
 * Revision 1.5  2000/07/13 14:59:12  nmw
 * output of errormsg now with SAC_RuntimeError()
 *
 * Revision 1.4  2000/07/12 10:15:44  nmw
 * RCS-header added
 *
 * Revision 1.3  2000/07/07 15:32:00  nmw
 * DEC-RC macro modified
 *
 * Revision 1.2  2000/07/06 15:54:08  nmw
 * macros debugged
 *
 * Revision 1.1  2000/07/05 14:01:58  nmw
 * Initial revision
 *
 * makros for compiling the SAC <-> c Interface wrapper functions
 *
 */

#ifndef _sac_cwrapper_h
#define _sac_cwrapper_h

/* this is a workaround to avoid errors after renaming the internal type */
#define _hidden_ void *

/* check for refcount >=1 , decrement refcounter */
#define SAC_IW_CHECKDEC_RC(a, CONST_T_HIDDEN)                                            \
    if (SAC_ARG_LRC (a) <= 0) {                                                          \
        SAC_RuntimeError ("Referencecounter reaches 0, no data available!\n");           \
    }                                                                                    \
    if ((SAC_ARG_DIM (a) > 0) || (SAC_ARG_TYPE (a) == CONST_T_HIDDEN)) {                 \
        /* decrement only if refcounted arg */                                           \
        SAC_ARG_LRC (a) = SAC_ARG_LRC (a) - 1;                                           \
    }

/* restore old refcounter */
#define SAC_IW_INC_RC(a, CONST_T_HIDDEN)                                                 \
    if ((SAC_ARG_DIM (a) > 0) || (SAC_ARG_TYPE (a) == CONST_T_HIDDEN)) {                 \
        /* increment only if refcounted arg */                                           \
        SAC_ARG_LRC (a) = SAC_ARG_LRC (a) + 1;                                           \
    }

/* argument is simple type */
#define SAC_ARGCALL_SIMPLE(var, type) (*((type *)(SAC_ARG_ELEMS (var))))

/* argument is array type - expand with additional refcounter */
#define SAC_ARGCALL_REFCNT(var, type) (type *)(SAC_ARG_ELEMS (var)), (SAC_ARG_RC (var))

/* argument is simple type */
#define SAC_ARGCALL_INOUT_SIMPLE(var, type) ((type *)(SAC_ARG_ELEMS ((var))))

/* argument is array type - expand with additional refcounter */
#define SAC_ARGCALL_INOUT_REFCNT(var, type)                                              \
    (type **)(&SAC_ARG_ELEMS ((var))), (int **)(&SAC_ARG_RC ((var)))

/* result is simple type */
#define SAC_RESULT_SIMPLE(var, type) ((type *)(SAC_ARG_ELEMS ((*var))))

/* result is array type - expand with additional refcounter
 * also used if arg is tagged as inout
 */
#define SAC_RESULT_REFCNT(var, type)                                                     \
    (type **)(&SAC_ARG_ELEMS ((*var))), (int **)(&SAC_ARG_RC ((*var)))

/* result is T_hidden and refcounted */
#define SAC_RESULT_HIDDEN_RC(var, type)                                                  \
    (type *)(&SAC_ARG_ELEMS ((*var))), (int **)(&SAC_ARG_RC ((*var)))

/* alloc data memory for simple results */
#define SAC_CI_INIT_SIMPLE_RESULT(var, type)                                             \
    SAC_ARG_ELEMS ((*var)) = SAC_MALLOC (sizeof (type));

/* simple type used as direct result */
#define SAC_ASSIGN_RESULT(var, type) (*((type *)SAC_ARG_ELEMS ((*var))))

/* set local refcount of var to value */
#define SAC_SETLOCALRC(var, value) SAC_ARG_LRC ((*var)) = value;

/*
 * decrement local refcounter
 * if refcount reaches 0, data and refcounter has been freed by SAC-function
 */
#define SAC_DECLOCALRC(var)                                                              \
    SAC_ARG_LRC (var) = SAC_ARG_LRC (var) - 1;                                           \
    if (SAC_ARG_LRC (var) == 0) {                                                        \
        SAC_ARG_ELEMS (var) = NULL;                                                      \
        SAC_ARG_RC (var) = NULL;                                                         \
    }

/* checks at the beginning of each wrapper if the module has been initialized */
#define SAC_IW_CHECK_MODINIT(flag, initfun)                                              \
    if (!flag) {                                                                         \
        flag = true;                                                                     \
        initfun ();                                                                      \
    }

#endif /* _sac_cwrapper_h */
