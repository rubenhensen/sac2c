#include "str_buffer.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
/* for memcpy */

#define DBUG_PREFIX "SBUF"
#include "debug.h"

#include "ctinfo.h"
#include "memory.h"
#include "str.h"

struct STR_BUF {
    char *buf;
    size_t len;
    size_t size;
};


/** <!--********************************************************************-->
 *
 * @fn str_buf *SBUFcreate( size_t size)
 *
 *   @brief  Creates a new string buffer with enough memory allocated for 
 *           size characters and a null byte.
 * 
 *   @param  size The number of characters the buffer should allocate, 
 *                excluding the null byte.
 *
 *   @return The newly created string buffer.
 *
 ******************************************************************************/
str_buf *
SBUFcreate (size_t size)
{
    str_buf *res;

    DBUG_ENTER ();
    size += 1; // +1 for the null byte.
    res = (str_buf *)MEMmalloc (sizeof (str_buf));

    DBUG_PRINT ("allocating buffer with size %zu : %p", size, (void *)res);

    res->buf = (char *)MEMmalloc (size * sizeof (char));
    res->buf[0] = '\0';
    res->len = 0; // Length of the stored string, excluding the null byte.
    res->size = size; // Null byte is included in the allocated size.


    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn str_buf *EnsureStrBufSpace( str_buf *s, size_t len)
 *
 *   @brief  Ensures that the buffer has enough room for an 
 *           additional len characters to be written.
 *
 *           NOTE: At this stage, we don't need to allocate space for the null 
 *                 byte because it already exists. It only has to be moved/rewritten
 *                 at the end of the string that's written with other functions.
 * 
 *   @param  s   The buffer.
 *   @param  len The number of characters that should be available to write.
 * 
 *   @return The given string buffer.
 *
 ******************************************************************************/
static str_buf *
EnsureStrBufSpace (str_buf *s, size_t len)
{
    size_t new_size;
    char *new_buf;

    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");

    // s->size - (s->len + 1) is equivalent to allocated space minus used space, i.e. available space
    if (len > (s->size - (s->len + 1))) {

        // Guarantees: globally s->size >=1, and in the scope, len >=1
        // new_size will always fit len characters, a null byte and possibly more.
        new_size = (len >= s->size ? s->size + 2 * len : 2 * s->size);

        DBUG_PRINT ("increasing buffer %p from size %zu to size %zu", (void *)s, s->size, new_size);

        new_buf = (char *)MEMmalloc (new_size * sizeof (char));
        memcpy (new_buf, s->buf, s->len + 1); // +1 is to also copy over the null byte
        s->buf = MEMfree (s->buf);
        s->buf = new_buf;
        s->size = new_size;
    }

    DBUG_RETURN (s);
}


/** <!--********************************************************************-->
 *
 * @fn str_buf *SBUFprint( str_buf *s, const char *string)
 *
 *   @brief  Append the given string to the buffer.
 *
 *   @param  s      The buffer.
 *   @param  string The string to append to the buffer.
 * 
 *   @return The given string buffer.
 *
 ******************************************************************************/
str_buf *
SBUFprint (str_buf *s, const char *string)
{
    size_t len;

    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");
    DBUG_ASSERT (string != NULL, "Expected the string to be non-null");

    len = STRlen (string);
    s = EnsureStrBufSpace (s, len);
    s->len += (size_t)sprintf (&s->buf[s->len], "%s", string);
    DBUG_PRINT ("len of buffer %p is now %zu", (void *)s, s->len);

    DBUG_RETURN (s);
}

/** <!--********************************************************************-->
 *
 * @fn str_buf *SBUFprintd( str_buf *s, char *string)
 *
 *   @brief  Append the given string to the buffer, then deallocate the string.
 *
 *   @param  s      The buffer.
 *   @param  string The string to append to the buffer.
 * 
 *   @return The given string buffer.
 *
 ******************************************************************************/
str_buf *
SBUFprintd (str_buf *s, char *string)
{
    size_t len;

    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");
    DBUG_ASSERT (string != NULL, "Expected the string to be non-null");

    len = STRlen (string);
    s = EnsureStrBufSpace (s, len);
    s->len += (size_t)sprintf (&s->buf[s->len], "%s", string);
    DBUG_PRINT ("len of buffer %p is now %zu", (void *)s, s->len);
    MEMfree (string);

    DBUG_RETURN (s);
}

/** <!--********************************************************************-->
 *
 * @fn str_buf *SBUFprintf( str_buf *s, const char *format, ...)
 *
 *   @brief  Applies the extra arguments to the given format string and appends 
 *           the result to the buffer.
 *
 *   @param  s      The buffer.
 *   @param  format The format string on which the extra arguments are applied.
 * 
 *   @return The given string buffer.
 *
 ******************************************************************************/
str_buf *
SBUFprintf (str_buf *s, const char *format, ...)
{
    va_list arg_p;

    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");
    DBUG_ASSERT (format != NULL, "Expected the format to be non-null");

    va_start (arg_p, format);
    s = SBUFvprintf (s, format, arg_p);
    va_end (arg_p);

    DBUG_RETURN (s);
}

/** <!--********************************************************************-->
 *
 * @fn str_buf *SBUFvprintf( str_buf *s, const char *format, va_list arg_list)
 *
 *   @brief  Applies the arguments captured in arg_list to the format string
 *           and appends the result to the buffer.
 *
 *   @param s        The buffer.
 *   @param format   The format string on which the arg_list is applied.
 *   @param arg_list The list of arguments to apply to the format string.
 * 
 *   @return The given string buffer.
 *
 ******************************************************************************/
str_buf *
SBUFvprintf (str_buf *s, const char *format, va_list arg_list)
{
    va_list arg_list_copy;
    int required_space;
    size_t available_space, required_space_p;
    bool ok = false;

    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");
    DBUG_ASSERT (format != NULL, "Expected the format to be non-null");

    do {
        available_space = s->size - s->len;

        // Get the required space (excludes null byte) by attempting to append to the buffer.
        // If there is enough space and no errors occur, this already appends to the buffer.
        va_copy (arg_list_copy, arg_list);
        required_space = vsnprintf (&s->buf[s->len], available_space, format, arg_list_copy);
        required_space_p = (size_t)required_space;
        va_end (arg_list_copy);

        // Deal with encoding errors - abort
        if (required_space < 0) {
            // We can't print the format string or the arguments since formatting them lead to a failure.
            CTIerrorInternal ("Encoding of a format string into a string buffer failed.");
            CTIabortOnError ();
        }
        
        // Make more space if necessary
        if (required_space_p + 1 > available_space) { // + 1 for the null byte
            s = EnsureStrBufSpace (s, required_space_p);
        } else { // No space is necessary, vsnprintf successfully wrote to the buffer.
            ok = true;
        }
    } while (!ok);
    
    s->len += required_space_p;

    DBUG_PRINT ("len of buffer %p is now %zu", (void *)s, s->len);

    DBUG_RETURN (s);
}


/** <!--********************************************************************-->
 *
 * @fn str_buf *SBUFsubstToken( str_buf *s, const char *token, const char *subst)
 *
 *   @brief  Substitutes all occurrences of token in the string buffer with subst.
 *
 *   @param  s     The buffer.
 *   @param  token The string to replace.
 *   @param  subst The string to replace the token with.
 * 
 *   @return The given string buffer.
 *
 ******************************************************************************/
str_buf *
SBUFsubstToken(str_buf *s, const char *token, const char *subst)
{
    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");
    DBUG_ASSERT (token != NULL, "Expected the token to be non-null");
    DBUG_ASSERT (subst != NULL, "Expected the subst parameter to be non-null");

    // STRsubstTokend deallocates s->buf, so s->buf is not leaked.
    s->buf = STRsubstTokend (s->buf, token, subst); 
    s->len = STRlen (s->buf);
    s->size = s->len + 1; // +1 for the null byte.

    DBUG_RETURN (s);
}

/** <!--********************************************************************-->
 *
 * @fn str_buf *SBUFinsertAfterToken( str_buf *s, const char *token, const char *insert)
 *
 *   @brief  Inserts insert after all occurrences of token in the string buffer.
 *
 *   @param  s      The buffer.
 *   @param  token  The string after which to insert.
 *   @param  insert The string to insert after each occurrence of token
 * 
 *   @return The given string buffer.
 *
 ******************************************************************************/
str_buf *
SBUFinsertAfterToken(str_buf *s, const char *token, const char *insert)
{
    char *subst;
    
    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");
    DBUG_ASSERT (token != NULL, "Expected the token to be non-null");
    DBUG_ASSERT (insert != NULL, "Expected the insert parameter to be non-null");

    subst = STRcat (token, insert);
    SBUFsubstToken (s, token, subst);

    MEMfree (subst);
    DBUG_RETURN (s);
}



/** <!--********************************************************************-->
 *
 * @fn char *SBUF2str( str_buf *s)
 *
 *   @brief  Returns a copy of the string of the string buffer
 *
 *   @param  s The buffer.
 * 
 *   @return A copy of the string contained in the internal buffer.
 *
 ******************************************************************************/
char *
SBUF2str (str_buf *s)
{
    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");

    DBUG_RETURN (STRcpy (s->buf));
}

/** <!--********************************************************************-->
 *
 * @fn size_t *SBUFlen( str_buf *s)
 *
 *   @brief  Returns the length of the string buffer.
 *
 *   @param  s The buffer.
 * 
 *   @return The length (excl. null byte) of the string stored in the string buffer.
 *
 ******************************************************************************/
size_t
SBUFlen (str_buf *s)
{
    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");

    DBUG_RETURN (s->len);
}

/** <!--********************************************************************-->
 *
 * @fn void *SBUFflush( str_buf *s)
 *
 *   @brief  Resets the string in the buffer so the allocated space can be reused.
 * 
 *           NOTE: This does NOT deallocate anything. If you want to free memory,
 *                 use SBUF2strAndFree or SBUFfree!
 *
 *   @param  s The buffer.
 * 
 ******************************************************************************/
void
SBUFflush (str_buf *s)
{
    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");

    s->len = 0;
    s->buf[0] = '\0';
    DBUG_PRINT ("len of buffer %p reset to %zu", (void *)s, s->len);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void *SBUFtruncate( str_buf *s, size_t length)
 *
 *   @brief  Truncates the string in the buffer to be a maximum of length 
 *           characters long.
 * 
 *           If the provided length is larger than the current length, 
 *           no action is taken.
 *
 *   @param  s      The buffer.
 *   @param  length The length of the string in the buffer after this function. 
 * 
 ******************************************************************************/
void
SBUFtruncate (str_buf *s, size_t length)
{
    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");

    if (length >= s->len) {
        DBUG_RETURN ();
    }

    s->len = length;
    s->buf[length] = '\0';
    DBUG_PRINT ("len of buffer %p truncated to %zu", (void *)s, s->len);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn bool *SBUFisEmpty( str_buf *s)
 *
 *   @brief  Returns whether string in the buffer has a length of 0.
 *
 *   @param  s The buffer.
 * 
 *   @return True if the string in the buffer has a length of 0, false otherwise.
 *
 ******************************************************************************/
bool
SBUFisEmpty (str_buf *s)
{
    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");

    DBUG_RETURN (s->len == 0);
}

/** <!--********************************************************************-->
 *
 * @fn char *SBUFgetBuffer( str_buf *s)
 *
 *   @brief  Returns the internal string buffer.
 *
 *   @param  s The buffer.
 * 
 *   @return The internal string buffer.
 *
 ******************************************************************************/
char *
SBUFgetBuffer (str_buf *s)
{
    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");

    DBUG_RETURN (s->buf);
}

/** <!--********************************************************************-->
 *
 * @fn char *SBUF2strAndFree( str_buf **s)
 *
 *   @brief  Frees str_buf, setting the pointer to NULL, and returns the internal 
 *           string buffer as a string without copying it.
 *
 *   @param  s The pointer to the buffer.
 * 
 *   @return The internal string buffer.
 *
 ******************************************************************************/
char *
SBUF2strAndFree (str_buf **s)
{
    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the pointer to the buffer to be non-null");
    DBUG_ASSERT (*s != NULL, "Expected the buffer to be non-null");

    char *result = (*s)->buf;
    *s = MEMfree (*s);

    DBUG_RETURN (result);
}


/** <!--********************************************************************-->
 *
 * @fn str_buf *SBUFfree( str_buf *s)
 *
 *   @brief  Frees the str_buf and the internal buffer.
 *
 *   @param  s The buffer.
 * 
 *   @return NULL.
 *
 ******************************************************************************/
str_buf *
SBUFfree (str_buf *s)
{
    DBUG_ENTER ();
    DBUG_ASSERT (s != NULL, "Expected the buffer to be non-null");

    s->buf = MEMfree (s->buf);
    s = MEMfree (s);

    DBUG_RETURN (s);
}

#undef DBUG_PREFIX
