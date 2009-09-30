/*
 * $Id$
 *
 */

/*****************************************************************************
 *
 * file:   resource.c
 *
 * prefix: RSC
 *
 * description:
 *
 *  This file contains all code for customizing sac2c for specific target
 *  architectures including their operating system pecularities as well
 *  as their different C compilers.
 *
 *  The concrete behaviour of sac2c may be configured by using two different
 *  configuration files. There is an installation specific configuration
 *  file as well as a user specific on. While the former is mandatory, the
 *  latter is optional. The installation specific configuration file is
 *    $SAC2CBASE/include/sac2crc ,
 *  the user specific configuration file must be called .sac2crc and ought
 *  to reside in the user's home directory.
 *
 *  Each configuration file defines a sequence of target configuration or
 *  targets for short. A special target named 'default' is required which
 *  specifies default settings for all configurable resources. Other targets
 *  only need to specify the respective relevant subset of resources.
 *  A target may inherit resource specifications from other targets.
 *  This includes multiple inheritence.
 *  All targets implicitly inherit the default resources.
 *
 * Caution:
 *  sac2c will not check for recursively inheriting targets.
 *
 *  The sac2c command line option -target <name>
 *  allows to compile for specific architectures. If no particular target
 *  is specified the default target is used. If a target is chosen which
 *  is specified in the user specific configuration file as well as in the
 *  installation specific configuration file then the former specification
 *  applies.
 *
 *  This mechanism allows to adapt sac2c to different platforms more easily
 *  and more comfortably.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "dbug.h"
#include "resource.h"
#include "scnprs.h"
#include "globals.h"
#include "ctinfo.h"
#include "str.h"
#include "memory.h"

#include "sac.tab.h"

/******************************************************************************
 *
 * global variable:
 *  static struct {...} resource_table[]
 *
 * description:
 *  This static table defines the configurable resources. For each resource
 *  a triple of information is stored:
 *  1) the resource name,
 *  2) the resource type,
 *  3) the resource's storage location .
 *
 *  If you want to add any configurable resources, simply extend this table
 *  and the type definition of configuration_t which can be found in the file
 *  types.h
 *
 *  Make sure that the last resource in the table is always the dummy "".
 *
 ******************************************************************************/

static struct {
    char name[20];
    enum { num, str } tag;
    void *store;
} resource_table[] = {

  {"CC", str, &global.config.cc},
  {"CCFLAGS", str, &global.config.ccflags},
  {"CCDIR", str, &global.config.ccdir},
  {"LDFLAGS", str, &global.config.ldflags},
  {"CCLINK", str, &global.config.cclink},
  {"CCMTLINK", str, &global.config.ccmtlink},

  {"BACKEND", str, &global.config.backend},

  {"TREE_CC", str, &global.config.tree_cc},
  {"TREE_LD", str, &global.config.tree_ld},
  {"TREE_LD_PATH", str, &global.config.tree_ld_path},
  {"LIB_VARIANT", str, &global.config.lib_variant},

  {"OPT_O0", str, &global.config.opt_O0},
  {"OPT_O1", str, &global.config.opt_O1},
  {"OPT_O2", str, &global.config.opt_O2},
  {"OPT_O3", str, &global.config.opt_O3},
  {"OPT_g", str, &global.config.opt_g},
  {"OPT_D", str, &global.config.opt_D},
  {"OPT_I", str, &global.config.opt_I},

  {"CPP_STDIN", str, &global.config.cpp_stdin},
  {"CPP_FILE", str, &global.config.cpp_file},
  {"TAR_CREATE", str, &global.config.tar_create},
  {"TAR_EXTRACT", str, &global.config.tar_extract},
  {"AR_CREATE", str, &global.config.ar_create},
  {"LD_DYNAMIC", str, &global.config.ld_dynamic},
  {"GENPIC", str, &global.config.genpic},
  {"LD_PATH", str, &global.config.ld_path},
  {"RANLIB", str, &global.config.ranlib},
  {"MKDIR", str, &global.config.mkdir},
  {"RMDIR", str, &global.config.rmdir},
  {"CHDIR", str, &global.config.chdir},
  {"CAT", str, &global.config.cat},
  {"MOVE", str, &global.config.move},
  {"RSH", str, &global.config.rsh},
  {"DUMP_OUTPUT", str, &global.config.dump_output},

  {"LIBPATH", str, &global.config.libpath},
  {"IMPPATH", str, &global.config.imppath},
  {"EXTLIBPATH", str, &global.config.extlibpath},
  {"TMPDIR", str, &global.config.tmpdir},

  {"CACHE1_SIZE", num, &global.config.cache1_size},
  {"CACHE1_LINE", num, &global.config.cache1_line},
  {"CACHE1_ASSOC", num, &global.config.cache1_assoc},
  {"CACHE1_WRITEPOL", str, &global.config.cache1_writepol},
  {"CACHE1_MSCA", num, &global.config.cache1_msca_factor},

  {"CACHE2_SIZE", num, &global.config.cache2_size},
  {"CACHE2_LINE", num, &global.config.cache2_line},
  {"CACHE2_ASSOC", num, &global.config.cache2_assoc},
  {"CACHE2_WRITEPOL", str, &global.config.cache2_writepol},
  {"CACHE2_MSCA", num, &global.config.cache2_msca_factor},

  {"CACHE3_SIZE", num, &global.config.cache3_size},
  {"CACHE3_LINE", num, &global.config.cache3_line},
  {"CACHE3_ASSOC", num, &global.config.cache3_assoc},
  {"CACHE3_WRITEPOL", str, &global.config.cache3_writepol},
  {"CACHE3_MSCA", num, &global.config.cache3_msca_factor},

  {"", 0, NULL},
};

/*****************************************************************************
 *
 * function:
 *  inheritence_list_t *RSCmakeInheritenceListEntry( char *name,
 *                                                   inheritence_list_t *next)
 *
 * description:
 *
 *   creates inheritence list
 *
 *
 ******************************************************************************/

inheritence_list_t *
RSCmakeInheritenceListEntry (char *name, inheritence_list_t *next)
{
    inheritence_list_t *new;

    DBUG_ENTER ("RSCmakeInheritenceListEntry");

    new = (inheritence_list_t *)MEMmalloc (sizeof (inheritence_list_t));

    new->name = name;
    new->next = next;

    DBUG_RETURN (new);
}

/*****************************************************************************
 *
 * function:
 *  resource_list_t *RSCmakeResourceListEntry(char *name,
 *                                            char *value_str,
 *                                            int  value_num,
 *                                            int  add_flag,
 *                                            resource_list_t *next)
 *
 * description:
 *
 *  Along with RSCmakeTargetListEntry(), this function is used in sac.y
 *  to construct the tree-like structure for temporary storage
 *  of resource information.
 *
 *  The strings need not to be copied since they were already copied by sac.l.
 *
 ******************************************************************************/

resource_list_t *
RSCmakeResourceListEntry (char *name, char *value_str, int value_num, int add_flag,
                          resource_list_t *next)
{
    resource_list_t *tmp;

    DBUG_ENTER ("RSCmakeResourceListEntry");

    tmp = (resource_list_t *)MEMmalloc (sizeof (resource_list_t));

    tmp->name = name;
    tmp->value_str = value_str;
    tmp->value_num = value_num;
    tmp->add_flag = add_flag;

    tmp->next = next;

    DBUG_RETURN (tmp);
}

/*****************************************************************************
 *
 * function:
 *  resource_list_t *RSCmakeTargetListEntry(char *name,
 *                                          inheritence_list_t *super_targets,
 *                                          resource_list_t *resource_list,
 *                                          target_list_t *next)
 *
 * description:
 *
 *  Along with RSCmakeResourceListEntry(), this function is used in sac.y
 *  to construct the tree-like structure for temporary storage
 *  of resource information.
 *
 *  The strings need not to be copied since they were already copied by sac.l.
 *
 ******************************************************************************/

target_list_t *
RSCmakeTargetListEntry (char *name, inheritence_list_t *super_targets,
                        resource_list_t *resource_list, target_list_t *next)
{
    target_list_t *tmp;

    DBUG_ENTER ("RSCmakeTargetListEntry");

    tmp = (target_list_t *)MEMmalloc (sizeof (target_list_t));

    tmp->name = name;
    tmp->super_targets = super_targets;
    tmp->resource_list = resource_list;
    tmp->next = next;

    DBUG_RETURN (tmp);
}

/*****************************************************************************
 *
 * function:
 *  target_list_t *RSCaddTargetList(target_list_t *list1,
 *                                  target_list_t *list2)
 *
 * description:
 *
 * This function combines two target lists to a single one. It's used to
 * combine the targets read from the private configuration file with those read
 * from the public configuration file.
 *
 ******************************************************************************/

target_list_t *
RSCaddTargetList (target_list_t *list1, target_list_t *list2)
{
    target_list_t *tmp;

    DBUG_ENTER ("RSCaddTargetList");

    if (list1 == NULL) {
        list1 = list2;
    } else {
        tmp = list1;

        while (tmp->next != NULL) {
            tmp = tmp->next;
        }

        tmp->next = list2;
    }

    DBUG_RETURN (list1);
}

/******************************************************************************
 *
 * function:
 *  target_list_t *FreeTargetList(target_list_t *target)
 *
 * description:
 *  FreeTargetList() releases the dynamic structure pointed to by target_list.
 *  This is no longer needed if the correct configuration is determined and
 *  stored in config.
 *
 ******************************************************************************/

static target_list_t *
FreeTargetList (target_list_t *target)
{
    resource_list_t *resource, *tmp;
    target_list_t *tmp_target;
    inheritence_list_t *tmp_inherit, *inherit;

    DBUG_ENTER ("FreeTargetList");

    while (target != NULL) {
        tmp_target = target;
        target = target->next;

        resource = tmp_target->resource_list;
        while (resource != NULL) {
            tmp = resource;
            resource = resource->next;

            tmp->name = MEMfree (tmp->name);
            tmp->value_str = MEMfree (tmp->value_str);
            tmp = MEMfree (tmp);
        }

        tmp_target->name = MEMfree (tmp_target->name);

        inherit = tmp_target->super_targets;
        while (inherit != NULL) {
            tmp_inherit = inherit;
            inherit = inherit->next;
            tmp_inherit = MEMfree (tmp_inherit);
        }

        tmp_target = MEMfree (tmp_target);
    }

    DBUG_RETURN ((target_list_t *)NULL);
}

/******************************************************************************
 *
 * function:
 *  void PrintResources()
 *
 * description:
 *  This function displays the current configuration. It's called in main.c
 *  if the compilation process is stopped after setting up sac2c which can
 *  be achieved using the command line options -bu or -b1, respectively.
 *
 ******************************************************************************/

static void
PrintResources ()
{
    int i;

    DBUG_ENTER ("PrintResources");

    printf ("\nConfiguration for target '%s`:\n\n", global.target_name);

    for (i = 0; resource_table[i].name[0] != '\0'; i++) {
        switch (resource_table[i].tag) {
        case str:
            printf ("%-15s :=  \"%s\"\n", resource_table[i].name,
                    *((char **)(resource_table[i].store)));
            break;
        case num:
            printf ("%-15s :=  %d\n", resource_table[i].name,
                    *((int *)(resource_table[i].store)));
            break;
        default:
            CTIabort ("Internal data structure resource_table corrupted");
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *  void RSCparseResourceFile()
 *
 * description:
 *  This function parses a single resource file and adds its content to the
 *  global variables.
 *
 ******************************************************************************/

bool
RSCparseResourceFile (char *buffer)
{
    bool ok = TRUE;

    DBUG_ENTER ("RSCparseResourceFile");

    yyin = fopen (buffer, "r");

    if (yyin == NULL) {
        ok = FALSE;
    } else {
        if (global.print_resources) {
            CTIstate ("Parsing configuration file \"%s\" ...", buffer);
        }

        global.linenum = 1;
        global.filename = buffer; /* set for better error messages only */
        global.start_token = PARSE_RC;

        SPmyYyparse ();

        fclose (yyin);
    }

    DBUG_RETURN (ok);
}

/******************************************************************************
 *
 * function:
 *  void ParseResourceFiles()
 *
 * description:
 *  This function does all the file handling. First, the public configuration
 *  file is read and parsed. If existing, the private configuration file is
 *  parsed afterwards.
 *  This depends on the environment variables HOME and SACBASE.
 *
 ******************************************************************************/

static void
ParseResourceFiles ()
{
    char *filename;
    char *envvar;
    bool ok;

    DBUG_ENTER ("ParseResourceFiles");

    /*
     * First, the public sac2crc file is read.
     * This file is mandatory for a correct installation.
     */

    envvar = getenv ("SAC2CBASE");

    if (envvar == NULL) {
        CTIabort ("Unable to open public sac2crc file.\n"
                  "The environment variable SAC2CBASE is not set properly.");
    }

    filename = STRcat (envvar, "/sac2crc");

    ok = RSCparseResourceFile (filename);

    if (!ok) {
        CTIabort ("Unable to parse public sac2crc file.\n"
                  "Somewhat, your installation is corrupted.");
    }

    MEMfree (filename);

    /*
     * Second, the private sac2crc file ist read.
     * This file resides optionally in the user's home directory.
     */

    envvar = getenv ("HOME");

    if (envvar != NULL) {
        filename = STRcat (envvar, "/.sac2crc");
        ok = RSCparseResourceFile (filename);
        MEMfree (filename);
    }

    global.filename = global.puresacfilename; /* What is this good for ? */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *  void EvaluateDefaultTarget(target_list_t *target)
 *
 * description:
 *  This function traverses the target_list until the default target is found.
 *  Then the resource_table is stepped through one resource after the other.
 *  Each resource is looked for in the resource list of the default target
 *  found before. Upon a match, the particular resource configuration is stored
 *  in the static configuration structure config.
 *
 ******************************************************************************/

static void
EvaluateDefaultTarget (target_list_t *target)
{
    int i;
    resource_list_t *resource;

    DBUG_ENTER ("EvaluateDefaultTarget");

    while ((target != NULL) && (!STReq (target->name, "default"))) {
        target = target->next;
    }

    if (target == NULL) {
        CTIabort ("Configuration files do not contain default target specification");
    }

    for (i = 0; resource_table[i].name[0] != '\0'; i++) {

        resource = target->resource_list;

        while ((resource != NULL) && (!STReq (resource_table[i].name, resource->name))) {
            resource = resource->next;
        }

        if (resource == NULL) {
            CTIerror ("Default target specification of resource '%s` missing",
                      resource_table[i].name);
        } else {
            switch (resource_table[i].tag) {
            case str:
                if (resource->value_str == NULL) {
                    CTIabort ("Default target specification of resource '%s` illegal",
                              resource_table[i].name);
                }
                *((char **)(resource_table[i].store)) = resource->value_str;
                resource->value_str = NULL;
                break;

            case num:
                if (resource->value_str != NULL) {
                    CTIabort ("Default target specification of resource '%s` illegal",
                              resource_table[i].name);
                }
                *((int *)(resource_table[i].store)) = resource->value_num;
                break;

            default:
                CTIabort ("Internal data structure resource_table corrupted");
            }
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *  void EvaluateCustomTarget(char *target, target_list_t *target_list)
 *
 * description:
 *  This function traverses the target_list until the given target is found.
 *  Afterwards, the resource list of this target is traversed. Each resource
 *  found is looked for in the resource_table. If the resource is known and
 *  the type of its value is correct, the value is stored in the static
 *  configuration structure config. The previous default entry is replaced
 *  and released if necessary.
 *
 ******************************************************************************/

static void
EvaluateCustomTarget (char *target, target_list_t *target_list)
{
    target_list_t *tmp;
    resource_list_t *resource;
    int i;
    inheritence_list_t *super_target;

    DBUG_ENTER ("EvaluateCustomTarget");

    tmp = target_list;

    while ((tmp != NULL) && (!STReq (tmp->name, target))) {
        tmp = tmp->next;
    }

    if (tmp == NULL) {
        CTIabort ("Configuration files do not contain specification of custom "
                  "target '%s`",
                  target);
    }

    super_target = tmp->super_targets;

    while (super_target != NULL) {
        EvaluateCustomTarget (super_target->name, target_list);
        super_target = super_target->next;
    }

    resource = tmp->resource_list;

    while (resource != NULL) {
        i = 0;
        while ((resource_table[i].name[0] != '\0')
               && (!STReq (resource_table[i].name, resource->name))) {
            i++;
        }

        if (resource_table[i].name[0] == '\0') {
            CTIwarn ("Specification of target '%s` contains unrecognized resource '%s`",
                     target, resource->name);
        } else {
            switch (resource_table[i].tag) {
            case str:
                if (resource->value_str == NULL) {
                    CTIwarn ("Specification of target '%s` contains illegal value for "
                             "resource '%s`",
                             target, resource->name);
                } else {
                    if (resource->add_flag) {
                        char *new;
                        new = STRcatn (3, *((char **)(resource_table[i].store)), " ",
                                       resource->value_str);
                        MEMfree (*((char **)(resource_table[i].store)));
                        *((char **)(resource_table[i].store)) = new;
                    } else {
                        MEMfree (*((char **)(resource_table[i].store)));
                        *((char **)(resource_table[i].store))
                          = STRcpy (resource->value_str);
                    }
                }
                break;
            case num:
                if (resource->value_str != NULL) {
                    CTIwarn ("Specification of target '%s` contains illegal value for "
                             "resource '%s`",
                             target, resource->name);
                } else {
                    if (resource->add_flag) {
                        *((int *)(resource_table[i].store)) += resource->value_num;
                    } else {
                        *((int *)(resource_table[i].store)) = resource->value_num;
                    }
                }
                break;
            default:
                CTIabort ("Internal data structure resource_table corrupted");
            }
        }

        resource = resource->next;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *  void RSCevaluateConfiguration(char *target)
 *
 * description:
 *  This function triggers the whole process of evaluating the configuration
 *  files.
 *
 ******************************************************************************/

void
RSCevaluateConfiguration ()
{
    DBUG_ENTER ("RSCevaluateConfiguration");

    ParseResourceFiles ();

    EvaluateDefaultTarget (global.target_list);

    if (!STReq (global.target_name, "default")) {
        EvaluateCustomTarget (global.target_name, global.target_list);
    }

    global.target_list = FreeTargetList (global.target_list);

    if (global.print_resources) {
        PrintResources ();
        exit (0);
    }

    DBUG_VOID_RETURN;
}
