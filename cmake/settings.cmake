
SET (CFLAGS_MT              -D_REENTRANT)

SET (SETTINGS_MT            -DMT)
SET (SETTINGS_PTH           -DPTH)
SET (SETTINGS_LPEL          -DLPEL)
SET (SETTINGS_OMP           -DOMP)
SET (SETTINGS_DIAG          -DDIAG)
SET (SETTINGS_XT            -DMT -DPHM_XT)

SET (LIB_M                  -Xl -lm)
SET (LIB_RT                 -Xl -lrt)

# BUILD TARGETS
SET (TARGETS                seq mt_pth mt_pth_xt)

