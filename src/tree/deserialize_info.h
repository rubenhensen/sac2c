/*
 *
 * $Log$
 * Revision 1.2  2004/10/28 17:20:46  sah
 * now deserialize as an internal state
 *
 * Revision 1.1  2004/10/25 16:07:32  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _DESERIALIZE_INFO_H
#define _DESERIALIZE_INFO_H
/*
 * INFO structure
 */
struct INFO {
    node *ret;
    node *ssacounter;
    node *module;
    node *fundefs;
    node *vardecs;
    node *args;
};

/*
 * INFO macros
 */
#define INFO_DS_RETURN(n) (n->ret)
#define INFO_DS_SSACOUNTER(n) (n->ssacounter)
#define INFO_DS_MODULE(n) (n->module)
#define INFO_DS_FUNDEFS(n) (n->fundefs)
#define INFO_DS_VARDECS(n) (n->vardecs)
#define INFO_DS_ARGS(n) (n->args)

#endif /* _DESERIALIZE_INFO_H */
