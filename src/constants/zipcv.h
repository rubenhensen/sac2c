/*
 * $Log$
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
    extern void COZipCvDummy##fun (void *arg1, int pos1, void *arg2, int pos2,           \
                                   void *res, int res_pos);

EXT_DECLS (Plus)
EXT_DECLS (Minus)
EXT_DECLS (Mul)
EXT_DECLS (Div)

#endif /* _zipcv_h */
