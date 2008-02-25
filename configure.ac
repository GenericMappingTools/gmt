dnl -----------------------------------------------------------------
dnl $Id: configure.ac,v 1.7 2008-02-25 17:18:38 remko Exp $
AC_REVISION($Revision: 1.7 $)
dnl
dnl Run GNU autoconf to produce a configure script from this input.
dnl -----------------------------------------------------------------
dnl
dnl
dnl -----------------------------------------------------------------
dnl First specify the GMT and GSHHS version and release date
dnl -----------------------------------------------------------------
dnl Below, set the VERSION to be created.
dnl Also, you may want to change the DATE values.
dnl
AC_INIT(GMT, 4.2.2b, gmt-help@hawaii.edu)
MANDATE="1 Apr 2008"
DOCDATE="April 2008"
DOCYEAR="2008"
dnl
dnl -----------------------------------------------------------------
dnl The remainder should need no editing
dnl -----------------------------------------------------------------
dnl
AC_CONFIG_SRCDIR(src/gmt.h)
AC_CONFIG_HEADER(src/gmt_notposix.h)
AC_CONFIG_AUX_DIR(`pwd`)
owncflags=${CFLAGS}
ownldflags=${LDFLAGS}
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
AC_SUBST(EXE)
AC_SUBST(COMPRESS)
AC_SUBST(MATLAB)
AC_SUBST(FTPSITE)
AC_SUBST(TXT2MAN)
AC_SUBST(NETCDF)
AC_SUBST(TRIANGLE_D)
AC_SUBST(TRIANGLE_O)
AC_SUBST(ALPHA_SINCOS_O)
AC_SUBST(CC_OPT)
AC_SUBST(LD_OPT)
AC_SUBST(LDFLAGS)
AC_SUBST(DOCDATE)
AC_SUBST(DOCYEAR)
AC_SUBST(MANDATE)
AC_SUBST(LIBEXT)
AC_SUBST(SL)
AC_SUBST(MATLAB)
AC_SUBST(MATLAB_EXT)
AC_SUBST(MATLAB_MEX)
AC_SUBST(MATLAB_LIB)
AC_SUBST(rootdir)
AC_SUBST(wwwdir)
dnl
dnl -----------------------------------------------------------------
dnl Special configure options for GMT installation
dnl -----------------------------------------------------------------
dnl
AC_ARG_ENABLE(update,   [  --enable-update=site    GMT ftp site [[ftp.soest.hawaii.edu]]])
AC_ARG_ENABLE(netcdf,   [  --enable-netcdf=dir     directory with netCDF [[/usr/local/netcdf]]])
AC_ARG_ENABLE(matlab,   [  --enable-matlab=dir     directory with Matlab [[/usr/local/matlab]]])
AC_ARG_ENABLE(debug,    [  --enable-debug          compile for debugging instead of optimizing code])
AC_ARG_ENABLE(shared,   [  --enable-shared         build shared (dynamic) GMT libraries instead of static])
AC_ARG_ENABLE(triangle, [  --enable-triangle       use J. Shewchuk's fast, non-GPL triangulation routine])
AC_ARG_ENABLE(US,       [  --enable-US             choose US (inch) rather than SI (cm) units as default])
AC_ARG_ENABLE(www,      [  --enable-www=dir        directory in which to install GMT www pages [[PREFIX/www]]])
AC_ARG_ENABLE(eps,      [  --enable-eps            set Encapsulated PostScript as default output format [[Freeform PS]]])
AC_ARG_ENABLE(universal,[  --enable-universal      make universal binaries [[OS X only]]])
AC_ARG_ENABLE(64,       [  --enable-64             enable 64-bit compilation if supported])
AC_ARG_ENABLE(flock,    [  --disable-flock         disable file locking on .gmtcommands4 file])
AC_ARG_ENABLE(mex,      [  --disable-mex           disable compilation of mex supplement which requires Matlab])
AC_ARG_ENABLE(xgrid,    [  --disable-xgrid         disable compilation of xgrid supplement which requires X11])
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
chip=`uname -p`
Sun_rev=`uname -r | grep '^5.'`
dnl
dnl -----------------------------------------------------------------
dnl Misc. parameters
dnl -----------------------------------------------------------------
dnl
rootdir=`pwd`
LIB_VERSION=`echo $PACKAGE_VERSION | tr '[a-zA-Z]' ' '`
dnl
dnl -----------------------------------------------------------------
dnl If not set already, guess TXT2MAN (preprocessor for manpages)
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for GMT manpage preprocessor)
if test "X$TXT2MAN" = "X" ; then
	if test "$os" = "SunOS" ; then		# Sun Workshop
		TXT2MAN="cc -E -Xs"
	elif test "$os" = "HP-UX" ; then	# HP
		TXT2MAN="/usr/ccs/lbin/cpp"
	else					# Linux, Darwin, other?
		TXT2MAN="/usr/bin/cpp -w -P -nostdinc"
	fi
fi
AC_MSG_RESULT($TXT2MAN)
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
dnl Allow user to set --enable-matlab to set Matlab dir
dnl Note: if Matlab is disabled, then so should Mex.
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for Matlab directory)
if test "X$enable_matlab" = "Xyes" ; then
	MATLAB=${MATLAB:-/usr/local/matlab}
elif test "X$enable_matlab" = "X" ; then
	MATLAB='NONE'
	enable_mex="no"
else
	MATLAB=$enable_matlab
fi
AC_MSG_RESULT($MATLAB)
dnl
dnl -----------------------------------------------------------------
dnl Determine Matlab mex extensions and dirs for this platform
dnl -----------------------------------------------------------------
dnl
if test "X$MATLAB" = "XNONE" ; then
	AC_MSG_RESULT([Matlab mex supplement will be skipped])
else
	AC_MSG_CHECKING(compiler options for Matlab)
	if test "$os" = "Linux" ; then		# Linux systems
		MATLAB_EXT='mexglx'
		MATLAB_MEX='glnx86'
		MATLAB_LIB='mat'
	elif test "$os" = "SunOS" ; then	# Sun Workshop
		MATLAB_EXT='mexsol'
		MATLAB_MEX='sol2'
		MATLAB_LIB='mat'
	elif test "$os" = "IRIX64" ; then	# SGI Irix 64
		MATLAB_EXT='mexsg64'
	elif test "$os" = "IRIX" ; then		# SGI Irix
		MATLAB_EXT='mexsg'
	elif test "$os" = "HP-UX" ; then	# HP
		MATLAB_EXT='mexhpux'
	elif test "$os" = "AIX" ; then		# IBM AIX
		MATLAB_EXT='mexrs6'
		MATLAB_MEX='ibm_rs'
		MATLAB_LIB='mat'
	elif test "$os" = "OSF1" ; then		# Dec Alpha
		MATLAB_EXT='mexds'
	elif test "$os" = "ULTRIX" ; then	# Dec Ultrix
		MATLAB_EXT='mexvx'
	elif test "$os" = "Darwin" ; then	# Mac OS X
		dir=`ls /Applications | grep MATLAB | head -n 1`
		if test "X$dir" = "X" ; then
			MATLAB=NONE
		else
			MATLAB=/Applications/$dir
		fi
		if test "$chip" = "i386" ; then		#Intel Mac
			MATLAB_EXT='mexmaci'
			MATLAB_MEX='maci'
		else
			MATLAB_EXT='mexmac'
			MATLAB_MEX='mac'
		fi
		MATLAB_LIB='mat'
	else					# Other
		MATLAB_EXT='mex'
	fi

	if test "X$MATLAB_LIB" = "X" || test "X$MATLAB_MEX" = "X" ; then
		# Users: pls supply your input
		AC_MSG_RESULT([Matlab mex-file extension and Matlab library name not known - guessed to be mex])
		AC_MSG_RESULT([If incorrect, please modify makefile manually and notify gmt@soest.hawaii.edu])
		MATLAB_MEX='mex'
		MATLAB_LIB='mex'
	fi
	AC_MSG_RESULT([-L$MATLAB/extern/lib/$MATLAB_MEX -l$MATLAB_LIB, extension .$MATLAB_EXT])
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-netcdf to set netCDF dir
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for netCDF directory)
if test "X$enable_netcdf" = "X" || test "X$enable_netcdf" = "Xyes" ; then
	NETCDF='/usr/local/netcdf'
else
	NETCDF=$enable_netcdf
fi
AC_MSG_RESULT($NETCDF)
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-www to set install www dir
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for www directory)
if test "X$enable_www" = "X" || test "X$enable_www" = "Xyes" ; then
	wwwdir='${prefix}/www'
else
	wwwdir=$enable_www
fi
AC_MSG_RESULT($wwwdir)
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-eps to set EPS as default format
dnl -----------------------------------------------------------------
dnl
dnl Message delayed until later
if test "X$enable_eps" = "Xyes" ; then
	eps=EPS
else
	eps=PS
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --disable-flock for turning off file locking
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for file locking)
if test "X$enable_flock" != "Xno" ; then
	enable_flock="yes"
fi
AC_MSG_RESULT($enable_flock)
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-64 for turning on 64-bit compilation
dnl -----------------------------------------------------------------
dnl
if test "X$enable_64" != "Xyes" ; then
	enable_64="no"
fi
AC_MSG_CHECKING(for 64-bit compiling)
AC_MSG_RESULT($enable_64)
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-universal for OS X universal binaries
dnl -----------------------------------------------------------------
dnl
if test "X$os" != "XDarwin" || test "X$enable_universal" != "Xyes" ; then
	enable_universal="no"
fi
AC_MSG_CHECKING(for universal binaries)
AC_MSG_RESULT($enable_universal)
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --disable-mex for turning off mex compilation
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(if OK to compile mex)
if test "X$enable_mex" = "Xno" ; then
	enable_mex="no"
elif test -f "$MATLAB/bin/mex" ; then
	enable_mex="yes"
else
	enable_mex="no"
fi
AC_MSG_RESULT($enable_mex)
if test "$enable_mex" = "no" && test -d src/mex; then
	touch src/mex/.skip
	AC_MSG_WARN([[Disabling mex compilation since Matlab was not found]])
else
	rm -f src/mex/.skip
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
AC_MSG_RESULT($enable_xgrid)
if test "X$enable_xgrid" = "Xno" && test -d src/xgrid; then
	touch src/xgrid/.skip
	AC_MSG_WARN([[Disabling xgrid compilation]])
else
	rm -f src/xgrid/.skip
fi
dnl
dnl -----------------------------------------------------------------
dnl Allow user to set --enable-debug to pass -g flag
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(if we should compile with debugging switch -g)
if test "X$enable_debug" = "Xyes" ; then
	AC_MSG_RESULT(yes)
	OPTG="-g -DDEBUG "
	OPTGO="-g -DDEBUG "
	OPTS=
else
	OPTG=
	if test "$os" = "Darwin" ; then
		OPTS="-Wl,-x "
	else
		OPTS="-s "
	fi
	if test "$os" = "Linux" || test "$os" = "AIX" ; then
		OPTGO="-O2 "
	else
		OPTGO="-O "
	fi
	AC_MSG_RESULT(no)
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
else
	AC_MSG_RESULT(no)
fi
dnl
dnl -----------------------------------------------------------------
dnl Special checking and action required for Cygwin or OS/2
dnl -----------------------------------------------------------------
dnl
EXE=
WIN32=
COMPRESS="@:"
if test "X$CYGWIN" = "Xyes" ; then
	EXE=".exe"
	WIN32=" -D_WIN32"
	if test ! -d /tmp ; then
		AC_MSG_WARN(Cygwin: /tmp not found - created)
		mkdir /tmp
	fi
	if test ! -f /bin/sh.exe ; then
		AC_MSG_WARN(Cygwin: /bin/sh not found - created)
		this_sh=`type -p sh`
		cp -f $this_sh /bin
	fi
fi
if test "$os" = "OS/2" ; then
	EXE=".exe"
	WIN32=" -D__EMX__"
	COMPRESS="lxlite /ML1"
fi
dnl
dnl -----------------------------------------------------------------
dnl Determine optional CFLAGS and LDFLAGS additions for OS X
dnl -----------------------------------------------------------------
dnl
if test "$enable_universal" = "yes" ; then
	if test "$enable_64" = "yes" ; then
		ppc=ppc64
	else
		ppc=ppc
	fi
	OSX_L=' -arch i386 -arch '$ppc
	OSX_C=$OSX_L
	if test "$chip" = "powerpc" ; then
		OSX_L=' -isysroot /Developer/SDKs/MacOSX10.4u.sdk'$OSX_L
	fi
fi
dnl
dnl -----------------------------------------------------------------
dnl Determine LD_OPT and LDFLAGS for this platform
dnl -----------------------------------------------------------------
dnl
shared_unknown=no
LDFLAGS=$OPTS
LD_OPT=
LD=ld
SL="so"
LIBEXT='$(SL)'
if test "$enable_shared" = "yes" ; then
	AC_MSG_CHECKING(how to make a shared object)
	if test "$os" = "SunOS" ; then
		if test "$Sun_rev" = "" ; then	# SunOS
			LD_OPT="-Bdynamic"
			LDFLAGS=$OPTS'-Wl,-L$(libdir):'$NETCDF/lib
		else				# Solaris
			LD_OPT="-G"
			LDFLAGS=$OPTS'-Wl,-R,$(libdir):'$NETCDF/lib
		fi
	elif test "$os" = "Linux" ; then	# Need optimization when doing shared
		LD=gcc
		LD_OPT="-shared"
		LDFLAGS=$OPTS'-Wl,-rpath,$(libdir):'$NETCDF/lib
	elif test "$os" = "FreeBSD" ; then
		LD_OPT="-shared"
		LDFLAGS=$OPTS'-Wl,-rpath,$(libdir):'$NETCDF/lib
	elif test "$os" = "NetBSD" ; then
		LD_OPT="-shared"
		LDFLAGS=$OPTS'-Wl,-rpath,$(libdir):'$NETCDF/lib
	elif test "$os" = "OSF1" ; then
		LD_OPT='-shared -expect_unresolved "*"'
		LDFLAGS=$OPTS'-Wl,-rpath,$(libdir):'$NETCDF/lib
	elif test "$os" = "IRIX64" ; then
		LD_OPT="-shared"
		LDFLAGS=$OPTS'-Wl,-rpath,$(libdir):'$NETCDF/lib
		# Need to remove -L/usr/lib to avoid mixing o32 and n32
		X_LIBS=
	elif test "$os" = "Darwin" ; then	# MacOS X
		LD=gcc
		LD_OPT='-flat_namespace -undefined suppress -install_name ${prefix}/lib/$*.4.$(SL) -compatibility_version 4.0.0 -current_version '$LIB_VERSION' -dynamiclib'
		LDFLAGS='-Wl,-x'$OSX_L
		SL=dylib
	elif test "$os" = "HP-UX" ; then
		LD_OPT="-b"
		LDFLAGS=$OPTS'-Wl,+b,$(libdir)'
		SL="sl"
	else	# Not yet implemented
		AC_MSG_RESULT(not supported for GMT under $os)
		enable_shared=no
		shared_unknown=yes
	fi
fi
if test "$enable_shared" = "yes" ; then
	AC_MSG_RESULT($LD $LD_OPT)
	AC_MSG_CHECKING(the file extension for a shared object)
	AC_MSG_RESULT($SL)
else
	LIBEXT='a'
fi
AC_MSG_CHECKING(linker options for GMT)
LDFLAGS=${ownldflags:-$LDFLAGS}			# Overrule with user settings
AC_MSG_RESULT($LDFLAGS)
dnl
dnl -----------------------------------------------------------------
dnl Determine CC_OPT for this platform
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(compiler options for GMT)
if test "$enable_64" = "yes" ; then
	OPT64=' -m64'
fi
if test "$GCC" = "yes"; then		# GNU cc options
	if test "$chip" = "alpha" ; then	# Alpha PC
		CC_OPT=$OPTG'-mieee -Wall'
	elif test "$os" = "Darwin" ; then	# MacOS X
		CC_OPT=$OPTGO'-pedantic -fno-common -Wall'$OPT64$OSX_C
	else
		CC_OPT=$OPTGO'-pedantic -Wall'
	fi
	if test "$enable_shared" = "yes" ; then
		CC_OPT=$CC_OPT' -fPIC'
	fi
elif test "$os" = "SunOS" ; then	# Sun Workshop CC options
	CC_OPT=$OPTGO'-Xc'
elif test "$os" = "IRIX64" ; then	# SGI Irix cc options
	CC_OPT=$OPTGO
elif test "$os" = "HP-UX" ; then	# HP cc options
	CC_OPT=$OPTG'-Aa +Ofltacc +O1 +Olibcalls'
	if test "$enable_shared" = "yes" ; then
		CC_OPT=$CC_OPT' +z'
	fi
elif test "$os" = "AIX" ; then		# IBM AIX cc options
	CC_OPT=$OPTGO
elif test "$os" = "AUX" ; then		# Apple A/UX cc options
	CC_OPT=$OPTG'-Daux'
elif test "$os" = "OSF1" ; then		# Dec Alpha OSF cc options
	CC_OPT=$OPTGO'-Olimit 1500 -ieee_with_inexact'
elif test "$os" = "ULTRIX" ; then	# Dec Ultrix cc options
	CC_OPT=$OPTGO'-Olimit 1500'
elif test "$os" = "Windows_NT" ; then	# Interix w/ MS cc options
	CC_OPT=$OPTGO
else					# Generic setting for unknown systems
	CC_OPT=$OPTGO
fi
CC_OPT=$CC_OPT$WIN32			# Adds Windows flag
CC_OPT=${owncflags:-$CC_OPT}		# Overrule with user settings
if test "$enable_flock" = "no" ; then
	CC_OPT=$CC_OPT' -DNO_LOCK'
fi
AC_MSG_RESULT($CC_OPT)
dnl
dnl -----------------------------------------------------------------
dnl	Create share/conf/gmt.conf and set site defaults
dnl -----------------------------------------------------------------
dnl
AC_MSG_CHECKING(for default GMT units)
if test ! -f share/conf/gmt.conf.orig ; then
	cp share/conf/gmt.conf share/conf/gmt.conf.orig
fi
if test ! -f share/conf/.gmtdefaults_SI.orig ; then
	cp share/conf/.gmtdefaults_SI share/conf/.gmtdefaults_SI.orig
fi
if test ! -f share/conf/.gmtdefaults_US.orig ; then
	cp share/conf/.gmtdefaults_US share/conf/.gmtdefaults_US.orig
fi
\rm -f share/conf/gmt.conf
if test "$enable_US" = "yes" ; then
	AC_MSG_RESULT(US (inch))
	paper=Letter
	origdef=share/conf/.gmtdefaults_US.orig
	thisdef=share/conf/.gmtdefaults_US
	echo "US        <- Set to SI or US" > share/conf/gmt.conf
else
	AC_MSG_RESULT(SI (cm))
	paper=A4
	origdef=share/conf/.gmtdefaults_SI.orig
	thisdef=share/conf/.gmtdefaults_SI
	echo "SI        <- Set to SI or US" > share/conf/gmt.conf
fi
egrep -v '^SI|^US' share/conf/gmt.conf.orig >> share/conf/gmt.conf
AC_MSG_CHECKING(for default PostScript format)
\rm -f $thisdef
if test "$enable_eps" = "yes" ; then
	AC_MSG_RESULT(EPS)
	if test "$enable_US" = "yes" ; then
		$AWK '{if ($1 == "PAPER_MEDIA") {printf "PAPER_MEDIA\t\t= Letter+\n"} else {print $0}}' $origdef > $thisdef
	else
		$AWK '{if ($1 == "PAPER_MEDIA") {printf "PAPER_MEDIA\t\t= A4+\n"} else {print $0}}' $origdef > $thisdef
	fi
else
	AC_MSG_RESULT(PS)
	if test "$enable_US" = "yes" ; then
		$AWK '{if ($1 == "PAPER_MEDIA") {printf "PAPER_MEDIA\t\t= Letter\n"} else {print $0}}' $origdef > $thisdef
	else
		$AWK '{if ($1 == "PAPER_MEDIA") {printf "PAPER_MEDIA\t\t= A4\n"} else {print $0}}' $origdef > $thisdef
	fi
fi
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
			AC_MSG_RESULT([I don't know how to fix sincos on this machine.])
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
AC_CHECK_FUNC(copysign, AC_DEFINE(HAVE_COPYSIGN), )
AC_CHECK_FUNC(log1p, AC_DEFINE(HAVE_LOG1P), )
AC_CHECK_FUNC(hypot, AC_DEFINE(HAVE_HYPOT), )
AC_CHECK_FUNC(acosh, AC_DEFINE(HAVE_ACOSH), )
AC_CHECK_FUNC(asinh, AC_DEFINE(HAVE_ASINH), )
AC_CHECK_FUNC(atanh, AC_DEFINE(HAVE_ATANH), )
AC_CHECK_FUNC(rint, AC_DEFINE(HAVE_RINT), )
AC_CHECK_FUNC(irint, AC_DEFINE(HAVE_IRINT), )
AC_CHECK_FUNC(isnanf, AC_DEFINE(HAVE_ISNANF), )
AC_CHECK_FUNC(isnand, AC_DEFINE(HAVE_ISNAND), )
AC_CHECK_FUNC(isnan, AC_DEFINE(HAVE_ISNAN), )
AC_CHECK_FUNC(j0, AC_DEFINE(HAVE_J0), )
AC_CHECK_FUNC(j1, AC_DEFINE(HAVE_J1), )
AC_CHECK_FUNC(jn, AC_DEFINE(HAVE_JN), )
AC_CHECK_FUNC(y0, AC_DEFINE(HAVE_Y0), )
AC_CHECK_FUNC(y1, AC_DEFINE(HAVE_Y1), )
AC_CHECK_FUNC(yn, AC_DEFINE(HAVE_YN), )
AC_CHECK_FUNC(erf, AC_DEFINE(HAVE_ERF), )
AC_CHECK_FUNC(erfc, AC_DEFINE(HAVE_ERFC), )
AC_CHECK_FUNC(strdup, AC_DEFINE(HAVE_STRDUP), )
AC_CHECK_FUNC(strtod, AC_DEFINE(HAVE_STRTOD), )
AC_CHECK_FUNCS([floor getcwd gettimeofday memset modf pow rint sqrt strchr strdup strpbrk strrchr strstr strtol])
dnl
dnl -----------------------------------------------------------------
dnl Override HYPOT under Interix and Cygwin as our function is
dnl much faster than what configure seems to find.
dnl -----------------------------------------------------------------
dnl
if test "$os" = "Windows_NT" ; then	# Interix
	AC_DEFINE(HAVE_HYPOT, 0)
elif test "$CYGWIN" = "yes" ; then	# Cygwin environment
	AC_DEFINE(HAVE_HYPOT, 0)
else					# Let configure it out
	AC_CHECK_FUNC(hypot, AC_DEFINE(HAVE_HYPOT), )
fi
dnl -----------------------------------------------------------------
dnl Find out about byte order
dnl -----------------------------------------------------------------
dnl
AC_C_BIGENDIAN
dnl
dnl -----------------------------------------------------------------
dnl Update the GMT macros
dnl -----------------------------------------------------------------
dnl
morefiles=
if test -f doc/GMT_version.tex.in ; then
	morefiles=doc/GMT_version.tex
fi
AC_OUTPUT(src/makegmt.macros src/GMT src/gmt_version.h $morefiles)
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
may try edit src/makegmt.macros and manually set the macros
CC_OPT, LD_OPT, LDFLAGS, and LIBEXT accordingly.
Please include the output of uname -a and mail the working
makegmt.macros to gmt-team@hawaii.edu.  Thanx.
=============================================================
EOF
	cat .gmtconfigure
fi
