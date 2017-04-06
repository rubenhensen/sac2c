#ifndef _SAC_PHDEFS_H_
#define _SAC_PHDEFS_H_

// LUR uses this for "loop count unknown", so we do, too.
#define UNR_NONE (-1)

/** <!--********************************************************************-->
 *
 * Prefix: PHDEFS
 *
 * This is used by POGO, PHUT, and by the Polyhedral code in
 * tools/cuda/sacislinterface.c
 *
 *****************************************************************************/

// ISL matrix column definitions
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

// ISL interface function op codes
#define SACISLINTERFACEBINARY "sacislinterface"
#define POLY_OPCODE_PWLF 'F'
#define POLY_OPCODE_INTERSECT 'I'
#define POLY_OPCODE_MATCH 'M'
#define POLY_OPCODE_PRINT 'P'
#define POLY_OPCODE_LOOPCOUNT 'C'
#define POLY_OPCODE_HELP 'H'
#define POLY_OPCODE_HELPLC 'h'

// ISL interface function return codes
// These are ORed together, so more than one result
// can appear at one time.

#define POLY_RET_INVALID 2
#define POLY_RET_UNKNOWN 4
#define POLY_RET_EMPTYSET_BCF 8
#define POLY_RET_EMPTYSET_BCG 16
#define POLY_RET_MATCH_BC 32
#define POLY_RET_EMPTYSET_BC 64
#define POLY_RET_CCONTAINSB 128
#define POLY_RET_SLICENEEDED 256
#define POLY_RET_EMPTYSET_B 512

// ISL entry classes
// This loop:
//     for ( IV=0, IV<N; IV+=4} {
//       S;
//     }
//
// can be represented in ISL as:
//
//   [N] -> { [IV] : exists P : 0 <= IV < N and IV = 4 P }
//                          ^--- existentially qualified variable
//             ^---------------- ISL set-variable
//    ^------------------------- ISL parameter
#define AVIS_ISLCLASSUNDEFINED 0
#define AVIS_ISLCLASSPARAMETER 1
#define AVIS_ISLCLASSSETVARIABLE 2
#define AVIS_ISLCLASSEXISTENTIAL 3

#endif /* _SAC_PHDEFS_H_ */
