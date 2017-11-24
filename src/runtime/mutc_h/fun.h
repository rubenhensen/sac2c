#ifndef _SAC_FUN_H_
#define _SAC_FUN_H_

/**
 * I (sbs) have no clue what the purpose of this file is.
 * It seems that this file is not even included in sac.h!
 * However, the definition of SAC_MUTC_ARG clashes with that 
 * of mutc_gen.h!
 * As we generically include all .h files into sac.h now,
 * I comment the entire body out....
 */

#if 0

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_DECL_FUN(fun) SAC_MUTC_DECL_THREAD_FUN (fun)
#else
#define SAC_MUTC_DECL_FUN(fun) fun
#endif

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_FUN_AP(ap)                                                              \
    {                                                                                    \
        create (ftmp;; 0; 0; 1;;;) ap;                                                   \
        sync (ftmp);                                                                     \
    }
#else
#define SAC_MUTC_FUN_AP(ap) ap
#endif

/*
 * #if SAC_MUTC_MACROS
 * #define SAC_MUTC_PARAM(type, name, var_NT) sl_glparm(type, name)
 * #else
 * #define SAC_MUTC_PARAM(type, name, var_NT) type name
 * #endif
 */

#if SAC_MUTC_MACROS
#define SAC_MUTC_ARG(type, name, var_NT) sl_glarg (type, name)
#else
#define SAC_MUTC_ARG(type, name, var_NT) name
#endif

#endif

#endif
