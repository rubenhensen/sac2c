/*
 *
 * $Log$
 * Revision 1.8  1996/09/11 06:29:43  cg
 * Added some new global variables for new command line options.
 *
 * Revision 1.7  1996/08/09  16:44:12  asi
 * dead function removal added
 *
 * Revision 1.6  1996/05/29  14:18:57  sbs
 * inserted noRCO opt_rco!
 *
 * Revision 1.5  1996/01/17  16:49:21  asi
 * added common subexpression elimination
 *
 * Revision 1.4  1996/01/16  16:42:22  cg
 * added global variable int check_malloc
 *
 * Revision 1.3  1996/01/05  12:25:18  cg
 * added cleanup and targetdir[]
 *
 * Revision 1.2  1996/01/02  15:45:44  cg
 * include of filemgr.h no longer necessary
 *
 * Revision 1.1  1995/12/30  14:47:06  cg
 * Initial revision
 *
 *
 *
 *
 */

#ifndef _sac_globals_h

#define _sac_globals_h

/*
 *  This header file contains external declarations for all global
 *  variables which are defined and initialized in main.c
 */

extern FILE *outfile;

extern char sacfilename[];
extern char prgname[];
extern char outfilename[];
extern char modulename[];
extern char cfilename[];
extern char ccflagsstr[];
extern char targetdir[];
extern char commandline[];

extern int Ccodeonly;
extern int break_compilation;

extern int optimize;
extern int sac_optimize;
extern int opt_dcr;
extern int opt_dfr;
extern int opt_cf;
extern int opt_wr;
extern int opt_lir;
extern int opt_inl;
extern int opt_unr;
extern int opt_uns;
extern int opt_ae;
extern int opt_cse;

extern int optvar;
extern int inlnum;
extern int unrnum;
extern int minarray;

extern int max_overload;
extern int max_optcycles;
extern int psi_optimize;
extern int psi_opt_ive;

extern int opt_rco;
extern int show_refcnt;
extern int show_idx;
extern int show_icm;
extern int traceflag;

extern int breakae;

extern int check_boundary;
extern int cleanup;
extern int check_malloc;

extern int useranlib;
extern int libstat;
extern int linkstyle;

extern int print_objdef_for_header_file;
extern int function_counter;

#endif /* _sac_globals_h */
