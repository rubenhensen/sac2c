
#include "cygwinhelpers.h"

#if IS_CYGWIN

#include <stdio.h>

/* includes needed for CYGHinitFunTable */
/* ######################## */
#include "str.h"
#include "serialize.h"
#include "serialize_helper.h"
#include "deserialize.h"
#include "shape.h"
#include "new_types.h"
#include "namespaces.h"
#include "constants.h"
#include "symboltable.h"
#include "libmanager.h"
#include "stringset.h"
/* ######################## */

#include "stringset.h"
#include "str.h"
#include "globals.h"
#include "memory.h"
#include "types.h"

#define DBUG_PREFIX "CYGH"
#include "debug.h"

extern node *SHLPmakeNode (int _node_type, int lineno, char *sfile, ...);

/** <!--********************************************************************-->
 *
 * @fn char *CYGHBuildLibSearchString( const char *lib,
 *                                  strstype_t kind,
 *                                  void *rest)
 *
 * @brief fold function to generate a string for -l for linking
 *        under cygwin. The suffix Mod is appended.
 *
 *****************************************************************************/
extern void *
CYGHbuildLibSearchString (const char *lib, strstype_t kind, void *rest)
{
    char *result;
    DBUG_ENTER ();

    /* only take STRS_saclib kinds */
    if ((rest != NULL) && (lib != NULL) && (kind == STRS_saclib)) {
        result = STRcatn (4, (char *)rest, " -l:lib", lib, "Mod.dll");
        rest = MEMfree (rest);
    } else if (lib != NULL && (kind == STRS_saclib)) {
        result = STRcatn (3, " -l:lib", lib, "Mod.dll");
    } else { /* rest != NULL */
        result = (char *)rest;
    }

    DBUG_RETURN ((void *)result);
}

/** <!--********************************************************************-->
 *
 * @fn char *CYGHBuildLibDirectoryString()
 *
 * @brief Builds a string of -L<libdir> from the global list of stdLib
 * directories. For use when linking under cygwin.
 *
 *****************************************************************************/
extern char *
CYGHbuildLibDirectoryString (void)
{
    char *result = "";
    char *ldDir = global.config.libpath;

    DBUG_ENTER ();

    ldDir = STRtok (ldDir, ":");
    while (STRlen (ldDir) != 0) {
        result = STRcatn (3, result, " -L", ldDir);
        ldDir = STRtok (NULL, ":");
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn char *CYGHAddToDeps()
 *
 * @brief adds a string to the global.dependencies stringset of kind STRS_saclib
 *        This is done so that the lib builder links against all discovered
 *        dependancies.
 *
 *****************************************************************************/
extern void
CYGHaddToDeps (const char *name)
{
    DBUG_ENTER ();

    global.dependencies = STRSadd (name, STRS_saclib, global.dependencies);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn char *CYGHgetCompleteLibString( void)
 *
 * @brief returns the dependancies for the linker plus the addition libsac
 *        dependancy.
 *
 *****************************************************************************/
extern char *
CYGHgetCompleteLibString (ldvariant_t type)
{
    char *result;
    stringset_t *deps = global.dependencies;

    DBUG_ENTER ();

    result = STRSfold (&CYGHbuildLibSearchString, deps, STRcpy (""));

    switch (type) {
    case CYGH_sac:
        result = STRcat (result, GetSacLibString ());
        break;
    case CYGH_sac2c:
        result = STRcat (result, GetSac2cLibString ());
        break;
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn char *GetSacLibString( void)
 *
 * @brief returns the option to link to the correct libsac lib.
 *
 *****************************************************************************/
const char *
GetSacLibString (void)
{
    char *result;
    DBUG_ENTER ();

    /*
     * TODO: needs to be tested and checked to ensure it is working.
     */
    switch (global.mtmode) {
    case MT_none:
        /* link to the seq library */
        result = " -l:libsac.seq.dll";
        break;
    default:
        /* link to the mt.pth library */
        result = " -l:libsac.mt.pth.dll";
        break;
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn char *GetSac2cLibString( void)
 *
 * @brief returns the option to link to the correct libsac2c lib.
 *
 *****************************************************************************/
inline const char *
GetSac2cLibString (void)
{
    DBUG_ENTER ();
    DBUG_RETURN (" -l:libcygcompat.a");
}

/** <!--********************************************************************-->
 *
 * @fn char *CYGHcygToWinPath( const char *name)
 *
 * @brief changes a cygwin absolute path to a windows absolute path.
 *
 *****************************************************************************/
const char *
CYGHcygToWinPath (const char *name)
{
    DBUG_ENTER ();

    if (STRprefix ("/cygdrive/", name)) {
        name = STRcat ("C:/", STRsubStr (name, 12, STRlen (name)));
    }

    DBUG_RETURN (name);
}

/** <!--********************************************************************-->
 *
 * @fn void CYGHinitFunTable( void)
 *
 * @brief sets up the function table ready to be passed to
 *        sac modules.
 *
 *****************************************************************************/
void
CYGHinitFunTable (void)
{
    DBUG_ENTER ();

    global.cyg_fun_table = malloc (sizeof (cyg_fun_table_t));
    cyg_fun_table_t *tab = global.cyg_fun_table;

    tab->STRcpy_fp = &STRcpy;
    tab->SHcreateShapeVa_fp = &SHcreateShapeVa;
    tab->TYdeserializeTypeVa_fp = &TYdeserializeTypeVa;
    tab->NSdeserializeNamespace_fp = &NSdeserializeNamespace;
    tab->NSaddMapping_fp = &NSaddMapping;
    tab->NSdeserializeView_fp = &NSdeserializeView;
    tab->SHLPmakeNodeVa_fp = &SHLPmakeNodeVa;
    tab->SHLPfixLink_fp = &SHLPfixLink;
    tab->SERbuildSerStack_fp = &SERbuildSerStack;
    tab->COdeserializeConstant_fp = &COdeserializeConstant;
    tab->DSlookupFunction_fp = &DSlookupFunction;
    tab->DSlookupObject_fp = &DSlookupObject;
    tab->DSfetchArgAvis_fp = &DSfetchArgAvis;
    tab->DShex2Double_fp = &DShex2Double;
    tab->DShex2Float_fp = &DShex2Float;
    tab->STinit_fp = &STinit;
    tab->STadd_fp = &STadd;
    tab->STRSadd_fp = &STRSadd;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void CYGHpassToMod( dynlib_t *lib)
 *
 * @brief gets the CYGFpassFunTable symbol from lib and calls it/
 *
 *****************************************************************************/
void
CYGHpassToMod (dynlib_t *lib)
{
    DBUG_ENTER ();

    if (global.cyg_fun_table == NULL) {
        CYGHinitFunTable ();
    }

    pass_fun_table_u fun;
    fun.v = LIBMgetLibraryFunction ("CYGFpassFunTable", lib);
    fun.f (global.cyg_fun_table);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
#endif /* IS_CYGWIN */
