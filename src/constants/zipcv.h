/*
 * $Log$
 * Revision 1.5  2004/11/22 18:55:29  cg
 * Moved all definitions/declarations of global variables to globals.mac
 *
 * Revision 1.4  2004/11/22 11:27:04  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 1.3  2003/04/09 15:37:16  sbs
 * zipcv_neg added.
 *
 * Revision 1.2  2001/03/22 14:28:07  nmw
 * macros and function tables for primitive ari functions added
 *
 * Revision 1.1  2001/03/05 16:59:16  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_ZIPCV_H_
#define _SAC_ZIPCV_H_

#include "types.h"

#define EXT_DECLS(fun)                                                                   \
    extern void COZipCvUShort##fun (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos);                             \
    extern void COZipCvUInt##fun (void *arg1, int pos1, void *arg2, int pos2, void *res, \
                                  int res_pos);                                          \
    extern void COZipCvULong##fun (void *arg1, int pos1, void *arg2, int pos2,           \
                                   void *res, int res_pos);                              \
    extern void COZipCvShort##fun (void *arg1, int pos1, void *arg2, int pos2,           \
                                   void *res, int res_pos);                              \
    extern void COZipCvInt##fun (void *arg1, int pos1, void *arg2, int pos2, void *res,  \
                                 int res_pos);                                           \
    extern void COZipCvLong##fun (void *arg1, int pos1, void *arg2, int pos2, void *res, \
                                  int res_pos);                                          \
                                                                                         \
    extern void COZipCvFloat##fun (void *arg1, int pos1, void *arg2, int pos2,           \
                                   void *res, int res_pos);                              \
    extern void COZipCvDouble##fun (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos);                             \
    extern void COZipCvLongDouble##fun (void *arg1, int pos1, void *arg2, int pos2,      \
                                        void *res, int res_pos);                         \
                                                                                         \
    extern void COZipCvBool##fun (void *arg1, int pos1, void *arg2, int pos2, void *res, \
                                  int res_pos);                                          \
                                                                                         \
    extern void COZipCvDummy##fun (void *arg1, int pos1, void *arg2, int pos2,           \
                                   void *res, int res_pos);

EXT_DECLS (Plus)
EXT_DECLS (Minus)
EXT_DECLS (Mul)
EXT_DECLS (Div)
EXT_DECLS (Mod)
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
EXT_DECLS (Toi)
EXT_DECLS (Tof)
EXT_DECLS (Tod)
EXT_DECLS (Abs)
EXT_DECLS (Neg)

#endif /* _SAC_ZIPCV_H_ */
