#ifndef _SAC_MUTC_GEN_SUPPORT_H_

#define _SAC_MUTC_GEN_SUPPORT_H_

#define SAC_MUTC_ND_ARG_INT_GLO(nt, type) sl_glarg (type, sl_anon, NT_NAME (nt))
#define SAC_MUTC_ND_ARG_FLO_GLO(nt, type) sl_shfarg (type, sl_anon, NT_NAME (nt))
#define SAC_MUTC_ND_ARG_INT_SHA(nt, type) sl_glarg (type, sl_anon, NT_NAME (nt))
#define SAC_MUTC_ND_ARG_FLO_SHA(nt, type) sl_shfarg (type, sl_anon, NT_NAME (nt))

#define SAC_MUTC_ND_PARAM_INT_GLO(nt, type) sl_glparam (type, sl_anon, NT_NAME (nt))
#define SAC_MUTC_ND_PARAM_FLO_GLO(nt, type) sl_shfparam (type, sl_anon, NT_NAME (nt))
#define SAC_MUTC_ND_PARAM_INT_SHA(nt, type) sl_glparam (type, sl_anon, NT_NAME (nt))
#define SAC_MUTC_ND_PARAM_FLO_SHA(nt, type) sl_shfparam (type, sl_anon, NT_NAME (nt))

#define SAC_MUTC_ND_GET_VAR_PAR(nt) sl_getp (NT_NAME (nt))
#define SAC_MUTC_ND_GET_VAR_DEF(nt) NT_NAME (nt)
#endif /* _SAC_MUTC_GEN_SUPPORT_H_ */
