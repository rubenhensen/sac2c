AC_DEFUN([EMPTY_DUMMY_MACRO])

AC_DEFUN([DISTMEM_INIT_DEFAULTS], dnl
[
   AC_ARG_ENABLE([distmem],
                 [AS_HELP_STRING([--enable-distmem=x,y,z],
                                 [Enable distmem targets x,y,z, or use --disable-distmem to disable entirely.
				  Available targets: gasnet,gpi,mpi,armci.
				  The default target is "auto" which enables whichever target can be found.])],
		 [], [enable_distmem=auto])
   enable_distmem_gasnet=no
   enable_distmem_gpi=no
   enable_distmem_mpi=no
   enable_distmem_armci=no
   case $enable_distmem in
      auto|yes)
	    enable_distmem_gasnet=auto
	    enable_distmem_gpi=auto
	    enable_distmem_mpi=auto
	    enable_distmem_armci=auto
	    ;;
   esac	    

   case $enable_distmem in *gasnet*) enable_distmem_gasnet=yes;; esac
   case $enable_distmem in *gpi*) enable_distmem_gpi=yes;; esac
   case $enable_distmem in *mpi*) enable_distmem_mpi=yes;; esac
   case $enable_distmem in *armci*) enable_distmem_armci=yes;; esac
   AC_ARG_ENABLE([distmem-gasnet], [AS_HELP_STRING([--disable-distmem-gasnet], [Disable the GASnet target.])])
   AC_ARG_ENABLE([distmem-gpi], [AS_HELP_STRING([--disable-distmem-gpi], [Disable the GPI target.])])
   AC_ARG_ENABLE([distmem-mpi], [AS_HELP_STRING([--disable-distmem-mpi], [Disable the MPI target.])])
   AC_ARG_ENABLE([distmem-armci], [AS_HELP_STRING([--disable-distmem-armci], [Disable the ARMci target.])])

])

AC_DEFUN([CHECK_DISTMEM_MPI], dnl
[
      AX_MPI([enable_distmem_mpi=yes], [enable_distmem_mpi=no])

      if test x"$enable_distmem_mpi" != xno; then
        AC_MSG_CHECKING([for MPI 3 support])
        ax_mpi_save_CC="$CC"
        CC="$MPICC"
        AC_TRY_COMPILE([#include <mpi.h>],
	               [
         /* This program uses MPI 3 one-sided communication to test
            whether the MPI installation does support these operations. */
         int main(int argc, char *argv[]) {
           static MPI_Win win = NULL;
           size_t SAC_DISTMEM_pagesz = 0;
         
           void *local_page_ptr = NULL;
           size_t owner_rank = 0;
           size_t remote_page_index;
         
           MPI_Get( local_page_ptr, SAC_DISTMEM_pagesz,
                    MPI_BYTE, owner_rank,
         	   remote_page_index * SAC_DISTMEM_pagesz,
         	   SAC_DISTMEM_pagesz, MPI_BYTE, win);
         }
                      ],
		      [AC_MSG_RESULT(yes)],
		      [enable_distmem_mpi=no;
                       AC_MSG_RESULT(no)])
        CC="$ax_mpi_save_CC"
      fi
])

AC_DEFUN([CHECK_DISTMEM_ARMCI], dnl
	 [
      enable_distmem_armci=no
      AC_MSG_CHECKING([whether ARMCI_HOME is set])
      if test x"$ARMCI_HOME" != x ; then
        AC_MSG_RESULT([using ARMCI_HOME: $ARMCI_HOME])

        AC_MSG_CHECKING([whether $ARMCI_HOME exists])
        if test -r "$ARMCI_HOME"; then
           AC_MSG_RESULT([yes])
           AC_DEFINE_UNQUOTED([ARMCI_DIR], ["$ARMCI_HOME"],
                              [ARMCI installation])
           AC_SUBST([ARMCI_DIR], [$ARMCI_HOME])
	   enable_distmem_armci=yes
        else
           AC_MSG_RESULT([no])
        fi
      else
        AC_MSG_RESULT([no])
      fi
])

AC_DEFUN([CHECK_DISTMEM_GPI], dnl
	 [
      enable_distmem_gpi=no
      AC_MSG_CHECKING([whether GPI_HOME is set])
      if test x"$GPI_HOME" != x ; then
        AC_MSG_RESULT([using GPI_HOME: $GPI_HOME])
        AC_MSG_CHECKING([whether $GPI_HOME exists])
        if test -r "$GPI_HOME"; then
         AC_MSG_RESULT([yes])
         AC_DEFINE_UNQUOTED([GPI_DIR], ["$GPI_HOME"],
                            [GPI installation])
         AC_SUBST([GPI_DIR], [$GPI_HOME])
	 enable_distmem_gpi=yes
        else
         AC_MSG_RESULT([no])
        fi
      else
       AC_MSG_RESULT([no])
      fi
])

AC_DEFUN([CHECK_DISTMEM_GASNET], dnl
	 [
    enable_distmem_gastnet=no
    AC_MSG_CHECKING([whether GASNET_HOME is set])
    if test x"$GASNET_HOME" != x ; then
      gasnet_home="$GASNET_HOME"
      AC_MSG_RESULT([using GASNET_HOME: $gasnet_home])
    
      AC_MSG_CHECKING([whether $gasnet_home exists])
      if test -r "$gasnet_home"; then
         AC_MSG_RESULT([yes])
	
         AC_MSG_CHECKING([for installed gasnet conduits])
 	 gasnet_conduits=$(echo $( (cd "$gasnet_home/include" && ls -d *-conduit 2>/dev/null | cut -d- -f1 ) ) )
         if test x"$gasnet_conduits" = x ; then
            AC_MSG_RESULT([none])
         else
             AC_MSG_RESULT([$gasnet_conduits])
	     enable_distmem_gasnet=yes
         fi
      else
       AC_MSG_RESULT([no])
      fi
      else
       AC_MSG_RESULT([no])
      fi
])

AC_DEFUN([GASNET_INIT], dnl
[
   dnl Create GASNet conduit targets file.
   true >./sac2crc.GASNetconduits

   dnl Create GASNet conduit build files.
   true >./build.GASNetconduits
   true >./build.GASNetconduitsCrossVariant

   dnl Create GASNet conduit settings file.
   true >./config.GASNetconduits

   for gasnet_conduit_name in $gasnet_conduits; do
       gasnet_conduit_path=$gasnet_home/include/$gasnet_conduit_name-conduit/
       gasnet_conduit_name_uc=$(echo $gasnet_conduit_name | tr '[a-z]' '[A-Z]')
       gasnet_conduit_makefile=$gasnet_conduit_path$gasnet_conduit_name-seq.mak

       gasnet_conduit_cc=$(getmakevar $gasnet_conduit_makefile GASNET_CC)
       gasnet_conduit_cppflags=$(getmakevar $gasnet_conduit_makefile GASNET_CPPFLAGS)
       gasnet_conduit_cflags=$(getmakevar $gasnet_conduit_makefile GASNET_CFLAGS)
       gasnet_conduit_ld=$(getmakevar $gasnet_conduit_makefile GASNET_LD)
       gasnet_conduit_ldflags=$(getmakevar $gasnet_conduit_makefile GASNET_LDFLAGS)
       gasnet_conduit_libs=$(getmakevar $gasnet_conduit_makefile GASNET_LIBS)

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

])

AC_DEFUN([CHECK_DISTMEM_BACKEND], dnl
[
   if test x"$enable_distmem_$1" != xno; then
      expected=$enable_distmem_$1
      
      $2
      
      if test x"$expected" = xyes -a x"$enable_distmem_$1" = xno; then
      	 AC_MSG_ERROR($3)
      fi
   fi
])


dnl check for Distributed Memory support
AC_DEFUN([CHECK_DISTMEM], dnl
[
   DISTMEM_INIT_DEFAULTS

   if test x"$enable_distmem" != xno; then
     CHECK_DISTMEM_BACKEND([mpi], [CHECK_DISTMEM_MPI], [unable to find a working MPI back-end])
     CHECK_DISTMEM_BACKEND([armci], [CHECK_DISTMEM_ARMCI], [AMRCI_HOME not set or ARMci directory not found])
     CHECK_DISTMEM_BACKEND([gpi], [CHECK_DISTMEM_GPI], [GPI_HOME not set or GPI directory not found])
     CHECK_DISTMEM_BACKEND([gasnet], [CHECK_DISTMEM_GASNET], [GASNET_HOME not set or directory not found or no GASnet conduits defined.])
     if test x"$enable_distmem_gasnet" != xno; then
      GASNET_INIT
     fi
   
     if test x"$enable_distmem_gasnet" = xno \
        -a x"$enable_distmem_gpi" = xno \
	-a x"$enable_distmem_mpi" = xno \
	-a x"$enable_distmem_armci" = xno \
	; then
          if test x"$enable_distmem" != xauto; then
	     AC_MSG_ERROR([unable to find a suitable distmem back-end.])
	  fi
	  enable_distmem=no
      else
          enable_distmem=yes
      fi
   fi

   
   AC_DEFINE_UNQUOTED([ENABLE_DISTMEM], [`test x"$enable_distmem" = xno && echo 0 || echo 1`],
                      [Define to 1 if distributed memory support is enabled, otherwise 0.])
   AC_DEFINE_UNQUOTED([ENABLE_DISTMEM_GASNET], [`test x"$enable_distmem_gasnet" = xno && echo 0 || echo 1`],
                      [Define to 1 if GASNet conduit is supported, otherwise 0.])
   AC_DEFINE_UNQUOTED([DISTMEM_GASNET_CONDUITS], ["$gasnet_conduits"],
                      [Available GASNet conduits.])
   AC_DEFINE_UNQUOTED([ENABLE_DISTMEM_GPI], [`test x"$enable_distmem_gpi" = xno && echo 0 || echo 1`],
                      [Define to 1 if GPI conduit is supported, otherwise 0.]) 
   AC_DEFINE_UNQUOTED([ENABLE_DISTMEM_MPI], [`test x"$enable_distmem_mpi" = xno && echo 0 || echo 1`],
                      [Define to 1 if MPI conduit is supported, otherwise 0.])
   AC_DEFINE_UNQUOTED([ENABLE_DISTMEM_ARMCI], [`test x"$enable_distmem_armci" = xno && echo 0 || echo 1`],
                      [Define to 1 if ARMCI conduit is supported, otherwise 0.])

   AC_SUBST([ENABLE_DISTMEM], [$enable_distmem])
   AC_SUBST([ENABLE_DISTMEM_GASNET], [$enable_distmem_gasnet])
   AC_SUBST([DISTMEM_GASNET_CONDUITS], [$gasnet_conduits])
   AC_SUBST([ENABLE_DISTMEM_GPI], [$enable_distmem_gpi])
   AC_SUBST([ENABLE_DISTMEM_MPI], [$enable_distmem_mpi])
   AC_SUBST([ENABLE_DISTMEM_ARMCI], [$enable_distmem_armci])
])
