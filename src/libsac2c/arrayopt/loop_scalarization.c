/*
 *
 * $Id$
 */

/**
 *
 * @file loop_scalarization.c
 *
 * Loop scalarization (LS) is the successor of array elimination (AE).
 * It is based on the observation that most simple cases of AE are covered
 * by the other optimizations, most notably Constant Folding.
 * The only case left is the case where small arrays are passed through
 * and modified by loop functions, i.e., we have a situation of the form:
 *
 * <pre>
 *   ...
 *   x = foo();
 *   ...
 *   do {
 *         x = [ op_0( x[[0]]), ..., op_n( x[[n]])];
 *      }
 *
 *   ... x ...
 *
 * </pre>
 *
 * Since x is not used as a vector apart from elementary selections on it,
 * we strive to avoid the repeated creation of intermediate vectors that
 * are taken apart within the next iteration.
 * The overall goal is to achieve a code of the form:
 *
 * <pre>
 *   ...
 *   x = foo();
 *   x_0 = x[[0]];
 *   ...
 *   x_n = x[[n]];
 *   ...
 *   do{
 *       x_0 = op_0( x_0);
 *         ...
 *       x_n = op_n( x_n);
 *     }
 *   x = [x_0, ..., x_n];
 *   ... x ...
 *
 * </pre>
 *
 * Since we generally do not want to modify the entire loop body,
 * we apply a simpler transformation and rely on other optimizations
 * to achieve our final goal.
 * More precisely, we generate the following code:
 *
 * <pre>
 *   ...
 *   x = foo();
 *   x_0 = x[[0]];        /* new! */
*... /* new! */
  *x_n
  = x[[n]]; /* new! */
*... *do
{
    *x = [ x_0, ..., x_n ]; /* new! */
    **x = [ op_0 (x[[0]]), ..., op_n (x[[n]]) ];
    **x_0 = x[[0]]; /* new! */
    *...            /* new! */
      *x_n
      = x[[n]]; /* new! */
    *
}
*... x... **</ pre> **This transformation suffices iff Constant Folding (CF)
  and *Loop Invariant Removal (LIR) are turned on.*CF transformes the loop above into
    : **<pre> *... *x = foo ();
*x_0 = x[[0]];
*... *x_n = x[[n]];
*... *do
{
    *tmp_0 = op_0 (x_0);
    *... *tmp_n = op_n (x_n);
    *x = [ tmp_0, .., tmp_n ];
    *x_0 = tmp_0;
    *... *x_n = tmp_n;
    *
}
*... x... **</ pre> **Now,
  x is (*hope *) moved behind the loop by LIR as it is not referenced
    * within the loop anymore.This leads to : **<pre> * ... * x = foo ();
*x_0 = x[[0]];
*... *x_n = x[[n]];
*... *do
{
    *tmp_0 = op_0 (x_0);
    *... *tmp_n = op_n (x_n);
    *x_0 = tmp_0;
    *... *x_n = tmp_n;
    *
}
*x = [ tmp_0, .., tmp_n ];
 *   ... x ...
 *
 * </pre>
 *
 * which is equivalent to our goal!
 *
 *
 * Some remarks on the extent of the optimization:
 * -----------------------------------------------
 *
 * So far, we apply the LS iff x is used as array argument in selections 
 * within the loop body only. If it is used in any other argument
 * position LS is NOT applied!
 * One may wonder, whether this transformation could be extended
 * for slightly more general situations. For example, we may consider
 * a situation where x occurs as an argument of F_add_SxA_, such as:
 *
 * <pre>
 *   ...
 *   x = [2,3];
 *... *do
 {
     *x = _add_SxA_ (1, x);
     *
 }
 **... x... **</ pre> **In order to avoid the intermediate vectors x,
   we would need *to scalarize F_add_SxA_.Although this could be done,
   it seems *to us that *(a)this is a rare case *(b)this prf scalarization could be
   handled in a more general setting *which would then enable LS as described above
     .**Another extension that could come to our mind would be to allow *occurrences of x
   in index vector positions since index vectors will *be eliminated by Index Vector
   Elimination (IVE) anyways.*However,
   this - in general - may leed to problems.*For example,
   consider the following scenario : **<pre> *... *i = [ 2, 3 ];
 *... *do
 {
     *i = _add_SxA_ (1, i);
     *a[7, 3] = a[i];
     *
 }
 **... i, a... **</ pre> **If we would apply an extended version of LS here,
   we would *obtain (after the cycle) : **<pre> * ... * i = [ 2, 3 ];
 *i_0 = 2;
 *i_1 = 3;
 *... *do
 {
     *i_0 = i_0 + 1;
     *i_1 = i_1 + 1;
     *i = [ i_0, i_1 ];
     *a[7, 3] = a[i];
     *
 }
 **... i, a... **</ pre> **Since the increment of i has been scalarized,
   IVE cannot transform *i into a single scalar index anymore.As a net result,
   we will *obtain a superfluous multiplication and a superfluous addition.*Note here,
   that the drawback increases as the length of i increases.* /

     /** <!--*******************************************************************-->
      *
      * @fn node *LSfundef( node *arg_node, info *arg_info)
      *
      *****************************************************************************/
     node *
     LSfundef (node *arg_node, info *arg_info)
 {

     DBUG_ENTER ("LSfundef");
     DBUG_RETURN (arg_node);
 }

 /** <!--*******************************************************************-->
  *
  * @fn node *LSap( node *arg_node, info *arg_info)
  *
  *****************************************************************************/
 node *
 LSap (node *arg_node, info *arg_info)
 {

     DBUG_ENTER ("LSap");
     DBUG_RETURN (arg_node);
 }

 /** <!--*******************************************************************-->
  *
  * @fn node *LSprf( node *arg_node, info *arg_info)
  *
  *****************************************************************************/
 node *
 LSprf (node *arg_node, info *arg_info)
 {

     DBUG_ENTER ("LSprf");
     DBUG_RETURN (arg_node);
 }

 /** <!--*******************************************************************-->
  *
  * @fn node *LSid( node *arg_node, info *arg_info)
  *
  *****************************************************************************/
 node *
 LSid (node *arg_node, info *arg_info)
 {

     DBUG_ENTER ("LSid");
     DBUG_RETURN (arg_node);
 }

 /** <!--********************************************************************-->
  *
  * @fn  node *LSdoLoopScalarization( node *syntax_tree)
  *
  *   @brief call this function for eliminating arrays within loops
  *   @param part of the AST (usually the entire tree) IVE is to be applied on.
  *   @return modified AST.
  *
  *****************************************************************************/
 node *
 LSdoLoopScalarization (node *syntax_tree)
 {
     info *info;

     DBUG_ENTER ("LSdoLoopScalarization");

     DBUG_PRINT ("OPT", ("Starting Loop Scalarization ...."));

     TRAVpush (TR_ls);

     info = MakeInfo ();
     syntax_tree = TRAVdo (syntax_tree, info);

     info = FreeInfo (info);
     TRAVpop ();

     DBUG_PRINT ("OPT", ("Loop Scalarization done!"));

     DBUG_RETURN (syntax_tree);
 }
