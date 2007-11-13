/*
 * $Id$
 */

#include "check_attribs.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "stringset.h"
#include "traverse.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "new_types.h"
#include "shape.h"
#include "constants.h"
#include "scheduling.h"
#include "namespaces.h"
#include "dbug.h"
#include "check_mem.h"

/** <!--******************************************************************-->
 *
 * @fn CHKMattribString
 *
 * @brief Touch String attribute
 *
 * @param attr String node to process
 * @param arg_info arg_info structure
 *
 * @return the string
 *
 ***************************************************************************/
char *
CHKMattribString (char *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribString");

    if (attr != NULL) {
        CHKMtouch (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribSharedString
 *
 * @brief Touch String attribute
 *
 * @param attr String node to process
 * @param arg_info arg_info structure
 *
 * @return NULL
 *
 ***************************************************************************/
char *
CHKMattribSharedString (char *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribSharedString");

    /* do nothing */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribOldType
 *
 * @brief Touch OldType attribute
 *
 * @param attr OldType node to process
 * @param arg_info arg_info structure
 *
 * @return the attribute
 *
 ***************************************************************************/
types *
CHKMattribOldType (types *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribOldType");

    if (attr != NULL) {
        CHKMtouch (attr, arg_info); /* problem */
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribNode
 *
 * @brief Touch Node attribute
 *
 * @param attr Node node to process
 * @param arg_info arg_info structure
 *
 * @return the attribute
 *
 ***************************************************************************/
node *
CHKMattribNode (node *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribNode");

    if (attr != NULL) {
        attr = TRAVdo (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribNamespace
 *
 * @brief Touch Namespace attribute
 *
 * @param attr Namespace node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
namespace_t *
CHKMattribNamespace (namespace_t *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribNamespace");

    if (attr != NULL) {
        NStouchNamespace (attr, arg_info); /* problem */
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribLink
 *
 * @brief Touch Link attribute
 *
 * @param attr Link node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
node *
CHKMattribLink (node *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribLink");

    /*
     * NEVER do anything with this kind of attribute
     * as you cannot make sure the node you reference
     * here really exists!
     */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribExtLink
 *
 * @brief Touch Link attribute
 *
 * @param attr Link node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
node *
CHKMattribExtLink (node *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribExtLink");

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribCodeLink
 *
 * @brief Touch Link attribute
 *
 * @param attr Link node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/

node *
CHKMattribCodeLink (node *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribCodeLink");

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribLUT
 *
 * @brief Touch LUT attribute
 *
 * @param attr LUT node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
lut_t *
CHKMattribLUT (lut_t *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribLUT");

    if (attr != NULL) {
        LUTtouchLut (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribNodeList
 *
 * @brief Touch NodeList attribute
 *
 * @param attr NodeList node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
nodelist *
CHKMattribNodeList (nodelist *attr, info *arg_info)
{
    nodelist *list = attr;

    DBUG_ENTER ("CHKMattribNodeList");

    while (list != NULL) {
        CHKMtouch (list, arg_info);
        list = NODELIST_NEXT (list);
    }

    CHKMtouch (attr, arg_info);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribSharedNodeList
 *
 * @brief Touch SharedNodeList attribute
 *
 * @param attr SharedNodeList node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
nodelist *
CHKMattribSharedNodeList (nodelist *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribSharedNodeList");

    /* do nothing here */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribDFMask
 *
 * @brief Touch DFMask attribute
 *
 * @param attr DFMask node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
dfmask_t *
CHKMattribDFMask (dfmask_t *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribDFMask");

    if (attr != NULL) {
        DFMtouchMask (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribDFMaskBase
 *
 * @brief Touch DFMaskBase attribute
 *
 * @param attr DFMaskBase node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
dfmask_base_t *
CHKMattribDFMaskBase (dfmask_base_t *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribDFMaskBase");

    if (attr != NULL) {
        DFMtouchMaskBase (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribNewType
 *
 * @brief Touch NewType attribute
 *
 * @param attr NewType node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
ntype *
CHKMattribNewType (ntype *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribNewType");

    if (attr != NULL) {
        TYtouchType (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribArgTab
 *
 * @brief Touch ArgTab attribute
 *
 * @param attr ArgTab node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
argtab_t *
CHKMattribArgTab (argtab_t *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribArgTab");

    if (attr != NULL) {
        CHKMtouch (attr->ptr_in, arg_info);
        CHKMtouch (attr->ptr_out, arg_info);
        CHKMtouch (attr->tag, arg_info);

        CHKMtouch (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribIndexPointer
 *
 * @brief Touch IndexPointer attribute
 *
 * @param attr IndexPointer node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
index_info *
CHKMattribIndexPointer (index_info *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribIndexPointer");

    if (attr != NULL) {

        CHKMtouch (attr->permutation, arg_info);
        CHKMtouch (attr->last, arg_info);
        CHKMtouch (attr->const_arg, arg_info);
        CHKMtouch (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribShape
 *
 * @brief Touch Shape attribute
 *
 * @param attr Shape node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
shape *
CHKMattribShape (shape *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribShape");

    if (attr != NULL) {
        SHtouchShape (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribConstVecPointer
 *
 * @brief Touch ConstVecPointer attribute
 *
 * @param attr ConstVecPointer node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
void *
CHKMattribConstVecPointer (void *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribConstVecPointer");

    if (attr != NULL) {
        CHKMtouch (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribAccess
 *
 * @brief Touch Access attribute
 *
 * @param attr Access node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
access_t *
CHKMattribAccess (access_t *attr, info *arg_info)
{
    access_t *tmp = attr;

    DBUG_ENTER ("CHKMattribAccess");

    while (tmp != NULL) {

        CHKMattribShpSeg (tmp->offset, arg_info);

        CHKMtouch (tmp, arg_info);
        tmp = tmp->next;
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribAccessInfo
 *
 * @brief Touch AccessInfo attribute
 *
 * @param attr AccessInfo node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
access_info_t *
CHKMattribAccessInfo (access_info_t *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribAccessInfo");

    if (attr != NULL) {
        attr->access = CHKMattribAccess (attr->access, arg_info);
        CHKMtouch (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribShpSeg
 *
 * @brief Touch ShpSeg attribute
 *
 * @param attr ShpSeg node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
shpseg *
CHKMattribShpSeg (shpseg *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribShpSeg");

    if (attr != NULL) {
        SHPSEG_NEXT (attr) = CHKMattribShpSeg (SHPSEG_NEXT (attr), arg_info);
        CHKMtouch (attr, arg_info);
    }
    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribIntegerPointer
 *
 * @brief Touch IntegerPointer attribute
 *
 * @param attr IntegerPointer node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
int *
CHKMattribIntegerPointer (int *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribIntegerPointer");

    if (attr != NULL) {
        CHKMtouch (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribIntegerPointerArray
 *
 * @brief Touch IntegerPointerArray attribute
 *
 * @param attr IntegerPointerArray node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
int *
CHKMattribIntegerPointerArray (int *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribIntegerPointerArray");

    if (attr != NULL) {
        CHKMtouch (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribScheduling
 *
 * @brief Touch Scheduling attribute
 *
 * @param attr Scheduling node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
sched_t *
CHKMattribScheduling (sched_t *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribScheduling");

    if (attr != NULL) {
        SCHtouchScheduling (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribTaskSel
 *
 * @brief Touch TaskSel attribute
 *
 * @param attr TaskSel node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
tasksel_t *
CHKMattribTaskSel (tasksel_t *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribTaskSel");

    if (attr != NULL) {
        SCHtouchTasksel (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribNodePointer
 *
 * @brief Touch NodePointer attribute
 *
 * @param attr NodePointer node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
node **
CHKMattribNodePointer (node **attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribNodePointer");

    /* TODO: implement node pointer free function */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribConstant
 *
 * @brief Touch Constant attribute
 *
 * @param attr Constant node to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
constant *
CHKMattribConstant (constant *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribConstant");

    if (attr != NULL) {
        COtouchConstant (attr, arg_info);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn CHKMattribStringSet
 *
 * @brief Touch RCCounter attribute
 *
 * @param attr StringSet attrib to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
stringset_t *
CHKMattribStringSet (stringset_t *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribRCCounter");

    STRStouch (attr, arg_info);

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn index_info *CHKMattribIndexInfo (index_info * attr)
 *
 * @brief Touch index_info attribute
 *
 * @param attr StringSet attrib to process
 * @param arg_info arg_info structure
 *
 * @return entire attribute
 *
 ***************************************************************************/
index_info *
CHKMattribIndexInfo (index_info *attr, info *arg_info)
{
    DBUG_ENTER ("CHKMattribIndexInfo");

    CHKMtouch (attr, arg_info);

    DBUG_RETURN (attr);
}
