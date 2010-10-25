#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "dbug.h"
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

    DBUG_ENTER ("freeLubInfo");

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

compinfo **
freeCompInfoArr (compinfo **cia, int n)
{

    DBUG_ENTER ("freeCompInfoArr");

    int i;
    compinfo **result;
    nodelist *nl;

    if (cia != NULL) {

        for (i = 0; i < n; i++) {

            if (cia[i] != NULL) {

                if (COMPINFO_CSRC (cia[i]) != NULL) {
                    freeDynarray (COMPINFO_CSRC (cia[i]));
                }

                if (COMPINFO_CTAR (cia[i]) != NULL) {
                    freeDynarray (COMPINFO_CTAR (cia[i]));
                }

                if (COMPINFO_TLTABLE (cia[i]) != NULL) {
                    freeDynarray (COMPINFO_TLTABLE (cia[i]));
                }

                if (COMPINFO_PREARR (cia[i]) != NULL) {
                    // freeDynarray( COMPINFO_PREARR( cia[i]));
                }

                if (COMPINFO_EULERTOUR (cia[i]) != NULL) {
                    freeDynarray (COMPINFO_EULERTOUR (cia[i]));
                }

                if (COMPINFO_CROSSCLOS (cia[i]) != NULL) {
                    freeMatrix (COMPINFO_CROSSCLOS (cia[i]));
                }

                if (COMPINFO_TLC (cia[i]) != NULL) {
                    freeMatrix (COMPINFO_TLC (cia[i]));
                }

                if (COMPINFO_LUB (cia[i]) != NULL) {
                    freeLubInfo (COMPINFO_LUB (cia[i]));
                }

                if (COMPINFO_DIST (cia[i]) != NULL) {
                    freeMatrix (COMPINFO_DIST (cia[i]));
                }

                while (COMPINFO_TOPOLIST (cia[i]) != NULL) {
                    nl = COMPINFO_TOPOLIST (cia[i]);
                    COMPINFO_TOPOLIST (cia[i])
                      = NODELIST_NEXT (COMPINFO_TOPOLIST (cia[i]));
                    MEMfree (nl);
                }

                cia[i] = MEMfree (cia[i]);
            }
        }
    }

    result = MEMfree (cia);

    DBUG_RETURN (result);
}
