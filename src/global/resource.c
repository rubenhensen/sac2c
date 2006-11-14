/*
 *
 * $Log$
 * Revision 3.18  2005/06/01 12:47:45  sah
 * added lots of runtime paths
 *
 * Revision 3.17  2005/04/12 15:15:36  sah
 * cleaned up module system compiler args
 * and sac2crc parameters
 *
 * Revision 3.16  2005/03/10 09:41:09  cg
 * Adjusted RSCevaluateConfiguration() to compiler phase driver function
 * calling conventions.
 * Paths are now reset immediately.
 *
 * Revision 3.15  2005/01/11 11:28:11  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 3.14  2005/01/07 19:54:13  cg
 * Converted compile time output from Error.h to ctinfo.c
 *
 * Revision 3.13  2004/11/27 05:02:55  ktr
 * Some Bugfixes.
 *
 * Revision 3.12  2004/11/25 17:53:48  cg
 * SacDevCamp 04
 *
 * Revision 3.11  2004/11/22 19:24:35  cg
 * Moved all definitions/declarations of global variables to globals.mac
 *
 * Revision 3.10  2004/10/17 17:48:42  sah
 * added LD_DYNAMIC
 *
 * Revision 3.9  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.8  2003/03/24 16:37:04  sbs
 * opt_I added.
 *
 * Revision 3.7  2003/03/21 15:24:04  sbs
 * RSCParseResourceFile added
 *
 * Revision 3.6  2003/03/08 22:26:53  dkr
 * CCFLAGS of custom targets patched for new backend as well
 *
 * Revision 3.5  2003/02/11 16:42:47  dkr
 * CCFLAGS patched for new backend
 *
 * Revision 3.4  2001/11/29 13:24:05  sbs
 * LDFLAGS added
 *
 * Revision 3.3  2001/05/17 11:15:59  sbs
 * return value of Free used now 8-()
 *
 * Revision 3.2  2001/05/17 08:35:33  sbs
 * MALLOC/FREE eliminated
 *
 * Revision 3.1  2000/11/20 17:59:37  sacbase
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
#include "dbug.h"
#include "resource.h"
#include "scnprs.h"
#include "globals.h"
#include "ctinfo.h"
#include "internal_lib.h"
#include "filemgr.h"

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

  {"CC", str, &global.config.cc},
  {"CCFLAGS", str, &global.config.ccflags},
  {"CCDIR", str, &global.config.ccdir},
  {"LDFLAGS", str, &global.config.ldflags},
  {"CCLINK", str, &global.config.cclink},
  {"CCMTLINK", str, &global.config.ccmtlink},
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

    new = (inheritence_list_t *)ILIBmalloc (sizeof (inheritence_list_t));

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

    tmp = (resource_list_t *)ILIBmalloc (sizeof (resource_list_t));

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

    tmp = (target_list_t *)ILIBmalloc (sizeof (target_list_t));

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

            tmp->name = ILIBfree (tmp->name);
            tmp->value_str = ILIBfree (tmp->value_str);
            tmp = ILIBfree (tmp);
        }

        tmp_target->name = ILIBfree (tmp_target->name);

        inherit = tmp_target->super_targets;
        while (inherit != NULL) {
            tmp_inherit = inherit;
            inherit = inherit->next;
            tmp_inherit = ILIBfree (tmp_inherit);
        }

        tmp_target = ILIBfree (tmp_target);
    }

    DBUG_RETURN ((target_list_t *)NULL);
}

/******************************************************************************
 *
 * function:
 *  void RSCshowResources()
 *
 * description:
 *  This function displays the current configuration. It's called in main.c
 *  if the compilation process is stopped after setting up sac2c which can
 *  be achieved using the command line options -bu or -b1, respectively.
 *
 ******************************************************************************/

void
RSCshowResources ()
{
    int i;

    DBUG_ENTER ("RSCshowResources");

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

        CTInote ("Parsing configuration file \"%s\" ...", buffer);

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
    char buffer[MAX_FILE_NAME];
    char *envvar;
    bool ok;

    DBUG_ENTER ("ParseResourceFiles");

    /*
     * First, the public sac2crc file is read.
     * This file is mandatory for a correct installation.
     */

    envvar = getenv ("SACBASE");

    if (envvar == NULL) {
        CTIabort ("Unable to open public sac2crc file.\n"
                  "Probably, the environment variable SACBASE is not set.");
    }

    strcpy (buffer, envvar);
    strcat (buffer, "/runtime/sac2crc");

    ok = RSCparseResourceFile (buffer);

    if (!ok) {
        CTIabort ("Unable to parse public sac2crc file.\n"
                  "Probably, your installation is corrupted.");
    }

    /*
     * Second, the private sac2crc file ist read.
     * This file resides optionally in the user's home directory.
     */

    envvar = getenv ("HOME");

    if (envvar != NULL) {

        strcpy (buffer, envvar);
        strcat (buffer, "/.sac2crc");

        ok = RSCparseResourceFile (buffer);
    }

    global.filename = global.puresacfilename;

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

    while ((target != NULL) && (!ILIBstringCompare (target->name, "default"))) {
        target = target->next;
    }

    if (target == NULL) {
        CTIabort ("Configuration files do not contain default target specification");
    }

    for (i = 0; resource_table[i].name[0] != '\0'; i++) {

        resource = target->resource_list;

        while ((resource != NULL)
               && (!ILIBstringCompare (resource_table[i].name, resource->name))) {
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

    while ((tmp != NULL) && (!ILIBstringCompare (tmp->name, target))) {
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
               && (!ILIBstringCompare (resource_table[i].name, resource->name))) {
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
                        new = ILIBstringConcat3 (*((char **)(resource_table[i].store)),
                                                 " ", resource->value_str);
                        ILIBfree (*((char **)(resource_table[i].store)));
                        *((char **)(resource_table[i].store)) = new;
                    } else {
                        ILIBfree (*((char **)(resource_table[i].store)));
                        *((char **)(resource_table[i].store))
                          = ILIBstringCopy (resource->value_str);
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
 *   The non-obvious signature is to obey the compiler subphase standard.
 *
 ******************************************************************************/

node *
RSCevaluateConfiguration (node *syntax_tree)
{
    DBUG_ENTER ("RSCevaluateConfiguration");

    ParseResourceFiles ();

    EvaluateDefaultTarget (global.target_list);

    if (!ILIBstringCompare (global.target_name, "default")) {
        EvaluateCustomTarget (global.target_name, global.target_list);
    }

    global.target_list = FreeTargetList (global.target_list);

    FMGRsetupPaths ();

    DBUG_RETURN (syntax_tree);
}
