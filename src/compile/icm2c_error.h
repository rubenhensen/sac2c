/*
 *
 * $Log$
 * Revision 1.3  2002/10/10 23:52:36  dkr
 * signature of TYPE_ERROR modified
 *
 * Revision 1.2  2002/09/09 14:24:58  dkr
 * signature of ICMCompileTYPE_ERROR modified
 *
 * Revision 1.1  2002/09/09 14:18:51  dkr
 * Initial revision
 *
 */

#ifndef _icm2c_error_h_
#define _icm2c_error_h_

extern void ICMCompileTYPE_ERROR (int cnt_to, char **to_any, char *funname, int cnt_from,
                                  char **from_any);

#endif /* _icm2c_error_h_ */
