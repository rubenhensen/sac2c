#ifndef _JUPYTER_H_
#define _JUPYTER_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int jupyter_init (void);
extern char * jupyter_parse_from_string (char *s);
extern void jupyter_free (void *p);
extern int jupyter_finalize (void);

#ifdef __cplusplus
}
#endif

#endif /* _JUPYTER_H_ */
