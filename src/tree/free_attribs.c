/*
 *
 * $Log$
 * Revision 1.4  2004/09/30 19:52:43  sah
 * added RCCounter support
 *
 * Revision 1.3  2004/09/24 20:22:44  sah
 * Dataflowmasks are no more freed
 *
 * Revision 1.2  2004/08/29 18:10:05  sah
 * general improvements
 *
 * Revision 1.1  2004/08/08 16:03:50  sah
 * Initial revision
 *
 *
 */

#include "free_attribs.h"
#include "tree_basic.h"
#include "tree_compound.h"

/** <!--******************************************************************-->
 *
 * @fn FreeStringAttrib
 *
 * @brief Frees String attribute
 *
 * @param attr String node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
char *
FreeStringAttrib (char *attr)
{
    DBUG_ENTER ("FreeStringAttrib");

    if (attr != NULL) {
        DBUG_PRINT ("FREE", ("Freeing string '%s' at " F_PTR, attr, attr));
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeSharedStringAttrib
 *
 * @brief Frees String attribute
 *
 * @param attr String node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
char *
FreeSharedStringAttrib (char *attr)
{
    DBUG_ENTER ("FreeSharedStringAttrib");

    /* do nothing */

    DBUG_RETURN ((char *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FreeOldTypeAttrib
 *
 * @brief Frees OldType attribute
 *
 * @param attr OldType node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
types *
FreeOldTypeAttrib (types *attr)
{
    DBUG_ENTER ("FreeOldTypeAttrib");

    if (attr != NULL) {
        attr = FreeOneTypes (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeNodeAttrib
 *
 * @brief Frees Node attribute
 *
 * @param attr Node node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FreeNodeAttrib (node *attr)
{
    DBUG_ENTER ("FreeNodeAttrib");

    if (attr != NULL) {
        DBUG_PRINT ("FREE", ("Starting to free %s node attribute at " F_PTR,
                             mdb_nodetype[NODE_TYPE (attr)], attr));
        attr = FreeTree (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeLinkAttrib
 *
 * @brief Frees Link attribute
 *
 * @param attr Link node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FreeLinkAttrib (node *attr)
{
    DBUG_ENTER ("FreeLinkAttrib");

    /* do nothing */

    DBUG_RETURN ((node *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FreeApLinkAttrib
 *
 * @brief Frees Link attribute
 *
 * @param attr Link node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FreeApLinkAttrib (node *attr)
{
    DBUG_ENTER ("FreeLinkAttrib");

    if (attr != NULL) {
        DBUG_PRINT ("FREE", ("Decrementing use count for '%s' at " F_PTR,
                             FUNDEF_NAME (attr), attr));
        DBUG_ASSERT ((NODE_TYPE (attr) == N_fundef), "illegal value in AP_FUNDEF found!");

        DBUG_ASSERT (((!FUNDEF_IS_LACFUN (attr))
                      || (FUNDEF_USED (attr) != USED_INACTIVE)),
                     "FUNDEF_USED must be active for LaC functions!");

        if (FUNDEF_USED (attr) != USED_INACTIVE) {
            (FUNDEF_USED (attr))--;

            DBUG_ASSERT ((FUNDEF_USED (attr) >= 0), "FUNDEF_USED dropped below 0");

            DBUG_PRINT ("FREE", ("decrementing used counter of %s to %d",
                                 FUNDEF_NAME (attr), FUNDEF_USED (attr)));

            if (FUNDEF_USED (attr) == 0) {
                /*
                 * referenced fundef no longer used
                 *  -> transform it into a zombie
                 */
                DBUG_PRINT ("FREE", ("Use count reached 0 for '%s' at " F_PTR,
                                     FUNDEF_NAME (attr), attr));
                attr = FreeNode (attr);
            }
        }
    }

    DBUG_RETURN ((node *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FreeIntegerArrayAttrib
 *
 * @brief Frees IntegerArray attribute
 *
 * @param attr IntegerArray node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
int *
FreeIntegerArrayAttrib (int *attr)
{
    DBUG_ENTER ("FreeIntegerArrayAttrib");

    /* TODO */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeNumsAttrib
 *
 * @brief Frees Nums attribute
 *
 * @param attr Nums node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
nums *
FreeNumsAttrib (nums *attr)
{
    DBUG_ENTER ("FreeNumsAttrib");

    while (attr != NULL) {
        DBUG_PRINT ("FREE", ("Freeing nums structure at " F_PTR, attr));
        nums *tmp = attr;
        attr = NUMS_NEXT (attr);
        tmp = Free (tmp);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeLUTAttrib
 *
 * @brief Frees LUT attribute
 *
 * @param attr LUT node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
LUT_t
FreeLUTAttrib (LUT_t attr)
{
    DBUG_ENTER ("FreeLUTAttrib");

    if (attr != NULL) {
        attr = RemoveLUT (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeMaskAttrib
 *
 * @brief Frees Mask attribute
 *
 * @param attr Mask node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
long *
FreeMaskAttrib (long *attr)
{
    DBUG_ENTER ("FreeMaskAttrib");

    if (attr != NULL) {
        DBUG_PRINT ("FREE", ("Freeing element of mask at " F_PTR, attr));
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeDepsAttrib
 *
 * @brief Frees Deps attribute
 *
 * @param attr Deps node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
deps *
FreeDepsAttrib (deps *attr)
{
    DBUG_ENTER ("FreeDepsAttrib");

    if (attr != NULL) {
        attr = FreeAllDeps (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeIdsAttrib
 *
 * @brief Frees Ids attribute
 *
 * @param attr Ids node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
ids *
FreeIdsAttrib (ids *attr)
{
    DBUG_ENTER ("FreeIdsAttrib");

    if (attr != NULL) {
        attr = FreeAllIds (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeNodeListAttrib
 *
 * @brief Frees NodeList attribute
 *
 * @param attr NodeList node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
nodelist *
FreeNodeListAttrib (nodelist *attr)
{
    DBUG_ENTER ("FreeNodeListAttrib");

    while (attr != NULL) {
        DBUG_PRINT ("FREE", ("Freeing nodelist structure at " F_PTR, attr));
        nodelist *tmp = attr;
        attr = NODELIST_NEXT (attr);
        tmp = Free (tmp);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeSharedNodeListAttrib
 *
 * @brief Frees SharedNodeList attribute
 *
 * @param attr SharedNodeList node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
nodelist *
FreeSharedNodeListAttrib (nodelist *attr)
{
    DBUG_ENTER ("FreeSharedNodeListAttrib");

    /* do nothing here */

    DBUG_RETURN ((nodelist *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FreePragmaLinkAttrib
 *
 * @brief Frees PragmaLink attribute
 *
 * @param attr PragmaLink node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FreePragmaLinkAttrib (node *attr)
{
    DBUG_ENTER ("FreePragmaLinkAttrib");

    /* do nothing here */

    DBUG_RETURN ((node *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FreeDFMMaskAttrib
 *
 * @brief Frees DFMMask attribute
 *
 * @param attr DFMMask node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
DFMmask_t
FreeDFMMaskAttrib (DFMmask_t attr)
{
    DBUG_ENTER ("FreeDFMMaskAttrib");

    /* TODO

    if (attr != NULL) {
      attr = DFMRemoveMask( attr);
    }

    */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeDFMMaskBaseAttrib
 *
 * @brief Frees DFMMaskBase attribute
 *
 * @param attr DFMMaskBase node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
DFMmask_base_t
FreeDFMMaskBaseAttrib (DFMmask_base_t attr)
{
    DBUG_ENTER ("FreeDFMMaskBaseAttrib");

    if (attr != NULL) {
        attr = DFMRemoveMaskBase (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeDFMFoldMaskAttrib
 *
 * @brief Frees DFMFoldMask attribute
 *
 * @param attr DFMFoldMask node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
DFMfoldmask_t *
FreeDFMFoldMaskAttrib (DFMfoldmask_t *attr)
{
    DBUG_ENTER ("FreeDFMFoldMaskAttrib");

    if (attr != NULL) {
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeNewTypeAttrib
 *
 * @brief Frees NewType attribute
 *
 * @param attr NewType node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
ntype *
FreeNewTypeAttrib (ntype *attr)
{
    DBUG_ENTER ("FreeNewTypeAttrib");

    if (attr != NULL) {
        attr = TYFreeType (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeArgTabAttrib
 *
 * @brief Frees ArgTab attribute
 *
 * @param attr ArgTab node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
argtab_t *
FreeArgTabAttrib (argtab_t *attr)
{
    DBUG_ENTER ("FreeArgTabAttrib");

    if (attr != NULL) {
        attr->ptr_in = Free (attr->ptr_in);
        attr->ptr_out = Free (attr->ptr_out);
        attr->tag = Free (attr->tag);
        attr->size = 0;

        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeIndexPointerAttrib
 *
 * @brief Frees IndexPointer attribute
 *
 * @param attr IndexPointer node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
index_info *
FreeIndexPointerAttrib (index_info *attr)
{
    DBUG_ENTER ("FreeIndexPointerAttrib");

    if (attr != NULL) {
        attr = FreeIndexInfo (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeShapeAttrib
 *
 * @brief Frees Shape attribute
 *
 * @param attr Shape node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
shape *
FreeShapeAttrib (shape *attr)
{
    DBUG_ENTER ("FreeShapeAttrib");

    if (attr != NULL) {
        attr = SHFreeShape (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeConstVecPointerAttrib
 *
 * @brief Frees ConstVecPointer attribute
 *
 * @param attr ConstVecPointer node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
void *
FreeConstVecPointerAttrib (void *attr)
{
    DBUG_ENTER ("FreeConstVecPointerAttrib");

    if (attr != NULL) {
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeAccessAttrib
 *
 * @brief Frees Access attribute
 *
 * @param attr Access node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
access_t *
FreeAccessAttrib (access_t *attr)
{
    DBUG_ENTER ("FreeAccessAttrib");

    while (attr != NULL) {
        access_t *tmp = attr;
        attr = attr->next;
        tmp->offset = FreeShpSegAttrib (tmp->offset);
        tmp = Free (tmp);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeAccessInfoAttrib
 *
 * @brief Frees AccessInfo attribute
 *
 * @param attr AccessInfo node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
access_info_t *
FreeAccessInfoAttrib (access_info_t *attr)
{
    DBUG_ENTER ("FreeAccessInfoAttrib");

    if (attr != NULL) {
        attr->access = FreeAccessAttrib (attr->access);
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeShpSegAttrib
 *
 * @brief Frees ShpSeg attribute
 *
 * @param attr ShpSeg node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
shpseg *
FreeShpSegAttrib (shpseg *attr)
{
    DBUG_ENTER ("FreeShpSegAttrib");

    if (attr != NULL) {
        SHPSEG_NEXT (attr) = FreeShpSegAttrib (SHPSEG_NEXT (attr));
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeIntegerPointerAttrib
 *
 * @brief Frees IntegerPointer attribute
 *
 * @param attr IntegerPointer node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
int *
FreeIntegerPointerAttrib (int *attr)
{
    DBUG_ENTER ("FreeIntegerPointerAttrib");

    if (attr != NULL) {
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeIntegerPointerArrayAttrib
 *
 * @brief Frees IntegerPointerArray attribute
 *
 * @param attr IntegerPointerArray node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
int *
FreeIntegerPointerArrayAttrib (int *attr)
{
    DBUG_ENTER ("FreeIntegerPointerArrayAttrib");

    if (attr != NULL) {
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeSchedulingAttrib
 *
 * @brief Frees Scheduling attribute
 *
 * @param attr Scheduling node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
SCHsched_t
FreeSchedulingAttrib (SCHsched_t attr)
{
    DBUG_ENTER ("FreeSchedulingAttrib");

    if (attr != NULL) {
        attr = SCHRemoveScheduling (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeTaskSelAttrib
 *
 * @brief Frees TaskSel attribute
 *
 * @param attr TaskSel node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
SCHtasksel_t
FreeTaskSelAttrib (SCHtasksel_t attr)
{
    DBUG_ENTER ("FreeTaskSelAttrib");

    if (attr != NULL) {
        attr = SCHRemoveTasksel (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeNodePointerAttrib
 *
 * @brief Frees NodePointer attribute
 *
 * @param attr NodePointer node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node **
FreeNodePointerAttrib (node **attr)
{
    DBUG_ENTER ("FreeNodePointerAttrib");

    /* TODO */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeConstantAttrib
 *
 * @brief Frees Constant attribute
 *
 * @param attr Constant node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
constant *
FreeConstantAttrib (constant *attr)
{
    DBUG_ENTER ("FreeConstantAttrib");

    if (attr != NULL) {
        attr = COFreeConstant (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeRCCounterAttrib
 *
 * @brief Frees RCCounter attribute
 *
 * @param attr RCCounter node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
constant *
FreeConstantAttrib (constant *attr)
{
    DBUG_ENTER ("FreeConstantAttrib");

    DBUG_ASSERT ((attr == NULL), "Found an RCCounter outside of emm!!!");

    DBUG_RETURN (attr);
}
