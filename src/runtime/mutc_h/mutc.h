
#ifndef _SAC_MUTC_H_

#define _SAC_MUTC_H_

#ifdef SAC_BACKEND_MUTC
/*#define bool boolbool*/

#include <svp/perf.h>

#define SAC_MUTC_TIME_BEGIN                                                              \
    {                                                                                    \
        uint64_t TIME_begin;                                                             \
        uint64_t TIME_end;                                                               \
        TIME_begin = get_cycles ();

#define SAC_MUTC_TIME_END                                                                \
    {                                                                                    \
        TIME_end = get_cycles ();                                                        \
        long long n = TIME_end - TIME_begin;                                             \
                                                                                         \
        sl_proccall (svp_io_puts, sl_glarg (const char *, sl_anon, "\nCYCLES: "));       \
        sl_proccall (svp_io_putn, sl_glarg (long long, sl_anon, n),                      \
                     sl_glarg (int, sl_anon, 10));                                       \
        sl_proccall (svp_io_puts, sl_glarg (const char *, sl_anon, "\n"));               \
    }                                                                                    \
    }

#define SAC_MUTC_DECL_FAMILY(name)
#define SAC_MUTC_DECL_INDEX(name) sl_index (name);

#if SAC_MUTC_SEQ_DATA_PARALLEL
#define SAC_MUTC_CREATE(name, place, lb, ub, st, bl, ap)                                 \
    sl_create (, place, lb, ub, st, bl, sl__forceseq, ap);
#else
#define SAC_MUTC_CREATE(name, place, lb, ub, st, bl, ap)                                 \
    sl_create (, place, lb, ub, st, bl, , ap);
#endif

#define SAC_MUTC_DECL_THREADFUN2(name, anon, ...) sl_decl (name, void, __VA_ARGS__)
#define SAC_MUTC_SYNC(name) sl_sync ();
#define SAC_MUTC_THREAD_AP2(name, ...) name, __VA_ARGS__

#define SAC_MUTC_SAVE_NODESC(nt) NT_NAME (nt) = sl_geta (CAT0 (SAC_ND_A_FIELD (nt), _sh));

#define SAC_MUTC_SAVE_DESC(nt)                                                           \
    SAC_ND_A_FIELD (nt) = sl_geta (CAT0 (SAC_ND_A_FIELD (nt), _sh));                     \
    SAC_ND_A_DESC (nt) = sl_geta (CAT0 (SAC_ND_A_DESC (nt), _sh));

#define SAC_MUTC_CREATE_BLOCK_START() {
#define SAC_MUTC_CREATE_BLOCK_END() }

#define SAC_MUTC_DEF_THREADFUN_BEGIN2(name, anon, ...)                                   \
    sl_def (name, void, __VA_ARGS__)                                                     \
    {                                                                                    \
        SAC_MUTC_THREAD_INIT
#define SAC_MUTC_THREADFUN_DEF_END(...)                                                  \
    SAC_MUTC_THREAD_CLEANUP                                                              \
    }                                                                                    \
    sl_enddef

#define SAC_MUTC_UNLOCK_SHARED(nt) sl_setp (NT_NAME (nt), 1);
#define SAC_MUTC_LOCK_SHARED(nt) sl_getp (NT_NAME (nt));

#define SAC_ND_PRF_SYNCIN_NODESC(nt, sh) NT_NAME (nt) = sl_getp (NT_NAME (sh));

#define SAC_ND_PRF_SYNCIN_DESC(nt, sh)                                                   \
    SAC_ND_PRF_SYNCIN_NODESC (nt, sh)                                                    \
    SAC_ND_A_DESC_NAME (nt) = sl_getp (SAC_ND_A_DESC_NAME (sh));

#define SAC_ND_PRF_SYNCOUT_NODESC(nt, sh) sl_setp (NT_NAME (sh), NT_NAME (nt));

#if SVP_HAS_SEP
#define SAC_MUTC_MEMORY_WRITE_BARRIER                                                    \
    {                                                                                    \
        __asm__ __volatile__("wmb # memory barrier" ::: "memory");                       \
    }
#else
#define SAC_MUTC_MEMORY_WRITE_BARRIER
#endif

#define SAC_ND_PRF_SYNCOUT_DESC(nt, sh)                                                  \
    SAC_MUTC_MEMORY_WRITE_BARRIER                                                        \
    SAC_ND_PRF_SYNCOUT_NODESC (nt, sh)                                                   \
    sl_setp (SAC_ND_A_DESC_NAME (sh), SAC_ND_A_DESC_NAME (nt));

#define SAC_MUTC_ND_PARAM_INT_GLO(t, name, nt) sl_glparm_mutable (t, name)
#define SAC_MUTC_ND_PARAM_FLO_GLO(t, name, nt) sl_glfparm_mutable (t, name)
#define SAC_MUTC_ND_PARAM_INT_SHA(t, name, nt) sl_shparm (t, name)
#define SAC_MUTC_ND_PARAM_FLO_SHA(t, name, nt) sl_shfparm (t, name)

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_PARAM_FUN(t, name, nt) SAC_MUTC_PARAM_THREAD (t, name, nt)
#else
#define SAC_MUTC_PARAM_FUN(t, name, nt) t name
#endif

#define SAC_MUTC_ND_ARG_INT_GLO(name, nt, t) sl_glarg (t, sl_anon, name)
#define SAC_MUTC_ND_ARG_FLO_GLO(name, nt, t) sl_glfarg (t, sl_anon, name)

#define SAC_MUTC_ND_ARG_INT_SHA(name, nt, t) sl_sharg (t, sh1, name)
#define SAC_MUTC_ND_ARG_FLO_SHA(name, nt, t) sl_shfarg (t, sh1, name)

#define SAC_MUTC_ARG_SHA(tag, type, name1, name2)                                        \
    CAT0 (CAT1 (sl_sh, tag), arg) (type, name1, name2)

#define SAC_MUTC_ARG_SHA_DESC(nt)                                                        \
    SAC_MUTC_ARG_SHA (, SAC_ND_DESC_TYPE (nt), CAT0 (SAC_ND_A_DESC_NAME (nt), _sh),      \
                      SAC_ND_A_DESC_NAME (nt))

#define SAC_MUTC_ARG_SHA_NAME(tag, nt, t)                                                \
    SAC_MUTC_ARG_SHA (tag, SAC_ND_TYPE (nt, t), CAT0 (SAC_ND_A_FIELD (nt), _sh),         \
                      SAC_ND_A_FIELD (nt))

#define SAC_MUTC_ARG_SHARED_DESC_INT(nt, t)                                              \
    SAC_MUTC_ARG_SHA_NAME (, nt, t), SAC_MUTC_ARG_SHA_DESC (nt)
#define SAC_MUTC_ARG_SHARED_DESC_FLO(nt, t)                                              \
    SAC_MUTC_ARG_SHA_NAME (f, nt, t), SAC_MUTC_ARG_SHA_DESC (nt)
#define SAC_MUTC_ARG_SHARED_NODESC_INT(nt, t) SAC_MUTC_ARG_SHA_NAME (, nt, t)
#define SAC_MUTC_ARG_SHARED_NODESC_FLO(nt, t) SAC_MUTC_ARG_SHA_NAME (f, nt, t)

#define SAC_ND_ARG_shared(nt, t) SAC_MUTC_ARG_SHARED (nt, t)

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_ARG_FUN(name, nt, t) SAC_MUTC_ARG_THREAD (name, nt, t)
#else
#define SAC_MUTC_ARG_FUN(name, nt, t) name
#endif

#define SAC_ND_REAL_PARAM(type, name, nt) SAC_MUTC_PARAM (type, name, nt)

#define SAC_ND_REAL_ARG(name, nt, type) SAC_MUTC_ARG (name, nt, type)

#define SAC_ND_REAL_ARG_out(name, nt, type) SAC_MUTC_ARG_out (name, nt, type)

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_ND_FUNAP2(name, ...) SAC_MUTC_THREAD_FUNAP (name, __VA_ARGS__);
#else
#define SAC_ND_FUNAP2(name, ...) name (__VA_ARGS__); // same as in std.h!
#endif /* FUNAP_AS_CREATE */

#define SAC_MUTC_THREAD_FUNAP(name, ...) sl_proccall (name, __VA_ARGS__);

#define SAC_MUTC_GETVAR(var_NT, name) name
#define SAC_MUTC_GETTHREADPAR(var_NT, name) sl_getp (name)

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_GETFUNPAR(var_NT, name) SAC_MUTC_GETTHREADPAR (var_NT, name)
#else
#define SAC_MUTC_GETFUNPAR(var_NT, name) name
#endif

#define SAC_INIT_LOCAL_MEM() SAC_MUTC_THREAD_INIT_MALLOC
#define SAC_CLEANUP_LOCAL_MEM() SAC_MUTC_THREAD_CLEANUP_MALLOC

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_ND_FUN_DEF_END2() SAC_MUTC_THREADFUN_DEF_END ()
#else  /* FUNAP_AS_CREATE */
#define SAC_ND_FUN_DEF_END2(...) } // same as in std.h!
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_ND_DECL_FUN2(name, type, ...)                                                \
    SAC_MUTC_DECL_FUN2_##type (name, type, __VA_ARGS__)
#define SAC_MUTC_DECL_FUN2_void(name, type, ...)                                         \
    SAC_MUTC_DECL_THREADFUN2 (name, type, __VA_ARGS__)
#else
#define SAC_ND_DECL_FUN2(name, type, ...) type name (__VA_ARGS__) // same as in std.h!
#endif /* FUNAP_AS_CREATE */

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_ND_DEF_FUN_BEGIN2(name, type, ...)                                           \
    SAC_MUTC_DEF_FUN_BEGIN2_##type (name, type, __VA_ARGS__)
#define SAC_MUTC_DEF_FUN_BEGIN2_void(name, type, ...)                                    \
    SAC_MUTC_DEF_THREADFUN_BEGIN2 (name, type, __VA_ARGS__)
#else
#define SAC_ND_DEF_FUN_BEGIN2(name, type, ...)                                           \
    SAC_ND_DECL_FUN2 (name, type, __VA_ARGS__)                                           \
    {
#endif /* FUNAP_AS_CREATE */

#define SAC_MUTC_LOCAL_ALLOC__DESC__NOOP(var_NT, dim) SAC_NOOP ()

#define SAC_MUTC_LOCAL_ALLOC__DESC__FIXED(var_NT, dim)                                   \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim == SAC_ND_A_MIRROR_DIM (var_NT)),                          \
                         ("Inconsistant dimension for array %s found!",                  \
                          NT_STR (var_NT)));                                             \
        SAC_MUTC_LOCAL_MALLOC (SAC_ND_A_DESC (var_NT), BYTE_SIZE_OF_DESC (dim),          \
                               SAC_ND_DESC_TYPE (var_NT))                                \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
    }

#define SAC_MUTC_LOCAL_ALLOC__DESC__AUD(var_NT, dim)                                     \
    {                                                                                    \
        SAC_ASSURE_TYPE ((dim >= (0)),                                                   \
                         ("Illegal dimension for array %s found!", NT_STR (var_NT)));    \
        SAC_MUTC_LOCAL_MALLOC (SAC_ND_A_DESC (var_NT), BYTE_SIZE_OF_DESC (dim), int)     \
        SAC_TR_MEM_PRINT (("ND_ALLOC__DESC( %s, %s) at addr: %p", NT_STR (var_NT), #dim, \
                           SAC_ND_A_DESC (var_NT)))                                      \
        SAC_ND_A_DESC_DIM (var_NT) = SAC_ND_A_MIRROR_DIM (var_NT) = dim;                 \
    }

#if SAC_MUTC_DISABLE_THREAD_MEM
#define SAC_MUTC_THREAD_INIT_MALLOC
#define SAC_MUTC_THREAD_CLEANUP_MALLOC
#define SAC_MUTC_LOCAL_MALLOC(var, size, basetype) var = (basetype)malloc (size);
#else
#define SAC_MUTC_THREAD_INIT_MALLOC
#define SAC_MUTC_THREAD_CLEANUP_MALLOC
#define SAC_MUTC_LOCAL_MALLOC(var, size, basetype) var = (basetype)tls_malloc (size);
#endif

#if SAC_MUTC_THREAD_MALLOC

#ifdef SAC_HM_MALLOC
#undef SAC_HM_MALLOC
#endif
#define SAC_HM_MALLOC(var, size, basetype) SAC_MUTC_LOCAL_MALLOC (var, size, basetype)

#ifdef SAC_HM_FREE
#undef SAC_HM_FREE
#endif
#define SAC_HM_FREE(var) free (var)

#endif /* SAC_MUTC_THREAD_MALLOC */

#if 0
#define SAC_MUTC_THREAD_INIT SAC_MUTC_THREAD_INIT_MALLOC
#define SAC_MUTC_THREAD_CLEANUP SAC_MUTC_THREAD_CLEANUP_MALLOC
#else
#define SAC_MUTC_THREAD_INIT
#define SAC_MUTC_THREAD_CLEANUP
#endif

#define SAC_MUTC_INIT_SUBALLOC_DESC_DO(var_NT)                                           \
    SAC_ND_A_DESC_NAME (var_NT) = sl_getp (SAC_ND_A_DESC_NAME (var_NT));

#define SAC_MUTC_SPAWN_AP(syncid, place, name, ...)                                      \
    sl_spawn (NT_NAME (syncid), place, , , , , SAC_MUTC_FORCE_SPAWN_FLAGS, name,         \
              __VA_ARGS__);

#define SAC_MUTC_DECL_SYNCVAR(syncid) sl_spawndecl (syncid);

#define SAC_MUTC_SPAWNFUN_DECL2(...) sl_decl (__VA_ARGS__)

#define SAC_MUTC_DEF_SPAWNFUN_BEGIN2(name, anon, ...)                                    \
    sl_def (name, void, __VA_ARGS__)                                                     \
    {                                                                                    \
        SAC_MUTC_THREAD_INIT

#define SAC_MUTC_SPAWNFUN_DEF_END(...)                                                   \
    SAC_MUTC_THREAD_CLEANUP                                                              \
    }                                                                                    \
    sl_enddef

#define SAC_MUTC_SPAWNSYNC(syncid) sl_spawnsync (NT_NAME (syncid));

#endif /* BACKEND */
#undef MUTC
#endif /* _SAC_MUTC_H_ */
