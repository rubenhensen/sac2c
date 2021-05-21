#include "math_utils.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

/******************************************************************************
 *
 * function:
 *   int MATHlcm( int x, int y)
 *
 * description:
 *   returns the lowest-common-multiple of x, y.
 *
 ******************************************************************************/

int
MATHlcm (int x, int y)
{
    int u, v;

    DBUG_ENTER ();

    DBUG_ASSERT (((x > 0) && (y > 0)), "Arguments of MATHlcm() must be >0");

    u = x;
    v = y;
    while (u != v) {
        if (u < v) {
            u += x;
        } else {
            v += y;
        }
    }

    DBUG_RETURN (u);
}

/******************************************************************************
 *
 * function:
 *   int MATHipow( int x, int y)
 *
 * description:
 *   returns x to the power of y
 *
 ******************************************************************************/

int
MATHipow (int base, int exp)
{
    int i;
    int res;

    DBUG_ENTER ();

    res = 1;

    for (i = 0; i < exp; i++) {
        res *= base;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int MATHnumDigits( int number)
 *
 * description:
 *   returns the number of digits of the given number
 *
 ******************************************************************************/

unsigned int
MATHnumDigits (int number)
{
    unsigned int digits = 1;

    DBUG_ENTER ();

    while (number / 10 >= 1) {
        number = number / 10;
        digits += 1;
    }

    DBUG_RETURN (digits);
}

/******************************************************************************
 *
 * function:
 *   int MATHmin( int x, int y)
 *
 * description:
 *   returns the minimum of two numbers
 *
 ******************************************************************************/

int
MATHmin (int x, int y)
{
    int min;

    DBUG_ENTER ();

    if (x < y) {
        min = x;
    } else {
        min = y;
    }

    DBUG_RETURN (min);
}

/******************************************************************************
 *
 * function:
 *   int MATHmax( int x, int y)
 *
 * description:
 *   returns the maximum of two numbers
 *
 ******************************************************************************/

int
MATHmax (int x, int y)
{
    int max;

    DBUG_ENTER ();

    if (x > y) {
        max = x;
    } else {
        max = y;
    }

    DBUG_RETURN (max);
}

#undef DBUG_PREFIX
