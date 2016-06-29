
SET (CFLAGS_MT              -D_REENTRANT)

SET (SETTINGS_MT            -DMT)
SET (SETTINGS_PTH           -DPTH)
SET (SETTINGS_LPEL          -DLPEL)
SET (SETTINGS_OMP           -DOMP)
SET (SETTINGS_DIAG          -DDIAG)
SET (SETTINGS_XT            -DMT -DPHM_XT)

SET (LIB_M                  -Xl -lm)
# FIXME(artem) we pass this to the library building via external project
#              as this variable is configuration-dependent.  The same should
#              happen with "-Xl -lm" as well.
#SET (LIB_RT                 -Xl -lrt)

# BUILD TARGETS
SET (TARGETS                seq mt_pth mt_pth_xt)

