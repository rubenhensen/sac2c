#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "memory.h"
#include "tree_compound.h"
#include "graphtypes.h"
#include "types.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "graphtypes.h"

lubinfo *
freeLubInfo (lubinfo *linfo)
{

    DBUG_ENTER ();

    lubinfo *result = NULL;
    int i;

    if (linfo != NULL) {

        if (LUBINFO_BLOCKMIN (linfo) != NULL) {
            freeDynarray (LUBINFO_BLOCKMIN (linfo));
        }

        if (LUBINFO_INTRAMATS (linfo) != NULL) {

            for (i = 0; i < LUBINFO_NUMINTRA (linfo); i++) {

                if (LUBINFO_INTRAMATS_POS (linfo, i) != NULL) {
                    freeMatrix (LUBINFO_INTRAMATS_POS (linfo, i));
                }
            }
        }

        if (LUBINFO_INTERMAT (linfo) != NULL) {
            freeMatrix (LUBINFO_INTERMAT (linfo));
        }

        if (LUBINFO_PCPTMAT (linfo) != NULL) {
            freeMatrix (LUBINFO_PCPTMAT (linfo));
        }

        if (LUBINFO_PCPCMAT (linfo) != NULL) {
            freeMatrix (LUBINFO_PCPCMAT (linfo));
        }

        result = MEMfree (linfo);
    }

    DBUG_RETURN (result);
}

compinfo *
freeCompInfo (compinfo *ci)
{

    DBUG_ENTER ();

    compinfo *result = NULL;
    nodelist *nl;

    if (ci != NULL) {

        if (COMPINFO_CSRC (ci) != NULL) {
            freeDynarray (COMPINFO_CSRC (ci));
        }

        if (COMPINFO_CTAR (ci) != NULL) {
            freeDynarray (COMPINFO_CTAR (ci));
        }

        if (COMPINFO_TLTABLE (ci) != NULL) {
            freeDynarray (COMPINFO_TLTABLE (ci));
        }

        if (COMPINFO_PREARR (ci) != NULL) {
            // freeDynarray( COMPINFO_PREARR( ci));
        }

        if (COMPINFO_EULERTOUR (ci) != NULL) {
            freeDynarray (COMPINFO_EULERTOUR (ci));
        }

        if (COMPINFO_CROSSCLOS (ci) != NULL) {
            freeMatrix (COMPINFO_CROSSCLOS (ci));
        }

        if (COMPINFO_TLC (ci) != NULL) {
            freeMatrix (COMPINFO_TLC (ci));
        }

        if (COMPINFO_LUB (ci) != NULL) {
            freeLubInfo (COMPINFO_LUB (ci));
        }

        if (COMPINFO_DIST (ci) != NULL) {
            freeMatrix (COMPINFO_DIST (ci));
        }

        while (COMPINFO_TOPOLIST (ci) != NULL) {
            nl = COMPINFO_TOPOLIST (ci);
            COMPINFO_TOPOLIST (ci) = NODELIST_NEXT (COMPINFO_TOPOLIST (ci));
            MEMfree (nl);
        }

        result = MEMfree (ci);
    }

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
