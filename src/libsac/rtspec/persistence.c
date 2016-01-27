/** <!--********************************************************************-->
 * @file  persistence.c
 *
 * @brief  This file contains the persistence handling for runtime
 * specializations.
 *
 *****************************************************************************/

#include "config.h"

#if SAC_DO_RTSPEC

#include "trace.h"

#define SAC_DO_TRACE 1
#include "sac.h"
#include "stdio.h"

#define MAX_INT_DIGITS 21

static int do_trace;

/** <!--*******************************************************************-->
 *
 * @fn encodeShapes( int *shapes)
 *
 * @brief  Creates a string representation of the shape information stored in
 * the integer array 'shapes'.
 *
 * The format of both the shape array and the string representation is as
 * follows:
 *
 * {
 * number_of_arguments
 *  dimension_arg_1
 *    extent_dim_1
 *    ...
 *    extent_dim_n
 *  dimension_arg_2
 *   ...
 *  dimension_arg_n
 * }
 *
 * @param shapes  The array of shape information.
 ****************************************************************************/
char *
encodeShapes (int *shapes)
{
    int num_args, i, j, k, l, length;

    if (shapes == NULL) {
        fprintf (stderr, "ERROR -- \t Missing shape information!");
        return '\0';
    }

    num_args = shapes[0];

    /*
     * Encode the base type information.
     */
    size_t shape_string_size = MAX_INT_DIGITS + 1;

    i = 1;
    k = 1;
    for (; i <= num_args; i++) {
        if (shapes[k] > 0) {
            j = 0;
            l = shapes[k];
            for (; j <= l; j++) {
                shape_string_size += MAX_INT_DIGITS;
                k++;
            }
        } else {
            shape_string_size += MAX_INT_DIGITS;
            k++;
        }
    }

    char *current = (char *)malloc (shape_string_size * sizeof (char));

    current[0] = '\0';

    length = 0;

    length += sprintf (current, "%d-", num_args);

    i = 1;
    k = 1;
    for (; i <= num_args; i++) {
        if (shapes[k] > 0) {
            j = 0;
            l = shapes[k];
            for (; j <= l; j++) {
                length += sprintf (current + length, "%d-", shapes[k]);
                k++;
            }
        } else {
            length += sprintf (current + length, "%d-", shapes[k]);
            k++;
        }
    }

    return current;
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_persistence_init(int trace)
 *
 * @brief Setup the persistence directory.
 *
 ****************************************************************************/
void
SAC_persistence_init (int trace)
{
    do_trace = trace;
}

#endif /* SAC_DO_RTSPEC */
