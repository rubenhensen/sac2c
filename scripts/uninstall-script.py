import os
import sys
import glob
import fnmatch
from optparse import OptionParser

SHARED_LIB_EXT = ".so"

# a list of binaries provided by one build type
SAC2C_BINARIES = [
        ("csima", ),
        ("csimt", ),
        ("sac2c", ),
        ("sac2tex", ),
        ("sac4c", ),
        ("saccc", ),
]


# a map of build-type to postfix
SAC2C_BUILD_TYPE_POSTFIXES = {
        "debug": "_d",
        "release": "_p",
}


def die (msg):
    print >>sys.stderr, msg
    sys.exit (-1)

def error (msg):
    die ("error: %s" % msg)

def warning (msg):
    print >>sys.stderr, "warning: %s" %msg

def info (msg):
    print "info: %s" % msg

def find_files_rec (d, pattern):
    l = []
    for root, dirnames, filenames in os.walk (d):
        for filename in fnmatch.filter (filenames, pattern):
            l.append (os.path.join(root, filename))
    return l

def dir_isempty (d):
    for root, dirs, files in os.walk (d):
        for name in files:
            return False
    return True


def rmdir_rec (d):
    "Remove directories recursively"
    # XXX(artem) it can be done easier by using shutil module, but this might be not
    #            available on standard python installations
    for root, dirs, files in os.walk (d, topdown=False):
        for name in files:
            os.remove (os.path.join (root, name))
        for name in dirs:
            os.rmdir (os.path.join (root, name))
    os.rmdir (d)

def list_of_files (prefix, ver, build_type):
    assert build_type in SAC2C_BUILD_TYPE_POSTFIXES

    bt_postfix = SAC2C_BUILD_TYPE_POSTFIXES[build_type]

    l = []
    # $prefix/lib/sac2c/$version/lib/*-$bt.$SHARED_LIB_EXT
    l.extend (find_files_rec (os.path.join (prefix, "lib", "sac2c", ver),
                              "*%s%s" % (bt_postfix, SHARED_LIB_EXT)))

    # $prefix/lib/sac2c/$version/lib/*-${bt}Mod$SHARED_LIB_EXT
    l.extend (find_files_rec (os.path.join (prefix, "lib", "sac2c", ver),
                              "*%sMod%s" % (bt_postfix, SHARED_LIB_EXT)))

    # $prefix/libexec/sac2c/$version/{libsacprelude${bt}*Tree.so}
    l.extend (find_files_rec (os.path.join (prefix, "libexec", "sac2c", ver),
                              "libsacprelude*%sTree%s" % (bt_postfix, SHARED_LIB_EXT)))

    # $prefix/libexec/sac2c/$version/{sac2c$bt, sac4c$bt, ...}
    for b in SAC2C_BINARIES:
        l.append (os.path.join (prefix, "libexec", "sac2c", ver, os.path.join (*b) + bt_postfix))

    # $prefix/libexec/sac2c/$version/libsac2c${bt}.so
    l.append (os.path.join (prefix, "libexec", "sac2c", ver, "libsac2c%s%s" % (bt_postfix, SHARED_LIB_EXT)))

    # $prefix/share/sac2c/$version/sac2crc$bt
    l.append (os.path.join (prefix, "share", "sac2c", ver, "sac2crc" + bt_postfix))

    # $prefix/include/sac2c/$version/$build_type/*
    l.extend (find_files_rec (os.path.join (prefix, "include", "sac2c", ver, build_type), "*"))

    # XXX This is internal sanity check
    lbad = []
    for f in l:
        if not os.path.isfile (f):
            lbad.append (f)

    if lbad != []:
        print >>sys.stderr, """\
innternal error:
    The `list_of_files' function returned inconsistent list of files,
    the following files don't exist:
\t%s

    Please fix the function in %s""" % ("\n\t".join (lbad), __file__)
        exit (-1)

    return l

def remove_version (prefix, ver_tuple):
    version, build_type = ver_tuple

    # Remove files generated by the LIST_OF_FILES
    for f in  list_of_files (prefix, version, build_type):
        os.remove (f)

    # Check whether there exists another build type in the given version
    # and if there isn't one, remove the directory structure of this
    # sac2c version.
    if glob_versions (prefix, version) == []:
        dirs_to_remove = [
            os.path.join (prefix, "include", "sac2c", version),
            os.path.join (prefix, "lib", "sac2c", version),
            os.path.join (prefix, "libexec", "sac2c", version),
            os.path.join (prefix, "share", "sac2c", version),
        ]

        for d in dirs_to_remove:
            if not dir_isempty (d):
                warning ("directory structure of sac2c version `%s' is not removed "
                         "as directory `%s' is not empty" % (version, d))
                exit (0)

        print "removing directory structure for sac2c versoin `%s'" % version
        for d in dirs_to_remove:
            rmdir_rec (d)

def verify_dirs_under_prefix (prefix):
    "Make sure that directory structure under prefix is sound"
    expected_dir = ['include', 'bin', 'libexec', 'share', 'lib']
    prefix_dirs = os.listdir (prefix)
    for d in expected_dir:
        if d not in prefix_dirs:
            error ("cannot find `%s' under `%s' --- is the prefix set correctly?" \
                    % (d, prefix))

def print_available_versions (prefix):
    "List available versions based on listing the content of libexec"
    versions = glob_versions (prefix, "*")
    if versions == []:
        print "No sac2c versions are installed."
        exit (0)

    print "sac2c versions available:"
    for v, bt in versions:
        print "\t%s [build-type = %s]" % (v, bt)

def dry_run_delete (prefix, versions):
    "List files that will be removed"
    print "files to be removed:"
    l = len (versions)
    for idx, (v, bt) in enumerate (versions):
        for f in list_of_files (prefix, v, bt):
            print "--remove `%s'" % f
        if idx != l - 1:
            print

def glob_versions (prefix, glob_expr, build_type=None):
    "Expand the list of versions that matches the argument passed to the script"
    glob_prefix = os.path.join (prefix, "libexec", "sac2c")
    glob_versions = glob.glob (os.path.join (glob_prefix, glob_expr))

    ver_dirs = [os.path.split (x)[1] for x in glob_versions]
    if ver_dirs == [] or ver_dirs == [""]:
        error ("none of the sac2c versions matching `%s' were found" % glob_expr)

    binary_to_build_types = None

    # Create a filter of build_types that we will look for in the given
    # version.  If the `build_type' argument is None then we consider
    # all the build_types.
    if build_type is None:
        binary_to_build_types = {
            "sac2c" + SAC2C_BUILD_TYPE_POSTFIXES[x]: x
            for x in SAC2C_BUILD_TYPE_POSTFIXES
        }
    else:
        binary_to_build_types = {"sac2c" + SAC2C_BUILD_TYPE_POSTFIXES[build_type] : build_type}

    assert binary_to_build_types is not None

    versions = []
    for v in ver_dirs:
        for b in binary_to_build_types:
            if os.path.isfile (os.path.join (glob_prefix, v, b)):
                versions.append ((v, binary_to_build_types[b]))

    return versions

def confirm_removal (prefix, versions):
    "Ask a user if he/she really wants to delete those files"
    sys.stdout.write ("delete %r sac2c versions in %s [Y/N] " % (versions, prefix))
    choice = raw_input ().lower ()
    return choice in ["y", "ye", "yes", "aye", "i don't give shit"]


def delete_versions (prefix, glob_expr, dryrun, build_type):
    "Expand versions to be deleted and delete them after confirmation"
    versions = glob_versions (prefix, glob_expr, build_type)

    if versions == []:
        warning ("there are no sac2c versions to remove")
        exit (0)

    if (dryrun):
        dry_run_delete (prefix, versions)
        exit (0)

    if confirm_removal (prefix, versions):
        for v in versions:
            remove_version (prefix, v)
    else:
        print "Nothing removed"

def binary_to_version (sac2c_binary):
    binary_to_build_types = {
        "sac2c" + SAC2C_BUILD_TYPE_POSTFIXES[x]: x
        for x in SAC2C_BUILD_TYPE_POSTFIXES
    }
    if not sac2c_binary in binary_to_build_types:
        error ("undefined build type `%s' of sac2c found" % sac2c_binary)

    return binary_to_build_types[sac2c_binary]


def current_version (prefix):
    "Find out which version of sac2c is currently chosen"

    if not os.path.isfile (os.path.join (prefix, "bin", "sac2c")):
        error ("sac2c is not set to any version")

    sac2c_real_binaries = [
            "sac2c" + SAC2C_BUILD_TYPE_POSTFIXES[x]
            for x in SAC2C_BUILD_TYPE_POSTFIXES
    ]

    sac2c_symlink = os.path.join (prefix, "bin", "sac2c")
    if not os.path.islink (sac2c_symlink):
        error ("broken sac2c installation: `%s' should be a symbolic link" % sac2c_symlink)

    # resolve symlink
    sac2c_path = os.path.realpath (sac2c_symlink)
    d, sac2c_binary = os.path.split (sac2c_path)
    if sac2c_binary not in sac2c_real_binaries:
        error ("the sac2c symlink should point to one of {%s}; either\n"
                 "sac2c installation is broken, or you need to update this script!"
                 %",".join (sac2c_real_binaries))

    print """Current sac2c symlink points to:
VERSION    = `%s'
BUILD_TYPE = `%s'""" % (os.path.split (d)[1], binary_to_version (sac2c_binary))


def switch_version (prefix, version, sac2c_postfix):
    "create symbolic links for the relevant files"

    # First, for every binary, we create a postfixed version that points
    # to the corresponding libexec location.
    for p in SAC2C_BINARIES:
        pp = [x for x in p]
        pp[-1] += sac2c_postfix
        link_name = os.path.join (prefix, "bin", *pp)
        link_src = os.path.join (prefix, "libexec", "sac2c", version,  *pp)
        # FIXME check whether we have permissions to delete the file
        if os.path.islink (link_name):
            os.unlink (link_name)
        elif os.path.isfile (link_name):
            os.remove (link_name)

        os.symlink (link_src, link_name)

    # Secondly, we create links for every binary.
    for p in SAC2C_BINARIES:
        pp = [x for x in p]
        pp[-1] += sac2c_postfix
        link_name = os.path.join (prefix, "bin", *p)
        link_src = os.path.join (prefix, "libexec", "sac2c", version,  *pp)

        link_name = os.path.join (prefix, "bin", *p)
        # FIXME check whether we have permissions to delete the file
        if os.path.islink (link_name):
            os.unlink (link_name)
        elif os.path.isfile (link_name):
            os.remove (link_name)

        os.symlink (link_src, link_name)


def usage (basename):
    print >>sys.stderr, """\
sac2c versions manager

usage:
  %(base)s list-versions
  %(base)s current-version
  %(base)s remove
  %(base)s switch

Type --help at the end of every option to get list of
command-specific options""" % {'base' : basename}
    sys.exit (0)

def handle_remove (argv):
    parser = OptionParser ("Remove sac2c versions")
    parser.add_option ("-p", "--prefix", dest="prefix", type="string",
                       help="installation prefix of sac2c versions")
    parser.add_option ("-v", "--sac2c-version", dest="sac2c_version", type="string",
                       help="version(-s) of sac2c to uninstall (regexps accepted)")
    parser.add_option ("-d", "--dry-run", dest="dryrun", action="store_true",
                       default=False, help="list the files that will be uninstalled")
    parser.add_option ("-b", "--build-type", dest="build_type", type="string",
            help="build type {%s} of sac2c to remove (ALL by default)"
                 % ",".join (SAC2C_BUILD_TYPE_POSTFIXES.keys()))

    (options, args) = parser.parse_args (argv)
    if args != []:
        error ("this script does not take arguments, (%s) found" %  ",".join (args))

    if options.prefix is None:
        error ("sac2c installation prefix is not specified (use --prefix to set it up)")

    if options.sac2c_version is None:
        error ("which sac2c versoin under `%s' should be uninstalled?  "\
               "(use --sac2c-version to set it up)" % options.prefix)

    # Expand home/user in the prefix
    prefix = os.path.expanduser (options.prefix)

    # Make sure that the directory structure is sane
    verify_dirs_under_prefix (prefix)
    delete_versions (prefix, options.sac2c_version, options.dryrun, options.build_type)


def handle_list_versions (argv):
    parser = OptionParser ("List sac2c versions")
    parser.add_option ("-p", "--prefix", dest="prefix", type="string",
                       help="installation prefix of sac2c versions")
    (options, args) = parser.parse_args (argv)
    if args != []:
        error ("this script does not take arguments, (%s) found" %  ",".join (args))

    if options.prefix is None:
        error ("please specify sac2c installation prefix")

    prefix = os.path.expanduser (options.prefix)
    print_available_versions (prefix)

def handle_current_version (argv):
    parser = OptionParser ("List current version of sac2c")
    parser.add_option ("-p", "--prefix", dest="prefix", type="string",
                       help="installation prefix of sac2c versions")
    (options, args) = parser.parse_args (argv)
    if args != []:
        error ("this script does not take arguments, (%s) found" %  ",".join (args))

    if options.prefix is None:
        error ("please specify sac2c installation prefix")

    prefix = os.path.expanduser (options.prefix)
    current_version (prefix)

def handle_switch (argv):
    parser = OptionParser ("List sac2c versions")
    parser.add_option ("-p", "--prefix", dest="prefix", type="string",
                       help="installation prefix of sac2c versions")
    parser.add_option ("-v", "--sac2c-version", dest="sac2c_version", type="string",
                       help="version(-s) of sac2c to uninstall (regexps accepted)")
    parser.add_option ("-b", "--build-type", dest="build_type", type="string",
            help="build type {%s} of sac2c to switch to" % ",".join (SAC2C_BUILD_TYPE_POSTFIXES.keys()))

    (options, args) = parser.parse_args (argv)
    if args != []:
        error ("this script does not take arguments, (%s) found" %  ",".join (args))

    if options.prefix is None:
        error ("please specify sac2c installation prefix")

    prefix = os.path.expanduser (options.prefix)

    if options.sac2c_version is None:
        error ("which sac2c versoin under `%s' should be used?  "
               "(use --sac2c-version to set it up)" % options.prefix)

    versions = glob_versions (prefix, options.sac2c_version)
    if versions == []:
        error ("sac2c version `%s' does not exist" % options.sac2c_version)

    build_type = None
    if options.build_type is None:
        if len (versions) > 1:
            error ("there are multiple build types for version `%s', please "
                   "set which one you want to use" % options.sac2c_version)
        else:
            build_type = versions[0][1]
    elif options.build_type not in SAC2C_BUILD_TYPE_POSTFIXES:
        error ("invalid build type `%s'")
    else:
        build_type = options.build_type

    assert build_type is not None

    switch_version (prefix, versions[0][0], SAC2C_BUILD_TYPE_POSTFIXES[build_type])


if __name__ == "__main__":
    if len (sys.argv) < 2:
        usage (sys.argv[0])

    command = sys.argv[1]
    args = sys.argv[2:]

    if command == "remove":
        handle_remove (args)
    elif command == "list-versions":
        handle_list_versions (args)
    elif command == "switch":
        handle_switch (args)
    elif command == "current-version":
        handle_current_version (args)
    else:
        usage (sys.argv[0])
