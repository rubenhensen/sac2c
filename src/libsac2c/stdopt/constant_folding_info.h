/*
 * $Id constant_folding_info.h rbe $
 */

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool remassign;
    bool onefundef;
    node *fundef;
    node *preassign;
    node *postassign;
    node *vardecs;
};

#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_ONEFUNDEF(n) (n->onefundef)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_POSTASSIGN(n) (n->postassign)
#define INFO_VARDECS(n) (n->vardecs)
