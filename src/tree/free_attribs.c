/*
 *
 * $Log$
 * Revision 1.1  2004/08/08 16:03:50  sah
 * Initial revision
 *
 *
 */

#include "free_attribs.h"
#include "tree_basic.h"

/** <!--******************************************************************-->
 *
 * @fn FreeString
 *
 * @brief Frees String attribute
 *
 * @param attr String node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
char *
FreeString (char *attr)
{
    DBUG_ENTER ("FreeString");

    attr = Free (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeOldType
 *
 * @brief Frees OldType attribute
 *
 * @param attr OldType node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
types *
FreeOldType (types *attr)
{
    DBUG_ENTER ("FreeOldType");

    attr = FreeOneTypes (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeNode
 *
 * @brief Frees Node attribute
 *
 * @param attr Node node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FreeNode (node *attr)
{
    DBUG_ENTER ("FreeNode");

    attr = FreeTree (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeLink
 *
 * @brief Frees Link attribute
 *
 * @param attr Link node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FreeLink (node *attr)
{
    DBUG_ENTER ("FreeLink");

    /* do nothing */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeIntegerArray
 *
 * @brief Frees IntegerArray attribute
 *
 * @param attr IntegerArray node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
int *
FreeIntegerArray (int *attr)
{
    DBUG_ENTER ("FreeIntegerArray");

    /* TODO */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeNums
 *
 * @brief Frees Nums attribute
 *
 * @param attr Nums node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
nums *
FreeNums (nums *attr)
{
    DBUG_ENTER ("FreeNums");

    while (attr != NULL) {
        nums *tmp = attr;
        attr = NUMS_NEXT (attr);
        tmp = Free (tmp);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeLUT
 *
 * @brief Frees LUT attribute
 *
 * @param attr LUT node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
LUT_t
FreeLUT (LUT_t attr)
{
    DBUG_ENTER ("FreeLUT");

    attr = RemoveLUT (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeMask
 *
 * @brief Frees Mask attribute
 *
 * @param attr Mask node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
long *
FreeMask (long *attr)
{
    DBUG_ENTER ("FreeMask");

    if (attr != NULL) {
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeDeps
 *
 * @brief Frees Deps attribute
 *
 * @param attr Deps node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
deps *
FreeDeps (deps *attr)
{
    DBUG_ENTER ("FreeDeps");

    attr = FreeAllDeps (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeIds
 *
 * @brief Frees Ids attribute
 *
 * @param attr Ids node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
ids *
FreeIds (ids *attr)
{
    DBUG_ENTER ("FreeIds");

    attr = FreeAllIds (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeNodeList
 *
 * @brief Frees NodeList attribute
 *
 * @param attr NodeList node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
nodelist *
FreeNodeList (nodelist *attr)
{
    DBUG_ENTER ("FreeNodeList");

    while (attr != NULL) {
        nodelist *tmp = attr;
        attr = NODELIST_NEXT (attr);
        tmp = Free (tmp);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeSharedNodeList
 *
 * @brief Frees SharedNodeList attribute
 *
 * @param attr SharedNodeList node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
nodelist *
FreeSharedNodeList (nodelist *attr)
{
    DBUG_ENTER ("FreeSharedNodeList");

    /* do nothing here */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreePragmaLink
 *
 * @brief Frees PragmaLink attribute
 *
 * @param attr PragmaLink node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FreePragmaLink (node *attr)
{
    DBUG_ENTER ("FreePragmaLink");

    /* do nothing here */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeDFMMask
 *
 * @brief Frees DFMMask attribute
 *
 * @param attr DFMMask node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
DFMmask_t
FreeDFMMask (DFMmask_t attr)
{
    DBUG_ENTER ("FreeDFMMask");

    attr = DFMRemoveMask (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeDFMMaskBase
 *
 * @brief Frees DFMMaskBase attribute
 *
 * @param attr DFMMaskBase node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
DFMmask_base_t
FreeDFMMaskBase (DFMmask_base_t attr)
{
    DBUG_ENTER ("FreeDFMMaskBase");

    attr = DFMRemoveMaskBase (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeDFMFoldMask
 *
 * @brief Frees DFMFoldMask attribute
 *
 * @param attr DFMFoldMask node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
DFMfoldmask_t *
FreeDFMFoldMask (DFMfoldmask_t *attr)
{
    DBUG_ENTER ("FreeDFMFoldMask");

    if (attr != NULL) {
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeNewType
 *
 * @brief Frees NewType attribute
 *
 * @param attr NewType node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
ntype *
FreeNewType (ntype *attr)
{
    DBUG_ENTER ("FreeNewType");

    attr = TYFreeType (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeArgTab
 *
 * @brief Frees ArgTab attribute
 *
 * @param attr ArgTab node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
argtab_t *
FreeArgTab (argtab_t *attr)
{
    DBUG_ENTER ("FreeArgTab");

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
 * @fn FreeIndexPointer
 *
 * @brief Frees IndexPointer attribute
 *
 * @param attr IndexPointer node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
index_info *
FreeIndexPointer (index_info *attr)
{
    DBUG_ENTER ("FreeIndexPointer");

    if (attr != NULL) {
        attr = FreeIndexInfo (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeShape
 *
 * @brief Frees Shape attribute
 *
 * @param attr Shape node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
shape *
FreeShape (shape *attr)
{
    DBUG_ENTER ("FreeShape");

    attr = SHFreeShape (attr);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeConstVecPointer
 *
 * @brief Frees ConstVecPointer attribute
 *
 * @param attr ConstVecPointer node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
void *
FreeConstVecPointer (void *attr)
{
    DBUG_ENTER ("FreeConstVecPointer");

    if (attr != NULL) {
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeAccess
 *
 * @brief Frees Access attribute
 *
 * @param attr Access node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
access_t *
FreeAccess (access_t *attr)
{
    DBUG_ENTER ("FreeAccess");

    while (attr != NULL) {
        access_t *tmp = attr;
        attr = attr->next;
        tmp->offset = FreeShpSeg (tmp->offset);
        tmp = Free (tmp);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeAccessInfo
 *
 * @brief Frees AccessInfo attribute
 *
 * @param attr AccessInfo node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
access_info_t *
FreeAccessInfo (access_info_t *attr)
{
    DBUG_ENTER ("FreeAccessInfo");

    if (attr != NULL) {
        attr->access = FreeAccess (attr->access);
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeShpSeg
 *
 * @brief Frees ShpSeg attribute
 *
 * @param attr ShpSeg node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
shpseg *
FreeShpSeg (shpseg *attr)
{
    DBUG_ENTER ("FreeShpSeg");

    if (attr != NULL) {
        FreeShpSeg (SHPSEG_NEXT (attr));
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeIntegerPointer
 *
 * @brief Frees IntegerPointer attribute
 *
 * @param attr IntegerPointer node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
int *
FreeIntegerPointer (int *attr)
{
    DBUG_ENTER ("FreeIntegerPointer");

    if (attr != NULL) {
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeIntegerPointerArray
 *
 * @brief Frees IntegerPointerArray attribute
 *
 * @param attr IntegerPointerArray node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
int *
FreeIntegerPointerArray (int *attr)
{
    DBUG_ENTER ("FreeIntegerPointerArray");

    if (attr != NULL) {
        attr = Free (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeScheduling
 *
 * @brief Frees Scheduling attribute
 *
 * @param attr Scheduling node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
SCHsched_t
FreeScheduling (SCHsched_t attr)
{
    DBUG_ENTER ("FreeScheduling");

    if (attr != NULL) {
        attr = SCHRemoveScheduling (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeTaskSel
 *
 * @brief Frees TaskSel attribute
 *
 * @param attr TaskSel node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
SCHtasksel_t
FreeTaskSel (SCHtasksel_t attr)
{
    DBUG_ENTER ("FreeTaskSel");

    if (attr != NULL) {
        attr = SCHRemoveTasksel (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeNodePointer
 *
 * @brief Frees NodePointer attribute
 *
 * @param attr NodePointer node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node **
FreeNodePointer (node **attr)
{
    DBUG_ENTER ("FreeNodePointer");

    /* TODO */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FreeConstant
 *
 * @brief Frees Constant attribute
 *
 * @param attr Constant node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
constant *
FreeConstant (constant *attr)
{
    DBUG_ENTER ("FreeConstant");

    if (attr != NULL) {
        attr = COFreeConstant (attr);
    }

    DBUG_RETURN (attr);
}
