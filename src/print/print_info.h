#ifndef _SAC_PRINT_INFO_H
#define _SAC_PRINT_INFO_H

/*
 * the INFO structure is shared between all phases that do use functions
 * from print, that are
 * - print itself
 * = writesib
 */

/* INFO structure */
struct INFO {
    /* print */
    node *cont;
    node *fundef;
    node *npart;
    node *nwith2;
    int sib;
    int ofp;
    int varno;
    int prototype;
    int separate;
    int dim;
    shpseg *shape;
    shpseg *shapecnt;
    /* writesib */
    nodelist *etypes;
    nodelist *efuns;
    nodelist *eobjs;
};

/* access macros print */
#define INFO_PRINT_CONT(n) (n->cont)
#define INFO_PRINT_FUNDEF(n) (n->fundef)
#define INFO_PRINT_NPART(n) (n->npart)
#define INFO_PRINT_NWITH2(n) (n->nwith2)
#define INFO_PRINT_SIB(n) (n->sib)
#define INFO_PRINT_OMIT_FORMAL_PARAMS(n) (n->ofp)
#define INFO_PRINT_VARNO(n) (n->varno)
#define INFO_PRINT_PROTOTYPE(n) (n->prototype)
#define INFO_PRINT_SEPARATE(n) (n->separate)
#define INFO_PRINT_DIM(n) (n->dim)
#define INFO_PRINT_SHAPE(n) (n->shape)
#define INFO_PRINT_SHAPE_COUNTER(n) (n->shapecnt)

/* access macros writesib */
#define INFO_WSIB_EXPORTTYPES(n) (n->etypes)
#define INFO_WSIB_EXPORTOBJS(n) (n->eobjs)
#define INFO_WSIB_EXPORTFUNS(n) (n->efuns)

#endif /* _SAC_PRINT_INFO_H */
