#ifndef _SAC_MUTC_H_

#define _SAC_MUTC_H_

#if SAC_MUTC_MACROS
#define SAC_MUTC_DECL_INDEX(name) sl_index (name);
#else
#define SAC_MUTC_DECL_INDEX(name) index name;
#endif

#if SAC_MUTC_MACROS
#define SAC_MUTC_DECL_FAMILY(name)
#else
#define SAC_MUTC_DECL_FAMILY(name) family name;
#endif

#if SAC_MUTC_MACROS
#define SAC_MUTC_DECL_THREAD_FUN(a) thread a
#else
#define SAC_MUTC_DECL_THREAD_FUN(a) a /* todo */
#endif

#if SAC_MUTC_MACROS
#define SAC_MUTC_CREATE(family, place, start, end, inc, bs, ap)                          \
    sl_create (family, place, start, end, inc, bs, , ap);
#else
#define SAC_MUTC_CREATE(family, place, start, end, inc, bs, ap)                          \
    create (family; place; start; end - 1; inc; bs;;) ap;
#endif

#if SAC_MUTC_MACROS
sl_sync (family);
#else
#define SAC_MUTC_SYNC(family) sync (family);
#endif

#endif /* _SAC_MUTC_H_ */
