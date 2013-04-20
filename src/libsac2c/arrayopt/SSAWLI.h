#ifndef _SAC_WLI_H_
#define _SAC_WLI_H_

#include "types.h"

extern node *WLIfundef (node *, info *);
extern node *WLIid (node *, info *);
extern node *WLIassign (node *, info *);
extern node *WLIcond (node *, info *);
extern node *WLIwith (node *, info *);
extern node *WLIlet (node *, info *);

extern node *WLImodarray (node *, info *);
extern node *WLIpart (node *, info *);
extern node *WLIgenerator (node *, info *);
extern node *WLIcode (node *, info *);

extern node *WLIdoWLI (node *arg_node);

#endif /* _SAC_SSAWLI_H_ */
