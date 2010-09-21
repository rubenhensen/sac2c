/*
 * $Id$
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "stringset.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "new_types.h"
#include "shape.h"
#include "constants.h"
#include "scheduling.h"
#include "namespaces.h"
#include "dbug.h"
#include "globals.h"
#include "structures.h"

/** <!--******************************************************************-->
 *
 * @fn FREEattribString
 *
 * @brief Frees String attribute
 *
 * @param attr String node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
char *
FREEattribString (char *attr, node *parent)
{
    DBUG_ENTER ("FREEattribString");

    if (attr != NULL) {
        DBUG_PRINT ("FREE", ("Freeing string '%s' at " F_PTR, attr, attr));
        attr = MEMfree (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribSharedString
 *
 * @brief Frees String attribute
 *
 * @param attr String node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
char *
FREEattribSharedString (char *attr, node *parent)
{
    DBUG_ENTER ("FREEattribSharedString");

    /* do nothing */

    DBUG_RETURN ((char *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribOldType
 *
 * @brief Frees OldType attribute
 *
 * @param attr OldType node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
types *
FREEattribOldType (types *attr, node *parent)
{
    DBUG_ENTER ("FREEattribOldType");

    if (attr != NULL) {
        attr = FREEfreeOneTypes (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribNode
 *
 * @brief Frees Node attribute
 *
 * @param attr Node node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FREEattribNode (node *attr, node *parent)
{
    DBUG_ENTER ("FREEattribNode");

    if (attr != NULL) {
        DBUG_PRINT ("FREE", ("Starting to free %s node attribute at " F_PTR,
                             NODE_TEXT (attr), attr));
        attr = FREEdoFreeTree (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribNamespace
 *
 * @brief Frees Namespace attribute
 *
 * @param attr Namespace node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
namespace_t *
FREEattribNamespace (namespace_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribNamespace");

    if (attr != NULL) {
        attr = NSfreeNamespace (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribLink
 *
 * @brief Frees Link attribute
 *
 * @param attr Link node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FREEattribLink (node *attr, node *parent)
{
    DBUG_ENTER ("FREEattribLink");

    /*
     * NEVER do anything with this kind of attribute
     * as you cannot make sure the node you reference
     * here really exists!
     */

    DBUG_RETURN ((node *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribExtLink
 *
 * @brief Frees Link attribute
 *
 * @param attr Link node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FREEattribExtLink (node *attr, node *parent)
{
    DBUG_ENTER ("FREEattribExtLink");

    if (attr != NULL) {
        if ((NODE_TYPE (attr) == N_fundef) && (NODE_TYPE (parent) == N_ap)) {
            if ((FUNDEF_ISCONDFUN (attr) || (FUNDEF_ISDOFUN (attr))
                 || (FUNDEF_ISLACINLINE (attr)))
                && !AP_ISRECURSIVEDOFUNCALL (parent)) {
                /**
                 * treat all lac funs as if they were inlined.
                 * Since FUNDEF_ISLACINLINE funs are degenerated
                 * lac funs these need to be traeted in the same
                 * way.
                 * However, we need to make sure that the external
                 * call is treated in that way ONLY!
                 * Hence, we demand that !AP_ISRECURSIVEDOFUNCALL( parent) !
                 */
                attr = FREEdoFreeNode (attr);
            }
        }
    }

    DBUG_RETURN ((node *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribCodeLink
 *
 * @brief Frees Link attribute
 *
 * @param attr Link node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/

node *
FREEattribCodeLink (node *attr, node *parent)
{
    DBUG_ENTER ("FREEattribCodeLink");

    if (attr != NULL) {
        CODE_DEC_USED (attr);
    }

    DBUG_RETURN ((node *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribLUT
 *
 * @brief Frees LUT attribute
 *
 * @param attr LUT node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
lut_t *
FREEattribLUT (lut_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribLUT");

    if (attr != NULL) {
        attr = LUTremoveLut (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribNodeList
 *
 * @brief Frees NodeList attribute
 *
 * @param attr NodeList node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
nodelist *
FREEattribNodeList (nodelist *attr, node *parent)
{
    DBUG_ENTER ("FREEattribNodeList");

    attr = FREEfreeNodelist (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribSharedNodeList
 *
 * @brief Frees SharedNodeList attribute
 *
 * @param attr SharedNodeList node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
nodelist *
FREEattribSharedNodeList (nodelist *attr, node *parent)
{
    DBUG_ENTER ("FREEattribSharedNodeList");

    /* do nothing here */

    DBUG_RETURN ((nodelist *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribDFMask
 *
 * @brief Frees DFMask attribute
 *
 * @param attr DFMask node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
dfmask_t *
FREEattribDFMask (dfmask_t *attr)
{
    DBUG_ENTER ("FREEattribDFMask");

    if (attr != NULL) {
        attr = DFMremoveMask (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribDFMaskBase
 *
 * @brief Frees DFMaskBase attribute
 *
 * @param attr DFMaskBase node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
dfmask_base_t *
FREEattribDFMaskBase (dfmask_base_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribDFMaskBase");

    if (attr != NULL) {
        attr = DFMremoveMaskBase (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribNewType
 *
 * @brief Frees NewType attribute
 *
 * @param attr NewType node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
ntype *
FREEattribNewType (ntype *attr, node *parent)
{
    DBUG_ENTER ("FREEattribNewType");

    if (attr != NULL) {
        DBUG_PRINT ("FREE", ("Freeing ntype at " F_PTR, attr));
        attr = TYfreeType (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribArgTab
 *
 * @brief Frees ArgTab attribute
 *
 * @param attr ArgTab node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
argtab_t *
FREEattribArgTab (argtab_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribArgTab");

    if (attr != NULL) {
        attr->ptr_in = MEMfree (attr->ptr_in);
        attr->ptr_out = MEMfree (attr->ptr_out);
        attr->tag = MEMfree (attr->tag);
        attr->size = 0;

        attr = MEMfree (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribIndexPointer
 *
 * @brief Frees IndexPointer attribute
 *
 * @param attr IndexPointer node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
index_info *
FREEattribIndexPointer (index_info *attr, node *parent)
{
    DBUG_ENTER ("FREEattribIndexPointer");

    if (attr != NULL) {
        attr = FREEfreeIndexInfo (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribShape
 *
 * @brief Frees Shape attribute
 *
 * @param attr Shape node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
shape *
FREEattribShape (shape *attr, node *parent)
{
    DBUG_ENTER ("FREEattribShape");

    if (attr != NULL) {
        attr = SHfreeShape (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribConstVecPointer
 *
 * @brief Frees ConstVecPointer attribute
 *
 * @param attr ConstVecPointer node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
void *
FREEattribConstVecPointer (void *attr, node *parent)
{
    DBUG_ENTER ("FREEattribConstVecPointer");

    if (attr != NULL) {
        attr = MEMfree (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribAccess
 *
 * @brief Frees Access attribute
 *
 * @param attr Access node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
access_t *
FREEattribAccess (access_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribAccess");

    while (attr != NULL) {
        access_t *tmp = attr;
        attr = attr->next;
        tmp->offset = FREEfreeShpseg (tmp->offset);
        tmp = MEMfree (tmp);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribAccessInfo
 *
 * @brief Frees AccessInfo attribute
 *
 * @param attr AccessInfo node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
access_info_t *
FREEattribAccessInfo (access_info_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribAccessInfo");

    if (attr != NULL) {
        attr->access = FREEattribAccess (attr->access, parent);
        attr = MEMfree (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribRc
 *
 * @brief Frees Rc attribute
 *
 * @param attr Access node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
rc_t *
FREEattribRc (rc_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribRc");

    while (attr != NULL) {
        rc_t *tmp = attr;
        attr = attr->next;
        tmp = MEMfree (tmp);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribReuseInfo
 *
 * @brief Frees ReuseInfo attribute
 *
 * @param attr ReuseInfo node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
reuse_info_t *
FREEattribReuseInfo (reuse_info_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribReuseInfo");

    if (attr != NULL) {
        attr->rcs = FREEattribRc (attr->rcs, parent);
        attr = MEMfree (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribIndex
 *
 * @brief Frees Index attribute
 *
 * @param attr Access node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
index_t *
FREEattribIndex (index_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribIndex");

    while (attr != NULL) {
        index_t *tmp = attr;
        attr = attr->next;
        tmp = MEMfree (tmp);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribCudaAccessInfo
 *
 * @brief
 *
 * @param attr ReuseInfo node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
cuda_access_info_t *
FREEattribCudaAccessInfo (cuda_access_info_t *attr, node *parent)
{
    int i;

    DBUG_ENTER ("FREEattribCudaAccessInfo");

    if (attr != NULL) {
        for (i = 0; i < MAX_REUSE_DIM; i++) {
            CUAI_INDICES (attr, i) = FREEattribIndex (CUAI_INDICES (attr, i), parent);
        }

        attr = MEMfree (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribShpSeg
 *
 * @brief Frees ShpSeg attribute
 *
 * @param attr ShpSeg node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
shpseg *
FREEattribShpSeg (shpseg *attr, node *parent)
{
    DBUG_ENTER ("FREEattribShpSeg");

    if (attr != NULL) {
        SHPSEG_NEXT (attr) = FREEattribShpSeg (SHPSEG_NEXT (attr), parent);
        attr = MEMfree (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribIntegerPointer
 *
 * @brief Frees IntegerPointer attribute
 *
 * @param attr IntegerPointer node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
int *
FREEattribIntegerPointer (int *attr, node *parent)
{
    DBUG_ENTER ("FREEattribIntegerPointer");

    if (attr != NULL) {
        attr = MEMfree (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribIntegerPointerArray
 *
 * @brief Frees IntegerPointerArray attribute
 *
 * @param attr IntegerPointerArray node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
int *
FREEattribIntegerPointerArray (int *attr, node *parent)
{
    DBUG_ENTER ("FREEattribIntegerPointerArray");

    if (attr != NULL) {
        attr = MEMfree (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribScheduling
 *
 * @brief Frees Scheduling attribute
 *
 * @param attr Scheduling node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
sched_t *
FREEattribScheduling (sched_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribScheduling");

    if (attr != NULL) {
        attr = SCHremoveScheduling (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribTaskSel
 *
 * @brief Frees TaskSel attribute
 *
 * @param attr TaskSel node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
tasksel_t *
FREEattribTaskSel (tasksel_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribTaskSel");

    if (attr != NULL) {
        attr = SCHremoveTasksel (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribNodePointer
 *
 * @brief Frees NodePointer attribute
 *
 * @param attr NodePointer node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node **
FREEattribNodePointer (node **attr, node *parent)
{
    DBUG_ENTER ("FREEattribNodePointer");

    /* TODO: implement node pointer free function */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribConstant
 *
 * @brief Frees Constant attribute
 *
 * @param attr Constant node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
constant *
FREEattribConstant (constant *attr, node *parent)
{
    DBUG_ENTER ("FREEattribConstant");

    if (attr != NULL) {
        attr = COfreeConstant (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribStringSet
 *
 * @brief Frees RCCounter attribute
 *
 * @param attr StringSet attrib to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
stringset_t *
FREEattribStringSet (stringset_t *attr, node *parent)
{
    DBUG_ENTER ("FREEattribRCCounter");

    attr = STRSfree (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn index_info *FREEattribIndexInfo (index_info * attr)
 *
 * @brief Frees index_info attribute
 *
 * @param attr StringSet attrib to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
index_info *
FREEattribIndexInfo (index_info *attr, node *parent)
{
    DBUG_ENTER ("FREEattribIndexInfo");

    attr = MEMfree (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn compinfo** FREEattribCompInfo (compinfo** attr, node *parent)
 *
 * @brief Frees Info attribute in N_tfvertex
 *
 * @param attr is a pointer to a structure of type compinfo **
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
compinfo **
FREEattribCompInfoArr (compinfo **attr, node *parent)
{

    DBUG_ENTER ("FREEattribReachMat");

    int i, j;

    if (attr != NULL) {

        for (i = 0; i < TFSPEC_NUMCOMP (parent); i++) {

            if (attr[i] != NULL) {

                if (COMPINFO_CSRC (attr[i]) != NULL) {
                    freeDynarray (COMPINFO_CSRC (attr[i]));
                }

                if (COMPINFO_CTAR (attr[i]) != NULL) {
                    freeDynarray (COMPINFO_CTAR (attr[i]));
                }

                if (COMPINFO_TLTABLE (attr[i]) != NULL) {
                    freeDynarray (COMPINFO_TLTABLE (attr[i]));
                }

                if (COMPINFO_CROSSCLOS (attr[i]) != NULL) {
                    freeMatrix (COMPINFO_CROSSCLOS (attr[i]));
                }

                if (COMPINFO_TLC (attr[i]) != NULL) {
                    freeMatrix (COMPINFO_TLC (attr[i]));
                }

                /*LUBMat consists of 3 matrices*/
                for (j = 0; j < 3; j++) {
                    if (COMPINFO_LUBPOS (attr[i], j) != NULL) {
                        freeMatrix (COMPINFO_LUBPOS (attr[i], j));
                    }
                }

                if (COMPINFO_DIST (attr[i]) != NULL) {
                    freeMatrix (COMPINFO_DIST (attr[i]));
                }

                attr[i] = MEMfree (attr[i]);
            }
        }
    }

    attr = MEMfree (attr);

    DBUG_RETURN (attr);
}
