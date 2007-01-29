/*
 *
 * $Log$
 * Revision 1.6  2005/06/16 09:48:48  sbs
 * changed TYPE_ERROR into DISPATCH_ERROR
 *
 * Revision 1.5  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 1.4  2003/09/17 12:56:28  dkr
 * postfix _any renamed into _ANY
 *
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

#ifndef _SAC_ICM2C_ERROR_H_
#define _SAC_ICM2C_ERROR_H_

extern void ICMCompileDISPATCH_ERROR (int cnt_to, char **to_ANY, char *funname,
                                      int cnt_from, char **from_ANY);

#endif /* _SAC_ICM2C_ERROR_H_ */
