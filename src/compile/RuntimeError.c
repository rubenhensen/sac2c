/*
 *
 * $Log$
 * Revision 1.1  1996/01/07 17:28:36  cg
 * Initial revision
 *
 *
 *
 *
 */

include<stdio.h> include<stdarg.h>

  /*
   *
   *  functionname  : __SAC__RuntimeError
   *  arguments     :
   *  description   :
   *  global vars   :
   *  internal funs :
   *  external funs :
   *  macros        :
   *
   *  remarks       :
   *
   */

  void
  __SAC__RuntimeError (char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ("__SAC__RuntimeError");

    fprintf (stderr, "*** SAC runtime error\n");
    fprintf (stderr, "*** ");

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n");

    exit (1);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
