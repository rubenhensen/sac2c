/*
 * $Log$
 * Revision 2.11  1999/06/03 13:40:36  her
 * added the line "self & cross int." to the output of SAC_CS_ShowResults.
 *
 * Revision 2.10  1999/05/20 14:16:29  cg
 * All simulation parameters may now be set dynamically, including
 * global/blocked simulation.
 *
 * Revision 2.9  1999/05/10 10:57:54  her
 * SAC_CS_CheckArguments got a new parameter: profilinglevel
 *
 * Revision 2.8  1999/05/06 14:05:45  her
 * eliminated THE segmentation fault caused by pde1-64
 *
 * Revision 2.7  1999/04/26 11:55:34  her
 * modifications for the piped-cachesimulation
 *
 * Revision 2.5  1999/04/14 09:23:15  cg
 * Cache simulation may now be triggered by pragmas.
 *
 * Revision 2.4  1999/04/12 09:40:54  cg
 * Bug removed in initialization of write access function table.
 * Presentation of results immproved.
 *
 * Revision 2.3  1999/04/06 13:45:39  cg
 * added extended setup of cache parameters.
 *
 * Revision 2.2  1999/03/26 14:34:21  her
 * new functions SAC_CS_Start and SAC_CS_Stop added
 *
 * Revision 2.1  1999/02/23 12:43:36  sacbase
 * new release made
 *
 * Revision 1.1  1999/02/17 17:25:18  her
 * Initial revision
 *
 */

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "sac_cachesim.h"
#include "libsac_cachesim.h"
#include "sac_message.h"

#include "main_args.h"

/*
 * The following defines are copied from globals.h for usage in function
 * SAC_CS_CheckArguments().
 */
#define CACHESIM_NO 0x0000
#define CACHESIM_YES 0x0001
#define CACHESIM_ADVANCED 0x0002
#define CACHESIM_FILE 0x0004
#define CACHESIM_PIPE 0x0008
#define CACHESIM_IMMEDIATE 0x0010
#define CACHESIM_BLOCK 0x0020

/*
 * BEGIN: The folowing variables are declared as extern in the
 * ´libsac_cachesim_access.c´-file. Changes here have to be done there
 * too!!!
 */
tCacheLevel *SAC_CS_cachelevel[MAX_CACHELEVEL + 1]; /* SAC_CS_cachelevel[0] is unused */

ULINT SAC_CS_hit[MAX_CACHELEVEL + 1], SAC_CS_invalid[MAX_CACHELEVEL + 1],
  SAC_CS_miss[MAX_CACHELEVEL + 1], SAC_CS_cold[MAX_CACHELEVEL + 1],
  SAC_CS_cross[MAX_CACHELEVEL + 1], SAC_CS_self[MAX_CACHELEVEL + 1];
/* SAC_CS_XXX[0] is unused */

int SAC_CS_level = 1;
FILE *SAC_CS_pipehandle;

char SAC_CS_separator[]
  = "==============================================================\n";

void (*SAC_CS_Finalize) (void);
void (*SAC_CS_RegisterArray) (void * /*baseaddress*/, int /*size*/);
void (*SAC_CS_UnregisterArray) (void * /*baseaddress*/);
void (*SAC_CS_Start) (char * /*tag*/);
void (*SAC_CS_Stop) (void);

tFunRWAccess SAC_CS_ReadAccess, SAC_CS_WriteAccess,
  SAC_CS_read_access_table[MAX_CACHELEVEL + 2],
  SAC_CS_write_access_table[MAX_CACHELEVEL + 2];
/* SAC_CS_xxx_access_table[0] is unused,
   SAC_CS_xxx_access_table[MAX_CACHELEVEL+1] for dummy/MainMem */
/* END: */
static int sim_incarnation = 0;
static char starttag[MAX_TAG_LENGTH];
static tProfilingLevel profiling_level;
static int global_simulation;

/* forward declarations */
static void Finalize (void);
static void RegisterArray (void *baseaddress, int size /* in byte */);
static void UnregisterArray (void *baseaddress);
static void Start (char *tag);
static void Stop (void);

static void Piped_Finalize (void);
static void Piped_RegisterArray (void *baseaddress, int size /* in byte */);
static void Piped_UnregisterArray (void *baseaddress);
static void Piped_ReadAccess (void *baseaddress, void *elemaddress);
static void Piped_WriteAccess (void *baseaddress, void *elemaddress);
static void Piped_Start (char *tag);
static void Piped_Stop (void);

void SAC_CS_Access_MM (void *baseaddress, void *elemaddress);

void SAC_CS_Access_DMRead_S (void *baseaddress, void *elemaddress);
void SAC_CS_Access_DMFOW_S (void *baseaddress, void *elemaddress);
void SAC_CS_Access_DMWV_S (void *baseaddress, void *elemaddress);
void SAC_CS_Access_DMWA_S (void *baseaddress, void *elemaddress);
void SAC_CS_Access_AS4Read_S (void *baseaddress, void *elemaddress);
void SAC_CS_Access_AS4FOW_S (void *baseaddress, void *elemaddress);
void SAC_CS_Access_AS4WV_S (void *baseaddress, void *elemaddress);
void SAC_CS_Access_AS4WA_S (void *baseaddress, void *elemaddress);

void SAC_CS_Access_DMRead_D (void *baseaddress, void *elemaddress);
void SAC_CS_Access_DMFOW_D (void *baseaddress, void *elemaddress);
void SAC_CS_Access_DMWV_D (void *baseaddress, void *elemaddress);
void SAC_CS_Access_DMWA_D (void *baseaddress, void *elemaddress);
void SAC_CS_Access_AS4Read_D (void *baseaddress, void *elemaddress);
void SAC_CS_Access_AS4FOW_D (void *baseaddress, void *elemaddress);
void SAC_CS_Access_AS4WV_D (void *baseaddress, void *elemaddress);
void SAC_CS_Access_AS4WA_D (void *baseaddress, void *elemaddress);

/*
 * int fastlog2(int value)
 *
 * if value < 1 or value > 67108864=65536K=64M
 * or there exists no n with power(2,n)==value
 * then fastlog2 returns -1. Otherwise fastlog2 returns n.
 */
static int
fastlog2 (int value)
{
    switch (value) {
    case 1:
        return (0);
    case 2:
        return (1);
    case 4:
        return (2);
    case 8:
        return (3);
    case 16:
        return (4);
    case 32:
        return (5);
    case 64:
        return (6);
    case 128:
        return (7);
    case 256:
        return (8);
    case 512:
        return (9);
    case 1024:
        return (10);
    case 2048:
        return (11);
    case 4096:
        return (12);
    case 8192:
        return (13);
    case 16384:
        return (14);
    case 32768:
        return (15);
    case 65536:
        return (16);
    case 131072:
        return (17);
    case 262144:
        return (18);
    case 524288:
        return (19);
    case 1048576:
        return (20);
    case 2097152:
        return (21);
    case 4194304:
        return (22);
    case 8388608:
        return (23);
    case 16777216:
        return (24);
    case 33554432:
        return (25);
    case 67108864:
        return (26);

    default:
        return (-1);
    }
} /* fastlog2 */

static char *
WritePolicyName (tWritePolicy wpol)
{
    switch (wpol) {
    case SAC_CS_default:
        return ("default");
    case SAC_CS_fetch_on_write:
        return ("fetch_on_write");
    case SAC_CS_write_validate:
        return ("write_validate");
    case SAC_CS_write_around:
        return ("write_around");
    default:
        return ("");
    }
} /* WritePolicyName */

static char *
WritePolicyShortName (tWritePolicy wpol)
{
    switch (wpol) {
    case SAC_CS_default:
        return ("d");
    case SAC_CS_fetch_on_write:
        return ("f");
    case SAC_CS_write_validate:
        return ("v");
    case SAC_CS_write_around:
        return ("a");
    default:
        return ("");
    }
} /* WritePolicyShortName */

static char *
ProfilingLevelName (tProfilingLevel level)
{
    switch (level) {
    case SAC_CS_default:
        return ("default");
    case SAC_CS_simple:
        return ("simple");
    case SAC_CS_advanced:
        return ("advanced");
    case SAC_CS_piped_simple:
        return ("piped_simple");
    case SAC_CS_piped_advanced:
        return ("piped_advanced");
    case SAC_CS_file:
        return ("file");
    default:
        return ("none");
    }
} /* ProfilingLevelName */

static char *
ProfilingLevelShortName (tProfilingLevel level)
{
    switch (level) {
    case SAC_CS_simple:
        return ("is");
    case SAC_CS_advanced:
        return ("ia");
    case SAC_CS_piped_simple:
        return ("ps");
    case SAC_CS_piped_advanced:
        return ("pa");
    case SAC_CS_file:
        return ("f");
    case SAC_CS_none:
        return ("n");
    default:
        return ("");
    }
} /* ProfilingLevelShortName */

static void
ResetCacheParms (char *spec, unsigned long int *cachesize, int *cachelinesize,
                 int *associativity, tWritePolicy *writepolicy)
{
    char *field, *full_spec;

    full_spec = (char *)malloc (strlen (spec) + 1);
    strcpy (full_spec, spec);

    field = strtok (spec, "/");

    if (field == NULL) {
        SAC_RuntimeError ("Invalid cache parameter specification: '%s`.", full_spec);
    }

    *cachesize = atoi (field);

    field = strtok (NULL, "/");

    if (field != NULL) {
        *cachelinesize = atoi (field);

        field = strtok (NULL, "/");

        if (field != NULL) {
            *associativity = atoi (field);

            field = strtok (NULL, "/");

            if (field != NULL) {
                if (field[1] == '\0') {
                    switch (field[0]) {
                    case 'd':
                        *writepolicy = SAC_CS_default;
                        break;
                    case 'f':
                        *writepolicy = SAC_CS_fetch_on_write;
                        break;
                    case 'v':
                        *writepolicy = SAC_CS_write_validate;
                        break;
                    case 'a':
                        *writepolicy = SAC_CS_write_around;
                        break;
                    default:
                        SAC_RuntimeError ("Invalid cache parameter specification: '%s`.",
                                          full_spec);
                    }
                } else {
                    SAC_RuntimeError ("Invalid cache parameter specification: '%s`.",
                                      full_spec);
                }
            }
        }
    }

    free (full_spec);
}

void
SAC_CS_CheckArguments (int argc, char *argv[], tProfilingLevel *profilinglevel,
                       int *cs_global, unsigned long int *cachesize1, int *cachelinesize1,
                       int *associativity1, tWritePolicy *writepolicy1,
                       unsigned long int *cachesize2, int *cachelinesize2,
                       int *associativity2, tWritePolicy *writepolicy2,
                       unsigned long int *cachesize3, int *cachelinesize3,
                       int *associativity3, tWritePolicy *writepolicy3)
{
    unsigned int cachesim = CACHESIM_NO;

    switch (*profilinglevel) {
    case SAC_CS_simple:
        cachesim |= CACHESIM_IMMEDIATE;
        break;
    case SAC_CS_advanced:
        cachesim |= CACHESIM_IMMEDIATE | CACHESIM_ADVANCED;
        break;
    case SAC_CS_piped_simple:
        cachesim |= CACHESIM_PIPE;
        break;
    case SAC_CS_piped_advanced:
        cachesim |= CACHESIM_PIPE | CACHESIM_ADVANCED;
        break;
    case SAC_CS_file:
        cachesim |= CACHESIM_FILE;
        break;
    case SAC_CS_none:
        break;
    default:
        break;
    }

    ARGS_BEGIN (argc, argv);

    ARGS_OPTION ("cs1", {
        ResetCacheParms (ARG, cachesize1, cachelinesize1, associativity1, writepolicy1);
    });

    ARGS_OPTION ("cs2", {
        ResetCacheParms (ARG, cachesize2, cachelinesize2, associativity2, writepolicy2);
    });

    ARGS_OPTION ("cs3", {
        ResetCacheParms (ARG, cachesize3, cachelinesize3, associativity3, writepolicy3);
    });

    ARGS_OPTION ("cs", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('s', cachesim &= ~CACHESIM_ADVANCED);
        ARG_FLAGMASK ('a', cachesim |= CACHESIM_ADVANCED);
        ARG_FLAGMASK ('g', *cs_global = 1);
        ARG_FLAGMASK ('b', *cs_global = 0);
        ARG_FLAGMASK ('f', cachesim |= CACHESIM_FILE; cachesim &= ~CACHESIM_PIPE;
                      cachesim &= ~CACHESIM_IMMEDIATE);
        ARG_FLAGMASK ('p', cachesim |= CACHESIM_PIPE; cachesim &= ~CACHESIM_FILE;
                      cachesim &= ~CACHESIM_IMMEDIATE);
        ARG_FLAGMASK ('i', cachesim |= CACHESIM_IMMEDIATE; cachesim &= ~CACHESIM_PIPE;
                      cachesim &= ~CACHESIM_FILE);
        ARG_FLAGMASK_END ();
    });

    ARGS_END ();

    if (cachesim & CACHESIM_FILE) {
        *profilinglevel = SAC_CS_file;
    } else if (cachesim & CACHESIM_IMMEDIATE) {
        if (cachesim & CACHESIM_ADVANCED) {
            *profilinglevel = SAC_CS_advanced;
        } else {
            *profilinglevel = SAC_CS_simple;
        }
    } else if (cachesim & CACHESIM_PIPE) {
        if (cachesim & CACHESIM_ADVANCED) {
            *profilinglevel = SAC_CS_piped_advanced;
        } else {
            *profilinglevel = SAC_CS_piped_simple;
        }
    } else {
        /*
         * profilinglevel remains unchanged if no changes are specified on the
         * command line.
         */
    }
} /* SAC_CS_CheckArguments */

static void
InitializeOneCacheLevel (int L, int nr_of_cpu, tProfilingLevel profilinglevel,
                         ULINT cachesize, int cachelinesize, int associativity,
                         tWritePolicy writepolicy)
{
    int i, integretyError;

    tCacheLevel *act_cl /* pointer to the ACTual CacheLevel */;

    /* ATTENTION: calloc initates the allocated memory by 0. That is important
     *            for the cachesimulation to function correctly!!!
     */

    /* bind the right Access_XX functions to Read- WriteAccess userfunction
     * and read- write_access_table */
    if (profilinglevel == SAC_CS_advanced) {
        if (associativity == 1) {
            SAC_CS_read_access_table[L] = &SAC_CS_Access_DMRead_D;
            switch (writepolicy) {
            case SAC_CS_default:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_DMFOW_D;
                break;
            case SAC_CS_fetch_on_write:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_DMFOW_D;
                break;
            case SAC_CS_write_validate:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_DMWV_D;
                break;
            case SAC_CS_write_around:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_DMWA_D;
                break;
            } /* switch */
        } else {
            SAC_CS_read_access_table[L] = &SAC_CS_Access_AS4Read_D;
            switch (writepolicy) {
            case SAC_CS_default:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_AS4FOW_D;
                break;
            case SAC_CS_fetch_on_write:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_AS4FOW_D;
                break;
            case SAC_CS_write_validate:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_AS4WV_D;
                break;
            case SAC_CS_write_around:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_AS4WA_D;
                break;
            } /* switch */
        }     /* if:associativity */
    } else {
        if (associativity == 1) {
            SAC_CS_read_access_table[L] = &SAC_CS_Access_DMRead_S;
            switch (writepolicy) {
            case SAC_CS_default:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_DMFOW_S;
                break;
            case SAC_CS_fetch_on_write:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_DMFOW_S;
                break;
            case SAC_CS_write_validate:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_DMWV_S;
                break;
            case SAC_CS_write_around:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_DMWA_S;
                break;
            } /* switch */
        } else {
            SAC_CS_read_access_table[L] = &SAC_CS_Access_AS4Read_S;
            switch (writepolicy) {
            case SAC_CS_default:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_AS4FOW_S;
                break;
            case SAC_CS_fetch_on_write:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_AS4FOW_S;
                break;
            case SAC_CS_write_validate:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_AS4WV_S;
                break;
            case SAC_CS_write_around:
                SAC_CS_write_access_table[L] = &SAC_CS_Access_AS4WA_S;
                break;
            } /* switch */
        }     /* if:associativity */
    }         /* if:profilinglevel */

    act_cl = (tCacheLevel *)calloc (1, sizeof (tCacheLevel));
    SAC_CS_cachelevel[L] = act_cl;
    if ((cachesize > 0) && (act_cl != NULL)) {
        /* init main-structure */
        act_cl->cachesize = cachesize * 1024; /* kbyte -> byte */
        act_cl->cachelinesize = cachelinesize;
        act_cl->associativity = associativity;
        act_cl->data = (ULINT *)calloc (1, act_cl->cachesize * sizeof (ULINT));

        /* integrety checks && evaluate some vars */
        integretyError = 0;
        if (associativity == 0) {
            integretyError = 1;
        } else {
            integretyError = integretyError || (act_cl->cachesize % associativity != 0);
            act_cl->internsize = act_cl->cachesize / associativity;
        }

        if (cachelinesize == 0) {
            integretyError = 1;
        } else {
            integretyError
              = integretyError
                || (fastlog2 (act_cl->internsize) <= fastlog2 (cachelinesize))
                || (fastlog2 (cachelinesize) == -1);
            act_cl->cls_bits = fastlog2 (act_cl->cachelinesize);
            act_cl->cls_mask = ~(0ul) << act_cl->cls_bits;
            act_cl->is_bits = fastlog2 (act_cl->internsize);
            act_cl->is_mask = ~(0ul) >> ((sizeof (ULINT) * 8) - act_cl->is_bits);
            act_cl->nr_cachelines = act_cl->internsize / act_cl->cachelinesize;
        }

        if (integretyError) {
            SAC_CS_read_access_table[L] = &SAC_CS_Access_MM;
            free (act_cl);
            act_cl = NULL;
            SAC_CS_cachelevel[L] = NULL;
            SAC_RuntimeError ("Invalid cache parameters for L1 cache:\n"
                              "cache size        : %ul KByte\n"
                              "cache line size   : %d Byte\n"
                              "associativity     : %d\n"
                              "write miss policy : %s\n",
                              cachesize, cachelinesize, associativity,
                              WritePolicyName (writepolicy));
        }
    } else {
        SAC_CS_read_access_table[L] = &SAC_CS_Access_MM;
        SAC_CS_write_access_table[L] = &SAC_CS_Access_MM;
        free (act_cl);
        act_cl = NULL;
        SAC_CS_cachelevel[L] = NULL;
    }

    /* init array of shadowarrays */
    for (i = 0; (i < MAX_SHADOWARRAYS) && (act_cl != NULL); i++) {
        act_cl->shadowarrays[i] = NULL;
    }
}

void
SAC_CS_Initialize (int nr_of_cpu, tProfilingLevel profilinglevel, int cs_global,
                   ULINT cachesize1, int cachelinesize1, int associativity1,
                   tWritePolicy writepolicy1, ULINT cachesize2, int cachelinesize2,
                   int associativity2, tWritePolicy writepolicy2, ULINT cachesize3,
                   int cachelinesize3, int associativity3, tWritePolicy writepolicy3)
{
    char filename[1024];

    profiling_level = profilinglevel;
    global_simulation = cs_global;
    /*
     * profiling_level and global_simulation are global variables!
     */

    if (nr_of_cpu > 1) {
        SAC_RuntimeError ("Cache simulation does not support multi-threaded execution.");
    }

    if ((cachesize3 != 0) && ((cachesize1 == 0) || (cachesize2 == 0))) {
        SAC_RuntimeError ("L3 cache specified but L1 or L2 cache missing.");
    }

    if ((cachesize2 != 0) && (cachesize1 == 0)) {
        SAC_RuntimeError ("L2 cache specified but L1 cache missing.");
    }

    if (cachesize1 == 0) {
        SAC_RuntimeError ("No caches specified for cache simulation.");
    }

    if (profilinglevel == SAC_CS_file) {
        SAC_RuntimeError ("Sorry, cache simulation via file not yet implemented.");
    }

    /*
     * The default write policy is resumed.
     * This is done here in order to provide correct information to the user
     * when printing the cache specification below.
     */
    if (writepolicy1 == SAC_CS_default) {
        writepolicy1 = SAC_CS_fetch_on_write;
    }
    if (writepolicy2 == SAC_CS_default) {
        writepolicy2 = SAC_CS_fetch_on_write;
    }
    if (writepolicy3 == SAC_CS_default) {
        writepolicy3 = SAC_CS_fetch_on_write;
    }

    if ((profilinglevel == SAC_CS_piped_simple)
        || (profilinglevel == SAC_CS_piped_advanced)) {

        sprintf (filename,
                 "$SACBASE/runtime/CacheSimAnalyser"
                 " -cs %s%s"
                 " -cs1 %lu/%d/%d/%s"
                 " -cs2 %lu/%d/%d/%s"
                 " -cs3 %lu/%d/%d/%s",
                 ProfilingLevelShortName (profilinglevel), global_simulation ? "g" : "b",
                 cachesize1, cachelinesize1, associativity1,
                 WritePolicyShortName (writepolicy1), cachesize2, cachelinesize2,
                 associativity2, WritePolicyShortName (writepolicy2), cachesize3,
                 cachelinesize3, associativity3, WritePolicyShortName (writepolicy3));

        SAC_CS_pipehandle = popen (filename, "w");

        if (SAC_CS_pipehandle == NULL) {
            SAC_RuntimeError ("Unable to invoke external cache simulation analyser:\n%s",
                              filename);
        }

        /*
         * set all other function-variables to the piped version.
         */
        SAC_CS_Finalize = &Piped_Finalize;
        SAC_CS_RegisterArray = &Piped_RegisterArray;
        SAC_CS_UnregisterArray = &Piped_UnregisterArray;
        SAC_CS_ReadAccess = &Piped_ReadAccess;
        SAC_CS_WriteAccess = &Piped_WriteAccess;
        SAC_CS_Start = &Piped_Start;
        SAC_CS_Stop = &Piped_Stop;
    } else { /* if (piped) */

        InitializeOneCacheLevel (1, nr_of_cpu, profilinglevel, cachesize1, cachelinesize1,
                                 associativity1, writepolicy1);

        InitializeOneCacheLevel (2, nr_of_cpu, profilinglevel, cachesize2, cachelinesize2,
                                 associativity2, writepolicy2);

        InitializeOneCacheLevel (3, nr_of_cpu, profilinglevel, cachesize3, cachelinesize3,
                                 associativity3, writepolicy3);

        /* the 4th level in memory hierachy is the MainMemory
         */
        SAC_CS_read_access_table[4] = &SAC_CS_Access_MM;
        SAC_CS_write_access_table[4] = &SAC_CS_Access_MM;

        /* set the userfunctions ReadAccess and WriteAccess to the right
         * tableentry (1st cachelevel)
         */
        SAC_CS_ReadAccess = SAC_CS_read_access_table[1];
        SAC_CS_WriteAccess = SAC_CS_write_access_table[1];
        /* set all other function-variables to the unpiped/direct version
         */
        SAC_CS_Finalize = &Finalize;
        SAC_CS_RegisterArray = &RegisterArray;
        SAC_CS_UnregisterArray = &UnregisterArray;
        SAC_CS_Start = &Start;
        SAC_CS_Stop = &Stop;

        fprintf (stderr,
                 "%s"
                 "SAC program running with %s cache simulation enabled.\n"
                 "This might delay program execution significantly !!\n"
                 "%s"
                 "L1 cache:  cache size        : %lu KByte\n"
                 "           cache line size   : %d Byte\n"
                 "           associativity     : %d\n"
                 "           write miss policy : %s\n",
                 SAC_CS_separator, ProfilingLevelName (profiling_level), SAC_CS_separator,
                 cachesize1, cachelinesize1, associativity1,
                 WritePolicyName (writepolicy1));

        if (cachesize2 > 0) {
            fprintf (stderr,
                     "%s\n"
                     "L2 cache:  cache size        : %lu KByte\n"
                     "           cache line size   : %d Byte\n"
                     "           associativity     : %d\n"
                     "           write miss policy : %s\n",
                     SAC_CS_separator, cachesize2, cachelinesize2, associativity2,
                     WritePolicyName (writepolicy2));
        }

        if (cachesize3 > 0) {
            fprintf (stderr,
                     "%s"
                     "L3 cache:  cache size        : %lu KByte\n"
                     "           cache line size   : %d Byte\n"
                     "           associativity     : %d\n"
                     "           write miss policy : %s\n",
                     SAC_CS_separator, cachesize3, cachelinesize3, associativity3,
                     WritePolicyName (writepolicy3));
        }

        printf ("%s", SAC_CS_separator);

    } /* if-else (piped) */

} /* SAC_CS_Initialize */

static void
Finalize (void)
{
    unsigned level, j;

    for (level = 1; level <= MAX_CACHELEVEL; level++) {
        if (SAC_CS_cachelevel[level] != NULL) {
            for (j = 0; j < MAX_SHADOWARRAYS; j++) {
                if (SAC_CS_cachelevel[level]->shadowarrays[j] != NULL) {
                    free (SAC_CS_cachelevel[level]->shadowarrays[j]);
                    SAC_CS_cachelevel[level]->shadowarrays[j] = NULL;
                } /* if */
            }     /* for: j */
            free (SAC_CS_cachelevel[level]);
            SAC_CS_cachelevel[level] = NULL;
        } /* if */
    }     /* for: level*/
} /* Finalize */

static void
RegisterArray (void *baseaddress, int size /* in byte */)
{
    int i = 0, level /* index for actual cachelevel */,
        nr_blocks /* the size of a block is the cachelinesize.
                   * nr_blocks keeps the number of blocks, which are affected
                   * by the given array */
      ,
        error_msg_done = 0 /* to avoid more than one errormessage */;
    tCacheLevel *cl; /* pointer to the actual CacheLevel */
    ULINT aligned_baseaddr;

    for (level = 1; level <= MAX_CACHELEVEL; level++) {
        cl = SAC_CS_cachelevel[level];
        if (cl != NULL) {
            /* find free slot in array of shadowarrays */
            while (i < MAX_SHADOWARRAYS && cl->shadowarrays[i] != NULL) {
                i++;
            }
            if (i < MAX_SHADOWARRAYS) {
                /* the following is aquivalent to:
                 * nr_blocks == size/cachelinesize + (size%cachelinesize==0 ? 0 : 1)
                 */
                nr_blocks = (size + (cl->cachelinesize - 1)) / cl->cachelinesize;
                /* nr_blocks is only correct if baseaddress is aligned to the
                 * blocksize, but if not...
                 */

                aligned_baseaddr = (ULINT)baseaddress & cl->cls_mask;
                if (((ULINT)baseaddress % cl->cachelinesize)
                    > (cl->cachelinesize - 1
                       - ((aligned_baseaddr + size - 1) % cl->cachelinesize))) {
                    nr_blocks++;
                }

                cl->shadowbases[i] = (ULINT)baseaddress;
                cl->shadowalignedtop[i] = ((ULINT)baseaddress + size - 1) & cl->cls_mask;
                cl->shadowmaxindices[i] = ((nr_blocks + 1) / 2) - 1;
                cl->shadownrcols[i]
                  = (nr_blocks + (cl->nr_cachelines - 1)) / cl->nr_cachelines;
                cl->shadowarrays[i]
                  = (char *)calloc (((cl->nr_cachelines * cl->shadownrcols[i]) + 1) / 2,
                                    sizeof (char));
                /* already initiated by 0 */

            } else {
                if (!error_msg_done) {
                    error_msg_done = 1;
                    SAC_RuntimeError ("libsac_cachesim: more than %d registered arrays.",
                                      MAX_SHADOWARRAYS);
                } /* if: !error_msg_done */
            }     /* if-else: i<MAX_SHADOWARRAYS */
        }         /* if: cl != NULL */
    }             /* for: a */
} /* RegisterArray */

static void
UnregisterArray (void *baseaddress)
{
    int i, j, lastused;
    tCacheLevel *cl;

    for (i = 1; i <= MAX_CACHELEVEL; i++) {
        cl = SAC_CS_cachelevel[i];
        if (cl != NULL) {
            /* find index (j) to remove */
            j = 0;
            while ((j < MAX_SHADOWARRAYS) && (cl->shadowbases[j] != (ULINT)baseaddress)) {
                j++;
            }
            if (j < MAX_SHADOWARRAYS) {
                /* find last used index */
                lastused = j;
                while ((lastused < MAX_SHADOWARRAYS)
                       && (cl->shadowarrays[lastused] != NULL)) {
                    lastused++;
                }           /* while: lastused */
                lastused--; /* correction in both cases */
                if (j != lastused) {
                    /* overwrite j with lastused and remove lastused then
                     * (to keep an contigous range of used indices) */
                    free (cl->shadowarrays[j]);
                    cl->shadowarrays[j] = cl->shadowarrays[lastused];
                    cl->shadowarrays[lastused] = NULL;
                    /* so the following free doesn´t kill the new j  */
                    cl->shadowbases[j] = cl->shadowbases[lastused];
                    cl->shadowalignedtop[j] = cl->shadowalignedtop[lastused];
                    cl->shadowmaxindices[j] = cl->shadowmaxindices[lastused];
                    cl->shadownrcols[j] = cl->shadownrcols[lastused];
                }
                /* remove lastused */
                free (cl->shadowarrays[lastused]);
                /* lastused is NULL if j was overwritten! */
                cl->shadowarrays[lastused] = NULL;
                cl->shadowbases[lastused] = 0;
                /* other members of this struct may stay as they are */
            } /* if */
        }     /* if */
    }         /*for: i*/
} /* UnregisterArray */

void
SAC_CS_Access_MM (void *baseaddress, void *elemaddress)
{
    SAC_CS_level = 1;
} /* SAC_CS_Access_MM */

void
SAC_CS_ShowResults (void)
{
    int i, digits;
    long unsigned int accesses, both;
    float hit_ratio;

    fprintf (stderr,
             "\n"
             "%s"
             "SAC cache simulation results:\n",
             SAC_CS_separator);

    if (starttag[0] != '#') {
        fprintf (stderr, "Block: %s\n", starttag);
    }

    fprintf (stderr, "%s", SAC_CS_separator);

    digits = (int)ceil (log10 ((double)SAC_CS_hit[1] + SAC_CS_miss[1]));

    for (i = 1; i <= MAX_CACHELEVEL; i++) {
        if (SAC_CS_cachelevel[i] != NULL) {
            accesses = SAC_CS_hit[i] + SAC_CS_miss[i];
            hit_ratio = ((float)SAC_CS_hit[i] / (float)accesses) * 100.0;

            fprintf (stderr,
                     "L%d cache:  accesses:  %*lu\n"
                     "           hits:      %*lu  (%5.1f%%)\n"
                     "           misses:    %*lu  (%5.1f%%)\n",
                     i, digits, accesses, digits, SAC_CS_hit[i], hit_ratio, digits,
                     SAC_CS_miss[i], 100.0 - hit_ratio);

            if ((profiling_level == SAC_CS_advanced)) {
                if (SAC_CS_miss[i] == 0) {
                    /*
                     * This is to avoid printing of NaN in percentages.
                     */
                    SAC_CS_miss[i] = 1;
                }

                /*
                 * A self- and cross interference (for one access) will be counted
                 * in SAC_CS_self and also in SAC_CS_cross! The following computations
                 * correct this!
                 */
                both = SAC_CS_self[i] + SAC_CS_cross[i] + SAC_CS_cold[i] - SAC_CS_miss[i];
                SAC_CS_self[i] -= both;
                SAC_CS_cross[i] -= both;

                fprintf (stderr,
                         "  misses:  cold start:          %*lu  (%5.1f%%)\n"
                         "           cross interference:  %*lu  (%5.1f%%)\n"
                         "           self interference:   %*lu  (%5.1f%%)\n"
                         "           self & cross int.:   %*lu  (%5.1f%%)\n"
                         "           invalidation:        %*lu  (%5.1f%%)\n",
                         digits, SAC_CS_cold[i],
                         ((float)SAC_CS_cold[i] / (float)SAC_CS_miss[i]) * 100.0, digits,
                         SAC_CS_cross[i],
                         ((float)SAC_CS_cross[i] / (float)SAC_CS_miss[i]) * 100.0, digits,
                         SAC_CS_self[i],
                         ((float)SAC_CS_self[i] / (float)SAC_CS_miss[i]) * 100.0, digits,
                         both, ((float)both / (float)SAC_CS_miss[i]) * 100.0, digits,
                         SAC_CS_invalid[i],
                         ((float)SAC_CS_invalid[i] / (float)SAC_CS_miss[i]) * 100.0);
            } /* if */
            fprintf (stderr, "%s", SAC_CS_separator);
        } /* if */
    }     /* for: i */
} /* SAC_CS_ShowResults */

static void
Start (char *tag)
{
    int i;

    if (sim_incarnation == 0) {
        if (((tag[0] == '#') && global_simulation) || (tag[0] != '#')) {
            strncpy (starttag, tag, MAX_TAG_LENGTH - 1);
            sim_incarnation++;
            /* set all counters to 0 */
            for (i = 1; i <= MAX_CACHELEVEL; i++) {
                SAC_CS_hit[i] = 0;
                SAC_CS_miss[i] = 0;
                SAC_CS_cold[i] = 0;
                SAC_CS_self[i] = 0;
                SAC_CS_cross[i] = 0;
                SAC_CS_invalid[i] = 0;
            } /* for */
        }
    } else {
        sim_incarnation++;
        if (starttag[0] != '#') {
            fprintf (stderr,
                     "Cachesim warning:\n"
                     "Simulation \"%s\" ignored:\n"
                     "Simulation \"%s\" still running !\n",
                     tag, starttag);
        }
    }

} /* Start */

static void
Stop (void)
{
    sim_incarnation--;

    if (sim_incarnation == 0) {
        SAC_CS_ShowResults ();
        starttag[0] = '\0';
    }
} /* Stop  */

/* The following functions are the piped versions of the static functions
 * above
 */

static void
Piped_Finalize (void)
{
    fprintf (SAC_CS_pipehandle, "F\n");
    fflush (SAC_CS_pipehandle);
    pclose (SAC_CS_pipehandle);
} /* Piped_Finalize */

static void
Piped_RegisterArray (void *baseaddress, int size)
{
    fprintf (SAC_CS_pipehandle, "G %.8lx %u\n", (ULINT)baseaddress, size);
} /* Piped_RegisterArray */

static void
Piped_UnregisterArray (void *baseaddress)
{
    fprintf (SAC_CS_pipehandle, "U %.8lx\n", (ULINT)baseaddress);
} /* Piped_RegisterArray */

static void
Piped_ReadAccess (void *baseaddress, void *elemaddress)
{
    fprintf (SAC_CS_pipehandle, "R %.8lx %.8lx\n", (ULINT)baseaddress,
             (ULINT)elemaddress);
} /* Piped_ReadAccessFun */

static void
Piped_WriteAccess (void *baseaddress, void *elemaddress)
{
    fprintf (SAC_CS_pipehandle, "W %.8lx %.8lx\n", (ULINT)baseaddress,
             (ULINT)elemaddress);
} /* Piped_WriteAccessFun */

static void
Piped_Start (char *tag)
{
    fprintf (SAC_CS_pipehandle, "B %s\n", tag);
} /* Piped_Start */

static void
Piped_Stop (void)
{
    fprintf (SAC_CS_pipehandle, "E\n");
} /* Piped_Stop */
