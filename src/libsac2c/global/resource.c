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
 *  The concrete behaviour of sac2c may be configured by using configuration
 *  files. There are 2 different kinds of configuration files:
 *  1) *system-files* which are generated when compiling and installing sac2c
 *  2) *customisation-files* which are either set up when downloading packages
 *     or which are set up manually
 *  While the former are mandatory, the latter are optional. The *system-files*
 *  are searched in 3 possible locations:
 *  1a) If there is an environment variable SAC2CRC it defines the first search
 *      option
 *  1b) The sac2c binary has its install prefix hard-encoded as second search
 *      option. However, this option is skipped if the current version is "dirty"
 *      ie it has been compiled from local sources that have not yet been
 *      committed to the git repo!
 *  1c) If the sac2c sources are present, the compiler also searches within its
 *      build directory!
 *
 *  The *customisation-files* reside in the user's home directory. They are
 *  either just a file ".sac2crc" or an entire directory ".sac2crc/" with
 *  files that match the pattern "sac2crc\..*". Typically, each package
 *  from sacbase adds one such file named sac2crc.<package>. It contains
 *  extensions to the search pathes for mod/tree files of the compiler.
 *
 *  Each configuration file defines a sequence of target configuration or
 *  targets for short. A special target named 'default' is required which
 *  specifies default settings for all configurable resources. Other targets
 *  only need to specify the respective relevant subset of resources.
 *  A target may inherit resource specifications from other targets.
 *  This includes multiple inheritence.
 *  All targets implicitly inherit the default resources.
 *  NOTE also, that all but the default target can be specified more than
 *  once! If that happens, the entries are processed in the order they have
 *  been parsed! This allows incremental definitions as used for the packages.
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
#include <limits.h>

#include "types.h"

#define DBUG_PREFIX "RSC"
#include "debug.h"

#include "resource.h"
#include "scnprs.h"
#include "globals.h"
#include "ctinfo.h"
#include "str.h"
#include "memory.h"
#include "sacdirs.h"
#include "filemgr.h"

#include "printable_target_functions.h"

//#include "sac.tab.h"

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

enum tag_t { num, str };

static struct {
    char name[20];
    enum tag_t tag;
    void *store;
} resource_table[] = {

#define DEF_RESOURCE(Name, Attr, Type1, Type2) {#Name, Type2, &global.config.Attr},
  DEF_RESOURCES_ALL
#undef DEF_RESOURCE

  {"", (enum tag_t)0, NULL},
};

/** <!--********************************************************************-->
 *
 * @fn void RSCprintConfigEntry( char *config)
 *
 * @brief Print out a single entry from the config file ( global.config. ...)
 *        where the entry is named 'config'
 *
 *****************************************************************************/

void
RSCprintConfigEntry (char *config)
{
    int i = 0;

    DBUG_ENTER ();

    for (i = 0; resource_table[i].name[0] != '\0'; i++) {
        if (STReq (resource_table[i].name, config)) {
            switch (resource_table[i].tag) {
            case str:
                printf ("%s\n", *((char **)resource_table[i].store));
                break;
            case num:
                printf ("%d\n", *((int *)resource_table[i].store));
                break;
            default:
                DBUG_UNREACHABLE ("Unknown type of config entry");
                break;
            }
            break;
        }
    }
    if (resource_table[i].name[0] == '\0') {
        CTIerror (EMPTY_LOC, "Resource %s unknown", config);
    }

    DBUG_RETURN ();
}

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
    inheritence_list_t *new_list;

    DBUG_ENTER ();

    new_list = (inheritence_list_t *)MEMmalloc (sizeof (inheritence_list_t));

    new_list->name = name;
    new_list->next = next;

    DBUG_RETURN (new_list);
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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
            tmp_inherit->name = MEMfree (tmp_inherit->name);
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

// static
void
PrintResources (void)
{
    int i;

    DBUG_ENTER ();

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
            CTIabort (EMPTY_LOC, "Internal data structure resource_table corrupted");
        }
    }

    DBUG_RETURN ();
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

    DBUG_ENTER ();

    yyin = fopen (buffer, "r");

    if (yyin == NULL) {
        ok = FALSE;
    } else {
        if (global.print_resources) {
            CTIstate ("Parsing configuration file \"%s\" ...", buffer);
        }

        global.linenum = 1;
        global.colnum = 1;
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
 *  void RSCsetSac2crcLocations()
 *
 * description:
 *   This function is meant to be called using dlsym (for instance in
 *   sactool.h), with it one can alter the global variables
 *   `global_sac2crc_location' and `build_sac2crc_location'.
 *
 *   The expected parameters are to have to from PATH + "sac2crc" + POSTFIX,
 *   e.g. `/usr/local/share/sac2c/VERSION/sac2crc_d' for debug sac2crc file.
 *
 *   These variables are used during runtime to locate the sac2crc file. Because
 *   we now allow for the user to compile the sac2c/sac2tex/sac4c/etc. binaries in
 *   order to `burn-in' there own install paths, we can no longer use the defuncted
 *   SAC2CRC_CONF and SAC2CRC_BUILD_CONF macros to search for the sac2crc file.
 *   These globals are meant to be modified by launch_function_from_library in
 *   sactools.h *before* we call the main function.
 *
 ******************************************************************************/

void
RSCsetSac2crcLocations (char *global_location, char *build_location)
{
    DBUG_ENTER ();

    global.global_sac2crc_location = global_location;
    global.build_sac2crc_location = build_location;

    DBUG_RETURN ();
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
 *  If we are of a `dirty' version, we only read from the build directory sac2crc
 *  file and sac2crc.local file.
 *  This depends on the environment variables HOME and SAC2CRC.
 *
 ******************************************************************************/

static void
MapParse (const char *path, const char *file, void *params)
{
    bool ok;
    char *filename;

    filename = STRcatn (3, path, "/", file);
    ok = RSCparseResourceFile (filename);
    MEMfree (filename);

    if (!ok) {
        CTIabort (EMPTY_LOC, "Error while parsing '%s'.", filename);
    }
}

static void
ParseResourceFiles (void)
{
    char *filename;
    char *envvar;
    bool is_dirty;
    bool ok;

    DBUG_ENTER ();

    /* If we are dirty, we should only read the sac2crc file from the build directory
     * otherwise try to read from env SAC2CRC, global install location, and then fallback
     * on build directory.
     */
    is_dirty = (bool)SAC2C_IS_DIRTY;

    /* If the configuration is done via SAC2CRC environment variable we assume
     * that it is a special case, parse the file and do no more file loading.
     */
    envvar = getenv ("SAC2CRC");

    if (envvar != NULL && FMGRcheckExistFile (envvar)) {
        ok = RSCparseResourceFile (envvar);
        if (!ok) {
            CTIabort (EMPTY_LOC, "Error while parsing '%s' (via SAC2CRC).", envvar);
        }
    } else if (!is_dirty && FMGRcheckExistFile (global.global_sac2crc_location)) {
        ok = RSCparseResourceFile (global.global_sac2crc_location);
        if (!ok) {
            CTIabort (EMPTY_LOC, "Error while parsing '%s'.", global.global_sac2crc_location);
        }
    } else {
        CTItell (4, "%sTrying to read sac2crc from %s.\n",
                 is_dirty ? "In a dirty state. " : "No global sac2crc file found. ",
                 global.build_sac2crc_location);

        /* We have to load the original sac2crc with all the targets.  */
        ok = RSCparseResourceFile (global.build_sac2crc_location);
        if (!ok) {
            CTIabort (EMPTY_LOC, "Error while parsing '%s'.", global.build_sac2crc_location);
        }
        /* And the sac2crc.local where pathes to libs and includes
         * are specified relatively to the build directory.
         */
        filename = STRcat (global.build_sac2crc_location, ".local");
        ok = RSCparseResourceFile (filename);
        if (!ok) {
            CTIabort (EMPTY_LOC, "Error while parsing '%s'.", filename);
        }

        MEMfree (filename);
    }

    /* Second, the private sac2crc file(s) is/are read.
     * This file resides optionally in the user's home directory.
     */
    envvar = getenv ("HOME");

    if (envvar != NULL) {
        filename = STRcat (envvar, "/.sac2crc");
        if (FMGRcheckExistDir (filename)) {
            char *sac2crc_pat = STRcatn (3, "sac2crc", SAC2C_POSTFIX, "\\..*");

            DBUG_PRINT ("local resource directory '%s` found", filename);
            FMGRforEach (filename, "sac2crc\\..*", NULL, MapParse);

            /* Depending on SAC2C_POSTFIX read in all files that start with
               "sac2crc_<postfix>".  */
            FMGRforEach (filename, sac2crc_pat, NULL, MapParse);
            MEMfree (sac2crc_pat);
        } else if (FMGRcheckExistFile (filename)) {
            DBUG_PRINT ("local resource file '%s` found", filename);
            ok = RSCparseResourceFile (filename);

            if (!ok) {
                CTIabort (EMPTY_LOC, "Error while parsing '%s'.", filename);
            }
        }
        MEMfree (filename);
    }

    global.filename = global.puresacfilename; /* What is this good for ? */

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * function:
 *  target_list_t * FindTarget( char *target_name, target_list_t *target)
 *
 * description:
 *  searches for the first occurrance of the specified target name within
 *  the list. If found, the corresponding pointer is returned; otherwise
 *  NULL is returned.
 *
 ******************************************************************************/
static target_list_t *
FindTarget (char *target_name, target_list_t *target)
{
    while ((target != NULL) && (!STReq (target->name, target_name))) {
        target = target->next;
    }
    return target;
}



/*******************************************************************************
 *
 * function:
 *  resource_list_t * FindResource( char * resource_name, resource_list_t *resource)
 *
 * description:
 *  searches for the first occurrance of the specified resource_name within
 *  the list. If found, the corresponding pointer is returned; otherwise
 *  NULL is returned.
 *
 ******************************************************************************/
static resource_list_t *
FindResource (char *resource_name, resource_list_t *resource)
{
    while ((resource != NULL) && (!STReq (resource_name, resource->name))) {
        resource = resource->next;
    }
    return resource;
}

/*******************************************************************************
 *
 * function:
 *  void UpdateResourceTable( int i, char *target_name,
 *                            resource_list_t *resource, bool allow_inc)
 *
 * description:
 *   updates the i-th entry in the global resource table with the provided
 *   resource.
 *   The allow_inc parameter signals whether the use of "+=" is permitted.
 *
 *
 ******************************************************************************/
static void
UpdateResourceTable (int i, char *target_name, resource_list_t *resource, bool allow_inc)
{
    switch (resource_table[i].tag) {
    case str:
        if (resource->value_str == NULL) {
            CTIabort (EMPTY_LOC, "'%s` target: specification of resource '%s` illegal", target_name,
                      resource_table[i].name);
        } else if (resource->add_flag) {
            if (!allow_inc) {
                CTIabort (EMPTY_LOC, 
                  "'%s` target: specification of '+=` on resource '%s` is illegal",
                  target_name, resource_table[i].name);
            } else {
                char *new_res;
                new_res
                  = STRcat (*((char **)(resource_table[i].store)), resource->value_str);
                MEMfree (*((char **)(resource_table[i].store)));
                *((char **)(resource_table[i].store)) = new_res;
            }
        } else {
            MEMfree (*((char **)(resource_table[i].store)));
            *((char **)(resource_table[i].store)) = STRcpy (resource->value_str);
        }
        break;

    case num:
        if (resource->value_str != NULL) {
            CTIabort (EMPTY_LOC, "'%s` target: specification of resource '%s` illegal", target_name,
                      resource_table[i].name);
        } else if (resource->add_flag) {
            if (!allow_inc) {
                CTIabort (EMPTY_LOC, 
                  "'%s` target: specification of '+=` on resource '%s` is illegal",
                  target_name, resource_table[i].name);
            } else {
                *((int *)(resource_table[i].store)) += resource->value_num;
            }
        } else {
            *((int *)(resource_table[i].store)) = resource->value_num;
        }
        break;

    default:
        CTIabort (EMPTY_LOC, "Internal data structure resource_table corrupted");
    }
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
 *  NOTE that this function is different from the CustomTarget version as the
 *  Default target has special requirements! It is required to have valid
 *  entries for ALL resources, it cannot inherit from any target, it cannot
 *  use the += form, and it must be uniquely defined!
 *
 ******************************************************************************/
static void
EvaluateDefaultTarget (target_list_t *target)
{
    int i;
    resource_list_t *resource;

    DBUG_ENTER ();

    target = FindTarget ("default", target);

    if (target == NULL) {
        CTIabort (EMPTY_LOC, "Configuration files do not contain default target specification");
    }

    if (target->super_targets != NULL) {
        CTIabort (EMPTY_LOC,
          "The default target specification must not inherit from any other target");
    }

    for (i = 0; resource_table[i].name[0] != '\0'; i++) {

        resource = FindResource (resource_table[i].name, target->resource_list);

        if (resource == NULL) {
            CTIerror (EMPTY_LOC, "Default target specification of resource '%s` missing",
                      resource_table[i].name);
        } else {
            UpdateResourceTable (i, "default", resource, FALSE);
        }
    }

    // Now we check for uniqueness of the default target:
    if (FindTarget ("default", target->next) != NULL) {
        CTIabort (EMPTY_LOC,
          "Configuration files contain more than one default target specification");
    }
    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *  void EvaluateCustomTarget(char *target, target_list_t *remaining_list,
 *                             target_list_t *target_list)
 *
 * description:
 *  This function traverses the target_list until the given target is found.
 *  Afterwards, the resource list of this target is traversed. Each resource
 *  found is looked for in the resource_table. If the resource is known and
 *  the type of its value is correct, the value is stored in the static
 *  configuration structure config. The previous default entry is replaced
 *  and released if necessary.
 *
 *  "Descendant" is only used when printing the targets, and exists to allow
 *  the SBI (even if inherited) to be determined. The default "placeholder"
 *  is a string of asterisks.
 *
 ******************************************************************************/

static void
EvaluateCustomTarget (char *target, target_list_t *remaining_list,
                      target_list_t *target_list)
{
    target_list_t *tmp;
    resource_list_t *resource;
    int i;
    inheritence_list_t *super_target;
    char *suffix;

    DBUG_ENTER ();

    tmp = FindTarget (target, remaining_list);

    if (tmp == NULL) {
        // This is only an error if we started with the whole list!
        if (remaining_list == target_list) {
            if (STReq (target, "arch_")) {
                CTIerror (EMPTY_LOC, 
                          "During the configuation of sac2c no CUDA architecture "
                          "was identified. Please use target 'cuda_<arch>` instead "
                          "of target 'cuda`; for example 'cuda_sm_60`.\n"
                          "Alternatively, you can provide a default "
                          "CUDA architecture by changing the definition "
                          "of 'target cuda` from "
                          "'target cuda :: cuda_core :: arch_:` to e.g. "
                          "'target cuda :: cuda_core :: arch_sm_60` or "
                          "by adding a target definition for 'target arch_`.");
            } else if (STRprefix ("cuda_", target)
                       || STRprefix ("arch_", target)) {
                suffix = STRsubStr (target, (size_t)5, (ssize_t)STRlen (target)-5);
                CTIerror (EMPTY_LOC,
                          "You are trying to compile for the CUDA architecture '%s`. "
                          "The configuration files do not contain "
                          "target '%s`.", suffix, target);
            } else {
                CTIerror (EMPTY_LOC, 
                          "The configuration files do not contain a specification of "
                          "target '%s`.", target);
            }
            CTIerrorContinued (EMPTY_LOC,
                               "You may choose a different target from the "
                               "compiler-specific configuration file '%s` or "
                               "your local ones in '~/.sac2crc/`. You may also add a new "
                               "target in a file in that folder, e.g. "
                               "in '~/.sac2crc/sac2crc.mine`.",
                               ( global.build_sac2crc_location != NULL ?
                                 global.build_sac2crc_location :
                                 global.global_sac2crc_location ));
            CTIabortOnError ();
        }
        DBUG_PRINT ("                   none found.");
    } else {

        DBUG_PRINT ("found target '%s`, looking for previous definitions...", target);
        // recursively look for potential "previous" definitions of the same target!
        EvaluateCustomTarget (target, tmp->next, target_list);

        DBUG_PRINT ("                 scanning inheritances...");
        // Evaluate inheritance targets left to right
        super_target = tmp->super_targets;
        while (super_target != NULL) {
            EvaluateCustomTarget (super_target->name, target_list, target_list);
            super_target = super_target->next;
        }
        DBUG_PRINT ("                 scanning inheritances done.");

        resource = tmp->resource_list;

        DBUG_PRINT ("                 scanning ressources...");
        while (resource != NULL) {
            i = 0;
            while ((resource_table[i].name[0] != '\0')
                   && (!STReq (resource_table[i].name, resource->name))) {
                i++;
            }

            if (resource_table[i].name[0] == '\0') {
                CTIwarn (
                  "Specification of target '%s` contains unrecognized resource '%s`",
                  target, resource->name);
            } else {
                UpdateResourceTable (i, target, resource, TRUE);
            }

            resource = resource->next;
        }
        DBUG_PRINT ("                 scanning resources done.");
    }

    DBUG_RETURN ();
}

/**
 * This function merely facilitates RSCevaluateConfiguration.
 */

void
EvaluateConfig (char *input, target_list_t *target_list)
{
    EvaluateDefaultTarget (global.target_list);

    if (!STReq (input, "default")) {
        char *target = STRtok (input, ":");
        while (target != NULL) {
            if (!STReq (target, "")) {
                // This process should allow for multiple colons.
                EvaluateCustomTarget (target, target_list, target_list);
            }

            // Prevent this from cluttering up the heap.
            target = MEMfree (target);

            target = STRtok (NULL, ":");
        }

        // Prevent this from cluttering up the heap.
        target = MEMfree (target);

    }
}

/******************************************************************************
 *
 * function:
 *  void RSCevaluateConfiguration ()
 *
 * description:
 *  This function triggers the whole process of evaluating the configuration
 *  files.
 *
 ******************************************************************************/

void
RSCevaluateConfiguration ()
{
    DBUG_ENTER ();

    ParseResourceFiles ();

    // This target gets evaluated twice if print_tagets is active.
    EvaluateConfig(global.target_name,global.target_list);

    if (global.print_resources) {
        // This is the scenario for the user printing ONE target's data.
        // Always the target that the compiler is using.
        PrintResources ();
        global.target_list = FreeTargetList (global.target_list);
        CTIexit (EXIT_SUCCESS);
    
    } else if (global.print_targets_and_exit) {
        target_list_t *temp = global.target_list;

        while (temp != NULL) {
            // Some targets may be evaluated multiple times :(
            // Readability at the cost of performance.
            EvaluateConfig (temp->name,global.target_list);

            // Now add this to the list of targets that will print.
            PTFappend (
              PTFmake
                (temp->name, global.config.sbi,
                    global.config.backend, global.config.target_env, NULL)
            );
            temp = temp->next;
        }

        // Now print the target list out and free the memory.
        PTFprint ();
        PTFfreeAll ();
        global.target_list = FreeTargetList (global.target_list);
        CTIexit (EXIT_SUCCESS);

    }

    // This should be freed even if no flags are active.
    global.target_list = FreeTargetList (global.target_list);

    DBUG_RETURN ();
}

void
xfree_configuration (configuration_t conf)
{
#define str(X)                                                                           \
    if (X)                                                                               \
        MEMfree (X);
#define num(X) /*nothing*/
#define DEF_RESOURCE(Name, Attr, Type1, Type2) Type2 (conf.Attr)
    DEF_RESOURCES_ALL
#undef DEF_RESOURCE
#undef str
#undef num
}

#undef DBUG_PREFIX

// vim: ts=2 sw=2 et:
