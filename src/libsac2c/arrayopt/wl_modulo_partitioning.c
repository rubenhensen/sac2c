/*******************************************************************************
 *
 * This traversal replaces specific forms of the _aplmod_SxS_ primitive function
 * inside of with-loops with a with-loop invariant computation and a cheaper
 * primitive function inside the with-loop.
 *
 *
 * A refresher on remainder and modulo in the context of SaC:
 *   The remainder, % or _mod_SxS_, is the value that remains after dividing a
 *   number (dividend) by another (divisor), where division rounds towards 0.
 *   Some examples:
 *     5 %  3 =  2    -5 %  3 = -2
 *     5 % -3 =  2    -5 % -3 = -2
 *     5 %  0 = error
 *
 *   Aplmod, _aplmod_SxS_(dividend, divisor), or mod() in the standard library,
 *   is similar to this remainder, but forces the values to lie between 0 and
 *   the divisor by adding the divisor to the remainder if the result is not yet
 *   in the desired range.
 *   Additionally, it does not crash when the divisor is 0. Instead, the result
 *   is defined to be equal to the dividend.
 *   This definition is in line with the APL programming language, which is why
 *   it is named aplmod.
 *   Some examples:
 *     mod( 5,  3) =  2    mod(-5,  3) =  1
 *     mod(-5, -3) = -2    mod( 5, -3) = -1
 *     mod(5, 0) = 5
 *   
 *   This optimization exclusively deals with this aplmod, which THIS file
 *   refers to as aplmod, mod, and modulo interchangeably.
 *   Remainder is completely ignored by this optimization.
 *   This is because this optimization is mainly made to make in indexing with
 *   modulo operations fast, and that doesn't work with the remainder because a
 *   negative dividend would result in out-of-bounds indexing.
 *   
 *
 * Lexicon:
 *   Name           | Abbreviation | Meaning
 *   ---------------------------------------------------------------------------
 *   Divisor        |      x       | The second argument of an aplmod primitive
 *   Dividend:      |              | The first argument of an aplmod primitive
 *   Index (vector) |     i/iv     | An index of a wl-generator*
 *   Offset:        |      c       | A constant offset added to the index vector
 *   Lower bound    |      l       | The lower bound of a wl-generator*
 *   Upper bound    |      u       | The upper bound of a wl-generator*
 *
 *   *These often refer to a specific dimension of the with-loop generator.
 *
 *
 * Example transformation:
 *   Consider the following piece of code where the with-loop is equivalent
 *   to rotate([2], grid):
 *
 *     grid = iota(10);
 *     with {
 *         ([0] <= iv < [10]): grid[mod(iv - 2), 10)];
 *     }: genarray ([10], 0);
 *
 *   After applying this transformation and constant folding, we get:
 *
 *     grid = iota(10);
 *     with {
 *         ([0] <= iv < [ 2]): grid[iv + 8];
 *         ([2] <= iv < [10]): grid[iv - 2];
 *     }: genarray([10], 0);
 *
 *   Effectively, the partition is cloned into two versions that each replace
 *   the aplmod with an addition with a different offset. This new offset is
 *   mathematically equivalent to the original result of the aplmod for that
 *   partition.
 *   We call this transformation modulo partitioning.
 *
 *
 * Motivation:
 *   Modulo operations are as expensive as divisions, making it one of the more
 *   expensive operations that exist on a CPU.
 *   This optimization seeks to eliminate modulo operations.
 *   The core idea stems from the realization that _aplmod_SxS_ calls often
 *   amount to only two different values across the iteration space.
 *   Hence, if the iteration space is divided up into two parts (partitions),
 *   the _aplmod_SxS_ call can be eliminated and replaced with a reference to
 *   the value computed for that specific partition outside the with-loop.
 *
 *   Modulo partitioning allows some code to be written elegantly using modulo
 *   while allowing it to be compiled into performant code by completely
 *   eliminating the modulo calls.
 *   Notably, the rotate function from the standard library can be written as
 *   an elegant oneliner:
 *
 *   int[dim:shp] rotate (int[dim] offset, int[dim:shp] grid)
 *   {
 *     return { iv -> grid[mod(iv - offset, shp)] | iv < shp }
 *   }
 *
 *
 * Space complexity:
 *   This optimization results in a worst-case space complexity of O(p^a),
 *   where 'p' denotes the number of partitions created per _aplmod_SxS_ call,
 *   and 'a' denotes the number of applications of _aplmod_SxS_.
 *   Typically, the number of applications relates to the dimensionality of the
 *   array. 
 *   For example, mod() leads to one _aplmod_SxS_ call per dimension per
 *   iteration. This optimization then applies to these aplmod calls one by one,
 *   which leads to a complexity of O(p^d), where 'p' is the same as before, 
 *   and 'd' is the dimensionality of the array that mod() was applied to.
 *
 *   Make no mistake about it, this IS terrible space complexity.
 *   But that's simply the price paid for fast code. For example, the previous
 *   definition of rotate would compile into at least as many partitions as
 *   with the new definition.
 *   
 *   The space complexity can make programs far too large to compile, so it has
 *   to be limited in some way.
 *   This is where the commandline option -maxwlmp <n> comes in. It limits how
 *   many partitions will be in the resulting code. Code transformations in this
 *   optimization will only apply if they result in less than or equal to
 *   maxwlmp partitions.
 *   The default value is 2, to allow rotates to be caught by the optimization.
 *   When the dimensionality, and by extension, the space complexity is too
 *   high, -maxwlmp 1 can be used to eliminate the explosion of partitions.
 *   The space complexity then suddenly reduces to O(1), at the cost of not
 *   optimizing all cases, leading to less performant code at runtime.
 *   With -maxwlmp set to 1, this optimization will still catch modulo calls
 *   that require no new partitions to eliminate.
 *
 *   Going back to the motivation for this optimization, tweaking the -maxwlmp
 *   parameter gives the SaC developer the option to switch between a fast 
 *   runtime version and a fast compiletime version.
 *
 *
 * Transformations:
 *    This optimization implements two different techniques (transformations) to
 *    apply modulo partitioning: Shared Offset Partitioning (SOP),
 *    and Shared Result Partitioning (SRP).
 *    SOP leads to the smallest number of partitions when the divisor is zero,
 *    or when the divisor is greater than or equal to the upper bound of the
 *    partition.
 *    SRP leads to the smallest number of partitions when the divisor is closest
 *    but not equal to zero.
 *    For any given _aplmod_SxS_ call, whichever method produces the fewest
 *    partitions is selected.
 *
 *    While the rotate example is the main use we identified, the optimization
 *    can apply to more situations than just that.
 *
 *    Both transformations apply to only one _aplmod_SxS_ call at a time, so
 *    the optimization repeatedly transforms the partitions using the different
 *    techniques until they can no longer be applied.
 *    For brevity's sakes, the patterns in the next section only show
 *    one-dimensional examples, but any dimensionality can be dealt with
 *    implicitly as long as the modulo calls of the different dimensions are
 *    reduced to individual _aplmod_SxS_ calls. The repeated application of SOP
 *    and SRP will take care of them.
 *    The dimension that SOP or SRP applies to for a given _aplmod_SxS_ call is
 *    determined by the scalar index used in the aplmod.
 *
 *   Shared Offset Partitioning (SOP):
 *     This transformation is the one that applies to the rotate example.
 *     Aplmod computations inside a partition are split up into multiple
 *     partitions that each replace the aplmod with addition of the iterator
 *     and an offset tailored to that partition.
 *     This transformation can be applied when the offset and divisor are
 *     with-loop invariant, and enough information is known at compile-time
 *     to determine how many partitions are required.
 *
 *     The pattern:
 *       c = ...;
 *       x = ...;
 *       with {
 *           ...
 *           ([l] <= iv < [u]) {... _aplmod_SxS_ (iv + c, x) ...}: ...;
 *           ...
 *       }: ...;
 *
 *     The pattern is analyzed by DetermineNrSharedOffsetPartitions() to
 *     determine whether any of the following cases, denoted as C(condition)
 *     apply:
 *     C(x = 0): The divisor is zero.
 *     C(everything): All values u, c & x are known at compile-time.
 *     C(u = x): The upper bound is the same as the divisor.
 *     C(u = -x): The upper bound is the same as negative divisor.
 *     C(u, x): The upper bound and divisor are known at compile-time.
 *
 *     In all of these cases, we can at least determine an upper bound on the
 *     required number of partitions to split the modulo computation.
 *     With additional information, such as the lower bound and the offset, the
 *     estimate can be refined.
 *     Using this estimate of the number of partitions, the transformation is
 *     applied to the pattern listed above as follows:
 *
 *     The transformation for 1 partition:
 *       c = ...;
 *       x = ...;
 *       offset = <Refer to GetSOPnewOffset()>;
 *       with {
 *           ...
 *           ([l] <= iv < [u]) {... _add_SxS_ (iv, offset) ...}: ...;
 *           ...
 *       }: ...;
 *
 *     The transformation for n | n > 1 partitions:
 *       c = ...;
 *       x = ...;
 *       partition_lower_bound_N = <Refer to GetSOPlowerBound()>;
 *       offset_N = <Refer to GetSOPnewOffset()>;
 *       with {
 *           ...
 *           ([l] <= iv < [partition_lower_bound_1]) {... _add_SxS_ (iv, offset_0) ...}: ...;
 *           ***
 *           ([partition_lower_bound_n] <= iv < [u]) {... _add_SxS_ (iv, offset_n) ...}: ...;
 *           ...
 *       }: ...;
 *
 *       Where at *** we insert a partition for every partition m | 0 < m < n that looks like:
 *           ([partition_lower_bound_m] <= iv < [partition_lower_bound_m_plus_1]) {... _add_SxS_ (iv, offset_m)}: ...;
 *
 *     At run-time, it may turn out that the number of partitions was an
 *     overestimation, leading to partitions with an empty iteration space,
 *     which is no cause for concern.
 *
 *   Shared Result Partitioning (SRP):
 *     Shared Result partitioning creates a partition for each result value
 *     of the aplmod computation.
 *     This means that one partition is required for each value in the range of
 *     0 and the divisor. The step of the partitions are set to equal the
 *     divisor.
 *
 *     This transformation can only be applied when the divisor of a modulo
 *     computation is known at compile-time, and is generally only worth
 *     applying to small divisors.
 *     This optimization mainly allows even and odd numbers to be addressed
 *     easily.
 *
 *     The pattern:
 *       c = ...;
 *       with {
 *           ([l] <= i < [u]): ... _aplmod_SxS_ (i + c, x) ...;
 *       }: ...
 *
 *     The transformation for n | n > 0 partitions:
 *       c = ...;
 *       abs_divisor = abs(x);
 *       mod_res_n = _aplmod_SxS_ (l + n + c, 2);
 *       with {
 *           ***
 *       }: ...
 * 
 *     Where at *** we insert a partition for every partition n that looks like:
 *       ([l + n] <= i < [u] step [abs_divisor]): ... mod_res_n ...;
 *
 *     For this optimization to be beneficial rather than detrimental, it is
 *     necessary to either target cuda, or for the with-loop transform 
 *     (wl_transform.c) optimization to eventually transform the with-loop into
 *     a fused for-loop of the following form, where the divisor is set to 2 to
 *     keep the example somewhat concise:
 *       c = ...;
 *       mod_res1 = _aplmod_SxS_ (l     + c, 2);
 *       mod_res2 = _aplmod_SxS_ (l + 1 + c, 2);
 *       divisible_size = (u - l) - (_aplmod_SxS_(u - l, 2));
 *       int i = l;
 *       for (i; i < divisible_size; i++) {
 *           ... mod_res1 ...;
 *           i++;
 *           ... mod_res2 ...;
 *       }
 *       for (i; i < u; i++) { ... mod_res1 ...} // effectively an if-statement.
 *
 *     wl_transform.c can CURRENTLY only do this if the lower and upper bound
 *     are known at compile-time.
 *     When either of the bounds are unknown at compile time, the transformation
 *     can technically still be made due to the divisible_size construct,
 *     but wl_transform does not currently implement this when either of the
 *     bounds are unknown.
 *     Applying SRP without the with-loops eventually being fused inside a
 *     single for-loop can reduce the performance by about 70%, so when either
 *     of the bounds are unknown and cuda isn't targeted, SRP is inhibited.
 *     To enable SRP in this situation regardless, the command-line option
 *     -dowlmp_aggressive can be used.
 *
 *
 * Traversal implementation:
 *   The WLMP traversal accommodates both SOP and SRP.
 *   The WLMP traversal, like most traversals, starts in Fundef.
 *   From there, it'll look for with-loops in a top-down manner, guaranteeing
 *   it reaches the inner-most with-loop before doing anything.
 *   At each partition, information about that partition is stored in the
 *   INFO struct.
 *   When no more with-loops are found, the traversal traverses the N_assign
 *   nodes in a bottom-up manner.
 *   Every time an _aplmod_SxS_ invocation is found while traversing the
 *   N_assign nodes in a bottom-up manner, MatchValidPatterns() tries to match
 *   it against known acceptable patterns for SOP and SRP.
 *   If the aplmod matches, DeterminePartitionMethod() determines how many
 *   partitions SOP and SRP require to transform the partition based on the
 *   matched aplmod.
 *   The method requiring the least partitions gets selected.
 *   If only one partition is required, the aplmod is replaced and the
 *   bottom-up traversal continues.
 *   If multiple partitions are required, the aplmod is marked so it can be
 *   found later. The traversal folds back up to the most recent partition.
 *   At this level, the partition is copied the required number of times.
 *   This is followed by traversing into these partitions using the anonymous
 *   traversal ATrav.
 *   The goal of ATrav is to make all the necessary modifications discovered
 *   in the main traversal.
 *   ATrav changes the bounds of the partition to match the requirements set
 *   by SOP or SRP, which are stored in the INFO struct. 
 *   The functions SetPartitionBounds() and SetSRPstep() facilitate this.
 *   Respectively, they change the bounds and step size.
 *   After changing the bounds, the anonymous traversal looks for the aplmod
 *   that was marked earlier. Once found, it will replace the aplmod expression
 *   to match the requirements set by SOP or SRP for the given partition.
 *   This is facilitated by the ReplaceAplmod() function.
 *   After the anonymous traversal replaces the aplmod, the traversal folds back
 *   up to the level of the partition. There, it will continue for the other
 *   copied partitions, before finally relinquishing control to the main
 *   traversal.
 *   The main traversal gets back control at the level of the partition.
 *   Here, it repeats the same discovery + modification process until no more
 *   aplmod calls are found that SOP or SRP can deal with.
 *   When no more aplmod calls are found, the process repeats for the next
 *   partition.
 *   When this is done for all partitions in this innermost with-loop, the
 *   process repeats for the parent with-loop until finally there are no more
 *   optimizations to make in the entire function.
 *   
 *   This optimization runs immediately after with-loop invariant removal
 *   is applied so with-loop invariance can easily be checked by whether a
 *   variable is not in the current partition.
 *   For each partition, INFO_LOCALS_LUT stores the local variables that were
 *   encountered in that partition during the top-down traversal. If a variable
 *   is in this LUT, it is local (and not with-loop invariant) and vice-versa. 
 *   The LUT effectively functions as a set.
 *
 *   The main traversal, prefixed with WLMP, is responsible for finding the
 *   information required to replace the aplmod computation.
 *   Here is a rundown of the responsibilities of the WLMP functions:
 *
 *   WLMPfundef: Entry point for normal functions and lac-functions.
 *   WLMPassign: Adds ids to the INFO_LOCALS_LUT.
 *               Manages the preassigns ONLY if the assign holds a with-loop.
 *               Traverses top-down if the assign holds a with-loop.
 *               Traverses bottom-up if the assign doesn't hold a with-loop.
 *               Mixing top-down/bottom-up traversal strategy ensures that the
 *               innermost with-loops are traversed first. This in turn ensures
 *               that nested with-loops are fully optimized before outer
 *               with-loops get optimized, avoiding duplicate work.
 *               Stops traversing if a valid aplmod candidate is found.
 *   WLMPwith:   Stacks the LOCALS_LUT and WITH_IDS: The old variables are
 *               stored locally and replaced with fresh variables for
 *               the new with-loop. They are restored after traversing.
 *               Traverses only into N_Part.
 *   WLMPpart:   Calls the recursive function EvaluatePartition(), which
 *               is the meat of the traversal.
 *               It repeatedly dives into the corresponding PART_CODE to
 *               discover valid aplmod candidates and apply SOP or SRP until
 *               it can no longer be applied.
 *               Once the partition cannot be optimized any further, it
 *               traverses to the next partition as one would expect.
 *   WLMPprf:    Matches the prf node against specific aplmod patterns as
 *               determined by the MatchValidPatterns() function.
 *               Infers INFO_PARTITION_METHOD and INFO_NR_REQUIRED_PARTITIONS
 *               using the DeterminePartitionMethod() function.
 *               Prepares and stores information required to replace the aplmod.
 *               Annotates the aplmod prf with the WLMP_SPLITCAUSE flag.
 *               If only one partition is required shortcuts the anonymous
 *               traversal by calling ReplaceAplmod() instead of going back
 *               up to WLMPpart.
 *
 *   The anonymous traversal, prefixed with ATrav, is responsible for modifying
 *   partitions bounds and replacing the aplmod computation.
 *   Here is a rundown of the responsibilities of the ATrav functions:
 *
 *   ATravPart:   Modifies the partition bounds using SetPartitionBounds().
 *                Traverses into PART_CODE.
 *                Traverses into PART_NEXT.
 *   ATravAssign: Traverses into the non-with-loop sons until the aplmod has
 *                been replaced.
 *   ATravPrf:    Replaces the aplmod using ReplaceAplMod().
 *
 *
 * Helper function implementations:
 *   Many helper functions are dedicated to constucting nodes in the abstract
 *   syntax tree.
 *
 *   Often, AST nodes that have to be created for a partition can be created
 *   by reusing AST nodes for the previous partition.
 *   The CacheWrapper() function provides a way to generate and cache AST nodes
 *   using function pointers. One function is given to create the base, and one
 *   function is given for the recursive step.
 *   The results are stored in the NodeCache struct stored within the INFO
 *   struct. The NodeCache struct consists of the node, the partition for which
 *   the node was created, and whether the cache is initialized.
 *
 *   Some functions have a cache that is not partition-dependent. These nodes
 *   cache the result by storing them directly in the INFO structure.
 *   Functions using either caching method are prefixed with 'Get'.
 *
 *   Node creation is done using functions that cache the result based on the
 *   current partition that is being dealt with.
 *   The advantage of such caching functions is twofold.
 *   We have have no dependency on if or when the functions are called, and
 *   nodes attached to the preassigns are not duplicated with every function
 *   call.
 *   It may be better to create prelude functions instead of generating AST
 *   node manually in this module, but the code already exists and works
 *   pretty well.
 *
 *
 * Related command-line options:
 *   -maxwlmp <n>      (default: 2)
 *     This setting limits when SOP/SRP are applied.
 *     If the required number of partitions to partition the modulo computation
 *     using SOP or SRP exceeds the -maxwlmp setting, the optimization is
 *     not applied.
 *     Note that while there is a one-to-one correspondence between the number
 *     of partitions in the AST and the number of partitions in the math, there
 *     is no such correspondence for the partition's code blocks.
 *     When a partition refers to a code block that other partitions also refer
 *     to, the code block is copied and uniquely assigned to the original
 *     partition.
 *   -dowlmp_aggressive
 *     This setting enables SRP even in cases where it should reduce runtime
 *     performance.
 *     Details are explained in SRP section above.
 *   -suppress_wlmp_code_size
 *     This setting disables warnings for excessive code size increases.
 *   -dosop / nosop (default: do)
 *     These flags enable and disable Shared Offset Partitioning.
 *     If srp is also disabled, traversal of this module is disabled.
 *   -dosrp / nosrp (default: do)
 *     These flags enable and disable Shared Result Partitioning.
 *     If sop is also disabled, traversal of this module is disabled.
 *
 *
 * Limitations/Potential improvements:
 *   - This traversal cannot currently deal with negative index vectors:
 *     The construct _aplmod_SxS_ (-iv + c, x) cannot be optimized.
 *     It is mathematically equivalent to -_aplmod_SxS_ (iv - c, -x), but since
 *     we cannot imagine a use case for this, this case is not dealt with.
 *   - This traversal can currently only deal with a step and width of 1.
 *     This is not because it is impossible to deal with these cases, but rather
 *     because no use case was imagined for it.
 *     If someone decides to tackle this, the most obvious start is to make
 *     the shared result partitioning method apply when the divisor is equal
 *     to the step.
 *   - It may be possible to apply very similar transformations to the
 *     remainder function, denoted as % or _mod_SxS_.
 *     Like with the other potential improvements, however, we have yet to
 *     identify a use case for this, so this idea remains unexplored.
 *   - Similarly, it may be possible to apply a similar transformation to
 *     side-effect free for-loops, although the math has to be redone there
 *     since the lower bound can be lower than 0 in that case.
 *   - SRP will not always be applied due to a limitation in wl_transform.c.
 *     For more explanations on this, look for dowlmp_aggressive in the SRP
 *     section above.
 *
 ******************************************************************************/
#include "globals.h"
#include "memory.h"
#include "constants.h"
#include "DupTree.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "tree_utils.h"
#include "LookUpTable.h"
#include "math_utils.h"
#include "pattern_match.h"
#include "pattern_match_attribs.h"
#include "traverse.h"
#include "flattengenerators.h"
#include "free.h"
#include "with_loop_utilities.h"

#include "print.h"

#define DBUG_PREFIX "WLMP"
#include "debug.h"

#include "wl_modulo_partitioning.h"

#define NODE_TO_INT(node) (COconst2IntAndFree (COaST2Constant (node)))
#define MAKE_NUM_2AVIS(num, arg_info) (                                    \
    FLATGexpression2Avis ((num),                                           \
                          &INFO_VARDECS (arg_info),                        \
                          &INFO_PREASSIGNS (arg_info), NULL))
#define MAKE_PRF1_2AVIS(function, expr1, arg_info) (                       \
    FLATGexpression2Avis (TCmakePrf1 ((function), TBmakeId (expr1)),       \
                          &INFO_VARDECS (arg_info),                        \
                          &INFO_PREASSIGNS (arg_info), NULL))
#define MAKE_PRF2_2AVIS(function, expr1, expr2, arg_info) (                \
    FLATGexpression2Avis (TCmakePrf2 ((function),                          \
                                      TBmakeId (expr1), TBmakeId (expr2)), \
                          &INFO_VARDECS (arg_info),                        \
                          &INFO_PREASSIGNS (arg_info), NULL))

#define ASSIGN_DIRECTLY_CONTAINS_WITH_LOOP(arg_node) (                     \
    NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let                            \
    && NODE_TYPE (LET_EXPR (ASSIGN_STMT (arg_node))) == N_with)

typedef enum {
    WLMP_noop,
    WLMP_shared_offset,
    WLMP_shared_result
} partition_method;


/*******************************************************************************
 * @struct node_cache
 *
 * @brief We use the node_cache struct to easily reuse nodes for each partition.
 * This makes it so we don't have to manually create a control flow for the
 * nodes that we reuse, and can instead retrieve the result whenever we want.
 *
 * @param cache_node The node that's cached.
 * @param partition The partition number that the node was generated for.
 * @param initialized Whether the cache has been initialized, that is to say,
 *                    whether it can be read from.
 *
 ******************************************************************************/
typedef struct node_cache {
    node *cache_node;
    size_t partition;
    bool initialized;
} node_cache;

#define CACHE_NODE(n) ((n)->cache_node)
#define CACHE_PARTITION(n) ((n)->partition)
#define CACHE_INITIALIZED(n) ((n)->initialized)


/*******************************************************************************
 * @fn void ResetNodeCache( node_cache *cache)
 *
 * Resets all values of the given cache to their default.
 * 
 ******************************************************************************/
static void
ResetNodeCache (node_cache *cache)
{
    DBUG_ENTER ();

    CACHE_INITIALIZED (cache) = false;
    CACHE_PARTITION (cache) = 0;
    CACHE_NODE (cache) = NULL;

    DBUG_RETURN ();
}

static node_cache *
MakeNodeCache (void)
{
    node_cache *result;

    DBUG_ENTER ();

    result = (node_cache *)MEMmalloc (sizeof (node_cache));
    ResetNodeCache (result);

    DBUG_RETURN (result);
}

static node_cache *
FreeNodeCache (node_cache **cache)
{
    DBUG_ENTER ();

    *cache = MEMfree (*cache);

    DBUG_RETURN (*cache);
}



/*******************************************************************************
 * @struct stackables
 *
 * @brief The stackables struct is used to easily swap out values that must be
 *        stacked when a new with-loop is entered.
 *        All entries in the stackables are used for the information discovery
 *        part of the traversal.
 *
 * @param locals_lut         A lut containing references to N_assigns that have
 *                           been traversed in the current partition.
 *                           It's used as a set of locals variables.
 * @param with_ids           The with-ids of the current partition.
 * @param lower_bound_array  The lower bound array of the current partition.
 * @param upper_bound_array  The upper bound array of the current partition.
 * @param step_array         The step array of the current partition.
 * @param is_unique_partition_code Whether the current N_code is referred to by
 *                                 exactly 1 partition.
 *
 ******************************************************************************/
typedef struct stackables {
    lut_t *locals_lut; /* We use this lut as a set for locals in partitions */
    node *with_ids;
    node *lower_bound_array;
    node *upper_bound_array;
    node *step_array;
    bool is_unique_partition_code;
} stackables;

#define STACKABLES_LOCALS_LUT(n) ((n)->locals_lut)
#define STACKABLES_WITH_IDS(n) ((n)->with_ids)
#define STACKABLES_LOWER_BOUND_ARRAY(n) ((n)->lower_bound_array)
#define STACKABLES_UPPER_BOUND_ARRAY(n) ((n)->upper_bound_array)
#define STACKABLES_STEP_ARRAY(n) ((n)->step_array)
#define STACKABLES_IS_UNIQUE_PARTITION_CODE(n) ((n)->is_unique_partition_code)

static stackables *
MakeStackables (void)
{
    stackables *result;

    DBUG_ENTER ();

    result = (stackables *)MEMmalloc (sizeof (stackables));
    STACKABLES_LOCALS_LUT (result) = LUTgenerateLut ();
    STACKABLES_WITH_IDS (result) = NULL;
    STACKABLES_LOWER_BOUND_ARRAY (result) = NULL;
    STACKABLES_UPPER_BOUND_ARRAY (result) = NULL;
    STACKABLES_STEP_ARRAY (result) = NULL;
    STACKABLES_IS_UNIQUE_PARTITION_CODE (result) = false;
    
    DBUG_RETURN (result);
}

static stackables *
FreeStackables (stackables **stack)
{
    DBUG_ENTER ();

    if (*stack == NULL) {
        DBUG_RETURN (*stack);
    }

    if (STACKABLES_LOCALS_LUT (*stack) != NULL) {
        LUTremoveLut (STACKABLES_LOCALS_LUT (*stack));
    }
    *stack = MEMfree (*stack);

    DBUG_RETURN (*stack);
}

struct INFO {
    node *fundef;
    node *vardecs;
    node *preassigns;

    /* Stackables hold with-loop specific information that is discovered
       during the traversal. */
    stackables *with_loop_specific_information;
    
    /* These nodes are used to create new expressions */
    node *lower_bound_scalar_avis;
    node *upper_bound_scalar_avis;
    node *iv_scalar_avis;
    node *offset_avis;
    node *abs_divisor_avis;
    node *divisor_avis;
    node *divisor_id;
    size_t partition_dimension;

    /* These are new node caches to reuse nodes */
    node_cache *sop_new_offset_cache;
    node_cache *lower_bound_cache;
    node_cache *upper_bound_cache;

    /* These are also used to cache, but don't use the caching struct */
    node *sop_lower_bound_base;
    node *sop_new_offset_base;
    node *srp_new_step_avis;

    /* These integers are used to signal the required actions */
    size_t partition_method;
    size_t nr_required_partitions;
    size_t nr_current_partition;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)

#define INFO_STACKABLES(n) ((n)->with_loop_specific_information)

#define INFO_LOWER_BOUND_SCALAR_AVIS(n) ((n)->lower_bound_scalar_avis)
#define INFO_UPPER_BOUND_SCALAR_AVIS(n) ((n)->upper_bound_scalar_avis)
#define INFO_IV_SCALAR_AVIS(n) ((n)->iv_scalar_avis)
#define INFO_OFFSET_AVIS(n) ((n)->offset_avis)
#define INFO_ABS_DIVISOR_AVIS(n) ((n)->abs_divisor_avis)
#define INFO_DIVISOR_AVIS(n) ((n)->divisor_avis)
#define INFO_DIVISOR_ID(n) ((n)->divisor_id)
#define INFO_PARTITION_DIMENSION(n) ((n)->partition_dimension)

#define INFO_SOP_NEW_OFFSET_CACHE(n) ((n)->sop_new_offset_cache)
#define INFO_LOWER_BOUND_CACHE(n) ((n)->lower_bound_cache)
#define INFO_UPPER_BOUND_CACHE(n) ((n)->upper_bound_cache)

#define INFO_SOP_LOWER_BOUND_BASE(n) ((n)->sop_lower_bound_base)
#define INFO_SOP_NEW_OFFSET_BASE(n) ((n)->sop_new_offset_base)
#define INFO_SRP_NEW_STEP_AVIS(n) ((n)->srp_new_step_avis)

#define INFO_PARTITION_METHOD(n) ((n)->partition_method)
#define INFO_NR_REQUIRED_PARTITIONS(n) ((n)->nr_required_partitions)
#define INFO_NR_CURRENT_PARTITION(n) ((n)->nr_current_partition)

/* Compound macros */
#define INFO_FOUND_SPLITCAUSE(n) (INFO_PARTITION_METHOD(n) != WLMP_noop)
#define INFO_IS_INSIDE_WITH_LOOP(n) (INFO_STACKABLES(n) != NULL)

#define INFO_LOCALS_LUT(n) (STACKABLES_LOCALS_LUT(INFO_STACKABLES(n)))
#define INFO_WITH_IDS(n) (STACKABLES_WITH_IDS(INFO_STACKABLES(n)))
#define INFO_LOWER_BOUND_ARRAY(n) (STACKABLES_LOWER_BOUND_ARRAY(INFO_STACKABLES(n)))
#define INFO_UPPER_BOUND_ARRAY(n) (STACKABLES_UPPER_BOUND_ARRAY(INFO_STACKABLES(n)))
#define INFO_STEP_ARRAY(n) (STACKABLES_STEP_ARRAY(INFO_STACKABLES(n)))
#define INFO_IS_UNIQUE_PARTITION_CODE(n) (STACKABLES_IS_UNIQUE_PARTITION_CODE(INFO_STACKABLES(n)))

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;

    // The stackables struct containing with-loop specific
    // information is intentionally uninitialized because
    // the traversal does not start inside a with-loop.
    INFO_STACKABLES (result) = NULL;

    INFO_LOWER_BOUND_SCALAR_AVIS (result) = NULL;
    INFO_UPPER_BOUND_SCALAR_AVIS (result) = NULL;
    INFO_IV_SCALAR_AVIS (result) = NULL;
    INFO_OFFSET_AVIS (result) = NULL;
    INFO_ABS_DIVISOR_AVIS (result) = NULL;
    INFO_DIVISOR_AVIS (result) = NULL;
    INFO_DIVISOR_ID (result) = NULL;
    INFO_PARTITION_DIMENSION (result) = 0;

    INFO_SOP_NEW_OFFSET_CACHE (result) = MakeNodeCache();
    INFO_LOWER_BOUND_CACHE (result) = MakeNodeCache();
    INFO_UPPER_BOUND_CACHE (result) = MakeNodeCache();

    INFO_SOP_LOWER_BOUND_BASE (result) = NULL;
    INFO_SOP_NEW_OFFSET_BASE (result) = NULL;
    INFO_SRP_NEW_STEP_AVIS (result) = NULL;

    INFO_PARTITION_METHOD (result) = WLMP_noop;
    INFO_NR_REQUIRED_PARTITIONS (result) = 0;
    INFO_NR_CURRENT_PARTITION (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    FreeStackables (&INFO_STACKABLES (info));

    FreeNodeCache (&INFO_SOP_NEW_OFFSET_CACHE (info));
    FreeNodeCache (&INFO_LOWER_BOUND_CACHE (info));
    FreeNodeCache (&INFO_UPPER_BOUND_CACHE (info));

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*******************************************************************************
 * @fn void ResetNodeCaches( info *arg_info)
 *
 * Resets all node_cache caches and all node pointers that function as 
 * partition-independent caches to their default state.
 * 
 ******************************************************************************/
static void
ResetNodeCaches (info *arg_info)
{
    DBUG_ENTER ();

    // Reset partition-dependent caches
    ResetNodeCache (INFO_SOP_NEW_OFFSET_CACHE (arg_info));
    ResetNodeCache (INFO_LOWER_BOUND_CACHE (arg_info));
    ResetNodeCache (INFO_UPPER_BOUND_CACHE (arg_info));

    // Reset partition-independent caches
    INFO_SOP_LOWER_BOUND_BASE (arg_info) = NULL;
    INFO_SOP_NEW_OFFSET_BASE (arg_info) = NULL;
    INFO_SRP_NEW_STEP_AVIS (arg_info) = NULL;
    
    DBUG_RETURN ();
}

node *
WLMPdoWithLoopModuloPartitioning( node *arg_node)
{
    info *info;

    DBUG_ENTER();

    info = MakeInfo ();

    TRAVpush (TR_wlmp);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN(arg_node);
}

/*******************************************************************************
 * Common node creation functions
 ******************************************************************************/

/*******************************************************************************
 * @fn node *CreateAplMod( node *iv, info *arg_info)
 *
 * @brief Returns a flattened avis of the form _aplmod_SxS_(iv + c, x)
 *        where iv = iv
 *              c  = INFO_OFFSET_AVIS (arg_info)
 *              x  = INFO_DIVISOR_AVIS (arg_info)
 *
 ******************************************************************************/
static node *
CreateAplMod (node *iv, info *arg_info)
{
    node *iv_plus_offset;
    node *res;

    DBUG_ENTER ();

    iv_plus_offset = MAKE_PRF2_2AVIS (F_add_SxS,
                                      iv,
                                      INFO_OFFSET_AVIS (arg_info), arg_info);

    res = MAKE_PRF2_2AVIS (F_aplmod_SxS,
                           iv_plus_offset, 
                           INFO_DIVISOR_AVIS (arg_info), arg_info);
    DBUG_RETURN (res);
}

/*******************************************************************************
 * @fn node *AddLimitedAbsDivisor (node *previous, info *arg_info)
 *
 * @brief Creates the following expression:
 *          min(previous + abs(divisor), INFO_UPPER_BOUND_SCALAR_AVIS)
 * 
 * @note For this function's use in the Shared Offset Partitioning, the min() 
 *       part is not necessary when abs(x) <= u or the exact number of required
 *       partitions is computed, but implementing that does not seem worth the
 *       added complexity at this time.
 *
 ******************************************************************************/
static node *
AddLimitedAbsDivisor (node *previous, info *arg_info)
{
    DBUG_ENTER ();
    
    DBUG_RETURN (MAKE_PRF2_2AVIS(
        F_min_SxS,
        MAKE_PRF2_2AVIS (
            F_add_SxS, 
            previous,
            INFO_ABS_DIVISOR_AVIS (arg_info), arg_info
        ),
        INFO_UPPER_BOUND_SCALAR_AVIS (arg_info), arg_info
    ));
}

/*******************************************************************************
 * @fn node *CacheWrapper (node_cache *cache,
 *                         cache_base_func BaseFunc,
 *                         cache_rec_func RecFunc,
 *                         info *arg_info)
 *
 * @brief This function uses BaseFunc and RecFunc to recursively generate nodes
 *        for the current partition. 
 *        The current partition is taken from arg_info.
 *        Results are stored in a cache.
 *
 * @param cache         The nodecache used to cache the result of this function.
 * @param baseFunc      The function used to generate the node for the first 
 *                      partition.
 * @param RecursionFunc The function used to generate the N+1th partition from
 *                      the Nth partition.
 *
 * Motivation:
 *  The modulo computations always require some initial computation for the first
 *  partition that can relatively costly at runtime.
 *  Subsequent partitions can be computed from the previous results in a cheaper
 *  manner.
 *  This is why this recursive approach was chosen.
 *  Is this overengineered? Probably, but it exists and it works.
 *  If it stops working, consider rewriting this module by moving the AST code
 *  generation part to the prelude. Doing so may save a thousand lines of
 *  code in this file.
 *
 ******************************************************************************/
typedef node * (*cache_base_func)(info *);
typedef node * (*cache_rec_func)(node *, info *);

static node *
CacheWrapper (node_cache *cache, 
               cache_base_func BaseFunc, 
               cache_rec_func RecFunc,
               info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (CACHE_PARTITION (cache) <= INFO_NR_CURRENT_PARTITION (arg_info),
                 "Inconsistent cache w.r.t. the current partition!\n"
                 "The cache partition %zu exceeds the current partition %zu!",
                 CACHE_PARTITION (cache), INFO_NR_CURRENT_PARTITION (arg_info));

    // Instantly return the result if it has already been computed
    if (CACHE_INITIALIZED (cache) &&
            INFO_NR_CURRENT_PARTITION (arg_info) == CACHE_PARTITION (cache)) {
        DBUG_PRINT_TAG("WLMP_CACHE", "Using cache for partition %zu",
                       CACHE_PARTITION (cache));
        DBUG_RETURN (CACHE_NODE (cache));
    }

    // Create the base case if necessary
    if (!CACHE_INITIALIZED (cache)) {
        CACHE_NODE (cache) = BaseFunc (arg_info);
        CACHE_PARTITION (cache) = 0;
        CACHE_INITIALIZED (cache) = true;
    }

    // Iterate over the recursive case until we arrive at the desired partition
    while (CACHE_PARTITION (cache) < INFO_NR_CURRENT_PARTITION  (arg_info)) {
        DBUG_PRINT_TAG("WLMP_CACHE", "Populating cache for partition %zu",
                       CACHE_PARTITION(cache) + 1);
        CACHE_NODE (cache) = RecFunc (CACHE_NODE (cache), arg_info);
        CACHE_PARTITION (cache)++;
    }

    DBUG_RETURN (CACHE_NODE (cache));
}

/*******************************************************************************
 * Shared offset partitioning method node creation functions
 ******************************************************************************/

/*******************************************************************************
 * @fn node *GetSOPnewOffset( info *arg_info)
 * @fn node *SOPnewOffsetBase( info *arg_info)
 * @fn node *SOPnewOffsetRec( node *previous, info *arg_info)
 *
 * @brief Provides an avis `res` such that
 *          iv + res == _aplmod_SxS_ (iv + c, x)
 *        for the current partition in the shared offset partitioning method.
 * 
 * The provided result is equal to _aplmod_SxS_(l + c, x) + -l
 * where l = lower bound of current partition
 *       c = INFO_OFFSET_AVIS (arg_info)
 *       x = INFO_DIVISOR_AVIS (arg_info)
 *
 * Base case:
 *   The full aplmod computation as stated above is created.
 *   Since this has multiple calling sites, we cache the result.
 * 
 * Recursive case:
 *   Adds -abs(x) to the previous result.
 *
 ******************************************************************************/
static node *
SOPnewOffsetBase (info *arg_info)
{
    DBUG_ENTER ();

    // If the base case is cached, retrieve it.
    if (INFO_SOP_NEW_OFFSET_BASE (arg_info) != NULL) {
        DBUG_RETURN (INFO_SOP_NEW_OFFSET_BASE (arg_info));
    }

    // Base case: create a flattened avis for _aplmod_SxS_(l + c, x) + -l
    // +-l is to preserve esd form
    INFO_SOP_NEW_OFFSET_BASE (arg_info) = MAKE_PRF2_2AVIS (
        F_add_SxS,
        CreateAplMod (INFO_LOWER_BOUND_SCALAR_AVIS (arg_info), arg_info),
        MAKE_PRF1_2AVIS (F_neg_S, INFO_LOWER_BOUND_SCALAR_AVIS (arg_info), arg_info), arg_info);

    DBUG_RETURN (INFO_SOP_NEW_OFFSET_BASE (arg_info));
}

static node *
SOPnewOffsetRec (node *previous, info *arg_info)
{
    DBUG_ENTER ();

    // Recursive case: res = previous + -abs(divisor)
    // The plus minus is to preserve esd form
    DBUG_RETURN (MAKE_PRF2_2AVIS (
        F_add_SxS,
        previous,
        MAKE_PRF1_2AVIS (F_neg_S, INFO_ABS_DIVISOR_AVIS (arg_info), arg_info), arg_info)
    );
}

static node *
GetSOPnewOffset (info *arg_info)
{
    DBUG_ENTER ();
    DBUG_RETURN (CacheWrapper (
        INFO_SOP_NEW_OFFSET_CACHE (arg_info),
        &SOPnewOffsetBase,
        &SOPnewOffsetRec,
        arg_info)
    );
}

/*******************************************************************************
 * @fn node *GetSOPlowerBound( info *arg_info)
 * @fn node *SOPlowerBoundBase( info *arg_info)
 *
 * @brief Provides the lower bound for the current partition in the shared offset 
 *        partitioning method.
 *
 * Base case: 
 *   Creates the first lower bound for the shared offset partitioning method as
 *   an avis.
 *   The formulae are added to INFO_PREASSIGNS in flattened SSA ESD form:
 *   - one prf per assign
 *   - one assign per variable
 *   - no (binary) subtraction/division operators
 * 
 *   We create these formulae:
 *    zero_case      = toi(x == 0) * l
 *    base_case      = toi(x != 0) * -replacement_offset 
 *    neg_adjustment = toi(x  < 0) * (x + 1))
 *    initial_lower_bound = zero_case + base_case + negative_adjustment
 *
 *   The variable replacement_offset is `aplmod(l + c, x) + -l`, as computed
 *   by GetSOPnewOffset where l is the lower bound
 *                                      c is the offset
 *                                      x is the divisor
 *
 *   Some more insight into the different cases:
 *   x = 0:  At runtime we might discover that we have a useless partition since
 *           we don't always have enough information at compile-time to compute
 *           the exact number of required partitions.
 *           Specifically, we can get two partitions for x = 0.
 *           Having extra partitions is no issue as long as redundant partitions
 *           range over no values, e.g. l <= iv < l.  
 *           Setting the lower bound to l when x = 0 achieves exactly that.
 *           We'll get partitions of l         <= iv < l + x = l 
 *                             and l + x = l <= iv < u
 *   x != 0: The lower bound of a partition just so happens to reuse the
 *           negative replacement offset.
 *   x < 0:  For negative x, a partition starts at x + 1 instead of at 0, so we
 *           adjust for that.
 *           For example, given x = -5, for iv in -4 <= iv < 1, the range is
 *           continuous: [-4, -3, -2, -1, 0]
 *           Whereas for x = 5, for iv in 0 <= iv < 5, the range is continuous:
 *           [0, 1, 2, 3, 4]
 * 
 * Recursive case:
 *   min(previous + abs(divisor), INFO_UPPER_BOUND_SCALAR_AVIS)
 *
 ******************************************************************************/
static node *
SOPlowerBoundBase (info *arg_info)
{
    node *zero;
    node *one;

    node *lower_bound;
    node *divisor;
    node *replacement_offset;

    node *x_equals_zero;
    node *int_x_equals_zero;
    node *zero_case;

    node *x_neq_zero;
    node *int_x_neq_zero;
    node *neg_replacement_offset;
    node *base_case;

    node *x_lt_zero;
    node *int_x_lt_zero;
    node *x_plus_one;
    node *neg_adjustment;

    node *zero_case_plus_base_case;
    node *initial_lower_bound;

    DBUG_ENTER ();

    if (INFO_SOP_LOWER_BOUND_BASE (arg_info) != NULL) {
        DBUG_RETURN (INFO_SOP_LOWER_BOUND_BASE (arg_info));
    }
    
    zero = MAKE_NUM_2AVIS (TBmakeNum (0), arg_info);
    one = MAKE_NUM_2AVIS (TBmakeNum (1), arg_info);

    lower_bound = INFO_LOWER_BOUND_SCALAR_AVIS (arg_info);
    divisor = INFO_DIVISOR_AVIS (arg_info);
    replacement_offset = SOPnewOffsetBase (arg_info);

    // Construct the zero case: toi(divisor == 0) * lower_bound
    x_equals_zero = MAKE_PRF2_2AVIS (F_eq_SxS, divisor, zero, arg_info);
    int_x_equals_zero = MAKE_PRF1_2AVIS (F_toi_S, x_equals_zero, arg_info);
    zero_case = MAKE_PRF2_2AVIS (F_mul_SxS, int_x_equals_zero, lower_bound, arg_info);
    
    // Construct the base case: toi(divisor != 0) * -replacement_offset 
    x_neq_zero = MAKE_PRF2_2AVIS (F_neq_SxS, divisor, zero, arg_info);
    int_x_neq_zero = MAKE_PRF1_2AVIS (F_toi_S, x_neq_zero, arg_info);
    neg_replacement_offset = MAKE_PRF1_2AVIS (F_neg_S, replacement_offset, arg_info);
    base_case = MAKE_PRF2_2AVIS (F_mul_SxS, int_x_neq_zero, neg_replacement_offset, arg_info);
    
    // Construct the negative adjustment: toi(divisor < 0) * (divisor + 1))
    x_lt_zero = MAKE_PRF2_2AVIS (F_lt_SxS, divisor, zero, arg_info);
    int_x_lt_zero = MAKE_PRF1_2AVIS (F_toi_S, x_lt_zero, arg_info);
    x_plus_one = MAKE_PRF2_2AVIS (F_add_SxS, divisor, one, arg_info);
    neg_adjustment = MAKE_PRF2_2AVIS (F_mul_SxS, int_x_lt_zero, x_plus_one, arg_info);

    // Construct the final result:
    // Zero case + base case + neg adjustment
    zero_case_plus_base_case = MAKE_PRF2_2AVIS (F_add_SxS, zero_case, base_case, arg_info);
    initial_lower_bound = MAKE_PRF2_2AVIS (F_add_SxS, zero_case_plus_base_case, neg_adjustment, arg_info);

    INFO_SOP_LOWER_BOUND_BASE (arg_info) = initial_lower_bound;
    DBUG_RETURN (initial_lower_bound);
}

static node *
GetSOPlowerBound (info *arg_info)
{
    DBUG_ENTER ();
    DBUG_RETURN (CacheWrapper (
        INFO_LOWER_BOUND_CACHE (arg_info),
        &SOPlowerBoundBase,
        &AddLimitedAbsDivisor,
        arg_info)
    );
}

/*******************************************************************************
 * @fn node *GetSOPupperBound( info *arg_info)
 * @fn node *SOPupperBoundBase( info *arg_info)
 * 
 * @brief Provides the upper bound for the current partition in the shared
 *        offset partitioning method.
 *
 * Base case:
 *   min(SOPlowerBoundBase + abs(divisor), INFO_UPPER_BOUND_SCALAR_AVIS).
 *
 * Recursive case:
 *   min(previous + abs(divisor), INFO_UPPER_BOUND_SCALAR_AVIS).
 *
 ******************************************************************************/
static node *
SOPupperBoundBase (info *arg_info)
{
    DBUG_ENTER ();
    DBUG_RETURN (AddLimitedAbsDivisor (SOPlowerBoundBase (arg_info), arg_info));
}

static node *
GetSOPupperBound (info *arg_info)
{
    DBUG_ENTER ();
    DBUG_RETURN (CacheWrapper (
        INFO_UPPER_BOUND_CACHE (arg_info),
        &SOPupperBoundBase,
        &AddLimitedAbsDivisor,
        arg_info)
    );
}

/*******************************************************************************
 * Shared result partitioning method node creation functions
 ******************************************************************************/

/*******************************************************************************
 * @fn node *GetSRPlowerBound( info *arg_info)
 * @fn node *SRPlowerBoundBase( info *arg_info)
 * @fn node *SRPlowerBoundRec( node *previous, info *arg_info)
 *
 * @brief Provides the lower bound for the current partition in the shared
 *        result partitioning method.
 *
 * Base case: 
 *   Returns the original lower bound
 * 
 * Recursive case:
 *   Adds 1 to the previous result.
 *
 ******************************************************************************/
static node *
SRPlowerBoundBase (info *arg_info)
{
    DBUG_ENTER ();
    DBUG_RETURN (INFO_LOWER_BOUND_SCALAR_AVIS (arg_info));
}

static node *
SRPlowerBoundRec (node *previous, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_RETURN (MAKE_PRF2_2AVIS (
        F_add_SxS,
        INFO_LOWER_BOUND_SCALAR_AVIS (arg_info),
        MAKE_NUM_2AVIS (TBmakeNum (1), arg_info), arg_info)
    );
}

static node *
GetSRPlowerBound (info *arg_info)
{
    DBUG_ENTER ();
    DBUG_RETURN (CacheWrapper (
        INFO_LOWER_BOUND_CACHE (arg_info),
        &SRPlowerBoundBase,
        &SRPlowerBoundRec,
        arg_info)
    );
}

/*******************************************************************************
 * @fn node *GetSRPupperBound( info *arg_info)
 *
 * @brief Gets the upper bound for the current partition in the shared result
 *        partitioning method - which is just the original upper bound.
 * 
 * @return The avis of the original upper bound.
 *
 ******************************************************************************/
static node *
GetSRPupperBound (info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (INFO_UPPER_BOUND_SCALAR_AVIS (arg_info));
}

/*******************************************************************************
 * @fn void SetSRPstep( node *partition, info *arg_info)
 *
 * @brief Replaces the index that matches the current dimension in the given 
 *        partition's step with abs(divisor).
 *
 * If no step array exists yet, a step array will first be initialized with
 * ones.
 * 
 ******************************************************************************/
static void
SetSRPstep (node *partition, info *arg_info)
{
    DBUG_ENTER ();

    // If we've already generated a new step for the partitions, reuse it
    // after potentially freeing the old step.
    if (INFO_SRP_NEW_STEP_AVIS (arg_info) != NULL) {
        PART_STEP (partition) = FREEoptFreeTree (PART_STEP (partition));
        PART_STEP (partition) = TBmakeId (INFO_SRP_NEW_STEP_AVIS (arg_info));
        DBUG_RETURN ();
    }

    // If we haven't generated a new step yet, we generate one that we
    // directly adjust and insert into PART_STEP(partition)

    // If the partition has no step size yet, create one with an array of 1s.
    if (PART_STEP (partition) == NULL) {
        PART_STEP (partition) = TCcreateIntVector (
            (int)(TCcountIds (INFO_WITH_IDS (arg_info))), 1, 0);
    }

    // Replace the step entry for the dimension that we operate on with
    // abs(divisor).
    WLUTupdateBoundNthDim (&PART_STEP (partition), 
                            INFO_PARTITION_DIMENSION (arg_info),
                            INFO_ABS_DIVISOR_AVIS (arg_info),
                            &INFO_VARDECS (arg_info),
                            &INFO_PREASSIGNS (arg_info));

    // We force the bound to become an id and store the avis for reuse in
    // other partitions.
    switch (NODE_TYPE (PART_STEP (partition))) {
    case N_id:
        INFO_SRP_NEW_STEP_AVIS (arg_info) = ID_AVIS (PART_STEP (partition));
        break;
    case N_array:
        INFO_SRP_NEW_STEP_AVIS (arg_info) = 
            FLATGexpression2Avis (PART_STEP (partition), 
                                  &INFO_VARDECS (arg_info), 
                                  &INFO_PREASSIGNS (arg_info), NULL);
        PART_STEP (partition) = TBmakeId (INFO_SRP_NEW_STEP_AVIS (arg_info));
        break;
    default:
        DBUG_UNREACHABLE ("Illegal node type %s for step!",
                          NODE_TEXT (PART_STEP (partition)));
    }
    DBUG_RETURN ();
}

/*******************************************************************************
 * AST modification functions
 ******************************************************************************/

/*******************************************************************************
 * @fn node *SetPartitionBounds( node *partition, info *arg_info)
 * 
 * @brief Sets the partition bounds for `partition` based on
 *        INFO_PARTITION_METHOD, INFO_NR_CURRENT_PARTITION, and
 *        INFO_NR_REQUIRED_PARTITIONS.
 * 
 * For the shared result partitioning method, bounds are transformed to:
 *     ([..., l + partition_nr, ...] 
 *      <= i <
 *      [..., max(u, l + partition_nr), ...]
 *      step [..., abs(x), ...])
 * 
 * For the shared offset partitioning method, bounds are transformed to:
 *     ([..., toi(x == 0) * l 
 *          + toi(x != 0) * -(aplmod(l + c, x) + -l)
 *          + toi(x  < 0) * (x + 1)), ...])
 *      <= i <
 *      [..., the new lower bound + abs(x), ...])
 *
 * where l is the lower bound
 *       c is the offset
 *       x is the divisor
 *
 ******************************************************************************/
static node *
SetPartitionBounds (node *partition, info *arg_info)
{
    size_t partition_method;
    size_t nr_current_partition;
    size_t nr_required_partitions;
    size_t dimension;

    DBUG_ENTER ();

    partition_method = INFO_PARTITION_METHOD (arg_info);
    nr_current_partition = INFO_NR_CURRENT_PARTITION (arg_info);
    nr_required_partitions = INFO_NR_REQUIRED_PARTITIONS (arg_info);
    dimension = INFO_PARTITION_DIMENSION (arg_info);

    DBUG_ASSERT (nr_required_partitions != 0, "Got 0 partitions!");

    switch (partition_method) {
    case WLMP_shared_result:
        // Set the bounds and update or create the step
        WLUTupdateBoundNthDim (&PART_BOUND1 (partition), dimension, 
                               GetSRPlowerBound (arg_info),
                               &INFO_VARDECS (arg_info),
                               &INFO_PREASSIGNS (arg_info));
        WLUTupdateBoundNthDim (&PART_BOUND2 (partition), dimension,
                               GetSRPupperBound (arg_info),
                               &INFO_VARDECS (arg_info),
                               &INFO_PREASSIGNS (arg_info));
        SetSRPstep (partition, arg_info);
        break;
    case WLMP_shared_offset:
        // Adjust the bounds ONLY if they aren't the outer bounds.
        if (nr_current_partition != 0) {
            WLUTupdateBoundNthDim (&PART_BOUND1 (partition), dimension,
                                   GetSOPlowerBound (arg_info),
                                   &INFO_VARDECS (arg_info),
                                   &INFO_PREASSIGNS (arg_info));
        }
        if (nr_current_partition + 1 != nr_required_partitions) {
            WLUTupdateBoundNthDim (&PART_BOUND2 (partition), dimension,
                                   GetSOPupperBound (arg_info),
                                   &INFO_VARDECS (arg_info),
                                   &INFO_PREASSIGNS (arg_info));
        }
        break;
    default:
        DBUG_UNREACHABLE ("Got an unexpected partition method: %zu", 
                          partition_method);
    }
    DBUG_RETURN (partition);
}

/*******************************************************************************
 * @fn node *ReplaceAplmod( node *aplmod, info *arg_info)
 * 
 * @brief Replaces the given aplmod node with the optimized version based on the
 *        INFO_PARTITION_METHOD and the INFO_NR_CURRENT_PARTITION.
 *
 ******************************************************************************/
static node *
ReplaceAplmod (node *aplmod, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (aplmod != NULL, "Got NULL instead of an aplmod");
    DBUG_ASSERT (PRF_PRF (aplmod) == F_aplmod_SxS, "Wrong primitive function");
    DBUG_ASSERT (PRF_WLMP_SPLITCAUSE (aplmod), "Aplmod is not replaceable!");

    switch (INFO_PARTITION_METHOD (arg_info)) {
    case WLMP_shared_result:
        // Since we replaced the step in the partition, all values in
        // this partition share the same result.
        // As such, we replace the partition's aplmod with a reference to
        // the equivalent aplmod, computed once outside of the with-loop.
        aplmod = FREEdoFreeTree (aplmod);
        aplmod = TBmakeId (CreateAplMod (GetSRPlowerBound (arg_info),
                                         arg_info));
        DBUG_PRINT ("Modulo replaced with id %s", ID_NAME (aplmod));
        break;
    case WLMP_shared_offset:
        // We replace a partition's aplmod with an addition with a
        // with-loop invariant offset so we only have to compute
        // the modulo (for the offset) once per partition.
        aplmod = FREEdoFreeTree (aplmod);
        aplmod = TCmakePrf2 (F_add_SxS, 
                               TBmakeId (INFO_IV_SCALAR_AVIS (arg_info)),
                               TBmakeId (GetSOPnewOffset (arg_info)));
        DBUG_PRINT ("Modulo replaced with _add_SxS_( %s, %s)",
                    AVIS_NAME (INFO_IV_SCALAR_AVIS (arg_info)),
                    AVIS_NAME (GetSOPnewOffset (arg_info)));
        break;
    default:
        DBUG_UNREACHABLE ("Illegal partition method!");
    }
    
    // We reset the caches only if this was the final partition for which we
    // have to replace this aplmod.
    if (INFO_NR_CURRENT_PARTITION (arg_info) + 1 
            == INFO_NR_REQUIRED_PARTITIONS (arg_info)) {
        ResetNodeCaches (arg_info);
    }

    DBUG_RETURN (aplmod);
}

/*******************************************************************************
 * Anonymous traversal functions
 *
 * The anonymous traversal is responsible for modifying partition bounds
 * as well as replacing the aplmod that caused the split
 ******************************************************************************/

/*******************************************************************************
 * @fn node *ATravPart( node *partition, info *arg_info)
 *
 * Adjusts the bounds of the partition and traverses into PART_CODE before
 * traversing into PART_NEXT.
 *
 ******************************************************************************/
static node *
ATravPart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    // Adjust the partition bounds and traverse into its code to adjust
    // the modulo within.
    arg_node = SetPartitionBounds (arg_node, arg_info);
    PART_CBLOCK (arg_node) = TRAVdo (PART_CBLOCK (arg_node), arg_info);

    // Continue the traversal for the remaining partitions
    INFO_NR_CURRENT_PARTITION (arg_info)++;
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
    INFO_NR_CURRENT_PARTITION (arg_info)--;

    DBUG_RETURN (arg_node);
}


/*******************************************************************************
 * @fn node *ATravAssign( node *arg_node, info *arg_info)
 *
 * Traverses into the statement if it is not a with-node.
 * Traverses into the next assign if the aplmod has not been replaced yet.
 *
 ******************************************************************************/
static node *
ATravAssign(node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    // Traverse this node if it is not a with-node.
    // The anonymous traversal can only traverse one with-loop level at a time.
    if (!(ASSIGN_DIRECTLY_CONTAINS_WITH_LOOP (arg_node))) {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    }

    // Stop traversing if we already replaced the aplmod.
    if (INFO_FOUND_SPLITCAUSE (arg_info)) {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*******************************************************************************
 * @fn node *ATravPrf( node *arg_node, info *arg_info)
 *
 * If this node is marked as the splitcause, change this node to a different
 * prf with addition, or replace it with an N_id node, depending on the
 * partition method.
 *
 ******************************************************************************/
static node *
ATravPrf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_WLMP_SPLITCAUSE (arg_node)) {
        DBUG_ASSERT (PRF_PRF (arg_node) == F_aplmod_SxS, "Unexpected prf node");
        arg_node = ReplaceAplmod (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*******************************************************************************
 * Support functions for the main traversal
 ******************************************************************************/

/*******************************************************************************
 * @fn node *CopyAndModifyPartitions( node *arg_node, info *arg_info)
 *
 * Copies and inserts the required number of partitions as determined by
 * INFO_NR_REQUIRED_PARTITIONS (arg_info).
 * It then launches an anonymous traversal into these partitions to modify
 * their bounds and the prf node that caused the split. 
 * 
 ******************************************************************************/
static node *
CopyAndModifyPartitions (node *arg_node, info *arg_info)
{
    node *remaining_partition_chain;
    node *remaining_code_chain;

    DBUG_ENTER ();
    
    // Isolate the partition and code from the rest of the chain so copies
    // can be made locally and we can iterate over the nodes.
    remaining_partition_chain = PART_NEXT (arg_node);
    remaining_code_chain = CODE_NEXT (PART_CODE (arg_node));
    PART_NEXT (arg_node) = NULL;
    CODE_NEXT (PART_CODE (arg_node)) = NULL;

    arg_node = 
        WLUTcreatePartitionCopies (INFO_FUNDEF (arg_info), arg_node, 
                                   INFO_NR_REQUIRED_PARTITIONS (arg_info));
    DBUG_PRINT ("Created partitions");
    switch (INFO_PARTITION_METHOD (arg_info)) {
    case WLMP_shared_offset:
        global.optcounters.wlmp_sop_partitions_created += 
            INFO_NR_REQUIRED_PARTITIONS (arg_info);
        break;
    case WLMP_shared_result:
        global.optcounters.wlmp_srp_partitions_created +=
            INFO_NR_REQUIRED_PARTITIONS (arg_info);
        break;
    default:
        DBUG_ASSERT (false, "Unreachable code.");
    }
    

    // Launch the anonymous traversal into ALL copies to adjust the trees.
    // Specifically, in partitions, the bounds are altered, and in the
    // code block, the aplmod prf gets reduced to a simpler operation.
    anontrav_t modification_traversal[4] = 
        {{N_part, &ATravPart}, {N_assign, &ATravAssign}, {N_prf, &ATravPrf},
         {(nodetype)0, NULL}};

    TRAVpushAnonymous (modification_traversal, &TRAVsons);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    // Restore the partitions
    arg_node = TCappendPart (arg_node, remaining_partition_chain);
    PART_CODE (arg_node) = TCappendCode (PART_CODE (arg_node), 
                                            remaining_code_chain);

    DBUG_RETURN (arg_node);
}

/*******************************************************************************
 * @fn node *EvaluatePartition( node *partition, info *arg_info)
 * 
 * @brief Discovers whether SOP or SRP can be applied to the partition's code
 *        and applies the best option using CopyAndModifyPartitions.
 *        This is recursively repeated until neither optimization can be
 *        applied.
 * 
 ******************************************************************************/
static node *
EvaluatePartition (node *partition, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT("Considering partition:");
    DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, PART_GENERATOR (partition)));

    /*****************************************************
     * Phase 1: Collect information about this partition *
     ****************************************************/

    // Collect and store partition bounds and step as arrays in INFO
    INFO_LOWER_BOUND_ARRAY (arg_info) = WLUTfindArrayForBound (PART_BOUND1 (partition));
    INFO_UPPER_BOUND_ARRAY (arg_info) = WLUTfindArrayForBound (PART_BOUND2 (partition));
    INFO_STEP_ARRAY (arg_info) = WLUTfindArrayForBound (PART_STEP (partition));
    // Infer whether the partition's code is uniquely used by that partition
    // so we can potentially shortcut partition creation in WLMPprf.
    INFO_IS_UNIQUE_PARTITION_CODE (arg_info) = CODE_USED (PART_CODE (partition)) == 1;

    // Traverse into into N_Code's CBlock to discover the required changes
    PART_CBLOCK (partition) = TRAVdo (PART_CBLOCK (partition), arg_info);

    // Reset only the lut contents - we often have multiple partitions
    // and can reuse the lut itself.
    INFO_LOCALS_LUT (arg_info) = 
        LUTremoveContentLut (INFO_LOCALS_LUT (arg_info));

     /**************************************
     * Phase 2: Base case & Recursive case *
     ***************************************/

    // Base case: return if no optimization can be applied.
    if (!INFO_FOUND_SPLITCAUSE (arg_info)) {
        DBUG_PRINT ("Abort partition: no reason to split found");
        DBUG_RETURN (partition);
    }

    // Recursive case: copy the partitions and apply the optimizations,
    // then repeat this in case more optimizations can be applied.

    DBUG_PRINT ("Partition will be split using the shared %s partitioning method",
                INFO_PARTITION_METHOD (arg_info) == WLMP_shared_offset
                ? "offset" : "result");
    DBUG_PRINT ("Number of partitions: %zu", 
                INFO_NR_REQUIRED_PARTITIONS (arg_info));
    partition = CopyAndModifyPartitions (partition, arg_info);
    DBUG_PRINT ("Modifying newly created partitions completed");

    // Reset the flag that indicates whether we should do anything after
    // the traversal
    INFO_PARTITION_METHOD (arg_info) = WLMP_noop;

    // Do it again since only one modulo call can be dealt with each time.
    DBUG_RETURN (EvaluatePartition (partition, arg_info));
}

/*******************************************************************************
 * @fn size_t DetermineNrSharedResultPartitions( bool is_const_lower_bound,
 *                                               bool is_const_upper_bound,
 *                                               bool is_const_divisor, 
 *                                               int divisor)
 *
 * @brief Determines how many partitions are necessary for the shared result
 *        partitioning method, returning 0 if the method cannot be applied.
 * 
 ******************************************************************************/
static size_t
DetermineNrSharedResultPartitions (bool is_const_lower_bound,
                                   bool is_const_upper_bound,
                                   bool is_const_divisor,
                                   int divisor)
{
    bool is_beneficial;

    DBUG_ENTER ();

    if (!global.optimize.dosrp) { // Exit if SRP is disabled.
         DBUG_RETURN (0); 
    }

    if (!is_const_divisor || divisor == 0) {
        DBUG_RETURN (0);
    }

    // Due to limitations in wl_transform.c, it is only beneficial to apply
    // SRP when the lower and upper bounds are known, when abs(divisor) is 1,
    // or when the target is cuda.
    is_beneficial = (is_const_lower_bound && is_const_upper_bound)
                    || abs(divisor) == 1
                    || global.backend == BE_cuda 
                    || global.backend == BE_cudahybrid;
    // We also add a command-line override -dowlmp_aggressive to make SRP
    // a candidate when the lower and upper bounds aren't known, but enabling
    // that comes at the cost of performance and is mainly meant for debugging.
    if (!is_beneficial && !global.dowlmp_aggressive) {
        DBUG_PRINT ("Cannot apply SRP with step size %d due to wl_transform.",
                    divisor);
        DBUG_RETURN (0);
    }
    DBUG_RETURN ((size_t) abs(divisor));
}

/*******************************************************************************
 * @fn size_t DetermineNrSharedOffsetPartitions(
 *      node *upper_bound_scalar, node *divisor_scalar,
 *      bool is_const_upper_bound, bool is_const_divisor, bool is const_offset,
 *      int lower_bound, int upper_bound, int divisor, int offset)
 *
 * @brief Determines how many partitions are necessary for shared offset
 *        partitioning method, returning 0 if the method cannot be applied.
 *
 * The parameters upper_bound, divisor and offset should ONLY ever be used
 * if the corresponding is_const argument is true.
 * 
 * The lower_bound parameter can always be used.
 * 
 ******************************************************************************/
static size_t
DetermineNrSharedOffsetPartitions (node *upper_bound_scalar,
                                   node *divisor_scalar,
                                   bool is_const_upper_bound,
                                   bool is_const_divisor,
                                   bool is_const_offset,
                                   int lower_bound, int upper_bound,
                                   int divisor, int offset)
{
    int nr_required_partitions;

    node *negative_divisor;

    DBUG_ENTER ();

    if (!global.optimize.dosop) { // Exit if SOP is disabled.
         DBUG_RETURN (0); 
    }

    if (is_const_divisor && divisor == 0) {
        DBUG_PRINT ("Shared offset partitioning case C(x = 0)");
        // For zero divisors, we only need one partition: aplmod(a, 0) = a
        // Structural constant folding will normally take care of this, but
        // that doesn't mean we shouldn't be able to handle it
        nr_required_partitions = 1;
    } else if (is_const_upper_bound && is_const_divisor && is_const_offset) {
        DBUG_PRINT ("Shared offset partitioning case C(everything)");
        // Read the formula as at least one partition, plus the difference
        // between the quotients of the upper and lower bound.
        nr_required_partitions = 1 + abs (
              MATHfloorDivide (upper_bound - 1 + offset, divisor)
            - MATHfloorDivide (lower_bound + offset, divisor)
        );
    } else if (TULSisValuesMatch (upper_bound_scalar, divisor_scalar)) {
        // When u = x, we have at least 1 partition, and at most 2.
        // Only if we know offset == 0, we can state that it is 1 partition.
        // Otherwise, we default to 2 partitions.
        if (is_const_offset && offset == 0) {
            DBUG_PRINT ("Shared offset partitioning case C(u = x, c = 0)");
            nr_required_partitions = 1;
        } else {
            DBUG_PRINT ("Shared offset partitioning case C(u = x)");
            nr_required_partitions = 2;
        }
    } else if ((is_const_upper_bound && is_const_divisor
                && upper_bound == -divisor)
               ||
               // Match u = -x in another way: pattern match for x = -u
               (PMmatchFlatF (
                    PMprf (1, PMAisPrf (F_neg_S),
                           1, PMvar (1, PMAgetNode (&negative_divisor), 0)),
                    divisor_scalar)
                && TULSisValuesMatch (upper_bound_scalar, negative_divisor)))
    {
        // When u = -x, we have at least 1 partition, and at most 2.
        // Only if we know offset == 1, we can state that it is 1 partition.
        if (is_const_offset && offset == 1) {
            DBUG_PRINT ("Shared offset partitioning case C(u = -x, c = 1)");
            nr_required_partitions = 1;
        } else if (is_const_offset && offset == 0 && lower_bound >= 1) {
            DBUG_PRINT ("Shared offset partitioning case"
                        " C(u = -x, c = 0, l >= 1)");
            nr_required_partitions = 1;
        } else {
            DBUG_PRINT ("Shared offset partitioning case C(u = -x)");
            nr_required_partitions = 2;
        }
    } else if (is_const_upper_bound && is_const_divisor) {
        DBUG_PRINT ("Shared offset partitioning case C(u, x)");
        // Since we don't know the offset in this case, we cannot determine
        // the quotients. Instead, we assume the worst case: the offset is 
        // always a value that causes an extra partition.
        // This means l + c = x.
        // Substituting occurrences of c with x - l yields the formula
        // 1 + abs(floor((u - 1 - l) / x))
        nr_required_partitions = 1 +
            abs(MATHfloorDivide (upper_bound - 1 - lower_bound,
                                 divisor));
    } else {
        DBUG_PRINT ("Shared offset partitioning: not enough information");
        nr_required_partitions = 0;
    }

    DBUG_ASSERT (nr_required_partitions >= 0, "Expected %d to be non-negative",
                 nr_required_partitions);
    DBUG_RETURN ((size_t) nr_required_partitions);
}

/*******************************************************************************
 * @fn int NodeToUnsafeInt( node *possible_constant)
 * 
 * @brief Returns the integer constant if the node has one.
 *        Otherwise, returns a value that should NEVER be relied upon.
 ******************************************************************************/
static int
NodeToUnsafeInt (node *possible_constant)
{
    int res = 0;

    DBUG_ENTER ();

    if (COisConstant (possible_constant)) {
        res = NODE_TO_INT (possible_constant);
    }

    DBUG_RETURN (res);
}

/*******************************************************************************
 * @fn void DeterminePartitionMethod( node *lower_bound_scalar, 
 *                                    node *upper_bound_scalar,
 *                                    node *step_scalar,
 *                                    info *arg_info)
 * 
 * @brief Based on what information is available at compile-time, this function
 *        determines and sets the partition_method and nr_required_partitions
 *        in the INFO struct.
 * 
 ******************************************************************************/
static void
DeterminePartitionMethod (node *lower_bound_scalar, node *upper_bound_scalar,
                          node *step_scalar, info *arg_info) 
{
    int int_lower_bound;
    int int_upper_bound;
    int int_divisor;
    int int_offset;

    size_t nr_required_partitions_srp;
    size_t nr_required_partitions_sop;
    size_t nr_required_partitions;
    size_t partition_method;
    
    DBUG_ENTER ();

    // TODO: Allow step sizes other than 1 if they are the exact step size that
    //       would be required to allow multiple modulos operating on the same 
    //       dimension to work for the shared result partitioning method
    if (!(step_scalar == NULL || 
          (COisConstant (step_scalar) && NODE_TO_INT (step_scalar) == 1))) {
        DBUG_PRINT ("Abort: Step size is not 1");
        DBUG_RETURN ();
    }

    // Treat the lower bound as 0 if it is unknown since 0 is
    // mathematically the worst case scenario
    int_lower_bound = COisConstant (lower_bound_scalar) 
                      ? NODE_TO_INT (lower_bound_scalar) : 0;

    // Convert nodes to their constant value if they are constant, or an
    // arbitrary value that should NEVER be used if they're not constant.
    int_upper_bound = NodeToUnsafeInt (upper_bound_scalar);
    int_divisor = NodeToUnsafeInt (INFO_DIVISOR_AVIS (arg_info));
    int_offset  = NodeToUnsafeInt (INFO_OFFSET_AVIS (arg_info));

    // Figure out the minimum required number of partitions with each method
    nr_required_partitions_srp = DetermineNrSharedResultPartitions (
        COisConstant (lower_bound_scalar), 
        COisConstant (upper_bound_scalar),
        COisConstant (INFO_DIVISOR_AVIS (arg_info)),
        int_divisor);
    nr_required_partitions_sop = DetermineNrSharedOffsetPartitions (
        upper_bound_scalar, INFO_DIVISOR_ID (arg_info),
        COisConstant (upper_bound_scalar),
        COisConstant (INFO_DIVISOR_AVIS (arg_info)),
        COisConstant (INFO_OFFSET_AVIS (arg_info)),
        int_lower_bound, int_upper_bound, int_divisor, int_offset);

    if (nr_required_partitions_sop == 0 && nr_required_partitions_srp == 0) {
        DBUG_PRINT ("Not enough information to apply WLMP!");
        DBUG_RETURN ();
    }

    // Determine the best method to use, throw away 0 values
    if (nr_required_partitions_sop != 0 &&
        (nr_required_partitions_srp == 0
        || nr_required_partitions_sop <= nr_required_partitions_srp)) 
    {
        nr_required_partitions = nr_required_partitions_sop;
        partition_method = WLMP_shared_offset;
    } else {
        nr_required_partitions = nr_required_partitions_srp;
        partition_method = WLMP_shared_result;
    }
    
    if (nr_required_partitions > global.maxwlmp) {
        DBUG_PRINT ("Partitioning requires %zu partitions but only %zu"
                    " partitions are allowed!",
                    nr_required_partitions,
                    global.maxwlmp);
        DBUG_RETURN ();
    }
    
    // Passed all checks, commit to the values found
    INFO_PARTITION_METHOD (arg_info) = partition_method;
    INFO_NR_REQUIRED_PARTITIONS (arg_info) = nr_required_partitions;
    DBUG_PRINT ("Partition method: shared %s partitioning", 
                INFO_PARTITION_METHOD (arg_info) == WLMP_shared_offset
                ? "offset" : "result");
    DBUG_PRINT ("Number of partitions: %zu", 
                INFO_NR_REQUIRED_PARTITIONS (arg_info));
    DBUG_RETURN ();
}

/*******************************************************************************
 * @fn node *FindWithLoopIndexDimension( node **id, info *arg_info)
 * 
 * @brief:  Determines whether an N_id node is defined in the closest with-loop 
 *          index. In doing so, it chases the N_id node redefinitions
 *          such as B = A; C = B;
 *          Here, C gets pattern matched to A before the with-loop index check 
 *          is made.
 * 
 * NOTE: This omits checking for WITHID_VEC, which is fine because we only
 *       consider _aplmod_SxS_ in this optimization, and therefore never call
 *       this function on a vector.
 *
 * @param id: The N_id node to compare. 
 *            Changed to the chased id if -1 is NOT returned
 * 
 * @return: The first partition dimension in which the chased id matches an
 *          N_ids node from the closest with-loop's with_ids.
 *          -1 if no N_ids node matches id.
 * 
 ******************************************************************************/
static int
FindWithLoopIndexDimension (node **id, info *arg_info)
{
    int partition_dimension;

    node *iv_scalar_candidate;

    DBUG_ENTER ();
    
    DBUG_ASSERT (id != NULL && *id != NULL, "Got a NULL id");
    DBUG_ASSERT (NODE_TYPE (*id) == N_id,
                 "Expected an N_id node but got %s", NODE_TEXT (*id));

    // Try to chase id to the original definition
    // Pattern match failure happens if id is not an arg or index variable
    if (!PMmatchFlatF (PMparam (1, PMAgetNode (&iv_scalar_candidate)), *id)) {
        DBUG_RETURN (-1);
    }
    
    // Check for a matching avis of the iv_scalar_candidate and the with_ids
    partition_dimension = 0;
    for (node *with_ids = INFO_WITH_IDS (arg_info);
         with_ids != NULL;
         with_ids = IDS_NEXT (with_ids), partition_dimension++)
    {
        if (IDS_AVIS (with_ids) == ID_AVIS (iv_scalar_candidate)) {
            *id = iv_scalar_candidate;
            DBUG_RETURN (partition_dimension);
        }
    }

    DBUG_RETURN (-1);
}

/*******************************************************************************
 * @fn bool isLocal( node *id, info *arg_info)
 * 
 * @brief Determines whether an N_id node is defined in the current partition.
 *        With-loop iterators are not counted as local.
 * 
 ******************************************************************************/
bool
isLocal (node *id, info *arg_info)
{
    node *res;

    DBUG_ENTER ();
    // If a node is present, the search returns the match.
    // We use the lut as a set and always put in NULL, so we have an equality
    // check on NULL.
    res = LUTsearchInLutPp (INFO_LOCALS_LUT (arg_info), ID_AVIS (id));

    DBUG_RETURN (res == NULL);
}

/*******************************************************************************
 * @fn bool MatchValidPatterns( node *prf, info *arg_info)
 * 
 * @brief Matches valid patterns for this optimization.
 *
 * Specifically, the following patterns are matched:
 *     _aplmod_SxS_(iv, x)
 *     _aplmod_SxS_(iv + c, x)
 *     _aplmod_SxS_(c + iv, x)
 * where iv is a scalar index vector from the immediately surrounding with-loop,
 *       c is a with-loop invariant offset that is added to iv
 *       x is the divisor of the modulo operation
 * 
 * When a pattern matches, discovered information is stored in
 * INFO_PARTITION_DIMENSION, INFO_IV_SCALAR_AVIS, INFO_OFFSET_AVIS, and 
 * INFO_DIVISOR_AVIS.
 *
 * Approach:
 * We start by checking some quick outs:
 * • Is the given prf an _aplmod_SxS_ expression?
 * • Is the divisor with-loop invariant?
 * Afterwards, there are two scenarios based on the first argument of the
 * aplmod, called the dividend, the dividend.
 * • Scenario 1: The dividend is an id
 *   -> We chase the id to its original definition and find for which dimension,
 *      if any, it matches with the scalar index vector.
 *   -> We initialize an offset to 0
 *   -> The match succeeds and we return the discovered information
 * • Scenario 2: The dividend is an addition of dividend1 and dividend2
 *   -> We chase both dividends to their original definition and find for which
 *      dimension, if any, they match with the scalar index vector.
 *   -> We only proceed if exactly one dividend matches.
 *   -> The other dividend is the offset
 *   -> The match succeeds and we return the discovered information
 *
 * @param prf - The prf node on which the pattern matching is applied
 * @param arg_info - The INFO struct
 *
 * @return: Whether pattern matching succeeded. 
 * 
 ******************************************************************************/
static bool
MatchValidPatterns (node *prf, info *arg_info)
{
    int found_dimension1;
    int found_dimension2;

    node *dividend;
    node *divisor;
    node *dividend1;
    node *dividend2;

    DBUG_ENTER ();
    DBUG_ASSERT (!INFO_FOUND_SPLITCAUSE (arg_info),
                 "This function shouldn't be called when the splitcause is"
                 " found! Executing this function would override information "
                 " that still has to be used!");

    if (!PMmatchFlatF (PMprf (1, PMAisPrf (F_aplmod_SxS),
                              2, PMvar (1, PMAgetNode (&dividend), 0),
                                 PMvar (1, PMAgetNode (&divisor), 0)),
                       prf)) 
    {
        DBUG_PRINT_TAG ("WLMP_PRIMITIVE",
                        "Abort pattern match: not an _aplmod_SxS_ primitive");
        DBUG_RETURN (false);
    }
    
    // With-loop invariant removal runs directly before this optimization,
    // so local means not with-loop invariant. We cannot deal with that case.
    if (isLocal (divisor, arg_info)) {
        DBUG_PRINT ("Abort pattern match: got local divisors");
        DBUG_RETURN (false);
    }
    INFO_DIVISOR_ID (arg_info) = divisor;
    INFO_DIVISOR_AVIS (arg_info) = ID_AVIS (divisor); 

    // Test for scenario 1:
    // Dividend is the iv_scalar, and there is no offset.
    // Approach:
    // • We check whether dividend is an iv_scalar
    found_dimension1 = FindWithLoopIndexDimension (&dividend, arg_info);
    if (found_dimension1 != -1) {
        INFO_PARTITION_DIMENSION (arg_info) = (size_t) found_dimension1;
        INFO_IV_SCALAR_AVIS (arg_info) = ID_AVIS (dividend);

        // Create an offset set to 0 so we can reuse code for addition
        INFO_OFFSET_AVIS (arg_info) = MAKE_NUM_2AVIS (TBmakeNum (0), arg_info);

        DBUG_PRINT ("Matched modulo scenario 1: _aplmod_SxS_ (%s, %s)",
                    ID_NAME (dividend), ID_NAME (divisor));
        DBUG_RETURN (true);
    }

    // Test for scenario 2:
    // Dividend is an addition.
    // Approach:
    // • Check whether dividend is an addition & retrieve the arguments
    //   • Ensure neither arguments are local
    //   • Ensure exactly one of them is an iv_scalar
    //   • The other is the offset
    if (PMmatchFlatF (PMprf (1, PMAisPrf (F_add_SxS),
                             2, PMvar (1, PMAgetNode (&dividend1), 0),
                                PMvar (1, PMAgetNode (&dividend2), 0)),
                      dividend))
    {
        // For soundness, none of the matched arguments may be locally defined.
        // Because with-loop invariant removal runs directly before this
        // optimization, not local means with-loop invariant.
        if (isLocal (dividend1, arg_info) || isLocal (dividend2, arg_info))
        {
            DBUG_PRINT ("Abort prf: got local dividends");
            DBUG_RETURN (false);
        }

        // Exactly one of the matched arguments should be an index scalar
        // in the immediately surrounding with-loop
        found_dimension1 = FindWithLoopIndexDimension (&dividend1, arg_info);
        found_dimension2 = FindWithLoopIndexDimension (&dividend2, arg_info);
        if (!((found_dimension1 == -1) ^ (found_dimension2 == -1))) {
            DBUG_PRINT ("Abort prf: did not get exactly one with loop index");
            DBUG_RETURN (false);
        }

        INFO_PARTITION_DIMENSION (arg_info) = found_dimension1 != -1 
            ? (size_t) found_dimension1 
            : (size_t) found_dimension2;
        INFO_IV_SCALAR_AVIS (arg_info) = ID_AVIS (found_dimension1 != -1 
            ? dividend1 : dividend2);
        INFO_OFFSET_AVIS (arg_info) = ID_AVIS (found_dimension1 == -1 
            ? dividend1 : dividend2);

        DBUG_PRINT ("Matched modulo scenario 2: _aplmod_SxS_ (%s: %s + %s, %s)",
                    ID_NAME (dividend), ID_NAME (dividend1),
                    ID_NAME (dividend2), ID_NAME (divisor));
        DBUG_RETURN (true);
    }

    DBUG_RETURN (false);
}

/*******************************************************************************
 * Main traversal functions
 *
 * The main traversal is responsible for: 
 * - Discovering aplmods that can be split up
 * - Replacing aplmods if they only need 1 partition 
 * - Splitting partitions if more than one partition is required
 *   - In this case, it will also call the anonymous traversal on each partition
 *     to modify it
 ******************************************************************************/

/*******************************************************************************
 * @fn node *WLMPfundef( node *arg_node, info *arg_info)
 * 
 * @brief Adds itself to the INFO structure and handles VARDECS before
 *        traversing the body.
 *        This is done in a stacked manner since the state must remain consistent
 *        after entering and leaving a lac-fun.
 * 
 ******************************************************************************/
node *
WLMPfundef (node *arg_node, info *arg_info)
{
    size_t new_partitions_count;

    DBUG_ENTER ();

    // Enter local functions (lacfuns) if we're inside a normal function, and
    // enter subsequent lacfuns if we're already inside a lacfun.
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_PRINT ("Entered function %s.", FUNDEF_NAME (arg_node));

    if (FUNDEF_BODY (arg_node) == NULL) {
        // NOTE: Don't be smart and remove this check to use TRAVopt instead.
        //       Functions without a body crash when accessing the vardec.
        DBUG_PRINT ("Left function %s since it has no body.",
                    FUNDEF_NAME (arg_node));
        DBUG_RETURN (arg_node);
    }

    // Set up info-specific information
    INFO_FUNDEF (arg_info) = arg_node;
    INFO_VARDECS (arg_info) = NULL;

    // Do the actual traversal and store the vardecs
    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_VARDECS (arg_node) = TCappendVardec (FUNDEF_VARDECS (arg_node),
                                                INFO_VARDECS (arg_info));
    
    new_partitions_count = global.optcounters.wlmp_sop_partitions_created
                         + global.optcounters.wlmp_srp_partitions_created;
    if (new_partitions_count > 32 && !global.suppress_wlmp_code_size) {
        CTIwarn(EMPTY_LOC,
                "Code size is increased by a factor of up to %zu in function "
                "\"%s\"\n"
                "If compilation takes too long, consider using the commandline "
                "option -maxwlmp 1 at the cost of runtime performance.\n"
                "Suppress this warning with -suppress_wlmp_code_size.",
                new_partitions_count, FUNDEF_NAME (arg_node));
    }
    DBUG_PRINT ("Left function %s.", FUNDEF_NAME (arg_node));
    DBUG_RETURN (arg_node);
}

/*******************************************************************************
 * @fn node *WLMPassign( node *arg_node, info *arg_info)
 * 
 * This function has different traversal behaviours based on the inner assign 
 * nodes.
 * For all nodes, they are added to the locals_lut to keep track of which
 * variables are locals, and inversely, which are loop-invariant.
 * With-loop nodes are traversed top-down and have their preassigns set.
 * Other nodes are traversed bottom-up and do not set preassigns.
 *
 * This behaviour ensures that our locals_lut is remains sound, while allowing
 * the innermost with-loops to always be fully traversed before outer with-loops
 * might copy them. This avoids duplicate work that could otherwise occur when a
 * partition that is copied contains an untraversed with-loop.
 * 
 ******************************************************************************/
node *
WLMPassign (node *arg_node, info *arg_info)
{
    node *old_preassigns;
    node *res_node;

    DBUG_ENTER ();

    // If we are inside of a with-loop we add this node's avis to the lut.
    // We do this top-down because we need to know which variables were created
    // inside of the with-loop before the aplmod computation. 
    if (INFO_IS_INSIDE_WITH_LOOP (arg_info)) {
        LUTupdateLutP(INFO_LOCALS_LUT (arg_info), 
                      IDS_AVIS (ASSIGN_LHS (arg_node)), NULL, NULL);
    }

    // For with-loop nodes, traverse top-down and set the preassigns.
    if (ASSIGN_DIRECTLY_CONTAINS_WITH_LOOP (arg_node)) {
        old_preassigns = INFO_PREASSIGNS (arg_info);
        INFO_PREASSIGNS (arg_info) = NULL;

        DBUG_PRINT ("Entering with-loop assigned to %s.", ASSIGN_NAME (arg_node));
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

        res_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = old_preassigns;

        DBUG_PRINT ("Leaving with-loop assigned to %s.", ASSIGN_NAME (arg_node));

        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_RETURN (res_node);
    } 

    // For all other nodes, we do a bottom-up traversal
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    // Only traverse into this assignment if we haven't found a reason
    // to split partitions yet.
    if (!INFO_FOUND_SPLITCAUSE (arg_info)) {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*******************************************************************************
 * @fn node *WLMPwith( node *arg_node, info *arg_info)
 * 
 * Stacks with_ids and the locals lut.
 * Traverses into N_part only.
 * TODO: Prevent entering with-loops that have already been optimized.
 * 
 ******************************************************************************/
node *
WLMPwith (node *arg_node, info *arg_info)
{
    stackables *old_with_loop_specific_information;

    DBUG_ENTER ();
    DBUG_PRINT ("Entered with-loop.");
    
    // Stack everything with-loop specific
    old_with_loop_specific_information = INFO_STACKABLES (arg_info);
    INFO_STACKABLES (arg_info) = MakeStackables ();

    // Add the with_ids of this with-loop to the new stackables in info
    INFO_WITH_IDS (arg_info) = WITH_IDS (arg_node);
    
    // Actual traversal
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    // Restore the original with-loop specific information

    INFO_STACKABLES (arg_info) = FreeStackables (&INFO_STACKABLES (arg_info));
    INFO_STACKABLES (arg_info) = old_with_loop_specific_information;

        DBUG_PRINT ("Left with-loop.");
    DBUG_RETURN (arg_node);
}

/*******************************************************************************
 * @fn node *WLMPpart( node *arg_node, info *arg_info)
 * 
 * Repeatedly traverses into this partition to determine whether it can be 
 * split up into multiple partitions to replace aplmod with addition.
 * 
 * If it can't be split up, it traverses into the next partition.
 * 
 * Otherwise, it creates and inserts the required number of partitions.
 * Afterwards, it launches the anonymous traversal into these partitions to 
 * modify their bounds and the prf node that caused the split. 
 * It then re-traverses into this partition to determine whether it can be
 * split up again.
 * 
 ******************************************************************************/
node *
WLMPpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER();

    // We can't make any changes without with_ids, but we can traverse to
    // look for nested with-loops that we can deal with.
    if (INFO_WITH_IDS (arg_info) == NULL) {
        PART_CBLOCK (arg_node) = TRAVdo (PART_CBLOCK (arg_node), arg_info);
        PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
        DBUG_RETURN (arg_node);
    }

    // Recursively apply SOP/SRP until it's no longer possible
    arg_node = EvaluatePartition(arg_node, arg_info);

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*******************************************************************************
 * @fn node *WLMPprf( node *arg_node, info *arg_info)
 * 
 * Pattern matches for expressions of the forms:
 * _aplmod_SxS_(iv + c, x)
 * _aplmod_SxS_(c + iv, x)
 * 
 * Assuming a pattern matched where iv matches an iterator scalar and c is
 * a with-loop invariant, this function determines the 
 * required number of partition splits and partitioning method.
 * This information is added to the INFO structure.
 * 
 ******************************************************************************/
node *
WLMPprf (node *arg_node, info *arg_info)
{
    node *lower_bound_scalar;
    node *upper_bound_scalar;
    node *step_scalar;
    
    DBUG_ENTER();

    DBUG_ASSERT (!INFO_FOUND_SPLITCAUSE (arg_info), 
                 "WLMPassign should have prevented us from entering WLMPprf"
                 " after the splitcause has already been found!");

    if (!INFO_IS_INSIDE_WITH_LOOP (arg_info)) {
        DBUG_PRINT ("Abort prf: not inside a with-loop");
        DBUG_RETURN (arg_node);
    }

    // Abort the traversal if we have no scalar with-ids since we can only
    // handle loops with known dimensions.
    // NOTE: We don't abort at the level of the with-loop because there might be
    //       nested with-loops that we CAN deal with.
    if (INFO_WITH_IDS (arg_info) == NULL) {
        DBUG_PRINT ("Abort prf: no scalar with-ids");
        DBUG_RETURN (arg_node);
    }

    if (!MatchValidPatterns (arg_node, arg_info)) {
        DBUG_RETURN (arg_node);
    }
    // The match added partition_dimension, iv_scalar, offset, and divisor
    // information to the INFO struct.

    // Using the found partition dimension, get the correct generator bounds
    lower_bound_scalar = TCgetNthExprsExpr (
        INFO_PARTITION_DIMENSION (arg_info),
        ARRAY_AELEMS (INFO_LOWER_BOUND_ARRAY (arg_info)));
    upper_bound_scalar = TCgetNthExprsExpr (
        INFO_PARTITION_DIMENSION (arg_info),
        ARRAY_AELEMS (INFO_UPPER_BOUND_ARRAY (arg_info)));
    step_scalar = TCgetNthExprsExprOrNull (
        INFO_PARTITION_DIMENSION (arg_info), 
        INFO_STEP_ARRAY (arg_info) == NULL 
            ? NULL : ARRAY_AELEMS (INFO_STEP_ARRAY (arg_info)));

    // Determine the partitioning method and maximum number of partitions
    // If the maximum number of partitions is less than the global maximum,
    // add the information to the INFO structure so WLMPpart can annotate
    // the information.
    DeterminePartitionMethod(lower_bound_scalar, upper_bound_scalar,
                             step_scalar, arg_info);

    if (INFO_FOUND_SPLITCAUSE (arg_info)) {
        PRF_WLMP_SPLITCAUSE (arg_node) = true;

        // At this point we know the optimization will apply, so we increment
        // the respective optimization counter.
        switch (INFO_PARTITION_METHOD (arg_info)) {
        case WLMP_shared_offset:
            global.optcounters.wlmp_sop_mod_removed++;
            break;
        case WLMP_shared_result:
            global.optcounters.wlmp_srp_mod_removed++;
            break;
        default:
            DBUG_ASSERT (false, "Unreachable code.");
        }

        // The divisor ID is freed as part of the replacement, so now that
        // the discovery part is done we set it to NULL to prevent the pointer
        // from going stale.
        // The discovered AVISes are not freed and are therefore kept.
        INFO_DIVISOR_ID (arg_info) = NULL;
        
        // For constructing the upper bound in shared offset partitioning and
        // the step in the shared result partitioning scenario, we need an
        // absolute version of the divisor.
        INFO_ABS_DIVISOR_AVIS (arg_info) = MAKE_PRF1_2AVIS (
            F_abs_S, 
            INFO_DIVISOR_AVIS (arg_info), arg_info);

        // For the bounds, we create an avis of the num so we can always work
        // with an avis when creating expressions for the new bounds/prf
        INFO_LOWER_BOUND_SCALAR_AVIS (arg_info) =
            NODE_TYPE (lower_bound_scalar) == N_id 
            ? ID_AVIS (lower_bound_scalar) 
            : MAKE_NUM_2AVIS (DUPdoDupNode (lower_bound_scalar), arg_info);
        INFO_UPPER_BOUND_SCALAR_AVIS (arg_info) =
            NODE_TYPE (upper_bound_scalar) == N_id 
            ? ID_AVIS (upper_bound_scalar) 
            : MAKE_NUM_2AVIS (DUPdoDupNode (upper_bound_scalar), arg_info);

        // If this is the only partition that uses this code and we only need
        // one partition, we can directly replace the aplmod with a cheaper
        // expression without the need to traverse back up to the partitions
        // and duplicate them.
        if (INFO_IS_UNIQUE_PARTITION_CODE (arg_info)
                && INFO_NR_REQUIRED_PARTITIONS (arg_info) == 1) {
            DBUG_PRINT ("Shortcutting the replacement");
            arg_node = ReplaceAplmod (arg_node, arg_info);
            INFO_PARTITION_METHOD (arg_info) = WLMP_noop;
        }
        // Otherwise, we implicitly go back up to to WLMPpart to create
        // additional partitions before we launch an anonymous traversal
        // to change this aplmod in each partition
    } else {
        DBUG_PRINT ("Determined no partition method can be applied");
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
