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
    node *fundef;
    node *preassign;
    node *postassign;
    node *vardecs;
    node *topblock;
    bool remassign;
    bool onefundef;
    bool lacfunok;
    bool travinlac;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_POSTASSIGN(n) (n->postassign)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_TOPBLOCK(n) (n->topblock)
#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_ONEFUNDEF(n) (n->onefundef)
#define INFO_LACFUNOK(n) (n->lacfunok)
#define INFO_TRAVINLAC(n) (n->travinlac)
