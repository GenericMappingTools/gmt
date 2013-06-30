dnl -----------------------------------------------------------------
dnl	$Id: configure.ac 18 2013-01-04 00:55:26Z pwessel $
dnl
dnl     This program is free software; you can redistribute it and/or modify
dnl     it under the terms of the GNU General Public License as published by
dnl     the Free Software Foundation; version 2 of the License.
dnl
dnl     This program is distributed in the hope that it will be useful,
dnl     but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl     GNU General Public License for more details.
dnl
dnl     Contact info: The Developer, Institution, you@address
dnl ----------------------------------------------------------------------------
dnl
dnl Run GNU autoconf to produce a configure script from this input.
dnl -----------------------------------------------------------------
dnl
dnl
dnl -----------------------------------------------------------------
dnl Since User Supplements require GMT5 we insist that the GMT5
dnl executables must be in the current path; otherwise abort.
dnl 
dnl We also need to gather the following information
dnl 1. Location of GMT5 include and library files
dnl 2. Location of netCDF library
dnl 3. Location of GDAL library
dnl -----------------------------------------------------------------
dnl
AC_INIT(GMT_APP, 5, youl@address)
dnl
dnl -----------------------------------------------------------------
dnl The remainder should need no editing
dnl -----------------------------------------------------------------
dnl
AC_CONFIG_SRCDIR(SIO_GMT_io.c)
AC_CONFIG_AUX_DIR(`pwd`)
AC_CYGWIN
AC_CANONICAL_SYSTEM
AC_LANG_C
AC_PROG_CC
AC_PROG_CPP
AC_PREFIX_DEFAULT(`pwd`)
AC_PATH_XTRA
dnl
dnl -----------------------------------------------------------------
dnl GMT5SAR-specific make macros
dnl -----------------------------------------------------------------
dnl
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LD_OPT)
AC_SUBST(LDFLAGS)
AC_SUBST(GMT_LIB)
AC_SUBST(GMT_INC)
AC_SUBST(LIBS)
AC_SUBST(LIBEXT)
AC_SUBST(SL)
AC_SUBST(SL_VERSION)
AC_SUBST(FFT)
AC_SUBST(OS)
AC_SUBST(rootdir)
dnl
dnl -----------------------------------------------------------------
dnl Special configure options for GMT_APP installation
dnl -----------------------------------------------------------------
dnl
AC_ARG_ENABLE(gmt,      [  --enable-gmt=dir        Directory with GMT5 [[auto]]])
AC_ARG_WITH(gmt_config, [  --with-gmt-config=      Full path to gmt-config [[auto]]])
AC_ARG_ENABLE(netcdf,   [  --enable-netcdf=dir     Directory with netCDF [[auto]]])
AC_ARG_WITH(nc_config,  [  --with-nc-config=       Full path to nc-config [[auto]]])
AC_ARG_ENABLE(debug,    [  --enable-debug          Compile for debugging instead of optimizing code])
AC_ARG_ENABLE(devdebug, [  --enable-devdebug       Extra debugging for developers])
AC_ARG_ENABLE(gdal,     [  --enable-gdal           Compile optional, experimental support for grid input via GDAL [[default=no]]], [], [enable_gdal=no])
AC_ARG_WITH(gdal_config,[  --with-gdal-config=     Full path to gdal-config [[auto]]], [], [with_gdal_config=no])
AC_ARG_ENABLE(rpath,	[  --disable-rpath         Do not hardcode runtime library paths])
AC_ARG_VAR(GMT_INC,Location of GMT5 headers (compile-time))
AC_ARG_VAR(GMT_LIB,Location of GMT5 library (compile-time))
dnl
dnl -----------------------------------------------------------------
dnl Check for ar, csh/bash, ld, awk, and ranlib
dnl -----------------------------------------------------------------
dnl
AC_CHECK_PROG(GMT_in_path, gmtvector, 1, 0)
if test "$GMT_in_path" = "X0" ; then
	AC_MSG_RESULT(GMT5 executables are not in your path - aborting)
	exit 1
fi
AC_CHECK_PROGS(PAGER, less more, cat)
AC_CHECK_PROG(AR, ar, ar, :)
AC_CHECK_PROG(CSH, csh, csh, bash, )
AC_CHECK_PROG(LD, ld, ld, :)
AC_CHECK_PROG(AS, as, as, :)
AC_PROG_LN_S
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_RANLIB
dnl
dnl -----------------------------------------------------------------
dnl Get current OS and chip type
dnl -----------------------------------------------------------------
dnl
os=`uname -s`
if test "X$os" = "XDarwin" ; then
	osr=`uname -r | cut -d. -f1`
fi
chip=`uname -p`
Sun_rev=`uname -r | grep '^5.'`
rootdir=`pwd`
dnl
dnl -----------------------------------------------------------------
dnl Check for libraries.  Most installs will need to have
dnl -lm but under OS X it is not needed.  Thus, we look
dnl for sin in -lm which should set -lm on most platforms.
dnl We also look for sincos in -m and -lsunmath for Solaris.
dnl -----------------------------------------------------------------
dnl
AC_SEARCH_LIBS(sin, m)
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-gmt to set GMT5 dir
dnl However, we use gmt-config if possible
dnl -----------------------------------------------------------------
dnl
if test "X$with_gmt_config" != "Xno" ; then
	AC_PATH_PROG([GMT_CONF], [gmt-config], [$with_gmt_config])
fi
if test -x "$GMT_CONF" && test "X$GMT_INC" = "X" && test "X$GMT_LIB" = "X" ; then
	AC_MSG_CHECKING(for GMT includes)
	GMT_INC=`$GMT_CONF --cflags`
	AC_MSG_RESULT($GMT_INC)
	AC_MSG_CHECKING(for GMT libs)
	GMT_LIB=`$GMT_CONF --libs`
	AC_MSG_RESULT($GMT_LIB)
	GMT_LIB_PATH=`echo $GMT_LIB | sed 's/^-L//;s/\ .*//'`
	rpath="$GMT_LIB_PATH"
	AC_MSG_CHECKING(for 32/64-bit GMT5 installation)
	bits=`$GMT_CONF --bits`
	GCC_64="-m${bits}"
	LDFLAGS="$LDFLAGS -m${bits}"
	AC_MSG_RESULT(found ${bits}-bit)
else
	if test "X$enable_gmt" = "X" || test "X$enable_gmt" = "Xyes" ; then
		AC_PATH_PROG([GMT], [gmtvector], [/usr/local/gmt/bin/gmtvector])
		GMT=`dirname $GMT`
	else
		GMT=$enable_gmt
	fi
	AC_MSG_CHECKING(for GMT5 include directory)
	if test "X$GMT_INC" = "X"; then
		GMT_INC=$GMT/include
	fi
	AC_MSG_RESULT($GMT_INC)
	if test ! -f $GMT_INC/gmt.h; then
		AC_MSG_WARN([[gmt.h can not be found in $GMT_INC]])
	fi
	GMT_INC="-I$GMT_INC"
	AC_MSG_CHECKING(for GMT5 library directory)
	if test "X$GMT_LIB" = "X"; then
		GMT_LIB=$GMT/lib
	fi
	AC_MSG_RESULT($GMT_LIB)
	if test ! -d $GMT_LIB; then
		AC_MSG_WARN([[No such directory: $GMT_LIB]])
	fi
	rpath="$GMT_LIB"
	GMT_LIB="-L$GMT_LIB -lgmt"
dnl 	Determine if GMT5 was installed 32- or 64-bit
	AC_MSG_CHECKING(for 32/64-bit GMT5 installation)
	answer=`gmtvector -^ 2>&1 | awk '{if (NR == 1) print substr($3,2,6)}'`
	if test "X$answer" = "X[64-bit]" ; then
		AC_MSG_RESULT(found 64-bit)
		GCC_64="-m64"
		LDFLAGS="$LDFLAGS -m64"
	elif test "X$answer" = "X[32-bit]" ; then
		GCC_64="-m32"
		LDFLAGS="$LDFLAGS -m32"
		AC_MSG_RESULT(found 32-bit)
	fi
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-debug to pass -g flag
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(if we should compile with debugging switch -g)
CFLAGS=$ac_save_CFLAGS
if test "X$enable_debug" = "Xyes" ; then
	AC_MSG_RESULT(yes)
	CFLAGS="$CFLAGS -g"
else
	CFLAGS=`echo $CFLAGS | sed 's/-g//'`
	if test "$os" = "Darwin" ; then
		LDFLAGS="$LDFLAGS -Wl,-x"
	else
		LDFLAGS="$LDFLAGS -s"
	fi
	if test "$GCC" = "yes" || test "$os" = "Linux" || test "$os" = "AIX"; then
		CFLAGS="$CFLAGS -O2"
	elif test "$chip" = "alpha" || test "$os" = "AUX" || test "$os" = "HP-UX"; then
		CFLAGS="$CFLAGS"
	else
		CFLAGS="$CFLAGS -O"
	fi
	AC_MSG_RESULT(no)
fi
dnl
dnl -----------------------------------------------------------------
dnl Determine LD_OPT for this platform
dnl -----------------------------------------------------------------
dnl
shared_unknown=no
LD_OPT=
LD=ld
SL=so
LIBEXT='$(SL)'
SL_VERSION='$(SL)'
if test "X$enable_shared" = "Xyes" ; then
	AC_MSG_CHECKING(how to make a shared object)
	if test "X$CYGWIN" = "Xyes" ; then
		SL=dll.a
		SL_VERSION=$LIB_MAJOR_VERSION.$SL
		LD='$(CC)'
		LD_OPT="-shared"
	elif test "$os" = "SunOS" ; then
		if test "$Sun_rev" = "" ; then	# SunOS
			LD_OPT="-Bdynamic"
		else				# Solaris
			LD_OPT="-G"
		fi
	elif test "$os" = "Linux" || test "$os" = "GNU/kFreeBSD" ; then	# Need optimization when doing shared
		LD='$(CC)'
		SL_VERSION=$SL.$LIB_MAJOR_VERSION
		LD_OPT='-shared -Wl,-soname=$*.'$SL_VERSION
	elif test "$os" = "FreeBSD" || test "$os" = "NetBSD" ; then
		SL_VERSION=$SL.$LIB_MAJOR_VERSION
		LD_OPT='-shared -Wl,-soname=$*.'$SL_VERSION
	elif test "$os" = "OSF1" ; then
		LD_OPT='-shared -expect_unresolved "*"'
	elif test "$os" = "IRIX64" ; then
		LD_OPT="-shared"
		# Need to remove -L/usr/lib to avoid mixing o32 and n32
		X_LIBS=
	elif test "$os" = "Darwin" ; then	# MacOS X
		SL=dylib
		SL_VERSION=$LIB_MAJOR_VERSION.$SL
		LD='$(CC)'
		LD_OPT='-install_name ${prefix}/lib/$*.'$SL_VERSION' -compatibility_version 4.0.0 -current_version '$LIB_VERSION' -dynamiclib '$OSX_L
		if test $osr -eq 8 ; then	# Tiger needs special option for shared libs
			CFLAGS="$CFLAGS -fno-common"
		fi
	elif test "$os" = "HP-UX" ; then
		LD_OPT="-b"
		SL=sl
	else	# Not yet implemented
		AC_MSG_RESULT(not supported for GMT5SAR under $os - building static libs instead)
		enable_shared=no
		shared_unknown=yes
	fi
fi
if test "X$enable_shared" = "Xyes" ; then
	rpath="$rpath"':$(libdir)'
	AC_MSG_RESULT($LD $LD_OPT)
	AC_MSG_CHECKING(the file extension for shared libraries)
	AC_MSG_RESULT($SL)
else
	LIBEXT='a'
fi
dnl
dnl -----------------------------------------------------------------
dnl Determine LDFLAGS for this platform
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(linker options)
flag=
if test "X$enable_rpath" = "Xno" ; then
	flag=
elif test "$os" = "SunOS" ; then
	if test "$Sun_rev" = "" ; then
		flag="-L"
	else
		flag="-R,"
	fi
elif test "$os" = "Linux" || test "$os" = "FreeBSD" || test "$os" = "NetBSD" || test "$os" = "OSF1" || test "$os" = "IRIX64" ; then
	flag="-rpath,"
elif test "$os" = "HP-UX" ; then
	flag="+b,"
fi
if test "X$flag" != "X" ; then
	LDFLAGS="$LDFLAGS -Wl,$flag$rpath"
fi
AC_MSG_RESULT($LDFLAGS)
dnl
dnl -----------------------------------------------------------------
dnl Determine CFLAGS for this platform
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(compiler options for GMT)
if test "$GCC" = "yes"; then		# GNU cc options
	if test "$chip" = "alpha" ; then	# Alpha PC
		CFLAGS="$CFLAGS -mieee -Wall $GCC_64"
	else				# MacOSX, Linux, ...
		CFLAGS="$CFLAGS -Wall $GCC_64"
	fi
	CFLAGS="$CFLAGS -fPIC -fno-strict-aliasing -std=c99"
elif test "$os" = "SunOS" ; then	# Sun Workshop CC options
	CFLAGS="$CFLAGS -Xc"
elif test "$os" = "HP-UX" ; then	# HP cc options
	CFLAGS="$CFLAGS -Aa +Ofltacc +O1 +Olibcalls"
	if test "$enable_shared" = "yes" ; then
		CFLAGS="$CFLAGS +z"
	fi
elif test "$os" = "OSF1" ; then		# Dec Alpha OSF cc options
	CFLAGS="$CFLAGS -Olimit 1500 -ieee_with_inexact"
elif test "$os" = "ULTRIX" ; then	# Dec Ultrix cc options
	CFLAGS="$CFLAGS -Olimit 1500"
fi
AC_MSG_RESULT($CFLAGS)
dnl
dnl -----------------------------------------------------------------
dnl Determine CPPFLAGS for this platform
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(compiler preprocessor flags for GMT5SAR)
if test "$os" = "AUX" ; then		# Apple A/UX cc options
	CPPFLAGS="$CPPFLAGS -Daux"
fi
AC_MSG_RESULT($CPPFLAGS)
dnl
dnl -----------------------------------------------------------------
dnl Check if we have GNU make. If not, add CPPFLAGS to CFLAGS
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(if CPPFLAGS need to be added to CFLAGS)
${MAKE:-make} -p -f /dev/null 2>&1 | grep CPPFLAGS > /dev/null
if test "$?" = "0" ; then
	AC_MSG_RESULT(no)
else
	CFLAGS="$CFLAGS "'$(CPPFLAGS)'
	AC_MSG_RESULT(yes)
fi
dnl
dnl -----------------------------------------------------------------
dnl Temporary: Set OS to be the architecture
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for architecture)
OS=`arch`
AC_MSG_RESULT(use $OS)
dnl
dnl -----------------------------------------------------------------
dnl Update the Make macros
dnl -----------------------------------------------------------------
dnl
AC_OUTPUT(config.mk)
dnl
