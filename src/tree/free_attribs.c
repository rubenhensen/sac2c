/*
 *
 * $Log$
 * Revision 1.23  2005/01/08 09:54:58  ktr
 * Fixed some issues related to loops.
 *
 * Revision 1.22  2004/12/19 18:09:33  sah
 * post dk fixes
 *
 * Revision 1.21  2004/12/12 08:00:49  ktr
 * removed sons.any, attribs.any, NODE_ISALIVE because they were incompatible
 * with CLEANMEM.
 *
 * Revision 1.20  2004/12/09 18:53:05  sah
 * extended signature of free functions for
 * attributes so that they contain a link
 * to the parent node, now
 *
 * Revision 1.19  2004/12/09 16:39:41  sbs
 * FUNDEF_USED in loops constitutes a problem in the newAST!!!
 * FREEattribExtLink lacks the context in which it is called
 * see comment in that function!
 *
 * Revision 1.18  2004/12/09 16:29:20  sbs
 * now, the used counter is decremented for all those do-loops which do not
 * constitute the immediate recursive call.
 *
 * Revision 1.17  2004/12/09 14:33:05  sah
 * added dbug message
 *
 * Revision 1.16  2004/11/27 00:19:26  cg
 * Typo fixed.
 *
 * Revision 1.15  2004/11/25 23:44:04  sbs
 * FREEattribIndexInfo added.
 *
 * Revision 1.14  2004/11/24 20:44:50  sah
 * COMPILES!
 *
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

#include "tree_basic.h"
#include "tree_compound.h"
#include "stringset.h"
#include "internal_lib.h"
#include "free.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "new_types.h"
#include "shape.h"
#include "constants.h"
#include "scheduling.h"
#include "dbug.h"

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
        attr = ILIBfree (attr);
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
     * when handling link attributes, always check whether the target
     * has already been freed. ( This is checked via the attribs union)
     */
    if (attr != NULL) {
        switch (NODE_TYPE (attr)) {
        case N_code:
            if (attr->attribs.N_code != NULL) {
                CODE_DEC_USED (attr);
            }
            break;

        default:
            break;
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
        if (NODE_TYPE (attr) == N_fundef) {
            if (attr->attribs.N_fundef != NULL) {
                DBUG_PRINT ("FREE", ("Decrementing use count for '%s' at " F_PTR,
                                     FUNDEF_NAME (attr), attr));
                DBUG_ASSERT ((NODE_TYPE (attr) == N_fundef),
                             "illegal value in AP_FUNDEF found!");

                DBUG_ASSERT (((!FUNDEF_ISLACFUN (attr))
                              || (FUNDEF_USED (attr) != USED_INACTIVE)),
                             "FUNDEF_USED must be active for LaC functions!");

                /* check whether this function is use-counted */
                if ((FUNDEF_USED (attr) != USED_INACTIVE)
                    && ((!FUNDEF_ISDOFUN (attr)) || (FUNDEF_INT_ASSIGN (attr) == NULL)
                        || (parent != ASSIGN_RHS (FUNDEF_INT_ASSIGN (attr))))) {
                    /*
                     * if the function is no dofun or
                     * if it is a dofun, but this Ap was not the inner recursive call
                     * decrement use counter
                     */
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
                        attr = FREEdoFreeNode (attr);
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
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/

node *
FREEattribDownLink (node *attr, node *parent)
{
    DBUG_ENTER ("FREEattribDownLink");

    /* as this link points downwards in the ast, the
     * referenced node is already freed.
     */

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
 * @fn FREEattribMask
 *
 * @brief Frees Mask attribute
 *
 * @param attr Mask node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
long *
FREEattribMask (long *attr, node *parent)
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
 * @fn FREEattribRCCounter
 *
 * @brief Frees RCCounter attribute
 *
 * @param attr RCCounter node to process
 * @param parent parent node
 *
 * @return result of Free call, usually NULL
 *
 ***************************************************************************/
rc_counter *
FREEattribRCCounter (rc_counter *attr, node *parent)
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

    attr = ILIBfree (attr);

    DBUG_RETURN (attr);
}
