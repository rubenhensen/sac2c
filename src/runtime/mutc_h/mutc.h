#ifndef _SAC_MUTC_H_

#define _SAC_MUTC_H_

#if SAC_MUTC_MACROS
#define bool boolbool

#define SAC_MUTC_DECL_FAMILY(name) sl_family_t family_##name;
#define SAC_MUTC_DECL_INDEX(name) sl_index (name);
#define SAC_MUTC_CREATE(name, place, lb, ub, st, bl, ap)                                 \
    sl_create (family_##name, place, lb, ub, st, bl, , ap);
#define SAC_MUTC_DECL_THREADFUN2(name, anon, ...) sl_decl (name, void, __VA_ARGS__)
#define SAC_MUTC_SYNC(name) sl_sync (family_##name);
#define SAC_MUTC_THREAD_AP2(name, ...) name, __VA_ARGS__
#define SAC_MUTC_START_DEF_THREADFUN2(name, anon, ...) sl_def (name, void, __VA_ARGS__)
#define SAC_MUTC_END_DEF_THREADFUN() sl_enddef
#define SAC_MUTC_UNLOCK_SHARED(nt) sl_setp (NT_NAME (nt), 1);
#define SAC_MUTC_LOCK_SHARED(nt) sl_getp (NT_NAME (nt));

#define SAC_MUTC_ND_PARAM_INT_GLO(nt, t, name) sl_glparm (t, name)
#define SAC_MUTC_ND_PARAM_FLO_GLO(nt, t, name) sl_glfparm (t, name)
#define SAC_MUTC_ND_PARAM_INT_SHA(nt, t, name) sl_shparm (t, name)
#define SAC_MUTC_ND_PARAM_FLO_SHA(nt, t, name) sl_shfparm (t, name)
#define SAC_MUTC_ND_ARG_INT_GLO(t, name, nt) sl_glarg (t, sl_anon, name)
#define SAC_MUTC_ND_ARG_FLO_GLO(t, name, nt) sl_glfarg (t, sl_anon, name)
#define SAC_MUTC_ND_ARG_INT_SHA(t, name, nt) sl_sharg (t, sl_anon, name)
#define SAC_MUTC_ND_ARG_FLO_SHA(t, name, nt) sl_shfarg (t, sl_anon, name)

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_ARG(name, nt, t, flag) SAC_MUTC_ARG2 (t, name, nt)
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_ARG(name, nt, t, flag) SAC_MUTC_ARG1_##flag (t, name, nt)
#define SAC_MUTC_ARG1_THREAD(t, name, nt) SAC_MUTC_ARG2 (t, name, nt)
#define SAC_MUTC_ARG1_FUN(t, name, nt) name
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_PARAM(t, name, nt, flag) SAC_MUTC_PARAM2 (nt, t, name)
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_PARAM(t, name, nt, flag) SAC_MUTC_PARAM1_##flag (nt, t, name)
#define SAC_MUTC_PARAM1_THREAD(nt, t, name) SAC_MUTC_PARAM2 (nt, t, name)
#define SAC_MUTC_PARAM1_FUN(nt, t, name) t name
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_FUNAP2(name, ...) sl_proccall (name, __VA_ARGS__);
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_FUNAP2(name, ...) name (__VA_ARGS__);
#endif /* FUNAP_AS_CREATE */

#define SAC_MUTC_GET_VAR(var_NT, name) name
#define SAC_MUTC_GET_PAR(var_NT, name) sl_getp (name)

#else /* SAC_MUTC_MACROS */

#define SAC_MUTC_GET_VAR(var_NT, name) name
#define SAC_MUTC_GET_PAR(var_NT, name) name

#define SAC_MUTC_FUNAP2(name, ...) name (__VA_ARGS__);

#define SAC_MUTC_PARAM(t, name, nt, flag) t name
#define SAC_MUTC_ARG(n, nt, t, flag) n
#define SAC_MUTC_DECL_FAMILY(name) family name;
#define SAC_MUTC_DECL_INDEX(name) index name;
#define SAC_MUTC_DECL_THREAD_FUN(a) thread a
#define SAC_MUTC_CREATE(family, place, start, end, inc, bs, ap)                          \
    create (family; place; start; end - 1; inc; bs;;) ap;
#define SAC_MUTC_SYNC(family) sync (family);
#endif

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_END_DEF_FUN() SAC_MUTC_END_DEF_THREADFUN ()
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_END_DEF_FUN() }
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_DECL_FUN2(name, type, void, ...)                                        \
    SAC_MUTC_DECL_FUN2_##void(name, type, __VA_ARGS__)
#define SAC_MUTC_DECL_FUN2_VOID(name, type, ...)                                         \
    SAC_MUTC_DECL_THREADFUN2 (name, type, __VA_ARGS__)
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_DECL_FUN2(name, type, void, ...) type name (__VA_ARGS__)
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_START_DEF_FUN2(name, type, void, ...)                                   \
    SAC_MUTC_START_DEF_FUN2_##void(name, type, __VA_ARGS__)
#define SAC_MUTC_START_DEF_FUN2_VOID(name, type, ...)                                    \
    SAC_MUTC_START_DEF_THREADFUN2 (name, type, __VA_ARGS__)
#define SAC_MUTC_START_DEF_FUN2_NONVOID(...) ERROR
#else /* FUNAP_AS_CREATE */
#define SAC_MUTC_START_DEF_FUN2(name, type, void, ...)                                   \
    type name (__VA_ARGS__)                                                              \
    {
#endif /* FUNAP_AS_CREATE */
#endif /* _SAC_MUTC_H_ */
