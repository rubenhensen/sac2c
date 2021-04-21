/**
 * Module implementing string vector functions. All string vectors are shallow, which means that all input gets consumed
 * when new vectors are computed. Of course, there are some exceptions like STRVEClen. Both the vectors and the strings
 * in side them are shallow. Use extra caution when:
 *
 *   - Copying a vector. The newly created vector is unique at that point, but the strings are still shared.
 *     A helper function STRVECcopyDeep exists, to create copies of all the strings as well.
 *   - Freeing a vector. The function STRVECfree does free the vector, but not the strings inside it.
 *     A helper function STRVECfreeDeep exists, to free all data inside the vector.
 *   - Inserting elements into the vector, either at creation or in a later stage. The strings will be consumed by the
 *     vector.
 *
 */

#include <memory.h>

#include "str_vec.h"

#define DBUG_PREFIX "STRVEC"

#include "debug.h"

#include "str.h"
#include "math_utils.h"

struct STRVEC {
    // The currently in use length of the vector
    size_t length;
    // The currently allocated length of the vector
    size_t alloc;
    // The vector pointer
    char** data;
};

#define STRVEC_LENGTH(vec) vec->length
#define STRVEC_ALLOC(vec) vec->alloc
#define STRVEC_DATA(vec) vec->data

/**
 * Internal helper function
 *
 * Make a new strvec*. The array is already allocated with the defined length, but the strings are still undefined.
 *
 * @param length the length of the new vector
 * @return a new strvec*
 */
strvec*
MakeStrvec(size_t alloc, size_t length) {
    DBUG_ENTER();

    strvec* vec = (strvec*) MEMmalloc(sizeof(strvec*));
    STRVEC_LENGTH(vec) = length;
    STRVEC_ALLOC(vec)  = alloc;
    STRVEC_DATA(vec)   = (char**) MEMmalloc((unsigned long) alloc * sizeof(char*));

    DBUG_RETURN(vec);
}

/**
 * Internal helper function
 *
 * Ensure that the allocated space can contain a vector of length `minalloc`. If it cannot, enlarge the vector by
 * (at least) twice the size, so we prevent too many reallocations.
 *
 * @param vec the vector that should be size checked
 * @param minalloc the minimum required length in the fector
 */
void
ReallocStrvec(strvec* vec, size_t minalloc) {
    DBUG_ENTER();

    if (STRVEC_ALLOC(vec) < minalloc) {
        STRVEC_ALLOC(vec) = (size_t) MATHmax((int) minalloc, (int) STRVEC_ALLOC(vec) * 2);
        STRVEC_DATA(vec)  = MEMrealloc(STRVEC_DATA(vec), STRVEC_ALLOC(vec) * sizeof(char*));
    }

    DBUG_RETURN();
}

/**
 * Create an empty vector with a given preallocated length.
 *
 * @param preallocate the number of preallocated elements. They are not filled yet, so the length of the vector is
 * still 0
 *
 * @return The newly created empty vector
 */
strvec*
STRVECempty(size_t preallocate) {
    DBUG_ENTER();

    strvec* vec = MakeStrvec(preallocate, 0);

    DBUG_RETURN(vec);
}

/**
 * Make a new vector from arguments. The strings are consumed, and shallow-copied into the vector.
 *
 * @param length the number of arguments
 * @param ... the strings to be put inside the vector
 * @return A vector containing the strings given as arguments
 */
strvec*
STRVECmake(size_t length, ...) {
    DBUG_ENTER();

    va_list arglist;
    va_start(arglist, length);

    strvec* vec = MakeStrvec(length, length);

    for (size_t i = 0; i < length; i++)
        STRVEC_DATA(vec)[i] = va_arg(arglist,
    char*);

    va_end(arglist);

    DBUG_RETURN(vec);
}

/**
 * Create a vector from an already existing array. The strings are consumed, and shallow copied into the vector.
 * The array is _not_ consumed.
 *
 * @param array The array with strings to be inserted
 * @param length The length of the array
 * @return A vector with the strings of the array
 */
strvec*
STRVECfromArray(char** array, size_t length) {
    DBUG_ENTER();

    strvec* vec = MakeStrvec(length, length);

    for (size_t i = 0; i < length; i++)
        STRVEC_DATA(vec)[i] = array[i];

    DBUG_RETURN(vec);
}

/**
 * Make a new string vector filled by calls to the generator function.
 * If the generator is NULL, empty strings are used instead
 *
 * @param length The length of the new array
 * @param generator The generator function used to fill the array
 * @return The filled up vector
 */
strvec*
STRVECgen(size_t length, char* (* generator)(void)) {
    DBUG_ENTER();

    strvec* vec = MakeStrvec(length, length);

    if (generator == NULL)
        generator = STRnull;

    for (size_t i = 0; i < length; i++)
        STRVEC_DATA(vec)[i] = generator();

    DBUG_RETURN(vec);
}

/**
 * Master gave vector socks! Vector is freeeeeee!
 *
 * Shallow version of the free function. This means that the vector will be freed, but the strings inside vector are not.
 * See STRVECfreeDeep for the deep-freed version.
 *
 * @param vec The vector to be freed
 * @return a null pointer
 */
strvec*
STRVECfree(strvec* vec) {
    DBUG_ENTER();

    MEMfree(STRVEC_DATA(vec));
    MEMfree(vec);

    DBUG_RETURN(NULL);
}

/**
 * Master gave vector socks! Vector is freeeeeee!
 *
 * Deep version of the free function. This means that the vector will be freed, and the strings inside it will be freed
 * as well. See STRVECfree for the shallow-freed version.
 *
 * @param vec The vector to be freed
 * @return a null pointer
 */
strvec*
STRVECfreeDeep(strvec* vec) {
    DBUG_ENTER();

    for (size_t i = 0; i < STRVEC_LENGTH(vec); i++)
        MEMfree(STRVEC_DATA(vec)[i]);
    MEMfree(STRVEC_DATA(vec));
    MEMfree(vec);

    DBUG_RETURN(NULL);
}

/**
 * Resize the vector to the given length. If the vector would become larger, the generator function is used to fill
 * the new indexes. If the generator function is null, empty strings will be used.
 *
 * Note: if the vector would become smaller, the strings inside it would _not_ be freed. Use with caution.
 * See STRVECresizeFree for a version that does free the strings on shrinking.
 *
 * @param vec The vector to be resized
 * @param length The new length of the vector
 * @param generator The generator function to be used for new values, if necessary (optional)
 */
void
STRVECresize(strvec* vec, size_t length, char* (* generator)(void)) {
    DBUG_ENTER();

    ReallocStrvec(vec, length);

    if (generator == NULL)
        generator = STRnull;

    // Generate new elements for vector growth
    for (size_t i = STRVEC_LENGTH(vec); i < length; i++)
        STRVEC_DATA(vec)[i] = generator();

    STRVEC_LENGTH(vec) = length;

    DBUG_RETURN();
}

/**
 * Resize the vector to the given length. If the vector would become larger, the generator function is used to fill
 * the new indexes. If the generator function is null, empty strings will be used.
 *
 * Note: if the vector would become smaller, the strings inside it would be freed. Use with caution.
 * See STRVECresize for a version that does _not_ free the strings on shrinking.
 *
 * @param vec The vector to be resized
 * @param length The new length of the vector
 * @param generator The generator function to be used for new values, if necessary (optional)
 */
void
STRVECresizeFree(strvec* vec, size_t length, char* (* generator)(void)) {
    DBUG_ENTER();

    ReallocStrvec(vec, length);

    if (generator == NULL)
        generator = STRnull;

    // Generate new elements for vector growth
    for (size_t i = STRVEC_LENGTH(vec); i < length; i++)
        STRVEC_DATA(vec)[i] = generator();
    // Free obsolete elements for vector shrinking
    for (size_t i           = length; i < STRVEC_LENGTH(vec); i++)
        MEMfree(STRVEC_DATA(vec)[i]);

    STRVEC_LENGTH(vec) = length;

    DBUG_RETURN();
}

/**
 * Get the length of a string vector
 *
 * @param vec The vector
 * @return The length
 */
size_t
STRVEClen(strvec* vec) {
    DBUG_ENTER();

    size_t length = STRVEC_LENGTH(vec);

    DBUG_RETURN(length);
}

/**
 * Print the string vector to a stream. The argument linesize can be used to set the max line size, to insert newlines
 * when the linesize is reached. Note that always at least one string is printed on each line, so when strings are
 * longer then the line size, the line size will be exceeded. Use linesize=0 for one string per line.
 *
 * @param vec The vector to be printed
 * @param stream The stream to be printed to
 * @param linesize The maximum line size
 */
void
STRVECprint(strvec* vec, FILE* stream, size_t linesize) {
    DBUG_ENTER();

    fprintf(stream, "String vector (length: %zu, allocated: %zu) [", STRVEC_LENGTH(vec), STRVEC_ALLOC(vec));

    // Set the currentlinesize to linesize to force printing of a newline just after the opening line.
    // If we print the newline in the fprintf above, we would get a double newline when the first string is longer
    // then linesize.
    size_t      currentlinesize = linesize;
    for (size_t i               = 0; i < STRVEC_LENGTH(vec); i++) {
        char* str = STRVEC_DATA(vec)[i];

        // Compute length to check the linesize. Add 2 for the ", ".
        size_t length = STRlen(str) + 2;

        if (currentlinesize + length > linesize) {
            fprintf(stream, "\n  ");
            currentlinesize = 2; // Two indentation spaces
        }

        currentlinesize += 2;
        fprintf(stream, "%s, ", str);

    }

    fprintf(stream, "\n]\n");

    DBUG_RETURN();
}

/**
 * Copy the vector. All strings will be shallow copied.
 * See STRVECcopyDeep for a deep-copying version of the strings.
 *
 * @param source The vector to be copied
 * @return A copy of the vector.
 */
strvec*
STRVECcopy(strvec* source) {
    DBUG_ENTER();

    size_t length = STRVEC_LENGTH(source);
    strvec* vec = MakeStrvec(length, length);

    for (size_t i = 0; i < STRVEC_LENGTH(source); i++)
        STRVEC_DATA(vec)[i] = STRVEC_DATA(source)[i];

    DBUG_RETURN(vec);
}

/**
 * Copy the vector. All strings will be deep copied.
 * See STRVECcopy for a shallow-copying version of the strings.
 *
 * @param source The vector to be copied
 * @return A copy of the vector.
 */
strvec*
STRVECcopyDeep(strvec* source) {
    DBUG_ENTER();

    size_t length = STRVEC_LENGTH(source);
    strvec* vec = MakeStrvec(length, length);

    for (size_t i = 0; i < STRVEC_LENGTH(source); i++)
        STRVEC_DATA(vec)[i] = STRcpy(STRVEC_DATA(source)[i]);

    DBUG_RETURN(vec);
}

/**
 * Append a string to a vector. The entire vector including strings, and the given append string will be consumed,
 * and a new appended vector will be returned.
 *
 * @param vec The vector
 * @param str The string to be appended to the vector
 * @return the appended vector.
 */
strvec*
STRVECappend(strvec* vec, char* str) {
    DBUG_ENTER();

    ReallocStrvec(vec, STRVEC_LENGTH(vec) + 1);

    STRVEC_DATA(vec)[STRVEC_LENGTH(vec)] = str;
    STRVEC_LENGTH(vec)++;

    DBUG_RETURN(vec);
}

/**
 * Concatenate the right vector onto the left vector. Consumes the left vector including strings, as well as the strings
 * of the right vector. Returns the newly concatenated vector.
 *
 * @param left The left side vector of the concatenation
 * @param right The right side vector of the concatenation
 * @return The concatenated vector
 */
strvec*
STRVECconcat(strvec* left, strvec* right) {
    DBUG_ENTER();

    ReallocStrvec(left, STRVEC_LENGTH(left) + STRVEC_LENGTH(right));

    for (size_t i = 0; i < STRVEC_LENGTH(right); i++) {
        STRVEC_DATA(left)[STRVEC_LENGTH(left) + i] = STRVEC_DATA(right)[i];
    }

    STRVEC_LENGTH(left) += STRVEC_LENGTH(right);

    DBUG_RETURN(left);
}

/**
 * Select a string value out of the vector. The string will be shallow copied.
 *
 * @param vec The vector with the string value to be selected
 * @param index The index of the string value in the vector
 * @return A shallow copy of the string value in vec on position index
 */
char*
STRVECsel(strvec* vec, size_t index) {
    DBUG_ENTER();

    char* str = STRVEC_DATA(vec)[index];

    DBUG_RETURN(str);
}

/**
 * Put a string value into the vector at a certain position. The string will be consumed, and the old string will be
 * returned.
 *
 * @param vec The vector where a value has to be replaced
 * @param index The index in the vector
 * @param str The string to be inserted as a shallow copy
 * @return the old string
 */
char*
STRVECswap(strvec* vec, size_t index, char* str) {
    DBUG_ENTER();

    char* old = STRVEC_DATA(vec)[index];
    STRVEC_DATA(vec)[index] = str;

    DBUG_RETURN(old);
}

char*
STRVECpop(strvec* vec) {
    DBUG_ENTER();

    char* str = STRVEC_DATA(vec)[STRVEC_LENGTH(vec) - 1];
    STRVEC_LENGTH(vec)--;

    DBUG_RETURN(str);
}

#undef DBUG_PREFIX
