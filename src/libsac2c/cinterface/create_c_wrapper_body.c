/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup ccwb Create C Wrapper Body traversal
 *
 * Module description goes here.
 *
 * @ingroup ccwb
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file create_c_wrapper_body.c
 *
 * Prefix: CCWB
 *
 *****************************************************************************/
#include "create_c_wrapper_body.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "filemgr.h"
#include "globals.h"
#include "bundle_to_fundef.h"
#include "build.h"
#include "str.h"
#include "namespaces.h"
#include "tree_compound.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    FILE *file;
};

/**
 * The info structure
 */
#define INFO_FILE(n) ((n)->file)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FILE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}
/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn void PrintFileHeader(FILE *file);
 *
 * @brief Prints the header for the file to be generated.
 *
 *****************************************************************************/
static void
PrintFileHeader (info *arg_info)
{
    DBUG_ENTER ();

    fprintf (INFO_FILE (arg_info),
             "/*\n"
             " * C interface stub file.\n"
             " *\n"
             " * generated by sac4c %s (%s) rev %s\n"
             " */\n\n"
             "#include \"sacinterface.h\"\n"
             "#include \"header.h\"\n"
             "#include <assert.h>\n\n",
             global.version_id, build_style, build_rev);

    /*
     * descriptor alloc and free functions
     */
    fprintf (INFO_FILE (arg_info),
             "static SAC_array_descriptor_t makeScalarDesc()\n{\n"
             "  SAC_array_descriptor_t result \n"
             "    = (SAC_array_descriptor_t) SAC_MALLOC( BYTE_SIZE_OF_DESC( 0)); \n"
             "  SAC_DESC_INIT_RC( result, 1);\n"
             "  DESC_DIM( result) = 0;\n"
             "  DESC_SIZE( result) = 1;\n"
             "  return( result);\n"
             "}\n\n");

    fprintf (INFO_FILE (arg_info),
             "static void freeScalarDesc( SAC_array_descriptor_t desc)\n{\n"
             "  int q_waslast;\n"
             "  SAC_DESC_DEC_RC_FREE( desc, 1, q_waslast);\n"
             "  assert(q_waslast);   /* must have been the last; but the data we keep! "
             "*/\n"
             "}\n\n");

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *CCWBdoCreateCWrapperBody( node *syntax_tree)
 *
 *****************************************************************************/
node *
CCWBdoCreateCWrapperBody (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    INFO_FILE (info) = FMGRwriteOpen ("%s/interface.c", global.tmp_dirname);

    PrintFileHeader (info);

    TRAVpush (TR_ccwb);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    INFO_FILE (info) = FMGRclose (INFO_FILE (info));

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CCWBfunbundle(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CCWBfunbundle (node *arg_node, info *arg_info)
{
    int noargs;
    int norets;
    int pos;

    DBUG_ENTER ();

    noargs = FUNBUNDLE_ARITY (arg_node);
    norets = TCcountRets (FUNDEF_RETS (FUNBUNDLE_FUNDEF (arg_node)));

    /* we only use non-{x,s}t funbundle here and manually do a dispatch between
       the versions to call */
    if (!FUNBUNDLE_ISXTBUNDLE (arg_node) && !FUNBUNDLE_ISSTBUNDLE (arg_node)) {
        /*
         * function header
         */
        fprintf (INFO_FILE (arg_info), "void %s(", FUNBUNDLE_EXTNAME (arg_node));

        for (pos = 0; pos < norets; pos++) {
            fprintf (INFO_FILE (arg_info), "void **ret%d%s", pos,
                     (norets - pos + noargs > 1) ? ", " : "");
        }
        for (pos = 0; pos < noargs; pos++) {
            fprintf (INFO_FILE (arg_info), "void *arg%d%s", pos,
                     (noargs - pos > 1) ? ", " : "");
        }

        fprintf (INFO_FILE (arg_info), ")\n{\n");

        /*
         * allocate arg descriptors and declare ret descriptors
         */
        for (pos = 0; pos < noargs; pos++) {
            fprintf (INFO_FILE (arg_info),
                     "  SAC_array_descriptor_t arg%d_desc = makeScalarDesc();\n", pos);
        }
        for (pos = 0; pos < norets; pos++) {
            fprintf (INFO_FILE (arg_info), "  SAC_array_descriptor_t ret%d_desc;\n", pos);
        }

        /*
         * call SAC fun
         */
        if (global.mtmode != MT_none) {
            /* MT-mode: insert XT function */
            char *fun_name = FUNDEF_NAME (FUNBUNDLE_FUNDEF (arg_node));

            fprintf (INFO_FILE (arg_info),
                     "  struct sac_bee_common_t *self = SAC_MT_CurrentBee();\n"
                     "  SAChive *stub_hive = NULL;\n"
                     "  if (!self || !self->hive) {\n"
                     "    static int was_warned = 0;\n"
                     "    if (!was_warned) {\n"
                     "      SAC_RuntimeWarning (\"In %s: there was no hive attached to "
                     "the calling thread!\\n"
                     "    Created a temporary hive of one. The warning will not be "
                     "repeated for this function.\");\n"
                     "      was_warned = 1;\n"
                     "    }\n"
                     "    stub_hive = SAC_AllocHive(1, 2, NULL, NULL);\n"
                     "    SAC_AttachHive(stub_hive);\n"
                     "    self = SAC_MT_CurrentBee();\n"
                     "  }\n",
                     FUNBUNDLE_EXTNAME (arg_node));

            fprintf (INFO_FILE (arg_info), "  %s%s((void*)self, ", CWRAPPER_PREFIX,
                     STRsubstToken (FUNBUNDLE_EXTNAME (arg_node), STRcat ("__", fun_name),
                                    STRcat ("_CL_XT__", fun_name)));
        } else {
            /* SEQ-mode */
            fprintf (INFO_FILE (arg_info), "  %s%s(", CWRAPPER_PREFIX,
                     FUNBUNDLE_EXTNAME (arg_node));
        }

        /* print function's arguments */
        for (pos = 0; pos < norets; pos++) {
            fprintf (INFO_FILE (arg_info), "ret%d, &ret%d_desc%s", pos, pos,
                     (norets - pos + noargs > 1) ? ", " : "");
        }

        for (pos = 0; pos < noargs; pos++) {
            fprintf (INFO_FILE (arg_info), "arg%d, arg%d_desc%s", pos, pos,
                     (noargs - pos > 1) ? ", " : "");
        }

        fprintf (INFO_FILE (arg_info), ");\n");

        if (global.mtmode != MT_none) {
            fprintf (INFO_FILE (arg_info), "  if (stub_hive) {\n"
                                           "    stub_hive = SAC_DetachHive();\n"
                                           "    SAC_ReleaseHive(stub_hive);\n"
                                           "    SAC_ReleaseQueen();\n"
                                           "  }\n");
        }

        /*
         * free return descs
         */
        for (pos = 0; pos < norets; pos++) {
            fprintf (INFO_FILE (arg_info), "  freeScalarDesc( ret%d_desc);\n", pos);
        }

        fprintf (INFO_FILE (arg_info), "}\n\n");
    }

    if (FUNBUNDLE_NEXT (arg_node) != NULL) {
        FUNBUNDLE_NEXT (arg_node) = TRAVdo (FUNBUNDLE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Create C Wrapper Body template -->
 *****************************************************************************/

#undef DBUG_PREFIX
