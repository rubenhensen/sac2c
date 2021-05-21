/**
 * @file
 * @brief Alternative phase cycle driver, similar to actual phase cycle driver
 *
 * This set of macros are meant to be used to driver some sort of cyclical
 * operation, such as apply several traversals one after the other, to a
 * fixed point. This is very similar to what is happening in the compiler phase
 * driver (@see global/phase.c). The main difference here is that we can count
 * anything we want, as the user specifies what variable to assign to. There
 * is no requirement to only use the OPTCOUNTERS (@see stdopt/optimize.mac).
 *
 * This does make things more verbose though, compared to using the statistics
 * helper (@see stdopt/statistics.c) to manipulate the OPTCOUNTERS.
 *
 * The typical use case is where one currently applies several traversals
 * n times, and always n times:
 *
 * ~~~~
 * for (i = 1; i < 10; i++) {
 *   node = fun1 (node);
 *   if (doopt) {
 *     node = fun2 (node);
 *   }
 *   node = fun3 (node);
 *   node = fun4 (node);
 * }
 * ~~~~
 *
 * We can change this over to:
 *
 * ~~~~
 * TOC_SETUP (2, COUNT_ONE, COUNT_TWO)
 * bool test = false;
 *
 * TOC_SETCOUNTER (COUNT_TWO, 10)
 *
 * for (i = 1; i < global.max_optcycles; i++) {
 *   TOC_RUNOPT ("OPT1", true, COUNT_ONE, some_count_value, node, fun1)
 *   TOC_RUNOPT ("OPT2", doopt, TOC_IGNORE, 0, node, fun2)
 *   TOC_RUNOPT ("OPT3", true, COUNT_ONE, some_count_value2, node, fun3)
 *   TOC_RUNOPT ("OPT4", true, COUNT_TWO, some_count_value3, node, fun4)
 *
 *   TOC_COMPARE (test)
 *
 *   printf ("Counter: ONE -> %zu, TWO -> %zu\n",
 *           TOC_GETCOUNTER (COUNT_ONE),
 *           TOC_GETCOUNTER (COUNT_TWO));
 *
 *   if (test)
 *      break;
 * }
 * ~~~~
 *
 */
#ifndef _TREE_TRAVERSE_OPT_COUNTER_H_
#define _TREE_TRAVERSE_OPT_COUNTER_H_

#include "phase.h"

/**
 * @brief Setup and initialise all needed variables
 *
 * Here the user passes in _names_ of counters, which are kept stored in an
 * enum which is used to access an array, where all the counter values are stored.
 *
 * A special counter, called `TOC_IGNORE`, is already set. This can be used instead
 * of a real counter in cases where nothing is being stored.
 *
 * @param num Number of counter names being passed in
 * @param ... The counter names (its suggested that these should be in all-caps)
 */
#define TOC_SETUP(num, ...)                                                             \
    enum toc_optcounter_labels { TOC_IGNORE, __VA_ARGS__ };                             \
    const size_t toc_optcount_size = num+1;                                             \
    size_t toc_store[num+1] = {0};                                                      \
    size_t toc_store_old[num+1] = {0};

/**
 * @brief Compare current counter state with previous counter state over a
 *        specified _range_ of counters.
 *
 * We iteratively compare the counter values between the current state and previous
 * state. If all counters states are found to be **equal**, then the cycle has reached
 * a fixed-point. If however one or more counter states are **unequal**, we continue
 * the next iteration of the cycle.
 *
 * @param start Some label or integer indicating the start; **cannot** be less-than zero
 * @param end Some label or integer indicating the end; **cannot** be greater than number
 *            of total counters
 * @param out Variable used to store boolean result: if _true_, we've reached a
 *            fixed-point
 */
#define TOC_COMPARE_RANGE(start, end, out)                      \
    out = true;                                                 \
    for (size_t toc_i = start;                                  \
         (toc_i < end) && (toc_i < toc_optcount_size);          \
         toc_i++) {                                             \
        out = out && toc_store[toc_i] == toc_store_old[toc_i];  \
        toc_store_old[toc_i] = toc_store[toc_i];                \
    }

/**
 * @brief Compare current counter state with previous counter state for all counters
 *
 * @see TOC_COMPARE_RANGE
 *
 * @param out Variable used to store boolean result: if _true_, we've reached a
 *            fixed-point
 */
#define TOC_COMPARE(out) TOC_COMPARE_RANGE(1, toc_optcount_size, out)

/**
 * @brief Set counter to specified value
 *
 * @param label The counter name
 * @param val The value to set
 */
#define TOC_SETCOUNTER(label, val)                                                      \
    toc_store[label] = toc_store_old[label] = val;

/**
 * @brief Get the current counter value
 *
 * @param label The counter name
 * @return a value as type `size_t`
 */
#define TOC_GETCOUNTER(label) (toc_store[label])

/**
 * @brief Reset all counters to default value (zero)
 */
#define TOC_RESETCOUNTERS()                                                             \
    for (size_t toc_i = 0; toc_i < toc_optcount_size; toc_i++) {                        \
        toc_store[toc_i] = 0;                                                           \
        toc_store_old[toc_i] = 0;                                                       \
    }

#ifdef DBUG_OFF

/* in production compiler PHrunConsistencyChecks is disabled */
#define TOC_RUNCHECK(name, node)
#define TOC_RUNCHECK_TAG(tag, name, node)

#else /* DBUG_OFF */

#define TOC_RUNCHECK(name, node)                                                         \
    if (global.check_frequency >= 3) {                                                   \
        DBUG_PRINT ("Cycle iteration %d: running post-" name " check", i);               \
        node = PHrunConsistencyChecks (node);                                            \
    }

#define TOC_RUNCHECK_TAG(tag, name, node)                                                \
    if (global.check_frequency >= 3) {                                                   \
        DBUG_PRINT_TAG (tag, "Cycle iteration %d: running post-" name " check", i);      \
        node = PHrunConsistencyChecks (node);                                            \
    }

#endif /* DBUG_OFF */

/**
 * @brief Perform one call of the given optimisation/traversal function
 *
 * @param name Some string name to use for printouts
 * @param cond Some condition to control when the traversal should run
 * @param label Name of the counter to use
 * @param stmt Some value(s) to set the counter
 * @param node The node to pass to the function
 * @param fun The function to be called
 */
#define TOC_RUNOPT(name, cond, label, stmt, node, fun)                                   \
    if (cond) {                                                                          \
        DBUG_PRINT ("Cycle iteration %d: running " name, i);                             \
        toc_store_old[label] = stmt;                                                     \
        node = fun (node);                                                               \
        toc_store[label] = stmt;                                                         \
        TOC_RUNCHECK (name, node)                                                        \
    }

/**
 * @brief Perform one call of the given optimisation/traversal function using
 *        a specified TAG for printing
 *
 * @param tag Some string label for printouts
 * @param name Some string name to use for printouts
 * @param cond Some condition to control when the traversal should run
 * @param label Name of the counter to use
 * @param stmt Some value(s) to set the counter
 * @param node The node to pass to the function
 * @param fun The function to be called
 */
#define TOC_RUNOPT_TAG(tag, name, cond, label, stmt, node, fun)                          \
    if (cond) {                                                                          \
        DBUG_PRINT_TAG (tag, "Cycle iteration %d: running " name, i);                    \
        toc_store_old[label] = stmt;                                                     \
        node = fun (node);                                                               \
        toc_store[label] = stmt;                                                         \
        TOC_RUNCHECK_TAG (tag, name, node)                                               \
    }

#endif /* _TREE_TRAVERSE_OPT_COUNTER_H_ */
