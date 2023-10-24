/**
 * @file
 *
 * @brief A collection of functions for string manipulation.
 */
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "str.h"

#include "memory.h"

#define DBUG_PREFIX "STR"
#include "debug.h"

#include "math_utils.h"
#include "str_buffer.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "shape.h"
#include "namespaces.h"

/**
 * @brief Modify string by turning all characters from start to stop-1
 *        into upper case.
 *
 * @param source source string
 * @param start index to start at
 * @param stop index to stop at
 */
void
STRtoupper (char *source, size_t start, size_t stop)
{
    size_t i;

    DBUG_ENTER ();

    if (source != NULL) {
        for (i = start; i < stop; i++) {
            source[i] = toupper (source[i]);
        }
    }

    DBUG_RETURN ();
}

/**
 * @brief Copy string and allocate memory for new string.
 *
 * @param source string to copy
 * @return new copied string
 */
char *
STRcpy (const char *source)
{
    size_t len;
    char *ret;

    DBUG_ENTER ();

    if (source != NULL) {
        len = STRlen (source) + 1; // including null char
        ret = MEMmalloc (sizeof (char) * len);
        strncpy (ret, source, len);
    } else {
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

/**
 * @brief Copy string and allocate memory for new string. Copy only maxlen
 *        characters.
 *
 * @param source string to copy
 * @param maxlen number of characters to copy
 * @return new copied string
 */
char *
STRncpy (const char *source, size_t maxlen)
{
    char *ret;
    size_t max;

    DBUG_ENTER ();

    if (source != NULL) {
        max = STRlen (source);
        if (max > maxlen) {
            max = maxlen;
        }

        ret = MEMmalloc (sizeof (char) * (max + 1));
        strncpy (ret, source, max);

        /* make sure string ends with 0 */
        ret[max] = '\0';
    } else {
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

/**
 * @brief copy part of a string from start to start + len.
 *        if len is <0 then len is relative to the length of the string.
 *
 * @param string string to copy from
 * @param start index to start from
 * @param len total length to take
 * @return copied part of string
 */
char *
STRsubStr (const char *string, size_t start, ssize_t len)
{
    size_t strlen = 0;
    char *ret = NULL;

    DBUG_ENTER ();

    strlen = STRlen (string);

    // Normalizing len against the length of `str`.
    size_t l = len < 0
               ? MATH_MAX (0, (ssize_t)strlen + len)
               : len;

    if ((start + l) > strlen) { /* to long take what we can */
        l = strlen < start ? 0 : strlen - start;
    }

    if (l == 0) {
        ret = STRnull ();
    } else {
        ret = memcpy (MEMmalloc (sizeof (char) * (l + 1)),
                      string + start, /* move to start of sub string */
                      l);
        ret[l] = '\0';
    }

    DBUG_RETURN (ret);
}

/**
 * @brief return an empty string
 *
 * @return empty "" string
 */
char *
STRnull ()
{
    char *ret = NULL;
    DBUG_ENTER ();

    ret = MEMmalloc (sizeof (char) * 1);
    ret[0] = '\0';

    DBUG_RETURN (ret);
}

/**
 * @brief return the size of an int when the int is a string.
 *        does not count the sine (+-)!!
 *
 * @return size of int in string
 */
size_t
STRsizeInt ()
{
    size_t size = 0;
    int s = sizeof (int) * 8;
    DBUG_ENTER ();

    switch (s) {
    case 8:
        size = 3;
        break;
    case 16:
        size = 5;
        break;
    case 32:
        size = 10;
        break;
    case 64:
        size = 19;
        break;
    case 128:
        size = 39;
        break;
    }

    DBUG_ASSERT (size != 0, "Can not work out the size of an int when in a string");

    DBUG_RETURN (size);
}

/**
 * @brief Concatenate two strings and allocate memory for new string.
 *
 * @param first first string
 * @param second second string
 *
 * @return new concatenated string
 */
char *
STRcat (const char *first, const char *second)
{
    char *result;

    DBUG_ENTER ();

    if (first == NULL) {
        result = STRcpy (second);
    } else if (second == NULL) {
        result = STRcpy (first);
    } else {
        result = MEMmalloc (STRlen (first) + STRlen (second) + 1);

        strcpy (result, first);
        strcat (result, second);
    }

    DBUG_RETURN (result);
}

/**
 * @brief Concatenate N strings and allocate memory for new string.
 *
 * @param n number of strings
 * @param ... n amount of "const char *"-type strings
 *
 * @return new concatenated string
 */
char *
STRcatn (int n, ...)
{
    int i;
    size_t length;
    char *result;
    const char *ptr;
    va_list arg_list;

    DBUG_ENTER ();

    DBUG_ASSERT (n > 2, "STRcatn called with %d arguments", n);

    va_start (arg_list, n);

    length = 0;

    for (i = 0; i < n; ++i) {
        ptr = va_arg (arg_list, const char *);
        if (ptr != NULL) {
            length += STRlen (ptr);
        }
    }

    va_end (arg_list);

    if (length == 0) {
        result = NULL;
    } else {
        result = MEMmalloc (length + 1);
        result[0] = '\0';

        va_start (arg_list, n);

        for (i = 0; i < n; ++i) {
            ptr = va_arg (arg_list, const char *);
            if (ptr != NULL) {
                strcat (result, ptr);
            }
        }

        va_end (arg_list);
    }

    DBUG_RETURN (result);
}

/** 
 * @brief Constructs a format string based on the format and list of arguments.
 *        Making sure that the number of arguments matches the 
 *        format string is the responsibility of the caller.
 *
 * @param format The format string on which to apply the arguments.
 * @param arg_list The list of arguments to apply to the format string.
 *
 * @return The processed format string.
 */
static char *
vformat (const char *format, va_list arg_list)
{
    str_buf *buf;
  
    DBUG_ENTER ();
  
    buf = SBUFcreate (0);
    SBUFvprintf (buf, format, arg_list);

    DBUG_RETURN (SBUF2strAndFree (&buf));
}

/** 
 * @brief Constructs a format string based on the format and list of arguments.
 *        Making sure that the number of arguments matches the 
 *        format string is the responsibility of the caller.
 *
 * @param format The format string on which to apply the arguments.
 *
 * @return The processed format string/
 */
char *
STRformat (const char *format, ...)
{
    va_list arg_list;
    char *res;

    DBUG_ENTER();

    va_start (arg_list, format);
    res = vformat (format, arg_list);
    va_end (arg_list);

    DBUG_RETURN (res);
}

/**
 * @brief Compare two strings.
 *
 * @param first first string to compare
 * @param second second string to compare
 *
 * @return TRUE, string contents are equal or
 *         FALSE, string contents are not equal
 */
bool
STReq (const char *first, const char *second)
{
    bool res;

    DBUG_ENTER ();

    if ((first == NULL) && (second == NULL)) {
        res = TRUE;
    } else if ((first == NULL) || (second == NULL)) {
        res = FALSE;
    } else {
        res = (strcmp (first, second) == 0);
    }

    DBUG_RETURN (res);
}

/**
 * @brief Compare two strings.
 *
 * @param first first string to compare
 * @param second second string to compare
 *
 * @return TRUE, string contents are equal or
 *         FALSE, string contents are not equal
 */
bool
STRgt (const char *first, const char *second)
{
    bool res;

    DBUG_ENTER ();

    if ((first == NULL) && (second == NULL)) {
        res = TRUE;
    } else if ((first == NULL) || (second == NULL)) {
        res = FALSE;
    } else {
        res = (strcmp (first, second) > 0);
    }

    DBUG_RETURN (res);
}

/**
 * @brief Compare two strings for referring to the same hexadecimal number
 *
 * @param first first string to compare
 * @param second second string to compare
 * @return TRUE if string contents are equal, otherwise FALSE
 */
bool
STReqhex (const char *first, const char *second)
{
    bool res = TRUE;

    DBUG_ENTER ();

    if ((first == NULL) && (second == NULL)) {
        res = TRUE;
    } else if ((first == NULL) || (second == NULL)) {
        res = FALSE;
    } else {
        if ((*first == '-') && (*second == '-')) {
            first += 3;
            second += 3;
        } else if ((*first != '-') && (*second != '-')) {
            first += 2;
            second += 2;
        } else {
            res = FALSE;
        }

        if (res) {
            while (*first == '0') {
                first++;
            }

            while (*second == '0') {
                second++;
            }

            res = STReqci (first, second);
        }
    }

    DBUG_RETURN (res);
}

/**
 * @brief Compare two strings for referring to the same octal number
 *
 * @param first first string to compare
 * @param second second string to compare
 * @return TRUE if string contents are equal, otherwise FALSE
 */
bool
STReqoct (const char *first, const char *second)
{
    bool res = TRUE;

    DBUG_ENTER ();

    if ((first == NULL) && (second == NULL)) {
        res = TRUE;
    } else if ((first == NULL) || (second == NULL)) {
        res = FALSE;
    } else {
        if ((*first == '-') && (*second == '-')) {
            first += 2;
            second += 2;
        } else if ((*first != '-') && (*second != '-')) {
            second++;
            first++;
        } else {
            res = FALSE;
        }

        if (res) {
            while (*first == '0') {
                first++;
            }

            while (*second == '0') {
                second++;
            }

            res = STReq (first, second);
        }
    }

    DBUG_RETURN (res);
}

/**
 * @brief Compare two strings in a case insensitive way.
 *
 * @param first first string to compare
 * @param second second string to compare
 * @return TRUE if string contents are equal, otherwise FALSE
 */
bool
STReqci (const char *first, const char *second)
{
    bool res;
    int i;

    DBUG_ENTER ();

    if ((first == NULL) && (second == NULL)) {
        res = TRUE;
    } else if ((first == NULL) || (second == NULL)) {
        res = FALSE;
    } else {
        i = 0;
        while ((first[i] != '\0') && (second[i] != '\0')
               && (tolower (first[i]) == tolower (second[i]))) {
            i += 1;
        }
        if ((first[i] == '\0') && (second[i] == '\0')) {
            res = TRUE;
        } else {
            res = FALSE;
        }
    }

    DBUG_RETURN (res);
}

/**
 * @brief Compare two strings.
 *
 * @param first first string to compare
 * @param second second string to compare
 * @param n number of relevant characters
 * @return TRUE if relevant prefixes of strings are equal, or FALSE if
 *         relevant prefixes of strings are not equal
 */
bool
STReqn (const char *first, const char *second, size_t n)
{
    bool res;

    DBUG_ENTER ();

    if ((first == NULL) && (second == NULL)) {
        res = TRUE;
    } else if ((first == NULL) || (second == NULL)) {
        if (n == 0) {
            res = TRUE;
        } else {
            res = FALSE;
        }
    } else {
        res = (0 == strncmp (first, second, n));
    }

    DBUG_RETURN (res);
}

/**
 * @brief Checks if prefix is prefix of str
 *
 * @param prefix first string to compare
 * @param str second string to compare
 * @return TRUE if prefix is prefix of str, otherwise FALSE
 */
bool
STRprefix (const char *prefix, const char *str)
{
    bool res;

    DBUG_ENTER ();

    if (prefix == NULL) {
        res = TRUE;
    } else {
        if (str == NULL) {
            res = FALSE;
        } else {
            size_t plen = STRlen (prefix);

            if (STRlen (str) < plen) {
                res = FALSE;
            } else {
                res = (0 == strncmp (prefix, str, STRlen (prefix)));
            }
        }
    }

    DBUG_RETURN (res);
}

/**
 * @brief check if suffix is present in string
 *
 * @param suffix string to find
 * @param str string being scanned through
 * @return TRUE if suffix is the end of str, otherwise FALSE
 */
bool
STRsuffix (const char *suffix, const char *str)
{
    bool res = FALSE;

    DBUG_ENTER ();

    if (STRlen (suffix) > STRlen (str)) {
        res = FALSE;
    } else {
        str = str + STRlen (str) - STRlen (suffix);
        res = (0 == strcmp (str, suffix));
    }

    DBUG_RETURN (res);
}

/*******************************************************************************
 *
 * @fn void STRsub( const char *sub, const char *str)
 * 
 *   @brief  Checks is sub is a substring of str.
 *
 *   @param  sub the substring to look for
 *   @param  str the string to find substrings in
 * 
 *   @return TRUE if sub is substring of str or sub is NULL, FALSE otherwise
 ******************************************************************************/
bool
STRsub (const char *sub, const char *str)
{
    bool res;

    DBUG_ENTER ();

    if (sub == NULL) {
        res = TRUE;
    } else {
        if (str == NULL) {
            res = FALSE;
        } else {
            res = (NULL != strstr (str, sub));
        }
    }

    DBUG_RETURN (res);
}

/**
 * @brief Yield length of string
 *
 * Mostly to provide a complete interface and to avoid using standard
 * string facilities throughout the compiler.
 *
 * @param s string to calculate length
 * @return length of string, or 0 when NULL or ""
 */
size_t
STRlen (const char *s)
{
    size_t len;

    DBUG_ENTER ();

    if (s == NULL) {
        len = 0;
    } else if (*s == '\0') {
        len = 0;
    } else {
        len = strlen (s);
    }

    DBUG_RETURN (len);
}

static bool
CharInString (char c, const char *str)
{
    int i;
    bool res;

    DBUG_ENTER ();

    if ((str == NULL) || (c == '\0')) {
        res = FALSE;
    } else {
        i = 0;
        while ((str[i] != '\0') && (str[i] != c)) {
            i += 1;
        }
        res = str[i] != '\0';
    }

    DBUG_RETURN (res);
}

/**
 * @brief Tokenize string.
 *
 * For the first call, the str will be copied to internal static variable.
 * For further calls, str should be NULL. 
 * The last call frees the copy that was made during the first call.
 *
 * In contrast to strtok, STRtok leaves the argument string untouched and always
 * allocates the tokens in fresh memory.
 *
 * @param str string to tokenize
 * @param tok tokenizer
 * @return A pointer to the next token, or NULL iff there are no more tokens.
 */
char *
STRtok (const char *first, const char *sep)
{
    static char *keep_string = NULL;
    static char *current = NULL;
    char *ret;
    int i;

    DBUG_ENTER ();

    if (first != NULL) {
        if (keep_string != NULL) {
            keep_string = MEMfree (keep_string);
        }
        keep_string = STRcpy (first);
        current = keep_string;
    }

    if (current == NULL) {
        keep_string = MEMfree (keep_string);
        ret = NULL;
    } else {
        i = 0;
        while ((current[i] != '\0') && !CharInString (current[i], sep)) {
            i += 1;
        }

        if (current[i] == '\0') {
            ret = STRcpy (current);
            current = NULL;
        } else {
            current[i] = '\0';
            ret = STRcpy (current);
            current += i + 1;
        }
    }

    DBUG_RETURN (ret);
}

/**
 * @brief Yields either the argument string if it is not NULL or an empty constant
 *        string otherwise.
 *
 * This is helpful for instance when printing strings with the %s conversion
 * specifier and the string to print may be NULL.
 *
 * @param alt alternative string to return
 * @param str string to check for NULL
 * @return str or alt iff str is NULL
 */
char *
STRonNull (char *alt, char *str)
{
    char *res;

    DBUG_ENTER ();

    if (str == NULL) {
        res = alt;
    } else {
        res = str;
    }

    DBUG_RETURN (res);
}

/**
 * @brief Convert integer to string in decimal representation.
 *
 * @param number number to convert
 * @return new allocated string representation of number
 */
char *
STRitoa (int number)
{
    char *str;
    int num;

    DBUG_ENTER ();

    str = MEMmalloc (sizeof (int) * 4);
    num = snprintf (str, (sizeof (int) * 4) - 1, "%d", number);
    DBUG_ASSERT ((unsigned)num < (sizeof (int) * 4) - 1, "Trouble in STRitoa");

    DBUG_RETURN (str);
}

/**
 * @brief Convert integer to string in octal representation.
 *
 * @param number number to convert
 * @return new allocated string representation of number
 */
char *
STRitoa_oct (int number)
{
    char *str;
    int tmp;
    size_t length;
    int base = 8;

    DBUG_ENTER ();

    tmp = number;
    length = 1;
    while (tmp >= base) {
        tmp /= base;
        length++;
    }

    str = MEMmalloc (sizeof (char) * length + 3);

    sprintf (str, "0%o", number);

    DBUG_RETURN (str);
}

/**
 * @brief Convert integer to string in hexadecimal representation.
 *
 * @param number number to convert
 * @return new allocated string representation of number
 */
char *
STRitoa_hex (int number)
{
    char *str;
    int tmp;
    size_t length;
    int base = 16;

    DBUG_ENTER ();

    tmp = number;
    length = 1;
    while (tmp >= base) {
        tmp /= base;
        length++;
    }

    str = MEMmalloc (sizeof (char) * length + 3);

    sprintf (str, "0x%x", number);

    DBUG_RETURN (str);
}

static unsigned char
Hex2Dig (const char x)
{
    unsigned char res;

    DBUG_ENTER ();

    if ((x >= '0') && (x <= '9')) {
        res = x - '0';
    } else {
        res = 10 + x - 'A';
    }

    DBUG_RETURN (res);
}

/**
 * @brief Convert hex-string to byte array.
 *
 * @param array converted byte array memory must be allocated before
 *              calling this function
 * @param string string to convert
 * @return converted byte array
 */
unsigned char *
STRhex2Bytes (unsigned char *array, const char *string)
{
    int pos;
    unsigned char low, high;

    DBUG_ENTER ();

    pos = 0;

    while (string[pos * 2] != 0) {
        low = Hex2Dig (string[pos * 2 + 1]);
        high = Hex2Dig (string[pos * 2]);

        array[pos] = high * 16 + low;
        pos++;
    }

    DBUG_RETURN (array);
}

static char
Dig2Hex (unsigned char x)
{
    char res;

    DBUG_ENTER ();

    if (x < 10) {
        res = '0' + x;
    } else {
        res = 'A' + x - 10;
    }

    DBUG_RETURN (res);
}

/**
 * @brief Convert byte array to hex-string.
 *
 * @param len length of byte array
 * @param array array to convert
 * @return new allocated string representation of byte array
 */
char *
STRbytes2Hex (size_t len, unsigned char *array)
{
    size_t pos;
    char *result;
    unsigned char low, high;

    DBUG_ENTER ();

    result = MEMmalloc ((1 + len * 2) * sizeof (char));

    for (pos = 0; pos < len; pos++) {
        low = array[pos] % 16;
        high = array[pos] / 16;

        result[2 * pos] = Dig2Hex (high);
        result[2 * pos + 1] = Dig2Hex (low);
    }

    result[2 * len] = '\0';

    DBUG_RETURN (result);
}

/**
 * @brief Replaces special characters such that they can be used as identifiers
 *        in a C program.
 *
 * @param name string to be scanned
 * @return string with special characters replaced
 */
char *
STRreplaceSpecialCharacters (const char *name)
{
    char *new_name;
    char *tmp;
    size_t i, j;

    DBUG_ENTER ();

    if (name == NULL) {
        new_name = NULL;
    } else {

        new_name = MEMmalloc (1 + (3 * STRlen (name)) * sizeof (char));
        new_name[0] = '\0';

        for (i = 0, j = 0; i < STRlen (name); i++, j++) {
            switch (name[i]) {
            case '.':
                tmp = "_DO";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case ',':
                tmp = "_CM";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '+':
                tmp = "_PL";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '-':
                tmp = "_MI";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '*':
                tmp = "_ST";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '/':
                tmp = "_DI";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '%':
                tmp = "_PR";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '<':
                tmp = "_LT";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '>':
                tmp = "_GT";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '=':
                tmp = "_EQ";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '&':
                tmp = "_AM";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '|':
                tmp = "_VE";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '!':
                tmp = "_EX";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '[':
                tmp = "_BL";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case ']':
                tmp = "_BR";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '~':
                tmp = "_TI";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '@':
                tmp = "_AT";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '#':
                tmp = "_HA";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '$':
                tmp = "_DO";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '^':
                tmp = "_PO";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '\\':
                tmp = "_BS";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case ':':
                tmp = "_CL";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case ' ':
                tmp = "_SP";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '{':
                tmp = "_CO";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            case '}':
                tmp = "_CC";
                strcat (&(new_name[j]), tmp);
                j += STRlen (tmp) - 1;
                break;
            default:
                new_name[j] = name[i];
                new_name[j + 1] = '\0';
                break;
            }
        }
    }

    DBUG_RETURN (new_name);
}

/**
 * @brief Converts its argument into a string that can be safely used when
 *        printing C code. It does so by replacing all occurrences of '"'
 *        by '\"' , all occurrences of '\' by '\\', and all occurrences of
 *        '\n' by '\\n' .
 *
 * @param string a string.
 * @return a safe string
 */
char *
STRstring2SafeCEncoding (const char *string)
{
    char *result, *tmp;
    size_t i, len;

    DBUG_ENTER ();

    if (string == NULL) {
        result = NULL;
    } else {
        len = STRlen (string);

        result = MEMmalloc (len * 2 + 1);
        tmp = result;

        for (i = 0; i < len; i++) {
            switch (string[i]) {
            case '\"':
                tmp += sprintf (tmp, "\\\"");
                break;
            case '\\':
                tmp += sprintf (tmp, "\\\\");
                break;
            case '\n':
                tmp += sprintf (tmp, "\\n");
                break;
            default:
                *tmp = string[i];
                tmp++;
                break;
            }
        }

        *tmp = '\0';
    }

    DBUG_RETURN (result);
}

/**
 * @brief Format string to be C-style multiline comment.
 *
 * @param string to commentify
 * @return commented string
 */
char *
STRcommentify (const char *string)
{
    char *result = NULL;
    char *split = NULL;
    str_buf *buffer = NULL;

    DBUG_ENTER ();

    if (string != NULL) {
        buffer = SBUFcreate (STRlen (string) + 42);
        split = STRtok (string, "\n");

        while (split != NULL) {
            SBUFprintf (buffer, "%s\n * ", split);
            split = MEMfree (split);
            split = STRtok (NULL, "\n");
        }

        result = SBUF2str (buffer);
        buffer = SBUFfree (buffer);
    }

    DBUG_RETURN (result);
}

/**
 * @brief Converts the given string into the corresponding synatx tree
 *        from.
 *
 * @param str a string
 * @return non-flattened syntax tree.
 */
node *
STRstring2Array (const char *str)
{
    node *new_exprs;
    size_t i, cnt;
    node *array;
    node *len_expr;
    node *res;

    DBUG_ENTER ();

    new_exprs = TBmakeExprs (TBmakeChar ('\0'), NULL);

    cnt = 0;
    /*
     * decrement after check for > 0, safe method for reverse loop ending on 0
     * i : (STRlen - 1) to 0
     */
    for (i = STRlen (str); i-- > 0; ) {
        if ((i > 0) && (str[i - 1] == '\\')) {
            switch (str[i]) {
            case 'n':
                new_exprs = TBmakeExprs (TBmakeChar ('\n'), new_exprs);
                i -= 1;
                break;
            case 't':
                new_exprs = TBmakeExprs (TBmakeChar ('\t'), new_exprs);
                i -= 1;
                break;
            case 'v':
                new_exprs = TBmakeExprs (TBmakeChar ('\v'), new_exprs);
                i -= 1;
                break;
            case 'b':
                new_exprs = TBmakeExprs (TBmakeChar ('\b'), new_exprs);
                i -= 1;
                break;
            case 'r':
                new_exprs = TBmakeExprs (TBmakeChar ('\r'), new_exprs);
                i -= 1;
                break;
            case 'f':
                new_exprs = TBmakeExprs (TBmakeChar ('\f'), new_exprs);
                i -= 1;
                break;
            case 'a':
                new_exprs = TBmakeExprs (TBmakeChar ('\a'), new_exprs);
                i -= 1;
                break;
            case '"':
                new_exprs = TBmakeExprs (TBmakeChar ('"'), new_exprs);
                i -= 1;
                break;
            case '\\':
                new_exprs = TBmakeExprs (TBmakeChar ('\\'), new_exprs);
                i -= 1;
                break;
            default:
                new_exprs = TBmakeExprs (TBmakeChar (str[i]), new_exprs);
                break;
            }
        } else {
            new_exprs = TBmakeExprs (TBmakeChar (str[i]), new_exprs);
        }

        cnt += 1;
    }

    //This modifies the 'length' arg in String::to_string for Stdlib
    // FIXME grzegorz: update to_string to accept unsigned long arg
    // before changing to TBmakeNumulong
    len_expr = TBmakeNum (cnt);
    array
      = TCmakeVector (TYmakeAKS (TYmakeSimpleType (T_char), SHmakeShape (0)), new_exprs);

#ifndef CHAR_ARRAY_NOT_AS_STRING
    ARRAY_STRING (array) = STRcpy (str);
#endif /* CHAR_ARRAY_AS_STRING */

    res = TCmakeSpap2 (NSgetNamespace ("String"), STRcpy ("to_string"), array, len_expr);

    DBUG_RETURN (res);
}

/**
 * @brief Substitute all occurrences of token in str with subst
 *
 * @param str The string in which to make substitutions
 * @param token a string to substitute occurrences of in str
 * @param subst a string to substitute tokens with
 *
 * @return A new String
 */
char *
STRsubstToken (const char *str, const char *token, const char *subst)
{
    size_t occurrences, tlen, slen;
    const char *found;
    char *pos;
    char *result;

    DBUG_ENTER ();

    /* Find number of occurrences of token in str */
    occurrences = 0;
    tlen = STRlen (token);
    slen = STRlen (subst);
    found = strstr (str, token);

    while (found != NULL) {
        occurrences++;
        found = strstr (found + tlen, token);
    }

    /* Make substitutions */
    result = MEMmalloc (sizeof (char)
                        * (STRlen (str)
                           + (occurrences * (STRlen (subst) - tlen))
                           + 1));

    pos = result;
    while (*str != '\0') {
        if (STRprefix (token, str)) {
            strncpy (pos, subst, slen);
            pos += slen;
            str += tlen;
        } else {
            *(pos++) = *(str++);
        }
    }
    *pos = '\0';

    DBUG_RETURN (result);
}

/**
 * @brief Same as STRsubstTokend, but deallocate the first argument after substitution.
 *
 * @param str The string in which to make substitutions
 * @param token a string to substitute occurrences of in str
 * @param subst a string to substitute tokens with
 *
 * @return A new String
 */
char *
STRsubstTokend (char *str, const char *token, const char *subst)
{
    DBUG_ENTER ();

    char *nstr = STRsubstToken (str, token, subst);
    MEMfree (str);

    DBUG_RETURN (nstr);
}

/**
 * @brief Substitute multiple patterns in a string
 *
 * @param str The string in which to make substitutions
 * @param n  The number of substitutions
 *
 * @return A new String or NULL if str is NULL
 */
char *
STRsubstTokens (const char *str, size_t n, ...)
{
    char *result;
    va_list arg_list;
    const char **patterns;
    const char **values;
    size_t *sizes;
    size_t i, j;

    DBUG_ENTER ();

    void *memory = MEMmalloc(n *
                (sizeof(char *) + sizeof(char *) + sizeof(size_t)));
    /* (char *) patterns[n]; */
    patterns = (const char **)memory;
    /* (char *) values[n]; */
    values = (const char **)((uintptr_t)memory + n  * sizeof(char *));
    /* size_t sizes[n]; */
    sizes = (size_t *)((uintptr_t)memory + 2 * n * sizeof(char *));

    va_start (arg_list, n);

    for (i = 0; i < n; ++i) {
        patterns[i] = va_arg (arg_list, const char *);
        sizes[i] = STRlen (patterns[i]);
        values[i] = va_arg (arg_list, const char *);
    }

    va_end (arg_list);

    str_buf *buf = SBUFcreate (1);

    for (i = 0; str[i] != '\0'; ++i) {
        for (j = 0; j < n; ++j) {
            if (strncmp (patterns[j], str + i, sizes[j]) == 0) {
                SBUFprint (buf, values[j]);
                i += sizes[j] - 1;
                break;
            }
        }
        if (j == n)
            SBUFprintf (buf, "%c", str[i]);
    }

    MEMfree(memory);
    result = SBUF2str (buf);
    buf = SBUFfree (buf);

    DBUG_RETURN (result);
}

/**
 * @brief Strip all space characters (space, newline, carrage return, and tab)
 *        from both the front and back of the given string.
 *
 * @param s string to be stripped
 * @return stripped string, using the same pointer as input
 */
char *
STRstrip (char *s)
{
    char *cp, *ft;

    DBUG_ENTER ();

    if (s != NULL && *s != '\0')
    {
        ft = cp = s;

        while (isspace (*ft)) ft++;
        while (*ft != '\0') *cp++ = *ft++;
        *cp = '\0';
        while (cp > s && isspace (*--cp)) *cp = '\0';
    }

    DBUG_RETURN (s);
}

/**
 * Check if the string represents an int. It may only contain numeric characters (0-9),
 * optionally with a leading sign (-).
 *
 * @param str The string to be checked
 * @return true if the string represents a number, false otherwise
 */
bool
STRisInt (const char* str) {
    DBUG_ENTER ();

    bool isint = true;

    for (size_t i = 0; str[i] != '\0'; i++) {
        const char c = str[i];
        bool charIsint = (c >= '0' && c <= '9') || (i == 0 && (c == '-' || c == '+'));
        isint = isint && charIsint;
    }

    DBUG_RETURN (isint);
}

/**
 * Convert the string value to an integer value. Note that this function is designed to work on strings that follow the
 * conditions posed in STRisInt. For all other strings the behaviour of this function is undefined.
 *
 * @param str The string to be converted
 * @return The integer value this string represents
 */
int
STRatoi (const char* str){
    DBUG_ENTER ();

    int result = atoi(str);

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
