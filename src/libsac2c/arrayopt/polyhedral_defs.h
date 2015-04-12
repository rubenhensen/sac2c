#ifndef _SAC_PHDEFS_H_
#define _SAC_PHDEFS_H_

/** <!--********************************************************************-->
 *
 * Prefix: PHDEFS
 *
 * This is used by POGO, PHUT, and by the Polyhedral code in tools/cuda/sacislinterface.c
 *
 *****************************************************************************/

// PolyLib input matrix format is found in Chapter 9 (Example)
// of the Polyulib User's Manual, page 35.
// ISL supports the same unreadable format, so we just use that.

// PolyLib/ISL matrix column definitions
#define PLFUN 0
#define PLFUNEQUALITY 0
#define PLFUNINEQUALITY 1
// PLWID is a fake variable that we use to support GENERATOR_WIDTH as an inner loop on W
// E.g., 0 <= W < WIDTH
#define PLWID 1
// PLAPV is a fake variable that we use to represent STEP*N, as part of
// an Arithmetic Progession Vector
#define PLAPV 2
// PLFAKEN is a fake variable used to represent the unknown N in PLAPV.
#define PLFAKEN 3
// PLVARS are the affine variables used in the expression.
// They start at column PLVARS and go upwards, except the last column, which
// holds constant values.
#define PLVARS 4

// PolyLib interface function op codes
#define SACISLINTERFACEBINARY "sacislinterface"
#define POLY_OPCODE_PWLF 'F'
#define POLY_OPCODE_INTERSECT 'I'
#define POLY_OPCODE_MATCH 'M'
#define POLY_OPCODE_PRINT 'P'

// PolyLib interface function return codes
// These are ORed together, so more than one result
// can appear at one time.

#define POLY_RET_INVALID 2
#define POLY_RET_UNKNOWN 4
#define POLY_RET_EMPTYSET_BCR 8
#define POLY_RET_EMPTYSET_BCC 16
#define POLY_RET_MATCH_BC 32
#define POLY_RET_EMPTYSET_BC 64
#define POLY_RET_CCONTAINSB 128

#endif /* _SAC_PHDEFS_H_ */
