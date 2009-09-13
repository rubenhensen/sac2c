
#ifndef _SAC_MUTC_H_

#define _SAC_MUTC_H_

#define MUTC 1
#if SAC_BACKEND == MUTC

#define bool boolbool

#define SAC_MUTC_TIME_BEGIN                                                              \
    {                                                                                    \
        uint64_t TIME_begin;                                                             \
        uint64_t TIME_end;                                                               \
        TIME_begin = get_cycles ();

#define SAC_MUTC_TIME_END                                                                \
    {                                                                                    \
        TIME_end = get_cycles ();                                                        \
        int n = TIME_end - TIME_begin;                                                   \
                                                                                         \
        sl_proccall (svp_io_puts, sl_glarg (const char *, sl_anon, "\nCYCLES: "));       \
        sl_proccall (sac_svp_io_putn, sl_glarg (int, sl_anon, n),                        \
                     sl_glarg (int, sl_anon, 10));                                       \
        sl_proccall (svp_io_puts, sl_glarg (const char *, sl_anon, "\n"));               \
    }                                                                                    \
    }

#define SAC_MUTC_DECL_FAMILY(name) sl_family_t family_##name;
#define SAC_MUTC_DECL_INDEX(name) sl_index (name);
#define SAC_MUTC_CREATE(name, place, lb, ub, st, bl, ap)                                 \
    sl_create (family_##name, place, lb, ub, st, bl, , ap);
#define SAC_MUTC_DECL_THREADFUN2(name, anon, ...) sl_decl (name, void, __VA_ARGS__)
#define SAC_MUTC_SYNC(name) sl_sync ();
#define SAC_MUTC_THREAD_AP2(name, ...) name, __VA_ARGS__
#define SAC_MUTC_DEF_THREADFUN_BEGIN2(name, anon, ...) sl_def (name, void, __VA_ARGS__)
#define SAC_MUTC_THREADFUN_DEF_END(...) sl_enddef
#define SAC_MUTC_UNLOCK_SHARED(nt) sl_setp (NT_NAME (nt), 1);
#define SAC_MUTC_LOCK_SHARED(nt) sl_getp (NT_NAME (nt));

#define SAC_MUTC_ND_PARAM_INT_GLO(t, name, nt) sl_glparm (t, name)
#define SAC_MUTC_ND_PARAM_FLO_GLO(t, name, nt) sl_glfparm (t, name)
#define SAC_MUTC_ND_PARAM_INT_SHA(t, name, nt) sl_shparm (t, name)
#define SAC_MUTC_ND_PARAM_FLO_SHA(t, name, nt) sl_shfparm (t, name)

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_PARAM_FUN(t, name, nt) SAC_MUTC_PARAM_THREAD (t, name, nt)
#else
#define SAC_MUTC_PARAM_FUN(t, name, nt) t name
#endif

#define SAC_MUTC_ND_ARG_INT_GLO(name, nt, t) sl_glarg (t, sl_anon, name)
#define SAC_MUTC_ND_ARG_FLO_GLO(name, nt, t) sl_glfarg (t, sl_anon, name)
#define SAC_MUTC_ND_ARG_INT_SHA(name, nt, t) sl_sharg (t, sl_anon, name)
#define SAC_MUTC_ND_ARG_FLO_SHA(name, nt, t) sl_shfarg (t, sl_anon, name)

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_ARG_FUN(name, nt, t) SAC_MUTC_ARG_THREAD (name, nt, t)
#else
#define SAC_MUTC_ARG_FUN(name, nt, t) name
#endif

#undef SAC_ND_REAL_PARAM
#define SAC_ND_REAL_PARAM(type, name, nt) SAC_MUTC_PARAM (type, name, nt)

#undef SAC_ND_REAL_ARG
#define SAC_ND_REAL_ARG(name, nt, type) SAC_MUTC_ARG (name, nt, type)

#if SAC_MUTC_FUNAP_AS_CREATE
#undef SAC_ND_FUNAP2
#define SAC_ND_FUNAP2(name, ...) SAC_MUTC_THREAD_FUNAP (name, __VA_ARGS__);
#endif /* FUNAP_AS_CREATE */

#define SAC_MUTC_THREAD_FUNAP(name, ...) sl_proccall (name, __VA_ARGS__);

#define SAC_MUTC_GETVAR(var_NT, name) name
#define SAC_MUTC_GETTHREADPAR(var_NT, name) sl_getp (name)

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_GETFUNPAR(var_NT, name) SAC_MUTC_GETTHREADPAR (var_NT, name)
#else
#define SAC_MUTC_GETFUNPAR(var_NT, name) name
#endif

#if SAC_MUTC_FUNAP_AS_CREATE
#undef SAC_ND_FUN_DEF_END
#define SAC_ND_FUN_DEF_END(...) SAC_MUTC_THREADFUN_DEF_END ()
#else  /* FUNAP_AS_CREATE */
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#undef SAC_ND_DECL_FUN2
#define SAC_ND_DECL_FUN2(name, type, ...)                                                \
    SAC_MUTC_DECL_FUN2_##type (name, type, __VA_ARGS__)
#define SAC_MUTC_DECL_FUN2_void(name, type, ...)                                         \
    SAC_MUTC_DECL_THREADFUN2 (name, type, __VA_ARGS__)
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#undef SAC_ND_DEF_FUN_BEGIN2
#define SAC_ND_DEF_FUN_BEGIN2(name, type, ...)                                           \
    SAC_MUTC_DEF_FUN_BEGIN2_##type (name, type, __VA_ARGS__)
#define SAC_MUTC_DEF_FUN_BEGIN2_void(name, type, ...)                                    \
    SAC_MUTC_DEF_THREADFUN_BEGIN2 (name, type, __VA_ARGS__)
#endif /* FUNAP_AS_CREATE */

#endif /* BACKEND */
#undef MUTC
#endif /* _SAC_MUTC_H_ */
