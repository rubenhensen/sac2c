/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

#ifndef _SAC__CONSTANTFOLDING_H_
#define _SAC__CONSTANTFOLDING_H_

struct INFO {
    size_t numidssofar;
    ntype *lhstype;
    node *fundef;
    node *preassign;
    node *postassign;
    node *vardecs;
    node *topblock;
    node *avismin;
    node *avismax;
    node *let;
    node *part;
    node *proxyarr;
    bool remassign;
    bool doingextrema;
};

#define INFO_NUM_IDS_SOFAR(n) (n->numidssofar)

#define INFO_LHSTYPE(n) (n->lhstype)
#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_POSTASSIGN(n) (n->postassign)

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_TOPBLOCK(n) (n->topblock)
#define INFO_AVISMIN(n) (n->avismin)
#define INFO_AVISMAX(n) (n->avismax)
#define INFO_LET(n) (n->let)
#define INFO_PART(n) (n->part)
#define INFO_PROXYARR(n) (n->proxyarr)

#define INFO_DOINGEXTREMA(n) (n->doingextrema)

#endif //  _SAC__CONSTANTFOLDING_H_
