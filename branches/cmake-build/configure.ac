dnl -----------------------------------------------------------------
dnl $Id$
dnl 	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
dnl 	See LICENSE.TXT file for copying and redistribution conditions.
dnl
dnl 	This program is free software; you can redistribute it and/or modify
dnl 	it under the terms of the GNU General Public License as published by
dnl 	the Free Software Foundation; version 2 or any later version.
dnl
dnl 	This program is distributed in the hope that it will be useful,
dnl 	but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl 	GNU General Public License for more details.
dnl
dnl 	Contact info: gmt.soest.hawaii.edu
dnl -------------------------------------------------------------------------------
AC_REVISION($Revision$)
dnl
dnl Run GNU autoconf to produce a configure script from this input.
dnl -----------------------------------------------------------------
dnl
dnl
dnl -----------------------------------------------------------------
dnl First specify the GMT and GSHHS version and release date
dnl -----------------------------------------------------------------
dnl Below, set the VERSION to be created.
dnl Also, fix the MANDATE and DOCDATE values.
dnl For final version set SVN_VERSION to 0
dnl
AC_INIT(GMT, 5.0.0, gmt-help@lists.hawaii.edu)
MANDATE="1 Nov 2011"
DOCDATE="November 2011"
DOCYEAR="2011"
GSHHS_VERSION=2.2.0
SVN_VERSION=1
dnl see whether subversion is installed
AC_PATH_PROG([SVNVERSIONCOMMAND], [svnversion], [])
dnl use svnversion to record the current repository revision only if
dnl subversion is installed and we are in a working copy
if test "$SVN_VERSION" != "0" && test -x "$SVNVERSIONCOMMAND"; then
  SVN_VERSION_STRING=`"$SVNVERSIONCOMMAND" -n '.'`
  if test "$SVN_VERSION_STRING" == "exported"; then
    SVN_VERSION=0
  fi
fi
dnl
dnl -----------------------------------------------------------------
dnl Specify the minimal supplement directories
dnl -----------------------------------------------------------------
dnl
SUPPL="dbase gshhs imgsrc meca mgd77 misc potential segyprogs sph spotter x2sys"
dnl
dnl -----------------------------------------------------------------
dnl The remainder should need no editing
dnl -----------------------------------------------------------------
dnl
AC_CONFIG_SRCDIR(src/gmt.h)
AC_CONFIG_HEADER(src/gmt_notposix.h)
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
dnl GMT-specific make macros
dnl -----------------------------------------------------------------
dnl
AC_SUBST(UNITS)
AC_SUBST(EPS)
AC_SUBST(EXE)
AC_SUBST(MATLAB)
AC_SUBST(TXT2MAN)
AC_SUBST(TRIANGLE_D)
AC_SUBST(TRIANGLE_O)
AC_SUBST(ALPHA_SINCOS_O)
AC_SUBST(CFLAGS)
AC_SUBST(C_WFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LD_OPT)
AC_SUBST(LDFLAGS)
AC_SUBST(DSYMUTIL)
AC_SUBST(NETCDF_LIB)
AC_SUBST(NETCDF_INC)
AC_SUBST(GDAL_LIB)
AC_SUBST(GDAL_INC)
AC_SUBST(DOCDATE)
AC_SUBST(DOCYEAR)
AC_SUBST(MANDATE)
AC_SUBST(FTPSITE)
AC_SUBST(GSHHS_VERSION)
AC_SUBST(SVN_VERSION)
AC_SUBST(SVN_VERSION_STRING)
AC_SUBST(GMT_MAJOR_VERSION)
AC_SUBST(GMT_MINOR_VERSION)
AC_SUBST(GMT_RELEASE_VERSION)
AC_SUBST(GMT_VERSION_YEAR)
AC_SUBST(GMT_COMPAT)
AC_SUBST(LIBEXT)
AC_SUBST(SL)
AC_SUBST(SL_VERSION)
AC_SUBST(MEX_BLD)
AC_SUBST(MEX_LIB)
AC_SUBST(MEX_INC)
AC_SUBST(MEX_EXT)
AC_SUBST(MEX_XDIR)
AC_SUBST(MEX_MDIR)
AC_SUBST(PCRE_INC)
AC_SUBST(PCRE_LIB)
AC_SUBST(SUPPL)
AC_SUBST(rootdir)
dnl
dnl -----------------------------------------------------------------
dnl Special configure options for GMT installation
dnl -----------------------------------------------------------------
dnl
AC_ARG_ENABLE(update,   [  --enable-update=site    GMT ftp site [[ftp.soest.hawaii.edu]]])
AC_ARG_ENABLE(netcdf,   [  --enable-netcdf=dir     Directory with netCDF [[auto]]])
AC_ARG_WITH(nc_config,  [  --with-nc-config=       Full path to nc-config [[auto]]])
AC_ARG_ENABLE(matlab,   [  --enable-matlab=dir     Build GMT-matlab interface, give directory with Matlab distribution [[auto]]])
AC_ARG_ENABLE(octave,   [  --enable-octave=dir     Build GMT-octave interface, give directory with Octave distribution [[auto]] (disables Matlab interface)])
AC_ARG_ENABLE(mex_mdir, [  --enable-mex-mdir=dir   Directory in which to install GMT-octave/Matlab .m files [[auto]]])
AC_ARG_ENABLE(mex_xdir, [  --enable-mex-xdir=dir   Directory in which to install GMT-octave/Matlab mex files [[auto]]])
AC_ARG_ENABLE(debug,    [  --enable-debug          Compile for debugging instead of optimizing code])
AC_ARG_ENABLE(devdebug, [  --enable-devdebug       Extra debugging for developers])
AC_ARG_ENABLE(compat,   [  --enable-compat         Attempt backward compatibility with GMT4])
AC_ARG_ENABLE(openmp,   [  --enable-openmp         Enable parallel calculations via OpenMP multi-processing])
AC_ARG_ENABLE(fapi,     [  --enable-fapi           Include FORTRAN API bindings])
AC_ARG_ENABLE(shared,   [  --enable-shared         Build shared (dynamic) GMT libraries instead of static])
AC_ARG_ENABLE(triangle, [  --enable-triangle       Use fast, non-GPL triangulation routine by J. Shewchuk])
AC_ARG_ENABLE(US,       [  --enable-US             Choose US (inch) rather than SI (cm) units as default])
AC_ARG_ENABLE(universal,[  --enable-universal      Make universal binaries [[OS X only]]])
AC_ARG_ENABLE(64,       [  --enable-64             Force 64-bit compilation if supported, use --disable-64 to force 32-bit compilation])
AC_ARG_ENABLE(flock,    [  --enable-flock          Enable file locking on .gmtcommands4 file (not good on networked drives)])
AC_ARG_ENABLE(gdal,     [  --enable-gdal           Compile optional, experimental support for grid input via GDAL [[default=no]]], [], [enable_gdal=no])
AC_ARG_WITH(gdal_config,[  --with-gdal-config=     Full path to gdal-config [[auto]]], [], [with_gdal_config=no])
AC_ARG_WITH(fftw,       [  --enable-fftw=          Enable the use of FFTs from FFTW and append path])
AC_ARG_WITH(accelerate, [  --enable-accelerate     Use Accelerate Framework under OS/X for FFTs [[OS X only]]])
AC_ARG_WITH(perflib,    [  --enable-perflib=       Use Performance Library under Solaris for FFTs [[Solaris only]]])
AC_ARG_ENABLE(mex,      [  --disable-mex           Disable compilation of mex supplement which requires Octave/Matlab])
AC_ARG_ENABLE(rpath,	[  --disable-rpath         Do not hardcode runtime library paths])
AC_ARG_ENABLE(xgrid,    [  --disable-xgrid         Disable compilation of xgrid supplement which requires X11])
AC_ARG_ENABLE(pcre,     [  --disable-pcre          Disable support of Perl Compatible Regular Expressions (PCRE)])
AC_ARG_WITH(pcre_config,[  --with-pcre-config=     Full path to pcre-config [[auto]]], [], [with_pcre_config=no])
AC_ARG_VAR(NETCDF_INC,Location of netCDF headers (compile-time))
AC_ARG_VAR(NETCDF_LIB,Location of netCDF library (compile-time))
AC_ARG_VAR(GDAL_INC,Location of GDAL headers (compile-time))
AC_ARG_VAR(GDAL_LIB,Location of GDAL library (compile-time))
AC_ARG_VAR(PCRE_INC,Location of PCRE headers (compile-time))
AC_ARG_VAR(PCRE_LIB,Location of PCRE library (compile-time))
dnl
dnl -----------------------------------------------------------------
dnl Check for ar, csh/bash, ld, awk, and ranlib
dnl -----------------------------------------------------------------
dnl
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
dnl Checks for typedefs, structures, and compiler characteristics.
dnl -----------------------------------------------------------------
dnl
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_CHECK_TYPES([ptrdiff_t])
dnl
dnl -----------------------------------------------------------------
dnl Checks for header files.
dnl -----------------------------------------------------------------
dnl
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h float.h limits.h stddef.h sys/time.h termios.h])
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
dnl
dnl -----------------------------------------------------------------
dnl Versioning information
dnl -----------------------------------------------------------------
dnl
rootdir=`pwd`
GMT_MAJOR_VERSION=`echo $PACKAGE_VERSION | cut -c1`
GMT_MINOR_VERSION=`echo $PACKAGE_VERSION | cut -c3`
GMT_RELEASE_VERSION=`echo $PACKAGE_VERSION | cut -c5`
GMT_VERSION_YEAR=`date +%Y`
#fi
dnl
dnl -----------------------------------------------------------------
dnl Check for libraries.  Most installs will need to have
dnl -lm but under OS X it is not needed.  Thus, we look
dnl for sin in -lm which should set -lm on most platforms.
dnl We also look for sincos in -m and -lsunmath for Solaris.
dnl -----------------------------------------------------------------
dnl
AC_SEARCH_LIBS(sin, m)
AC_SEARCH_LIBS(sincos, [m sunmath], [SINCOS=1], [SINCOS=0])
dnl
dnl OSF specific check for broken sincos from Lloyd Parkes (lloyd@must-have-coffee.gen.nz)
dnl
if test "$SINCOS" = "1" ; then
	AC_MSG_CHECKING(for a broken sincos function)
	AC_TRY_RUN([int main ()
	{
		double s = 0.1, c = 0.2;
		double s1, c1;

		s1 = s; c1 = c;
		sincos (0.5, &s, &c);
		return (s == s1 || c == c1);	/* FALSE if sincos works ok */
	}
	], [ AC_MSG_RESULT(no)
	AC_DEFINE(HAVE_SINCOS)
	], [
	case $target in
		alpha*-osf*)
			AC_DEFINE(HAVE_ALPHASINCOS)
			ALPHA_SINCOS_O=alpha-sincos.o
			AC_MSG_RESULT([yes.])
			AC_MSG_RESULT([An assembly language wrapper will be used to fix sincos.]
		)
		;;
		*)
			AC_MSG_RESULT([yes.])
			AC_MSG_RESULT([I do not know how to fix sincos on this machine.])
	esac
	], [
	AC_MSG_RESULT([no sincos found.])
	])
fi
dnl
dnl -----------------------------------------------------------------
dnl Check for the availability of many ANSI-C functions.
dnl -----------------------------------------------------------------
dnl
AC_FUNC_CHOWN
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([copysign log2 log1p hypot acosh asinh atanh rint irint isnanf isnand isnan j0 j1 jn y0 y1 yn erf erfc sincos alphasincos])
AC_CHECK_FUNCS([getpwuid qsort_r strdup strtod strtok_r])
dnl -----------------------------------------------------------------
dnl Find out about byte order and if default compilation is 64-bit
dnl -----------------------------------------------------------------
dnl
AC_C_BIGENDIAN
dnl
dnl -----------------------------------------------------------------
dnl If not set already, guess TXT2MAN (preprocessor for manpages)
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for GMT manpage preprocessor)
if test "X$TXT2MAN" = "X" ; then
	if test "$GCC" = "yes"; then		# Compiler is gcc
		TXT2MAN="cpp -w -P -nostdinc"
	elif test "$os" = "SunOS" ; then	# Sun Workshop
		TXT2MAN="cc -E -Xs"
	elif test "$os" = "HP-UX" ; then	# HP
		TXT2MAN="/usr/ccs/lbin/cpp"
	else					# other?
		TXT2MAN="false"
	fi
fi
AC_MSG_RESULT($TXT2MAN)
dnl
dnl -----------------------------------------------------------------
dnl When --includedir is the @prefix@/include, make it @prefix@/include/gmt
dnl -----------------------------------------------------------------
dnl
if test "$includedir" = '${prefix}/include' ; then
	includedir='${prefix}/include/gmt'
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-64 for turning on 64-bit compilation
dnl -----------------------------------------------------------------
dnl
qsort_check=0
AC_CHECK_SIZEOF(long)
AC_MSG_CHECKING(for 64-bit compilation)
if test "X$enable_64" = "Xyes" ; then
	AC_MSG_RESULT(yes: forcing 64-bit)
	GCC_64="-m64"
	LDFLAGS="$LDFLAGS -m64"
	if test "X$os" = "XDarwin" ; then
		qsort_check=1
	fi
elif test "X$enable_64" = "Xno" ; then
	GCC_64="-m32"
	LDFLAGS="$LDFLAGS -m32"
	AC_MSG_RESULT(no: forcing 32-bit)
elif test "$ac_cv_sizeof_long" = "8" ; then
	AC_MSG_RESULT(using default: 64-bit)
	if test "X$os" = "XDarwin" ; then
		qsort_check=1
	fi
else
	AC_MSG_RESULT(using default: 32-bit)
fi
if test "$qsort_check" = "1" ; then
	AC_MSG_CHECKING(for buggy qsort under 64-bit OS X)
	if test $osr -lt 10 ; then
		# Circumvent qsort bug on OSX prior to Snow Leopard
		CPPFLAGS="$CPPFLAGS -DGMT_QSORT"
		AC_MSG_RESULT(yes: use GMT version of qsort)
	else
		AC_MSG_RESULT(no: use OS X version of qsort)
	fi
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-universal for OS X universal binaries
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for universal binaries)
if test "X$os" = "XDarwin" && test "X$enable_universal" = "Xyes" ; then
	if test "$enable_64" = "yes" ; then
		ppc=ppc64
		i386=x86_64
	else
		ppc=ppc
		i386=i386
	fi
	OSX_L="-arch $i386 -arch $ppc"
	GCC_64="$GCC_64 $OSX_L"
	if test "$chip" = "powerpc" ; then
		OSX_L="-isysroot /Developer/SDKs/MacOSX10.4u.sdk $OSX_L"
	fi
	LDFLAGS="$OSX_L"
else
	enable_universal="no"
fi
AC_MSG_RESULT($enable_universal)
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-update to set GMT ftp site for patches
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for GMT ftp site selection)
if test "X$enable_update" = "X" || test "X$enable_update" = "Xyes" ; then
	FTPSITE='ftp.soest.hawaii.edu'
else
	FTPSITE=$enable_update
fi
AC_MSG_RESULT($FTPSITE)
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-matlab to set Matlab dir or
dnl --enable-octave=yes to set up Octave compilation.
dnl Note: if Matlab|Octave is disabled, then so should Mex.
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for Octave or Matlab directory)
if test "X$enable_matlab" = "Xyes" && test "X$enable_octave" = "Xyes"; then
	AC_MSG_WARN([[Cannot configure for both Octave and Matlab - Octave selected]])
	enable_matlab="no"
fi
if test "X$enable_octave" = "Xyes" ; then
	AC_PATH_PROG([MATLAB], [octave], [${OCTAVE:-/usr/local/octave}/bin/octave])
	MATLAB=`dirname $MATLAB`
	MATLAB=`dirname $MATLAB`
	AC_MSG_RESULT([setting Octave directory... $MATLAB])
elif test "X$enable_matlab" = "Xyes" ; then
	AC_PATH_PROG([MATLAB], [matlab], [${MATLAB:-/usr/local/matlab}/bin/matlab])
	MATLAB=`dirname $MATLAB`
	MATLAB=`dirname $MATLAB`
	AC_MSG_RESULT([setting Matlab directory... $MATLAB])
elif test ! "X$enable_octave" = "X"; then
	MATLAB=$enable_octave
	AC_MSG_RESULT([setting Octave directory... $MATLAB])
elif test ! "X$enable_matlab" = "X"; then
	MATLAB=$enable_matlab
	AC_MSG_RESULT([setting Matlab directory... $MATLAB])
else
	MATLAB='NONE'
	enable_mex="no"
	AC_MSG_RESULT([[none... mex compilation disabled]])
fi
dnl
dnl -----------------------------------------------------------------
dnl Determine Octave/Matlab mex extensions and dirs for this platform
dnl -----------------------------------------------------------------
dnl
if test "X$MATLAB" = "XNONE" ; then
	AC_MSG_RESULT([Octave/Matlab mex supplement will be skipped])
elif test ! "X$enable_matlab" = "X"; then
	AC_MSG_CHECKING(Compiler options for Matlab)
	MEX_BLD="$MATLAB/bin/mex -DGMT_MATLAB"
	if test "$os" = "Linux" ; then		# Linux systems
		if test "$ac_cv_sizeof_long" = "8" ; then
			MEX_EXT='mexa64'
			MATLAB_MEX='glnxa64'
		else
			MEX_EXT='mexglx'
			MATLAB_MEX='glnx86'
		fi
		MATLAB_LIB='mat'
	elif test "$os" = "SunOS" ; then	# Sun Workshop
		MEX_EXT='mexsol'
		MATLAB_MEX='sol2'
		MATLAB_LIB='mat'
	elif test "$os" = "IRIX64" ; then	# SGI Irix 64
		MEX_EXT='mexsg64'
	elif test "$os" = "IRIX" ; then		# SGI Irix
		MEX_EXT='mexsg'
	elif test "$os" = "HP-UX" ; then	# HP
		MEX_EXT='mexhpux'
	elif test "$os" = "AIX" ; then		# IBM AIX
		MEX_EXT='mexrs6'
		MATLAB_MEX='ibm_rs'
		MATLAB_LIB='mat'
	elif test "$os" = "OSF1" ; then		# Dec Alpha
		MEX_EXT='mexds'
	elif test "$os" = "ULTRIX" ; then	# Dec Ultrix
		MEX_EXT='mexvx'
	elif test "$os" = "Darwin" ; then	# Mac OS X
		dir=`ls /Applications | grep MATLAB | head -n 1`
		if test "X$dir" = "X" ; then
			MATLAB=NONE
		else
			MATLAB=/Applications/$dir
		fi
		if test "$chip" = "i386" ; then		#Intel Mac
			if test "$ac_cv_sizeof_long" = "8" ; then
				MEX_EXT='mexmaci64'
				MATLAB_MEX='maci64'
			else
				MEX_EXT='mexmaci'
				MATLAB_MEX='maci'
			fi
		else	#Legacy PPC
			MEX_EXT='mexmac'
			MATLAB_MEX='mac'
		fi
		MATLAB_LIB='mat'
	else					# Other
		MEX_EXT='mex'
	fi
	if test "X$MATLAB_LIB" = "X" || test "X$MATLAB_MEX" = "X" ; then
		# Users: pls supply your input
		AC_MSG_RESULT([Matlab mex-file extension and Matlab library name not known - guessed to be mex])
		AC_MSG_RESULT([If incorrect, please modify makefile manually and notify gmt@soest.hawaii.edu])
		MATLAB_MEX='mex'
		MATLAB_LIB='mex'
	fi
	MEX_LIB="-L$MATLAB/extern/lib/$MATLAB_MEX -l$MATLAB_LIB"
	MEX_INC="-I$MATLAB/extern/include"
	if test "X$enable_mex_mdir" = "X" || test "X$enable_mdir" = "Xyes" ; then
		MEX_MDIR=$MATLAB/toolbox/local
	else
		MEX_MDIR=$enable_mex_mdir
	fi
	if test "X$enable_mex_xdir" = "X" || test "X$enable_xdir" = "Xyes" ; then
		MEX_XDIR=$MATLAB/toolbox/local
	else
		MEX_XDIR=$enable_mex_xdir
	fi
	AC_MSG_RESULT([$MEX_BLD $MEX_LIB, extension .$MEX_EXT])
	AC_MSG_CHECKING(Matlab install dir for .m files)
	AC_MSG_RESULT([$MEX_MDIR])
	AC_MSG_CHECKING(Matlab install dir for .mex files)
	AC_MSG_RESULT([$MEX_XDIR])
else
	AC_MSG_CHECKING(Compiler options for Octave)
	MEX_BLD='mkoctfile --mex -DGMT_OCTAVE'
	MEX_EXT='mex'
	MEX_LIB=
	MEX_INC=
	if test "X$enable_mex_mdir" = "X" || test "X$enable_mdir" = "Xyes" ; then
		MEX_MDIR=`octave-config --m-site-dir`
	else
		MEX_MDIR=$enable_mex_mdir
	fi
	if test "X$enable_mex_xdir" = "X" || test "X$enable_xdir" = "Xyes" ; then
		MEX_XDIR=`octave-config --oct-site-dir`
	else
		MEX_XDIR=$enable_mex_xdir
	fi
	AC_MSG_RESULT([$MEX_BLD, extension .$MEX_EXT])
	AC_MSG_CHECKING(Octave install dir for .m files)
	AC_MSG_RESULT([$MEX_MDIR])
	AC_MSG_CHECKING(Octave install dir for .mex files)
	AC_MSG_RESULT([$MEX_XDIR])
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-netcdf to set netCDF dir
dnl Use nc-config if possible
dnl -----------------------------------------------------------------
dnl
test -x "$with_nc_config" -a ! -d "$with_nc_config" && NC_CONF="$with_nc_config"
AC_PATH_PROG([NC_CONF], [nc-config], [], [$with_nc_config$PATH_SEPARATOR$PATH])
if test -x "$NC_CONF" && test "X$NETCDF_INC" = "X" && test "X$NETCDF_LIB" = "X" ; then
	AC_MSG_CHECKING(for netCDF includes)
	NETCDF_INC=`$NC_CONF --cflags`
	AC_MSG_RESULT($NETCDF_INC)
	AC_MSG_CHECKING(for netCDF libs)
	NETCDF_LIB=`$NC_CONF --libs`
	AC_MSG_RESULT($NETCDF_LIB)
	NC_LIB_PATH=`echo $NETCDF_LIB | sed 's/^-L//;s/\ .*//'`
	rpath="$NC_LIB_PATH"
else
	if test "X$enable_netcdf" = "X" || test "X$enable_netcdf" = "Xyes" ; then
		AC_PATH_PROG([NETCDF], [ncdump], [/usr/local/netcdf/bin/ncdump])
		NETCDF=`dirname $NETCDF`
		NETCDF=`dirname $NETCDF`
	else
		NETCDF=$enable_netcdf
	fi
	AC_MSG_CHECKING(for netCDF include directory)
	if test "X$NETCDF_INC" = "X"; then
		NETCDF_INC=$NETCDF/include
	fi
	AC_MSG_RESULT($NETCDF_INC)
	if test ! -f $NETCDF_INC/netcdf.h; then
		AC_MSG_WARN([[netcdf.h can not be found in $NETCDF_INC]])
	fi
	AC_MSG_CHECKING(for netCDF library directory)
	if test "X$NETCDF_LIB" = "X"; then
		NETCDF_LIB=$NETCDF/lib
	fi
	AC_MSG_RESULT($NETCDF_LIB)
	if test ! -d $NETCDF_LIB; then
		AC_MSG_WARN([[No such directory: $NETCDF_LIB]])
	fi
	rpath="$NETCDF_LIB"
	NETCDF_LIB="-L$NETCDF_LIB -lnetcdf"
	NETCDF_INC="-I$NETCDF_INC"
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-gdal to experimental GDAL support
dnl Set path to gdal-config explicitly with --with-gdal-config
dnl -----------------------------------------------------------------
dnl
if test "X$enable_gdal" != "Xno" || test "X$with_gdal_config" != "Xno"; then
	AC_MSG_RESULT([Compile experimental support for GDAL input])
	test -x "$with_gdal_config" -a ! -d "$with_gdal_config" && GDAL_CONF="$with_gdal_config"
	AC_PATH_PROG([GDAL_CONF], [gdal-config], [], [$with_gdal_config$PATH_SEPARATOR$PATH])
	if test -x "$GDAL_CONF" ; then
		AC_MSG_CHECKING(for GDAL includes)
		if test "X$GDAL_INC" = "X"; then
			GDAL_INC=`$GDAL_CONF --cflags`
		fi
		AC_MSG_RESULT($GDAL_INC)
		AC_MSG_CHECKING(for GDAL libs)
		if test "X$GDAL_LIB" = "X"; then
			GDAL_LIB=`$GDAL_CONF --libs`
		fi
		AC_MSG_RESULT($GDAL_LIB)
		GDAL_LIB_PATH=`echo $GDAL_LIB | sed 's/^-L//;s/\ .*//'`
		rpath="$rpath:$GDAL_LIB_PATH"
		CPPFLAGS="$CPPFLAGS -DUSE_GDAL"
	else
		AC_MSG_RESULT([The gdal-config script distributed with GDAL could not be found. If you have installed the GDAL libraries, then make sure that gdal-config is in your path. If not, use: --with-gdal-config.])
	fi
	if test "X$GDAL_INC" = "X" || test "X$GDAL_LIB" = "X"; then
		exit 1
	fi
else
	GDAL_LIB=
	GDAL_INC=
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-fftw[=dir] to set FFTW dir
dnl -----------------------------------------------------------------
dnl
if test ! "X$enable_fftw" = "X"; then
	FFTW=$enable_fftw
	AC_MSG_RESULT([Using FFTW directory $FFTW])
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-perflib[=dir] to set Performance Library dir
dnl -----------------------------------------------------------------
dnl
if test ! "X$enable_perflib" = "X"; then
	PERFLIB=$enable_perflib
	AC_MSG_RESULT([Using Performance Library directory $PERFLIB])
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-accelerate to set FFTW dir
dnl -----------------------------------------------------------------
dnl
if test "X$enable_accelerate" = "Xyes"; then
	CPPFLAGS="$CPPFLAGS -DWITH_ACCELERATE"
	LDFLAGS="$LDFLAGS -framework Accelerate"
	AC_MSG_RESULT([Using Accelerate framework])
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-flock for turning on file locking
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for file locking)
if test "X$enable_flock" = "Xyes" ; then
	CPPFLAGS="$CPPFLAGS -DFLOCK"
else
	enable_flock="no"
fi
AC_MSG_RESULT($enable_flock)
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --disable-mex for turning off mex compilation
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(if OK to compile mex)
if test ! -d src/mex; then
	AC_MSG_RESULT([[not available]])
elif test "X$enable_mex" = "Xno"; then
	AC_MSG_RESULT([[no]])
else
	SUPPL=$SUPPL" mex"
	AC_MSG_RESULT(yes)
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --disable-xgrid for turning off xgrid compilation
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(if OK to compile xgrid)
if test "X$enable_xgrid" = "Xno" || test "X$have_x" != "Xyes" ; then
	enable_xgrid="no"
else
	enable_xgrid="yes"
fi
if test ! -d src/xgrid; then
	AC_MSG_RESULT([not available])
elif test "X$enable_xgrid" = "Xno"; then
	AC_MSG_RESULT([[no]])
else
	SUPPL=$SUPPL" xgrid"
	AC_MSG_RESULT(yes)
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --disable-pcre for turning off PCRE support
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(if PCRE support is requested)
if test "X$enable_pcre" = "Xno" ; then
	AC_MSG_RESULT([[no]])
	AC_CHECK_HEADERS([regex.h], [CPPFLAGS="$CPPFLAGS -DHAVE_POSIX_ERE"])
else
	AC_MSG_RESULT([[yes]])
	test -x "$with_pcre_config" -a ! -d "$with_pcre_config" && PCRE_CONF="$with_pcre_config"
	AC_PATH_PROG([PCRE_CONF], [pcre-config], [], [$with_pcre_config$PATH_SEPARATOR$PATH])
	if test -x "$PCRE_CONF" ; then
		AC_MSG_CHECKING(for PCRE includes)
		PCRE_INC=`$PCRE_CONF --cflags`
		AC_MSG_RESULT($PCRE_INC)
		AC_MSG_CHECKING(for PCRE libs)
		PCRE_LIB=`$PCRE_CONF --libs`
		AC_MSG_RESULT($PCRE_LIB)
		CPPFLAGS="$CPPFLAGS -DHAVE_PCRE"
	else
		AC_PATH_PROG([PKG_CONF], [pkg-config])
		if test -x "$PKG_CONF" && "$PKG_CONF" libpcre ; then
			AC_MSG_CHECKING(for PCRE includes)
			PCRE_INC=`$PKG_CONF --cflags libpcre`
			AC_MSG_RESULT($PCRE_INC)
			AC_MSG_CHECKING(for PCRE libs)
			PCRE_LIB=`$PKG_CONF --libs libpcre`
			AC_MSG_RESULT($PCRE_LIB)
			CPPFLAGS="$CPPFLAGS -DHAVE_PCRE"
		else
			AC_MSG_RESULT([[Warning: Could not find either pkg-config or pcre-config. No pcre support included.]])
			AC_CHECK_HEADERS([regex.h], [CPPFLAGS="$CPPFLAGS -DHAVE_POSIX_ERE"])
		fi
	fi
fi
dnl
dnl -----------------------------------------------------------------
dnl Test if CFLAGS variable was already set
dnl If unset, AC_PROG_CC will set it to -g -O2 for gcc and leave 
dnl ac_save_CFLAGS empty. Otherwise ac_save_CFLAGS == CFLAGS.
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(whether configure should try to set CFLAGS)
if test "X$ac_save_CFLAGS" = "X" ; then
	enable_cflags_setting=yes
else
	enable_cflags_setting=no
fi
AC_MSG_RESULT($enable_cflags_setting)
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-debug to pass -g flag
dnl Note: AC_PROG_CC already sets CFLAGS to -g for all compilers
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(if we should compile with debugging switch -g)
if test "X$enable_cflags_setting" = "Xyes" ; then
	if test "X$enable_debug" = "Xyes" ; then
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
		dnl Remove -g:
		CFLAGS=`echo "$CFLAGS " | sed -e "s/-g //"`
		dnl Do not mess up gcc defaults (I suppose this can be cleaned up):
		if ! test "$GCC" = "yes" ; then
			if test "$os" = "AIX"; then
				CFLAGS="-O2 $CFLAGS"
			elif ! ( test "$chip" = "alpha" || test "$os" = "AUX" || test "$os" = "HP-UX" ); then
				CFLAGS="-O $CFLAGS"
			fi
		fi
	fi
else
	AC_MSG_RESULT(CFLAGS were already set: leaving CFLAGS untouched)
fi
dnl -----------------------------------------------------------------
dnl Allow developers to set --enable-devdebug to pass -DDEBUG
dnl -----------------------------------------------------------------
dnl
if test "X$enable_devdebug" = "Xyes" ; then
	CPPFLAGS="$CPPFLAGS -DDEBUG"
fi
dnl -----------------------------------------------------------------
dnl Allow developers to set --enable-compat to pass -DGMT_COMPAT
dnl -----------------------------------------------------------------
dnl
if test "X$enable_compat" = "Xyes" ; then
	CPPFLAGS="$CPPFLAGS -DGMT_COMPAT"
	GMT_COMPAT=""
else
	GMT_COMPAT="#"
fi
dnl -----------------------------------------------------------------
dnl Allow developers to set --enable-fapi to include Fortran bindings
dnl -----------------------------------------------------------------
dnl
if test "X$enable_fapi" = "Xyes" ; then
	CPPFLAGS="$CPPFLAGS -DFORTRAN_API"
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-triangle to use J. Shewchuk's triangulation
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(if the triangulation by Shewchuk should be used)
if test "X$enable_triangle" = "Xyes" ; then
	AC_MSG_RESULT(yes)
	TRIANGLE_D=-DTRIANGLE_D
	TRIANGLE_O=triangle.o
	TRIANGULATE=Shewchuk
else
	TRIANGULATE=Watson
	AC_MSG_RESULT(no)
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-US for US units
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for default GMT units)
if test "$enable_US" = "yes" ; then
	UNITS=US
	AC_MSG_RESULT(US (inch))
else
	UNITS=SI
	AC_MSG_RESULT(SI (cm))
fi
dnl
dnl -----------------------------------------------------------------
dnl Special checking for Cygwin since an .exe extension is used
dnl -----------------------------------------------------------------
dnl
EXE=
if test "X$CYGWIN" = "Xyes" ; then
	EXE=".exe"
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
AC_MSG_CHECKING(how to make a shared object)
if test "X$CYGWIN" = "Xyes" ; then
	SL=dll.a
	SL_VERSION=$GMT_MAJOR_VERSION.$SL
	LD='$(CC)'
	LD_OPT="-shared"
elif test "$os" = "SunOS" ; then
	if test "$Sun_rev" = "" ; then	# SunOS
		LD_OPT="-Bdynamic"
	else				# Solaris
		LD_OPT="-G"
	fi
elif test "$os" = "Linux" ; then	# Need optimization when doing shared
	LD='$(CC)'
	SL_VERSION=$SL.$GMT_MAJOR_VERSION
	LD_OPT='-shared -Wl,-soname=$*.'$SL_VERSION
elif test "$os" = "FreeBSD" || test "$os" = "NetBSD" ; then
	SL_VERSION=$SL.$GMT_MAJOR_VERSION
	LD_OPT='-shared -Wl,-soname=$*.'$SL_VERSION
elif test "$os" = "OSF1" ; then
	LD_OPT='-shared -expect_unresolved "*"'
elif test "$os" = "IRIX64" ; then
	LD_OPT="-shared"
	# Need to remove -L/usr/lib to avoid mixing o32 and n32
	X_LIBS=
elif test "$os" = "Darwin" ; then	# MacOS X
	SL=dylib
	SL_VERSION=$GMT_MAJOR_VERSION.$SL
	LD='$(CC)'
	LD_OPT='-install_name ${prefix}/lib/$*.'$SL_VERSION' -compatibility_version '$GMT_MAJOR_VERSION'.0.0 -current_version '$GMT_MAJOR_VERSION.$GMT_MINOR_VERSION.$GMT_RELEASE_VERSION' -dynamiclib '$OSX_L
	if test "X$enable_accelerate" = "Xyes"; then
		LD_OPT="$LD_OPT -framework Accelerate"
	fi
	if test $osr -eq 8 ; then	# Tiger needs special option for shared libs
		CFLAGS="$CFLAGS -fno-common"
	fi
elif test "$os" = "HP-UX" ; then
	LD_OPT="-b"
	SL=sl
elif test "$os" = "Interix" ; then
	LD='$(CC)'
	SL_VERSION=$SL.$GMT_MAJOR_VERSION
	LD_OPT="-shared"
else	# Not yet implemented
	enable_shared=no
	shared_unknown=yes
fi
if test "X$shared_unknown" = "Xyes" ; then
	AC_MSG_RESULT(not supported for GMT under $os - building static libs instead)
else
	AC_MSG_RESULT($LD $LD_OPT)
	AC_MSG_CHECKING(for file extension of shared libraries)
	AC_MSG_RESULT($SL)
	AC_MSG_CHECKING(for file extension of versioned shared libraries)
	AC_MSG_RESULT($SL_VERSION)
fi
AC_MSG_CHECKING(if shared libraries are requested)
if test "$enable_shared" = "yes" ; then
	AC_MSG_RESULT(yes)
	rpath="$rpath"':$(libdir)'
	if test "X$enable_debug" = "Xyes" && test "$os" = "Darwin"; then
		DSYMUTIL="dsymutil"
	fi
else
	AC_MSG_RESULT(no)
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
	CFLAGS="$CFLAGS -fPIC -fno-strict-aliasing"
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
AC_MSG_CHECKING(compiler preprocessor flags for GMT)
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
dnl Allow user to set --enable-openmp for turning on parallel computing with GCC
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for openMP)
if test "X$enable_openmp" = "Xyes" ; then
	CFLAGS="$CFLAGS -fopenmp"
	LDFLAGS="$LDFLAGS -fopenmp"
	if test ! "X$MEX_BLD" = "X" ; then
		MEX_BLD="$MEX_BLD -fopenmp"
	fi
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
dnl
dnl -----------------------------------------------------------------
dnl Update the GMT macros
dnl -----------------------------------------------------------------
dnl
morefiles=
if test -f doc/GMT_version.tex.in ; then
	morefiles=doc/GMT_version.tex
fi
AC_OUTPUT(src/config.mk src/gmt_version.h src/isogmt $morefiles)
sed -e s/@UNITS@/Template\ SI/ -e s/@PS_CHAR_ENCODING@/ISOLatin1+/ -e s/@PS_MEDIA@/a4/ -e s/@PROJ_LENGTH_UNIT@/cm/ \
	-e s/@PACKAGE_VERSION@/$PACKAGE_VERSION/ -e s/@TRIANGULATE@/$TRIANGULATE/ \
	share/conf/gmt.conf.in > share/conf/gmt_SI.conf
sed -e s/@UNITS@/Template\ US/ -e s/@PS_CHAR_ENCODING@/Standard+/ -e s/@PS_MEDIA@/letter/ -e s/@PROJ_LENGTH_UNIT@/inch/ \
	-e s/@PACKAGE_VERSION@/$PACKAGE_VERSION/ -e s/@TRIANGULATE@/$TRIANGULATE/  \
	share/conf/gmt.conf.in > share/conf/gmt_US.conf
if test "$UNITS" = "SI" ; then
	sed -e s/@UNITS@/System-wide/ -e s/@PS_CHAR_ENCODING@/ISOLatin1+/ -e s/@PS_MEDIA@/a4/ -e s/@PROJ_LENGTH_UNIT@/cm/ \
	-e s/@PACKAGE_VERSION@/$PACKAGE_VERSION/ -e s/@TRIANGULATE@/$TRIANGULATE/  \
	share/conf/gmt.conf.in > share/conf/gmt.conf
else
	sed -e s/@UNITS@/System-wide/ -e s/@PS_CHAR_ENCODING@/Standard+/ -e s/@PS_MEDIA@/letter/ -e s/@PROJ_LENGTH_UNIT@/inch/ \
	-e s/@PACKAGE_VERSION@/$PACKAGE_VERSION/ -e s/@TRIANGULATE@/$TRIANGULATE/  \
	share/conf/gmt.conf.in > share/conf/gmt.conf
fi
dnl
dnl -----------------------------------------------------------------
dnl Remove and, if trouble, write the .gmtconfigure file
dnl -----------------------------------------------------------------
dnl
if test -f .gmtconfigure ; then
	\rm -f .gmtconfigure
fi
if test "$shared_unknown" = "yes" ; then
	cat << EOF > .gmtconfigure
=============================================================
Shared libraries for GMT under $os are not implemented.  This
can mean that your OS does not support shared libraries at all
or that the GMT configure script simply does not know how to.
Therefore, configure has ignored the --enable-shared option.
However, if shared libraries ARE supported on this system you
may try edit src/config.mk and manually set the macros
CFLAGS, CPPFLAGS, LD_OPT, LDFLAGS, and LIBEXT accordingly.
Please include the output of uname -a and mail the working
config.mk to gmt-team@lists.hawaii.edu.  Thanx.
=============================================================
EOF
	cat .gmtconfigure
fi
