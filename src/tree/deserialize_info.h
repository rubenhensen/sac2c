/*
 *
 * $Log$
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
    node *ast;
};

/*
 * INFO macros
 */
#define INFO_DS_RETURN(n) (n->ret)
#define INFO_DS_SSACOUNTER(n) (n->ssacounter)
#define INFO_DS_AST(n) (n->ast)

#endif /* _DESERIALIZE_INFO_H */
