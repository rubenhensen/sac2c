#include "str_buffer.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/* for memcpy */

#define DBUG_PREFIX "SBUF"
#include "debug.h"

#include "memory.h"
#include "str.h"

struct STR_BUF {
    char *buf;
    int pos;
    int size;
};

str_buf *
SBUFcreate (int size)
{
    str_buf *res;

    DBUG_ENTER ();

    res = (str_buf *)MEMmalloc (sizeof (str_buf));
    res->buf = (char *)MEMmalloc (size * sizeof (char));
    res->buf[0] = '\0';
    res->pos = 0;
    res->size = size;

    DBUG_PRINT ("allocating buffer size %d : %p", size, res);

    DBUG_RETURN (res);
}

static str_buf *
EnsureStrBufSpace (str_buf *s, int len)
{
    int new_size;
    char *new_buf;

    DBUG_ENTER ();

    if ((len + 1) > (s->size - s->pos)) {

        new_size = (len >= s->size ? s->size + 2 * len : 2 * s->size);

        DBUG_PRINT ("increasing buffer %p from size %d to size %d", s, s->size, new_size);

        new_buf = (char *)MEMmalloc (new_size * sizeof (char));
        memcpy (new_buf, s->buf, s->pos + 1);
        s->buf = MEMfree (s->buf);
        s->buf = new_buf;
        s->size = new_size;
    }

    DBUG_RETURN (s);
}

str_buf *
SBUFprint (str_buf *s, const char *string)
{
    int len;

    DBUG_ENTER ();

    len = STRlen (string);

    s = EnsureStrBufSpace (s, len);

    s->pos += sprintf (&s->buf[s->pos], "%s", string);
    DBUG_PRINT ("pos of buffer %p now is %d", s, s->pos);

    DBUG_RETURN (s);
}

str_buf *
SBUFprintf (str_buf *s, const char *format, ...)
{
    va_list arg_p;
    int len = 0, rem;
    bool ok;

    DBUG_ENTER ();

    ok = FALSE;

    while (!ok) {
        rem = s->size - s->pos;

        va_start (arg_p, format);
        len = vsnprintf (&s->buf[s->pos], rem, format, arg_p);
        va_end (arg_p);

        if ((len >= 0) && (len < rem)) {
            ok = TRUE;
        } else {
            if (len < 0) {
                len = 2 * (s->size + 10);
            }
            s = EnsureStrBufSpace (s, len);
        }
    }

    s->pos += len;

    DBUG_RETURN (s);
}

char *
SBUF2str (str_buf *s)
{
    DBUG_ENTER ();

    DBUG_RETURN (STRcpy (s->buf));
}

int
SBUFlen (str_buf *s)
{
    DBUG_ENTER ();

    DBUG_RETURN (s->pos);
}

void
SBUFflush (str_buf *s)
{
    DBUG_ENTER ();

    s->pos = 0;
    DBUG_PRINT ("pos of buffer %p reset to %d", s, s->pos);

    DBUG_RETURN ();
}

bool
SBUFisEmpty (str_buf *s)
{
    DBUG_ENTER ();

    DBUG_RETURN (s->pos == 0);
}

char *
SBUFgetBuffer (str_buf *s)
{
    DBUG_ENTER ();

    DBUG_RETURN (s->buf);
}

char *
SBUF2strAndFree (str_buf **s)
{
    DBUG_ENTER ();

    char *result = (*s)->buf;
    *s = MEMfree (*s);

    DBUG_RETURN (result);
}

str_buf *
SBUFfree (str_buf *s)
{
    DBUG_ENTER ();

    s->buf = MEMfree (s->buf);
    s = MEMfree (s);

    DBUG_RETURN (s);
}

#undef DBUG_PREFIX
