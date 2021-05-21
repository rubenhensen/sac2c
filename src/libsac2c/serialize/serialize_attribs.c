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
#include "str_buffer.h"
#include "stringset.h"
#include "memory.h"
#include "new_types.h"
#include "shape.h"
#include "globals.h"
#include "constants.h"
#include "namespaces.h"

#define DBUG_PREFIX "SET"
#include "debug.h"

/** <!--******************************************************************-->
 *
 * @fn SATserializeIdagFun
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeIdagFun (info *info, idag_fun_t attr, node *parent)
{
    DBUG_ENTER ();

    DBUG_RETURN ();
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeDag
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeDag (info *info, dag *attr, node *parent)
{
    DBUG_ENTER ();

    DBUG_RETURN ();
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeVertex
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeVertex (info *info, vertex *attr, node *parent)
{
    DBUG_ENTER ();

    DBUG_RETURN ();
}

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
SATserializeString (info *info, const char *attr, node *parent)
{
    char *tmp;

    DBUG_ENTER ();

    if (attr == NULL) {
        DBUG_PRINT ("Processing String (null)");

        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        DBUG_PRINT ("Processing String `%s'", attr);

        tmp = STRstring2SafeCEncoding (attr);
        fprintf (INFO_SER_FILE (info), "STRcpy(\"%s\")", tmp);
        tmp = MEMfree (tmp);
    }

    DBUG_RETURN ();
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
SATserializeSharedString (info *info, const char *attr, node *parent)
{
    DBUG_ENTER ();

    SATserializeString (info, attr, parent);

    DBUG_RETURN ();
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeStringSet
 *
 * @brief generates code to serialize a StringSet attribute
 *
 * @param info   info structure of serialize traversal
 * @param strs   the string set
 * @param parent the parent node
 *
 ***************************************************************************/
void
SATserializeStringSet (info *info, stringset_t *strs, node *parent)
{
    str_buf *sbuf;

    DBUG_ENTER ();

    if (strs == NULL) {
        DBUG_PRINT ("Processing StringSet (null)");

        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        DBUG_PRINT ("Processing StringSet");

        sbuf = SBUFcreate (1024);
        sbuf = SBUFprintf (sbuf, "NULL");
        sbuf = STRSfold (&STRStoSafeCEncodedStringFold, strs, sbuf);
        fprintf (INFO_SER_FILE (info), "%s", SBUF2str (sbuf));
        sbuf = SBUFfree (sbuf);
    }

    DBUG_RETURN ();
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeSizet
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeSizet (info *info, size_t attr, node *parent)
{
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "ASSERT_TYPESIZE (size_t, (size_t)%zu)", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "ASSERT_TYPESIZE (int, %d)", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "ASSERT_TYPESIZE (long, %ldL)", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "ASSERT_TYPESIZE (long long, %lldLL)", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%u", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "ASSERT_TYPESIZE (unsigned int, %u)", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%u", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "ASSERT_TYPESIZE (unsigned long, %luUL)", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "ASSERT_TYPESIZE (unsigned long long, %lluULL)", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
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

    DBUG_ENTER ();

    data = STRbytes2Hex (sizeof (float), (unsigned char *)&attr);

    /**
     * on some systems, all floats in vararg position are implicitly casted
     * to double (eg on OSX 10.13). This may result in an "implicit cast"
     * warning. To avoid this we explicitly cast to a double and hope
     * that the assignment to a float will work out :-)
     */
    fprintf (INFO_SER_FILE (info), "(double)DShex2Float( \"%s\")", data);

    data = MEMfree (data);

    DBUG_RETURN ();
}

void
SATserializeFloatvec (info *info, floatvec attr, node *parent)
{
    char *data;

    DBUG_ENTER ();

    data = STRbytes2Hex (sizeof (floatvec), (unsigned char *)&attr);

    fprintf (INFO_SER_FILE (info), "DShex2Float( \"%s\")", data);

    data = MEMfree (data);

    DBUG_RETURN ();
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

    DBUG_ENTER ();

    data = STRbytes2Hex (sizeof (double), (unsigned char *)&attr);

    fprintf (INFO_SER_FILE (info), "DShex2Double( \"%s\")", data);

    data = MEMfree (data);

    DBUG_RETURN ();
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
SATserializeChar (info *info, char attr, node *parent)
{
    DBUG_ENTER ();
    
    /* 
     * Using %d instead of '%c' despite char as this 
     * '%c' introduces warnings and errors when '\n' is passed during stdlib-build
     * and are not contained leading to newlines being printed into actual code. 
     * Going forward, will serialize char as int and read later as int, 
     * as cannot introduce inconsistency here. 
     */
    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        /* we have to eat the additional , created by SET traversal */
        fprintf (INFO_SER_FILE (info), "DROP( x");
        TRAVdo (attr, info);
        fprintf (INFO_SER_FILE (info), ")");
    }

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    if (attr == NULL) {
        fprintf (INFO_SER_FILE (info), "NULL");
    } else {
        NSserializeNamespace (INFO_SER_FILE (info), attr);
    }

    DBUG_RETURN ();
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
    DBUG_ENTER ();

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

            DBUG_ASSERT (args != NULL, "found a link to an ARG_AVIS which does not "
                                       "belong to current fundef");

            fprintf (INFO_SER_FILE (info), "DSfetchArgAvis( %d)", pos);
        } else {
            fprintf (INFO_SER_FILE (info), "NULL");
        }
    } else {
        fprintf (INFO_SER_FILE (info), "NULL");
    }

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    if (attr != NULL) {
        switch (NODE_TYPE (attr)) {
        case N_fundef:
            SERserializeFundefLink (attr, INFO_SER_FILE (info));
            break;
        case N_objdef:
            SERserializeObjdefLink (attr, INFO_SER_FILE (info));
            break;
        default:
            DBUG_UNREACHABLE ("unknown target for ExtLink found!");
            fprintf (INFO_SER_FILE (info), "NULL");
            break;
        }
    } else {
        fprintf (INFO_SER_FILE (info), "NULL");
    }

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    /*
     * links to N_code nodes are serialized and deserialized
     * by the serialize traversal, so nothing has to be done
     * here
     */

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    DBUG_PRINT ("Starting traversal for ntype attribute");

    TYserializeType (INFO_SER_FILE (info), attr);

    DBUG_PRINT ("Finished traversal for ntype attribute");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    SHserializeShape (INFO_SER_FILE (info), attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    COserializeConstant (INFO_SER_FILE (info), attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeCudaExecMode
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeCudaExecMode (info *info, cudaexecmode_t attr, node *parent)
{
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeCompInfo
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeCompInfo (info *info, compinfo *attr, node *parent)
{
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
}

/** <!--******************************************************************-->
 *
 * @fn SATserializeVertexWrapper
 *
 * @brief generates code to serialize the given attribute
 *
 * @param info   info structure of serialize traversal
 * @param attr   the attribute itself
 * @param parent the parent node
 *
 ***************************************************************************/

void
SATserializeVertexWrapper (info *info, vertex *attr, node *parent)
{
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "NULL");

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "%d", attr);

    DBUG_RETURN ();
}
/**
 * @}
 */

#undef DBUG_PREFIX
