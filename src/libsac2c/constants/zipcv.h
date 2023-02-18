#ifndef _SAC_ZIPCV_H_
#define _SAC_ZIPCV_H_

#include "types.h"

#define EXT_DECLS(fun)                                                                         \
    extern void COzipCvUByte##fun (void *arg1, size_t pos1, void *arg2, size_t pos2,           \
                                   void *res, size_t pes_pos);                                 \
    extern void COzipCvUShort##fun (void *arg1, size_t pos1, void *arg2, size_t pos2,          \
                                    void *res, size_t pes_pos);                                \
    extern void COzipCvUInt##fun (void *arg1, size_t pos1, void *arg2, size_t pos2, void *res, \
                                  size_t pes_pos);                                             \
    extern void COzipCvULong##fun (void *arg1, size_t pos1, void *arg2, size_t pos2,           \
                                   void *res, size_t pes_pos);                                 \
    extern void COzipCvULongLong##fun (void *arg1, size_t pos1, void *arg2, size_t pos2,       \
                                       void *res, size_t pes_pos);                             \
    extern void COzipCvByte##fun (void *arg1, size_t pos1, void *arg2, size_t pos2, void *res, \
                                  size_t res_pos);                                             \
    extern void COzipCvShort##fun (void *arg1, size_t pos1, void *arg2, size_t pos2,           \
                                   void *res, size_t pes_pos);                                 \
    extern void COzipCvInt##fun (void *arg1, size_t pos1, void *arg2, size_t pos2, void *res,  \
                                 size_t pes_pos);                                              \
    extern void COzipCvLong##fun (void *arg1, size_t pos1, void *arg2, size_t pos2, void *res, \
                                  size_t pes_pos);                                             \
    extern void COzipCvLongLong##fun (void *arg1, size_t pos1, void *arg2, size_t pos2,        \
                                      void *res, size_t pes_pos);                              \
                                                                                               \
    extern void COzipCvFloat##fun (void *arg1, size_t pos1, void *arg2, size_t pos2,           \
                                   void *res, size_t pes_pos);                                 \
    extern void COzipCvDouble##fun (void *arg1, size_t pos1, void *arg2, size_t pos2,          \
                                    void *res, size_t pes_pos);                                \
    extern void COzipCvLongDouble##fun (void *arg1, size_t pos1, void *arg2, size_t pos2,      \
                                        void *res, size_t pes_pos);                            \
                                                                                               \
    extern void COzipCvBool##fun (void *arg1, size_t pos1, void *arg2, size_t pos2, void *res, \
                                  size_t pes_pos);                                             \
    extern void COzipCvChar##fun (void *arg1, size_t pos1, void *arg2, size_t pos2, void *res, \
                                  size_t pes_pos);                                             \
                                                                                               \
    extern void COzipCvDummy##fun (void *arg1, size_t pos1, void *arg2, size_t pos2,           \
                                   void *res, size_t pes_pos);

EXT_DECLS (Plus)
EXT_DECLS (Minus)
EXT_DECLS (Mul)
EXT_DECLS (Div)
EXT_DECLS (Mod)
EXT_DECLS (AplMod)
EXT_DECLS (Min)
EXT_DECLS (Max)
EXT_DECLS (And)
EXT_DECLS (Or)
EXT_DECLS (Eq)
EXT_DECLS (Neq)
EXT_DECLS (Le)
EXT_DECLS (Lt)
EXT_DECLS (Gt)
EXT_DECLS (Ge)
EXT_DECLS (Not)
EXT_DECLS (Toub)
EXT_DECLS (Tous)
EXT_DECLS (Toui)
EXT_DECLS (Toul)
EXT_DECLS (Toull)
EXT_DECLS (Tob)
EXT_DECLS (Tos)
EXT_DECLS (Toc)
EXT_DECLS (Tobool)
EXT_DECLS (Toi)
EXT_DECLS (Tol)
EXT_DECLS (Toll)
EXT_DECLS (Tof)
EXT_DECLS (Tod)
EXT_DECLS (Abs)
EXT_DECLS (Neg)

#undef EXT_DECLS

#endif /* _SAC_ZIPCV_H_ */
