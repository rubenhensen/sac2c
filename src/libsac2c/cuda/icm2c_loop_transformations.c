#include "memory.h"

// TODO: change this for the real variable representation(s)
typedef int Var_repr;

/**
 * Index space representation
 *
 */
struct ISPACE {
    int dim;
    int alloc;
    struct Var_repr* lowerbound;
    struct Var_repr* upperbound;
    struct Var_repr* step;
    struct Var_repr* width;
};

typedef struct ISPACE ispace;

#define ISPACE_DIM(ispace) ispace->dim
#define ISPACE_ALLOC(ispace) ispace->alloc

#define ISPACE_LB(ispace) ispace->lowerbound
#define ISPACE_UB(ispace) ispace->upperbound
#define ISPACE_STEP(ispace) ispace->step
#define ISPACE_WIDTH(ispace) ispace->width

#define ISPACE_LB_DIM(ispace, dim) ISPACE_LB(ispace)[dim]
#define ISPACE_UB_DIM(ispace, dim) ISPACE_UB(ispace)[dim]
#define ISPACE_STEP_DIM(ispace, dim) ISPACE_STEP(ispace)[dim]
#define ISPACE_WIDTH_DIM(ispace, dim) ISPACE_WIDTH(ispace)[dim]

static ispace*
MakeIspace(int dim) {
    ispace* space_repr = (ispace*) MEMmalloc(sizeof(ispace));

    ISPACE_DIM(space_repr)   = dim;
    ISPACE_ALLOC(space_repr) = dim;
    ISPACE_LB(space_repr)    = (Var_repr *)MEMmalloc(ISPACE_ALLOC(space_repr) * sizeof(Var_repr));
    ISPACE_UB(space_repr)    = (Var_repr *)MEMmalloc(ISPACE_ALLOC(space_repr) * sizeof(Var_repr));
    ISPACE_STEP(space_repr)  = (Var_repr *)MEMmalloc(ISPACE_ALLOC(space_repr) * sizeof(Var_repr));
    ISPACE_WIDTH(space_repr) = (Var_repr *)MEMmalloc(ISPACE_ALLOC(space_repr) * sizeof(Var_repr));
 }

static void
FreeIspace(ispace* ispace) {
    MEMfree(ISPACE_LB(ispace));
    MEMfree(ISPACE_UB(ispace));
    MEMfree(ISPACE_STEP(ispace));
    MEMfree(ISPACE_WIDTH(ispace));
    MEMfree(ispace);
}

bool redim(ispace* space_repr, int dim) {
    if (dim > ISPACE_ALLOC(space_repr)) {
        // Realloc to twice the needed size to prevent constant reallocation.
        // Probably not necessary, but it can save time in some cases.
        ISPACE_ALLOC(space_repr) = 2 * dim;
        MEMrealloc(ISPACE_LB(space_repr), ISPACE_ALLOC(space_repr) * sizeof(Var_repr));
        MEMrealloc(ISPACE_UB(space_repr), ISPACE_ALLOC(space_repr) * sizeof(Var_repr));
        MEMrealloc(ISPACE_STEP(space_repr), ISPACE_ALLOC(space_repr) * sizeof(Var_repr));
        MEMrealloc(ISPACE_WIDTH(space_repr), ISPACE_ALLOC(space_repr) * sizeof(Var_repr));
    }
    ISPACE_DIM(space_repr) = dim;
}

