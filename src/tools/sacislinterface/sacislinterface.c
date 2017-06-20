/*
 * Interface between sac2c and isl
 *
 */

#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
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
#include <isl/schedule.h>
#include <isl/ast_build.h>
#include <barvinok/isl.h>
#include "polyhedral_defs.h"

void
printBasicSet (struct isl_basic_set *pset, char *titl)
{ // Print a BasicSet to stdout

    isl_printer *p;
    struct isl_ctx *ctx;

    printf ("Printing %s\n", titl);
    ctx = isl_basic_set_get_ctx (pset);
    p = isl_printer_to_file (ctx, stdout);
    p = isl_printer_print_basic_set (p, pset);
    p = isl_printer_end_line (p);
    isl_printer_free (p);
    printf ("Done printing %s\n", titl);
    return;
}

#ifdef DEADCODE
// This from Roman Gareev, isl-development google group
// 2014-03-08.

static __isl_give isl_ast_build *
set_options (__isl_take isl_ast_build *control, __isl_keep isl_union_map *schedule)
{
    isl_ctx *ctx = isl_union_map_get_ctx (schedule);
    isl_space *range_space = isl_space_set_alloc (ctx, 0, 1);
    range_space = isl_space_set_tuple_name (range_space, isl_dim_set, "separate");
    isl_union_set *range = isl_union_set_from_set (isl_set_universe (range_space));
    isl_union_set *domain = isl_union_map_range (isl_union_map_copy (schedule));
    domain = isl_union_set_universe (domain);
    isl_union_map *options = isl_union_map_from_domain_and_range (domain, range);
    return isl_ast_build_set_options (control, options);
}
#endif // DEADCODE

void
printSchedule (FILE *fd, isl_schedule *sched, char *titl, int verbose, int fmt)
{
    // Print a schedule. Use fmt as format, if non-zero.
    isl_printer *p;
    struct isl_ctx *ctx;

    if (verbose) {
        ctx = isl_schedule_get_ctx (sched);
        fprintf (fd, "Union %s is:\n", titl);
        p = isl_printer_to_file (ctx, fd);
        if (0 != fmt) {
            p = isl_printer_set_output_format (p, fmt);
        }
        p = isl_printer_print_schedule (p, sched);
        p = isl_printer_end_line (p);
        isl_printer_free (p);
    }
    return;
}

void
printAst (FILE *fd, isl_ast_node *ast, char *titl, int verbose, int fmt)
{
    // Print an ast. Use fmt as format, if non-zero.
    isl_printer *p;
    struct isl_ctx *ctx;

    if (verbose) {
        ctx = isl_ast_node_get_ctx (ast);
        fprintf (fd, "AST %s is:\n", titl);
        p = isl_printer_to_file (ctx, fd);
        if (0 != fmt) {
            p = isl_printer_set_output_format (p, fmt);
        }
        p = isl_printer_print_ast_node (p, ast);
        p = isl_printer_end_line (p);
        isl_printer_free (p);
    }
    return;
}

void
printMap (FILE *fd, struct isl_union_map *pmap, char *titl, int verbose, int fmt)
{
    // Print a union_map. Use fmt as format, if non-zero.
    // no fmt support
    isl_printer *p;
    struct isl_ctx *ctx;

    if (verbose) {
        ctx = isl_union_map_get_ctx (pmap);
        fprintf (fd, "Union %s is:\n", titl);
        p = isl_printer_to_file (ctx, fd);
        p = isl_printer_print_union_map (p, pmap);
        p = isl_printer_end_line (p);
        isl_printer_free (p);
    }
    return;
}

void
printUnion (FILE *fd, struct isl_union_set *pset, char *titl, int verbose, int fmt)
{
    // Print a union_set. Use fmt as format, if non-zero.
    isl_printer *p;
    struct isl_ctx *ctx;

    if (verbose) {
        ctx = isl_union_set_get_ctx (pset);
        fprintf (fd, "Union %s is:\n", titl);
        p = isl_printer_to_file (ctx, fd);
        if (0 != fmt) {
            p = isl_printer_set_output_format (p, fmt);
        }
        p = isl_printer_print_union_set (p, pset);
        p = isl_printer_end_line (p);
        isl_printer_free (p);
    }
    return;
}

#ifdef CRUD
// Stolen from iscc.c
static struct isl_obj
convert (isl_ctx *ctx, struct isl_obj obj, isl_obj_type type)
{
    if (obj.type == type)
        return obj;
    if (obj.type == isl_obj_map && type == isl_obj_union_map) {
        obj.type = isl_obj_union_map;
        obj.v = isl_union_map_from_map (obj.v);
        return obj;
    }
    if (obj.type == isl_obj_set && type == isl_obj_union_set) {
        obj.type = isl_obj_union_set;
        obj.v = isl_union_set_from_set (obj.v);
        return obj;
    }
    if (obj.type == isl_obj_pw_qpolynomial && type == isl_obj_union_pw_qpolynomial) {
        obj.type = isl_obj_union_pw_qpolynomial;
        obj.v = isl_union_pw_qpolynomial_from_pw_qpolynomial (obj.v);
        return obj;
    }
    if (obj.type == isl_obj_pw_qpolynomial_fold
        && type == isl_obj_union_pw_qpolynomial_fold) {
        obj.type = isl_obj_union_pw_qpolynomial_fold;
        obj.v = isl_union_pw_qpolynomial_fold_from_pw_qpolynomial_fold (obj.v);
        return obj;
    }
    if (obj.type == isl_obj_union_set && isl_union_set_is_empty (obj.v)) {
        if (type == isl_obj_union_map) {
            obj.type = isl_obj_union_map;
            return obj;
        }
        if (type == isl_obj_union_pw_qpolynomial) {
            isl_space *dim = isl_union_set_get_space (obj.v);
            isl_union_set_free (obj.v);
            obj.v = isl_union_pw_qpolynomial_zero (dim);
            obj.type = isl_obj_union_pw_qpolynomial;
            return obj;
        }
        if (type == isl_obj_union_pw_qpolynomial_fold) {
            isl_space *dim = isl_union_set_get_space (obj.v);
            isl_union_set_free (obj.v);
            obj.v = isl_union_pw_qpolynomial_fold_zero (dim, isl_fold_list);
            obj.type = isl_obj_union_pw_qpolynomial_fold;
            return obj;
        }
    }
    if (obj.type == isl_obj_list) {
        struct isl_list *list = obj.v;
        if (list->n == 2 && list->obj[1].type == isl_obj_bool)
            return convert (ctx, obj_at (obj, 0), type);
    }
    if (type == isl_obj_str) {
        isl_str *str;
        isl_printer *p;
        char *s;

        p = isl_printer_to_str (ctx);
        if (!p)
            goto error;
        p = obj.type->print (p, obj.v);
        s = isl_printer_get_str (p);
        isl_printer_free (p);

        str = isl_str_from_string (ctx, s);
        if (!str)
            goto error;
        free_obj (obj);
        obj.v = str;
        obj.type = isl_obj_str;
        return obj;
    }
error:
    free_obj (obj);
    obj.type = isl_obj_none;
    obj.v = NULL;
    return obj;
}

// Stolen from iscc.c
//
/* Generate an AST for the given schedule and options and print
 * the AST on the printer.
 */
static __isl_give isl_printer *
print_code (__isl_take isl_printer *p, __isl_take isl_union_map *schedule,
            __isl_take isl_union_map *options)
{
    isl_set *context;
    isl_ast_build *build;
    isl_ast_node *tree;
    int format;

    context = isl_set_universe (isl_union_map_get_space (schedule));

    build = isl_ast_build_from_context (context);
    build = isl_ast_build_set_options (build, options);
    tree = isl_ast_build_ast_from_schedule (build, schedule);
    isl_ast_build_free (build);

    if (!tree)
        return p;

    format = isl_printer_get_output_format (p);
    p = isl_printer_set_output_format (p, ISL_FORMAT_C);
    p = isl_printer_print_ast_node (p, tree);
    p = isl_printer_set_output_format (p, format);

    isl_ast_node_free (tree);

    return p;
}

isl_printer *
Codegen (FILE *fd, struct isl_union_set *pset)
{ // Perform code generation for a PWLF intersect

    isl_printer *p;
    isl_union_map *schedule;
    isl_union_map *options;
    struct isl_ctx *ctx;
    struct isl_obj obj;

    ctx = isl_union_set_get_ctx (pset);
    p = isl_printer_to_file (ctx, fd);

    obj = convert (ctx, obj, isl_obj_union_set);
    schedule = isl_union_set_identity (obj.v);
    options = isl_union_map_empty (isl_union_map_get_space (schedule));
    p = print_code (p, schedule, options);

    return (p);
}
#endif // CRUD

static struct isl_union_set *
doMap (struct isl_union_set *unionb, struct isl_union_map *mapc, char *titl, int verbose)
{ // apply map to set and return the resulting set
    struct isl_union_set *intersectbc;

    intersectbc
      = isl_union_set_apply (isl_union_set_copy (unionb), isl_union_map_copy (mapc));
    if (verbose) {
        printUnion (stderr, intersectbc, titl, verbose, 0);
    }

    return (intersectbc);
}

static struct isl_union_set *
doIntersect (struct isl_union_set *unionb, struct isl_union_set *unionc, char *titl,
             int verbose)
{ // Intersect two sets and return the resulting set
    struct isl_union_set *intersectbc;

    intersectbc = isl_union_set_intersect (isl_union_set_copy (unionb),
                                           isl_union_set_copy (unionc));
    if (verbose) {
        printUnion (stderr, intersectbc, titl, verbose, 0);
    }

    return (intersectbc);
}

int
PolyhedralWLFIntersectCalc (int verbose)
{
    // cwl is the consumerWL index vector's affine expn tree;
    // pwl is the producerWL bounds, step, width affine expn tree.
    //
    // If the intersect of cwl and pwl is cwl, then the fold
    // can proceed immediately. If the intersect is NULL, then
    // no folding can happen. Otherwise, the cwl must be sliced.
    //
    int z = POLY_RET_UNKNOWN;
    struct isl_ctx *ctx = isl_ctx_alloc ();
    struct isl_union_set *pwl = NULL;
    struct isl_union_set *cwl = NULL;
    struct isl_union_set *peq = NULL;
    struct isl_union_set *intr = NULL;
    struct isl_union_set *cwleq = NULL;
    struct isl_union_set *pwleq = NULL;
    struct isl_set *domain = NULL;
    isl_ast_build *build = NULL;
    isl_schedule *sched = NULL;
    isl_schedule_constraints *schedcon = NULL;
    isl_ast_node *ast = NULL;

    pwl = isl_union_set_read_from_file (ctx, stdin);
    cwl = isl_union_set_read_from_file (ctx, stdin);
    peq = isl_union_set_read_from_file (ctx, stdin);
    printUnion (stderr, pwl, "pwl", verbose, 0);
    printUnion (stderr, cwl, "cwl", verbose, 0);
    printUnion (stderr, peq, "peq", verbose, 0);

    // ISL: Is (cwl * eq) = ( cwl * eq * pwl)?
    cwleq = doIntersect (cwl, peq, "cwleq", verbose);
    pwleq = doIntersect (pwl, peq, "pwleq", verbose);
    intr = doIntersect (cwleq, pwleq, "intr", verbose);
    if (isl_union_set_is_subset (cwleq, intr)) {
        z = z | POLY_RET_CCONTAINSB;
        z = z & ~POLY_RET_UNKNOWN;
        if (verbose) {
            fprintf (stderr, "cwleq is subset of intersect( cwleq, pwleq)\n");
        }
    } else {
        if (isl_union_set_is_empty (intr)) {
            z = z | POLY_RET_EMPTYSET_BCF;
            z = z & ~POLY_RET_UNKNOWN;
            if (verbose) {
                fprintf (stderr, "cwleq and pwleq do not intersect\n");
            }
        }
    }

    if (POLY_RET_UNKNOWN == z) {
        z = z | POLY_RET_SLICENEEDED;
        z = z & ~POLY_RET_UNKNOWN;
        // Attach a default schedule to the domain
        schedcon = isl_schedule_constraints_on_domain (intr);
        sched = isl_schedule_constraints_compute_schedule (schedcon);
        printSchedule (stdout, sched, "schedule", 1, ISL_FORMAT_ISL);

        // Create a generic "build"
        build = isl_ast_build_alloc (ctx);

        // Perform the ISL codegen operation, using the intersect data.
        ast = isl_ast_build_node_from_schedule (build, sched);

        fprintf (stdout, "\n using { [b] -> separate[x] };");
        printAst (stdout, ast, "ISL AST calculated is:", verbose, ISL_FORMAT_C);
    }

    fprintf (stdout, "%d\n", z); // Write the result that PHUT will read.
    fprintf (stdout, "ast \n");

    isl_union_set_free (pwl);
    isl_union_set_free (cwl);
    isl_union_set_free (peq);
    isl_union_set_free (intr);
    isl_union_set_free (cwleq);
    isl_union_set_free (pwleq);
    isl_schedule_free (sched);
    isl_ast_node_free (ast);
    isl_ast_build_free (build);
    isl_set_free (domain);
    isl_ctx_free (ctx);

    return (z);
}

int
PolyhedralRelationalCalc (int verbose)
{ // Check for matching polyhedra in relationals for POGO
    //   Read four Polylib polyhedra (B,C,rfn,cfn) from stdin,
    //   where B and C are PRF_ARG1 and PRF_ARG2 for the relational,
    //   and where rfn represents the relational itself;
    //   cfn represents the complementary function for the relational.
    //   E.g., for B < C, the complementary relational is B >= C.
    //   Intersect B,C,rfn, and return result of POLY_RET_EMPTYSET_BCF if the
    //   intersect is a NULL set.
    //
    //   Else, intersect B,C,cfn and return result of POLY_RET_EMPTYSET_BCG if the
    //   intersect is a NULL set.
    //
    //   Else, return POLY_RET_UNKNOWN.

    int z = POLY_RET_UNKNOWN;

    struct isl_union_set *unionb = NULL;
    struct isl_union_set *unionc = NULL;
    struct isl_union_map *fn = NULL;
    struct isl_union_map *cfn = NULL;
    struct isl_union_set *intersectbc = NULL;
    struct isl_union_set *intersectbcr = NULL;
    struct isl_union_set *intersectbcc = NULL;
    struct isl_ctx *ctx = isl_ctx_alloc ();

    unionb = isl_union_set_read_from_file (ctx, stdin);
    printUnion (stderr, unionb, "unionb", verbose, 0);

    unionc = isl_union_set_read_from_file (ctx, stdin);
    printUnion (stderr, unionc, "unionc", verbose, 0);

    fn = isl_union_map_read_from_file (ctx, stdin);
    printMap (stderr, fn, "fn", verbose, 0);

    cfn = isl_union_map_read_from_file (ctx, stdin);
    printMap (stderr, cfn, "cfn", verbose, 0);

    if (POLY_RET_UNKNOWN == z) { // Intersect b,c
        intersectbc = doIntersect (unionb, unionc, "intersectbc", verbose);
        if (isl_union_set_is_empty (intersectbc)) {
            z = z | POLY_RET_EMPTYSET_BC;
            z = z & ~POLY_RET_UNKNOWN;
            if (verbose) {
                fprintf (stderr, "b does not intersect c\n");
            }
        } else {
            if (verbose) {
                fprintf (stderr, "b intersects c\n");
            }
        }
    }

    if (POLY_RET_UNKNOWN == z) { // Intersect b,c,fn
        intersectbcr = doMap (intersectbc, fn, "intersectbcr", verbose);
        if (isl_union_set_is_empty (intersectbcr)) {
            z = z | POLY_RET_EMPTYSET_BCF;
            z = z & ~POLY_RET_UNKNOWN;
            if (verbose) {
                fprintf (stderr, "no intersect in b,c,fn\n");
            }
        } else {
            if (verbose) {
                fprintf (stderr, "(b,c) intersects fn\n");
            }
        }
    }

    if (POLY_RET_UNKNOWN == z) { // Intersect b,c,cfn
        intersectbcc = doMap (intersectbc, cfn, "intersectbcc", verbose);
        if (isl_union_set_is_empty (intersectbcc)) {
            if (verbose) {
                fprintf (stderr, "no intersect in b,c,cfn\n");
            }
            z = z | POLY_RET_EMPTYSET_BCG;
            z = z & ~POLY_RET_UNKNOWN;
        } else {
            if (verbose) {
                fprintf (stderr, "(b,c) intersects cfn\n");
            }
        }
    }

    isl_union_set_free (unionb);
    isl_union_set_free (unionc);
    isl_union_map_free (fn);
    isl_union_map_free (cfn);
    isl_union_set_free (intersectbc);
    isl_union_set_free (intersectbcr);
    isl_union_set_free (intersectbcc);
    isl_ctx_free (ctx);

    printf ("%d\n", z);

    return (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn int ISLUgetLoopCount(...)
 *
 * stolen from ISLU
 *
 *
 ******************************************************************************/
static int
ISLUgetLoopCount (__isl_keep isl_union_set *dom, int verbose)
{

    int z = UNR_NONE;
    struct isl_union_pw_qpolynomial *pwcard = NULL;
    struct isl_ctx *ctx;
    isl_space *dim = NULL;
    isl_point *zro = NULL;
    isl_val *val = NULL;

    ctx = isl_union_set_get_ctx (dom);
    if (NULL == dom) {
        fprintf (stderr, "LoopCount expected union set, got something invalid\n");
    }

    // Attempt to extract value from pwcard.
    pwcard = isl_union_set_card (dom);
    dim = isl_union_pw_qpolynomial_get_space (pwcard);
    zro = isl_point_zero (isl_space_copy (dim));
    val = isl_union_pw_qpolynomial_eval (pwcard, zro);
    z = (NULL != val) ? isl_val_get_num_si (val) : z;
    z = (0 == z) ? UNR_NONE : z;
    isl_val_free (val);
    isl_space_free (dim);
    isl_ctx_free (ctx);

    return (z);
}

int
LoopCount (__isl_keep isl_union_set *domain, int verbose)
{
    // Try to find integer constant iteration count of FOR-loop.
    // If no luck, return -1.
    //
    // We fake up a schedule, merely so that we can get at the AST.

    int z = -1;

    isl_val *V;
    int Dim = -1;
    struct isl_set *LoopDomain = NULL;

    printUnion (stderr, domain, "domain for LoopCount is ", verbose, 0);
    Dim = 1; // Vector only

    // Calculate a map similar to the identity map, but with the last input
    // and output dimension not related.
    //  [i0, i1, i2, i3] -> [i0, i1, i2, o0]
    isl_space *Space = isl_set_get_space (LoopDomain);
    Space = isl_space_drop_dims (Space, isl_dim_out, Dim - 1, 1);
    Space = isl_space_map_from_set (Space);
    isl_map *Identity = isl_map_identity (Space);
    Identity = isl_map_add_dims (Identity, isl_dim_in, 1);
    Identity = isl_map_add_dims (Identity, isl_dim_out, 1);

    LoopDomain = isl_set_reset_tuple_id (LoopDomain);

    isl_map *Map = isl_map_from_domain_and_range (isl_set_copy (LoopDomain),
                                                  isl_set_copy (LoopDomain));
    isl_set_free (LoopDomain);
    Map = isl_map_intersect (Map, Identity);

    isl_map *LexMax = isl_map_lexmax (isl_map_copy (Map));
    isl_map *LexMin = isl_map_lexmin (Map);
    isl_map *Sub = isl_map_sum (LexMax, isl_map_neg (LexMin));

    isl_set *Elements = isl_map_range (Sub);

    if (isl_set_is_singleton (Elements)) {
        printf ("set is singleton\n");
        // I think this works because there is only one element.
        isl_point *P = isl_set_sample_point (Elements);
        Dim = 1;
        V = isl_point_get_coordinate_val (P, isl_dim_set, Dim - 1);
        z = isl_val_get_num_si (V);
        printf ("z is %d\n", z);
        isl_val_free (V);
        isl_point_free (P);
        z = (-1 != z) ? z + 1 : z; // iteration count = 1 + LexMax-MexMin
    } else {
        isl_set_free (Elements);
    }
    return (z);
}

int
PolyhedralLoopCount (int verbose)
{ // Attempt to determine a constant loop count for a FOR-loop,
    // from a union set.
    int z;
    int z2;
    struct isl_union_set *islus;
    struct isl_ctx *ctx = isl_ctx_alloc ();

    islus = isl_union_set_read_from_file (ctx, stdin);

    z = LoopCount (islus, verbose);
    z2 = ISLUgetLoopCount (islus, verbose);
    printf ("LoopCount: %d, ISLgetLoopCount: %d\n", z, z2);

    isl_union_set_free (islus);

    return (z);
}

int
main (int argc, char *argv[])
{
    char opcode;
    int z = POLY_RET_UNKNOWN;
    int verbose = 0;

    if (argc > 1) {
        opcode = *argv[1];
    } else {
        fprintf (stderr, "Specify opcode as command-line argument, please");
        exit (EXIT_FAILURE);
    }

    if (argc > 2) { // Any second argument forces verbosity. This is crude, I admit.
        verbose = 1;
        fprintf (stderr, "Running in verbose mode\n");
    }

    switch (opcode) {

    case POLY_OPCODE_INTERSECT: // POlyhedral Guard Optimization (POGO)
        z = PolyhedralRelationalCalc (verbose);
        break;

    case POLY_OPCODE_PWLF: // Polyhedral WLF (PWLF)
        z = PolyhedralWLFIntersectCalc (verbose);
        break;

    case POLY_OPCODE_LOOPCOUNT: // Polyhedral Setup (and loop count analysis)
        z = PolyhedralLoopCount (verbose);
        break;

    default:
        fprintf (stderr, "caller is confused. We got opcode=%c\n", opcode);
        // break elision is intentional

    case POLY_OPCODE_HELP:
    case POLY_OPCODE_HELPLC:
        fprintf (stderr, "legal opcodes are %c, %c, %c, %c, %c\n", POLY_OPCODE_INTERSECT,
                 POLY_OPCODE_PWLF, POLY_OPCODE_LOOPCOUNT, POLY_OPCODE_HELP,
                 POLY_OPCODE_HELPLC);
        break;
    }

    if (verbose) {
        fprintf (stderr, "result is %d\n", z);
    }

    return (z);
}
