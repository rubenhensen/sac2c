/*
 *
 * $Log$
 * Revision 3.6  2004/11/22 15:42:55  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 3.5  2004/10/17 17:48:42  sah
 * added LD_DYNAMIC
 *
 * Revision 3.4  2003/03/24 16:37:04  sbs
 * opt_I added.
 *
 * Revision 3.3  2003/03/21 15:24:04  sbs
 * RSCParseResourceFile added
 *
 * Revision 3.2  2001/11/29 13:24:05  sbs
 * LDFLAGS added
 *
 * Revision 3.1  2000/11/20 17:59:38  sacbase
 * new release made
 *
 * Revision 2.6  2000/02/03 17:20:55  cg
 * Added new resource table entries CACHE[123]_MSCA
 *
 * Revision 2.5  1999/06/10 09:47:56  cg
 * added new resource entry RSH for the specification of how to
 * invoke a remote shell.
 *
 * Revision 2.4  1999/05/18 12:29:10  cg
 * added new resource entry TMPDIR to specify where sac2c puts
 * its temporary files.
 *
 * Revision 2.3  1999/04/01 13:27:24  cg
 * new cache write policy resource entry added to resource data type
 *
 * Revision 2.2  1999/03/31 08:50:49  cg
 * added new resource entries CACHEx_WRITEPOL
 *
 * Revision 2.1  1999/02/23 12:39:42  sacbase
 * new release made
 *
 * Revision 1.5  1998/11/19 16:12:36  cg
 * new configuration entry: CCMTLINK
 * specifies libraries to link with for multi-threaded programs only.
 * This makes the target 'par' obsolete.
 *
 * Revision 1.4  1998/07/07 13:41:49  cg
 * improved the resource management by implementing multiple inheritence
 * between targets
 *
 * Revision 1.3  1998/03/17 12:14:24  cg
 * added resource SYSTEM_LIBPATH.
 * This makes the gcc special feature '--print-file-name' obsolete.
 * A fourth search path is used instead for system libraries.
 * This additional path may only be set via the sac2crc file,
 * but not by environment variables or command line parameters.
 *
 * Revision 1.2  1998/03/04 16:23:27  cg
 *  C compiler invocations and file handling converted to new
 * to usage of new  configuration files.
 *
 * Revision 1.1  1998/02/27 10:04:47  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file: resource.h
 *
 * Prefix: RSC
 *
 * description:
 *
 *  This file contains type definitions, global variable declarations,
 *  as well as function prototypes which are used for dealing with
 *  sac2crc resource definition files. These allow for customization
 *  of sac2c for various different hardware architectures, operating
 *  systems, and C compilers.
 *
 ******************************************************************************/

#ifndef _SAC_RESOURCE_H_
#define _SAC_RESOURCE_H_

#include "types.h"

/*****************************************************************************
 *
 * Declarations of global variables
 *
 ******************************************************************************/

extern target_list_t *target_list;

extern configuration_t config;

/*****************************************************************************
 *
 * Prototypes of functions
 *
 ******************************************************************************/

extern bool RSCparseResourceFile (char *file);

extern resource_list_t *RSCmakeResourceListEntry (char *resource, char *value_str,
                                                  int value_num, int add_flag,
                                                  resource_list_t *next);

extern target_list_t *RSCmakeTargetListEntry (char *target, node *super_targets,
                                              resource_list_t *resource_list,
                                              target_list_t *next);

extern target_list_t *RSCaddTargetList (target_list_t *list1, target_list_t *list2);

extern void RSCshowResources ();

extern void RSCevaluateConfiguration (char *target);

#endif /* _SAC_RESOURCE_H_ */
