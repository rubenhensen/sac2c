/*
 * $Log$
 * Revision 1.1  2004/11/21 17:30:35  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   concurrent_info.h
 *
 * description:
 *   extra header file for all concurrent-files to make it compartible with
 *   the new info-structure
 *
 *****************************************************************************/

#ifndef _CONCURRENT_INFO_H

#define _CONCURRENT_INFO_H

/*
 * INFO structure
 */

struct INFO {
    node *fundef;
    int mt;
    int first;
    int last;
};

/*
 * INFO macros
 *   node*      FUNDEF
 *   int        MT
 *   int        FIRST
 *   int        LAST
 */

#define INFO_CONC_FUNDEF(n) (n->fundef)
#define INFO_SPMDL_MT(n) (n->mt)
#define INFO_SYNCI_FIRST(n) (n->first)
#define INFO_SYNCI_LAST(n) (n->last)

#endif /* _CONCURRENT_INFO_H */
