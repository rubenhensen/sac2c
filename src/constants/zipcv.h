/*
 * $Log$
 * Revision 1.2  2001/03/22 14:28:07  nmw
 * macros and function tables for primitive ari functions added
 *
 * Revision 1.1  2001/03/05 16:59:16  sbs
 * Initial revision
 *
 *
 */

#ifndef _zipcv_h
#define _zipcv_h

typedef void (*zipcvfunptr) (void *, int, void *, int, void *, int);

extern zipcvfunptr zipcv_plus[];
extern zipcvfunptr zipcv_minus[];
extern zipcvfunptr zipcv_mul[];
extern zipcvfunptr zipcv_div[];
extern zipcvfunptr zipcv_mod[];
extern zipcvfunptr zipcv_min[];
extern zipcvfunptr zipcv_max[];
extern zipcvfunptr zipcv_and[];
extern zipcvfunptr zipcv_or[];
extern zipcvfunptr zipcv_eq[];
extern zipcvfunptr zipcv_neq[];
extern zipcvfunptr zipcv_le[];
extern zipcvfunptr zipcv_lt[];
extern zipcvfunptr zipcv_gt[];
extern zipcvfunptr zipcv_ge[];
extern zipcvfunptr zipcv_not[];
extern zipcvfunptr zipcv_toi[];
extern zipcvfunptr zipcv_tof[];
extern zipcvfunptr zipcv_tod[];
extern zipcvfunptr zipcv_abs[];

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

#endif /* _zipcv_h */
