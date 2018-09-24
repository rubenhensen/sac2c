#ifndef _MATH_UTILS_H_
#define _MATH_UTILS_H_

extern int MATHlcm (int x, int y);
extern long MATHipow (int base, int exp);
extern int MATHnumDigits (int number);

extern int MATHmin (int x, int y);
extern int MATHmax (int x, int y);

#define MATH_MAX_ANY(x, y) ( x > y ? x : y)

#endif /*_MATH_UTILS_H_ */
