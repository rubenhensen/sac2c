/*
 *
 * $Log$
 * Revision 1.13  2004/11/23 10:05:24  sah
 * SaC DevCamp 04
 *
 * Revision 1.12  2004/11/01 21:51:07  sah
 * added FreeDownLinkAttrib and cleaned up some code
 *
 * Revision 1.11  2004/10/28 17:25:13  sah
 * added FreeStringSetAttrib
 *
 * Revision 1.10  2004/10/13 15:19:44  sah
 * added NODE_ISALIVE check when processing Link attributes
 *
 * Revision 1.9  2004/10/11 16:48:33  sah
 * fixed problem with NCODE_DEC_USED
 *
 * Revision 1.8  2004/10/11 14:57:53  sah
 * made INC/DEC NCODE_USED explicit
 *
 * Revision 1.7  2004/10/05 16:16:05  sah
 * recursive self calls of loopfuns are handled
 * correctly now
 *
 * Revision 1.6  2004/10/04 17:17:00  sah
 * fixed bug and rearranged some code
 *
 * Revision 1.5  2004/09/30 20:15:10  sah
 * fixed signature of FreeRCCounterAttrib
 *
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
#include "stringset.h"

/** <!--******************************************************************-->
 *
 * @fn FREEattribString
 *
 * @brief Frees String attribute
 *
 * @param attr String node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
char *
FREEattribString (char *attr)
{
    DBUG_ENTER ("FREEattribString");

    if (attr != NULL) {
        DBUG_PRINT ("FREE", ("Freeing string '%s' at " F_PTR, attr, attr));
        attr = FREEfree (attr);
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
char *
FREEattribSharedString (char *attr)
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
types *
FREEattribOldType (types *attr)
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FREEattribNode (node *attr)
{
    DBUG_ENTER ("FREEattribNode");

    if (attr != NULL) {
        DBUG_PRINT ("FREE", ("Starting to free %s node attribute at " F_PTR,
                             NODE_TEXT (attr), attr));
        attr = FREEfreeTree (attr);
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FREEattribLink (node *attr)
{
    DBUG_ENTER ("FREEattribLink");

    /* when handling link attributes, always check whether the target   */
    /* has already been freed. This can be done using the NODE_ISALIVE  */
    /* macro...                                                         */
    if ((attr != NULL) && (NODE_ISALIVE (attr))) {
        if (NODE_TYPE (attr) == N_ncode) {
            NCODE_DEC_USED (attr);
        }
    }

    DBUG_RETURN ((node *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribExtLink
 *
 * @brief Frees Link attribute
 *
 * @param attr Link node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node *
FREEattribExtLink (node *attr)
{
    DBUG_ENTER ("FREEattribExtLink");

    if (attr != NULL) {
        if (NODE_ISALIVE (attr)) {
            if (NODE_TYPE (attr) == N_fundef) {
                DBUG_PRINT ("FREE", ("Decrementing use count for '%s' at " F_PTR,
                                     FUNDEF_NAME (attr), attr));
                DBUG_ASSERT ((NODE_TYPE (attr) == N_fundef),
                             "illegal value in AP_FUNDEF found!");

                DBUG_ASSERT (((!FUNDEF_ISLACFUN (attr))
                              || (FUNDEF_USED (attr) != USED_INACTIVE)),
                             "FUNDEF_USED must be active for LaC functions!");

                /* check whether this function is use-counted */
                if ((FUNDEF_USED (attr) != USED_INACTIVE)
                    && (!(FUNDEF_ISLOOPFUN (attr)))) {
                    (FUNDEF_USED (attr))--;

                    DBUG_ASSERT ((FUNDEF_USED (attr) >= 0),
                                 "FUNDEF_USED dropped below 0");

                    DBUG_PRINT ("FREE", ("decrementing used counter of %s to %d",
                                         FUNDEF_NAME (attr), FUNDEF_USED (attr)));

                    if (FUNDEF_USED (attr) == 0) {
                        /*
                         * referenced fundef no longer used
                         *  -> transform it into a zombie
                         */
                        DBUG_PRINT ("FREE", ("Use count reached 0 for '%s' at " F_PTR,
                                             FUNDEF_NAME (attr), attr));
                        attr = FREEfreeNode (attr);
                    }
                }
            }
        }
    }

    DBUG_RETURN ((node *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribDownLink
 *
 * @brief Frees Link attribute
 *
 * @param attr Link node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/

node *
FREEattribDownLink (node *attr)
{
    DBUG_ENTER ("FREEattribDownLink");

    /* as this link points downwards in the ast, the
     * referenced node is already freed.
     */

    DBUG_RETURN ((node *)NULL);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribIntegerArray
 *
 * @brief Frees IntegerArray attribute
 *
 * @param attr IntegerArray node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
int *
FREEattribIntegerArray (int *attr)
{
    DBUG_ENTER ("FREEattribIntegerArray");

    /* TODO */

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribNums
 *
 * @brief Frees Nums attribute
 *
 * @param attr Nums node to process
 *
 * @return result of Free call, usually NULL
 *
 * TODO: remove as soon as nums are removed
 *
 ***************************************************************************/
nums *
FREEattribNums (nums *attr)
{
    DBUG_ENTER ("FREEattribNums");

    while (attr != NULL) {
        nums *tmp = attr;

        DBUG_PRINT ("FREE", ("Freeing nums structure at " F_PTR, attr));

        attr = NUMS_NEXT (attr);

        tmp = ILIBfree (tmp);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribLUT
 *
 * @brief Frees LUT attribute
 *
 * @param attr LUT node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
LUT_t
FREEattribLUT (LUT_t attr)
{
    DBUG_ENTER ("FREEattribLUT");

    if (attr != NULL) {
        attr = LUTremoveLUT (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribMask
 *
 * @brief Frees Mask attribute
 *
 * @param attr Mask node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
long *
FREEattribMask (long *attr)
{
    DBUG_ENTER ("FREEattribMask");

    if (attr != NULL) {
        DBUG_PRINT ("FREE", ("Freeing element of mask at " F_PTR, attr));
        attr = ILIBfree (attr);
    }

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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
nodelist *
FREEattribSharedNodeList (nodelist *attr)
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
dfmask_t *
FREEattribDFMask (dfmask_t *attr)
{
    DBUG_ENTER ("FREEattribDFMask");

#if 0 /* TODO: dfmasks are not correctly freed */
       
    if (attr != NULL) {
      attr = DFMremoveMask( attr);
    }

#else
    attr = NULL;
#endif

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribDFMaskBase
 *
 * @brief Frees DFMaskBase attribute
 *
 * @param attr DFMaskBase node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
DFMmask_base_t
FREEattribDFMaskBase (DFMmask_base_t attr)
{
    DBUG_ENTER ("FREEattribDFMaskBase");

    if (attr != NULL) {
        attr = DFMRemoveMaskBase (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribDFMFoldMask
 *
 * @brief Frees DFMFoldMask attribute
 *
 * @param attr DFMFoldMask node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
DFMfoldmask_t *
FREEattribDfFoldMask (DFMfoldmask_t *attr)
{
    DBUG_ENTER ("FREEattribDfFoldMask");

    if (attr != NULL) {
        attr = ILIBfree (attr);
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
ntype *
FREEattribNewType (ntype *attr)
{
    DBUG_ENTER ("FREEattribNewType");

    if (attr != NULL) {
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
argtab_t *
FREEattribArgTab (argtab_t *attr)
{
    DBUG_ENTER ("FREEattribArgTab");

    if (attr != NULL) {
        attr->ptr_in = ILIBfree (attr->ptr_in);
        attr->ptr_out = ILIBfree (attr->ptr_out);
        attr->tag = ILIBfree (attr->tag);
        attr->size = 0;

        attr = ILIBfree (attr);
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
index_info *
FREEattribIndexPointer (index_info *attr)
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
shape *
FREEattribShape (shape *attr)
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
void *
FREEattribConstVecPointer (void *attr)
{
    DBUG_ENTER ("FREEattribConstVecPointer");

    if (attr != NULL) {
        attr = ILIBfree (attr);
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
access_t *
FREEattribAccess (access_t *attr)
{
    DBUG_ENTER ("FREEattribAccess");

    while (attr != NULL) {
        access_t *tmp = attr;
        attr = attr->next;
        tmp->offset = FREEfreeShpSeg (tmp->offset);
        tmp = ILIBfree (tmp);
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
access_info_t *
FREEattribAccessInfo (access_info_t *attr)
{
    DBUG_ENTER ("FREEattribAccessInfo");

    if (attr != NULL) {
        attr->access = FREEattribAccess (attr->access);
        attr = ILIBfree (attr);
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
shpseg *
FREEattribShpSeg (shpseg *attr)
{
    DBUG_ENTER ("FREEattribShpSeg");

    if (attr != NULL) {
        SHPSEG_NEXT (attr) = FREEattribShpSeg (SHPSEG_NEXT (attr));
        attr = ILIBfree (attr);
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
int *
FREEattribIntegerPointer (int *attr)
{
    DBUG_ENTER ("FREEattribIntegerPointer");

    if (attr != NULL) {
        attr = ILIBfree (attr);
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
int *
FREEattribIntegerPointerArray (int *attr)
{
    DBUG_ENTER ("FREEattribIntegerPointerArray");

    if (attr != NULL) {
        attr = ILIBfree (attr);
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
SCHsched_t
FREEattribScheduling (SCHsched_t attr)
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
SCHtasksel_t
FREEattribTaskSel (SCHtasksel_t attr)
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
node **
FREEattribNodePointer (node **attr)
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
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
constant *
FREEattribConstant (constant *attr)
{
    DBUG_ENTER ("FREEattribConstant");

    if (attr != NULL) {
        attr = COfreeConstant (attr);
    }

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribRCCounter
 *
 * @brief Frees RCCounter attribute
 *
 * @param attr RCCounter node to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
rc_counter *
FREEattribRCCounter (rc_counter *attr)
{
    DBUG_ENTER ("FREEattribRCCounter");

    DBUG_ASSERT ((attr == NULL), "Found an RCCounter outside of emm!!!");

    DBUG_RETURN (attr);
}

/** <!--******************************************************************-->
 *
 * @fn FREEattribStringSet
 *
 * @brief Frees RCCounter attribute
 *
 * @param attr StringSet attrib to process
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
stringset_t *
FREEattribStringSet (stringset_t *attr)
{
    DBUG_ENTER ("FREEattribRCCounter");

    attr = SSfree (attr);

    DBUG_RETURN (attr);
}
