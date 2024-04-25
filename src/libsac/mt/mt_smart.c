/*****************************************************************************
 *
 * file:   mt_smart.c
 *
 * prefix: SAC_MT
 *
 * description:
 *   This file contains the runtime components of the smart decision tool
 *   library. The compile time components of this library are located in
 *   compile.c. The smart decision tool tries to find the "optimal" amount of
 *   threads to solve a parallel task. The tool is used to optimize both
 *   the performance and energy consumption of parallel with loops. The smart
 *   decisions consist of two phases: the training phase and the decision
 *   phase. Each phase requires a recompilation of the sac file, using a
 *   special compilation mode. To compile for the training phase one has to
 *   set the compiler option -mt_smart_mode to 'train'. To compile for the
 *   decision phase the -mt_smart_mode option should be set to 'on'. The
 *   default for the -mt_smart_mode option is 'off', meaning that the smart
 *   decision tool is not being used. The training phase binary has to be
 *   executed before the decision binary is executed. During the training phase
 *   data is collected to create a performance profile of the current machine.
 *   The training phase can be done multiple times in order to collect more
 *   data to construct a more accurate performance profile. The profile is
 *   stored in a separate database (.db) file. After training the decision
 *   binary can be executed, during this phase smart decisions are used to
 *   optimize for the number of threads that are used for each parallel with
 *   loop. The decision phase uses the database file in order to predict the
 *   most optimal settings. If the program has to be executed on several
 *   machines, one has to make sure that a profile is created for each of
 *   these machines.
 *
 * important files:
 *   - mt_smart.h: contains the function primitives of this file
 *   - compile.c: contains the compile time components of this library
 *     (initialization of compile time components of smart tool: COMPdoPrepareSmart
 *function executing compile time components of smart tool: COMPdoDecideSmart function
 *		finalization of compile time components of smart tool: COMPdoFinalizeSmart
 *function there are also a few helper functions in compile.c, such as: rank,
 *create_smart_decision_data, and destroy_smart_decision_data)
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include "mt_smart.h"

// A struct that contains some parameters
// that needs to be shared between SAC_MT_smart_init
// and the many invocations of the SAC_MT_smart_train or
// SAC_MT_smart_decide functions.
typedef struct SAC_MT_smart_share {
    FILE *fp;
    int64_t *stat;
    int idx;
    unsigned tot_nr_threads;
    unsigned line_count;
    unsigned line_size;
    bool new_file;
    bool line_read;
    bool new_line;
    bool first_measurement;
} SAC_MT_smart_share_t;

static SAC_MT_smart_share_t share;

// Some global parameters that are used to configure
// SaC on runtime. For example 'current_nr_threads' is
// used to tweak the number of threads that are used
// to compute the next parallel problem.
unsigned current_nr_threads;
unsigned smart_sample_size;
unsigned problem_size;
struct timespec begin, end;

/******************************************************************************
 *
 * function:
 *    int ndigits(int nr)
 *
 * description:
 *    Helper function that computes the number of digits in a decimal number.
 *
 * arguments:
 *    - int nr: integer number that you want to analyze
 *
 ******************************************************************************/
int
ndigits (int nr)
{
    /* INT based log10 may be better in the future */
    return  (int)(floor (log10 (fabs (nr + 0.0))) + 1.0); 
}

/******************************************************************************
 *
 * function:
 *    void SAC_MT_smart_init(int type, char * file_id, char * arch_id, unsigned
 *nr_threads)
 *
 * description:
 *    This function is used to initialize the runtime components of the smart
 *	  decision tool. The initialization consists of creating or loading the
 *	  database file and the setup of several environmental settings.
 *
 * arguments:
 *    - int type: integer number that defines the phase that the SaC file is
 *		compiled for. If type=1 the SaC file is compiled for the training phase,
 *		if type=2 the SaC file is compiled for the decision phase. Any other
 *		number can be used to indicate that the smart decision tool is not
 *		being used, but in these cases the SAC_MT_smart_init function should
 *		not be called.
 *	  - char * file_id: a filename for the database file
 *	  - char * arch_id: the name of the current architecture that is being used
 *	  - unsigned nr_threads: the maximum number of threads that can be used on
 *		the current architecture
 *
 ******************************************************************************/
void
SAC_MT_smart_init (int type, char *file_id, char *arch_id, unsigned nr_threads)
{
    char *filename;

    // if SaC file is compiled for the training phase do:
    if (type == 1) {
        // read or create the database file
        filename
          = malloc ((11 + strlen (file_id) + strlen (arch_id) + ndigits (nr_threads))
                    * sizeof (char));
        if (filename == NULL) {
            fprintf(stderr, "Allocation failed\n");
            abort();
        }
        sprintf (filename, "stat.%s.%s.%i.db", file_id, arch_id, nr_threads);

        share.new_file = (access (filename, F_OK) < 0);
        if (share.new_file == true) {
            share.fp = fopen (filename, "w+");
        } else {
            share.fp = fopen (filename, "r+");
        }

        // set some parameters that are needed to create
        // a performance profile
        share.line_count = nr_threads + 3;
        share.line_size = share.line_count * sizeof (int64_t);
        share.line_read = false;
        share.new_line = false;
        share.first_measurement = true;
        share.stat = malloc ((share.line_count) * sizeof (int64_t));
        free (filename);
    }

    // set some additional parameters that are used to
    // tweak the behaviour of how pieces of parallel code
    // are executed
    share.tot_nr_threads = nr_threads;
    current_nr_threads = 0;
    smart_sample_size = 1;
}

/******************************************************************************
 *
 * function:
 *    int SAC_MT_smart_train(int spmd_id, int64_t measurement_period)
 *
 * description:
 *	  This function is only used during the training phase. The function creates
 *	  and stores a performance profile, by measuring the execution time and
 *	  problem size of every with loop.
 *
 * arguments:
 *    - int spmd_id: unique identifier for the current with loop.
 *	  - int64_t measurement_period: this number is used to set the maximum
 *		number of msec that can be used to measure the execution time of a
 *		single with loop.
 *
 *		(Be aware that a single with loop can be called multiple times so that
 *		the total measurment time can take much more time than the
 *		measurement_period. Also note that multiple time measurements are
 *		performed on a single with loop during the training phase in order to
 *		collect performance data for different numbers of threads.)
 *
 ******************************************************************************/
int
SAC_MT_smart_train (int spmd_id, int64_t measurement_period)
{
    int64_t tot_time;
    int64_t bsec, bnsec, esec, ensec;
    int count;
    int idx;

    // find or create the line in the database file that is used to store
    // the measurements for the current with loop
    if (share.line_read == false && share.new_file == false) {
        rewind (share.fp);
        idx = 0;
        do {
            count = fread (share.stat, sizeof (int64_t), share.line_count, share.fp);
            idx++;
        } while (count == (signed)share.line_count
                 && (share.stat[0] != spmd_id || share.stat[1] != (signed)problem_size));
        share.idx = (idx - 1) * share.line_size;
        if (count != (signed)share.line_count) {
            share.new_line = true;
        } else {
            share.line_read = true;
        }
    }
    if ((share.line_read == false && share.new_file == true)
        || (share.line_read == false && share.new_line == true)) {
        for (unsigned i = 2; i < share.line_count; i++) {
            share.stat[i] = 0;
        }
        share.new_line = true;
        share.line_read = true;
        share.new_file = false;
    }

    idx = current_nr_threads + 2;
    if (current_nr_threads > 0) {
        // compute the total execution time after a measurement took place
        bsec = begin.tv_sec;
        bnsec = begin.tv_nsec;
        esec = end.tv_sec;
        ensec = end.tv_nsec;
        tot_time = (esec * 1000000000 + ensec) - (bsec * 1000000000 + bnsec);

        // the code below is used to tune the time that is used to measure a single with
        // loop, so that the measurement does not take more time than is allowed by the
        // measurement_period parameter. The code is only used once for every with loop
        // invocation.
        if (share.first_measurement) {
            share.first_measurement = false;
            smart_sample_size = (measurement_period * 1000000) / tot_time;
            if (smart_sample_size > 1) {
                return 1;
            }
            smart_sample_size = 1;
        }

        // temporarly store the execution time
        share.stat[idx] += tot_time;
    }

    if (current_nr_threads < share.tot_nr_threads) {
        // increase the number of threads and start the next measurement
        current_nr_threads++;
        return 1;
    } else {
        // write the current set of measurements to the line in the database
        current_nr_threads = 0;
        share.line_read = false;
        share.first_measurement = true;
        share.stat[0] = spmd_id;
        share.stat[1] = (int64_t)problem_size;
        share.stat[2] += (int64_t)smart_sample_size;
        smart_sample_size = 1;
        if (share.new_line == false) {
            fseek (share.fp, share.idx, SEEK_SET);
        } else {
            fseek (share.fp, 0, SEEK_END);
            share.new_line = false;
        }
        fwrite (share.stat, sizeof (int64_t), share.line_count, share.fp);
        return 0;
    }
}

/******************************************************************************
 *
 * function:
 *    void SAC_MT_smart_decide(int size, const int (* recommendations)[2])
 *
 * description:
 *	  This function is used during the decision phase and will estimate the
 *	  "optimal" number of threads based on a recommendation table. The
 *	  recommendation table is precomputed by the compiler based on the
 *	  performance profile stored in the database file.
 *
 * arguments:
 *    - int size: the length of the recommendation table
 *	  - const int (* recommendations)[2]: the recommendation table. The
 *		recommendation table consists of multiple rows with two values each.
 *		The first value is a number that represents a possible problem size,
 *		the second value is a recommendation. The recommendation table is
 *		scanned to find a problem size that matches the problem size of the
 *		current with loop. The recommendation is used to configure the number
 *		of threads that are being used for the computation of the with loop.
 *
 ******************************************************************************/
void
SAC_MT_smart_decide (int size, const int (*recommendations)[2])
{
    float perc = 0.0;
    float dX, dY;
    unsigned nr_threads;

    // default number of threads set to 0
    current_nr_threads = 0;

    // scan recommendation table
    for (int i = 0; i < size; i++) {
        // if the current problem size is in between two recommendations
        // use linear interpolation to find the number of threads that
        // is best for the current problem size
        if (recommendations[i][0] > (signed)problem_size && i > 0) {
            dY = (float)(problem_size - recommendations[i - 1][0]);
            dX = (float)(recommendations[i][0] - recommendations[i - 1][0]);
            perc = dY / dX;
            nr_threads = (unsigned)llroundf (
              perc * (recommendations[i][1] - recommendations[i - 1][1])
              + recommendations[i - 1][1]);
            if (share.tot_nr_threads < nr_threads) {
                current_nr_threads = share.tot_nr_threads;
            } else {
                current_nr_threads = nr_threads;
            }
            break;
        }
        // if the current problem size matches a problem size from the table
        // use the recommendation.
        if (recommendations[i][0] == (signed)problem_size || i == size - 1
            || (i == 0 && recommendations[i][0] > (signed)problem_size)) {
            if (share.tot_nr_threads < (unsigned)(recommendations[i][1])) {
                current_nr_threads = share.tot_nr_threads;
            } else {
                current_nr_threads = (unsigned)(recommendations[i][1]);
            }
            break;
        }
        // no extrapolation is applied. If the current problem size is smaller
        // than any problem size in the recommendation table, the recommendation
        // for the smallest problem size in the table is used.
        // If the current problem size is larger than any problem size in the
        // table, the recommendation for the largest problem size in the table
        // is used.
    }

    // use all threads if the number of threads equals 0
    if (current_nr_threads == 0) {
        current_nr_threads = share.tot_nr_threads;
    }
}

/******************************************************************************
 *
 * function:
 *    void SAC_MT_smart_finalize()
 *
 * description:
 *	  Free memory that what used to store data that was needed during the
 *	  training and decision phase
 *
 ******************************************************************************/
void
SAC_MT_smart_finalize (void)
{
    free (share.stat);
    fclose (share.fp);
}
