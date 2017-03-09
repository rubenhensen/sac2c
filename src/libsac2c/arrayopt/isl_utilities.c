/** <!--********************************************************************-->
 *
 * @defgroup ISLU ISL utility functions
 *
 *  Overview: These functions are an interface to the Integer Set Library,
 *            and Barvinok, both of which may or may not exist on this system.
 *            If both are not present, these functions all complete with error.
 *
 *    ISL input data Terminology, taken from
 *      http://barvinok.gforge.inria.fr/tutorial.pdf,
 *      Sven Verdoolaege's ISCC tutorial.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file isl_utilities.c
 *
 * Prefix: ISLU
 *
 *****************************************************************************/

#include "globals.h"

#define DBUG_PREFIX "ISLU"
#include <stdlib.h>
#include <sys/wait.h>
#include "debug.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "constants.h"
#include "shape.h"
#include "type_utils.h"
#include "new_types.h"
#include "constants.h"
#include "new_typecheck.h"
#include "print.h"
#include "system.h"
#include "filemgr.h"
#include "sys/param.h"
#include "isl_utilities.h"
#include "polyhedral_defs.h"
#include "config.h"
#include "polyhedral_utilities.h"

#if ENABLE_ISL && ENABLE_BARVINOK
#include <isl/union_set.h>
#include <isl/ctx.h>
#include <isl/vec.h>
#include <isl/obj.h>
#include <isl/stream.h>
#include <isl/set.h>
#include <isl/map.h>
#include <isl/vertices.h>
#include <isl/flow.h>
#include <isl/band.h>
#include <isl/ast_build.h>
#include <isl/schedule.h>
#include <barvinok/isl.h>
#endif // ENABLE_ISL && ENABLE_BARVINOK

/** <!-- ****************************************************************** -->
 *
 * @fn void ISLUprintUnionMap(...)
 *     void ISLUprintUnionSet(...)
 *
 * @brief: print a union set or map to stdout
 *
 * @param: fd - a file descriptor, usually stderr
 * @param: pmap - a union map
 * @param: pset - a union set
 * @param: titl - human-readable text as title for set/map.
 *
 * @result: none
 *
 ******************************************************************************/
#if ENABLE_ISL && ENABLE_BARVINOK
void
ISLUprintUnionMap (FILE *fd, struct isl_union_map *pmap, char *titl)
{
    // Print a union_map to fd (e.g., stderr)
    isl_printer *p;
    struct isl_ctx *ctx;

    DBUG_ENTER ();

    ctx = isl_union_map_get_ctx (pmap);
    fprintf (fd, "UnionMap %s is:\n", titl);
    p = isl_printer_to_file (ctx, fd);
    p = isl_printer_print_union_map (p, pmap);
    p = isl_printer_end_line (p);
    isl_printer_free (p);

    DBUG_RETURN ();
}

void
ISLUprintUnionSet (FILE *fd, struct isl_union_set *pset, char *titl)
{
    // Print union_set pset to fd, with title titl
    isl_printer *p;
    struct isl_ctx *ctx;

    DBUG_ENTER ();

    ctx = isl_union_set_get_ctx (pset);
    fprintf (fd, "UnionSet %s is:\n", titl);
    p = isl_printer_to_file (ctx, fd);
    p = isl_printer_print_union_set (p, pset);
    p = isl_printer_end_line (p);
    isl_printer_free (p);

    DBUG_RETURN ();
}
#endif // ENABLE_ISL && ENABLE_BARVINOK

/** <!-- ****************************************************************** -->
 *
 * @fn int ISLUexprs2String()
 *
 * @brief: Convert an ISLish N_exprs chain into a character string
 *         suitable for input to ISL.
 *
 * @param: exprs: A PHUTish N_exprs chain comprising an ISL union set,
 *         or equivalent.
 *
 * @param: varlut: Address of the LUT containing the union set variable names.
 *
 * @param: lbl: The address of a character string used to label the string
 *              for debugging purposes only.
 * @param: isunionset: TRUE if we are to write a union set;
 *                    FALSE if we are to write a union map.
 * @param: lhsname: the LHS of the expression we are trying to simplify
 *
 * @result: a pointer to the generated character string. The caller of
 *          this function assumes responsibility for freeing the string.
 *
 ******************************************************************************/
char *
ISLUexprs2String (node *exprs, lut_t *varlut, char *lbl, bool isunionset, char *lhsname)
{
    FILE *matrix_file;
    long fsize;
    size_t sz;
    char polyhedral_arg_filename[PATH_MAX];
    static const char *argfile = "polyhedral_args";
    char *str = NULL;

    DBUG_ENTER ();

    if (!global.cleanup) { // If -d nocleanup, keep all ISL files
        global.polylib_filenumber++;
    }

    sprintf (polyhedral_arg_filename, "%s/%s%d.arg", global.tmp_dirname, argfile,
             global.polylib_filenumber);

    DBUG_PRINT ("ISL arg filename: %s", polyhedral_arg_filename);
    matrix_file = FMGRreadWriteOpen (polyhedral_arg_filename, "w+");
    PHUTwriteUnionSet (matrix_file, exprs, varlut, lbl, isunionset, lhsname);
    fflush (matrix_file); // This is NOT optional, apparently.

    // Get entire file into a string buffer
    fsize = ftell (matrix_file);
    rewind (matrix_file);
    str = (char *)MEMmalloc ((1 + fsize) * sizeof (char));
    sz = fread (str, sizeof (char), (size_t)fsize, matrix_file);
    DBUG_ASSERT (sz == (size_t)fsize, "fread did not return expected size");
    str[sz] = '\0'; // Terminate string
    DBUG_PRINT ("sz=%d, strlen(str)=%d", sz, strlen (str));
    FMGRclose (matrix_file);

    DBUG_RETURN (str);
}

/** <!-- ****************************************************************** -->
 *
 * @fn int ISLUgetLoopCount( char *str, lut_t *varlut)
 *
 * @brief
 *
 * @param A string, suitable for input to ISL, representing an
 *        ISL union set.
 *
 * @return the loop count for the union set, if ISL can determine it,
 *         or UNR_NONE.
 *
 ******************************************************************************/
int
ISLUgetLoopCount (char *str, lut_t *varlut)
{
    // Try to find integer constant iteration count of FOR-loop,
    // described by ISL union set str.
    // If we can not determine the loop count, return UNR_NONE.

    int z = UNR_NONE;
#if ENABLE_ISL && ENABLE_BARVINOK
    struct isl_union_pw_qpolynomial *pwcard = NULL;
    struct isl_ctx *ctx = NULL;
    isl_space *dim = NULL;
    isl_point *zro = NULL;
    isl_val *val = NULL;
    struct isl_union_set *dom = NULL;
#endif // ENABLE_ISL && ENABLE_BARVINOK

    DBUG_ENTER ();

#if ENABLE_ISL && ENABLE_BARVINOK
    ctx = isl_ctx_alloc ();
    dom = isl_union_set_read_from_str (ctx, str);
    if (NULL == dom) {
        fprintf (stderr, "LoopCount expected union set, got something invalid\n");
    }
    pwcard = isl_union_set_card (dom);

    // Attempt to extract value from pwcard.
    dim = isl_union_pw_qpolynomial_get_space (pwcard);
    zro = isl_point_zero (isl_space_copy (dim));
    val = isl_union_pw_qpolynomial_eval (pwcard, zro);
    z = (NULL != val) ? isl_val_get_num_si (val) : z;
    z = (0 == z) ? UNR_NONE : z;
    isl_val_free (val);
    isl_space_free (dim);
    isl_ctx_free (ctx);
#endif // ENABLE_ISL && ENABLE_BARVINOK

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn int ISLUgetSetIntersections( ...)
 *
 * @brief Compute the intersect of exprspwl and exprscwl,
 *        and that result against exprsfn and exprscfn.
 *        A NULL result is the only interesting one, used by
 *        POGO.
 *
 *        If exprscond is supplied, we blindly intersect it with exprspwl
 *        before proceeding.
 *
 * @param: exprs*: Four PHUTish N_exprs chains, each
 *         comprising ISL union set, or equivalent.
 * @param: exprscond: An optional N_exprs chain, representing the
 *         AFT for the COND_COND of a loopfun.
 * @param: varlut: Address of the LUT containing the union set variable names.
 * @param: lhsname: the AVIS_NAME of the LHS of the expression
 *         we are trying to simplify. This is for debugging only...
 *
 *
 * @return One of the ISL set results.
 *
 ******************************************************************************/
int
ISLUgetSetIntersections (node *exprspwl, node *exprscwl, node *exprscond, node *exprsfn,
                         node *exprscfn, lut_t *varlut, char *lhsname)
{
    int z = POLY_RET_UNKNOWN;
#if ENABLE_ISL && ENABLE_BARVINOK
    struct isl_ctx *ctx = NULL;
    char *str;
    struct isl_union_set *dompwl = NULL;
    struct isl_union_set *domcwl = NULL;
    struct isl_union_set *domcond = NULL;
    struct isl_union_map *mapfn = NULL;
    struct isl_union_map *mapcfn = NULL;
    struct isl_union_set *intpc = NULL;
    struct isl_union_set *intersectpc = NULL;
    struct isl_union_set *intersectpcf = NULL;
    struct isl_union_set *intersectpcc = NULL;

    DBUG_ENTER ();

    ctx = isl_ctx_alloc ();

    str = ISLUexprs2String (exprspwl, varlut, "pwl for intersect", TRUE, lhsname);
    dompwl = isl_union_set_read_from_str (ctx, str);
    DBUG_ASSERT (NULL != dompwl, "ISL did not like exprspwl as union set");
    DBUG_PRINT ("pwl is %s", str);
    str = MEMfree (str);
    DBUG_EXECUTE (ISLUprintUnionSet (stderr, dompwl, "dompwl"));
    // If dompwl is empty set, we stop right now.
    if (isl_union_set_is_empty (dompwl)) {
        z = z | POLY_RET_EMPTYSET_B;
        z = z & ~POLY_RET_UNKNOWN;
        DBUG_PRINT ("pwl is null set\n");
    } else {
        DBUG_PRINT ("pwl is non-null set\n");
    }

    // If caller supplied exprscond, blindly intersect it with exprspwl
    if ((POLY_RET_UNKNOWN == z) && (NULL != exprscond)) {
        str = ISLUexprs2String (exprscond, varlut, "cond for intersect", TRUE, lhsname);
        domcond = isl_union_set_read_from_str (ctx, str);
        DBUG_PRINT ("cond is %s", str);
        str = MEMfree (str);
        DBUG_EXECUTE (ISLUprintUnionSet (stderr, domcond, "domcond"));
        dompwl = isl_union_set_intersect (isl_union_set_copy (dompwl),
                                          isl_union_set_copy (domcond));
    }

    if (POLY_RET_UNKNOWN == z) {
        str = ISLUexprs2String (exprscwl, varlut, "cwl for intersect", TRUE, lhsname);
        domcwl = isl_union_set_read_from_str (ctx, str);
        // No check on domcwl, as it can be elided for monadic calls
        DBUG_PRINT ("cwl is %s", str);
        str = MEMfree (str);
        DBUG_EXECUTE (ISLUprintUnionSet (stderr, domcwl, "domcwl"));

        str = ISLUexprs2String (exprsfn, varlut, "fn for intersect", FALSE, lhsname);
        mapfn = isl_union_map_read_from_str (ctx, str);
        DBUG_ASSERT (NULL != mapfn, "ISL did not like mapfn as union map");
        DBUG_PRINT ("fn is %s", str);
        str = MEMfree (str);
        DBUG_EXECUTE (ISLUprintUnionMap (stderr, mapfn, "mapfn"));

        str = ISLUexprs2String (exprscfn, varlut, "cfn for intersect", FALSE, lhsname);
        mapcfn = isl_union_map_read_from_str (ctx, str);
        DBUG_PRINT ("cfn is %s", str);
        DBUG_ASSERT (NULL != mapcfn, "ISL did not like mapcfn as union map");
        str = MEMfree (str);
        DBUG_EXECUTE (ISLUprintUnionMap (stderr, mapcfn, "mapcfn"));

        intersectpc = isl_union_set_intersect (isl_union_set_copy (dompwl),
                                               isl_union_set_copy (domcwl));
        DBUG_ASSERT (NULL != intersectpc,
                     "ISL did not like intersectpc as union set intersect");
        DBUG_EXECUTE (ISLUprintUnionSet (stderr, intersectpc, "intersectpc"));

        if (POLY_RET_UNKNOWN == z) { // Intersect dompwl,domcwl,mapfn
            intersectpcf = isl_union_set_apply (isl_union_set_copy (intersectpc),
                                                isl_union_map_copy (mapfn));
            DBUG_EXECUTE (ISLUprintUnionSet (stderr, intersectpcf, "intersectpcf"));
            if (isl_union_set_is_empty (intersectpcf)) {
                z = z | POLY_RET_EMPTYSET_BCF;
                z = z & ~POLY_RET_UNKNOWN;
                DBUG_PRINT ("no intersect for pwl,cwl,fn\n");
            } else {
                DBUG_PRINT ("(pwl, cwl) intersects fn\n");
            }
        }

        if (POLY_RET_UNKNOWN == z) { // Intersect dompwl,domcwl,mapcfn
            intersectpcc = isl_union_set_apply (isl_union_set_copy (intersectpc),
                                                isl_union_map_copy (mapcfn));
            DBUG_EXECUTE (ISLUprintUnionSet (stderr, intersectpcc, "intersectpcc"));
            if (isl_union_set_is_empty (intersectpcc)) {
                DBUG_PRINT ("no intersect in pwl,cwl,cfn");
                z = z | POLY_RET_EMPTYSET_BCG;
                z = z & ~POLY_RET_UNKNOWN;
            } else {
                DBUG_PRINT ("(pwl, cwl) intersects cfn");
            }
        }
    }

    isl_union_set_free (dompwl);
    isl_union_set_free (domcwl);
    isl_union_set_free (domcond);
    isl_union_map_free (mapfn);
    isl_union_map_free (mapcfn);
    isl_union_set_free (intpc);
    isl_union_set_free (intersectpc);
    isl_union_set_free (intersectpcc);
    isl_union_set_free (intersectpcf);
#else  // ENABLE_ISL && ENABLE_BARVINOK
    DBUG_ENTER ();
#endif // ENABLE_ISL && ENABLE_BARVINOK

    DBUG_RETURN (z);
}

#undef DBUG_PREFIX
