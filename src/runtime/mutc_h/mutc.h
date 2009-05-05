#ifndef _SAC_MUTC_H_

#define _SAC_MUTC_H_

#if SAC_MUTC_MACROS

#define SAC_MUTC_DECL_FAMILY(name) sl_family_t family_##name;
#define SAC_MUTC_DECL_INDEX(name) sl_index (name);
#define SAC_MUTC_CREATE(name, place, lb, ub, st, bl, ap)                                 \
    sl_create (family_##name, place, lb, yb, st, bl, , ap);
#define SAC_MUTC_DECL_THREADFUN2(name, anon, ...) sl_decl (name, , __VA_ARGS__);
#define SAC_MUTC_SYNC(name) sl_sync (family_##name);
#define SAC_MUTC_THREADAP2(name, ...) name, __VA_ARGS__
#define SAC_MUTC_START_DEF_THREADFUN2(name, anon, ...) sl_def (name, , __VA_ARGS__)
#define SAC_MUTC_END_DEF_THREADFUN() sl_enddef
#define SAC_MUTC_UNLOCK_SHARED(nt) sl_setp (NT_NAME (nt), 1);
#define SAC_MUTC_LOCK_SHARED(nt) sl_getp (NT_NAME (nt));

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_ARG1(nt, t, flag) SAC_MUTC_ARG2 (nt, t)
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_ARG1(nt, t, flag) SAC_MUTC_ARG1_##flag (nt, t)
#define SAC_MUTC_ARG1_THREAD(nt, t) SAC_MUTC_ARG2 (nt, t)
#define SAC_MUTC_ARG1_FUN(nt, t) t NT_NAME (nt)
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_PARAM(t, name, nt, flag) SAC_MUTC_PARAM2 (nt, t)
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_PARAM(t, name, nt, flag) SAC_MUTC_PARAM1_##flag (nt, t)
#define SAC_MUTC_PARAM1_THREAD(nt, t) SAC_MUTC_PARAM2 (nt, t)
#define SAC_MUTC_PARAM1_FUN(nt, t) t NT_NAME (nt)
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_FUNAP2(name, ...) sl_proccall (name, __VA_ARGS__);
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_FUNAP2(name, ...) name (__VA_ARGS__);
#endif /* FUNAP_AS_CREATE */

#define SAC_MUTC_GET_VAR(var_NT) sl_getp (var_NT)

#else /* SAC_MUTC_MACROS */

#define SAC_MUTC_GET_VAR(var_NT) SAC_ND_A_FIELD__DEFAULT (var_NT)

#define SAC_MUTC_PARAM(t, n, nt, flag) t n
#define SAC_MUTC_FUNAP(...) __VA_ARGS__
#define SAC_MUTC_DECL_FAMILY(name) family name;
#define SAC_MUTC_DECL_INDEX(name) index name;
#define SAC_MUTC_DECL_THREAD_FUN(a) thread a
#define SAC_MUTC_CREATE(family, place, start, end, inc, bs, ap)                          \
    create (family; place; start; end - 1; inc; bs;;) ap;
#define SAC_MUTC_SYNC(family) sync (family);
#endif

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_END_DEF_FUN() SAC_MUTC_END_DEF_THREADFUN
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_END_DEF_FUN() }
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_DECL_FUN2(name, type, void, ...)                                        \
    SAC_MUTC_DECL_FUN2_##void(name, type, __VA_ARGS__)
#define SAC_MUTC_DECL_FUN2_VOID(name, type, ...)                                         \
    SAC_MUTC_DECL_THREADFUN2 (name, type, , __VA_ARGS__)
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_DECL_FUN2(name, type, void, ...) type name (__VA_ARGS__);
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_START_DEF_FUN2(name, type, void, ...)                                   \
    SAC_MUTC_START_DEF_FUN2_##void(name, type, __VA_ARGS__)
#define SAC_MUTC_START_DEF_FUN2_VOID(name, type, ...)                                    \
    SAC_MUTC_START_DEF_THREADFUN2 (name, type, , __VA_ARGS__)
#define SAC_MUTC_START_DEF_FUN2_NONVOID(...) ERROR
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_START_DEF_FUN2(name, type, void, ...)                                   \
    type name (__VA_ARGS__)                                                              \
    {
#endif /* FUNAP_AS_CREATE */

#endif /* _SAC_MUTC_H_ */
