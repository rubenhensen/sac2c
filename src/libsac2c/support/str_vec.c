#include <memory.h>

#include "str_vec.h"

#define DBUG_PREFIX "STRVEC"

#include "debug.h"

#include "str.h"
#include "math_utils.h"

/**
* Module implementing string vector functions. All string operations result in deep copies.
* This is a little bit less efficient, but it is safe to use.
* One exception is the STRVECsel_shallow function, which has to be used with caution!
*/

struct STRVEC {
    // The currently in use length of the vector
    unsigned int length;
    // The currently allocated length of the vector
    unsigned int alloc;
    // The vector pointer
    char** vec;
};

/**
 * Make a new strvec. The array is already allocated with the defined length, but the strings are still undefined.
 *
 * @param length the length of the new vector
 * @return a new strvec
 */
strvec
MakeStrvec(unsigned int length) {
    DBUG_ENTER();

    strvec vec = (strvec) MEMmalloc(sizeof(strvec));
    vec->length = length;
    vec->alloc  = length;
    vec->vec    = (char**) MEMmalloc((unsigned long) length * sizeof(char*));

    DBUG_RETURN(vec);
}

/**
 * Ensure that the allocated space can contain a vector of length `minalloc`. If it cannot, enlarge the vector by
 * (at least) twice the size, so we prevent too many reallocations.
 *
 * @param vec the vector that should be size checked
 * @param minalloc the minimum required length in the fector
 */
inline void
ReallocStrvec(strvec vec, unsigned int minalloc) {
    DBUG_ENTER();

    if (vec->alloc < minalloc) return;

    vec->alloc = (unsigned int) MATHmax((int) minalloc, (int) vec->alloc * 2);
    vec->vec   = MEMrealloc(vec->vec, vec->alloc * sizeof(char*));

    DBUG_RETURN();
}

/**
 * Make a new vector from arguments.
 *
 * @param length the number of arguments
 * @param ... the strings to be put inside the vector
 * @return A vector containing the strings given as arguments
 */
strvec
STRVECmake(int length, ...) {
    DBUG_ENTER();

    va_list arglist;

    strvec vec = MakeStrvec((unsigned int) length);

    for (int i = 0; i < length; i++)
        vec->vec[i] = STRcpy(va_arg(arglist, char*));

    DBUG_RETURN(vec);
}

/**
 * Create a vector from an already existing array. The strings are deep copied into the vector.
 *
 * @param array The array with strings to be inserted
 * @param length The length of the array
 * @return A vector with the strings of the array
 */
strvec
STRVECfromArray(char** array, unsigned int length) {
    DBUG_ENTER();

    strvec vec = MakeStrvec(length);

    for (unsigned int i=0; i<length; i++)
        vec->vec[i] = STRcpy(array[i]);

    DBUG_RETURN(vec);
}

/*******************************************************************************
 *
 * Description: Make a new string vector filled by calls to the generator function
 *              If the generator is NULL, empty strings are used instead
 *
 *
 * Parameters: - length: the length of the new string vector
 *             - generator: a generator function that will generate the
 *                          strings inside the new vector (optional)
 *
 * Return: - the new and filled string vector
 *
 *******************************************************************************/

/**
 * Make a new string vector filled by calls to the generator function.
 * If the generator is NULL, empty strings are used instead
 *
 * @param length The length of the new array
 * @param generator The generator function used to fill the array
 * @return The filled up vector
 */
strvec
STRVECgen(unsigned int length, char* (* generator)(void)) {
    DBUG_ENTER();

    strvec vec = MakeStrvec(length);

    if (generator == NULL)
        generator = STRnull;

    for (unsigned int i = 0; i < length; i++)
        vec->vec[i] = generator();

    DBUG_RETURN(vec);
}

/**
 * Master gave vector socks! Vector is freeeeeee!
 *
 * @param vec The vector to be freed
 */
void
STRVECfree(strvec vec) {
    DBUG_ENTER();

    for (unsigned int i = 0; i < vec->length; i++)
        MEMfree(vec->vec[i]);
    MEMfree(vec->vec);
    MEMfree(vec);

    DBUG_RETURN();
}

/**
 * Resize the vector to the given length. If the vector would become larger, the generator function is used to fill
 * the new indexes. If the generator function is null, empty strings will be used.
 *
 * @param vec The vector to be resized
 * @param length The new length of the vector
 * @param generator The generator function to be used for new values, if necessary (optional)
 */
void
STRVECresize(strvec vec, unsigned int length, char* (* generator)(void)) {
    DBUG_ENTER();

    ReallocStrvec(vec, length);

    if (generator == NULL)
        generator = STRnull;

    for (; vec->length < length; vec->length++)
        vec->vec[vec->length] = generator();

    DBUG_RETURN();
}

/**
 * Copy the vector. All strings will be deep copied as well.
 *
 * @param source The vector to be copied
 * @return A copy of the vector.
 */
strvec
STRVECcopy(strvec source) {
    DBUG_ENTER();

    strvec vec = MakeStrvec(source->length);

    for (unsigned int i = 0; i < source->length; i++)
        vec->vec[i] = STRcpy(source->vec[i]);

    DBUG_RETURN(vec);
}

/**
 * Append a string to a vector. The string will be deep copied into the vector.
 *
 * @param vec The vector
 * @param str The string to be appended to the vector
 */
void
STRVECappend(strvec vec, char* str) {
    DBUG_ENTER();

    ReallocStrvec(vec, vec->length + 1);

    vec->vec[vec->length] = STRcpy(str);
    vec->length++;

    DBUG_RETURN();
}

/**
 * Concatenate the right vector onto the left vector. The strings from the right vector will be deep copied into the
 * left vector.
 *
 * @param left The vector that will be enlarged and filled with the elements of the right vector
 * @param right The vector with values that will be copied to the left vector
 */
void
STRVECconcat(strvec left, strvec right) {
    DBUG_ENTER();

    ReallocStrvec(left, left->length + right->length);

    for (unsigned int i = 0; i < right->length; i++) {
        left->vec[left->length + i] = right->vec[i];
    }

    left->length += right->length;

    DBUG_RETURN();
}

/**
 * Select a string value out of the vector. The string will be deep copied.
 *
 * @param vec The vector with the string value to be selected
 * @param index The index of the string value in the vector
 * @return A deep copy of the string value in vec on position index
 */
char*
STRVECsel(strvec vec, unsigned int index) {
    DBUG_ENTER();

    char* str = STRcpy(vec->vec[index]);

    DBUG_RETURN(str);
}

/**
 * Select a string value out of the vector. The string will _not_ be deep copied, but the char pointer will just be
 * returned. Use with caution.
 *
 * @param vec The vector with the string value to be selected
 * @param index The index of the string value in the vector
 * @return A char pointer to the string value in vec on position index
 */
char*
STRVECsel_shallow(strvec vec, unsigned int index) {
    DBUG_ENTER();

    char* str = vec->vec[index];

    DBUG_RETURN(str);
}

/**
 * Put a string value into the vector at a certain position. The string will be deep copied into the vector.
 *
 * @param vec The vector where a value has to be replaced
 * @param index The index in the vector
 * @param str The string to be inserted as a deep copy
 */
void
STRVECput(strvec vec, unsigned int index, char* str) {
    DBUG_ENTER();

    MEMfree(vec->vec[index]);
    vec->vec[index] = STRcpy(str);

    DBUG_RETURN();
}

#undef DBUG_PREFIX