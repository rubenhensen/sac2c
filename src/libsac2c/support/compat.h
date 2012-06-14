/* NOTE: We intetnionally do not define preprocessing guards for this file.  */

/* When including a code that uses memory routines from the C
   standard library, SAC system gets really upset.

   In order to avoid renaming, here is a collection of macros
   which one should include as a *last* include-file in a source.
   This would preprocess-out standard routuines replacing them
   with the names that make SAC memory allocation tracker shut-up.  */

#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#undef malloc
#undef realloc
#undef free
#undef strdup
#undef exit

#define malloc(x) MEMmalloc (x)
#define realloc(x, y) __MEMrealloc (x, y)
#define free(x) MEMfree (x)
#define strdup(x) STRcpy (x)
#define exit(x) CTIexit (x)

/* FIXME: Anything else?  */
/* All the rest should be pretty much harmless.  */
