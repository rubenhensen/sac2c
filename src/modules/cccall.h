/*
 *
 * $Log$
 * Revision 1.6  1997/03/19 13:46:23  cg
 * entirely re-implemented
 * now a global dependency tree is built up by import.c and readsib.c
 * So, linking should work now.
 *
 * Revision 1.5  1997/03/11  16:29:10  cg
 * new list of standard modules
 * old compiler option -deps ((updating makefile) no longer supported
 * use absolute pathnames for libstat
 *
 * Revision 1.4  1996/09/11  06:21:34  cg
 * Converted to new lib-file format.
 * Added facilities for updating makefiles with dependencies
 * and creating libstat information.
 *
 * Revision 1.3  1996/01/05  12:35:35  cg
 * added extern declaration for CreateLibrary
 *
 * Revision 1.2  1996/01/02  07:57:55  cg
 * first working revision
 *
 * Revision 1.1  1995/12/29  17:19:26  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_cccall_h

#define _sac_cccall_h

extern void InvokeCC ();
extern void CreateLibrary ();
extern void PrintLibStat ();

#endif /* _sac_cccall_h */
