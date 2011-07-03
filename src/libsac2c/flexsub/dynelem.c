#include "ctinfo.h"
#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "tree_compound.h"
#include "globals.h"
#include "dynelem.h"

void
initElem (elem *e)
{

    ELEM_IDX (e) = 0;
    ELEM_DATA (e) = NULL;
}

void
freeElem (elem *e)
{

    if (e != NULL) {

        if (ELEM_DATA (e) != NULL) {

            MEMfree (ELEM_DATA (e));
            ELEM_DATA (e) = NULL;
        }

        MEMfree (e);
        e = NULL;
    }
}
