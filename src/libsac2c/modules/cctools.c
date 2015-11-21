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

static void *
AddIncPath (const char *path, void *buf)
{
    DBUG_ENTER ();

    str_buf *sbuf = (str_buf *)buf;
    sbuf = SBUFprintf (sbuf, " -I%s", path);

    DBUG_RETURN (buf);
}

static void *
AddLibPath (const char *path, void *buf)
{
    DBUG_ENTER ();

    str_buf *sbuf = (str_buf *)buf;

    /* make path absolute as the linker can get confused otherwise. */
    char *abspath = FMGRabsName (path);

    char *str = STRsubstToken (global.config.ldpath, "%path%", abspath);
    sbuf = SBUFprintf (sbuf, " %s", str);

    str = MEMfree (str);
    abspath = MEMfree (abspath);

    DBUG_RETURN (buf);
}

static void *
AddModLibPath (const char *path, void *buf)
{
    DBUG_ENTER ();

    str_buf *sbuf = (str_buf *)buf;
    char *fpath
      = STRcatn (5, path, "/", global.config.target_env, "/", global.config.sbi);

    /* make path absolute as the linker can get confused otherwise. */
    char *absfpath = FMGRabsName (fpath);

    char *str = STRsubstToken (global.config.ldpath, "%path%", absfpath);
    sbuf = SBUFprintf (sbuf, " %s", str);

    str = MEMfree (str);
    absfpath = MEMfree (absfpath);
    fpath = MEMfree (fpath);

    DBUG_RETURN (buf);
}

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
            CTInote ("External object %s picked from current directory.", lib);
            SBUFprintf (sbuf, " %s", lib);
            break;
        }

        if (lib[0] != '/') {
            rpath = STRcatn (3, global.targetdir, "/", lib);

            if (FMGRcheckExistFile (rpath)) {
                CTInote ("External object %s picked from build target directory (%s)",
                         lib, rpath);
                SBUFprintf (sbuf, " %s", rpath);
                break;
            }

            if (global.target_modlibdir != NULL) {
                rpath = MEMfree (rpath);
                rpath = STRcatn (3, global.target_modlibdir, "/", lib);
                if (FMGRcheckExistFile (rpath)) {
                    CTInote (
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
                CTInote ("External object %s picked from source directory (%s)", lib,
                         rpath);
                SBUFprintf (sbuf, " %s", rpath);
                break;
            }
        }

        CTIerror ("Unable to find external object: %s", lib);

        break;
    default:
        break;
    }

    rpath = MEMfree (rpath);
    src_dirname = MEMfree (src_dirname);

    DBUG_RETURN (buf);
}

char *
CCTperformTask (ccm_task_t task)
{
    DBUG_ENTER ();

    /******************* compilation flags ***********************/
    // %opt%
    const char *opt_subst = "";
    switch (global.cc_optimize) {
    case 0:
        opt_subst = global.config.opt_o0;
        break;
    case 1:
        opt_subst = global.config.opt_o1;
        break;
    case 2:
        opt_subst = global.config.opt_o2;
        break;
    case 3:
        opt_subst = global.config.opt_o3;
        break;
    }

    // %dbg%
    const char *dbg_subst = global.cc_debug ? global.config.opt_g : "";

    // %sacincludes%
    const char *sacincludes_subst = global.config.sacincludes;

    // %cflags%
    str_buf *cflags_buf = SBUFcreate (1);
    SBUFprint (cflags_buf, global.config.cflags);
    FMGRmapPath (PK_inc_path, AddIncPath, cflags_buf);
    SBUFprintf (cflags_buf, " %s", global.cflags);
    SBUFprintf (cflags_buf,
                " -DSAC_TARGET_STRING=\\\"%s\\\""
                " -DSAC_MODEXT_STRING=\\\"%s\\\""
                " -DSAC_TARGET_ENV_STRING=\\\"%s\\\""
                " -DSAC_SBI_STRING=\\\"%s\\\""
                " -DSAC_RC_METHOD=SAC_RCM_%s"
                " -DSAC_BACKEND_%s"
                " -DSAC_MT_LIB_%s"
                " -DSAC_MT_MODE=%d"
                " -DSAC_DO_RTSPEC=%d",
                global.target_name, global.config.modext, global.config.target_env,
                global.config.sbi, global.config.rc_method,
                global.backend_string[global.backend], global.config.mt_lib,
                global.config.mt_mode, global.config.rtspec);
    char *cflags_subst = SBUF2strAndFree (&cflags_buf);

    // %tree_cflags%
    char *tree_cflags_subst = global.tree_cflags;

    // %compileflags%
    char *compileflags_subst
      = STRcatn (7, opt_subst, " ", dbg_subst, " ", cflags_subst, " ", sacincludes_subst);

    if (task == CCT_compileflags) {
        MEMfree (cflags_subst);
        DBUG_RETURN (compileflags_subst);
    }

    // %cc%
    const char *cc_subst = global.config.cc;

    /******************* link flags ***********************/

    // %extlibdirs%
    str_buf *extlibdirs_buf = SBUFcreate (1);

    FMGRmapPath (PK_extlib_path, AddLibPath, extlibdirs_buf);

    char *extlibdirs_subst = SBUF2strAndFree (&extlibdirs_buf);

    // %modlibdirs%
    str_buf *modlibdirs_buf = SBUFcreate (1);

    FMGRmapPath (PK_lib_path, AddModLibPath, modlibdirs_buf);

    char *modlibdirs_subst = SBUF2strAndFree (&modlibdirs_buf);

    // %modlibs%
    str_buf *modlibs_buf = SBUFcreate (1);
    STRSfold (AddLibDependency, global.dependencies, modlibs_buf);
    char *modlibs_subst = SBUF2strAndFree (&modlibs_buf);

    // %saclibs%
    char *saclibs_subst;
    if (global.loadprelude == TRUE) {
        // libsac invokes SAC_HM_ShowDiagnostics, which is
        // provided by libphm(c), which in turn depends on libsac.
        // Express -llsacphm  before *and* after -lsac
        // to keep single-pass linkers happy.
        saclibs_subst = STRcatn (6, "-lsacphm", global.optimize.dophm ? "" : "c",
                                 global.runtimecheck.heap ? ".diag" : "",
                                 " -lsac "
#if ENABLE_DISTMEM
                                 " -lsacdistmem "
#endif
                                 " -lsacphm",
                                 global.optimize.dophm ? "" : "c",
                                 global.runtimecheck.heap ? ".diag " : " ");
    } else {
        saclibs_subst = STRcpy ("");
    }

    // %libs%
    const char *libs_subst = global.config.libs;

    // %ldflags%
    char *ldflags_subst = STRcatn (3, global.config.ldflags, " ", global.ldflags);

    // %tree_ldflags%
    char *tree_ldflags_subst = global.tree_ldflags;

    // %linkflags%
    char *linkflags_subst
      = STRcatn (11, ldflags_subst, " ", modlibdirs_subst, " ", modlibs_subst, " ",
                 extlibdirs_subst, " ", saclibs_subst, " ", libs_subst);
    // Normally this should only be called by sac4c
    if (task == CCT_linkflags) {
        MEMfree (cflags_subst);
        MEMfree (extlibdirs_subst);
        MEMfree (modlibdirs_subst);
        MEMfree (modlibs_subst);
        MEMfree (saclibs_subst);
        MEMfree (ldflags_subst);
        DBUG_RETURN (linkflags_subst);
    }

    // %ld%
    const char *ld_subst = global.config.ld;

    // %libname%
    char *libname_subst;

    if (task == CCT_doall) {
        switch (global.filetype) {
        case FT_cmod:
            libname_subst = STRcat ("lib", global.outfilename);
            break;
        case FT_classimp:
        case FT_modimp:
            libname_subst = STRcatn (3, "lib", global.modulename, "Mod");
            break;
        default:
            libname_subst = STRcpy ("");
            break;
        }
    } else if (task == CCT_clinkonly) {
        libname_subst = FMGRbasename (global.outfilename);
        // Strip extension if present
        char *ppos = strrchr (libname_subst, '.');
        if (ppos != NULL)
            *ppos = '\0';
    } else {
        libname_subst = STRcpy ("");
    }

    // %target% is substituted later
    // %objects% is substituted later
    // %source% is substituted later
    // %path% is substituted later

#define SUBST(Name) "%" #Name "%", Name##_subst
#define DO_SUBST(CommandString)                                                          \
    STRsubstTokens (CommandString, 21, SUBST (cc), SUBST (ld), SUBST (opt), SUBST (dbg), \
                    SUBST (sacincludes), SUBST (tree_cflags), SUBST (cflags),            \
                    SUBST (compileflags), SUBST (extlibdirs), SUBST (modlibdirs),        \
                    SUBST (modlibs), SUBST (saclibs), SUBST (libs), SUBST (ldflags),     \
                    SUBST (tree_ldflags), SUBST (linkflags), SUBST (path),               \
                    SUBST (target), SUBST (libname), SUBST (objects), SUBST (source))

#define DO_COMPILE(CompileString, SourceDir, Source, Kind)                               \
    do {                                                                                 \
        const char *path_subst = global.tmp_dirname;                                     \
        char *source_subst                                                               \
          = STRcatn (4, SourceDir, "/", Source, global.config.Kind##cext);               \
        char *target_subst                                                               \
          = STRcatn (4, path_subst, "/", Source, global.config.Kind##objext);            \
        SBUFprintf (Kind##objs_buf, " %s", target_subst);                                \
        const char *objects_subst = "";                                                  \
        char *compile_cmd = DO_SUBST (CompileString);                                    \
        source_subst = MEMfree (source_subst);                                           \
        target_subst = MEMfree (target_subst);                                           \
                                                                                         \
        CTInote ("Compiling C source \"%s\"", Source);                                   \
        DBUG_PRINT ("compile command: %s", compile_cmd);                                 \
        SYScall ("%s", compile_cmd);                                                     \
        compile_cmd = MEMfree (compile_cmd);                                             \
    } while (0)

#define DO_LINK(LinkString, Objects)                                                     \
    do {                                                                                 \
        const char *source_subst = "";                                                   \
        const char *objects_subst = Objects;                                             \
        char *link_cmd = DO_SUBST (LinkString);                                          \
                                                                                         \
        CTInote ("Linking \"%s\"", target_subst);                                        \
        DBUG_PRINT ("link command: %s", link_cmd);                                       \
        SYScall ("%s", link_cmd);                                                        \
        link_cmd = MEMfree (link_cmd);                                                   \
    } while (0)

    if (task == CCT_ccompileonly || task == CCT_clinkonly) {
        // Special case: we arrive here directly after options.c,
        // before any SAC compilation and thus before FMGRsetFileNames.
        if (global.sacfilename == NULL) {
            CTIabort ("Cannot proceed: no input file(s) specified");
        }
        if (global.outfilename == NULL) {
            CTIabort ("Cannot proceed: no output file specified");
        }

        char *path_subst = FMGRdirname (global.outfilename);
        const char *target_subst = global.outfilename;
        const char *source_subst, *objects_subst;
        char *cmd;

        if (task == CCT_ccompileonly) {
            source_subst = global.sacfilename;
            objects_subst = "";
            cmd = global.do_ccompile == DO_C_mod ? global.config.compile_mod
                                                 : global.config.compile_prog;
            CTInote ("Compiling C source \"%s\"", source_subst);
        } else { // task == CCT_clinkonly
            source_subst = "";
            objects_subst = global.sacfilename;
            cmd = global.do_clink == DO_C_mod ? global.config.link_mod
                                              : global.config.link_prog;
            CTInote ("Linking C objects \"%s\"", objects_subst);
        }
        cmd = DO_SUBST (cmd);
        DBUG_PRINT (" command: %s", cmd);
        SYScall ("%s", cmd);
        cmd = MEMfree (cmd);
        path_subst = MEMfree (path_subst);
    } else {

        // Compile phase: run each compiler command, and accumulate the name
        // of generate objects in objects_buf.

        str_buf *objs_buf = SBUFcreate (1);
        str_buf *tree_objs_buf = SBUFcreate (1);

        if (global.filetype == FT_prog) {
            DO_COMPILE (global.config.compile_prog, global.targetdir,
                        global.outfilename, );
        } else {
            int i;
            char *source = MEMmalloc (20);
            for (i = 1; i <= global.num_fun_files; ++i) {
                snprintf (source, 20, "fun%d", i);
                DO_COMPILE (global.config.compile_mod, global.tmp_dirname, source, );
            }
            MEMfree (source);

            DO_COMPILE (global.config.compile_mod, global.tmp_dirname, "globals", );

            if (global.filetype == FT_cmod) {
                DO_COMPILE (global.config.compile_mod, global.tmp_dirname, "interface", );
                DO_COMPILE (global.config.compile_mod, global.tmp_dirname,
                            "sacargcopy", );
                DO_COMPILE (global.config.compile_mod, global.tmp_dirname,
                            "sacargfree", );
            } else {
                DO_COMPILE (global.config.compile_tree, global.tmp_dirname, "serialize",
                            tree_);
                DO_COMPILE (global.config.compile_tree, global.tmp_dirname, "filenames",
                            tree_);
                DO_COMPILE (global.config.compile_tree, global.tmp_dirname,
                            "namespacemap", tree_);
                DO_COMPILE (global.config.compile_tree, global.tmp_dirname, "symboltable",
                            tree_);
                DO_COMPILE (global.config.compile_tree, global.tmp_dirname,
                            "dependencytable", tree_);
            }
        }

        // Link phase: append the object dependencies to objects_buf,
        // Then proceed to link command.

        STRSfold (AddObjDependency, global.dependencies, objs_buf);

        char *objects = SBUF2strAndFree (&objs_buf);
        char *tree_objects = SBUF2strAndFree (&tree_objs_buf);

        if (global.filetype == FT_prog) {
            const char *path_subst = global.targetdir;
            char *target_subst = STRcat (global.outfilename, global.config.exeext);

            DO_LINK (global.config.link_prog, objects);

            MEMfree (target_subst);
        } else {
            char *path_subst;

            str_buf *path_buf = SBUFcreate (1);

            if (global.filetype == FT_cmod) {
                SBUFprintf (path_buf, "%s/%s/%s",
                            (NULL == global.lib_dirname) ? global.outfilename
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

            char *target_subst
              = STRcatn (4, path_subst, "/", libname_subst, global.config.modext);

            DO_LINK (global.config.link_mod, objects);

            if (global.filetype != FT_cmod) {

                MEMfree (path_subst);
                MEMfree (target_subst);

                path_subst
                  = STRcatn (3, global.targetdir, "/tree/", global.config.target_env);

                // The destination directory may not exist at this point,
                // so invoke mkdir just in case.
                SYScall ("%s %s", global.config.mkdir, path_subst);

                target_subst = STRcatn (5, path_subst, "/lib", global.modulename, "Tree",
                                        global.config.tree_dllext);

                DO_LINK (global.config.link_tree, tree_objects);
            }

            MEMfree (target_subst);
            MEMfree (path_subst);
        }

        MEMfree (objects);
        MEMfree (tree_objects);
    }

    // Release all non-const strings allocated
    MEMfree (cflags_subst);
    MEMfree (extlibdirs_subst);
    MEMfree (modlibdirs_subst);
    MEMfree (modlibs_subst);
    MEMfree (saclibs_subst);
    MEMfree (ldflags_subst);
    MEMfree (linkflags_subst);
    MEMfree (libname_subst);

    DBUG_RETURN (NULL);
}

node *
CCTrunTools (node *syntax_tree)
{
    DBUG_ENTER ();

    if (global.gen_cccall) {
        /*
         * enable system call tracking
         */
        SYSstartTracking ();
    }

    (void)CCTperformTask (CCT_doall);

    if (global.gen_cccall) {
        /*
         * stop tracking and close file
         */
        SYSstopTracking ();
    }

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
