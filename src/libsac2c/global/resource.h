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
 * Prototypes of functions
 *
 ******************************************************************************/
extern void RSCprintConfigEntry (char *config);
extern bool RSCparseResourceFile (char *file);
extern void RSCsetSac2crcLocations (char *global_location, char *build_location);

extern inheritence_list_t *RSCmakeInheritenceListEntry (char *name,
                                                        inheritence_list_t *next);

extern resource_list_t *RSCmakeResourceListEntry (char *resource, char *value_str,
                                                  int value_num, int add_flag,
                                                  resource_list_t *next);

extern target_list_t *RSCmakeTargetListEntry (char *target,
                                              inheritence_list_t *super_targets,
                                              resource_list_t *resource_list,
                                              target_list_t *next);

extern target_list_t *RSCaddTargetList (target_list_t *list1, target_list_t *list2);

extern void RSCevaluateConfiguration (void);

extern void xfree_configuration (configuration_t conf);

#endif /* _SAC_RESOURCE_H_ */
