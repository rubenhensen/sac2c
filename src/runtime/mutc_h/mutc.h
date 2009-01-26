#ifndef _SAC_MUTC_H_

#define _SAC_MUTC_H_

#define SAC_MUTC_DECL_INDEX(name) index name;

#define SAC_MUTC_DECL_SHARED(arg) shared arg

#define SAC_MUTC_DECL_FAMILY(name) family name;

#define SAC_MUTC_CREATE(family, place, start, end, inc, bs, ap)                          \
    create (family; place; start; end - 1; inc; bs;;) ap;

#define SAC_MUTC_SYNC(family) sync (family);

#endif
