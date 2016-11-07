import os
import sys
import glob
from optparse import OptionParser

# a list of directories under PREFIX that contatin a VERSION diretory that
# can be removed with its content
version_dir = ["include/sac2c", "lib/sac2c", "libexec/sac2c", "share/sac2c"]

# a list of files under PREFIX that shall be deleted
sac2c_binaries = ["bin/csima", "bin/csimt", "bin/sac2c", \
                  "bin/sac2tex", "bin/sac4c", "bin/saccc"]

def die (msg):
    print >>sys.stderr, msg
    sys.exit (-1)

def error (msg):
    die ("error: %s" % msg)

def warning (msg):
    print >>sys.stderr, "warning: %s" %msg

def info (msg):
    print "info: %s" % msg

def list_of_files (prefix, ver):
    l = []
    # Append VERSION_DIR with PREFIX
    full_dirs = [os.path.join (prefix, x, ver) for x in version_dir]

    for d in full_dirs:
        for root, dirs, files in os.walk (d):
            for f in files:
                l.append (os.path.join (root, f))

    # Add SAC2C_BINARIES prepended with PREFIX
    l.extend ([os.path.join (prefix, x) for x in sac2c_binaries])

    return l

def remove_version (prefix, ver):
    for d in version_dir:
        # Remove directories recursively
        # XXX(artem) it can be done easier by using shutil module, but this might be not
        #            available on standard python installations
        vd = os.path.join (prefix, d, ver)
        for root, dirs, files in os.walk (vd, topdown=False):
            for name in files:
                os.remove (os.path.join (root, name))
            for name in dirs:
                os.rmdir (os.path.join (root, name))
        os.rmdir (vd)

    for f in sac2c_binaries:
        # We can assume that sac2c installs can be broken...
        ff = os.path.join (prefix, f)
        if os.path.isfile (ff):
            os.remove (ff)
        else:
            warning ("cannot find `%s', skipping removal" % ff)

def parse_options ():
    "Parse the options to the script"
    parser = OptionParser ("Uninstall sac2c versions under the given prefix")
    parser.add_option ("-p", "--prefix", dest="prefix", type="string",
                       help="installation prefix of sac2c versions")
    parser.add_option ("-v", "--sac2c-version", dest="sac2c_version", type="string",
                       help="version(-s) of sac2c to uninstall (regexps accepted)")
    parser.add_option ("-l", "--list-versions", dest="list_versions", action="store_true",
                       default=False, help="list verisons of sac2c that can be uninstalled")
    parser.add_option ("-d", "--dry-run", dest="dryrun", action="store_true",
                       default=False, help="list the files that will be uninstalled")
    parser.add_option ("-r", "--remove", dest="remove", action="store_true",
                       default=False, help="remove sac2c versions")

    (options, args) = parser.parse_args ()
    if args != []:
        error ("this script does not take arguments, (%s) found" %  ",".join (args))

    return options

def verify_options (options):
    "Make sure that essential options are passed in"
    if options.prefix is None:
        error ("sac2c installation prefix is not specified (use --prefix to set it up)")

    if options.remove and options.sac2c_version is None:
        error ("which sac2c versoin under `%s' should be uninstalled?  "\
               "(use --sac2c-version to set it up)" % options.prefix)

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
    available_versions = os.listdir (os.path.join (prefix, "libexec/sac2c"))
    if available_versions == []:
        print "No sac2c versions are installed."
        exit (0)

    print "sac2c versions available for removal:"
    for v in available_versions:
        print "\t%s" %v
    exit (0)

def dry_run_delete (prefix, versions):
    "List files that will be removed"
    print "files to be removed:"
    for v in versions:
        for f in list_of_files (prefix, v):
            print "--remove `%s'" % f
        print
    exit (0)


def glob_versions (prefix, glob_expr):
    "Expand the list of versions that matches the argument passed to the script"
    glob_prefix = os.path.join (prefix, "libexec", "sac2c")
    glob_versions = glob.glob (os.path.join (glob_prefix, glob_expr))
    versions = [os.path.split (x)[1] for x in glob_versions]
    if versions == [] or versions == [""]:
        error ("none of the sac2c versions matching `%s' were found" % glob_expr)
    return versions

def confirm_removal (prefix, versions):
    "Ask a user if he/she really wants to delete those files"
    sys.stdout.write ("delete %r sac2c versions in %s [Y/N] " % (versions, prefix))
    choice = raw_input ().lower ()
    return choice in ["y", "ye", "yes", "aye", "i don't give shit"]
    

def delete_versions (prefix, glob_expr, dryrun):
    "Expand versions to be deleted and delete them after confirmation"
    versions = glob_versions (prefix, glob_expr)

    if (dryrun):
        # Tohis call will terminate the script
        dry_run_delete (prefix, versions)

    if confirm_removal (prefix, versions):
        for v in versions:
            remove_version (prefix, v)
    else:
        print "Nothing removed"


def current_version (prefix):
    "Find out which version of sac2c is currently chosen"
    
    # FIXME(artem) move this outside the function
    sac2c_real_binaries = ('sac2c-d', 'sac2c-p')

    sac2c_symlink = os.path.join (prefix, "bin", "sac2c")
    if not os.path.islink (sac2c_symlink):
        error ("broken sac2c installation: `%s' should be a symbolic link" % sac2c_symlink)

    # resolve symlink
    sac2c_path = os.path.realpath (sac2c_symlink)
    d, sac2c_binary = os.path.split (sac2c_path)
    if sac2c_binary not in sac2c_real_binaries:
        warning ("the sac2c symlink should point to one of {%s}; either\n"
                 "sac2c installation is broken, or you need to update this script!"
                 %",".join (sac2c_real_binaries))

    ver = os.path.split (d)[1]
    return ver


# FIXME(artem) This is a rough idea.
def switch_version (prefix, version, sac2c_postfix):
    "create symbolic links for the relevant files"
    # SASC2C_POSTFIX is '-d' or -p' which links sac2c -> sac2c-d, etc.
    
    d = {('bin', 'sac2c'): ('libexec', 'sac2c', version, 'sac2c' + sac2c_postfix)}

    for p in d:
        print os.path.join (prefix, *p), "->", os.path.join (prefix, *d[p])
        #os.symlink (os.path.join (prefix, *d[p]), os.path.join (prefix, *p))

    # Swithc TOOLS:
    #    * sac2c, sac4c, sac2tex
    #    * includes: sacinterface.h, sacinterface.f, sac_serialize.h, polyhedral_defs.h
    #    * saclib tools: csima, csimt
    #    * sac.h, sac2crc


def main ():
    options = parse_options ()
    verify_options (options)

    # Expand home/user in the prefix
    prefix = os.path.expanduser (options.prefix)

    verify_dirs_under_prefix (prefix)

    if options.list_versions:
        # This call will terminate the script
        print_available_versions (prefix)

    if options.remove:
        delete_versions (prefix, options.sac2c_version, options.dryrun)

    print "version = ", current_version (prefix)
    switch_version (prefix, "1.2-beta-BlackForest-220-g6ee9e", "-d")

if __name__ == "__main__":
    main ()
