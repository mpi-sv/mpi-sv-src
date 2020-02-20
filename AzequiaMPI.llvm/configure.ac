#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Disclaimer
# This file has been taken from mpich2 distribution and adapted to Azequia distribution.

# Configure version could be >= 2.61, but some machines complaints, so 2.00
AC_PREREQ(2.50)
AC_INIT([azequiampi], [2.0.0], [jarico@unex.es, juancarl@unex.es])
AC_CONFIG_AUX_DIR([./config])

AC_CONFIG_HEADER(config/config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
#AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Automake initialisation and check for minimal automake API version 1.11.1
AM_INIT_AUTOMAKE([1.09 subdir-objects tar-ustar dist-bzip2 foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_PROG_CC_C_O

# Check for arguments.
AC_ARG_WITH(model,
[  --with-model=name - Specify the synchronization model for Azequia.
                      Posible values are:
      blk     - Blocking model.
      lfq     - Lock-free queue model (default).],,with_model=lfq)

AC_ARG_WITH(device,
[  --with-device=name - Specify the communication device for Azequia.
                       Posible values are:
      tcpip     - TCP/IP-based communication device (default).
      udpip     - UDP/IP-based communication device.
      none      - dummy communication device.],,with_device=tcpip)

AC_ARG_WITH(hwloc,
[  --with-hwloc[[=path]] - Specify the path of the 'hwloc' library in case of it has been installed 
                           #in a custom location.],,with_hwloc=empty)

AC_ARG_ENABLE(hwloc,
[  --disable-hwloc  Disable support for binding Azequia nodes to processors using the 'hwloc' library.],
    disable_hwloc=yes, disable_hwloc=no)


#AC_ARG_WITH(osi,
#[  --with-osi[[=path]] - Use OSI (Operating System Interface) instead of pthreads as library for
                      #thread management and synchronization. Optionally you can specify the path
                      #of the 'osi' library in case of it has been installed in a custom location.],,with_osi=no)

AC_ARG_ENABLE(fast,
[  --enable-fast=option - Control the level of fast execution in the
                         Azequia implementation. 'option' is a list of
                         comma separated names including:
      dflt     - Default compiler optimization -O2 for all language bindings (default).
      O<n>     - Appends default optimization flags, -O<n>, to all internal compiler flags.
      none     - None of above options.],,enable_fast=dflt)

AC_ARG_ENABLE(g,
[  --enable-g=option - Control the level of debugging support in the
                      azequiampi implementation. Option is a list of comma
                      separated names including:
      none        - No debugging (default).
      debug       - Add compiler flag, -g, to all internal compiler flags.
      profile     - Add compiler flag, -pg, to all internal compiler flags.
      xpn         - Print exceptions.
      com         - Enable COM debug.
      dlq         - Enable DLQ debug.
      rqst        - Enable RQST debug.
      util        - Enable UTIL debug.
      pmp         - Enable PMP debug.
      grp         - Enable GRP debug.
      rpc         - Enable RPC debug.
      mbx         - Enable MBX debug.
      rpq         - Enable RPQ debug.
      inet        - Enable INET debug.
      malloc      - Enable MALLOC debug.
      hwloc       - Enable HWLOC debug.
      mpi         - Enable MPI debug.
      mpi-check   - Enable MPI checking.
      mpi-verbose - Enable MPI verbosing.
      all         - All of the above choices.],,enable_g=none)

AC_ARG_ENABLE(azqmpi,
[  --disable-azqmpi - Only build Azequia library (disabled by default).],disable_azqmpi=yes,disable_azqmpi=no)

AC_ARG_ENABLE(profiling,
[  --enable-profiling - Build profiling interface AzequiaMPI library.],enable_profiling=yes)

AC_ARG_ENABLE(fastboxes,
[  --disable-fastboxes - Disable fastboxes support in Azequia.],
    disable_fastboxes=yes, disable_fastboxes=no)

##############################################################################
# Checks for any incoherent argument combination.
###############################################################################




# comprobar las cabeceras de hwloc!!!!!!!!!!!







##############################################################################
# Checks for libraries.
##############################################################################


##############################################################################
# Checks for header files.
##############################################################################
AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h libintl.h limits.h malloc.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/socket.h unistd.h])

##############################################################################
# Checks for typedefs, structures, and compiler characteristics.
##############################################################################
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
#AC_TYPE_UINT64_T
AC_C_VOLATILE

# Check for "long" type size (help to find the architecture)
AC_CHECK_SIZEOF(long)

# Find the values of "host", "host_cpu", "host_vendor", "host_os" and "host_alias"
AC_CANONICAL_HOST

# Find architecture and operating system names:
m4_include([./config/azqmpi_arch.m4])
AZQMPI_CONFIG_ARCH


##############################################################################
# Checks for library functions.
##############################################################################
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([bzero dup2 gethostbyname gethostname inet_ntoa memset socket stpcpy strchr strdup strerror])

###############################################################################
# Try to find a correct version of python
###############################################################################
AC_CHECK_PROG(PY26, python2.6, python2.6)
AC_CHECK_PROG(PY25, python2.5, python2.5)
AC_CHECK_PROG(PY24, python2.4, python2.4)
AC_CHECK_PROG(PY23, python2.3, python2.3)
AC_CHECK_PROG(PY22, python2.2, python2.2)
AC_CHECK_PROG(PY2,  python2, python2)
AC_CHECK_PROG(PY,   python, python)

if test -z "$PY26" -a -z "$PY25" -a -z "$PY24" -a -z "$PY23" -a  \
        -z "$PY22" -a -z "$PY2" -a -z "$PY" ; then
  AC_MSG_ERROR(Could not find a python executable in path: $PATH)
fi

PYTHON=
for pypgm in $PY26 $PY25 $PY24 $PY23 $PY22 $PY2 $PY ; do
  AC_MSG_CHECKING(that $pypgm is a valid python version)
  changequote(,)dnl
  py_version=`$pypgm -c "import sys;print sys.version_info[0]"`
  py_subversion=`$pypgm -c "import sys;print sys.version_info[1]"`
  changequote([,])dnl
  if test "$py_version" -gt "2" -o "$py_version" -ge "2" -a "$py_subversion" -ge "2" ; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    continue
  fi
  PYTHON="$pypgm"
  break
done

if test -z "$PYTHON" ; then
  AC_MSG_ERROR(Python required (version 2.2 or greater))
fi

AC_SUBST(PYTHON)

###############################################################################
# Compiler default flags
###############################################################################

# Initialize 'CFLAGS' just here.
CFLAGS=-Wall


###############################################################################
# with-model
###############################################################################
AM_CONDITIONAL(WANT_USE_BLK, test "$with_model" = "blk")
AM_CONDITIONAL(WANT_USE_LFQ, test "$with_model" = "lfq")

case "$with_model" in
  "blk"      ) modeldir=blk;;
  "lfq"      ) modeldir=lfq;;
esac

AC_SUBST(modeldir)

###############################################################################
# with-device
###############################################################################
AM_CONDITIONAL(WANT_USE_TCPIP, test "$with_device" = "tcpip")
AM_CONDITIONAL(WANT_USE_UDPIP, test "$with_device" = "udpip")
AM_CONDITIONAL(WANT_USE_NONE, test "$with_device" = "none")

case "$with_device" in
  "tcpip"  ) devicedir=inet/tcpip;;
  "udpip"  ) devicedir=inet/udpip;;
  "none"   ) devicedir=inet/none;;
esac

AC_SUBST(devicedir)

# At the moment, it's hardcoded, but we must do it depending on OS.
AC_DEFINE_UNQUOTED(_POSIX_THREADS, 1, [Define if OS has POSIX_THREADS])

###############################################################################
# with-hwloc
###############################################################################

if test "$disable_hwloc" = "yes" ; then

  use_hwloc="no"

else

  if test "$with_hwloc" = "empty"; then

    AC_CHECK_LIB([hwloc], [hwloc_topology_init], , AC_MSG_ERROR(Unable to find 'hwloc' library. Remember you can specify  its path through --with-hwloc=<path> or disable through --disable-hwloc))
    hwloc_root="default path"

  else

    LDFLAGS="$LDFLAGS -L$with_hwloc/lib"
    AC_CHECK_LIB([hwloc], [hwloc_topology_init], , AC_MSG_ERROR(Unable to find 'hwloc' library in '$with_hwloc'))
    hwloc_root=$with_hwloc
    AC_SUBST(hwloc_root)

  fi

  use_hwloc="yes"

fi



###############################################################################
# with-fastboxes
###############################################################################

if test "$disable_fastboxes" = "yes" ; then
  use_fastboxes="no"
else
  use_fastboxes="yes"
  AC_DEFINE_UNQUOTED(USE_FASTBOXES, 1, [Enable fastboxes feature in AzequiaMPI])
fi


###############################################################################
# with-osi
###############################################################################
#if test "$with_osi" != "no" ; then
  #AC_MSG_CHECKING(for pthread_create in -losi)

  #rm -f conftest*
  #cat > conftestc.c <<EOF
    ##include <osi.h>
    ##include "confdefs.h"

    #void *func (void *args)
    #{
      #return NULL;
    #}
    #int main (int argc, char *argv)
    #{
      #pthread_t thr;
      #if (pthread_create(&thr, NULL, func, NULL))
        #printf("KO\n");
      #else
        #printf("OK\n");
    #}
#EOF

  #if test "$with_osi" = "yes" ; then
    #myccompile='${CC-cc} -c $CFLAGS conftestc.c 1>&AC_FD_CC'
    #myclink='${CC-cc} $CFLAGS -o conftest conftestc.o -losi -lpthread >conftest.out 2>&1'
  #else
    #myccompile='${CC-cc} -I${with_osi}/include -c $CFLAGS conftestc.c 1>&AC_FD_CC'
    #myclink='${CC-cc} $CFLAGS -o conftest conftestc.o -L$with_osi/lib -losi -lpthread >conftest.out 2>&1'
  #fi

  ## Try the compile step
  #if AC_TRY_EVAL(myccompile) && test -s conftestc.o ; then
    #:
  #else
    #echo "configure: failed program was:" >&AC_FD_CC
    #cat conftestc.c >&AC_FD_CC
  #fi

  ## Now try the link step
  #if AC_TRY_EVAL(myclink) && test -x conftest${ac_exeext} ; then
    #AC_MSG_RESULT(yes)
    #AC_DEFINE_UNQUOTED(__OSI, 1, [Define if Azequia must use 'osi' library])
    #use_osi="yes"
    #osi_root=$with_osi
    #libosi="-losi"
    #AC_SUBST(osi_root)
    #AC_SUBST(libosi)
  #else
    #AC_MSG_RESULT(no)
    #if test "$with_osi" = "yes" ; then
      #AC_MSG_ERROR(Unable to find 'osi' library. Remember you can specify its path through --with-osi=<path>)
    #else
      #AC_MSG_ERROR(Unable to find 'osi' library in '$with_osi')
    #fi
  #fi

#else
  #use_osi="no"
#fi

###############################################################################
# enable-fast
###############################################################################
save_IFS="$IFS"
IFS=","
for option in $enable_fast ; do
  case "$option" in
    dflt    ) enable_default_optimize=yes    ;;
    O*      )
              # Allows O<n> where <n> can be [0-9] or ' '.
              opt_flags=`echo $option | sed -e 's%\(O[0-9] \)%\1%g'`
              if test -n "$opt_flags" ; then
                enable_default_optimize=yes
                USER_CFLAGS="-$option"
              else
                AC_MSG_WARN([Unknown value $option for --enable-fast])
              fi                             ;;
    none    ) enable_default_optimize=no     ;;
    *       ) AC_MSG_WARN([Unknown value $option for --enable-fast]) ;;
  esac
done
IFS="$save_IFS"

if test "$enable_default_optimize" = "yes" ; then
  if test -n "$USER_CFLAGS" ; then
    CFLAGS="$CFLAGS $USER_CFLAGS"
  else
    CFLAGS="$CFLAGS -O2"
  fi
fi

###############################################################################
# enable-g
###############################################################################
save_IFS="$IFS"
IFS=","
for option in $enable_g ; do
  case "$option" in
    debug       ) CFLAGS="$CFLAGS -g"     ;;
    profile     ) CFLAGS="$CFLAGS -pg"    ;;
    none        )                         ;;
    xpn         ) have_xpn_debug="1"      ;;
    com         ) have_com_debug="1"      ;;
    dlq         ) have_dlq_debug="1"      ;;
    rqst        ) have_rqst_debug="1"     ;;
    util        ) have_util_debug="1"     ;;
    pmp         ) have_pmp_debug="1"      ;;
    grp         ) have_grp_debug="1"      ;;
    rpc         ) have_rpc_debug="1"      ;;
    mbx         ) have_mbx_debug="1"      ;;
    rpq         ) have_rpq_debug="1"      ;;
    inet        ) have_inet_debug="1"     ;;
    malloc      ) have_malloc_debug="1"   ;;
    hwloc       ) have_hwloc_debug="1"    ;;
    mpi         ) have_mpi_debug="1"      ;;
    mpi-check   ) have_mpi_check="1"      ;;
    mpi-verbose ) have_mpi_verbose="1"    ;;
    all         ) have_all_debug="1"      ;;
    *           ) AC_MSG_ERROR(Invalid value $enable_g for enable-g) ;;
  esac
done
IFS="$save_IFS"

debug_mods=
n_debug_mods=
if test -n "$have_xpn_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__XPN_PRINT, 1, [Enable XPN debugging])
  debug_mods="xpn"
else
  n_debug_mods="xpn"
fi
if test -n "$have_com_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__COM_DEBUG, 1, [Enable COM debugging])
  debug_mods="$debug_mods com"
else
  n_debug_mods="$n_debug_mods com"
fi
if test -n "$have_dlq_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__DLQ_DEBUG, 1, [Enable DLQ debugging])
  debug_mods="$debug_mods dlq"
else
  n_debug_mods="$n_debug_mods dlq"
fi
if test -n "$have_rqst_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__RQST_DEBUG, 1, [Enable RQST debugging])
  debug_mods="$debug_mods rqst"
else
  n_debug_mods="$n_debug_mods rqst"
fi
if test -n "$have_util_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__UTIL_DEBUG, 1, [Enable UTIL debugging])
  debug_mods="$debug_mods util"
else
  n_debug_mods="$n_debug_mods util"
fi
if test -n "$have_pmp_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__PMP_DEBUG, 1, [Enable PMP debugging])
  debug_mods="$debug_mods pmp"
else
  n_debug_mods="$n_debug_mods pmp"
fi
if test -n "$have_grp_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__GRP_DEBUG, 1, [Enable GRP debugging])
  debug_mods="$debug_mods grp"
else
  n_debug_mods="$n_debug_mods grp"
fi
if test -n "$have_rpc_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__RPC_DEBUG, 1, [Enable RPC debugging])
  debug_mods="$debug_mods rpc"
else
  n_debug_mods="$n_debug_mods rpc"
fi
if test -n "$have_mbx_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__MBX_DEBUG, 1, [Enable MBX debugging])
  debug_mods="$debug_mods mbx"
else
  n_debug_mods="$n_debug_mods mbx"
fi
if test -n "$have_rpq_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__RPQ_DEBUG, 1, [Enable RPQ debugging])
  debug_mods="$debug_mods rpq"
else
  n_debug_mods="$n_debug_mods rpq"
fi
if test -n "$have_inet_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__INET_DEBUG, 1, [Enable INET debugging])
  debug_mods="$debug_mods inet"
else
  n_debug_mods="$n_debug_mods inet"
fi
if test -n "$have_malloc_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__DEBUG_MALLOC, 1, [Enable MALLOC debugging])
  debug_mods="$debug_mods malloc"
else
  n_debug_mods="$n_debug_mods malloc"
fi
if test -n "$have_hwloc_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(__HWLOC_VERBOSE, 1, [Enable HWLOC debugging])
  debug_mods="$debug_mods hwloc"
else
  n_debug_mods="$n_debug_mods hwloc"
fi
if test -n "$have_mpi_debug" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(DEBUG_MODE, 1, [Enable MPI debugging])
  debug_mods="$debug_mods mpi"
else
  n_debug_mods="$n_debug_mods mpi"
fi
if test -n "$have_mpi_check" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(CHECK_MODE, 1, [Enable MPI checking])
  debug_mods="$debug_mods mpi-check"
else
  n_debug_mods="$n_debug_mods mpi-check"
fi
if test -n "$have_mpi_verbose" -o -n "$have_all_debug" ; then
  AC_DEFINE_UNQUOTED(VERBOSE_MODE, 1, [Enable MPI verbosing])
  debug_mods="$debug_mods mpi-verbose"
else
  n_debug_mods="$n_debug_mods mpi-verbose"
fi

###############################################################################
# disable-azqmpi
###############################################################################
if test "$disable_azqmpi" = "no" ; then
  AC_DEFINE_UNQUOTED(AZQ_MPI, 1, [Enable support for AzequiaMPI])
fi

AM_CONDITIONAL(WANT_BUILD_AZQMPI, test "$disable_azqmpi" = "no")


###############################################################################
# enable-profiling interface
###############################################################################
if test "$enable_profiling" = "yes" ; then
  use_profiling_interface="yes"
  AC_DEFINE_UNQUOTED(AZQMPI_PROFILING, 1, [Enable profiling interface in AzequiaMPI])
else
  use_profiling_interface="no"
fi

#AM_CONDITIONAL(WANT_PROFILING_AZQMPI, test "$enable_profiling" = "yes")

###############################################################################
# Try to select the timer to use depending on what is available
###############################################################################
AC_CHECK_HEADERS([time.h], [], [AC_MSG_ERROR([Unable to find time.h])])

AC_CHECK_LIB([rt], [clock_gettime], [], [missing_rt="yes"])

if test "$missing_rt" != "yes" ; then
  AC_CHECK_FUNCS([clock_gettime], [], [])
else
  AC_CHECK_HEADERS([sys/time.h], [], [AC_MSG_ERROR([Unable to find sys/time.h])])
  AC_CHECK_FUNCS([gettimeofday], [], [AC_MSG_ERROR([No timer found])])
fi

###############################################################################
# 1. First, check for 'pthreads' library existence.
# 2. Second, select 'sem_init' or 'sem_open' depending on what is available
#    and not broken (MAC OS for example).
###############################################################################
AC_CHECK_HEADERS([pthread.h], [], [AC_MSG_ERROR([Unable to find pthread.h])])
AC_CHECK_HEADERS([semaphore.h], [], [AC_MSG_ERROR([Unable to find semaphore.h])])

AC_CHECK_LIB([pthread], [pthread_key_create], [], [AC_MSG_ERROR([Unable to find pthreads library])])

AC_CHECK_FUNC([sem_init], [missing_sem_init="no"], [missing_sem_init="yes"])

if test "$missing_sem_init" = "no" ; then
  AC_MSG_CHECKING([whether sem_init works])
  SAVED_LDFLAGS="$LDFLAGS"
  SAVED_LIBS="$LIBS"
  LDFLAGS=
  LIBS=-lpthread
  AC_TRY_RUN(
    [
#include <semaphore.h>
int main(void){sem_t s;return sem_init(&s,0,0);}
    ],
    [
      AC_MSG_RESULT([yes])
      working_sem_init="yes"
    ],
    [
      AC_MSG_RESULT([no])
      working_sem_init="no"
    ]
  )
  LDFLAGS="$SAVED_LDFLAGS"
  LIBS="$SAVED_LIBS"
fi

if test "$missing_sem_init" = "yes" ; then
  AC_CHECK_FUNCS([sem_open], [], [AC_MSG_ERROR([Neither 'sem_init' nor 'sem_open' were found.])])
else
  if test "$working_sem_init" = "no" ; then
    AC_CHECK_FUNCS([sem_open], [], [AC_MSG_ERROR(['sem_init' is broken and 'sem_open' not found.])])
  else
    AC_DEFINE_UNQUOTED([HAVE_SEM_INIT], [1], [Define if sem_init exists.])
  fi
fi

# ahora mismo están hardcodec, pero hay que cambiarlas en un futuro

AC_DEFINE_UNQUOTED(F77_NAME_LOWER_USCORE, 1, [¿¿¿¿¿¿¿¿??????????])


# Files to generate via autotools (.am or .in source files).
AC_CONFIG_FILES([Makefile])

# Generate the final Makefile etc.
AC_OUTPUT

echo \
"------------------------------------------------------------------------------
------------------------------------------------------------------------------
 ${PACKAGE_NAME} version ${PACKAGE_VERSION}

 General Options

  Prefix . . . . . . .: '${prefix}'
  Compiler . . . . . .: '${CC} ${CFLAGS}
  Only azequia . . . .: '${disable_azqmpi}'
  Device . . . . . . .: '${devicedir}'
  Sync Model . . . . .: '${with_model}'
  Use hwloc . . . . . : '${use_hwloc}'
  Hwloc path . . . . .: '${hwloc_root}'
  Use Fastboxes . . . : '${use_fastboxes}'
  Profiling interface : '${use_profiling_interface}'

 Debug Options

  Enabled......: '${debug_mods}'
  Disabled.....: '${n_debug_mods}'

 Now type 'make @<:@<target>@:>@', where the optional <target> is:

  all        - build library
  install    - install everything
  dist       - create the distribution tarball
  check      - build, but not execute, all tests
------------------------------------------------------------------------------
------------------------------------------------------------------------------"

  #Use osi......: '${use_osi}'
  #Osi path.....: '${osi_root}'
