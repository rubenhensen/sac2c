/*    $Id$
 *
 * $Log$
 * Revision 1.9  1998/03/22 18:15:53  srs
 * moved typedefs and macros from WithloopFolding.c,
 * moved some export declarations to WLT.h, WLI.h, WLF.h
 *
 * Revision 1.8  1998/03/18 08:33:15  srs
 * first running version of WLI
 *
 * Revision 1.7  1998/03/06 13:29:05  srs
 * added new WLI functions
 *
 * Revision 1.6  1998/02/24 14:19:26  srs
 * *** empty log message ***
 *
 * Revision 1.5  1998/02/09 15:58:47  srs
 * *** empty log message ***
 *
 * Revision 1.4  1998/02/06 14:33:29  srs
 * RCS-Test
 *
 * Revision 1.3  1998/02/06 14:32:49  srs
 * *** empty log message ***
 *
 */

#ifndef _WithloopFolding_h
#define _WithloopFolding_h

/******************************************************************************
 *
 * types
 *
 ******************************************************************************/

/* The following struct may only be annotated to N_assign nodes which are
   inside a WL body and which have ASSIGN_INSTRs N_let and N_prf(F_psi). */
typedef struct INDEX_INFO {
    int vector;               /* this is an index vector (>0) or a scalar (0)
                                 in case of a vector this number is the
                                 shape of the vector. */
    int *permutation;         /* Describes the permutation of index vars in
                                 this vector (if this is a vector) or stores
                                 the base scalar index in permutation[0].
                                 The index scarales are counted starting
                                 with 1. E.g. in [i,j,k] j has the no 2.
                                 If one elements is not based on an index
                                 (ONLY a constant, else the vector is not
                                 valid) this value is 0 and the constant
                                 can be found in const_arg. */
    struct INDEX_INFO **last; /* points to last transformations */

    /* the next 3 components describe the executed transformation */
    prf prf;        /* prf +,-,*,/ or */
    int *const_arg; /* the constant arg has to be an integer.
                       For every element of a vector there is an
                       own constant arg.
                       If this struct is an annotation for a scalar,
                       only const_arg[0] is valid.
                       If the corresponding permutation is 0, the
                       vector's element is a constant which is
                       stored here (no prf arg). */
    int arg_no;     /* const_arg is the first (1) or second (2)
                       argument of prf. arg_no may be 0 which
                       means that no prf given. Can only be in:
                        tmp = [i,j,c];
                        val = psi(...,tmp);
                       Well, can also happen when CF is deacivated.
                       If arg_no is 0, prf is undefined */
} index_info;

/******************************************************************************
 *
 * exported functions
 *
 ******************************************************************************/

extern node *WithloopFolding (node *, node *);

extern void DbugIndexInfo (index_info *iinfo);
extern index_info *CreateIndex (int vector);
extern index_info *DuplicateIndexInfo (index_info *iinfo);
extern index_info *ValidLocalId (node *idn);

extern int LocateIndexVar (node *idn, node *wln);

/******************************************************************************
 *
 * defines
 *
 ******************************************************************************/

#define INDEX(n) ((index_info *)ASSIGN_INDEX (n))
#define DEF_MASK 0
#define USE_MASK 1

#define FREE_INDEX(tmp)                                                                  \
    {                                                                                    \
        FREE (tmp->permutation);                                                         \
        FREE (tmp->last);                                                                \
        FREE (tmp->const_arg);                                                           \
        FREE (tmp);                                                                      \
        tmp = NULL;                                                                      \
    }

#endif
