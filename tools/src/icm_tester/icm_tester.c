#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <stdarg.h>

#define DBUG 0
#define BUFLEN 5000

void
CleanUp (char *prefix)
{
    char filename[BUFLEN];

    sprintf (filename, "%s.c", prefix);
    remove (filename);

    sprintf (filename, "%s.E", prefix);
    remove (filename);
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
        fprintf (stderr, "ERROR: System call failed (exit code %d)!\n\n", exit_code);
        CleanUp (prefix);
        exit (2);
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
    do {
        p = fgets (line, BUFLEN, file);
        fprintf (stdout, "%s", line);
    } while (p != NULL);

    return (file);
}

void
CreateCFile (char *prefix, int *use_shp, int *use_hid, int *use_unq)
{
    char header[] = "
#ifndef _SHP_
#define _SHP_ SCL /* SCL, AKS, AKD, AUD */
#endif

#ifndef _HID_
#define _HID_ HID /* NHD, HID */
#endif

#ifndef _UNQ_
#define _UNQ_ NUQ /* NUQ, UNQ */
#endif

#define TAGGED_ARRAYS
#define SAC_DO_MULTITHREAD 1
#include \"sac.h\"
                    ";

                    FILE
                    * file;
    char input[BUFLEN];

    file = OpenFile (prefix, ".c", "w");

#if 0
  fscanf( stdin, "%s", input);
#else
    {
        int ch;
        int pos = 0;
        while ((ch = getc (stdin)) != '\n') {
            input[pos++] = ch;
        }
        input[pos] = '\0';
    }
#endif

    fprintf (file, header);
    fprintf (file, input);
    fprintf (file, "\n\n");

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

                file = OpenFile (prefix, ".E", "r");

                file = SkipFileHeader (file, prefix);
                file = PrintFile (file, use_shp ? shp[i] : NULL, use_hid ? hid[j] : NULL,
                                  use_unq ? unq[k] : NULL);

                fclose (file);
            }
        }
    }
}

int
main ()
{
    int use_shp, use_hid, use_unq;
    char *tmpfileprefix = tempnam (NULL, "SAC_");

    fprintf (stdout, "Enter the H-ICM call to be tested:\n");
    fprintf (stdout,
             "(_SHP_, _HID_, _UNQ_ will be replaced by all possible tag combinations)\n");

    CreateCFile (tmpfileprefix, &use_shp, &use_hid, &use_unq);
    ProcessCFile (tmpfileprefix, use_shp, use_hid, use_unq);
    CleanUp (tmpfileprefix);

    return (0);
}
