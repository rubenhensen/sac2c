AC_DEFUN([EMPTY_DUMMY_MACRO])

dnl check for Distributed Memory support
AC_DEFUN([CHECK_DISTMEM], dnl
[
   AC_ARG_ENABLE([distmem],
                 [AS_HELP_STRING([--disable-distmem],
                                 [Disable checking for Distributed Memory support])],
                 [enable_distmem_gasnet=$enableval]
                 [enable_distmem_gpi=$enableval]
                 [enable_distmem_mpi=$enableval]
                 [enable_distmem_armci=$enableval],
                 [enable_distmem_gasnet=yes]
                 [enable_distmem_gpi=yes]
                 [enable_distmem_mpi=yes]
                 [enable_distmem_armci=yes])

   if test x"$enable_distmem_gasnet" != xno; then
      AC_ARG_ENABLE([distmem_gasnet],
                    [AS_HELP_STRING([--disable-distmem_gasnet],
                                    [Disable checking for Distributed Memory GASNet support])],
                    [enable_distmem_gasnet=$enableval],
                    [enable_distmem_gasnet=yes])
   fi

   if test x"$enable_distmem_mpi" != xno; then
      AC_ARG_ENABLE([distmem_mpi],
                    [AS_HELP_STRING([--disable-distmem_mpi],
                                    [Disable checking for Distributed Memory MPI support])],
                    [enable_distmem_mpi=$enableval],
                    [enable_distmem_mpi=yes])
   fi

   if test x"$enable_distmem_armci" != xno; then
      AC_ARG_ENABLE([distmem_armci],
                    [AS_HELP_STRING([--disable-distmem_armci],
                                    [Disable checking for Distributed Memory ARMCI support])],
                    [enable_distmem_armci=$enableval],
                    [enable_distmem_armci=yes])
   fi

   if test x"$enable_distmem_gpi" != xno; then
      AC_ARG_ENABLE([distmem_gpi],
                    [AS_HELP_STRING([--disable-distmem_gpi],
                                    [Disable checking for Distributed Memory GPI support])],
                    [enable_distmem_gpi=$enableval],
                    [enable_distmem_gpi=yes])
   fi

   dnl Create GASNet conduit targets file.
   cat >./sac2crc.GASNetconduits

   dnl Create GASNet conduit build files.
   cat >./build.GASNetconduits
   cat >./build.GASNetconduitsCrossVariant

   dnl Create GASNet conduit settings file.
   cat >./config.GASNetconduits

   if test x"$enable_distmem_mpi" != xno; then
      AX_MPI([enable_distmem_mpi=yes], [enable_distmem_mpi=no])

      if test x"$enable_distmem_mpi" != xno; then
        AC_MSG_CHECKING([for MPI 3 support])
        ax_mpi_save_CC="$CC"
	      CC="$MPICC"
        AC_TRY_COMPILE([#include <mpi.h>],[
/* This program uses MPI 3 one-sided communication to test whether the MPI installation does support these operations. */
int main(int argc, char *argv[]) {
  static MPI_Win win = NULL;
  size_t SAC_DISTMEM_pagesz = 0;

  void *local_page_ptr = NULL;
  size_t owner_rank = 0;
  size_t remote_page_index;

  MPI_Get( local_page_ptr, SAC_DISTMEM_pagesz, MPI_BYTE, owner_rank, remote_page_index * SAC_DISTMEM_pagesz, SAC_DISTMEM_pagesz, MPI_BYTE, win);
}
],[AC_MSG_RESULT(yes)], [enable_distmem_mpi=no
	        AC_MSG_RESULT(no)])
        CC="$ax_mpi_save_CC"
      fi
   fi

   if test x"$enable_distmem_armci" != xno; then
      AC_MSG_CHECKING(if ARMCI_HOME is set)
      if test x"$ARMCI_HOME" != x ; then
        AC_MSG_RESULT([using ARMCI_HOME: $ARMCI_HOME])

        AC_MSG_CHECKING(if $ARMCI_HOME exists)
        if test -r $ARMCI_HOME; then
           AC_MSG_RESULT([yes])
           AC_DEFINE_UNQUOTED([ARMCI_DIR], ["$ARMCI_HOME"],
                              [ARMCI installation])
           AC_SUBST([ARMCI_DIR], [$ARMCI_HOME])
        else
           AC_MSG_RESULT([no])
           enable_distmem_armci=no
        fi
      else
        AC_MSG_RESULT([no])
        enable_distmem_armci=no
      fi
   fi

   if test x"$enable_distmem_gpi" != xno; then
      AC_MSG_CHECKING(if GPI_HOME is set)
      if test x"$GPI_HOME" != x ; then
        AC_MSG_RESULT([using GPI_HOME: $GPI_HOME])
      else
        GPI_HOME=/opt/GPI2
        AC_MSG_RESULT([using default location: $GPI_HOME])
      fi

      AC_MSG_CHECKING(if $GPI_HOME exists)
      if test -r $GPI_HOME; then
         AC_MSG_RESULT([yes])
         AC_DEFINE_UNQUOTED([GPI_DIR], ["$GPI_HOME"],
                            [GPI installation])
         AC_SUBST([GPI_DIR], [$GPI_HOME])
      else
         AC_MSG_RESULT([no])
         enable_distmem_gpi=no
      fi
   fi

   if test x"$enable_distmem_gasnet" != xno; then

################################################################################
#
# feature setup:
#

    AC_MSG_CHECKING(if GASNET_HOME is set)
    if test x"$GASNET_HOME" != x ; then
      gasnet_home="$GASNET_HOME"
      AC_MSG_RESULT([using GASNET_HOME: $gasnet_home])
    else
      gasnet_home=/usr/local/gasnet
      AC_MSG_RESULT([using default location: $gasnet_home])
    fi

    gasnet_conduit_names=""
    AC_MSG_CHECKING(if $gasnet_home exists)
    if test -r $gasnet_home; then
        AC_MSG_RESULT([yes])
        AC_MSG_CHECKING(installed gasnet conduits)

        cat <<EOF >>./config.GASNetconduits
################################################################################
#
# GASNet conduit settings:
#

EOF

        for gasnet_conduit_path in $gasnet_home/include/*-conduit/; do
          gasnet_conduit_filename="$(basename "${gasnet_conduit_path}")"
          gasnet_conduit_name="${gasnet_conduit_filename//-conduit/}"
          gasnet_conduit_name_uc=`echo $gasnet_conduit_name | tr '[a-z]' '[A-Z]'`
          if test x"$gasnet_conduit_names" != x ; then
              gasnet_conduit_names="$gasnet_conduit_names "
          fi
          gasnet_conduit_names="$gasnet_conduit_names$gasnet_conduit_name"
          gasnet_conduit_makefile="$gasnet_conduit_path$gasnet_conduit_name-seq.mak"

          gasnet_conduit_cc="$(getmakevar $gasnet_conduit_makefile GASNET_CC)"
          gasnet_conduit_cppflags="$(getmakevar $gasnet_conduit_makefile GASNET_CPPFLAGS)"
          gasnet_conduit_cflags="$(getmakevar $gasnet_conduit_makefile GASNET_CFLAGS)"
          gasnet_conduit_ld="$(getmakevar $gasnet_conduit_makefile GASNET_LD)"
          gasnet_conduit_ldflags="$(getmakevar $gasnet_conduit_makefile GASNET_LDFLAGS)"
          gasnet_conduit_libs="$(getmakevar $gasnet_conduit_makefile GASNET_LIBS)"

          cat <<EOF >>./sac2crc.GASNetconduits
target distmem_gasnet_$gasnet_conduit_name::distmem_gasnet:
COMMLIB_CONDUIT  := "$gasnet_conduit_name"
CC               := "$gasnet_conduit_ld -std=gnu99"
CCLINK           += "$gasnet_conduit_libs"
LDFLAGS          += "$gasnet_conduit_ldflags"

target distmemcheck_gasnet_$gasnet_conduit_name::distmemcheck_gasnet:
COMMLIB_CONDUIT  := "$gasnet_conduit_name"
CC               := "$gasnet_conduit_ld -std=gnu99"
CCLINK           += "$gasnet_conduit_libs"
LDFLAGS          += "$gasnet_conduit_ldflags"

target distmemprofile_gasnet_$gasnet_conduit_name::distmemprofile_gasnet:
COMMLIB_CONDUIT  := "$gasnet_conduit_name"
CC               := "$gasnet_conduit_ld -std=gnu99"
CCLINK           += "$gasnet_conduit_libs"
LDFLAGS          += "$gasnet_conduit_ldflags"

EOF

          cat <<EOF >>./config.GASNetconduits
GASNET_${gasnet_conduit_name_uc}_CC        := $gasnet_conduit_cc -std=gnu99
GASNET_${gasnet_conduit_name_uc}_CPPFLAGS  := $gasnet_conduit_cppflags
GASNET_${gasnet_conduit_name_uc}_CFLAGS    := $gasnet_conduit_cflags
GASNET_${gasnet_conduit_name}_cap       := $gasnet_conduit_name_uc

EOF

          dnl Append the rules for the libsacdistmem GASNet object files to build.mkf
          dnl At the places where we print \$$** or \$$*@ we want $* $@ to end up in the makefile.
          dnl However, $* is replaced by an empty string and $@ by \, escaping does not help.
          cat <<EOF >>./build.GASNetconduits
%.\$(\$(STYLE)_short).gasnet${gasnet_conduit_name}.o: %.c
	@if [[ "compile \$(dir \$$**)" != "\`cat .make_track\`" ]] ; \\
          then \$(ECHO) "compile \$(dir \$$**)" > .make_track; \\
              \$(ECHO) ""; \\
              \$(ECHO) "Compiling files in directory \$(PREFIX_LOCAL)\$(dir \$@)"; \\
          fi
	@\$(ECHO) "  Compiling GasNET ${gasnet_conduit_name} \$(\$(STYLE)_long) code: \$(notdir \$<)"
	\$(HIDE) \$(GASNET_${gasnet_conduit_name_uc}_CC) \\
                  \$(GASNET_${gasnet_conduit_name_uc}_CPPFLAGS) \$(GASNET_${gasnet_conduit_name_uc}_CFLAGS) \$(CC_FLAGS_\$(\$(STYLE)_cap)) \$(SETTINGS_\$(\$(STYLE)_cap)) \\
                  \$(CC_FLAGS_YY) \$(SETTINGS_DISTMEM) \$(SETTINGS_DISTMEM_GASNET) \\
                  \$(INCDIRS) -o \$$*@ -c \$<
	@\$(CLOCK_SKEW_ELIMINATION)
EOF

          dnl Append the rules for the libsacdistmem GASNet cross variant object files to build.mkf
          dnl At the places where we print \$$** or \$$*@ we want $* $@ to end up in the makefile.
          dnl However, $* is replaced by an empty string and $@ by \, escaping does not help.
          cat <<EOF >>./build.GASNetconduitsCrossVariant
%.\$(\$(STYLE)_short)\$(CROSS_VARIANT).gasnet${gasnet_conduit_name}.o: %.c
	@if [[ "compile \$(dir \$$**)" != "\`cat .make_track\`" ]] ; \\
          then \$(ECHO) "compile \$(dir \$$**)" > .make_track; \\
              \$(ECHO) ""; \\
              \$(ECHO) "Compiling files in directory \$(PREFIX_LOCAL)\$(dir \$@)"; \\
          fi
	@\$(ECHO) "  Compiling GasNET ${gasnet_conduit_name} \$(\$(STYLE)_long) code: \$(notdir \$<)"
	\$(HIDE) \$(GASNET_${gasnet_conduit_name_uc}_CC) \\
                  \$(GASNET_${gasnet_conduit_name_uc}_CPPFLAGS) \$(GASNET_${gasnet_conduit_name_uc}_CFLAGS) \$(CC_FLAGS_\$(\$(STYLE)_cap)) \$(SETTINGS_\$(\$(STYLE)_cap)) \\
                  \$(CC_FLAGS_YY) \$(SETTINGS_DISTMEM) \$(SETTINGS_DISTMEM_GASNET) \\
                  \$(INCDIRS) -o \$$*@ -c \$<
	@\$(CLOCK_SKEW_ELIMINATION)
EOF

          done

          if test x"$gasnet_conduit_names" = x ; then
            AC_MSG_RESULT([none])
            enable_distmem_gasnet=no
          else
            AC_MSG_RESULT([$gasnet_conduit_names])
          fi
      else
          AC_MSG_RESULT([no])
          enable_distmem_gasnet=no
      fi

      if test x"$enable_distmem_gasnet" = xno -a x"$enable_distmem_gpi" = xno -a x"$enable_distmem_mpi" = xno -a x"$enable_distmem_armci" = xno ; then
          enable_distmem=no
      else
          enable_distmem=yes
      fi

   fi

   have_distmem=`if test x"$enable_distmem" != xno; then echo 1; else echo 0; fi`
   have_distmem_gasnet=`if test x"$enable_distmem_gasnet" != xno; then echo 1; else echo 0; fi`
   have_distmem_gpi=`if test x"$enable_distmem_gpi" != xno; then echo 1; else echo 0; fi`
   have_distmem_mpi=`if test x"$enable_distmem_mpi" != xno; then echo 1; else echo 0; fi`
   have_distmem_armci=`if test x"$enable_distmem_armci" != xno; then echo 1; else echo 0; fi`

   AC_DEFINE_UNQUOTED([ENABLE_DISTMEM], [$have_distmem],
                      [Define to 1 if distributed memory support is enabled, otherwise 0.])
   AC_DEFINE_UNQUOTED([ENABLE_DISTMEM_GASNET], [$have_distmem_gasnet],
                      [Define to 1 if GASNet conduit is supported, otherwise 0.])
   AC_DEFINE_UNQUOTED([DISTMEM_GASNET_CONDUITS], ["$gasnet_conduit_names"],
                      [Available GASNet conduits.])
   AC_DEFINE_UNQUOTED([ENABLE_DISTMEM_GPI], [$have_distmem_gpi],
                      [Define to 1 if GPI conduit is supported, otherwise 0.]) 
   AC_DEFINE_UNQUOTED([ENABLE_DISTMEM_MPI], [$have_distmem_mpi],
                      [Define to 1 if MPI conduit is supported, otherwise 0.])
   AC_DEFINE_UNQUOTED([ENABLE_DISTMEM_ARMCI], [$have_distmem_armci],
                      [Define to 1 if ARMCI conduit is supported, otherwise 0.])

   AC_SUBST([ENABLE_DISTMEM], [$enable_distmem])
   AC_SUBST([ENABLE_DISTMEM_GASNET], [$enable_distmem_gasnet])
   AC_SUBST([DISTMEM_GASNET_CONDUITS], [$gasnet_conduit_names])
   AC_SUBST([ENABLE_DISTMEM_GPI], [$enable_distmem_gpi])
   AC_SUBST([ENABLE_DISTMEM_MPI], [$enable_distmem_mpi])
   AC_SUBST([ENABLE_DISTMEM_ARMCI], [$enable_distmem_armci])
])