/*
 *
 * $Id$
 *
 */

#include "icm2c_basic.h"

#define DBUG_PREFIX "COMP"
#include "debug.h"

#include "str.h"
#include "globals.h"
#include "print.h"
#include "icm2c_utils.h"

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
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ();

    DBUG_ASSERT (shp1_read_fun != NULL, "1st shape-read-fun not found!");
    if (shp2 == NULL) {
        DBUG_ASSERT (shp2_size == 0, "inconsistant 2nd shape found!");
    } else {
        DBUG_ASSERT (shp2_read_fun != NULL, "2nd shape-read-fun not found!");
    }

    /*
     * check of SAC_ND_A_DIM
     */
    if (to_sc != C_aud) {
        ASSURE_TYPE_ASS (
          fprintf (global.outfile, "SAC_ND_A_DIM( %s) == ", to_NT);
          GetAttr (shp1, shp1_size, shp1_size_fun);
          if (shp2 != NULL) {
              fprintf (global.outfile, " + ");
              GetAttr (shp2, shp2_size, shp2_size_fun);
          },
          fprintf (global.outfile, "Assignment with incompatible types found!"););
    }

    /*
     * simplify 'shp1_size', 'shp2_size'
     */
    if (to_dim >= 0) {
        if ((shp1_size >= 0) && (shp2_size >= 0)) {
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

    /*
     * check of SAC_ND_A_SHAPE
     */
    if ((to_sc == C_scl) || (to_sc == C_aks)) {
        DBUG_ASSERT (to_dim >= 0, "illegal dimension found!");
        if (shp1_size >= 0) {
            for (i = 0; i < shp1_size; i++) {
                ASSURE_TYPE_ASS (fprintf (global.outfile,
                                          "SAC_ND_A_SHAPE( %s, %d) == ", to_NT, i);
                                 shp1_read_fun (shp1, NULL, i);
                                 ,
                                 fprintf (global.outfile,
                                          "Assignment with incompatible types found!"););
            }
            for (; i < to_dim; i++) {
                DBUG_ASSERT (shp2 != NULL, "second shape not found!");
                ASSURE_TYPE_ASS (fprintf (global.outfile,
                                          "SAC_ND_A_SHAPE( %s, %d) == ", to_NT, i);
                                 shp2_read_fun (shp2, NULL, i - shp1_size);
                                 ,
                                 fprintf (global.outfile,
                                          "Assignment with incompatible types found!"););
            }
        } else {
            for (i = 0; i < to_dim; i++) {
                DBUG_ASSERT (shp2 != NULL, "second shape not found!");
                ASSURE_TYPE_ASS (fprintf (global.outfile, "((%d < ", i);
                                 shp1_size_fun (shp1); fprintf (global.outfile, ") && ");
                                 fprintf (global.outfile,
                                          "(SAC_ND_A_SHAPE( %s, %d) == ", to_NT, i);
                                 shp1_read_fun (shp1, NULL, i);
                                 fprintf (global.outfile, ")) ||");

                                 fprintf (global.outfile, "((%d >= ", i);
                                 shp1_size_fun (shp1); fprintf (global.outfile, ") && ");
                                 fprintf (global.outfile,
                                          "(SAC_ND_A_SHAPE( %s, %d) == ", to_NT, i);
                                 shp2_read_fun (shp2, NULL, i - shp1_size);
                                 ,
                                 fprintf (global.outfile,
                                          "Assignment with incompatible types found!"););
            }
        }
    }

    /*
     * check of SAC_ND_A_SIZE is missing here ...
     */
    if ((to_sc == C_scl) || (to_sc == C_aks)) {
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

    DBUG_ENTER ();

    DBUG_ASSERT (shp1_read_fun != NULL, "1st shape-read-fun not found!");
    if (shp2 == NULL) {
        DBUG_ASSERT (shp2_size == 0, "inconsistant 2nd shape found!");
    } else {
        DBUG_ASSERT (shp2_read_fun != NULL, "2nd shape-read-fun not found!");
    }

    if (to_dim >= 0) {
        if ((shp1_size >= 0) && (shp2_size >= 0)) {
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

    /*
     * check constant part of mirror
     */
    Check_Mirror (to_NT, to_sdim, shp1, shp1_size, shp1_size_fun, shp1_read_fun, shp2,
                  shp2_size, shp2_size_fun, shp2_read_fun);

    /*
     * set descriptor entries and non-constant part of mirror
     */
    switch (to_sc) {
    case C_aud:
        /*
         * ND_A_DESC_DIM, ND_A_MIRROR_DIM have already been set by ND_ALLOC__DESC!
         */
        ASSURE_TYPE_ASS (
          fprintf (global.outfile, "SAC_ND_A_DIM( %s) == ", to_NT);
          GetAttr (shp1, shp1_size, shp1_size_fun);
          if (shp2 != NULL) {
              fprintf (global.outfile, " + ");
              GetAttr (shp2, shp2_size, shp2_size_fun);
          },
          fprintf (global.outfile, "Assignment with incompatible types found!"););
        BLOCK_VARDECSS (fprintf (global.outfile, "int SAC_i");
                        if (shp2_size < 0) { fprintf (global.outfile, ", SAC_j"); } if (
                          (shp1_prod_fun == NULL) || (shp2_prod_fun == NULL)) {
                            fprintf (global.outfile, ", SAC_size = 1");
                        } fprintf (global.outfile, ";");
                        ,
                        /*
                         * although 'to_NT' is AUD, 'to_dim' may indeed be >=0 if the
                         * sac2c flag -minarrayrep has been used, i.e. 'to_NT' may be
                         * implemented as AUD although it is AKD!!!
                         */
                        SET_SHAPES_AUD__XXX (to_NT, i, fprintf (global.outfile, "SAC_i");
                                             , 0, fprintf (global.outfile, "0");
                                             , shp1_size, shp1_size_fun (shp1);
                                             ,
                                             if (shp1_prod_fun == NULL) {
                                                 INDENT;
                                                 fprintf (global.outfile,
                                                          "SAC_size *= \n");
                                             },
                                             shp1_read_fun (shp1, NULL, i);
                                             , shp1_read_fun (shp1, "SAC_i", -1););
                        if ((shp1_size >= 0) && (shp2_size != 0)) {
                            /* to ease the code generation for the next loop */
                            fprintf (global.outfile, "SAC_i = %d;", shp1_size);
                        }

                        if (shp2_size >= 0) {
                            for (i = 0; i < shp2_size; i++) {
                                DBUG_ASSERT (shp2 != NULL, "second shape not found!");
                                if (shp2_prod_fun == NULL) {
                                    INDENT;
                                    fprintf (global.outfile, "SAC_size *= \n");
                                }
                                SET_SHAPE_AUD (to_NT,
                                               fprintf (global.outfile, "SAC_i + %d", i);
                                               , shp2_read_fun (shp2, NULL, i););
                            }
                        } else {
                            DBUG_ASSERT (shp2 != NULL, "second shape not found!");
                            FOR_LOOP (fprintf (global.outfile, "SAC_j = 0");
                                      , fprintf (global.outfile,
                                                 "SAC_i < SAC_ND_A_DIM( %s)", to_NT);
                                      , fprintf (global.outfile, "SAC_i++, SAC_j++");
                                      ,
                                      if (shp2_prod_fun == NULL) {
                                          INDENT;
                                          fprintf (global.outfile, "SAC_size *= \n");
                                      } SET_SHAPE_AUD (to_NT,
                                                       fprintf (global.outfile, "SAC_i");
                                                       , shp2_read_fun (shp2, "SAC_j",
                                                                        -1);););
                        }

                        SET_SIZE (to_NT,
                                  if ((shp1_prod_fun == NULL)
                                      || (shp2_prod_fun == NULL)) {
                                      fprintf (global.outfile, "SAC_size");
                                  } else {
                                      fprintf (global.outfile, "1");
                                  } if (shp1_prod_fun != NULL) {
                                      fprintf (global.outfile, " * ");
                                      shp1_prod_fun (shp1);
                                  } if (shp2_prod_fun != NULL) {
                                      fprintf (global.outfile, " * ");
                                      shp2_prod_fun (shp2);
                                  }););
        break;

    case C_akd:
        DBUG_ASSERT (to_dim >= 0, "illegal dimension found!");
        BLOCK_VARDECSS (
          if (shp1_size < 0) {
              fprintf (global.outfile, "int SAC_i, SAC_j; ");
          } if ((shp1_prod_fun == NULL) || (shp2_prod_fun == NULL)) {
              fprintf (global.outfile, "int SAC_size = 1;");
          },
          if (shp1_size < 0) {
              DBUG_ASSERT (shp2 != NULL, "second shape not found!");
              FOR_LOOP_INC (fprintf (global.outfile, "SAC_i");
                            , fprintf (global.outfile, "0");, shp1_size_fun (shp1);
                            ,
                            if (shp1_prod_fun == NULL) {
                                INDENT;
                                fprintf (global.outfile, "SAC_size *= \n");
                            } SET_SHAPE_AUD (to_NT, /* mirror is set separately */
                                             fprintf (global.outfile, "SAC_i");
                                             , shp1_read_fun (shp1, "SAC_i", -1);););
              FOR_LOOP (fprintf (global.outfile, "SAC_j = 0");
                        , fprintf (global.outfile, "SAC_i < %d", to_dim);
                        , fprintf (global.outfile, "SAC_i++, SAC_j++");
                        ,
                        if (shp2_prod_fun == NULL) {
                            INDENT;
                            fprintf (global.outfile, "SAC_size *= \n");
                        } SET_SHAPE_AUD (to_NT, /* mirror is set separately */
                                         fprintf (global.outfile, "SAC_i");
                                         , shp2_read_fun (shp2, "SAC_j", -1);););
              /* refresh mirror */
              for (i = 0; i < to_dim; i++) {
                  INDENT;
                  fprintf (global.outfile,
                           "SAC_ND_A_MIRROR_SHAPE( %s, %d)"
                           " = SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                           to_NT, i, to_NT, i);
              }
          } else {
              SET_SHAPES_AKD (to_NT, i, 0, shp1_size,
                              if (shp1_prod_fun == NULL) {
                                  INDENT;
                                  fprintf (global.outfile, "SAC_size *= \n");
                              },
                              shp1_read_fun (shp1, NULL, i););
              SET_SHAPES_AKD (to_NT, i, shp1_size, to_dim,
                              if (shp2_prod_fun == NULL) {
                                  INDENT;
                                  fprintf (global.outfile, "SAC_size *= \n");
                              },
                              DBUG_ASSERT (shp2 != NULL, "second shape not found!");
                              shp2_read_fun (shp2, NULL, i - shp1_size););
          }

          SET_SIZE (to_NT,
                    if ((shp1_prod_fun == NULL) || (shp2_prod_fun == NULL)) {
                        fprintf (global.outfile, "SAC_size");
                    } else { fprintf (global.outfile, "1"); } if (shp1_prod_fun != NULL) {
                        fprintf (global.outfile, " * ");
                        shp1_prod_fun (shp1);
                    } if (shp2_prod_fun != NULL) {
                        fprintf (global.outfile, " * ");
                        shp2_prod_fun (shp2);
                    }););
        break;

    case C_aks:
        /* here is no break missing */
    case C_scl:
        INDENT;
        fprintf (global.outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT (0, "Unknown shape class found!");
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
#ifndef DBUG_OFF
        shape_class_t dc = ICUGetShapeClass (scl);
#endif

        DBUG_ASSERT (((dc == C_scl) || (dc == C_aud)), "tagged id is no scalar!");
        fprintf (global.outfile, "SAC_ND_WRITE( %s, 0)", scl);
    } else {
        /* 'scl' is a scalar constant */
        fprintf (global.outfile, "%s", scl);
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
        fprintf (global.outfile, "SAC_ND_READ( %s, %s)", (char *)var_NT, idx_str);
    } else {
        DBUG_ASSERT (idx >= 0, "illegal index found!");
        fprintf (global.outfile, "SAC_ND_READ( %s, %d)", (char *)var_NT, idx);
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
    DBUG_PRINT ("scalar = " F_PTR ", idx_s = %s, idx = %d", scl, STRonNull ("", idx_str),
                idx);
    DBUG_PRINT ("scalar = \"%s\", idx_s = %s, idx = %d", scl, STRonNull ("", idx_str),
                idx);

    if (((char *)scl)[0] == '(') {
        /* 'scl' is a tagged id */
#ifndef DBUG_OFF
        shape_class_t sc = ICUGetShapeClass (scl);
#endif

        DBUG_ASSERT (((sc == C_scl) || (sc == C_aud)), "tagged id is no scalar!");
        ReadId (scl, idx_str, idx);
    } else {
        if (idx_str == NULL) {
            DBUG_ASSERT (idx == 0, "illegal index found!");
        }

        /* 'scl' is a scalar constant */
        fprintf (global.outfile, "%s", (char *)scl);
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
        shape_class_t sc = ICUGetShapeClass (scl);

        DBUG_ASSERT (((sc == C_scl) || (sc == C_aud)), "tagged id is no scalar!");
        if (sc == C_aud) {
            fprintf (global.outfile, "\n");
            global.indent++;
            fprintf (global.outfile, "( ");
            ASSURE_TYPE_EXPR (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0",
                                       (char *)scl);
                              , fprintf (global.outfile, "Scalar expected but array with "
                                                         "(dim > 0) found!"););
            fprintf (global.outfile, " , \n");
            INDENT;
            fprintf (global.outfile, "  ");
            ReadId (scl, idx_str, idx);
            fprintf (global.outfile, " )");
            global.indent--;
        } else {
            ReadId (scl, idx_str, idx);
        }
    } else {
        /* 'scl' is a scalar constant */
        fprintf (global.outfile, "%s", (char *)scl);
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
        DBUG_ASSERT (0, "illegal argument for ReadConstArray_Str() found!");
    } else {
        DBUG_ASSERT (idx >= 0, "illegal index for ReadConstArray_Str() found!");
        DBUG_ASSERT (v != NULL, "array for ReadConstArray_Str() not found!");
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

    DBUG_ASSERT (idx_str == NULL, "illegal argument for ReadConstArray_Num() found!");
    DBUG_ASSERT (idx >= 0, "illegal index for ReadConstArray_Num() found!");
    DBUG_ASSERT (v != NULL, "array for ReadConstArray_Num() not found!");
    DBUG_PRINT ("array = " F_PTR ", idx = %d", v, idx);
    fprintf (global.outfile, "%d", ((int *)v)[idx]);

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

    fprintf (global.outfile, "SAC_ND_A_DIM( %s)", (char *)var_NT);

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
        fprintf (global.outfile, "SAC_ND_A_SHAPE( %s, %s)", (char *)var_NT, idx_str);
    } else {
        fprintf (global.outfile, "SAC_ND_A_SHAPE( %s, %d)", (char *)var_NT, idx);
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

    fprintf (global.outfile, "SAC_ND_A_SIZE( %s)", (char *)var_NT);

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
        fprintf (global.outfile, "%d", v_attr);
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

    DBUG_ASSERT ((((v_size >= 0) || (v_size_fun != NULL))
                  && ((a_dim >= 0) || (a_dim_fun != NULL))),
                 "access function not found!");

    if (v_size == 0) {
        INDENT;
        WriteScalar (off_ANY);
        fprintf (global.outfile, " = 0;\n");
    } else if ((v_size >= 0) && (a_dim >= 0)) {
        INDENT;
        WriteScalar (off_ANY);
        fprintf (global.outfile, " = ");
        for (i = v_size - 1; i > 0; i--) {
            fprintf (global.outfile, "( ");
            a_shape_fun (a_ANY, NULL, i);
            fprintf (global.outfile, " * ");
        }
        v_read_fun (v_ANY, NULL, 0);
        for (i = 1; i < v_size; i++) {
            fprintf (global.outfile, " + ");
            v_read_fun (v_ANY, NULL, i);
            fprintf (global.outfile, " )");
        }
        for (i = v_size; i < a_dim; i++) {
            fprintf (global.outfile, " * ");
            a_shape_fun (a_ANY, NULL, i);
        }
        fprintf (global.outfile, ";\n");
    } else if (a_dim < 0) {
        BLOCK_VARDECSS (
          fprintf (global.outfile, "int SAC_i, SAC_l;");,
                                                        /*
                                                         * init offset
                                                         */
                                                        INDENT;
          fprintf (global.outfile, "SAC_l = 0;\n");

          /*
           * compute offset for indices (0 <= .. < v_size)
           */
          if (v_size < 0) {
              FOR_LOOP (fprintf (global.outfile, "SAC_i = 0");
                        , fprintf (global.outfile, "SAC_i < "); v_size_fun (v_ANY);
                        , fprintf (global.outfile, "SAC_i++");, INDENT;
                        fprintf (global.outfile, "SAC_l = ");
                        a_shape_fun (a_ANY, "SAC_i", -1);
                        fprintf (global.outfile, " * SAC_l + ");
                        v_read_fun (v_ANY, "SAC_i", -1);
                        fprintf (global.outfile, ";\n"););
          } else {
              INDENT;
              fprintf (global.outfile, "SAC_l = ");
              for (i = v_size - 1; i > 0; i--) {
                  fprintf (global.outfile, "( ");
                  a_shape_fun (a_ANY, NULL, i);
                  fprintf (global.outfile, " * ");
              }
              v_read_fun (v_ANY, NULL, 0);
              for (i = 1; i < v_size; i++) {
                  fprintf (global.outfile, " + ");
                  v_read_fun (v_ANY, NULL, i);
                  fprintf (global.outfile, " )");
              }
              fprintf (global.outfile, ";\n");
          }

          /*
           * compute offset for indices (v_size <= .. < a_dim)
           */
          FOR_LOOP (fprintf (global.outfile, "SAC_i = ");
                    GetAttr (v_ANY, v_size, v_size_fun);
                    , fprintf (global.outfile, "SAC_i < ");
                    GetAttr (a_ANY, a_dim, a_dim_fun);
                    , fprintf (global.outfile, "SAC_i++");, INDENT;
                    fprintf (global.outfile, "SAC_l *= ");
                    a_shape_fun (a_ANY, "SAC_i", -1); fprintf (global.outfile, ";\n"););
          /*
           * write back result
           */
          INDENT; WriteScalar (off_ANY); fprintf (global.outfile, " = SAC_l;\n");

        );
    } else { /* ((a_dim >= 0) && (v_size < 0)) */
        BLOCK_VARDECSS (
          fprintf (global.outfile, "int SAC_l;\n");, INDENT;
          fprintf (global.outfile, "SAC_l = 0;\n"); for (i = 0; i < a_dim; i++) {
              INDENT;
              fprintf (global.outfile, "SAC_l *= ");
              a_shape_fun (a_ANY, NULL, i);
              fprintf (global.outfile, ";\n");
              COND1 (fprintf (global.outfile, "%d < ", i); v_size_fun (v_ANY);
                     , fprintf (global.outfile, "SAC_l += "); v_read_fun (v_ANY, NULL, i);
                     fprintf (global.outfile, ";\n"););
          } WriteScalar (off_ANY);
          fprintf (global.outfile, " = SAC_l; "););
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
