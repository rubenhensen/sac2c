#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "config.h"

#define DBUG 0
#define USE_CB 1

#define BUFLEN 5000

void
RemoveFile (char *prefix, char *postfix)
{
    char filename[BUFLEN];

    sprintf (filename, "%s%s", prefix, postfix);
    remove (filename);
}

void
RenameFile (char *prefix, char *old_postfix, char *new_postfix)
{
    char filename1[BUFLEN];
    char filename2[BUFLEN];

    sprintf (filename1, "%s%s", prefix, old_postfix);
    sprintf (filename2, "%s%s", prefix, new_postfix);
    rename (filename1, filename2);
}

void
CleanUp (char *prefix)
{
    RemoveFile (prefix, ".c");
    RemoveFile (prefix, ".E");
}

int
SystemCall (char *format, ...)
{
    va_list arg_p;
    static char syscall[BUFLEN];
    int exit_code;

    va_start (arg_p, format);
    vsprintf (syscall, format, arg_p);
    va_end (arg_p);

#if DBUG
    fprintf (stderr, "Using syscall \"%s\"\n", syscall);
#endif

    exit_code = system (syscall);

    return (exit_code);
}

FILE *
OpenFile (char *prefix, char *postfix, char *mode)
{
    FILE *file;
    char filename[BUFLEN];

    sprintf (filename, "%s%s", prefix, postfix);
#if DBUG
    fprintf (stderr, "Opening file \"%s\"\n", filename);
#endif
    file = fopen (filename, mode);

    if (!file) {
        fprintf (stderr, "ERROR: File \"%s\" not found!\n\n", filename);
        CleanUp (prefix);
        exit (1);
    }

    return (file);
}

void
InvokeCC (char *prefix, char *shp, char *hid, char *unq)
{
    char cc[] = "gcc";

    int exit_code;

    exit_code = SystemCall ("%s -D_SHP_=%s -D_HID_=%s -D_UNQ_=%s -I %s/runtime"
                            " -o %s.E -E %s.c",
                            cc, shp, hid, unq, getenv ("SACBASE"), prefix, prefix);

    if (exit_code != 0) {
        fprintf (stderr, "ERROR: Call of C compiler failed (exit code %d)!\n\n",
                 exit_code);
        CleanUp (prefix);
        exit (2);
    }
}

void
InvokeCB (char *prefix)
{
    char cb[] = "cb";
    char cb_flags[] = "-r";

    int exit_code;

    exit_code
      = SystemCall ("%s %s %s.E 1> %s.cb 2> /dev/null", cb, cb_flags, prefix, prefix);

    if (exit_code != 0) {
        RemoveFile (prefix, ".cb");

        fprintf (stderr, "WARNING: Call of code beautifier failed (exit code %d)!\n\n",
                 exit_code);
    } else {
        RenameFile (prefix, ".cb", ".E");
    }
}

FILE *
SkipFileHeader (FILE *file, char *prefix)
{
    char line[BUFLEN];
    char *p;
    int cnt;
    char filename[BUFLEN];
    int line1, line2;

    p = fgets (line, BUFLEN, file);
    while (p != NULL) {
        cnt = sscanf (line, "# %d %s %d", &line1, filename, &line2);
        if ((cnt == 3) && (filename[0] == '\"')
            && (strncmp (&(filename[1]), prefix, strlen (prefix)) == 0) && (line1 > 1)) {
            break;
        }
        p = fgets (line, BUFLEN, file);
    }

    return (file);
}

void
PrintTag (char *tag)
{
    if (tag != NULL) {
        fprintf (stdout, " %s", tag);
    } else {
        fprintf (stdout, " ---");
    }
}

FILE *
PrintFile (FILE *file, char *shp, char *hid, char *unq)
{
    char line[BUFLEN];
    char *p;

    fprintf (stdout, "\n");
    fprintf (stdout, "/*\n");
    fprintf (stdout, " * ");
    PrintTag (shp);
    PrintTag (hid);
    PrintTag (unq);
    fprintf (stdout, "\n");
    fprintf (stdout, " */\n");
    p = fgets (line, BUFLEN, file);
    while (p != NULL) {
        fprintf (stdout, "%s", line);
        p = fgets (line, BUFLEN, file);
    }
    fprintf (stdout, "\n");

    return (file);
}

void
CreateCFile (char *prefix, int *use_shp, int *use_hid, int *use_unq)
{
    char header[] = "\n"
                    "#ifndef _SHP_\n"
                    "#define _SHP_ SCL  /* SCL, AKS, AKD, AUD */\n"
                    "#endif\n"
                    "\n"
                    "#ifndef _HID_\n"
                    "#define _HID_ HID  /* NHD, HID */\n"
                    "#endif\n"
                    "\n"
                    "#ifndef _UNQ_\n"
                    "#define _UNQ_ NUQ  /* NUQ, UNQ */\n"
                    "#endif\n"
                    "\n"
                    "\n"

                    "#define SAC_DO_CHECK           1\n"
                    "#define SAC_DO_CHECK_TYPE      1\n"
                    "#define SAC_DO_CHECK_BOUNDARY  1\n"
                    "#define SAC_DO_CHECK_MALLOC    1\n"
                    "#define SAC_DO_CHECK_ERRNO     1\n"
                    "#define SAC_DO_CHECK_HEAP      1\n"

                    "#define SAC_DO_PHM             0\n"
                    "#define SAC_DO_APS             0\n"
                    "#define SAC_DO_DAO             0\n"
                    "#define SAC_DO_MSCA            0\n"

                    "#define SAC_DO_PROFILE         1\n"
                    "#define SAC_DO_PROFILE_WITH    1\n"
                    "#define SAC_DO_PROFILE_FUN     1\n"
                    "#define SAC_DO_PROFILE_INL     1\n"
                    "#define SAC_DO_PROFILE_LIB     1\n"

                    "#define SAC_DO_TRACE           1\n"
                    "#define SAC_DO_TRACE_REF       1\n"
                    "#define SAC_DO_TRACE_MEM       1\n"
                    "#define SAC_DO_TRACE_PRF       1\n"
                    "#define SAC_DO_TRACE_FUN       1\n"
                    "#define SAC_DO_TRACE_WL        1\n"
                    "#define SAC_DO_TRACE_AA        1\n"
                    "#define SAC_DO_TRACE_MT        1\n"

                    "#define SAC_DO_CACHESIM        0\n"
                    "#define SAC_DO_CACHESIM_ADV    0\n"
                    "#define SAC_DO_CACHESIM_GLOBAL 1\n"
                    "#define SAC_DO_CACHESIM_FILE   0\n"
                    "#define SAC_DO_CACHESIM_PIPE   1\n"
                    "#define SAC_DO_CACHESIM_IMDT   0\n"

                    "#define SAC_DO_MULTITHREAD     1\n"
                    "#define SAC_DO_THREADS_STATIC  1\n"

                    "#define SAC_DO_COMPILE_MODULE  0\n"

                    "#include \"sac.h\"\n";

    FILE *file;
    char input[BUFLEN];

    file = OpenFile (prefix, ".c", "w");

#if 0
  fscanf( stdin, "%s", input);
#else
    {
        int ch;
        int pos = 0;
        while ((ch = getc (stdin)) != '\n') {
            input[pos++] = (char)ch;
        }
        input[pos] = '\0';
    }
#endif

    fprintf (file, "%s\n", header);
    fprintf (file, "%s\n", input);

    fclose (file);

    (*use_shp) = (strstr (input, "_SHP_") != NULL);
    (*use_hid) = (strstr (input, "_HID_") != NULL);
    (*use_unq) = (strstr (input, "_UNQ_") != NULL);
}

void
AdjustTags (char **tags, int use_tag)
{
    if (!use_tag) {
        tags[0] = "---";
        tags[1] = NULL;
    }
}

void
ProcessCFile (char *prefix, int use_shp, int use_hid, int use_unq)
{
    char *shp[] = {"SCL", "AKS", "AKD", "AUD", NULL};
    char *hid[] = {"NHD", "HID", NULL};
    char *unq[] = {"NUQ", "UNQ", NULL};

    FILE *file;
    int i, j, k;

    AdjustTags (shp, use_shp);
    AdjustTags (hid, use_hid);
    AdjustTags (unq, use_unq);
    for (i = 0; (shp[i] != NULL); i++) {
        for (j = 0; (hid[j] != NULL); j++) {
            for (k = 0; (unq[k] != NULL); k++) {
                InvokeCC (prefix, shp[i], hid[j], unq[k]);
#if USE_CB
                InvokeCB (prefix);
#endif

                file = OpenFile (prefix, ".E", "r");

                file = SkipFileHeader (file, prefix);
                file = PrintFile (file, use_shp ? shp[i] : NULL, use_hid ? hid[j] : NULL,
                                  use_unq ? unq[k] : NULL);

                fclose (file);
            }
        }
    }
}

char *
CreateTmpFileName (void)
{
    char *tmp_file;

#if HAVE_MKDTEMP
    /* mkdtemp is safer than tempnam and recommended */
    /* on linux/bsd platforms.                       */

    tmp_file = (char *)malloc (12);

    if (tmp_file == NULL) {
        fprintf (stderr, "System failed to create temporary file.");
    }

    tmp_file = strcpy (tmp_file, "SAC_XXXXXX");

    tmp_file = mkdtemp (tmp_file);

    if (tmp_file == NULL) {
        fprintf (stderr, "System failed to create temporary file.");
    }

#else /* HAVE_MKDTEMP */

    /* the old way for platforms not */
    /* supporting mkdtemp            */

    tmp_file = tempnam (NULL, "SAC_");

    if (tmp_file == NULL) {
        fprintf (stderr, "System failed to create temporary file.");
    }

#endif /* HAVE_MKDTEMP */

    return (tmp_file);
}

int
main (void)
{
    int use_shp, use_hid, use_unq;
    char *tmpfileprefix;

    tmpfileprefix = CreateTmpFileName ();

    fprintf (stdout, "Enter the H-ICM call to be tested:\n");
    fprintf (stdout,
             "(_SHP_, _HID_, _UNQ_ will be replaced by all possible tag combinations)\n");

    CreateCFile (tmpfileprefix, &use_shp, &use_hid, &use_unq);
    ProcessCFile (tmpfileprefix, use_shp, use_hid, use_unq);
    CleanUp (tmpfileprefix);

    return (0);
}
