/*
 *
 * $Log$
 * Revision 1.1  1995/08/30 13:50:23  cg
 * Initial revision
 *
 *
 */

/* Some macros for comparing types, names, modules, etc. */

#define MOD(a) (NULL == a) ? "" : a

#define CMP_TYPE_ID(a, b)                                                                \
    ((NULL == a->id_mod)                                                                 \
       ? ((!strcmp (a->id, b->id)) && (NULL == b->id_mod))                               \
       : ((NULL == b->id_mod)                                                            \
            ? 0                                                                          \
            : ((!strcmp (a->id, b->id)) && (!strcmp (a->id_mod, b->id_mod)))))

#define CMP_TYPE_USER(a, b)                                                              \
    ((NULL == a->name_mod)                                                               \
       ? ((!strcmp (a->name, b->name)) && (NULL == b->name_mod))                         \
       : ((NULL == b->name_mod)                                                          \
            ? 0                                                                          \
            : ((!strcmp (a->name, b->name)) && (!strcmp (a->name_mod, b->name_mod)))))

#define CMP_TYPE_HIDDEN(a, b)                                                            \
    ((NULL == a->name)                                                                   \
       ? ((NULL == b->name)                                                              \
            ? 0                                                                          \
            : (!(strcmp (a->id, b->name) && CMP_MOD (a->id_mod, b->name_mod))))          \
       : ((NULL == b->name)                                                              \
            ? (!(strcmp (a->name, b->id) && CMP_MOD (a->name_mod, b->id_mod)))           \
            : (!(strcmp (a->name, b->name) && CMP_MOD (a->name_mod, b->name_mod)))))

#define CMP_FUN_ID(a, b) CMP_TYPE_ID (a, b)

#define CMP_MOD(a, b) ((NULL == a) ? (NULL == b) : ((NULL == b) ? 0 : (!strcmp (a, b))))

#define CMP_TYPEDEF(a, b) CMP_TYPE_ID (a->info.types, b->info.types)

#define CMP_OBJDEF(a, b) CMP_TYPE_ID (a->info.types, b->info.types)

#define CMP_FUNDEF(a, b)                                                                 \
    ((CMP_FUN_ID (a->info.types, b->info.types)) ? (CmpDomain (a->node[2], b->node[2]))  \
                                                 : 0)
