/*
 *
 * $Log$
 * Revision 1.3  2004/12/07 20:36:58  ktr
 * eliminated CONSTVEC which is superseded by ntypes.
 *
 * Revision 1.2  2004/11/26 21:18:50  sah
 * pour Bodo *<8-)
 *
 * Revision 1.1  2004/11/23 22:40:58  sah
 * Initial revision
 *
 *
 *
 */

/**
 * @file serialize_attribs.c
 *
 * Functions needed by serialize traversal to serialize attributes
 *
 */

/**
 * @defgroup serialize Serialize Tree Functions.
 *
 * Functions needed by serialize traversal.
 *
 * @{
 */

#define NEW_INFO

#include "serialize_attribs.h"
#include "serialize_info.h"
#include "serialize_stack.h"
#include "serialize.h"
#include "constants.h"
#include "tree_basic.h"
#include "traverse.h"
#include "tree_compound.h"
#include "new_types.h"
#include "shape.h"
#include "globals.h"
#include "constants.h"
#include "dbug.h"

/** <!--******************************************************************-->
 *
 * @fn SATserializeString
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeString (info *info, char *attr, node *parent)
{
    DBUG_ENTER ("SATserializeString");

    if (attr == NULL) {
        DBUG_PRINT ("SET", ("Processing String (null)"));

        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        DBUG_PRINT ("SET", ("Processing String `%s'", attr));

        fprintf (INFO_SER_FILE (info), "StringCopy(\"%s\")", attr);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeSharedString
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeSharedString (info *info, char *attr, node *parent)
{
    DBUG_ENTER ("SATserializeSharedString");

    if (attr == NULL) {
        DBUG_PRINT ("SET", ("Processing String (null)"));

        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        DBUG_PRINT ("SET", ("Processing String `%s'", attr));

        fprintf (INFO_SER_FILE (info), "StringCopy(\"%s\")", attr);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeInteger
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeInteger (info *info, int attr, node *parent)
{
    DBUG_ENTER ("SATserializeInteger");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeLong
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeLong (info *info, long attr, node *parent)
{
    DBUG_ENTER ("SATserializeLong");

    fprintf (INFO_SER_FILE (info), "%ld", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeBool
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeBool (info *info, bool attr, node *parent)
{
    DBUG_ENTER ("SATserializeBool");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeFloat
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeFloat (info *info, float attr, node *parent)
{
    DBUG_ENTER ("SATserializeFloat");

    fprintf (INFO_SER_FILE (info), "%f", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeDouble
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeDouble (info *info, double attr, node *parent)
{
    DBUG_ENTER ("SATserializeDouble");

    fprintf (INFO_SER_FILE (info), "%f", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeChar
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeChar (info *info, char attr, node *parent)
{
    DBUG_ENTER ("SATserializeChar");

    fprintf (INFO_SER_FILE (info), "'%c'", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeOldType
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeOldType (info *info, types *attr, node *parent)
{
    DBUG_ENTER ("SATserializeOldType");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        fprintf (INFO_SER_FILE (info), "MakeTypes1( %d)", T_unknown);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeNode
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeNode (info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SATserializeNode");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        /* we have to eat the additional , created by SET traversal */
        fprintf (INFO_SER_FILE (info), "DROP( x");
        TRAVdo (attr, info);
        fprintf (INFO_SER_FILE (info), ")");
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeLink
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeLink (info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SATserializeLink");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeExtLink
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeExtLink (info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SATserializeExtink");

    if (attr != NULL) {
        if (NODE_TYPE (attr) == N_fundef) {
            SERserializeFundefLink (attr, INFO_SER_FILE (info));
        }
    } else {
        fprintf (INFO_SER_FILE (info), "NULL");
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeDownLink
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeDownLink (info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SATserializeDownLink");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeIntegerArray
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeIntegerArray (info *info, int *attr, node *parent)
{
    DBUG_ENTER ("SATserializeIntegerArray");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        int cnt;

        DBUG_ASSERT ((NODE_TYPE (parent) == N_pragma),
                     ("Found an IntegerArrayute attached to a node different from "
                      "N_pragma! "));

        fprintf (INFO_SER_FILE (info), "CreateIntegerArray( %d",
                 PRAGMA_NUMPARAMS (parent));

        for (cnt = 0; cnt < PRAGMA_NUMPARAMS (parent); cnt++) {
            fprintf (INFO_SER_FILE (info), ", %d", attr[cnt]);
        }

        fprintf (INFO_SER_FILE (info), ")");
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeFileType
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeFileType (info *info, file_type attr, node *parent)
{
    DBUG_ENTER ("SATserializeFileType");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeLUT
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeLUT (info *info, lut_t *attr, node *parent)
{
    DBUG_ENTER ("SATserializeLUT");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializePrf
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializePrf (info *info, prf attr, node *parent)
{
    DBUG_ENTER ("SATserializePrf");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeMask
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param pos    position within the current array
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeMask (info *info, int pos, long *attr, node *parent)
{
    DBUG_ENTER ("SATserializeMask");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeDeps
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeDeps (info *info, deps *attr, node *parent)
{
    DBUG_ENTER ("SATserializeDeps");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeStatusType
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeStatusType (info *info, statustype attr, node *parent)
{
    DBUG_ENTER ("SATserializeStatusType");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeNodeList
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeNodeList (info *info, nodelist *attr, node *parent)
{
    DBUG_ENTER ("SATserializeNodeList");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeSharedNodeList
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeSharedNodeList (info *info, nodelist *attr, node *parent)
{
    DBUG_ENTER ("SATserializeSharedNodeList");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializePragmaLink
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializePragmaLink (info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SATserializePragmaLink");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeDFMask
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeDFMask (info *info, dfmask_t *attr, node *parent)
{
    DBUG_ENTER ("SATserializeDFMask");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeDFMaskBase
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeDFMaskBase (info *info, dfmask_base_t *attr, node *parent)
{
    DBUG_ENTER ("SATserializeDFMaskBase");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeNewType
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeNewType (info *info, ntype *attr, node *parent)
{
    DBUG_ENTER ("SATserializeNewType");

    DBUG_PRINT ("SET", ("Starting traversal for ntype attribute"));

    TYserializeType (INFO_SER_FILE (info), attr);

    DBUG_PRINT ("SET", ("Finished traversal for ntype attribute"));

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeArgTab
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeArgTab (info *info, argtab_t *attr, node *parent)
{
    DBUG_ENTER ("SATserializeArgTab");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeIndexPointer
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeIndexPointer (info *info, index_info *attr, node *parent)
{
    DBUG_ENTER ("SATserializeIndexPointer");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeShape
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeShape (info *info, shape *attr, node *parent)
{
    DBUG_ENTER ("SATserializeShape");

    SHserializeShape (INFO_SER_FILE (info), attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeSimpleType
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeSimpleType (info *info, simpletype attr, node *parent)
{
    DBUG_ENTER ("SATserializeSimpleType");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeUseFlag
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeUseFlag (info *info, useflag attr, node *parent)
{
    DBUG_ENTER ("SATserializeUseFlag");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeAccessInfo
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeAccessInfo (info *info, access_info_t *attr, node *parent)
{
    DBUG_ENTER ("SATserializeAccessInfo");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeShpSeg
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeShpSeg (info *info, shpseg *attr, node *parent)
{
    DBUG_ENTER ("SATserializeShpSeg");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeIntegerPointer
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeIntegerPointer (info *info, int *attr, node *parent)
{
    DBUG_ENTER ("SATserializeIntegerPointer");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeIntegerPointerArray
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param pos    position within the current array
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeIntegerPointerArray (info *info, int pos, int *attr, node *parent)
{
    DBUG_ENTER ("SATserializeIntegerPointerArray");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeScheduling
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeScheduling (info *info, sched_t *attr, node *parent)
{
    DBUG_ENTER ("SATserializeScheduling");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeTaskSel
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeTaskSel (info *info, tasksel_t *attr, node *parent)
{
    DBUG_ENTER ("SATserializeTaskSel");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeNodePointer
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeNodePointer (info *info, node **attr, node *parent)
{
    DBUG_ENTER ("SATserializeNodePointer");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeSSAPhi
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeSSAPhi (info *info, ssaphit_t attr, node *parent)
{
    DBUG_ENTER ("SATserializeSSAPhi");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeBitField
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeBitField (info *info, int attr, node *parent)
{
    DBUG_ENTER ("SATserializeBitField");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeConstant
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeConstant (info *info, constant *attr, node *parent)
{
    DBUG_ENTER ("SATserializeConstant");

    COserializeConstant (INFO_SER_FILE (info), attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeMTExecMode
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeMTExecMode (info *info, mtexecmode_t attr, node *parent)
{
    DBUG_ENTER ("SATserializeMTExecMode");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeRCCounter
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeRCCounter (info *info, rc_counter *attr, node *parent)
{
    DBUG_ENTER ("SATserializeMTExecMode");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/**
 * @}
 */
