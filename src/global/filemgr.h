***$Log$ *Revision 1.4 2004 / 11 / 22 16 : 14 : 46 ktr *SacDevCamp 04 * *Revision 1.3 2004
  / 11 / 22 16 : 05 : 32 cg *Moved macro definitions to globals.h **Revision 1.2 2004 / 11
  / 22 15 : 49 : 57 cg *Moved from subdirectory modules to global.**Revision 1.1 2004 / 11
  / 22 15 : 45 : 56 cg *Initial revision **Moved from subdirectory modules **
      Revision 3.4 2004
  / 10 / 11 16 : 55 : 48 sah *removes TempFileName again **Revision 3.3 2004 / 09
  / 21 16 : 32 : 42 sah *Added TempFileName **Revision 3.2 2003 / 03
  / 25 14 : 40 : 41 sah *added CheckSystemLibrary **Revision 3.1 2000 / 11
  / 20 18 : 00 : 52 sacbase *new release made **Revision 2.3 2000 / 11
  / 17 16 : 14 : 53 sbs *locationtype FindLocationOfFile (char *file) * added;
 *
 * Revision 2.2  1999/05/18 11:21:46  cg
 * added function CheckExistFile().
 *
 * Revision 2.1  1999/02/23 12:42:06  sacbase
 * new release made
 *
 * Revision 1.9  1998/03/17 12:14:24  cg
 * added resource SYSTEM_LIBPATH.
 * This makes the gcc special feature '--print-file-name' obsolete.
 * A fourth search path is used instead for system libraries.
 * This additional path may only be set via the sac2crc file,
 * but not by environment variables or command line parameters.
 *
 * Revision 1.8  1998/03/04 16:23:27  cg
 *  C compiler invocations and file handling converted to new
 * to usage of new  configuration files.
 *
 * Revision 1.7  1997/04/24 09:55:55  cg
 * standard library search paths relative to environment variable SACBASE
 * added in addition to user-defined paths
 *
 * Revision 1.6  1997/03/19  13:54:30  cg
 * Converted  to single tmp directory tmp_dirname instaed of build_dirnameand store_dirname
 *
 * Revision 1.5  1996/09/11  06:22:51  cg
 * Modified construction of paths.
 * Now: 1. paths added by command line option, 2. cwd, 3.shell variable
 *
 * Revision 1.4  1996/01/07  16:58:23  cg
 * handling of temporary directories modified.
 * cccall.c now only needs 2 of these
 *
 * Revision 1.3  1996/01/05  12:36:29  cg
 * added global variables tmp_dirname, store_dirname, build_dirname
 * and functions WriteOpen, CreateTmpDirectories, RemoveDirectory
 *
 * Revision 1.2  1995/04/05  17:24:16  sbs
 * GenLinkList inserted
 *
 * Revision 1.1  1994/12/11  17:33:27  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_FILEMGR_H_
#define _SAC_FILEMGR_H_

#include <stdio.h>
#include "types.h"

/******************************************************************************
 *
 * Filemanager
 *
 * Prefix: FMGR
 *
 *****************************************************************************/
extern char *FMGRfindFile( pathkind p, char *name);
 extern void FMGRinitPaths ();
 extern void FMGRappendPath (pathkind p, char *path);
 extern void FMGRrearrangePaths ();
 extern char *FMGRabsolutePathname (char *path);
 extern FILE *FMGRwriteOpen (char *format, ...);
 extern int FMGRcheckExistFile (char *dir, char *name);
 extern locationtype FMGRfindLocationOfFile (char *file);
 extern int FMGRcheckSystemLibrary (char *name);

#endif /* _SAC_FILEMGR_H_ */
