/*    $Id$
 *
 * $Log$
 * Revision 2.3  1999/02/28 21:45:21  srs
 * moved two types to types.h and FREE_INDEX to free.h
 *
 * Revision 2.2  1999/02/26 14:49:21  dkr
 * file moved from folder /optimize
 *
 * Revision 2.1  1999/02/23 12:41:43  sacbase
 * new release made
 *
 * Revision 1.16  1998/05/15 14:41:21  srs
 * changed MakeNullVec()
 *
 * Revision 1.15  1998/05/12 09:46:46  srs
 * added TRANSF_TRUE_PERMUTATIONS
 *
 * Revision 1.14  1998/04/24 17:32:31  srs
 * changed comments and added export of MakeNullVec()
 *
 * Revision 1.13  1998/04/20 09:01:24  srs
 * new function WithloopFoldingWLT()
 *
 * Revision 1.12  1998/04/08 20:44:29  srs
 * exported new functions
 *
 * Revision 1.11  1998/04/03 11:39:40  srs
 * changed arguments for SearchWL
 *
 * Revision 1.10  1998/04/01 07:45:57  srs
 * added new struct definitions, macros and export declarations
 *
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
 * exported functions
 *
 ******************************************************************************/

/* general functions */
extern node *WithloopFolding (node *, node *);
extern node *WithloopFoldingWLT (node *, node *);
extern int LocateIndexVar (node *idn, node *wln);
extern node *CreateVardec (char *name, types *type, node **vardecs);
extern node *StartSearchWL (node *idn, node *assignn, int mode);
extern void ArrayST2ArrayInt (node *arrayn, int **iarray, int shape);
extern node *MakeNullVec (int dim, simpletype type);

/* index_info related functions */
extern void DbugIndexInfo (index_info *iinfo);
extern index_info *CreateIndex (int vector);
extern index_info *DuplicateIndexInfo (index_info *iinfo);
extern index_info *ValidLocalId (node *idn);

/* intern_gen related functions */
extern void DbugInternGen (intern_gen *ig);
extern intern_gen *Tree2InternGen (node *wln, node *filter);
extern node *InternGen2Tree (node *wln, intern_gen *ig);
extern int NormalizeInternGen (intern_gen *ig);
extern intern_gen *CreateInternGen (int shape, int stepwidth);
extern intern_gen *AppendInternGen (intern_gen *, int, node *, int);
extern intern_gen *CopyInternGen (intern_gen *source);
extern intern_gen *MoveInternGen (intern_gen *source, intern_gen **dest);
extern intern_gen *FreeInternGenChain (intern_gen *ig);

/******************************************************************************
 *
 * defines
 *
 ******************************************************************************/

/* if not defined, indexes with more than one occurence of an
   index scalar are allowed to be valid transformations, e.g. [i,i,j] */
/* #define TRANSF_TRUE_PERMUTATIONS */

/* general macros */
#define DEF_MASK 0
#define USE_MASK 1

/* index_info related macros. See FREE_INDEX_INFO in free.h */
#define INDEX(n) ((index_info *)ASSIGN_INDEX (n))

/* intern_gen related macros*/
#define FREE_INTERN_GEN(tmp)                                                             \
    {                                                                                    \
        FREE (tmp->l);                                                                   \
        FREE (tmp->u);                                                                   \
        FREE (tmp->step);                                                                \
        FREE (tmp->width);                                                               \
        FREE (tmp);                                                                      \
        tmp = NULL;                                                                      \
    }

#endif
