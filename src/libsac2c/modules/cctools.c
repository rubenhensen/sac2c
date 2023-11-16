/*******************************************************************************
 *
 * This file is needed to help the orchestration of C compiler calls (cc) and
 * linker calls (ld)!
 *
 * The challenge of orchestrating these lies in the huge range of variability
 * that stems from:
 *
 * a) The environment sac2c runs in as investigated by cmake
 * b) The target that the user is using
 * c) Potential cutomisation through sac2c flags
 *
 * At first glance, it may seem to be a rather straight-forward problem.
 * cmake adds environmental info into sac2crc such as 
 *
 * CC, CFLAGS, LD, etc.
 *
 * We can take these as default values for global variables and possibly
 * overwrite them by sac2c flags when instructed by the user to do so.
 *
 * But then, it turns out that different tools require slightly 
 * different "pattern" in their invocation such as the way to tell the 
 * linker where to search for dynamic libraries. This implies that fixed
 * entries in sac2crc are no longer good enough.
 * To deal with this situation, sac2crc supports the concept of 
 * substitutions. The idea is that resource entries can contain a fixed
 * set of substrings of the form %xyz% which are replaced by sac2c whenever
 * a concrete compilation happens. 
 * The exact set of these substitutions is described in sac2crc.
 *
 * This file is taking care of those substitutions whenever they are 
 * being needed for actually invoking cc / ld. Concretely, these are:
 *
 *   %cc%
 *   %ld%
 *   %opt%
 *   %dbg%
 *   %cuda_arch%
 *   %sacincludes%
 *   %tree_cflags%
 *   %tree_ldflags%
 *   %cppflags%
 *   %cflags%
 *   %compileflags%
 *   %extlibdirs%
 *   %modlibdirs%
 *   %modlibs%
 *   %saclibs%
 *   %libs%
 *   %ldflags%
 *   %linkflags%
 *   %path%
 *   %target%
 *   %libname%
 *   %objects%
 *   %source%
 *
 * Most of these have a "fixed" replacement, where the replacement
 * just depends on on certain sac2crc variables, certain global
 * variables from the command line or a combination thereof.
 * For all that require some "assembly" such as joining sac2crc
 * info and global variables together, we provide a set of local
 * functions named ReturnXYZ () which allocate a substitution 
 * string. Two of these are being made publicly available as
 * sac4c relies on them. These are:
 *
 *         char *CCTreturnCompileFlags ()  for %compileflags%
 *         char *CCTreturnLinkFlags ()     for %linkflags%
 *
 * The actual source and target files are task dependent and
 * their replacements are constructed in the actual task 
 * performing operations. These are:
 *
 *    void CCTpreprocessCompileAndLink (void)  used by sac2c
 *    void CCTcompileOnly (void)  used by saccc (script that calls sac2c -cc...)
 *    void CCTlinkOnly (void)     used by saccc (script that calls sac2c -cc...)
 *
 * These three functions put together system calls for preprocessing,
 * compiling, and linking files, building on one static function for
 * each of these tasks:
 *
 *    void RunCpp (const char *command, const char *source_dir, const char *source,
 *                 const char *libname_subst)
 *
 *    void RunCc (const char *command, const char *source_dir, const char *source,
 *                const char *libname_subst, const char *source_ext, const char *obj_ext,
 *                str_buf *objs_buf)
 *
 *     void RunLd (const char *command, const char *path_subst, const char *objects_subst,
 *                 const char *target_subst, const char *libname_subst)
 *
 * which, in turn, are small wrappers around the actual substitution 
 * and execution function:
 *
 *     void SubstituteAndRun (const char *command,
 *                            const char *path_subst, const char *source_subst,
 *                            const char *objects_subst, const char *target_subst,
 *                            const char *libname_subst)
 */
#include "config.h"
#include "cctools.h"
#include "system.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"

#define DBUG_PREFIX "CCT"
#include "debug.h"

#include "ctinfo.h"
#include "globals.h"
#include "filemgr.h"
#include "resource.h"
#include "stringset.h"

#include <limits.h>

/*******************************************************************************
 * To make sure that we do not use the same rpath specification in linker
 * calls, we need a mechanism for tracking which paths we have already added.
 * We do this through a static global array of strings which we clear 
 * whenever we start a linking process.
 * The following interface functions are provided here:
 *
 * void InitPathList ()                which resets the path list, deleting all
 *                                     paths contained and pot. allocates the
 *                                     path list
 * void AddToPathList (char *path)     which adds path to the path list
 *                                     and potentially enlarges the list
 * bool PathListContains (char *path)  which checks whether path is 
 *                                     contained in the path list
 * void DeletePathList ()              which frees the path list and all
 *                                     the paths conatined in it
 */

static
char **path_list = NULL;
static
size_t path_list_num_entries = 0;
static
size_t path_list_sz = 16;

/*******************************************************************************
 *
 * @fn void InitPathList ()
 *
 * @brief initialises the global path-list, potentially allocating it and
 *        resetting the num_entries counter to 0.
 *
 *****************************************************************************/
static void
InitPathList (void)
{
    size_t i;

    DBUG_ENTER ();
    if (path_list == NULL)
        path_list = MEMmalloc (sizeof (char *) * path_list_sz);
    for (i=0; i<path_list_num_entries; i++)
        path_list[i] = MEMfree (path_list[i]);
    for (; i<path_list_sz; i++)
        path_list[i] = NULL;
    path_list_num_entries = 0;
    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * @fn void AddToPathList (char *path)
 *
 * @brief put path into the the path_list, increment the num_entries counter
 *        and increase the path-list length if needed.
 *
 *****************************************************************************/
static void
AddToPathList (char *path)
{
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (path_list != NULL, "called AddToPathList without InitPathList");

    path_list[path_list_num_entries] = path;
    path_list_num_entries += 1;
    if (path_list_num_entries == path_list_sz) {
        path_list_sz += 16;
        path_list = MEMrealloc (path_list, sizeof (char *) * path_list_sz);
        for (i=path_list_num_entries; i<path_list_sz; i++)
            path_list[i] = NULL;
    }
    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * @fn void PathListContains (char *path)
 *
 * @brief checks whether path is in path_list (deep comparison!)
 *
 *****************************************************************************/
static bool
PathListContains (char *path)
{
    size_t i = 0;
    bool found = false;

    DBUG_ENTER ();
    while ((i < path_list_num_entries) && (found == false)) {
        found = STReq (path, path_list[i]);
        i++;
    }
    DBUG_RETURN (found);
}

/*******************************************************************************
 *
 * @fn void DeletePathList (void)
 *
 * @brief deletes all paths in the path list and the path-list itself
 *
 *****************************************************************************/
static void
DeletePathList (void)
{
    size_t i;

    DBUG_ENTER ();
    for (i=0; i<path_list_num_entries; i++)
        path_list[i] = MEMfree (path_list[i]);
    path_list = MEMfree (path_list);
    DBUG_RETURN ();
}




/*******************************************************************************
 *
 * Next are a few static helper functions for adding paths for includes,
 * external libraries, and sac libraries:
 *
 * str_buf *AddIncPath (const char *path, str_buf *buf),
 * str_buf *AddLibPath (const char *path, str_buf *buf)
 * str_buf *AddModLibPath (const char *path, str_buf *buf)
 *
 * The latter two implicitly make use of the path_list and expect it to be 
 * properly initialised.
 *
 * Furthermore, we have two functions for adding object dependencies:
 *
 * str_buf *AddLibDependency (const char *lib, strstype_t kind, str_buf *buf)
 * str_buf *AddObjDependency (const char *lib, strstype_t kind, str_buf *buf)
 */

/*******************************************************************************
 *
 * @fn void *AddIncPath (const char *path, void *buf)
 *
 * @brief adds path to the str_buf *buf, which is passed and returned as void *
 *        to enable generic mapping of this function
 *
 *****************************************************************************/
static void *
AddIncPath (const char *path, void *buf)
{
    DBUG_ENTER ();

    str_buf *sbuf = (str_buf *)buf;
    sbuf = SBUFprintf (sbuf, " -I%s", path);

    DBUG_RETURN (buf);
}

/*******************************************************************************
 *
 * @fn void *AddLibPath (const char *path, void *buf)
 *
 * @brief adds path to the str_buf *buf, which is passed and returned as void *
 *        to enable generic mapping of this function
 *
 *****************************************************************************/
static void *
AddLibPath (const char *path, void *buf)
{
    DBUG_ENTER ();

    str_buf *sbuf = (str_buf *)buf;

    /* make path absolute as the linker can get confused otherwise. */
    char *abspath = FMGRabsName (path);

    /* only include the path if the the path does actually exist and avoid duplicates!
     * This is needed for picky linkers that issue warnings for paths that do not exist
     * or that are declared multiple times such as OSX's clang.
     */
    if (FMGRcheckExistDir (abspath) && !PathListContains (abspath)) {
        char *str = STRsubstToken (global.config.ldpath, "%path%", abspath);
        sbuf = SBUFprintf (sbuf, " %s", str);
        str = MEMfree (str);
        AddToPathList (abspath);
    } else {
        abspath = MEMfree (abspath);
    }

    DBUG_RETURN (buf);
}

/*******************************************************************************
 *
 * @fn void *AddModLibPath (const char *path, void *buf)
 *
 * @brief adds path to the str_buf *buf, which is passed and returned as void *
 *        to enable generic mapping of this function
 *
 *****************************************************************************/
static void *
AddModLibPath (const char *path, void *buf)
{
    DBUG_ENTER ();

    str_buf *sbuf = (str_buf *)buf;
    char *fpath
      = STRcatn (5, path, "/", global.config.target_env, "/", global.config.sbi);

    /* make path absolute as the linker can get confused otherwise. */
    char *absfpath = FMGRabsName (fpath);

    /* only include the path if the the path does actually exist and avoid duplicates!
     * This is needed for picky linkers that issue warnings for paths that do not exist
     * or that are declared multiple times such as OSX's clang.
     */
    if (FMGRcheckExistDir (absfpath) && !PathListContains (absfpath)) {
        char *str = STRsubstToken (global.config.ldpath, "%path%", absfpath);
        sbuf = SBUFprintf (sbuf, " %s", str);
        str = MEMfree (str);
        AddToPathList (absfpath);
    } else {
        absfpath = MEMfree (absfpath);
    }

    fpath = MEMfree (fpath);

    DBUG_RETURN (buf);
}

/*******************************************************************************
 *
 * @fn void *AddLibDependency (const char *lib, strstype_t kind, void *buf)
 *
 * @brief adds lib to the str_buf *buf, which is passed and returned as void *
 *        to enable generic mapping of this function
 *
 *****************************************************************************/
static void *
AddLibDependency (const char *lib, strstype_t kind, void *buf)
{
    DBUG_ENTER ();

    str_buf *sbuf = (str_buf *)buf;

    switch (kind) {
    case STRS_saclib:
        SBUFprintf (sbuf, " -l%sMod", lib);
        break;
    case STRS_extlib:
        SBUFprintf (sbuf, " -l%s", lib);
        break;
    default:
        break;
    }

    DBUG_RETURN (buf);
}

/*******************************************************************************
 *
 * @fn void *AddObjDependency (const char *lib, strstype_t kind, void *buf)
 *
 * @brief adds lib to the str_buf *buf, which is passed and returned as void *
 *        to enable generic mapping of this function
 *
 *****************************************************************************/
static void *
AddObjDependency (const char *lib, strstype_t kind, void *buf)
{
    DBUG_ENTER ();

    char *rpath = NULL;
    char *src_dirname = NULL;
    str_buf *sbuf = (str_buf *)buf;

    switch (kind) {
    case STRS_objfile:

        if (FMGRcheckExistFile (lib)) {
            CTInote (EMPTY_LOC, "External object %s picked from current directory.", lib);
            SBUFprintf (sbuf, " %s", lib);
            break;
        }

        if (lib[0] != '/') {
            rpath = STRcatn (3, global.targetdir, "/", lib);

            if (FMGRcheckExistFile (rpath)) {
                CTInote (EMPTY_LOC, "External object %s picked from build target directory (%s)",
                         lib, rpath);
                SBUFprintf (sbuf, " %s", rpath);
                break;
            }

            if (global.target_modlibdir != NULL) {
                rpath = MEMfree (rpath);
                rpath = STRcatn (3, global.target_modlibdir, "/", lib);
                if (FMGRcheckExistFile (rpath)) {
                    CTInote (EMPTY_LOC,
                      "External object %s picked from module target directory (%s)", lib,
                      rpath);
                    SBUFprintf (sbuf, " %s", rpath);
                    break;
                }
            }

            src_dirname = FMGRdirname (global.sacfilename);
            rpath = MEMfree (rpath);
            rpath = STRcatn (3, src_dirname, "/", lib);
            if (FMGRcheckExistFile (rpath)) {
                CTInote (EMPTY_LOC, "External object %s picked from source directory (%s)", lib,
                         rpath);
                SBUFprintf (sbuf, " %s", rpath);
                break;
            }
        }

        CTIerror (EMPTY_LOC, "Unable to find external object: %s", lib);

        break;
    default:
        break;
    }

    rpath = MEMfree (rpath);
    src_dirname = MEMfree (src_dirname);

    DBUG_RETURN (buf);
}




/*******************************************************************************
 *
 * static functions for constructing substitution strings. Most of them
 * return dynamically allocated strings; those that return pointers to statically
 * allocated ones are marked as const!
 * Those that would simply return a single global variable, such as CC or LD,
 * do not exist! For these use the global strings directly!
 * We mark those that are filtered through path_list by PATH_LIST.
 *
 * Overall, we have:
 *         char *ReturnOptFlags ()         for %opt%
 *   const char *ReturnDebugFlags ()       for %dbg%
 *         char *ReturnCppFlags ()         for %cppflags%
 *         char *ReturnCFlags ()           for %cflags%
 *         char *ReturnLdFlags ()          for %ldflags%
 *         char *ReturnModlibDirs ()       for %modlibdirs%     PATH_LIST
 *         char *ReturnModlibs ()          for %modlibs%
 *         char *ReturnExtlibDirs ()       for %extlibdirs%     PATH_LIST
 *         char *ReturnSaclibs ()          for %saclibs%
 *         char *ReturnCudaArchFlags ()    for %cuda_arch%
 *         char *ReturnCompileFlags ()     for %compileflags%
 *         char *ReturnLinkFlags ()        for %linkflags%      PATH_LIST
 */

static char *
ReturnOptFlags (void)
{
    DBUG_ENTER ();

    const char *p_opt_subst = "";
    switch (global.cc_optimize) {
    case 0:
        p_opt_subst = global.config.opt_o0;
        break;
    case 1:
        p_opt_subst = global.config.opt_o1;
        break;
    case 2:
        p_opt_subst = global.config.opt_o2;
        break;
    case 3:
        p_opt_subst = global.config.opt_o3;
        break;
    }

    // concat the tune flags
    char *opt_subst = global.cc_tune_generic
                      ? STRcatn (3, p_opt_subst, " ", global.config.tune_generic)
                      : STRcatn (3, p_opt_subst, " ", global.config.tune_native);
    DBUG_RETURN (opt_subst);
}

static const char *
ReturnDebugFlags (void)
{
    DBUG_ENTER ();
    DBUG_RETURN (global.cc_debug ? global.config.opt_g : "");
}

static char *
ReturnCppFlags (void)
{
    str_buf *cppflags_buf;
    char *cppflags_subst;
    
    DBUG_ENTER ();

    cppflags_buf = SBUFcreate (1);
    FMGRmapPath (PK_inc_path, AddIncPath, cppflags_buf);
    SBUFprintf (cppflags_buf, " %s", global.cppflags);
    SBUFprintf (cppflags_buf, " %s", global.config.sacincludes);
    SBUFprintf (cppflags_buf,
                " -DSAC_TARGET_%s"
                " -DSAC_TARGET_STRING=\\\"%s\\\""
                " -DSAC_MODEXT_STRING=\\\"%s\\\""
                " -DSAC_TARGET_ENV_STRING=\\\"%s\\\""
                " -DSAC_SBI_STRING=\\\"%s\\\""
                " -DSAC_RC_METHOD=SAC_RCM_%s"
                " -DSAC_BACKEND_%s"
                " -DSAC_MT_LIB_%s"
                " -DSAC_MT_MODE=%d"
                " -DSAC_DO_RTSPEC=%d"
                " -DSAC_DO_CUDA_ALLOC=SAC_CA_%s"
                " -DSAC_DO_CUDA_SYNC=%d",
                global.target_name,
                global.target_name, global.config.modext, global.config.target_env,
                global.config.sbi, global.config.rc_method,
                global.backend_string[global.backend], global.config.mt_lib,
                global.config.mt_mode, global.config.rtspec, global.config.cuda_alloc,
                global.cuda_async_mode);
    cppflags_subst = SBUF2strAndFree (&cppflags_buf);

    DBUG_RETURN (cppflags_subst);
}

static char *
ReturnCFlags (void)
{
    DBUG_ENTER ();
    DBUG_RETURN (STRcatn (3, global.config.cflags, " ", global.cflags));
}

static char *
ReturnLdFlags (void)
{
    DBUG_ENTER ();
    DBUG_RETURN (STRcatn (3, global.config.ldflags, " ", global.ldflags));
}

static char *
ReturnModlibDirs (void)
{
    str_buf *modlibdirs_buf;
    char *modlibdirs_subst;

    DBUG_ENTER ();

    modlibdirs_buf = SBUFcreate (1);
    FMGRmapPath (PK_lib_path, AddModLibPath, modlibdirs_buf);
    modlibdirs_subst = SBUF2strAndFree (&modlibdirs_buf);

    DBUG_RETURN (modlibdirs_subst);
}

static char *
ReturnModlibs (void)
{
    str_buf *modlibs_buf;
    char *modlibs_subst;

    DBUG_ENTER ();

    modlibs_buf = SBUFcreate (1);
    STRSfold (AddLibDependency, global.dependencies, modlibs_buf);
    modlibs_subst = SBUF2strAndFree (&modlibs_buf);

    DBUG_RETURN (modlibs_subst);
}

static char *
ReturnExtlibDirs (void)
{
    str_buf *extlibdirs_buf;
    char *extlibdirs_subst;

    DBUG_ENTER ();

    extlibdirs_buf = SBUFcreate (1);
    FMGRmapPath (PK_extlib_path, AddLibPath, extlibdirs_buf);
    extlibdirs_subst = SBUF2strAndFree (&extlibdirs_buf);

    DBUG_RETURN (extlibdirs_subst);
}

static char *
ReturnSaclibs (void)
{
    char *saclibs_subst;

    DBUG_ENTER ();

    if (global.loadsaclibs) {
        saclibs_subst
          = STRcatn (4,
                     " -lsac" BUILD_TYPE_POSTFIX " "
#if ENABLE_DISTMEM
                     " -lsacdistmem" BUILD_TYPE_POSTFIX " "
#endif
                     " -lsacphm",
                     global.optimize.dophm ? "" : "c",
                     global.runtimecheck.heap ? ".diag" BUILD_TYPE_POSTFIX " "
                                              : BUILD_TYPE_POSTFIX " ",
                     global.config.rtspec ? "-lsacrtspec" BUILD_TYPE_POSTFIX " " : "");
    } else {
        saclibs_subst = STRcpy ("");
    }

    DBUG_RETURN (saclibs_subst);
}

static char *
ReturnCudaArchFlags (void)
{
    DBUG_ENTER ();
    DBUG_RETURN (STRcat ("-arch=", global.config.cuda_arch));
}

static char *
ReturnCompileFlags (void)
{
    char *opt_subst;
    const char *dbg_subst;
    char *cflags_subst;
    char *compileflags_subst;

    DBUG_ENTER ();

    opt_subst = ReturnOptFlags ();
    dbg_subst = ReturnDebugFlags ();
    cflags_subst = ReturnCFlags ();
    compileflags_subst = STRcatn (5, opt_subst, " ", dbg_subst, " ", cflags_subst);

    MEMfree (opt_subst);
    MEMfree (cflags_subst);

    DBUG_RETURN (compileflags_subst);
}

static char *
ReturnLinkFlags (void)
{
    char *ldflags_subst;
    char *modlibdirs_subst;
    char *modlibs_subst;
    char *extlibdirs_subst;
    char *saclibs_subst;
    const char *libs_subst;
    char *linkflags_subst;



    DBUG_ENTER ();

    ldflags_subst = ReturnLdFlags ();
    modlibdirs_subst = ReturnModlibDirs ();
    modlibs_subst = ReturnModlibs ();
    extlibdirs_subst = ReturnExtlibDirs ();
    saclibs_subst = ReturnSaclibs ();
    libs_subst = global.config.libs;

    linkflags_subst = STRcatn (11, ldflags_subst, " ", modlibdirs_subst, " ",
                                   modlibs_subst, " ", extlibdirs_subst, " ",
                                   saclibs_subst, " ", libs_subst);

    MEMfree (ldflags_subst);
    MEMfree (modlibdirs_subst);
    MEMfree (modlibs_subst);
    MEMfree (extlibdirs_subst);
    MEMfree (saclibs_subst);

    DBUG_RETURN (linkflags_subst);
}

/*******************************************************************************
 *
 * Now, we come to the actual substitution & execution functions.
 * The central function is SubstituteAndRun. It does most of the heavy lifting.
 * It builds all substitutions other than those for
 *   %path%, %source%, %objects%, %target%, and %libname%
 * which need to be provided as arguments. It also expects the surrounding
 * functions to have initialised the global path_list which is being used/
 * relied on when building the libdir substitutions (see above).
 *
 * Based on this abstraction, we have three further static execution functions
 * for preprocessing, compiling, and linking: RunCpp, RunCc RunLd.
 * All these four static functions are being used by the exported functions
 * further below.
 */

/*******************************************************************************
 *
 * @fn void SubstituteAndRun (const char *command, const char *path_subst,
 *                            const char *source_subst, const char *objects_subst,
 *                            const char *target_subst, const char *libname_subst)
 *
 * @brief central function for running the compiler tools
 *
 *****************************************************************************/
static void
SubstituteAndRun (const char *command, const char *path_subst, const char *source_subst,
                                       const char *objects_subst, const char *target_subst,
                                       const char *libname_subst)
{
    char *cmd;

    DBUG_ENTER ();

    char *opt_subst = ReturnOptFlags ();
    char *cuda_arch_subst = ReturnCudaArchFlags ();
    char *cppflags_subst = ReturnCppFlags ();
    char *cflags_subst = ReturnCFlags ();
    char *compileflags_subst = ReturnCompileFlags ();
    char *modlibs_subst = ReturnModlibs ();
    char *saclibs_subst = ReturnSaclibs ();
    char *ldflags_subst = ReturnLdFlags ();
    char *extlibdirs_subst = ReturnExtlibDirs ();
    char *modlibdirs_subst = ReturnModlibDirs ();

    // we MUST NOT use ReturnLinkFlags here as this would find the path_list filled!
    char *linkflags_subst = STRcatn (11, ldflags_subst, " ", modlibdirs_subst, " ",
                                         modlibs_subst, " ", extlibdirs_subst, " ",
                                         saclibs_subst, " ", global.config.libs);

    char *san_source_subst = SYSsanitizePath (source_subst);
    char *san_target_subst = SYSsanitizePath (target_subst);
    
    cmd = STRsubstTokens (command, 23, "%cc%", global.config.cc,
                                       "%ld%", global.config.ld,
                                       "%opt%", opt_subst,
                                       "%dbg%", ReturnDebugFlags (),
                                       "%cuda_arch%", cuda_arch_subst,
                                       "%sacincludes%", global.config.sacincludes,
                                       "%tree_cflags%", global.tree_cflags,
                                       "%cppflags%", cppflags_subst,
                                       "%cflags%", cflags_subst,
                                       "%compileflags%", compileflags_subst,
                                       "%extlibdirs%", extlibdirs_subst,
                                       "%modlibdirs%", modlibdirs_subst,
                                       "%modlibs%", modlibs_subst,
                                       "%saclibs%", saclibs_subst,
                                       "%libs%", global.config.libs,
                                       "%ldflags%", ldflags_subst,
                                       "%tree_ldflags%", global.tree_ldflags,
                                       "%linkflags%", linkflags_subst,
                                       "%path%", path_subst,
                                       "%target%", san_target_subst,
                                       "%libname%", libname_subst,
                                       "%objects%", objects_subst,
                                       "%source%", san_source_subst);
    SYScall ("%s", cmd);

    MEMfree (cmd);
    MEMfree (opt_subst);
    MEMfree (cuda_arch_subst);
    MEMfree (cppflags_subst);
    MEMfree (cflags_subst);
    MEMfree (compileflags_subst);
    MEMfree (extlibdirs_subst);
    MEMfree (modlibdirs_subst);
    MEMfree (modlibs_subst);
    MEMfree (saclibs_subst);
    MEMfree (ldflags_subst);
    MEMfree (linkflags_subst);
    MEMfree (san_source_subst);
    MEMfree (san_target_subst);

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * @fn void RunCpp (const char *command, const char *source_dir,
 *                  const char *source, const char *libname_subst)
 *
 * @brief central tool for calling cpp
 *
 *****************************************************************************/
static void
RunCpp (const char *command, const char *source_dir, const char *source,
        const char *libname_subst)
{
    const char *path_subst = "";
    const char *objects_subst = "";
    char *source_subst;
    char *target_subst;
 
    DBUG_ENTER ();

    source_subst = STRcatn (4, source_dir, "/", source, global.config.ccp_cext);
    target_subst = STRcatn (4, source_dir, "/", source, global.config.ccp_objext);

    CTInote (EMPTY_LOC, "Preprocessing C source \"%s%s\"", source, global.config.ccp_cext);

    InitPathList ();

    SubstituteAndRun (command, path_subst, source_subst,
                               objects_subst, target_subst, libname_subst);

    source_subst = MEMfree (source_subst);
    target_subst = MEMfree (target_subst);

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * @fn void RunCc (const char *command, const char *source_dir,
 *                 const char *source, const char *libname_subst,
 *                 const char *source_ext, const char *obj_ext, str_buf *objs_buf)
 *
 * @brief central tool for calling cc
 *
 *****************************************************************************/
static void
RunCc (const char *command, const char *source_dir, const char *source,
       const char *libname_subst, const char *source_ext, const char *obj_ext,
       str_buf *objs_buf)
{
    const char *path_subst = global.tmp_dirname;
    const char *objects_subst = "";

    DBUG_ENTER ();
    char *source_subst;
    char *target_subst;

    source_subst = STRcatn (4, source_dir, "/", source, source_ext);
    target_subst = STRcatn (4, source_dir, "/", source, obj_ext);

    SBUFprintf (objs_buf, " %s", target_subst);
  
    CTInote (EMPTY_LOC, "Compiling C source \"%s%s\"", source, source_ext);

    InitPathList ();

    SubstituteAndRun (command, path_subst, source_subst,
                               objects_subst, target_subst, libname_subst);

    source_subst = MEMfree (source_subst);
    target_subst = MEMfree (target_subst);

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * @fn void RunLd (const char *command, const char *path_subst,
 *                 const char *objects_subst, const char *target_subst,
 *                 const char *libname_subst)
 *
 * @brief central tool for calling ld
 *
 *****************************************************************************/
static void
RunLd (const char *command, const char *path_subst, const char *objects_subst,
       const char *target_subst, const char *libname_subst)
{
    DBUG_ENTER ();

    CTInote (EMPTY_LOC, "Linking \"%s\"", target_subst);

    InitPathList ();
    AddToPathList (STRcpy (path_subst));

    SubstituteAndRun (command, path_subst, "", objects_subst, target_subst, libname_subst);

    DBUG_RETURN ();
}




/*******************************************************************************
 *
 * Finally, we have the exported API which builds on the static functions
 * above:
 */



/*******************************************************************************
 *
 * @fn char *CCTreturnCompileFlags (void)
 *
 * @brief AFAIK this is called from sac4c only; a simple wrapper around 
 *        ReturnCompileFlags.
 *
 *****************************************************************************/
char *
CCTreturnCompileFlags (void)
{
    DBUG_ENTER ();
    DBUG_RETURN (ReturnCompileFlags ());
}

/*******************************************************************************
 *
 * @fn char *CCTreturnLinkFlags (void)
 *
 * @brief AFAIK this is called from sac4c only; a simple wrapper around
 *        ReturnLinkFlags. Caveat: here we need to initialise the path_list!
 *
 *****************************************************************************/
char *
CCTreturnLinkFlags (void)
{
    char *res;
    DBUG_ENTER ();

    InitPathList ();
    res = ReturnLinkFlags ();
    DeletePathList ();

    DBUG_RETURN (res);
}

/*******************************************************************************
 *
 * @fn node *CCTpreprocessCompileAndLink (node *syntax_tree)
 *
 * @brief This is the central compilation function; depending on whether this
 *        is a sac program (with main) or a module two rather different
 *        compilation processes are being needed. For better code readability,
 *        these have been lifted into two static functions:
 *        HandleProgram and HandleModule
 *        The argument is returned unmodified; it is only needed for conformity
 *        of the traversal mechanism.
 *
 *****************************************************************************/
static void
HandleProgram (void)
{
    str_buf *objs_buf;
    char *objects_subst;
    char *target_subst;

    DBUG_ENTER ();

    objs_buf = SBUFcreate (1);

    RunCpp (global.config.ccp_prog, global.targetdir, global.outfilename, "");
    RunCc (global.config.compile_prog, global.targetdir, global.outfilename, "",
           global.config.cext, global.config.objext, objs_buf);

    // Link phase: append the object dependencies to objs_buf,
    // Then proceed to link command.

    STRSfold (AddObjDependency, global.dependencies, objs_buf);

    objects_subst = SBUF2strAndFree (&objs_buf);
    target_subst = STRcat (global.outfilename, global.config.exeext);

    RunLd (global.config.link_prog, global.targetdir, objects_subst, target_subst, "");

    MEMfree (objects_subst);
    MEMfree (target_subst);

    DBUG_RETURN ();
}

static void
HandleModule (void)
{
    str_buf *objs_buf;
    str_buf *tree_objs_buf;
    str_buf *path_buf;
    char *objects_subst;
    char *tree_objects_subst;
    char *target_subst;
    char *path_subst;
    int i;
    char *libname_subst;
    char *source;

    DBUG_ENTER ();

    source = MEMmalloc (20);
    objs_buf = SBUFcreate (1);
    tree_objs_buf = SBUFcreate (1);


    if (global.filetype == FT_cmod) {
        libname_subst = STRcat ("lib", global.outfilename);
    } else {
        libname_subst = STRcatn (3, "lib", global.modulename, "Mod");
    }

    for (i = 1; i <= global.num_fun_files; ++i) {
        snprintf (source, 20, "fun%d", i);
        RunCpp (global.config.ccp_mod, global.tmp_dirname, source, libname_subst);
        RunCc (global.config.compile_mod, global.tmp_dirname, source, libname_subst,
               global.config.cext, global.config.objext, objs_buf);
    }
    MEMfree (source);

    RunCpp (global.config.ccp_mod, global.tmp_dirname, "globals", libname_subst);
    RunCc (global.config.compile_mod, global.tmp_dirname, "globals", libname_subst,
           global.config.cext, global.config.objext, objs_buf);

    if (global.filetype == FT_cmod) {
        RunCpp (global.config.ccp_mod, global.tmp_dirname, "interface", libname_subst);
        RunCc (global.config.compile_mod, global.tmp_dirname, "interface", libname_subst,
               global.config.cext, global.config.objext, objs_buf);
        RunCpp (global.config.ccp_mod, global.tmp_dirname, "sacargcopy", libname_subst);
        RunCc (global.config.compile_mod, global.tmp_dirname, "sacargcopy", libname_subst,
               global.config.cext, global.config.objext, objs_buf);
        RunCpp (global.config.ccp_mod, global.tmp_dirname, "sacargfree", libname_subst);
        RunCc (global.config.compile_mod, global.tmp_dirname, "sacargfree", libname_subst,
               global.config.cext, global.config.objext, objs_buf);
    } else if (!global.notree) {
        RunCc (global.config.compile_tree, global.tmp_dirname, "serialize", libname_subst,
               global.config.tree_cext, global.config.tree_objext, tree_objs_buf);
        RunCc (global.config.compile_tree, global.tmp_dirname, "filenames", libname_subst,
               global.config.tree_cext, global.config.tree_objext, tree_objs_buf);
        RunCc (global.config.compile_tree, global.tmp_dirname, "namespacemap", libname_subst,
               global.config.tree_cext, global.config.tree_objext, tree_objs_buf);
        RunCc (global.config.compile_tree, global.tmp_dirname, "symboltable", libname_subst,
               global.config.tree_cext, global.config.tree_objext, tree_objs_buf);
        RunCc (global.config.compile_tree, global.tmp_dirname, "dependencytable", libname_subst,
               global.config.tree_cext, global.config.tree_objext, tree_objs_buf);
    }

    // Link phase: append the object dependencies to objects_buf,
    // Then proceed to link command.

    STRSfold (AddObjDependency, global.dependencies, objs_buf);

    objects_subst = SBUF2strAndFree (&objs_buf);
    tree_objects_subst = SBUF2strAndFree (&tree_objs_buf);

    path_buf = SBUFcreate (1);

    if (global.filetype == FT_cmod) {
        SBUFprintf (path_buf, "%s/%s/%s",
                    (global.lib_dirname == NULL) ? global.outfilename
                                                 : global.lib_dirname,
                    global.config.target_env, global.config.sbi);
    } else {
        SBUFprintf (path_buf, "%s/%s/%s", global.target_modlibdir,
                    global.config.target_env, global.config.sbi);
    }

    if (STRlen (global.config.variant) > 0)
        SBUFprintf (path_buf, "/%s", global.config.variant);

    path_subst = SBUF2strAndFree (&path_buf);

    // The destination directory may not exist at this point,
    // so invoke mkdir just in case.
    SYScall ("%s %s", global.config.mkdir, path_subst);

    target_subst = STRcatn (4, path_subst, "/", libname_subst, global.config.modext);

    RunLd (global.config.link_mod, path_subst, objects_subst, target_subst, libname_subst);

    if (global.filetype != FT_cmod && !global.notree) {

        MEMfree (path_subst);
        path_subst = STRcatn (3, global.targetdir, "/tree/", global.config.target_env);

        // The destination directory may not exist at this point,
        // so invoke mkdir just in case.
        SYScall ("%s %s", global.config.mkdir, path_subst);

        MEMfree (target_subst);
        target_subst = STRcatn (5, path_subst, "/lib", global.modulename, "Tree",
                                    global.config.tree_dllext);

        RunLd (global.config.link_tree, path_subst, tree_objects_subst, target_subst, libname_subst);

    }

    MEMfree (tree_objects_subst);
    MEMfree (objects_subst);
    MEMfree (target_subst);
    MEMfree (path_subst);
    MEMfree (libname_subst);

    DBUG_RETURN ();
}

node *CCTpreprocessCompileAndLink (node *syntax_tree)
{
    DBUG_ENTER ();

    InitPathList ();

    if (global.gen_cccall) {
        /*
         * enable system call tracking
         */
        SYSstartTracking ();
    }

    if (global.filetype == FT_prog) {
        HandleProgram ();
    } else { // FT_cmod, FT_classimp, or FT_modimp:
        HandleModule ();
    }

    if (global.gen_cccall) {
        /*
         * stop tracking and close file
         */
        SYSstopTracking ();
    }

    DeletePathList ();

    DBUG_RETURN (syntax_tree);
}

/*******************************************************************************
 *
 * @fn void CCTcompileOnly (void)
 *
 * @brief This is being triggered when compiling the runtime library which
 *        happens through the script in saccc. It calls sac2c -cc... to
 *        trigger compilation here.
 *
 *****************************************************************************/
void CCTcompileOnly (void)
{
    char *path_subst;

    DBUG_ENTER ();

    // Special case: we arrive here directly after options.c,
    // before any SAC compilation and thus before FMGRsetFileNames.
    if (global.sacfilename == NULL) {
        CTIabort (EMPTY_LOC, "Cannot proceed: no input file(s) specified");
    }
    if (global.outfilename == NULL) {
        CTIabort (EMPTY_LOC, "Cannot proceed: no output file specified");
    }

    path_subst = FMGRdirname (global.outfilename);

    CTInote (EMPTY_LOC, "Compiling C source \"%s\"", global.sacfilename);

    InitPathList ();
    /*
     * Here we use global.config.compile_rmod for all cases as this is the
     * only way to avoid splitting the compilation into CPP and CC.
     * and COMPILE_PROG, COMPILE_MOD, COMPILE_RMOD are identical, otherwise.
     */
    SubstituteAndRun (global.config.compile_rmod, path_subst, global.sacfilename,
                      "", global.outfilename, "");
    DeletePathList ();

    MEMfree (path_subst);

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * @fn void CCTlinkOnly (void)
 *
 * @brief This is being triggered when compiling the runtime library which
 *        happens through the script in saccc. It calls sac2c -cc... to
 *        trigger linking here.
 *
 *****************************************************************************/
void CCTlinkOnly (void)
{
    char *tmp;
    char *path_subst;
    char *libname_subst;
    const char *cmd;

    DBUG_ENTER ();

    // Special case: we arrive here directly after options.c,
    // before any SAC compilation and thus before FMGRsetFileNames.
    if (global.sacfilename == NULL) {
        CTIabort (EMPTY_LOC, "Cannot proceed: no input file(s) specified");
    }
    if (global.outfilename == NULL) {
        CTIabort (EMPTY_LOC, "Cannot proceed: no output file specified");
    }

    path_subst = FMGRdirname (global.outfilename);
    tmp = FMGRbasename (global.outfilename);
    libname_subst = FMGRstripExt (tmp);

    cmd = (global.do_clink == DO_C_rmod) ? global.config.link_rmod :
          (global.do_clink == DO_C_mod) ? global.config.link_mod :
                                          global.config.link_prog;

    CTInote (EMPTY_LOC, "Linking C objects \"%s\"", global.outfilename);

    InitPathList ();
    SubstituteAndRun (cmd, path_subst, "", global.sacfilename, global.outfilename, libname_subst);
    DeletePathList ();

    MEMfree (tmp);
    MEMfree (path_subst);
    MEMfree (libname_subst);

    DBUG_RETURN ();
}



#undef DBUG_PREFIX
