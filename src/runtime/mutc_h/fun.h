#ifndef _SAC_FUN_H_

#define _SAC_FUN_H_

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_DECL_FUN(fun) thread fun
#else
#define SAC_MUTC_DECL_FUN(fun) fun
#endif

#if SAC_MUTC_FUNAP_AS_CREATE
#define SAC_MUTC_FUN_AP(ap)                                                              \
    {                                                                                    \
        family ftmp;                                                                     \
        create (ftmp;; 0; 0; 1;;;) ap;                                                   \
        sync (ftmp);                                                                     \
    }
#else
#define SAC_MUTC_FUN_AP(ap) ap
#endif

#endif
