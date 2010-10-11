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

compinfo **
freeCompInfoArr (compinfo **cia, int n)
{

    DBUG_ENTER ("freeCompInfoArr");

    int i, j;

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

                if (COMPINFO_EULERTOUR (cia[i]) != NULL) {
                    freeDynarray (COMPINFO_EULERTOUR (cia[i]));
                }

                if (COMPINFO_CROSSCLOS (cia[i]) != NULL) {
                    freeMatrix (COMPINFO_CROSSCLOS (cia[i]));
                }

                if (COMPINFO_TLC (cia[i]) != NULL) {
                    freeMatrix (COMPINFO_TLC (cia[i]));
                }

                /*LUBMat consists of 3 matrices*/
                for (j = 0; j < 3; j++) {
                    if (COMPINFO_LUBPOS (cia[i], j) != NULL) {
                        freeMatrix (COMPINFO_LUBPOS (cia[i], j));
                    }
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

    cia = MEMfree (cia);

    DBUG_RETURN (cia);
}
