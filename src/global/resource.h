/*
 *
 * $Log$
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
 * description:
 *  This file contains type definitions, global variable declarations,
 *  as well as function prototypes which are used for dealing with
 *  sac2crc resource definition files. These allow for customization
 *  of sac2c for various different hardware architectures, operating
 *  systems, and C compilers.
 *
 ******************************************************************************/

#ifndef SAC_RESOURCE_H

#define SAC_RESOURCE_H

/*****************************************************************************
 *
 * type: resource_list_t
 * type: target_list_t
 *
 * description:
 *  These types are used to build up a tree-like structure for temporaily
 *  storing all information read in from sac2crc files.
 *
 ******************************************************************************/

typedef struct resource_list_t {
    char *name;
    char *value_str;
    int value_num;
    struct resource_list_t *next;
} resource_list_t;

typedef struct target_list_t {
    char *name;
    resource_list_t *resource_list;
    struct target_list_t *next;
} target_list_t;

/*****************************************************************************
 *
 * type: resource_t
 *
 * description:
 *  This structure is used to permanently store all relevant resource
 *  information for the selected target.
 *
 ******************************************************************************/

typedef struct {
    char *cc;
    char *ccflags;
    char *ccdir;
    char *cclink;
    char *opt_O0;
    char *opt_O1;
    char *opt_O2;
    char *opt_O3;
    char *opt_g;
    char *opt_D;

    char *cpp_stdin;
    char *cpp_file;
    char *tar_create;
    char *tar_extract;
    char *ar_create;
    char *ranlib;
    char *mkdir;
    char *rmdir;
    char *chdir;
    char *cat;
    char *move;
    char *dump_output;

    char *stdlib_decpath;
    char *stdlib_libpath;

    int cache1_size;
    int cache1_line;
    int cache1_assoc;

    int cache2_size;
    int cache2_line;
    int cache2_assoc;

    int cache3_size;
    int cache3_line;
    int cache3_assoc;
} configuration_t;

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

extern resource_list_t *RSCMakeResourceListEntry (char *resource, char *value_str,
                                                  int value_num, resource_list_t *next);

extern target_list_t *RSCMakeTargetListEntry (char *target,
                                              resource_list_t *resource_list,
                                              target_list_t *next);

extern target_list_t *RSCAddTargetList (target_list_t *list1, target_list_t *list2);

extern void RSCShowResources ();

extern void RSCEvaluateConfiguration (char *target);

#endif /* SAC_RESOURCE_H */
