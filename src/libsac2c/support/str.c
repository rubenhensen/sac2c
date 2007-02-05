/*
 *
 * $Id$
 *
 */

#include <string.h>
#include <stdarg.h>

#include "str.h"

#include "memory.h"
#include "dbug.h"
#include "math_utils.h"

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
        ret = (char *)MEMmalloc (sizeof (char) * (strlen (source) + 1));
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
        max = strlen (source);
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
        result = (char *)MEMmalloc (strlen (first) + strlen (second) + 1);

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
            length += strlen (ptr);
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
 * Parameters: - str, string to tokenize
 *             - tok, tokenizer
 *
 * Return: - pointer to the next token
 *         - NULL, no more tokens
 *
 *******************************************************************************/

char *
STRtok (char *first, char *sep)
{
    static char *act_string = NULL;
    char *new_string = NULL;
    char *ret;

    DBUG_ENTER ("STRtok");

    if (first != NULL) {
        if (act_string != NULL) {
            act_string = MEMfree (act_string);
        }
        new_string = STRcpy (first);
        act_string = new_string;

        ret = strtok (new_string, sep);

        if (ret == NULL) {
            act_string = MEMfree (act_string);
        }
    } else {
        ret = NULL;
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
 * Description: Convert long to string.
 *
 * Parameters: - number, number to convert
 *
 * Return: - new allocated string representation of number
 *
 *******************************************************************************/

char *
STRitoa (long number)
{
    char *str;
    int tmp;
    int length, i;

    DBUG_ENTER ("STRitoa");

    tmp = number;
    length = 1;
    while (9 < tmp) {
        tmp /= 10;
        length++;
    }

    str = (char *)MEMmalloc (sizeof (char) * length + 1);
    str[length] = atoi ("\0");

    for (i = 0; i < length; i++) {
        str[i] = ((int)'0') + (number / MATHipow (10, (length - 1)));
        number = number % MATHipow (10, (length - 1));
    }

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

        new_name = MEMmalloc ((3 * strlen (name)) * sizeof (char));
        new_name[0] = '\0';

        for (i = 0, j = 0; (size_t)i < strlen (name); i++, j++) {
            switch (name[i]) {
            case '.':
                tmp = "_DO";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case ',':
                tmp = "_CM";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '+':
                tmp = "_PL";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '-':
                tmp = "_MI";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '*':
                tmp = "_ST";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '/':
                tmp = "_DI";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '%':
                tmp = "_PR";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '<':
                tmp = "_LT";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '>':
                tmp = "_GT";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '=':
                tmp = "_EQ";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '&':
                tmp = "_AM";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '|':
                tmp = "_VE";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '!':
                tmp = "_EX";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '[':
                tmp = "_BL";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case ']':
                tmp = "_BR";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '~':
                tmp = "_TI";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '@':
                tmp = "_AT";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '#':
                tmp = "_HA";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '$':
                tmp = "_DO";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '^':
                tmp = "_PO";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '\\':
                tmp = "_BS";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case ':':
                tmp = "_CL";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case ' ':
                tmp = "_SP";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '{':
                tmp = "_CO";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
                break;
            case '}':
                tmp = "_CC";
                strcat (&(new_name[j]), tmp);
                j += strlen (tmp) - 1;
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
        len = strlen (string);

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
