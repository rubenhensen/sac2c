import os
import sys
import glob
from optparse import OptionParser

# a list of directories under PREFIX that contatin a VERSION diretory that
# can be removed with its content
version_dir = ["include/sac2c", "lib/sac2c", "libexec/sac2c", "share/sac2c"]

# a list of files under PREFIX that shall be deleted
sac2c_binaries = ["bin/csima", "bin/csimt", "bin/sac2c",
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


parser = OptionParser ("Uninstall sac2c versions under the given prefix")
parser.add_option ("-p", "--prefix", dest="prefix", type="string",
                   help="installation prefix of sac2c versions")
parser.add_option ("-v", "--sac2c-version", dest="sac2c_version", type="string",
                   help="version(-s) of sac2c to uninstall (regexps accepted)")
parser.add_option ("-l", "--list-versions", dest="lsver", action="store_true",
                   default=False, help="list verisons of sac2c that can be uninstalled")
parser.add_option ("-d", "--dry-run", dest="dryrun", action="store_true",
                   default=False, help="list the files that will be uninstalled")

(options, args) = parser.parse_args ()

if options.prefix is None:
    error ("sac2c installation prefix is not specified (use --prefix to set it up)")

if not options.lsver and options.sac2c_version is None:
    error ("which sac2c versoin under `%s' should be uninstalled?  "\
           "(use --sac2c-version to set it up)" % options.prefix)

# Expand home/user in the prefix
prefix = os.path.expanduser (options.prefix)
sac2c_v = options.sac2c_version
listonly = options.lsver
dryrun = options.dryrun

# List directories under prefix
expected_dir = ['include', 'bin', 'libexec', 'share', 'lib']
prefix_dirs = os.listdir (prefix)
for d in expected_dir:
    if d not in prefix_dirs:
        error ("cannot find `%s' under `%s' --- is the prefix set correctly?" \
                % (d, prefix))

# List available versions
available_versions = os.listdir (os.path.join (prefix, "libexec/sac2c"))

if (listonly):
    print "sac2c versions available for removal"
    for v in available_versions:
        print "\t%s" %v
    exit (0)

# sac2c versions that match sac2c_v argument
glob_prefix = os.path.join (prefix, "libexec", "sac2c")
glob_versions = glob.glob (os.path.join (glob_prefix, sac2c_v))
versions = [os.path.split (x)[1] for x in glob_versions]
if versions == [] or versions == [""]:
    error ("none of the sac2c versions matching `%s' were found" % sac2c_v)

# Only print the files and exit
if (dryrun):
    print "files to be removed:"
    for v in versions:
        for f in list_of_files (prefix, v):
            print "--remove `%s'" % f
        print
    exit (0)

# Ask a user if he/she really wants to delete those files
sys.stdout.write ("delete %r sac2c versions in %s [Y/N] " % (versions, prefix))
choice = raw_input ().lower ()
if choice in ["y", "ye", "yes", "aye", "i don't give shit"]:
    for v in versions:
        remove_version (prefix, v)
else:
    print "Nothing removed"
