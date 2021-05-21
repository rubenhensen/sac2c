#include "icm2c_basic.h"

#define DBUG_PREFIX "COMP"
#include "debug.h"

#include "str.h"
#include "globals.h"
#include "print.h"
#include "icm2c_utils.h"

/*
 * NOTE In this file we are going to use 8-space indentation, as otherwise
 * the code is simply unreadable.  It can be adjusted later, in case one is
 * stong enough to simplify it.
 */

int print_comment = 1; /* bool */

/******************************************************************************
 *
 * Function:
 *   void Check_Mirror( char *to_NT, int to_sdim,
 *                      void *shp1, int shp1_size,
 *                      void (*shp1_size_fun)( void *),
 *                      void (*shp1_read_fun)( void *, char *, int),
 *                      void *shp2, int shp2_size,
 *                      void (*shp2_size_fun)( void *),
 *                      void (*shp2_read_fun)( void *, char *, int))
 *
 * Description:
 *   Assures that the constant part of the mirror of 'to_NT' contains correct
 *   values with respect to the shape 'shp1 ++ shp2'.
 *
 *   'shp?'           a data structure representing a shape.
 *                    This may be an integer array (int *) of shape components,
 *                    or a string array (char **) of shape components, or a
 *                    tagged identifier (char *) from which shape information
 *                    can be obtained somehow, e.g. from its data vector or its
 *                    shape vector.
 *   'shp?_size'      >=0: number of shape components (if statically known).
 *                    < 0: number of shape components not statically known ---
 *                         'shp?_size_fun' must be used.
 *   'shp?_size_fun'  ==NULL: number of shape components is statically known
 *                            (see 'shp?_size').
 *                    !=NULL: points to a function which prints the number of
 *                            shape components to 'outfile'.
 *   'shp?_read_fun'  points to a function which prints a read access to a
 *                    shape component (using 'shp?' as argument) to 'outfile'.
 *
 ******************************************************************************/

void
Check_Mirror (char *to_NT, int to_sdim, void *shp1, int shp1_size,
              void (*shp1_size_fun) (void *), void (*shp1_read_fun) (void *, char *, int),
              void *shp2, int shp2_size, void (*shp2_size_fun) (void *),
              void (*shp2_read_fun) (void *, char *, int))
{
#define ASSURE_TYPE_HEADER() indout ("SAC_ASSURE_TYPE_LINE ((")
#define ASSURE_TYPE_FOOTER()                                                             \
    out ("), %zu, \"Assignment with incompatible types found!\""                          \
         ");\n",                                                                         \
         global.linenum)
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ();

    DBUG_PRINT ("checking mirror of %s", to_NT);

    DBUG_ASSERT (shp1_read_fun != NULL, "1st shape-read-fun not found!");
    if (shp2 == NULL) {
        DBUG_ASSERT (shp2_size == 0, "inconsistant 2nd shape found!");
    } else {
        DBUG_ASSERT (shp2_read_fun != NULL, "2nd shape-read-fun not found!");
    }

    /* Check of SAC_ND_A_DIM.  */
    if (to_sc != C_aud) {
        ASSURE_TYPE_HEADER ()
            ;
            out ("SAC_ND_A_DIM( %s) == ", to_NT);
            GetAttr (shp1, shp1_size, shp1_size_fun);
            if (shp2 != NULL) {
                out (" + ");
                GetAttr (shp2, shp2_size, shp2_size_fun);
            }
        ASSURE_TYPE_FOOTER ();
    }

    /* Simplify 'shp1_size', 'shp2_size'  */
    if (to_dim >= 0) {
        if (shp1_size >= 0 && shp2_size >= 0) {
            DBUG_ASSERT (shp1_size == to_dim - shp2_size,
                         "inconsistant dimensions/sizes found!");
        } else if (shp1_size < 0 && shp2_size >= 0) {
            shp1_size = to_dim - shp2_size;
        } else if (shp1_size >= 0 && shp2_size < 0) {
            shp2_size = to_dim - shp1_size;
        } else {
            /* ((shp1_size < 0) && (shp2_size < 0)) */
        }
    }

    /* Check of SAC_ND_A_SHAPE.  */
    if (to_sc == C_scl || to_sc == C_aks) {
        DBUG_ASSERT (to_dim >= 0, "illegal dimension found!");
        if (shp1_size >= 0) {
            for (i = 0; i < shp1_size; i++) {
                ASSURE_TYPE_HEADER ()
                    ;
                    out ("SAC_ND_A_SHAPE( %s, %d) == ", to_NT, i);
                    shp1_read_fun (shp1, NULL, i);
                ASSURE_TYPE_FOOTER ();
            }

            for (; i < to_dim; i++) {
                DBUG_ASSERT (shp2 != NULL, "second shape not found!");

                ASSURE_TYPE_HEADER ()
                    ;
                    out ("SAC_ND_A_SHAPE( %s, %d) == ", to_NT, i);
                    shp2_read_fun (shp2, NULL, i - shp1_size);
                ASSURE_TYPE_FOOTER ();
            }

        } else {
            for (i = 0; i < to_dim; i++) {
                DBUG_ASSERT (shp2 != NULL, "second shape not found!");

                /* FIXME This code seems to have a typo.
                   It also seems that it is never being executed.  */
                ASSURE_TYPE_HEADER ()
                    ;
                    out ("(%d < ", i);
                    shp1_size_fun (shp1);
                    out (" && SAC_ND_A_SHAPE( %s, %d) == ", to_NT, i);
                    shp1_read_fun (shp1, NULL, i);
                    out (") || (%d >= ", i);
                    /* XXX Shouldn't it be shp2 here?  */
                    shp1_size_fun (shp1);
                    out (" && SAC_ND_A_SHAPE( %s, %d) == ", to_NT, i);
                    shp2_read_fun (shp2, NULL, i - shp1_size);
                    out (")");
                ASSURE_TYPE_FOOTER ();
            }
        }
    }

    /* Check of SAC_ND_A_SIZE is missing here...  */
    if (to_sc == C_scl || to_sc == C_aks) {
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void Set_Shape( char *to_NT, int to_sdim,
 *                   void *shp1, int shp1_size,
 *                   void (*shp1_size_fun)( void *),
 *                   void (*shp1_prod_fun)( void *),
 *                   void (*shp1_read_fun)( void *, char *, int),
 *                   void *shp2, int shp2_size,
 *                   void (*shp2_size_fun)( void *),
 *                   void (*shp2_prod_fun)( void *),
 *                   void (*shp2_read_fun)( void *, char *, int))
 *
 * Description:
 *   Sets the non-constant part of the mirror and the relevant descriptor
 *   entries with respect to the shape 'shp1 ++ shp2'.
 *
 *   'shp?'           a data structure representing a shape.
 *                    This may be an integer array (int *) of shape components,
 *                    or a string array (char **) of shape components, or a
 *                    tagged identifier (char *) from which shape information
 *                    can be obtained somehow, e.g. from its data vector or its
 *                    shape vector.
 *   'shp?_size'      >=0: number of shape components (if statically known).
 *                    < 0: number of shape components not statically known ---
 *                         'shp?_size_fun' must be used.
 *   'shp?_size_fun'  ==NULL: number of shape components is statically known
 *                            (see 'shp?_size').
 *                    !=NULL: points to a function which prints the number of
 *                            shape components to 'outfile'.
 *   'shp?_prod_fun'  ==NULL: product of shape components must be computed.
 *                    !=NULL: points to a function which prints the product of
 *                            all shape components (using 'shp?' as argument)
 *                            to 'outfile'.
 *   'shp?_read_fun'  points to a function which prints a read access to a
 *                    shape component (using 'shp?' as argument) to 'outfile'.
 *
 ******************************************************************************/

void
Set_Shape (char *to_NT, int to_sdim, void *shp1, int shp1_size,
           void (*shp1_size_fun) (void *), void (*shp1_prod_fun) (void *),
           void (*shp1_read_fun) (void *, char *, int), void *shp2, int shp2_size,
           void (*shp2_size_fun) (void *), void (*shp2_prod_fun) (void *),
           void (*shp2_read_fun) (void *, char *, int))
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    distributed_class_t to_dc = ICUGetDistributedClass (to_NT);

    DBUG_ENTER ();
    DBUG_PRINT ("setting shape of %s", to_NT);
    DBUG_ASSERT (shp1_read_fun != NULL, "1st shape-read-fun not found!");

    if (shp2 == NULL) {
        DBUG_ASSERT (shp2_size == 0, "inconsistant 2nd shape found!");
    } else {
        DBUG_ASSERT (shp2_read_fun != NULL, "2nd shape-read-fun not found!");
    }

    if (to_dim >= 0) {
        if (shp1_size >= 0 && shp2_size >= 0) {
            DBUG_ASSERT (shp1_size == to_dim - shp2_size,
                         "inconsistant dimensions/sizes found!");
        } else if ((shp1_size < 0) && (shp2_size >= 0)) {
            shp1_size = to_dim - shp2_size;
        } else if ((shp1_size >= 0) && (shp2_size < 0)) {
            shp2_size = to_dim - shp1_size;
        } else {
            /* ((shp1_size < 0) && (shp2_size < 0)) */
        }
    }

    /* Check constant part of mirror.  */
    Check_Mirror (to_NT, to_sdim, shp1, shp1_size, shp1_size_fun, shp1_read_fun, shp2,
                  shp2_size, shp2_size_fun, shp2_read_fun);

    /* Set descriptor entries and non-constant part of mirror.  */
    switch (to_sc) {
    case C_aud:
        /*
         * ND_A_DESC_DIM, ND_A_MIRROR_DIM have already been
         * set by ND_ALLOC__DESC!
         */
        ASSURE_TYPE_HEADER ()
            ;
            out ("SAC_ND_A_DIM( %s) == ", to_NT);
            GetAttr (shp1, shp1_size, shp1_size_fun);
            if (shp2 != NULL) {
                out (" + ");
                GetAttr (shp2, shp2_size, shp2_size_fun);
            }
        ASSURE_TYPE_FOOTER ()
        ;

        BLOCK_BEGIN ("int SAC_i;")
            ;
            if (shp2_size < 0) {
                indout ("int SAC_j;\n");
            }

            if (shp1_prod_fun == NULL || shp2_prod_fun == NULL) {
                indout ("int SAC_size = 1;\n");
            }

            /*
             * Although 'to_NT' is AUD, 'to_dim' may indeed be >=0 if
             * the sac2c flag -minarrayrep has been used, i.e.
             * 'to_NT' may be implemented as AUD although it is
             * AKD!
             */
            if (shp1_size >= 0) {
                DBUG_ASSERT (shp1_size >= 0, "illegal dimension found");
                for (i = 0; i < shp1_size; i++) {
                    if (shp1_prod_fun == NULL) {
                        out ("SAC_size *= \n");
                    }
                    SET_SHP_AUD_NUM (to_NT, i, shp1_read_fun (shp1, NULL, i));
                }
            } else {
                indout ("for (SAC_i = 0; SAC_i < ");
                shp1_size_fun (shp1);
                out ("; SAC_i++)\n");
                BLOCK_NOVAR_BEGIN ()
                    ;
                    if (shp1_prod_fun == NULL) {
                        out ("SAC_size *= \n");
                    }
                    SET_SHP_AUD (to_NT, out ("SAC_i"), shp1_read_fun (shp1, "SAC_i", -1));
                BLOCK_END ();
            }

            if (shp1_size >= 0 && shp2_size != 0) {
                /* To ease the code generation for the next loop.  */
                out ("SAC_i = %d;", shp1_size);
            }

            if (shp2_size >= 0) {
                for (i = 0; i < shp2_size; i++) {
                    DBUG_ASSERT (shp2 != NULL, "second shape not found!");
                    if (shp2_prod_fun == NULL) {
                        indout ("SAC_size *= \n");
                    }
                    SET_SHP_AUD (to_NT, out ("SAC_i + %d", i),
                                 shp2_read_fun (shp2, NULL, i));
                }
            } else {
                DBUG_ASSERT (shp2 != NULL, "second shape not found!");
                FOR_LOOP_BEGIN ("SAC_j = 0; SAC_i < SAC_ND_A_DIM( %s); "
                                "SAC_i++, SAC_j++",
                                to_NT)
                    ;
                    if (shp2_prod_fun == NULL) {
                        indout ("SAC_size *= \n");
                    }
                    SET_SHP_AUD (to_NT, out ("SAC_i"), shp2_read_fun (shp2, "SAC_j", -1));
                FOR_LOOP_END ();
            }

            SET_SIZE (to_NT,
                      if (shp1_prod_fun == NULL || shp2_prod_fun == NULL) {
                          out ("SAC_size");
                      } else { out ("1"); }

                      if (shp1_prod_fun != NULL) {
                          out (" * ");
                          shp1_prod_fun (shp1);
                      }

                      if (shp2_prod_fun != NULL) {
                          out (" * ");
                          shp2_prod_fun (shp2);
                      });
        BLOCK_END ();

        if (global.backend == BE_distmem && to_dc == C_distr) {
            /* Array is potentially distributed. */
            indout ("SAC_ND_A_MIRROR_IS_DIST( %s) = SAC_DISTMEM_DET_DO_DISTR_ARR( "
                    "SAC_ND_A_SIZE( %s), SAC_ND_A_SHAPE( %s, 0));\n",
                    to_NT, to_NT, to_NT);
            indout ("SAC_ND_A_DESC_IS_DIST( %s) = SAC_ND_A_MIRROR_IS_DIST( %s);\n", to_NT,
                    to_NT);
        }

        break;

    case C_akd:
        DBUG_ASSERT (to_dim >= 0, "illegal dimension found!");
        BLOCK_NOVAR_BEGIN ()
            ;
            if (shp1_size < 0) {
                indout ("int SAC_i, SAC_j;\n");
            }

            if (shp1_prod_fun == NULL || shp2_prod_fun == NULL) {
                indout ("int SAC_size = 1;\n");
            }

            if (shp1_size < 0) {
                DBUG_ASSERT (shp2 != NULL, "second shape not found!");
                indout ("for (SAC_i = 0; SAC_i < ");
                shp1_size_fun (shp1);
                out ("; SAC_i++)\n");
                BLOCK_NOVAR_BEGIN ()
                    ;
                    if (shp1_prod_fun == NULL) {
                        indout ("SAC_size *= \n");
                    }

                    /* mirror is set separately */
                    SET_SHP_AUD (to_NT, out ("SAC_i"), shp1_read_fun (shp1, "SAC_i", -1));
                BLOCK_END ();

                FOR_LOOP_BEGIN ("SAC_j = 0; SAC_i < %d; "
                                "SAC_j++, SAC_i++",
                                to_dim)
                    ;
                    if (shp2_prod_fun == NULL) {
                        indout ("SAC_size *= \n");
                    }

                    /* mirror is set separately */
                    SET_SHP_AUD (to_NT, out ("SAC_i"), shp2_read_fun (shp2, "SAC_j", -1));
                FOR_LOOP_END ();

                /* refresh mirror */
                for (i = 0; i < to_dim; i++) {
                    indout ("SAC_ND_A_MIRROR_SHAPE( %s, %d)"
                            " = SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                            to_NT, i, to_NT, i);
                }
            } else {
                DBUG_ASSERT (shp1_size >= 0, "illegal dimension found!");
                for (int i = 0; i < shp1_size; i++) {
                    if (shp1_prod_fun == NULL) {
                        indout ("SAC_size *= \n");
                    }
                    SET_SHP_AKD (to_NT, i, shp1_read_fun (shp1, NULL, i));
                }

                for (int i = shp1_size; i < to_dim; i++) {
                    if (shp2_prod_fun == NULL) {
                        indout ("SAC_size *= \n");
                    }
                    DBUG_ASSERT (shp2 != NULL, "second shape not found!");
                    SET_SHP_AKD (to_NT, i, shp2_read_fun (shp2, NULL, i - shp1_size));
                }

                SET_SIZE (to_NT,
                          if (shp1_prod_fun == NULL || shp2_prod_fun == NULL) {
                              out ("SAC_size");
                          } else { out ("1"); }

                          if (shp1_prod_fun != NULL) {
                              out (" * ");
                              shp1_prod_fun (shp1);
                          }

                          if (shp2_prod_fun != NULL) {
                              out (" * ");
                              shp2_prod_fun (shp2);
                          });
            }
        BLOCK_END ();

        if (global.backend == BE_distmem && to_dc == C_distr) {
            /* Array is potentially distributed. */
            indout ("SAC_ND_A_MIRROR_IS_DIST( %s) = SAC_DISTMEM_DET_DO_DISTR_ARR( "
                    "SAC_ND_A_SIZE( %s), SAC_ND_A_SHAPE( %s, 0));\n",
                    to_NT, to_NT, to_NT);
            indout ("SAC_ND_A_DESC_IS_DIST( %s) = SAC_ND_A_MIRROR_IS_DIST( %s);\n", to_NT,
                    to_NT);
        }

        break;

    case C_aks:
    case C_scl:
        indout ("SAC_NOOP()\n");
        break;

    default:
        DBUG_UNREACHABLE ("Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void WriteScalar( char *scl)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
WriteScalar (char *scl)
{
    DBUG_ENTER ();

    if (scl[0] == '(') {
        /* 'scl' is a tagged id */
        shape_class_t dc = ICUGetShapeClass (scl);

        DBUG_ASSERT (dc == C_scl || dc == C_aud, "tagged id is no scalar!");
        out ("SAC_ND_WRITE( %s, 0)", scl);
    } else {
        /* 'scl' is a scalar constant */
        out ("%s", scl);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void ReadId( void *var_NT, char *idx_str, int idx)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ReadId (void *var_NT, char *idx_str, int idx)
{
    DBUG_ENTER ();
    DBUG_ASSERT (((char *)var_NT)[0] == '(', "no tag found!");

    if (idx_str != NULL) {
        out ("SAC_ND_READ( %s, %s)", (char *)var_NT, idx_str);
    } else {
        DBUG_ASSERT (idx >= 0, "illegal index found!");
        out ("SAC_ND_READ( %s, %d)", (char *)var_NT, idx);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void ReadScalar( void *scl, char *idx_str, int idx)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ReadScalar (void *scl, char *idx_str, int idx)
{
    DBUG_ENTER ();

    DBUG_ASSERT (scl != NULL, "scalar for ReadScalar() not found!");
    DBUG_PRINT ("scalar = " F_PTR " (\"%s\"), idx_s = %s, idx = %d",
                scl, (char *)scl, STRonNull ("", idx_str), idx);

    if (((char *)scl)[0] == '(') {
        /* 'scl' is a tagged id */
        shape_class_t sc = ICUGetShapeClass ((char *)scl);

        DBUG_ASSERT (sc == C_scl || sc == C_aud, "tagged id is no scalar!");
        ReadId (scl, idx_str, idx);
    } else {
        if (idx_str == NULL) {
            DBUG_ASSERT (idx == 0, "illegal index found!");
        }

        /* 'scl' is a scalar constant */
        out ("%s", (char *)scl);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void ReadScalar_Check( void *scl, char *idx_str, int idx)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ReadScalar_Check (void *scl, char *idx_str, int idx)
{
    DBUG_ENTER ();

    if (((char *)scl)[0] == '(') {
        /* 'scl' is a tagged id */
        shape_class_t sc = ICUGetShapeClass ((char *)scl);

        DBUG_ASSERT (sc == C_scl || sc == C_aud, "tagged id is no scalar!");
        if (sc == C_aud) {
            out ("\n");
            global.indent++;
            out ("( ");

            ASSURE_EXPR (ASSURE_COND ("SAC_ND_A_DIM( %s) == 0", (char *)scl),
                         ASSURE_TEXT ("Scalar expected but array "
                                      "with (dim > 0) found!"));
            out (" , \n");
            indout ("  ");
            ReadId (scl, idx_str, idx);
            out (" )");
            global.indent--;
        } else {
            ReadId (scl, idx_str, idx);
        }
    } else {
        /* 'scl' is a scalar constant */
        out ("%s", (char *)scl);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void ReadConstArray_Str( void *v, char *idx_str, int idx)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ReadConstArray_Str (void *v, char *idx_str, int idx)
{
    DBUG_ENTER ();

    if (idx_str != NULL) {
        DBUG_UNREACHABLE ("illegal argument for "
                          "ReadConstArray_Str() found!");
    } else {
        DBUG_ASSERT (idx >= 0, "illegal index for "
                               "ReadConstArray_Str() found!");
        DBUG_ASSERT (v != NULL, "array for "
                                "ReadConstArray_Str() not found!");
        DBUG_PRINT ("array = " F_PTR ", idx = %d", v, idx);
        ReadScalar (((char **)v)[idx], NULL, 0);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void ReadConstArray_Num( void *v, char *idx_str, int idx)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ReadConstArray_Num (void *v, char *idx_str, int idx)
{
    DBUG_ENTER ();

    DBUG_ASSERT (idx_str == NULL, "illegal argument for "
                                  "ReadConstArray_Num() found!");
    DBUG_ASSERT (idx >= 0, "illegal index for ReadConstArray_Num() found!");
    DBUG_ASSERT (v != NULL, "array for ReadConstArray_Num() not found!");
    DBUG_PRINT ("array = " F_PTR ", idx = %d", v, idx);
    out ("%d", ((int *)v)[idx]);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void DimId( void *var_NT)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
DimId (void *var_NT)
{
    DBUG_ENTER ();

    out ("SAC_ND_A_DIM( %s)", (char *)var_NT);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void ShapeId( void *var_NT, char *idx_str, int idx)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ShapeId (void *var_NT, char *idx_str, int idx)
{
    DBUG_ENTER ();

    if (idx_str != NULL) {
        out ("SAC_ND_A_SHAPE( %s, %s)", (char *)var_NT, idx_str);
    } else {
        out ("SAC_ND_A_SHAPE( %s, %d)", (char *)var_NT, idx);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void SizeId( void *var_NT)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
SizeId (void *var_NT)
{
    DBUG_ENTER ();

    out ("SAC_ND_A_SIZE( %s)", (char *)var_NT);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   char *GetAttr( void *v, int v_attr, void (*v_attr_fun)( void *))
 *
 * Description:
 *
 *
 ******************************************************************************/

void
GetAttr (void *v, int v_attr, void (*v_attr_fun) (void *))
{
    DBUG_ENTER ();

    if (v_attr < 0) {
        DBUG_ASSERT (v_attr_fun != NULL, "access function not found!");
        v_attr_fun (v);
    } else {
        out ("%d", v_attr);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void Vect2Offset2( char *off_ANY,
 *                      void *v_ANY, int v_size,
 *                      void (*v_size_fun)( void *),
 *                      void (*v_read_fun)( void *, char *, int),
 *                      void *a_ANY, int a_dim,
 *                      void (*a_dim_fun)( void *),
 *                      void (*a_shape_fun)( void *, char *, int))
 *
 * Description:
 *
 *
 ******************************************************************************/

void
Vect2Offset2 (char *off_ANY, void *v_ANY, int v_size, void (*v_size_fun) (void *),
              void (*v_read_fun) (void *, char *, int), void *a_ANY, int a_dim,
              void (*a_dim_fun) (void *), void (*a_shape_fun) (void *, char *, int))
{
    int i;

    DBUG_ENTER ();

    DBUG_ASSERT (v_read_fun != NULL, "access function not found!");
    DBUG_ASSERT (a_shape_fun != NULL, "access function not found!");
    DBUG_ASSERT ((v_size >= 0 || v_size_fun != NULL) && (a_dim >= 0 || a_dim_fun != NULL),
                 "access function not found!");

    if (v_size == 0) {
        INDENT;
        WriteScalar (off_ANY);
        out (" = 0;\n");
    } else if (v_size >= 0 && a_dim >= 0) {
        INDENT;
        WriteScalar (off_ANY);
        out (" = ");

        for (i = v_size - 1; i > 0; i--) {
            out ("( ");
            a_shape_fun (a_ANY, NULL, i);
            out (" * ");
        }

        v_read_fun (v_ANY, NULL, 0);
        for (i = 1; i < v_size; i++) {
            out (" + ");
            v_read_fun (v_ANY, NULL, i);
            out (" )");
        }

        for (i = v_size; i < a_dim; i++) {
            out (" * ");
            a_shape_fun (a_ANY, NULL, i);
        }

        out (";\n");
    } else if (a_dim < 0) {
        BLOCK_BEGIN ("int SAC_i, SAC_l;")
            ;
            indout ("SAC_l = 0;\n");
            if (v_size < 0) {
                indout ("for (SAC_i = 0; SAC_i < ");
                v_size_fun (v_ANY);
                out ("; SAC_i++)\n");
                BLOCK_NOVAR_BEGIN ()
                    ;
                    indout ("SAC_l = ");
                    a_shape_fun (a_ANY, "SAC_i", -1);
                    out (" * SAC_l + ");
                    v_read_fun (v_ANY, "SAC_i", -1);
                    out (";\n");
                BLOCK_END ();
            } else {
                indout ("SAC_l = ");
                for (i = v_size - 1; i > 0; i--) {
                    out ("( ");
                    a_shape_fun (a_ANY, NULL, i);
                    out (" * ");
                }

                v_read_fun (v_ANY, NULL, 0);
                for (i = 1; i < v_size; i++) {
                    out (" + ");
                    v_read_fun (v_ANY, NULL, i);
                    out (" )");
                }

                out (";\n");
            }

            /* Compute offset for indices (v_size <= .. < a_dim)  */
            indout ("for (SAC_i = ");
            GetAttr (v_ANY, v_size, v_size_fun);
            out ("; SAC_i < ");
            GetAttr (a_ANY, a_dim, a_dim_fun);
            out ("; SAC_i++)\n");

            BLOCK_NOVAR_BEGIN ()
                ;
                indout ("SAC_l *= ");
                a_shape_fun (a_ANY, "SAC_i", -1);
                out (";\n");
            BLOCK_END ();

            /* Write back result.  */
            INDENT;
            WriteScalar (off_ANY);
            out (" = SAC_l;\n");
        BLOCK_END ();
    } else {
        /* ((a_dim >= 0) && (v_size < 0)) */
        BLOCK_BEGIN ("int SAC_l;")
            ;
            indout ("SAC_l = 0;\n");
            for (i = 0; i < a_dim; i++) {
                indout ("SAC_l *= ");
                a_shape_fun (a_ANY, NULL, i);
                out (";\n");

                indout ("if (%d < ", i);
                v_size_fun (v_ANY);
                out (")\n");
                BLOCK_NOVAR_BEGIN ()
                    ;
                    indout ("SAC_l += ");
                    v_read_fun (v_ANY, NULL, i);
                    out (";\n");
                BLOCK_END ();
            }

            WriteScalar (off_ANY);
            out (" = SAC_l; ");
        BLOCK_END ();
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void Vect2Offset( char *off_ANY,
 *                     void *v_ANY, int v_size,
 *                     void (*v_size_fun)( void *),
 *                     void (*v_read_fun)( void *, char *, int),
 *                     void *a_NT, int a_dim)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
Vect2Offset (char *off_ANY, void *v_ANY, int v_size, void (*v_size_fun) (void *),
             void (*v_read_fun) (void *, char *, int), void *a_NT, int a_dim)
{
    DBUG_ENTER ();

    Vect2Offset2 (off_ANY, v_ANY, v_size, v_size_fun, v_read_fun, a_NT, a_dim, DimId,
                  ShapeId);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
