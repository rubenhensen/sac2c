/*******************************************************************************
 *  TARGETS  for Alternate Compilers:
 */

/*******************************************************************************
 *
 * Target suncc: use the Sun C compiler instead of gcc
 *
 */

target cc_sun:

CC             :=  "cc"
CFLAGS         :=  "-KPIC -dalign -xsafe=mem"
LDFLAGS        :=  "-lpthread -mt"

OPT_O0         :=  ""
OPT_O1         :=  "-xO2"
OPT_O2         :=  "-xO4"
OPT_O3         :=  "-fast"


/*******************************************************************************
 *
 * Target intelcc: use the Intel C compiler instead of gcc
 *
 */

target cc_icc:

CC             := "icc"
CFLAGS         := " @GENPIC@ -shared-intel -xsse4.2"

target cc_icc_sse2 :: cc_icc:

CFLAGS         += " -msse2"

target cc_icc_fast :: cc_icc:

CFLAGS         += " -fast"


/*******************************************************************************
 *
 * Target clang: use the LLVM CLang compiler instead of gcc
 *
 */

target cc_clang:
CC             := "clang"
OPT_O2         :=  "-O4"
OPT_O3         :=  "-O3"


/*******************************************************************************
 *
 * Target opencc: use the Open64 C compiler instead of gcc
 *
 */

target cc_opencc:
CC             := "opencc"
OPT_O3         := "-O3"


/*******************************************************************************
 *
 * Target milepost: use the Milepost Gcc C compiler instead of gcc
 *
 */

target cc_milepost:
CC             := "milepost-gcc"
OPT_O3         := "-O3"


