/*
 *
 * $Log$
 * Revision 1.4  2004/09/24 20:21:53  sah
 * intermediate version
 *
 * Revision 1.3  2004/09/23 21:12:25  sah
 * ongoing implementation
 *
 * Revision 1.2  2004/09/21 16:34:27  sah
 * ongoing implementation of
 * serialize traversal
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
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

/** <!--******************************************************************-->
 *
 * @fn SerializeStringAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeStringAttrib (char *vname, info *info, char *attr, node *parent)
{
    DBUG_ENTER ("SerializeStringAttrib");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);
    } else {
        fprintf (INFO_SER_FILE (info), "%s = \"%s\";\n", vname, attr);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSharedStringAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSharedStringAttrib (char *vname, info *info, char *attr, node *parent)
{
    DBUG_ENTER ("SerializeSharedStringAttrib");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);
    } else {
        fprintf (INFO_SER_FILE (info), "%s = \"%s\";\n", vname, attr);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerAttrib (char *vname, info *info, int attr, node *parent)
{
    DBUG_ENTER ("SerializeIntegerAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %d;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeLongAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeLongAttrib (char *vname, info *info, long attr, node *parent)
{
    DBUG_ENTER ("SerializeLongAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %ld;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeBoolAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeBoolAttrib (char *vname, info *info, bool attr, node *parent)
{
    DBUG_ENTER ("SerializeBoolAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %d;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeFloatAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeFloatAttrib (char *vname, info *info, float attr, node *parent)
{
    DBUG_ENTER ("SerializeFloatAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %f;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDoubleAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDoubleAttrib (char *vname, info *info, double attr, node *parent)
{
    DBUG_ENTER ("SerializeDoubleAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %f;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeCharAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeCharAttrib (char *vname, info *info, char attr, node *parent)
{
    DBUG_ENTER ("SerializeCharAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %c;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeOldTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeOldTypeAttrib (char *vname, info *info, types *attr, node *parent)
{
    DBUG_ENTER ("SerializeOldTypeAttrib");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);
    } else {
        fprintf (INFO_SER_FILE (info), "%s = MakeTypes1( T_unknown);\n", vname);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNodeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNodeAttrib (char *vname, info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializeNodeAttrib");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);
    } else {
        Trav (attr, info);
        fprintf (INFO_SER_FILE (info), "%s = LOOKUP( %d);\n", vname,
                 SerStackFindPos (attr, INFO_SER_STACK (info)));
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeLinkAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeLinkAttrib (char *vname, info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializeLinkAttrib");

    fprintf (INFO_SER_FILE (info), "%s = ", vname);

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "NULL;\n");
    } else {
        int pos = SerStackFindPos (attr, INFO_SER_STACK (info));

        if (pos == SERSTACK_NOT_FOUND) {
            DBUG_PRINT ("SET", ("missing link on stack: [%s] -> [%s]",
                                mdb_nodetype[NODE_TYPE (parent)],
                                mdb_nodetype[NODE_TYPE (attr)]));

            fprintf (INFO_SER_FILE (info), "NULL;\n");
        } else {
            fprintf (INFO_SER_FILE (info), "LOOKUP( %d);\n", pos);
        }
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeApLinkAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeApLinkAttrib (char *vname, info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializeApLinkAttrib");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);
    } else {
        fprintf (INFO_SER_FILE (info),
                 "%s = SHLPLookupFunction( "
                 "\"%s\", \"%s\");\n",
                 vname, FUNDEF_MOD (attr), GenerateSerFunName (STE_funhead, attr));
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerArrayAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerArrayAttrib (char *vname, info *info, int *attr, node *parent)
{
    DBUG_ENTER ("SerializeIntegerArrayAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNumsAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNumsAttrib (char *vname, info *info, nums *attr, node *parent)
{
    DBUG_ENTER ("SerializeNumsAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeFileTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeFileTypeAttrib (char *vname, info *info, file_type attr, node *parent)
{
    DBUG_ENTER ("SerializeFileTypeAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %d;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeLUTAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeLUTAttrib (char *vname, info *info, LUT_t attr, node *parent)
{
    DBUG_ENTER ("SerializeLUTAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeWithOpTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeWithOpTypeAttrib (char *vname, info *info, WithOpType attr, node *parent)
{
    DBUG_ENTER ("SerializeWithOpTypeAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %d;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializePrfAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializePrfAttrib (char *vname, info *info, prf attr, node *parent)
{
    DBUG_ENTER ("SerializePrfAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %d;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeMaskAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param pos    position within the current array
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeMaskAttrib (char *vname, info *info, int pos, long *attr, node *parent)
{
    DBUG_ENTER ("SerializeMaskAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDepsAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDepsAttrib (char *vname, info *info, deps *attr, node *parent)
{
    DBUG_ENTER ("SerializeDepsAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIdsAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIdsAttrib (char *vname, info *info, ids *attr, node *parent)
{
    DBUG_ENTER ("SerializeIdsAttrib");

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);
    } else {
        char *tmpvar = TmpVar ();

        fprintf (INFO_SER_FILE (info), "{ /* serialize ids structure */\n");
        fprintf (INFO_SER_FILE (info), "ids *%s;\n", tmpvar);

        if (IDS_NEXT (attr) != NULL) {
            SerializeIdsAttrib (tmpvar, info, IDS_NEXT (attr), parent);
        } else {
            fprintf (INFO_SER_FILE (info), "%s = NULL;\n", tmpvar);
        }

        fprintf (INFO_SER_FILE (info), "%s = MakeIds( \"%s\", ", vname, IDS_NAME (attr));
        if (IDS_MOD (attr) == NULL) {
            fprintf (INFO_SER_FILE (info), "NULL");
        } else {
            fprintf (INFO_SER_FILE (info), "\"%s\"", IDS_MOD (attr));
        }
        fprintf (INFO_SER_FILE (info), ", %d);\n", IDS_STATUS (attr));

        fprintf (INFO_SER_FILE (info), "IDS_NEXT( %s) = %s;\n", vname, tmpvar);

        fprintf (INFO_SER_FILE (info), "}\n");

        tmpvar = Free (tmpvar);
    }

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeStatusTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeStatusTypeAttrib (char *vname, info *info, statustype attr, node *parent)
{
    DBUG_ENTER ("SerializeStatusTypeAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %d;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNodeListAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNodeListAttrib (char *vname, info *info, nodelist *attr, node *parent)
{
    DBUG_ENTER ("SerializeNodeListAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSharedNodeListAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSharedNodeListAttrib (char *vname, info *info, nodelist *attr, node *parent)
{
    DBUG_ENTER ("SerializeSharedNodeListAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializePragmaLinkAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializePragmaLinkAttrib (char *vname, info *info, node *attr, node *parent)
{
    DBUG_ENTER ("SerializePragmaLinkAttrib");

    fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDFMMaskAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDFMMaskAttrib (char *vname, info *info, DFMmask_t attr, node *parent)
{
    DBUG_ENTER ("SerializeDFMMaskAttrib");

    fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDFMMaskBaseAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDFMMaskBaseAttrib (char *vname, info *info, DFMmask_base_t attr, node *parent)
{
    DBUG_ENTER ("SerializeDFMMaskBaseAttrib");

    fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeDFMFoldMaskAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeDFMFoldMaskAttrib (char *vname, info *info, DFMfoldmask_t *attr, node *parent)
{
    DBUG_ENTER ("SerializeDFMFoldMaskAttrib");

    fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNewTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNewTypeAttrib (char *vname, info *info, ntype *attr, node *parent)
{
    DBUG_ENTER ("SerializeNewTypeAttrib");

    fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeArgTabAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeArgTabAttrib (char *vname, info *info, argtab_t *attr, node *parent)
{
    DBUG_ENTER ("SerializeArgTabAttrib");

    fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIndexPointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIndexPointerAttrib (char *vname, info *info, index_info *attr, node *parent)
{
    DBUG_ENTER ("SerializeIndexPointerAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeShapeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeShapeAttrib (char *vname, info *info, shape *attr, node *parent)
{
    DBUG_ENTER ("SerializeShapeAttrib");

    SHSerializeShape (INFO_SER_FILE (info), attr, vname);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSimpleTypeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSimpleTypeAttrib (char *vname, info *info, simpletype attr, node *parent)
{
    DBUG_ENTER ("SerializeSimpleTypeAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeConstVecPointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeConstVecPointerAttrib (char *vname, info *info, void *attr, node *parent)
{
    DBUG_ENTER ("SerializeConstVecPointerAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeUseFlagAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeUseFlagAttrib (char *vname, info *info, useflag attr, node *parent)
{
    DBUG_ENTER ("SerializeUseFlagAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %d;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeAccessInfoAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeAccessInfoAttrib (char *vname, info *info, access_info_t *attr, node *parent)
{
    DBUG_ENTER ("SerializeAccessInfoAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeShpSegAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeShpSegAttrib (char *vname, info *info, shpseg *attr, node *parent)
{
    DBUG_ENTER ("SerializeShpSegAttrib");

    fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerPointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerPointerAttrib (char *vname, info *info, int *attr, node *parent)
{
    DBUG_ENTER ("SerializeIntegerPointerAttrib");

    fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeIntegerPointerArrayAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param pos    position within the current array
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeIntegerPointerArrayAttrib (char *vname, info *info, int pos, int *attr,
                                    node *parent)
{
    DBUG_ENTER ("SerializeIntegerPointerArrayAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSchedulingAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSchedulingAttrib (char *vname, info *info, SCHsched_t attr, node *parent)
{
    DBUG_ENTER ("SerializeSchedulingAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeTaskSelAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeTaskSelAttrib (char *vname, info *info, SCHtasksel_t attr, node *parent)
{
    DBUG_ENTER ("SerializeTaskSelAttrib");
    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeNodePointerAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeNodePointerAttrib (char *vname, info *info, node **attr, node *parent)
{
    DBUG_ENTER ("SerializeNodePointerAttrib");

    fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeSSAPhiAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeSSAPhiAttrib (char *vname, info *info, ssaphit_t attr, node *parent)
{
    DBUG_ENTER ("SerializeSSAPhiAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %d;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeBitFieldAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeBitFieldAttrib (char *vname, info *info, int attr, node *parent)
{
    DBUG_ENTER ("SerializeBitFieldAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %d;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeConstantAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeConstantAttrib (char *vname, info *info, constant *attr, node *parent)
{
    DBUG_ENTER ("SerializeConstantAttrib");

    /* TODO */

    fprintf (INFO_SER_FILE (info), "%s = NULL;\n", vname);

    DBUG_VOID_RETURN;
}

/** <!--******************************************************************-->
 *
 * @fn SerializeMTExecModeAttrib
 *
 * @brief generates code to de-serialize the given attribute
 *
 * @param vname  name of the variable holding the result in the generated code
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SerializeMTExecModeAttrib (char *vname, info *info, mtexecmode_t attr, node *parent)
{
    DBUG_ENTER ("SerializeMTExecModeAttrib");

    fprintf (INFO_SER_FILE (info), "%s = %d;\n", vname, attr);

    DBUG_VOID_RETURN;
}

/**
 * @}
 */
