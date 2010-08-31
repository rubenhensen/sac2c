/*
 *
 * $Id$
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

#include "serialize_attribs.h"
#include "serialize_info.h"
#include "serialize_stack.h"
#include "serialize.h"
#include "constants.h"
#include "tree_basic.h"
#include "traverse.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "shape.h"
#include "globals.h"
#include "constants.h"
#include "namespaces.h"
#include "dbug.h"

/** <!--******************************************************************-->
 *
 * @fn SATserializeString
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeString (info *info, char *attr, node *parent)
{
    char *tmp;

    DBUG_ENTER ("SATserializeString");

    if (attr == NULL) {
        DBUG_PRINT ("SET", ("Processing String (null)"));

        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        DBUG_PRINT ("SET", ("Processing String `%s'", attr));

        tmp = STRstring2SafeCEncoding (attr);
        fprintf (INFO_SER_FILE (info), "STRcpy(\"%s\")", tmp);
        tmp = MEMfree (tmp);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeSharedString
 *
 * @brief generates code to serialize the given attribute
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

    SATserializeString (info, attr, parent);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeInteger
 *
 * @brief generates code to serialize the given attribute
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
 * @fn SATserializeByte
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeByte (info *info, char attr, node *parent)
{
    DBUG_ENTER ("SATserializeByte");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeShort
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeShort (info *info, short attr, node *parent)
{
    DBUG_ENTER ("SATserializeShort");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeInt
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeInt (info *info, int attr, node *parent)
{
    DBUG_ENTER ("SATserializeInt");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeLong
 *
 * @brief generates code to serialize the given attribute
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
 * @fn SATserializeLonglong
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeLonglong (info *info, long long attr, node *parent)
{
    DBUG_ENTER ("SATserializeLonglong");

    fprintf (INFO_SER_FILE (info), "%lldLL", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeUbyte
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeUbyte (info *info, unsigned char attr, node *parent)
{
    DBUG_ENTER ("SATserializeUbyte");

    fprintf (INFO_SER_FILE (info), "%u", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeUshort
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeUshort (info *info, unsigned short attr, node *parent)
{
    DBUG_ENTER ("SATserializeUshort");

    fprintf (INFO_SER_FILE (info), "%u", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeUint
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeUint (info *info, unsigned int attr, node *parent)
{
    DBUG_ENTER ("SATserializeUint");

    fprintf (INFO_SER_FILE (info), "%u", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeUlong
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeUlong (info *info, unsigned long attr, node *parent)
{
    DBUG_ENTER ("SATserializeUlong");

    fprintf (INFO_SER_FILE (info), "%lu", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeUlonglong
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeUlonglong (info *info, unsigned long long attr, node *parent)
{
    DBUG_ENTER ("SATserializeUlonglong");

    fprintf (INFO_SER_FILE (info), "%lluULL", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeBool
 *
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeFloat (info *info, float attr, node *parent)
{
    char *data;

    DBUG_ENTER ("SATserializeFloat");

    data = STRbytes2Hex (sizeof (float), (unsigned char *)&attr);

    fprintf (INFO_SER_FILE (info), "DShex2Float( \"%s\")", data);

    data = MEMfree (data);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeDouble
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeDouble (info *info, double attr, node *parent)
{
    char *data;

    DBUG_ENTER ("SATserializeDouble");

    data = STRbytes2Hex (sizeof (double), (unsigned char *)&attr);

    fprintf (INFO_SER_FILE (info), "DShex2Double( \"%s\")", data);

    data = MEMfree (data);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeChar
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeChar (info *info, unsigned char attr, node *parent)
{
    DBUG_ENTER ("SATserializeChar");

    fprintf (INFO_SER_FILE (info), "%u", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeOldType
 *
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @fn SATserializNamespace
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeNamespace (info *info, namespace_t *attr, node *parent)
{
    DBUG_ENTER ("SATserializeNamespace");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        NSserializeNamespace (INFO_SER_FILE (info), attr);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeLink
 *
 * @brief generates code to serialize the given attribute
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

    if (attr != NULL) {
        /*
         * if it is a link to an avis node, we have to check whether this
         * avis belongs to an arg. In that case, we serialize it here by
         * storing the arg-no. During deserialisation, this can be reconstructed
         * when the body is loaded.
         * All other links are handeled by the addLink traversal.
         */
        if ((NODE_TYPE (attr) == N_avis) && (NODE_TYPE (AVIS_DECL (attr)) == N_arg)
            && (INFO_SER_ARGAVISDIRECT (info))) {
            int pos = 0;
            node *args = FUNDEF_ARGS (INFO_SER_CURRENT (info));

            while ((args != NULL) && (ARG_AVIS (args) != attr)) {
                pos++;
                args = ARG_NEXT (args);
            }

            DBUG_ASSERT ((args != NULL), "found a link to an ARG_AVIS which does not "
                                         "belong to current fundef");

            fprintf (INFO_SER_FILE (info), "DSfetchArgAvis( %d)", pos);
        } else {
            fprintf (INFO_SER_FILE (info), "NULL");
        }
    } else {
        fprintf (INFO_SER_FILE (info), "NULL");
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeExtLink
 *
 * @brief generates code to serialize the given attribute
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
        switch (NODE_TYPE (attr)) {
        case N_fundef:
            SERserializeFundefLink (attr, INFO_SER_FILE (info));
            break;
        case N_objdef:
            SERserializeObjdefLink (attr, INFO_SER_FILE (info));
            break;
        default:
            DBUG_ASSERT (0, "unknown target for ExtLink found!");
            fprintf (INFO_SER_FILE (info), "NULL");
            break;
        }
    } else {
        fprintf (INFO_SER_FILE (info), "NULL");
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeLink
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeCodeLink (info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SATserializeCodeLink");

    /*
     * links to N_code nodes are serialized and deserialized
     * by the serialize traversal, so nothing has to be done
     * here
     */

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeFileType
 *
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @fn SATserializeNodeList
 *
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @fn SATserializeAccessInfo
 *
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @fn SATserializeBitField
 *
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @brief generates code to serialize the given attribute
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
 * @fn SATserializeTypeCheckingStatus
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeTypeCheckingStatus (info *info, NTC_stat attr, node *parent)
{
    DBUG_ENTER ("SATserializeTypeCheckingStatus");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeCompilerPhase
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeCompilerPhase (info *info, compiler_phase_t attr, node *parent)
{
    DBUG_ENTER ("SATserializeTypeCheckingStatus");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeMatrices
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeMatrices (info *info, matrix **attr, node *parent)
{
    DBUG_ENTER ("SATserializeMatrices");

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeMatrices
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 ***************************************************************************/

void
SATserializeOmpOP (info *info, omp_reduction_op attr, node *parent)
{
    DBUG_ENTER ("SATserializeOmpOP");

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_VOID_RETURN;
}
/**
 * @}
 */
