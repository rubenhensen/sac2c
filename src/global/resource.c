/*
 *
 * $Log$
 * Revision 2.3  1999/05/06 15:38:46  sbs
 * call of yyparse changed to My_yyparse.
 *
 * Revision 2.2  1999/03/31 08:50:49  cg
 * added new resource entries CACHEx_WRITEPOL
 *
 * Revision 2.1  1999/02/23 12:39:39  sacbase
 * new release made
 *
 * Revision 1.6  1998/11/19 16:12:36  cg
 * new configuration entry: CCMTLINK
 * specifies libraries to link with for multi-threaded programs only.
 * This makes the target 'par' obsolete.
 *
 * Revision 1.5  1998/07/07 13:41:49  cg
 * improved the resource management by implementing multiple inheritence
 * between targets
 *
 * Revision 1.4  1998/05/27 11:19:44  cg
 * global variable 'filename' which contains the current file name in order
 * to provide better error messages is now handled correctly.
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
 *    $SACBASE/runtime/sac2crc ,
 *  the user specific configuration file must be called .sac2crc and ought
 *  to reside in the user's home directory.
 *  A hint for sac2c developers: the installation specific configuration file
 *  may actually be found in the directory $RCSROOT/src/runtime/ .
 *  A symbolic link from the above mentioned directory to this location is
 *  required.
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
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "dbug.h"
#include "resource.h"
#include "scnprs.h"
#include "filemgr.h"
#include "free.h"
#include "globals.h"
#include "Error.h"

/******************************************************************************
 *
 * global variable:
 *  target_list_t *target_list
 *
 * description:
 *  When the configuration files are parsed, a dynamic tree like structure
 *  is generated and its root is stored in target_list. This dynamic structure
 *  is afterwards processed, the relevant information is stored in the static
 *  structure config, and the target_list is finally released.
 *
 ******************************************************************************/

target_list_t *target_list;

/******************************************************************************
 *
 * global variable:
 *  configuration_t config
 *
 * This global variable permanently stores the desired configuration.
 *
 ******************************************************************************/

configuration_t config;

/******************************************************************************
 *
 * global variable:
 *  struct {...} resource_table[]
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
 *  resource.h
 *
 *  Make sure that the last resource in the table is always the dummy "".
 *
 ******************************************************************************/

static struct {
    char name[20];
    enum { num, str } tag;
    void *store;
} resource_table[] = {

  {"CC", str, &config.cc},
  {"CCFLAGS", str, &config.ccflags},
  {"CCDIR", str, &config.ccdir},
  {"CCLINK", str, &config.cclink},
  {"CCMTLINK", str, &config.ccmtlink},
  {"OPT_O0", str, &config.opt_O0},
  {"OPT_O1", str, &config.opt_O1},
  {"OPT_O2", str, &config.opt_O2},
  {"OPT_O3", str, &config.opt_O3},
  {"OPT_g", str, &config.opt_g},
  {"OPT_D", str, &config.opt_D},

  {"CPP_STDIN", str, &config.cpp_stdin},
  {"CPP_FILE", str, &config.cpp_file},
  {"TAR_CREATE", str, &config.tar_create},
  {"TAR_EXTRACT", str, &config.tar_extract},
  {"AR_CREATE", str, &config.ar_create},
  {"RANLIB", str, &config.ranlib},
  {"MKDIR", str, &config.mkdir},
  {"RMDIR", str, &config.rmdir},
  {"CHDIR", str, &config.chdir},
  {"CAT", str, &config.cat},
  {"MOVE", str, &config.move},
  {"DUMP_OUTPUT", str, &config.dump_output},

  {"STDLIB_DECPATH", str, &config.stdlib_decpath},
  {"STDLIB_LIBPATH", str, &config.stdlib_libpath},

  {"SYSTEM_LIBPATH", str, &config.system_libpath},

  {"CACHE1_SIZE", num, &config.cache1_size},
  {"CACHE1_LINE", num, &config.cache1_line},
  {"CACHE1_ASSOC", num, &config.cache1_assoc},
  {"CACHE1_WRITEPOL", str, &config.cache1_writepol},

  {"CACHE2_SIZE", num, &config.cache2_size},
  {"CACHE2_LINE", num, &config.cache2_line},
  {"CACHE2_ASSOC", num, &config.cache2_assoc},
  {"CACHE2_WRITEPOL", str, &config.cache2_writepol},

  {"CACHE3_SIZE", num, &config.cache3_size},
  {"CACHE3_LINE", num, &config.cache3_line},
  {"CACHE3_ASSOC", num, &config.cache3_assoc},
  {"CACHE3_WRITEPOL", str, &config.cache3_writepol},

  {"", 0, NULL},
};

/*****************************************************************************
 *
 * function:
 *  resource_list_t *RSCMakeResourceListEntry(char *name,
 *                                            char *value_str,
 *                                            int  value_num,
 *                                            int  add_flag,
 *                                            resource_list_t *next)
 *
 * description:
 *
 *  Along with RSCMakeTargetListEntry(), this function is used in sac.y
 *  to construct the tree-like structure for temporary storage
 *  of resource information.
 *
 *  The strings need not to be copied since they were already copied by sac.l.
 *
 ******************************************************************************/

resource_list_t *
RSCMakeResourceListEntry (char *name, char *value_str, int value_num, int add_flag,
                          resource_list_t *next)
{
    resource_list_t *tmp;

    DBUG_ENTER ("MakeResourceListEntry");

    tmp = (resource_list_t *)Malloc (sizeof (resource_list_t));

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
 *  resource_list_t *RSCMakeTargetListEntry(char *name, ids *super_targets,
 *                                          resource_list_t *resource_list,
 *                                          target_list_t *next)
 *
 * description:
 *
 *  Along with RSCMakeResourceListEntry(), this function is used in sac.y
 *  to construct the tree-like structure for temporary storage
 *  of resource information.
 *
 *  The strings need not to be copied since they were already copied by sac.l.
 *
 ******************************************************************************/

target_list_t *
RSCMakeTargetListEntry (char *name, ids *super_targets, resource_list_t *resource_list,
                        target_list_t *next)
{
    target_list_t *tmp;

    DBUG_ENTER ("MakeTargetListEntry");

    tmp = (target_list_t *)Malloc (sizeof (target_list_t));

    tmp->name = name;
    tmp->super_targets = super_targets;
    tmp->resource_list = resource_list;
    tmp->next = next;

    DBUG_RETURN (tmp);
}

/*****************************************************************************
 *
 * function:
 *  target_list_t *RSCAddTargetList(target_list_t *list1,
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
RSCAddTargetList (target_list_t *list1, target_list_t *list2)
{
    target_list_t *tmp;

    DBUG_ENTER ("AddTargetList");

    if (list1 == NULL) {
        list1 = list2;
    } else {
        tmp = list1;

        while (tmp->next != NULL)
            tmp = tmp->next;

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

    DBUG_ENTER ("FreeTargetList");

    while (target != NULL) {
        tmp_target = target;
        target = target->next;

        resource = tmp_target->resource_list;
        while (resource != NULL) {
            tmp = resource;
            resource = resource->next;

            FREE (tmp->name);
            FREE (tmp->value_str);
            FREE (tmp);
        }
        FREE (tmp_target->name);
        if (tmp_target->super_targets != NULL) {
            FreeAllIds (tmp_target->super_targets);
        }

        FREE (tmp_target);
    }

    DBUG_RETURN ((target_list_t *)NULL);
}

/******************************************************************************
 *
 * function:
 *  void RSCShowResources()
 *
 * description:
 *  This function displays the current configuration. It's called in main.c
 *  if the compilation process is stopped after setting up sac2c which can
 *  be achieved using the command line options -bu or -b1, respectively.
 *
 ******************************************************************************/

void
RSCShowResources ()
{
    int i;

    DBUG_ENTER ("ShowResources");

    printf ("\nConfiguration for target '%s`:\n\n", target_name);

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
            SYSABORT (("Internal data structure resource_table corrupted"));
        }
    }

    DBUG_VOID_RETURN;
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
    char buffer[MAX_FILE_NAME];
    char *envvar;

    DBUG_ENTER ("ParseResourceFiles");

    /*
     * First, the public sac2crc file is read.
     * This file is mandatory for a correct installation.
     */

    envvar = getenv ("SACBASE");

    if (envvar == NULL) {
        SYSABORT (("Unable to open public sac2crc file.\n"
                   "Probably, your installation is corrupted."));
    }

    strcpy (buffer, envvar);
    strcat (buffer, "/runtime/sac2crc");

    yyin = fopen (buffer, "r");

    if (yyin == NULL) {
        SYSABORT (("Unable to open public sac2crc file.\n"
                   "Probably, your installation is corrupted."));
    }

    NOTE (("  Parsing public configuration file \"%s\" ...", buffer));

    linenum = 1;
    filename = "sac2crc"; /* set for better error messages only */
    start_token = PARSE_RC;

    My_yyparse ();

    fclose (yyin);

    /*
     * Second, the private sac2crc file ist read.
     * This file resides optionally in the user's home directory.
     */

    envvar = getenv ("HOME");

    if (envvar != NULL) {

        strcpy (buffer, envvar);
        strcat (buffer, "/.sac2crc");

        yyin = fopen (buffer, "r");

        if (yyin != NULL) {

            NOTE (("  Parsing private configuration file \"%s\" ...", buffer));

            linenum = 1;
            filename = ".sac2crc"; /* set for better error messages only */
            start_token = PARSE_RC;
            My_yyparse ();

            fclose (yyin);
        }
    }

    filename = puresacfilename;

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

    while ((target != NULL) && (0 != strcmp (target->name, "default")))
        target = target->next;

    if (target == NULL) {
        SYSABORT (("Configuration files do not contain default target specification"));
    }

    for (i = 0; resource_table[i].name[0] != '\0'; i++) {

        resource = target->resource_list;

        while ((resource != NULL)
               && (0 != strcmp (resource_table[i].name, resource->name))) {
            resource = resource->next;
        }

        if (resource == NULL) {
            SYSERROR (("Default target specification of resource '%s` missing",
                       resource_table[i].name));
        } else {
            switch (resource_table[i].tag) {
            case str:
                if (resource->value_str == NULL) {
                    SYSABORT (("Default target specification of resource '%s` illegal",
                               resource_table[i].name));
                }
                *((char **)(resource_table[i].store)) = resource->value_str;
                resource->value_str = NULL;
                break;

            case num:
                if (resource->value_str != NULL) {
                    SYSABORT (("Default target specification of resource '%s` illegal",
                               resource_table[i].name));
                }
                *((int *)(resource_table[i].store)) = resource->value_num;
                break;

            default:
                SYSABORT (("Internal data structure resource_table corrupted"));
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
    ids *super_target;

    DBUG_ENTER ("EvaluateCustomTarget");

    tmp = target_list;

    while ((tmp != NULL) && (0 != strcmp (tmp->name, target)))
        tmp = tmp->next;

    if (tmp == NULL) {
        SYSABORT (
          ("Configuration files do not contain specification of custom target '%s`",
           target));
    }

    super_target = tmp->super_targets;

    while (super_target != NULL) {
        EvaluateCustomTarget (IDS_NAME (super_target), target_list);
        super_target = IDS_NEXT (super_target);
    }

    resource = tmp->resource_list;

    while (resource != NULL) {
        i = 0;
        while ((resource_table[i].name[0] != '\0')
               && (0 != strcmp (resource_table[i].name, resource->name))) {
            i++;
        }

        if (resource_table[i].name[0] == '\0') {
            SYSWARN (("Specification of target '%s` contains unrecognized resource '%s`",
                      target, resource->name));
        } else {
            switch (resource_table[i].tag) {
            case str:
                if (resource->value_str == NULL) {
                    SYSWARN (("Specification of target '%s` contains illegal value for "
                              "resource '%s`",
                              target, resource->name));
                } else {
                    if (resource->add_flag) {
                        char *new;
                        new = (char *)Malloc (
                          strlen (resource->value_str)
                          + strlen (*((char **)(resource_table[i].store))) + 2);
                        strcpy (new, *((char **)(resource_table[i].store)));
                        strcat (new, " ");
                        strcat (new, resource->value_str);
                        FREE (*((char **)(resource_table[i].store)));
                        *((char **)(resource_table[i].store)) = new;
                    } else {
                        FREE (*((char **)(resource_table[i].store)));
                        *((char **)(resource_table[i].store))
                          = StringCopy (resource->value_str);
                    }
                }
                break;
            case num:
                if (resource->value_str != NULL) {
                    SYSWARN (("Specification of target '%s` contains illegal value for "
                              "resource '%s`",
                              target, resource->name));
                } else {
                    if (resource->add_flag) {
                        *((int *)(resource_table[i].store)) += resource->value_num;
                    } else {
                        *((int *)(resource_table[i].store)) = resource->value_num;
                    }
                }
                break;
            default:
                SYSABORT (("Internal data structure resource_table corrupted"));
            }
        }

        resource = resource->next;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *  void RSCEvaluateConfiguration(char *target)
 *
 * description:
 *  This function triggers the whole process of evaluating the configuration
 *  files.
 *
 ******************************************************************************/

void
RSCEvaluateConfiguration (char *target)
{
    DBUG_ENTER ("RSCEvaluateConfiguration");

    ParseResourceFiles ();

    EvaluateDefaultTarget (target_list);

    if (0 != strcmp (target, "default")) {
        EvaluateCustomTarget (target, target_list);
    }

    target_list = FreeTargetList (target_list);

    DBUG_VOID_RETURN;
}
