/*
 * $Log$
 * Revision 1.1  2000/07/20 11:32:05  nmw
 * Initial revision
 *
 *
 */

#ifndef _sac_free_interface_handler_h
#define _sac_free_interface_handler_h

extern void SAC_FIH_AddFreeFunction (void (*freefun) ());
extern void SAC_FIH_FreeAllModules ();

#endif
