/* $Id$ */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sac.h"

#define SAC_ARG_MAX_VARS_IN_DIR 100
#define CHAR_BUFFER_SIZE 128

/* Typenames used for output */
#define TYP_IFpr_str(str) str
char *basetype_string[] = {
#include "type_info.mac"
};

/* datatype for SAC_arg inUseDirectory */
typedef struct SAC_ARG_DIR_S {
    SAC_arg *vars[SAC_ARG_MAX_VARS_IN_DIR];
    int act_slot;
    struct SAC_ARG_DIR_S *next;
} in_use_directory_T;

static in_use_directory_T *GetNewInUseDirectory (in_use_directory_T *old);
static void AddToInUseDirectory (SAC_arg *sa);

static in_use_directory_T *in_use_directory;

/******************************************************************************
 *
 * function:
 *   SAC_arg *SAC_NewSACArg(SAC_ARG_simpletype basetype,char *tname,
 *                         int dim, int *shpvec)
 *
 * description:
 *   creates a SAC_arg data type of basetype and dimension with the specified
 *   shapevec, it DOES reuse the shpvec argument internally!
 *   it DOES NOT allocate memory for refcount and data!
 *
 *
 *
 ******************************************************************************/

SAC_arg *
SAC_CI_NewSACArg (SAC_ARG_simpletype basetype, char *tname, int dim, int *shpvec)
{
    SAC_arg *result;

    result = (SAC_arg *)SAC_MALLOC (sizeof (SAC_ARG_STRUCT));

    /* add to directory */
    AddToInUseDirectory (result);

    /* init structure */
    SAC_ARG_LRC (result) = 0;
    SAC_ARG_RC (result) = NULL;
    SAC_ARG_ELEMS (result) = NULL;
    SAC_ARG_TYPE (result) = basetype;
    SAC_ARG_DIM (result) = dim;
    SAC_ARG_TNAME (result) = tname;

    if (dim == 0) {
        /* create simple type */
        SAC_ARG_SHPVEC (result) = NULL; /* no shape needed */
    } else {
        /* create array tyoe */
        SAC_ARG_SHPVEC (result) = shpvec;
    }

    return (result);
}

/******************************************************************************
 *
 * function:
 *   SAC_arg *SAC_CreateSACArg(simpletype basetype, char *tname, int dim, ...)
 *
 * description:
 *   creates a SAC_arg data type of basetype and dimension with the specified
 *   shape by varargs, it does NOT allocate memory for refcount and data!
 *
 * remark:
 *   uses var_args and calls SAC_CI_NewSACArg()
 *
 ******************************************************************************/

SAC_arg *
SAC_CI_CreateSACArg (SAC_ARG_simpletype basetype, char *tname, int dim, ...)
{
    va_list Argp;
    int *shpvec;
    int i;

    if (dim == 0) {
        /* create simple type */
        shpvec = NULL; /* no shape needed */
    } else {
        /* create array tyoe */
        shpvec = (int *)SAC_MALLOC (dim * sizeof (int));
        va_start (Argp, dim);
        for (i = 0; i < dim; i++) {
            shpvec[i] = va_arg (Argp, int);
        }
    }

    return (SAC_CI_NewSACArg (basetype, tname, dim, shpvec));
}

/******************************************************************************
 *
 * function:
 *   bool SAC_CmpSACArgType(SAC_arg *sa, SAC_ARG_simpletype basetype,
 *                          char *tname, int dim, ...)
 *
 * description:
 *   compares SAC_arg argument with given basetype, dimension and shape
 *
 ******************************************************************************/

bool
SAC_CI_CmpSACArgType (SAC_arg *sa, SAC_ARG_simpletype basetype, char *tname, int dim, ...)
{
    va_list Argp;
    bool res = true;
    int i;

    /* check if equal user types or no user types at all */
    if (((SAC_ARG_TNAME (sa) == NULL) && (tname == 0))
        || ((SAC_ARG_TNAME (sa) != NULL) && (tname != NULL)
            && (strcmp (SAC_ARG_TNAME (sa), tname) == 0))) {
        if ((SAC_ARG_TYPE (sa) == basetype) && (SAC_ARG_DIM (sa) == dim)) {
            /* check shape */
            va_start (Argp, dim);
            for (i = 0; i < dim; i++) {
                if ((SAC_ARG_SHPVEC (sa))[i] != va_arg (Argp, int))
                    res = false;
            }
        } else {
            res = false;
        }
    } else {
        res = false;
    }
    return (res);
}

/******************************************************************************
 *
 * function:
 *   void SAC_FreeSACArg(SAC_arg *sa)
 *
 * description:
 *   frees SAC_arg data structure and used resources, if they have not been
 *   released yet.
 *
 ******************************************************************************/

static void
SAC_CI_FreeSACArg (SAC_arg *sa)
{
    if (sa != NULL) {
        /* free all allocated resources and the data structure */
        if (SAC_ARG_LRC (sa) > 0) {
            /* free data if arraytype */
            if ((SAC_ARG_ELEMS (sa) != NULL) && (SAC_ARG_DIM (sa) > 0))
                SAC_FREE (SAC_ARG_ELEMS (sa));
            if (SAC_ARG_RC (sa) != NULL)
                SAC_FREE (SAC_ARG_RC (sa));
        }

        /* if arraytype free shapevector and referencecounter*/
        if (SAC_ARG_DIM (sa) > 0) {
            SAC_FREE (SAC_ARG_SHPVEC (sa));
        }

        /*free SAC_arg *datastructure */
        SAC_FREE (sa);
    }
}

/******************************************************************************
 *
 * function:
 *   SAC_arg *SAC_CI_InitRefcounter(SAC_arg *sa, int initvalue)
 *
 * description:
 *   allocate refcounter variable and init refcounter and local refcounter
 *   with initvalue.
 *   returns the modified SAC_arg
 *
 ******************************************************************************/

SAC_arg *
SAC_CI_InitRefcounter (SAC_arg *sa, int initvalue)
{
    if (SAC_ARG_DIM (sa) > 0) {
        /* arraytype with refcounting */
        SAC_ARG_RC (sa) = (int *)SAC_MALLOC (sizeof (int));
        *SAC_ARG_RC (sa) = initvalue; /* init refcounter */
    } else {
        /* no refcounting for simple types */
        SAC_ARG_RC (sa) = NULL;
    }
    SAC_ARG_LRC (sa) = initvalue; /* init local refcounter */
    return (sa);
}

/******************************************************************************
 *
 * function:
 *   void SAC_CI_ExitOnInvalidArg(SAC_arg *sa, SAC_ARG_simpletype basetype,
 *                                char *tname int arg_mode)
 *
 * description:
 *   checks SAC_arg for valid content
 *   calls SAC_RuntimeError on error
 *
 ******************************************************************************/

void
SAC_CI_ExitOnInvalidArg (SAC_arg *sa, SAC_ARG_simpletype basetype, char *tname, int flag)
{
    if (SAC_ARG_LRC (sa) > 0) {
        /* no usertype checks */
        if (((SAC_ARG_TNAME (sa) == NULL) && (tname == 0))
            || ((SAC_ARG_TNAME (sa) != NULL) && (tname != NULL)
                && (strcmp (SAC_ARG_TNAME (sa), tname) == 0))) {
            /* check basetype */
            if (SAC_ARG_TYPE (sa) == basetype) {
                if (flag == SAC_CI_SIMPLETYPE && SAC_ARG_DIM (sa) != 0) {
                    SAC_RuntimeError ("SAC_Sac2XXX: access to array as simple type!\n");
                }
                if (flag == SAC_CI_ARRAYTYPE && SAC_ARG_DIM (sa) < 1) {
                    SAC_RuntimeError ("SAC_Sac2XXX: access to simple type as array!\n");
                }
            } else {
                SAC_RuntimeError ("SAC_Sac2XXX: access to wrong basetype!\n");
            }
        } else {
            SAC_RuntimeError ("SAC_Sac2XXX: access to user defined type!\n");
        }
    } else {
        SAC_RuntimeError ("SAC_Sac2XXX: access to invalid SAC_arg data,\n"
                          "maybe increase the reference counter!\n");
    }
}

/******************************************************************************
 *
 * function:
 *   void SAC_CI_InitSACArgDirectory()
 *
 * description:
 *   inits the directory of SAC_args used in the c programm
 *   sets up global variable
 *
 *****************************************************************************/

void
SAC_CI_InitSACArgDirectory ()
{
    in_use_directory = GetNewInUseDirectory (NULL);
}

/******************************************************************************
 *
 * function:
 *   in_use_directory_T *GetNewInUseDirectory(in_use_directory_T *old)
 *
 * description:
 *   inits datastructure and append it to existing directory
 *   return new directory
 *
 *****************************************************************************/

static in_use_directory_T *
GetNewInUseDirectory (in_use_directory_T *old)
{
    in_use_directory_T *result;
    result = (in_use_directory_T *)SAC_MALLOC (sizeof (in_use_directory_T));
    result->act_slot = 0;
    result->next = old;
    return (result);
}

/******************************************************************************
 *
 * function:
 *   static void AddToInUseDirectory(SAC_arg *sa)
 *
 * description:
 *   adds SAC_arg to directory; if no free slot available -> alloc new directory
 *
 *****************************************************************************/

static void
AddToInUseDirectory (SAC_arg *sa)
{
    if (in_use_directory->act_slot == SAC_ARG_MAX_VARS_IN_DIR) {
        /* directory full, append new one */
        in_use_directory = GetNewInUseDirectory (in_use_directory);
        in_use_directory->act_slot = 0;
    }
    (in_use_directory->vars)[in_use_directory->act_slot] = sa;
    (in_use_directory->act_slot)++;
}

/******************************************************************************
 *
 * function:
 *   void SAC_CI_FreeSACArgDirectory()
 *
 * description:
 *   Frees all SAC_args in the directory
 *   to be done before exit to clean up datastructures
 *
 *****************************************************************************/

void
SAC_CI_FreeSACArgDirectory ()
{
    int i;
    in_use_directory_T *act_dir;

    do {
        act_dir = in_use_directory;
        for (i = 0; i < act_dir->act_slot; i++) {
            SAC_CI_FreeSACArg ((act_dir->vars)[i]);
        }
        in_use_directory = in_use_directory->next;
        SAC_FREE (act_dir);
    } while (in_use_directory != NULL);
}

/******************************************************************************
 *
 * function:
 *   char*  SAC_CI_SACArg2string(SAC_arg *sa, char* buffer)
 *
 * description:
 *   creates a string representation for the SAC_arg type like 'int[1,2,3]'
 *   and places it in buffer. if buffer is NULL a new buffer is allocated.
 *
 *****************************************************************************/

char *
SAC_CI_SACArg2string (SAC_arg *sa, char *buffer)
{
    char *internal_buffer;
    char tempstr[CHAR_BUFFER_SIZE];
    int dim;

    if (buffer == NULL) {
        /* alloc new buffer */
        internal_buffer = (char *)SAC_MALLOC (CHAR_BUFFER_SIZE * sizeof (char));
    } else {
        /* use given buffer */
        internal_buffer = buffer;
    }
    internal_buffer[0] = '\0';

    if (sa != NULL) {
        /* fill buffer with type info */

        strcat (internal_buffer, basetype_string[SAC_ARG_TYPE (sa)]);
        if (SAC_ARG_DIM (sa) > 0) {
            strcat (internal_buffer, "[");
            for (dim = 0; dim < SAC_ARG_DIM (sa); dim++) {
                sprintf (tempstr, "%d", (SAC_ARG_SHPVEC (sa))[dim]);
                strcat (internal_buffer, tempstr);
                if (dim < (SAC_ARG_DIM (sa) - 1)) {
                    strcat (internal_buffer, ",");
                }
            }
            strcat (internal_buffer, "]");
        }

    } else {
        strcat (internal_buffer, "<na>");
    }
    return (internal_buffer);
}
