dnl
dnl Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
dnl                         University Research and Technology
dnl                         Corporation.  All rights reserved.
dnl Copyright (c) 2004-2005 The University of Tennessee and The University
dnl                         of Tennessee Research Foundation.  All rights
dnl                         reserved.
dnl Copyright (c) 2004-2006 High Performance Computing Center Stuttgart, 
dnl                         University of Stuttgart.  All rights reserved.
dnl Copyright (c) 2004-2005 The Regents of the University of California.
dnl                         All rights reserved.
dnl Copyright (c) 2008-2009 Cisco Systems, Inc.  All rights reserved.
dnl Copyright (c) 2010      Oracle and/or its affiliates.  All rights reserved.
dnl $COPYRIGHT$
dnl 
dnl Additional copyrights may follow
dnl 
dnl $HEADER$
dnl



######################################################################
#
# FIND_ARCH
#
# Find current architecture
#

# "host" vble. is available through AC_CANONICAL_HOST (in configure.ac)
# "ac_cv_sizeof_long" is available through AC_CHECK_SIZEOF(long) (in configure.ac)

# USAGE:
#   FIND_ARCH
#

# Results:
#   IA32 -  Intel Architecture 32-bits
#   AMD64 - Intel/AMD architecture 64-bits
#   IA64 -  Itanium architecture (64-bits)
#  Rest are not supported.

######################################################################
AC_DEFUN([FIND_ARCH],[


PWD=`pwd`

arch="UNSUPPORTED"

    AC_MSG_CHECKING([for the architecture])

    case "$host" in

        i?86-*|x86_64*)

            if test "$ac_cv_sizeof_long" = "4" ; then
                arch="IA32"
            else
                arch="AMD64"
            fi
            ;;

        ia64-*)

            arch="IA64"
            ;;

        *)
            AC_MSG_ERROR([Architecture <$host> UNSUPPORTED])
            ;;

    esac

    AC_MSG_RESULT($arch)

    AC_DEFINE_UNQUOTED([AZQMPI_ARCH], [$arch],
                       [Architecture detected])

])


######################################################################
#
# FIND_OS
#
# Find operating system name

# "host" vble. is available through AC_CANONICAL_HOST (in configure.ac)

# USAGE:
#   FIND_OS()
#
# Results:
#   DARWIN - Mac OS X operating system
#   LINUX  - Linux operating system
#  Rest are not supported.

######################################################################
AC_DEFUN([FIND_OS],[

osystem="UNSUPPORTED"

    AC_MSG_CHECKING([for the Operating System name])

    case "$host_os" in

        *darwin*)
            osystem="DARWIN"
	    ;;

        *linux*)
            osystem="LINUX"
	    ;;

        *)  
	    AC_MSG_ERROR([Operating System <$host_os> UNSUPPORTED])
            exit
	    ;;

    esac

    AC_MSG_RESULT($osystem)

    AC_DEFINE_UNQUOTED(AZQMPI_OS, [$osystem],[Operating System name])

])



######################################################################
#
# AZQMPI_CONFIG_ARCH
#
# Detect the operating system namearchitecture and the target for compiling
#
# USAGE:
#   AZQMPI_CONFIG_ARCH()
#
######################################################################
AC_DEFUN([AZQMPI_CONFIG_ARCH],[

    AC_REQUIRE([FIND_OS])
    AC_REQUIRE([FIND_ARCH])

])


