/*
 * $Log$
 * Revision 1.1  2004/07/03 15:12:39  sah
 * Initial revision
 *
 *
 */

#ifndef _sac_node_compat_h
#define _sac_node_compat_h

/*****************************************************************************
 * N_Id:
 */

#define ID_NAME(n) (IDS_NAME (ID_IDS (n)))
#define ID_AVIS(n) (IDS_AVIS (ID_IDS (n)))
#define ID_VARDEC(n) (IDS_VARDEC (ID_IDS (n)))
#define ID_OBJDEF(n) (IDS_VARDEC (ID_IDS (n)))
#define ID_MOD(n) (IDS_MOD (ID_IDS (n)))
#define ID_STATUS(n) (IDS_STATUS (ID_IDS (n)))
#define ID_DEF(n) (IDS_DEF (ID_IDS (n)))
#define ID_REFCNT(n) (IDS_REFCNT (ID_IDS (n)))
#define ID_NAIVE_REFCNT(n) (IDS_NAIVE_REFCNT (ID_IDS (n)))

#endif
