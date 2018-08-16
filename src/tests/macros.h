// Some convenience macros.
#define make_vec_type(__x) TYmakeAKD (TYmakeSimpleType (__x), 1, SHmakeShape (0))
#define make_scalar_type(__x) TYmakeAKS (TYmakeSimpleType (__x), SHmakeShape (0))

#define int_akv_avis(__n, __v) TBmakeAvis (strdup (__n), \
        TYmakeAKV (TYmakeSimpleType (T_int), COmakeConstantFromInt (__v))) 
#define int_avis(__x) TBmakeAvis (strdup (__x), make_scalar_type (T_int))
#define int_vec_avis(__x) TBmakeAvis (strdup (__x), make_vec_type (T_int))

#define binary_prf(prf, arg1, arg2)                                                      \
    TBmakePrf (prf, TBmakeExprs (arg1, TBmakeExprs (arg2, NULL)))
#define make_let(avis, rhs) TBmakeLet (TBmakeIds (avis, NULL), rhs)

