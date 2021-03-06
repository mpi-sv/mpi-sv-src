dnl **************************************************************************
dnl * Initialize
dnl **************************************************************************
AC_INIT([[KLEE]],[[0.01]],[daniel@minormatter.com])

dnl Identify where LLVM source tree is (this is patched by
dnl AutoRegen.sh)
LLVM_SRC_ROOT=XXX

dnl Tell autoconf that the auxilliary files are actually located in
dnl the LLVM autoconf directory, not here.
AC_CONFIG_AUX_DIR($LLVM_SRC_ROOT/autoconf)

dnl Tell autoconf that this is an LLVM project being configured
dnl This provides the --with-llvmsrc and --with-llvmobj options
LLVM_CONFIG_PROJECT("","")

dnl Verify that the source directory is valid
AC_CONFIG_SRCDIR(["Makefile.config.in"])

dnl Configure a common Makefile
AC_CONFIG_FILES(Makefile.config)

dnl Configure project makefiles
dnl List every Makefile that exists within your source tree
AC_CONFIG_HEADERS([include/klee/Config/config.h])

dnl FIXME: Make out of tree builds work.

AC_LANG([C++])

dnl **************************************************************************
dnl Find the host

AC_CANONICAL_TARGET

dnl Determine the platform type and cache its value. This helps us configure
dnl the System library to the correct build platform.
AC_CACHE_CHECK([type of operating system we're going to host on],
               [klee_cv_os_type],
[case $host in
  *-*-linux*)
    host_supports_posix_runtime=yes ;;
  *)
    host_supports_posix_runtime=no ;;
esac])

dnl **************************************************************************
dnl Verify that we can find llvm

dnl --with-llvm is a shortcut for setting srcdir and objdir.
AC_ARG_WITH(llvm,
  AS_HELP_STRING([--with-llvm],
    [Location of LLVM Source and Object code]),,)

AC_MSG_CHECKING([llvm source dir])

if test X${with_llvm} != X; then
    dnl Verify that --with-llvm{src,obj} were not given.
    if test X${with_llvmsrc} != X; then
       AC_MSG_ERROR([--with-llvmsrc cannot be specified when using --with-llvm])
    fi   
    if test X${with_llvmobj} != X; then
       AC_MSG_ERROR([--with-llvmobj cannot be specified when using --with-llvm])
    fi   
    with_llvmsrc=$with_llvm
    with_llvmobj=$with_llvm
fi

dnl If one of with_llvmsrc or with_llvmobj was given, we must have both.
if (test X${with_llvmsrc} != X || test X${with_llvmobj} != X); then
    dnl Verify that with_llvmobj was given as well.
    if test X${with_llvmsrc} = X; then
       AC_MSG_ERROR([--with-llvmsrc must be specified when using --with-llvmobj])
    fi      
    if test X${with_llvmobj} = X; then
       AC_MSG_ERROR([--with-llvmobj must be specified when using --with-llvmsrc])
    fi      
else
    dnl Otherwise try and use llvm-config to find.
    llvm_version=`llvm-config --version`
    if test X${llvm_version} = X; then
       AC_MSG_ERROR([unable to find llvm, use --with-llvmsrc and --with-llvmobj])
    fi
    
    with_llvmsrc=`llvm-config --src-root`
    with_llvmobj=`llvm-config --obj-root`
fi

dnl Try to validate directories.
if test ! -f ${with_llvmsrc}/Makefile.rules; then
   AC_MSG_ERROR([invalid llvmsrc directory: ${with_llvmsrc}])
fi
if test ! -f ${with_llvmobj}/Makefile.config; then
   AC_MSG_ERROR([invalid llvmobj directory: ${with_llvmobj}])
fi

dnl Make the paths absolute.
llvm_src=`cd $with_llvmsrc 2> /dev/null; pwd`
llvm_obj=`cd $with_llvmobj 2> /dev/null; pwd`
   
AC_MSG_RESULT([$llvm_src])

dnl Report obj dir as well.
AC_MSG_CHECKING([llvm obj dir])
AC_MSG_RESULT([$llvm_obj])

AC_SUBST(LLVM_SRC,$llvm_src)
AC_SUBST(LLVM_OBJ,$llvm_obj)

dnl Determine LLVM version.
AC_MSG_CHECKING([llvm package version])
llvm_package_version=`grep PACKAGE_VERSION= $with_llvmsrc/configure | cut -d\' -f 2`
AC_MSG_RESULT([$llvm_package_version])

llvm_version_split=`python -c "import re; print '\t'.join(map(str, re.match('([[0-9]]+)[.]([[0-9]]+)(svn)?', \"$llvm_package_version\").groups()))"`

AC_MSG_CHECKING([llvm version major])
llvm_version_major=`echo "$llvm_version_split" | cut -f 1`
AC_MSG_RESULT([$llvm_version_major])

AC_MSG_CHECKING([llvm version minor])
llvm_version_minor=`echo "$llvm_version_split" | cut -f 2`
AC_MSG_RESULT([$llvm_version_minor])

AC_MSG_CHECKING([llvm is release version])
llvm_version_svn=`echo "$llvm_version_split" | cut -f 3`
if test "$llvm_version_svn" == "svn"; then
  llvm_is_release=0
else
  llvm_is_release=1
fi
AC_MSG_RESULT([$llvm_is_release])

AC_DEFINE_UNQUOTED(LLVM_VERSION_MAJOR, $llvm_version_major, [LLVM major version number])
AC_SUBST(LLVM_VERSION_MAJOR,$llvm_version_major)
AC_DEFINE_UNQUOTED(LLVM_VERSION_MINOR, $llvm_version_minor, [LLVM minor version number])
AC_SUBST(LLVM_VERSION_MINOR,$llvm_version_minor)
AC_DEFINE_UNQUOTED(LLVM_IS_RELEASE, $llvm_is_release, [LLVM version is release (instead of development)])
AC_SUBST(LLVM_IS_RELEASE,$llvm_is_release)

dnl **************************************************************************
dnl User option to enable uClibc support.

AC_ARG_WITH(uclibc,
  AS_HELP_STRING([--with-uclibc],
    [Enable use of the klee uclibc at the given path]),,)

dnl If uclibc wasn't given, check for a uclibc in the current
dnl directory.
if (test X${with_uclibc} = X && test -d uclibc); then
   with_uclibc=uclibc
fi

dnl Validate uclibc if given.

AC_MSG_CHECKING([uclibc])
if (test X${with_uclibc} != X); then
   if test ! -d ${with_uclibc}; then
       AC_MSG_ERROR([invalid uclibc directory: ${with_uclibc}])
   fi

   dnl Make the path absolute
   with_uclibc=`cd $with_uclibc 2> /dev/null; pwd`

   AC_MSG_RESULT([$with_uclibc])
else
   AC_MSG_RESULT([no])
fi

AC_DEFINE_UNQUOTED(KLEE_UCLIBC, "$with_uclibc", [Path to KLEE's uClibc])
AC_SUBST(KLEE_UCLIBC)         

if test X${with_uclibc} != X ; then
  AC_SUBST(ENABLE_UCLIBC,[[1]])
else
  AC_SUBST(ENABLE_UCLIBC,[[0]])
fi


AC_MSG_CHECKING([uclibc])
if (test X${with_uclibc} != X); then
   if test ! -d ${with_uclibc}; then
       AC_MSG_ERROR([invalid uclibc directory: ${with_uclibc}])
   fi

   dnl Make the path absolute
   with_uclibc=`cd $with_uclibc 2> /dev/null; pwd`

   AC_MSG_RESULT([$with_uclibc])
else
   AC_MSG_RESULT([no])
fi

AC_DEFINE_UNQUOTED(KLEE_UCLIBC, "$with_uclibc", [Path to KLEE's uClibc])
AC_SUBST(KLEE_UCLIBC)         

if test X${with_uclibc} != X ; then
  AC_SUBST(ENABLE_UCLIBC,[[1]])
else
  AC_SUBST(ENABLE_UCLIBC,[[0]])
fi

dnl *************************************************************************


AC_ARG_WITH(libMPI,
  AS_HELP_STRING([--with-libMPI],
    [Enable use of the libMPI at the given path]),enableval=default)

dnl Validate libMPI if given.

AC_MSG_CHECKING([libMPI])
if (test X${with_libMPI} != X); then
   if test ! -d ${with_libMPI}; then
       AC_MSG_ERROR([invalid libMPI directory: ${with_libMPI}])
   fi

   dnl Make the path absolute
   with_libMPI=`cd $with_libMPI 2> /dev/null; pwd`

   AC_MSG_RESULT([$with_libMPI])
else
   AC_MSG_RESULT([no])
fi        

AC_DEFINE_UNQUOTED(KLEE_LIBMPI, "$with_libMPI", [Path to LIBMPI])
AC_SUBST(KLEE_LIBMPI)  

if test X${with_libMPI} != X ; then
  AC_SUBST(ENABLE_LIBMPI,[[1]])
else
  AC_SUBST(ENABLE_LIBMPI,[[0]])
fi

dnl **************************************************************************
dnl User option to enable the POSIX runtime

AC_ARG_ENABLE(posix-runtime,
              AS_HELP_STRING([--enable-posix-runtime],
                             [Enable the POSIX runtime]),
                             ,enableval=default)

AC_MSG_CHECKING([POSIX runtime])
if test ${enableval} = "default" ; then
  if test X${with_uclibc} != X; then
    enableval=$host_supports_posix_runtime
    if test ${enableval} = "yes"; then
      AC_MSG_RESULT([default (enabled)])
    else
      AC_MSG_RESULT([default (disabled, unsupported target)])
    fi
  else
    enableval="no"
    AC_MSG_RESULT([default (disabled, no uclibc)])
  fi
else
  if test ${enableval} = "yes" ; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
fi

if test ${enableval} = "yes" ; then
  AC_SUBST(ENABLE_POSIX_RUNTIME,[[1]])
else
  AC_SUBST(ENABLE_POSIX_RUNTIME,[[0]])
fi

dnl **************************************************************************
dnl User option to select runtime version

AC_ARG_WITH(runtime,
  AS_HELP_STRING([--with-runtime],
    [Select build configuration for runtime libraries (default [Release])]),,
    withval=default)

if test X"${withval}" = Xdefault; then
   with_runtime=Release+Asserts
fi

AC_MSG_CHECKING([runtime configuration])
if test X${with_runtime} = XRelease; then
    AC_MSG_RESULT([Release])
    AC_SUBST(RUNTIME_ENABLE_OPTIMIZED,[[1]])
elif test X${with_runtime} = XDebug; then
   AC_MSG_RESULT([Debug])
   AC_SUBST(RUNTIME_ENABLE_OPTIMIZED,[[0]])
elif test X${with_runtime} = XRelease+Asserts; then
    AC_MSG_RESULT([Release+Asserts])
    AC_SUBST(RUNTIME_ENABLE_OPTIMIZED,[[1]])
elif test X${with_runtime} = XDebug+Asserts; then
    AC_MSG_RESULT([Debug+Asserts])
    AC_SUBST(RUNTIME_ENABLE_OPTIMIZED,[[0]])
else
   AC_MSG_ERROR([invalid configuration: ${with_runtime}])
fi

AC_DEFINE_UNQUOTED(RUNTIME_CONFIGURATION, "$with_runtime", [Configuration for runtime libraries])
AC_SUBST(RUNTIME_CONFIGURATION)

dnl **************************************************************************
dnl See if we should support __ctype_b_loc externals.

dnl FIXME: Do the proper test if we continue to need this.
case $host in
  *-*-linux*)
    AC_DEFINE_UNQUOTED(HAVE_CTYPE_EXTERNALS, 1, [Does the platform use __ctype_b_loc, etc.])
esac

dnl **************************************************************************
dnl Checks for header files.

dnl NOTE: This is mostly just to force autoconf to make CFLAGS defines
dnl for us.
AC_LANG_PUSH([C])

AC_CHECK_HEADERS([sys/acl.h])

AC_LANG_POP([C])

AC_CHECK_HEADERS([selinux/selinux.h],
        AC_SUBST(HAVE_SELINUX, 1),
        AC_SUBST(HAVE_SELINUX, 0))

AC_ARG_WITH(stp,
  AS_HELP_STRING([--with-stp],
    [Location of STP installation directory]),,)

if test X$with_stp = X ; then
  AC_SUBST(ENABLE_EXT_STP,[[0]])
else
  stp_root=`cd $with_stp 2> /dev/null; pwd`

  old_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS -I$stp_root/include"
  AC_CHECK_HEADER(stp/c_interface.h,, [
         AC_MSG_ERROR([Unable to use stp/c_interface.h header])
  ])
  CPPFLAGS="$old_CPPFLAGS"

  AC_CHECK_LIB(stp, vc_setInterfaceFlags,, [
         AC_MSG_ERROR([Unable to link with libstp])
  ], -L$stp_root/lib)

  AC_DEFINE(HAVE_EXT_STP, 1, [Using external STP])
  AC_SUBST(ENABLE_EXT_STP,[[1]])
  AC_SUBST(STP_ROOT,$stp_root)
fi

dnl **************************************************************************
dnl * Create the output files
dnl **************************************************************************

dnl Do special configuration of Makefiles
AC_CONFIG_MAKEFILE(Makefile)
AC_CONFIG_MAKEFILE(Makefile.common)
AC_CONFIG_MAKEFILE(lib/Makefile)
AC_CONFIG_MAKEFILE(runtime/Makefile)
AC_CONFIG_MAKEFILE(stp/Makefile)
AC_CONFIG_MAKEFILE(test/Makefile)
AC_CONFIG_MAKEFILE(test/Makefile.tests)
AC_CONFIG_MAKEFILE(tools/Makefile)
AC_CONFIG_MAKEFILE(unittests/Makefile)

dnl This must be last
AC_OUTPUT
