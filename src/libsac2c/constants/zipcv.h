/*
 * $Log$
 * Revision 1.7  2005/08/23 14:23:42  sbs
 * introduced the Char versions
 *
 * Revision 1.6  2004/11/23 11:38:17  cg
 * SacDevCamp renaming
 *
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
    extern void COzipCvUShort##fun (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos);                             \
    extern void COzipCvUInt##fun (void *arg1, int pos1, void *arg2, int pos2, void *res, \
                                  int res_pos);                                          \
    extern void COzipCvULong##fun (void *arg1, int pos1, void *arg2, int pos2,           \
                                   void *res, int res_pos);                              \
    extern void COzipCvByte##fun (void *arg1, int pos1, void *arg2, int pos2, void *res, \
                                  int res_pos);                                          \
    extern void COzipCvShort##fun (void *arg1, int pos1, void *arg2, int pos2,           \
                                   void *res, int res_pos);                              \
    extern void COzipCvInt##fun (void *arg1, int pos1, void *arg2, int pos2, void *res,  \
                                 int res_pos);                                           \
    extern void COzipCvLong##fun (void *arg1, int pos1, void *arg2, int pos2, void *res, \
                                  int res_pos);                                          \
                                                                                         \
    extern void COzipCvFloat##fun (void *arg1, int pos1, void *arg2, int pos2,           \
                                   void *res, int res_pos);                              \
    extern void COzipCvDouble##fun (void *arg1, int pos1, void *arg2, int pos2,          \
                                    void *res, int res_pos);                             \
    extern void COzipCvLongDouble##fun (void *arg1, int pos1, void *arg2, int pos2,      \
                                        void *res, int res_pos);                         \
                                                                                         \
    extern void COzipCvBool##fun (void *arg1, int pos1, void *arg2, int pos2, void *res, \
                                  int res_pos);                                          \
    extern void COzipCvChar##fun (void *arg1, int pos1, void *arg2, int pos2, void *res, \
                                  int res_pos);                                          \
                                                                                         \
    extern void COzipCvDummy##fun (void *arg1, int pos1, void *arg2, int pos2,           \
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
EXT_DECLS (Toby)
EXT_DECLS (Toi)
EXT_DECLS (Tof)
EXT_DECLS (Tod)
EXT_DECLS (Abs)
EXT_DECLS (Neg)

#undef EXT_DECLS

#endif /* _SAC_ZIPCV_H_ */
