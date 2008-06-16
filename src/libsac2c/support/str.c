/*
 *
 * $Id$
 *
 */

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "str.h"

#include "memory.h"
#include "dbug.h"
#include "math_utils.h"
#include "str_buffer.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "shape.h"
#include "namespaces.h"

/*******************************************************************************
 *
 * Description: Copy string and allocate memory for new string.
 *
 * Parameters: - source, string to copy
 *
 * Return: - new copied string
 *
 *******************************************************************************/

char *
STRcpy (const char *source)
{
    char *ret;

    DBUG_ENTER ("STRcpy");

    if (source != NULL) {
        ret = (char *)MEMmalloc (sizeof (char) * (STRlen (source) + 1));
        strcpy (ret, source);
    } else {
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

/*******************************************************************************
 *
 * Description: Copy string and allocate memory for new string.
 *              Copy only maxlen characters.
 *
 * Parameters: - source, string to copy
 *             - maxlen, number of characters to copy
 *
 * Return: - new copied string
 *
 *******************************************************************************/

char *
STRncpy (const char *source, int maxlen)
{
    char *ret;
    int max;

    DBUG_ENTER ("STRncpy");

    if (source != NULL) {
        max = STRlen (source);
        if (max > maxlen) {
            max = maxlen;
        }

        ret = (char *)MEMmalloc (sizeof (char) * (max + 1));
        strncpy (ret, source, max);

        /* make sure string ends with 0 */
        ret[max] = '\0';
    } else {
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

/*******************************************************************************
 *
 * Description: Concatenate two strings and allocate memory for new string.
 *
 * Parameters: - first, first string
 *             - second, second string
 *
 * Return: - new concatenated string
 *
 *******************************************************************************/

char *
STRcat (const char *first, const char *second)
{
    char *result;

    DBUG_ENTER ("STRcat");

    if (first == NULL) {
        result = STRcpy (second);
    } else if (second == NULL) {
        result = STRcpy (first);
    } else {
        result = (char *)MEMmalloc (STRlen (first) + STRlen (second) + 1);

        strcpy (result, first);
        strcat (result, second);
    }

    DBUG_RETURN (result);
}

/*******************************************************************************
 *
 * Description: Concatenate N strings and allocate memory for new string.
 *
 * Parameters: - n, number of strings
 *             - ..., n amount of "const char *"-type strings
 *
 * Return: - new concatenated string
 *
 *******************************************************************************/

char *
STRcatn (int n, ...)
{
    int i;
    int length;
    char *result;
    const char *ptr;
    va_list arg_list;

    DBUG_ENTER ("STRcatn");

    DBUG_ASSERTF (n > 2, ("STRcatn called with %d arguments", n));

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
        result = (char *)MEMmalloc (length + 1);
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

/*******************************************************************************
 *
 * Description: Compare two strings.
 *
 * Parameters: - first, first string to compare
 *             - second, second string to compare
 *
 * Return: - TRUE, string contents are equal
 *         - FALSE, string contents are not equal
 *
 *******************************************************************************/

bool
STReq (const char *first, const char *second)
{
    bool res;

    DBUG_ENTER ("STReq");

    if ((first == NULL) && (second == NULL)) {
        res = TRUE;
    } else if ((first == NULL) || (second == NULL)) {
        res = FALSE;
    } else {
        res = (0 == strcmp (first, second));
    }

    DBUG_RETURN (res);
}

/*******************************************************************************
 *
 * Description: Compare two strings for referring to the same hexadecimal number
 *
 * Parameters: - first, first string to compare
 *             - second, second string to compare
 *
 * Return: - TRUE, string contents are equal
 *         - FALSE, string contents are not equal
 *
 *******************************************************************************/

bool
STReqhex (const char *first, const char *second)
{
    bool res;

    DBUG_ENTER ("STReqnum");

    if ((first == NULL) && (second == NULL)) {
        res = TRUE;
    } else if ((first == NULL) || (second == NULL)) {
        res = FALSE;
    } else {
        first = first + 2;
        while (*first == '0') {
            first++;
        }

        second = second + 2;
        while (*second == '0') {
            second++;
        }

        res = STReqci (first, second);
    }

    DBUG_RETURN (res);
}

/*******************************************************************************
 *
 * Description: Compare two strings for referring to the same octal number
 *
 * Parameters: - first, first string to compare
 *             - second, second string to compare
 *
 * Return: - TRUE, string contents are equal
 *         - FALSE, string contents are not equal
 *
 *******************************************************************************/

bool
STReqoct (const char *first, const char *second)
{
    bool res;

    DBUG_ENTER ("STReqnum");

    if ((first == NULL) && (second == NULL)) {
        res = TRUE;
    } else if ((first == NULL) || (second == NULL)) {
        res = FALSE;
    } else {
        while (*first == '0') {
            first++;
        }

        while (*second == '0') {
            second++;
        }

        res = STReq (first, second);
    }

    DBUG_RETURN (res);
}

/*******************************************************************************
 *
 * Description: Compare two strings in a case insensitive way.
 *
 * Parameters: - first, first string to compare
 *             - second, second string to compare
 *
 * Return: - TRUE, string contents are equal
 *         - FALSE, string contents are not equal
 *
 *******************************************************************************/

bool
STReqci (const char *first, const char *second)
{
    bool res;
    int i;

    DBUG_ENTER ("STReqci");

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

/*******************************************************************************
 *
 * Description: Compare two strings.
 *
 * Parameters: - first, first string to compare
 *             - second, second string to compare
 *             - n, number of relevant characters
 *
 * Return: - TRUE, relevant prefixes of strings are equal
 *         - FALSE, relevant prefixes of strings are not equal
 *
 *******************************************************************************/

bool
STReqn (const char *first, const char *second, int n)
{
    bool res;

    DBUG_ENTER ("STReq");

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

/*******************************************************************************
 *
 * Description: Checks if prefix is prefix of str
 *
 * Parameters: - prefix, first string to compare
 *             - str, second string to compare
 *
 * Return: - TRUE, prefix is prefix of str
 *         - FALSE, otherwise
 *
 *******************************************************************************/

bool
STRprefix (const char *prefix, const char *str)
{
    bool res;

    DBUG_ENTER ("STRprefix");

    if (prefix == NULL) {
        res = TRUE;
    } else {
        if (str == NULL) {
            res = FALSE;
        } else {
            res = (0 == strncmp (prefix, str, STRlen (prefix)));
        }
    }

    DBUG_RETURN (res);
}

/*******************************************************************************
 *
 * Description: Checks if prefix is prefix of str
 *
 * Parameters: - sub, first string to compare
 *             - str, second string to compare
 *
 * Return: - TRUE, sub is substring of str
 *         - FALSE, otherwise
 *
 *******************************************************************************/

bool
STRsub (const char *sub, const char *str)
{
    bool res;

    DBUG_ENTER ("STRsub");

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

/*******************************************************************************
 *
 * Description: Yield length of string
 *
 *  Mostly to provide a complete interface and to avoid using standard
 *  string facilities throughout the compiler.
 *
 *******************************************************************************/

int
STRlen (const char *s)
{
    int len;

    DBUG_ENTER ("STRlen");

    if (s == NULL) {
        len = 0;
    } else {
        len = strlen (s);
    }

    DBUG_RETURN (len);
}

/*******************************************************************************
 *
 * Description: Tokenize string. On first call the str will be copied to internal
 *              static variable, next calls str should be NULL. With last call the
 *              allocated memory of the copy will be freed.
 *
 *              In contrast to strtok, STRtok leaves the argument string untouched
 *              and always allocates the tokens in fresh memory.
 *
 * Parameters: - str, string to tokenize
 *             - tok, tokenizer
 *
 * Return: - pointer to the next token
 *         - NULL, no more tokens
 *
 *******************************************************************************/

static bool
CharInString (char c, const char *str)
{
    int i;
    bool res;

    DBUG_ENTER ("CharInString");

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

char *
STRtok (const char *first, const char *sep)
{
    static char *keep_string = NULL;
    static char *current = NULL;
    char *ret;
    int i;

    DBUG_ENTER ("STRtok");

    if (first != NULL) {
        if (keep_string != NULL) {
            keep_string = MEMfree (keep_string);
        }
        keep_string = STRcpy (first);
        current = keep_string;
    }

    if (current == NULL) {
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

/*******************************************************************************
 *
 * Description:
 *
 *  yields either the argument string if it is not NULL or an empty constant
 *  string otherwise.
 *
 * This is helpful for instance when printing strings with the %s conversion
 * specifier and the string to print may be NULL.
 *
 *******************************************************************************/

char *
STRonNull (char *alt, char *str)
{
    char *res;

    DBUG_ENTER ("STRonNull");

    if (str == NULL) {
        res = alt;
    } else {
        res = str;
    }

    DBUG_RETURN (res);
}

/*******************************************************************************
 *
 * Description: Convert integer to string in decimal representation.
 *
 * Parameters: - number to convert
 *
 * Return: - new allocated string representation of number
 *
 *******************************************************************************/

char *
STRitoa (int number)
{
    char *str;
    int tmp;
    int length;
    int base = 10;

    DBUG_ENTER ("STRitoa");

    tmp = number;
    length = 1;
    while (tmp >= base) {
        tmp /= base;
        length++;
    }

    str = (char *)MEMmalloc (sizeof (char) * length + 3);

    sprintf (str, "%d", number);

    DBUG_RETURN (str);
}

/*******************************************************************************
 *
 * Description: Convert integer to string in octal representation.
 *
 * Parameters: - number to convert
 *
 * Return: - new allocated string representation of number
 *
 *******************************************************************************/

char *
STRitoa_oct (int number)
{
    char *str;
    int tmp;
    int length;
    int base = 8;

    DBUG_ENTER ("STRitoa_oct");

    tmp = number;
    length = 1;
    while (tmp >= base) {
        tmp /= base;
        length++;
    }

    str = (char *)MEMmalloc (sizeof (char) * length + 3);

    sprintf (str, "0%o", number);

    DBUG_RETURN (str);
}

/*******************************************************************************
 *
 * Description: Convert integer to string in hexadecimal representation.
 *
 * Parameters: - number to convert
 *
 * Return: - new allocated string representation of number
 *
 *******************************************************************************/

char *
STRitoa_hex (int number)
{
    char *str;
    int tmp;
    int length;
    int base = 16;

    DBUG_ENTER ("STRitoa_hex");

    tmp = number;
    length = 1;
    while (tmp >= base) {
        tmp /= base;
        length++;
    }

    str = (char *)MEMmalloc (sizeof (char) * length + 3);

    sprintf (str, "0x%x", number);

    DBUG_RETURN (str);
}

/*******************************************************************************
 *
 * Description: Convert hex-string to byte array.
 *
 * Parameters: - array, converted byte array
 *               memory must be allocated before calling this function
 *             - string, string to convert
 *
 * Return: - converted byte array
 *
 *******************************************************************************/

static unsigned char
Hex2Dig (const char x)
{
    unsigned char res;

    DBUG_ENTER ("Hex2Dig");

    if ((x >= '0') && (x <= '9')) {
        res = x - '0';
    } else {
        res = 10 + x - 'A';
    }

    DBUG_RETURN (res);
}

unsigned char *
STRhex2Bytes (unsigned char *array, const char *string)
{
    int pos;
    unsigned char low, high;

    DBUG_ENTER ("STRhex2Bytes");

    pos = 0;

    while (string[pos * 2] != 0) {
        low = Hex2Dig (string[pos * 2 + 1]);
        high = Hex2Dig (string[pos * 2]);

        array[pos] = high * 16 + low;
        pos++;
    }

    DBUG_RETURN (array);
}

/*******************************************************************************
 *
 * Description: Convert byte array to hex-string.
 *
 * Parameters: - len, length of byte array
 *             - array, array to convert
 *
 * Return: - new allocated string representation of byte array
 *
 *******************************************************************************/

static char
Dig2Hex (unsigned char x)
{
    char res;

    DBUG_ENTER ("Dig2Hex");

    if (x < 10) {
        res = '0' + x;
    } else {
        res = 'A' + x - 10;
    }

    DBUG_RETURN (res);
}

char *
STRbytes2Hex (int len, unsigned char *array)
{
    int pos;
    char *result;
    unsigned char low, high;

    DBUG_ENTER ("STRbytes2Hex");

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

/******************************************************************************
 *
 * function:
 *   char *STRreplaceSpecialCharacters( const char *name)
 *
 * description:
 *   Replaces special characters such that they can be used as identifiers
 *   in a C program.
 *
 *****************************************************************************/

char *
STRreplaceSpecialCharacters (const char *name)
{
    char *new_name;
    char *tmp;
    int i, j;

    DBUG_ENTER ("STRreplaceSpecialCharacters");

    if (name == NULL) {
        new_name = NULL;
    } else {

        new_name = MEMmalloc ((3 * STRlen (name)) * sizeof (char));
        new_name[0] = '\0';

        for (i = 0, j = 0; (size_t)i < STRlen (name); i++, j++) {
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

/** <!-- ****************************************************************** -->
 *
 * @brief Converts its argument into a string that can be safely used when
 *        printing C code. It does so by replacing all occurences of '"'
 *        by '\"'.
 *
 * @param string a string.
 *
 * @return a safe string
 *
 ******************************************************************************/

char *
STRstring2SafeCEncoding (const char *string)
{
    char *result, *tmp;
    int i, len;

    DBUG_ENTER ("STRstring2SafeCEncoding");

    if (string == NULL) {
        result = NULL;
    } else {
        len = STRlen (string);

        result = MEMmalloc (len * 2 + 1);
        tmp = result;

        for (i = 0; i < len; i++) {
            if (string[i] == '"') {
                tmp += sprintf (tmp, "\\\"");
            } else {
                *tmp = string[i];
                tmp++;
            }
        }

        *tmp = '\0';
    }

    DBUG_RETURN (result);
}

char *
STRcommentify (const char *string)
{
    char *result = NULL;
    char *split = NULL;
    str_buf *buffer = NULL;

    DBUG_ENTER ("STRcommentify");

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

/** <!-- ****************************************************************** -->
 * @brief Converts the given string into the corresponding synatx tree
 *        from.
 *
 * @param str a string
 *
 * @return non-flattened syntax tree.
 ******************************************************************************/
node *
STRstring2Array (const char *str)
{
    node *new_exprs;
    int i, cnt;
    node *array;
    node *len_expr;
    node *res;

    DBUG_ENTER ("STRstring2Array");

    new_exprs = TBmakeExprs (TBmakeChar ('\0'), NULL);

    cnt = 0;

    for (i = STRlen (str) - 1; i >= 0; i--) {
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
            default:
                new_exprs = TBmakeExprs (TBmakeChar (str[i]), new_exprs);
                break;
            }
        } else {
            new_exprs = TBmakeExprs (TBmakeChar (str[i]), new_exprs);
        }

        cnt += 1;
    }

    len_expr = TBmakeNum (cnt);
    array
      = TCmakeVector (TYmakeAKS (TYmakeSimpleType (T_char), SHmakeShape (0)), new_exprs);

#ifndef CHAR_ARRAY_NOT_AS_STRING
    ARRAY_STRING (array) = STRcpy (str);
#endif /* CHAR_ARRAY_AS_STRING */

    res = TCmakeSpap2 (NSgetNamespace ("String"), STRcpy ("to_string"), array, len_expr);

    DBUG_RETURN (res);
}
