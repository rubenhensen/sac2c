/* $Id$ */

/*
 * mapping of arguments and results
 */
#define SAC4C_DECL_ARG(no) SACarg *SAC4C_ARG (no)
#define SAC4C_DECL_RET(no) SACarg **SAC4C_RET (no)

#define SAC4C_ARG(no) CAT0 (arg, no)
#define SAC4C_RET(no) CAT1 (result, no)

#define SAC4C_VOID void

/*
 * mapping of function names
 */
#define SAC4C_FUNNAME(arity, ns, name) CAT4 (CAT3 (CAT2 (ns, __), name), arity)

/*
 * mapping of extern directive
 */
#define SAC4C_EXTERN extern

/*
 * wrapper decision tree
 */
#define SAC4C_DISPATCH_BLOCK_BEGIN

#define SAC4C_DISPATCH_BLOCK_END

#define SAC4C_DISPATCH(test, instance)

#define SAC4C_ARG_MATCH_BASE(arg1, arg2)

#define SAC4C_ARG_MATCH_UDT(arg1, arg2)

/*
 * concatenation macros
 */
#define CAT0(x, y) xCAT0 (x, y)
#define xCAT0(x, y) x##y
#define CAT1(x, y) xCAT1 (x, y)
#define xCAT1(x, y) x##y
#define CAT2(x, y) xCAT2 (x, y)
#define xCAT2(x, y) x##y
#define CAT3(x, y) xCAT3 (x, y)
#define xCAT3(x, y) x##y
#define CAT4(x, y) xCAT4 (x, y)
#define xCAT4(x, y) x##y
#define CAT5(x, y) xCAT5 (x, y)
#define xCAT5(x, y) x##y
#define CAT6(x, y) xCAT6 (x, y)
#define xCAT6(x, y) x##y
#define CAT7(x, y) xCAT7 (x, y)
#define xCAT7(x, y) x##y
#define CAT8(x, y) xCAT8 (x, y)
#define xCAT8(x, y) x##y
#define CAT9(x, y) xCAT9 (x, y)
#define xCAT9(x, y) x##y
#define CAT10(x, y) xCAT10 (x, y)
#define xCAT10(x, y) x##y
