# # m4 macros from the Unidata netcdf 2.3.2 pl4 distribution. Modified for use
# # with GNU Autoconf 2.1. I renamed these from UC_* to DODS_* so that there
# # will not be confusion when porting future versions of netcdf into the DODS
# # source distribution. Unidata, Inc. wrote the text of these macros and holds
# # a copyright on them.
# #
# # jhrg 3/27/95
# #
# # Added some of my own macros (don't blame Unidata for them!) starting with
# # DODS_PROG_LEX and down in the file. jhrg 2/11/96
# #
# # I've added a table of contents for this file. jhrg 2/2/98
# # 
# # 1. Unidata-derived macros. 
# # 2. Macros for finding libraries used by the core software.
# # 3. Macros used to test things about the compiler
# # 4. Macros for locating various systems (Matlab, etc.)
# # 5. Macros used to test things about the computer/OS/hardware
# #
# # $Id$
# 
# # 1. Unidata's macros
# #-------------------------------------------------------------------------
# 
# # 2. Finding libraries 
# #
# # To use these in DODS software, in the Makefile.in use LIBS XTRALIBS for 
# # non-gui and LIBS GUILIBS XTRALIBS for the clients that link with the 
# # gui DAP++ library. These should be set by a line like LIBS=@LIBS@, etc.
# # Then the group should be used on the link line. 10/16/2000 jhrg
# #--------------------------------------------------------------------------
# 
# # Electric fence and dbnew are used to debug malloc/new and free/delete.
# # I assume that if you use these switches you know enough to build the 
# # libraries. 2/3/98 jhrg
# 
# AC_DEFUN([DODS_EFENCE], [dnl
#     AC_ARG_ENABLE(efence,
# 		  [  --enable-efence         Runtime memory checks (malloc)],
# 		  EFENCE=$enableval, EFENCE=no)
# 
#     case "$EFENCE" in
#     yes)
#       AC_MSG_RESULT(Configuring dynamic memory checks on malloc/free calls)
#       LIBS="$LIBS -lefence"
#       ;;
#     *)
#       ;;
#     esac])
# 
# AC_DEFUN([DODS_DBNEW], [dnl
#     AC_ARG_ENABLE(dbnew,
# 	          [  --enable-dbnew          Runtime memory checks (new)],
# 		  DBNEW=$enableval, DBNEW=no)
# 
#     case "$DBNEW" in
#     yes)
#       AC_MSG_RESULT(Configuring dynamic memory checks on new/delete calls)
#       AC_DEFINE(TRACE_NEW, , [Use the dbnew library to trace bew/delete calls])
#       LIBS="$LIBS -ldbnew"
#       ;;
#     *)
#       ;;
#     esac])
# 
# # check for hdf libraries
# # cross-compile problem with test option -d
# AC_DEFUN([DODS_HDF_LIBRARY], [dnl
#     AC_ARG_WITH(hdf,
#         [  --with-hdf=ARG          Where is the HDF library (directory)],
#         HDF_PATH=${withval}, HDF_PATH="$HDF_PATH")
#     if test ! -d "$HDF_PATH"
#     then
#         HDF_PATH="/usr/local/hdf"
#     fi
#     if test "$HDF_PATH"
#     then
#             LDFLAGS="$LDFLAGS -L${HDF_PATH}/lib"
#             INCS="$INCS -I${HDF_PATH}/include"
#             AC_SUBST(INCS)
#     fi
# 
# dnl None of this works with HDF 4.1 r1. jhrg 8/2/97
# 
#     AC_CHECK_LIB(z, deflate, LIBS="-lz $LIBS", nohdf=1)
#     AC_CHECK_LIB(jpeg, jpeg_start_compress, LIBS="-ljpeg $LIBS", nohdf=1)
#     AC_CHECK_LIB(df, Hopen, LIBS="-ldf $LIBS" , nohdf=1)
#     AC_CHECK_LIB(mfhdf, SDstart, LIBS="-lmfhdf $LIBS" , nohdf=1)])
# 
# # 3. Compiler test macros
# #--------------------------------------------------------------------------
# 
# AC_DEFUN([DODS_GCC_VERSION], [dnl
#     AC_MSG_CHECKING(for gcc/g++ 2.8 or greater)
# 
#     GCC_VER=`gcc -v 2>&1 | awk '/version/ {print}'`
# 
#     dnl We need the gcc version number as a number, without `.'s and limited
#     dnl to three digits. The old version below was replaced by Andy Jacobson's 
#     dnl patch which works with gcc 3, including the pre-release versions.
# 
#     dnl GCC_VER=`echo $GCC_VER | sed 's@[[a-z ]]*\([[0-9.]]\)@\1@'`
#     GCC_VER=`echo $GCC_VER | sed 's@.*gcc version \([[0-9\.]]*\).*@\1@'`
# 
#     case $GCC_VER in
#         *egcs*) AC_MSG_RESULT(Found egcs version ${GCC_VER}.) ;;
# 	2.[[7-9]]*)   AC_MSG_RESULT(Found gcc/g++ version ${GCC_VER}) ;;
# 	3.[[0-9]]*)   AC_MSG_RESULT(Found gcc/g++ version ${GCC_VER}) ;;
#         *)      AC_MSG_ERROR(must be at least version 2.7.x) ;;
#     esac])
# 
# # 4. Macros to locate various programs/systems used by parts of DODS
# #---------------------------------------------------------------------------
# 
# # Added by Ethan, 1999/06/21
# # Look for perl.
# # 
# # I modified the regexp below to remove any text that follows the version
# # number. This extra text was hosing the test. 7/15/99 jhrg
# 
# AC_DEFUN([DODS_PROG_PERL], [dnl
#     AC_CHECK_PROG(PERL, perl, `which perl`)
#     case "$PERL" in
# 	*perl*)
# 	    perl_ver=`$PERL -v 2>&1 | awk '/This is perl/ {print}'`
# 	    perl_ver=`echo $perl_ver | sed 's/This is perl,[[^0-9]]*\([[0-9._]]*\).*/\1/'`
#             perl_ver_main=`echo $perl_ver | sed 's/\([[0-9]]*\).*/\1/'`
# 	    if test -n "$perl_ver" && test $perl_ver_main -ge 5
# 	    then
# 		AC_MSG_RESULT(Found perl version ${perl_ver}.)
# 	    else
# 		AC_MSG_ERROR(perl version: found ${perl_ver} should be at least 5.000.)
# 	    fi
# 	    ;;
# 	*)
# 	    AC_MSG_WARN(perl is required.)
# 	    ;;
#     esac
# 
#     AC_SUBST(PERL)])
# 
# # Added by Ethan, 1999/06/21
# # Look for GNU tar.
# # 
# # I modified the regexp below but it still does not work exactly correctly; 
# # the variable tar_ver should have only the version number in it. However,
# # my version (1.12) spits out a multi-line thing. The regexp below gets the
# # version number from the first line but does not remove the subsequent lines
# # of garbage. 7/15/99 jhrg
# # Added awk line to handle multiline output. 1999/07/22 erd
# 
# AC_DEFUN([DODS_PROG_GTAR], [dnl
#     AC_CHECK_PROGS(TAR,gtar tar,tar)
#     case "$TAR" in
# 	*tar)
# 	    tar_ver=`$TAR --version 2>&1 | awk '/G[[Nn]][[Uu]] tar/ {print}'`
# 	    tar_ver=`echo $tar_ver | sed 's/.*GNU tar[[^0-9.]]*\([[0-9._]]*\)/\1/'`
# 	    if test -n "$tar_ver"
# 	    then
# 		AC_MSG_RESULT(Found Gnu tar version ${tar_ver}.)
# 	    else
# 		AC_MSG_WARN(GNU tar is required for some Makefile targets.)
# 	    fi
# 	    ;;
# 	*)
# 	    AC_MSG_WARN(GNU tar is required for some Makefile targets.)
# 	    ;;
#     esac
# 
#     AC_SUBST(TAR)])
# 
# # Find the matlab root directory
# # cross-compile problem with test option -d
# 
# AC_DEFUN([DODS_MATLAB], [dnl
#     AC_ARG_WITH(matlab,
#         [  --with-matlab=ARG       Where is the Matlab root directory],
#         MATLAB_ROOT=${withval}, MATLAB_ROOT="$MATLAB_ROOT")
#     if test "$MATLAB_ROOT" = no; then
#         MATLAB_ROOT="$MATLAB_ROOT"
#     elif test ! -d "$MATLAB_ROOT"
#     then
#         MATLAB_ROOT=""
#     fi
#     if test -z "$MATLAB_ROOT"
#     then
#         AC_MSG_CHECKING(for matlab root)
# 
# 	MATLAB_ROOT=`mex -v 2>&1 | awk '/MATLAB *= / {print}'`
# 	MATLAB_ROOT=`echo $MATLAB_ROOT | sed 's@[[^/]]*\(/.*\)@\1@'`
# 
# 	if test -z "$MATLAB_ROOT"
# 	then
# 	    AC_MSG_ERROR(Matlab not found! Run configure --help)
#         else
# 	    AC_SUBST(MATLAB_ROOT)
# 	    AC_MSG_RESULT($MATLAB_ROOT)
#         fi
#     else
#         AC_SUBST(MATLAB_ROOT)
#         AC_MSG_RESULT("Set Matlab root to $MATLAB_ROOT")
#     fi
# 
#     dnl Find the lib directory (which is named according to machine type).
#     dnl The test was using -z; I changed it to -n to fix a problem withthe 
#     dnl matlab server build. The build did not find the libraries with the
#     dnl env var MATLAB_LIB was not set. 04/12/04 jhrg
#     AC_MSG_CHECKING(for matlab library dir)
#     if test -n "$MATLAB_LIB"
#     then
#         matlab_lib_dir=${MATLAB_LIB}
#     else
#         matlab_lib_dir=`find ${MATLAB_ROOT}/extern/lib -name 'libmat*' -print | sed '1q'`
#         matlab_lib_dir=`echo $matlab_lib_dir | sed 's@\(.*\)/libmat.*@\1@'`
#     fi
#     AC_MSG_RESULT($matlab_lib_dir)
# 
#     if test "$matlab_lib_dir"
#     then
# 	LDFLAGS="$LDFLAGS -L$matlab_lib_dir"
# 	dnl This is used by the nph script to set LD_LIBRARY_PATH
# 	AC_SUBST(matlab_lib_dir)
#     fi
#     
#     dnl sleazy test for version 5; look for the version 4 compat flag
# 
#     if grep V4_COMPAT ${MATLAB_ROOT}/extern/include/mat.h > /dev/null 2>&1
#     then
# 	dnl if we're here, we're linking under ML 5 or 6. 02/10/03 jhrg
# 	MAT_VERSION_FLAG="-V4"
# 	dnl ML 6 lacks libmi.a/.so. 02/10/03 jhrg
# 	if access -r ${matlab_lib_dir}/libmi.*
# 	then
# 	    MATLIBS="-lmat -lmi -lmx -lut"
# 	else
# 	    MATLIBS="-lmat -lmx -lut"
# 	fi
#     else
#        MAT_VERSION_FLAG=""
#        MATLIBS="-lmat"
#     fi
# 
#     AC_CHECK_LIB(ots, _OtsDivide64Unsigned, MATLIBS="$MATLIBS -lots", )
#     AC_SUBST(MATLIBS)
#     AC_SUBST(MAT_VERSION_FLAG)])
# 
# # cross-compile problem with test option -d
# AC_DEFUN([DODS_DSP_ROOT], [dnl
# 
#     AC_ARG_WITH(dsp,
# 		[  --with-dsp=DIR          Directory containing DSP software from U of Miami],
# 		DSP_ROOT=${withval}, DSP_ROOT="$DSP_ROOT")
# 
#     if test ! -d "$DSP_ROOT"
#     then
#         DSP_ROOT=""
#     fi
#     if test -z "$DSP_ROOT"
#     then
# 	AC_MSG_CHECKING(for the DSP library root directory)
# 
# 	for p in /usr/local/src/DSP /usr/local/DSP \
# 		 /usr/local/src/dsp /usr/local/dsp \
# 		 /usr/contrib/src/dsp /usr/contrib/dsp \
# 		 $DODS_ROOT/third-party/dsp /usr/dsp /data1/dsp
# 	do
# 	    if test -z "$DSP_ROOT"
# 	    then
# 	    	for d in `ls -dr ${p}* 2>/dev/null`
# 		do
# 		    if test -f ${d}/inc/dsplib.h
# 		    then
# 		        DSP_ROOT=${d}
# 		        break
# 		    fi
# 	        done
# 	    fi
# 	done
#     fi
# 
#     if test "$DSP_ROOT"
#     then
# 	AC_SUBST(DSP_ROOT)
# 	dnl Only add this path to gcc's options... jhrg 11/15/96
# 	INCS="$INCS -I\$(DSP_ROOT)/inc"
# 	AC_SUBST(INCS) 		# 09/20/02 jhrg
# 	LDFLAGS="$LDFLAGS -L\$(DSP_ROOT)/lib -L\$(DSP_ROOT)/shlib"
# 	AC_MSG_RESULT(Set DSP root directory to $DSP_ROOT) 
#     else
#         AC_MSG_ERROR(not found! see configure --help)
#     fi])
# 
# # Find IDL. 9/23/99 jhrg
# 
# AC_DEFUN([DODS_IDL], [dnl
#     AC_ARG_WITH(idl,
#         [  --with-idl=ARG       Where is the IDL root directory],
#         IDL_ROOT=${withval}, IDL_ROOT="$IDL_ROOT")
#     AC_REQUIRE([AC_CANONICAL_HOST])
# 
#     AC_MSG_CHECKING(for the IDL root directory)
#     if test -z "$IDL_ROOT"
#     then
#         # Find IDL's root directory by looking at the exectuable and then 
#         # finding where that symbolic link points.
#         # !!! Doesn't work if idl isn't a symbolic link - erd !!!
# 	# I think that the following 'if' fixes the symbolic link problem. 
# 	# 05/02/03 jhrg 
#         idl_loc=`which idl`
# 	if echo `ls -l $idl_loc` | grep '.*->.*' > /dev/null 2>&1
# 	then
#             idl_loc=`ls -l $idl_loc | sed 's/.*->[ ]*\(.*\)$/\1/'`
# 	fi
#         IDL_ROOT=`echo $idl_loc | sed 's/\(.*\)\/bin.*/\1/'`
#     fi
# 
#     AC_MSG_RESULT($IDL_ROOT)
#     AC_SUBST(IDL_ROOT)
# 
#     # Now find where the IDL 5.2 or later sharable libraries live.
#     # NB: This won't work if libraries for several architecutures are 
#     # installed for several machines.
#     AC_MSG_CHECKING(for the IDL sharable library directory)
#     # cd to the IDL root because it is likely a symbolic link and find 
#     # won't normally follow symbolic links.
#     IDL_LIBS=`(cd $IDL_ROOT; find . -name 'libidl.so' -print)`
#     # Strip off the leading `.' (it's there because we ran find in the CWD) 
#     # and the name of the library used to find the directory.
#     IDL_LIBS=`echo $IDL_LIBS | sed 's/\.\(.*\)\/libidl.so/\1/' | sed '1q'`
#     IDL_LIBS=${IDL_ROOT}${IDL_LIBS}
#     AC_MSG_RESULT($IDL_LIBS)
#     AC_SUBST(IDL_LIBS)])
# 
# 
# # 5. Misc stuff
# #---------------------------------------------------------------------------
# 
# AC_DEFUN([DODS_DEBUG_OPTION], [dnl
#     AC_ARG_ENABLE(debug, 
# 		  [  --enable-debug=ARG      Program instrumentation (1,2)],
# 		  DEBUG=$enableval, DEBUG=no)
# 
#     case "$DEBUG" in
#     no) 
#       ;;
#     1)
#       AC_MSG_RESULT(Setting debugging to level 1)
#       AC_DEFINE(DODS_DEBUG)
#       ;;
#     2) 
#       AC_MSG_RESULT(Setting debugging to level 2)
#       AC_DEFINE(DODS_DEBUG, , [Set instrumentation to level 1 (see debug.h)])
#       AC_DEFINE(DODS_DEBUG2, , [Set instrumentation to level 2])
#       ;;
#     *)
#       AC_MSG_ERROR(Bad debug value)
#       ;;
#     esac])
# 
AC_DEFUN([DODS_OS], [dnl
    AC_MSG_CHECKING(type of operating system)
    # I have removed the following test because some systems (e.g., SGI)
    # define OS in a way that breaks this code but that is close enough
    # to also be hard to detect. jhrg 3/23/97
    #  if test -z "$OS"; then
    #  fi 
    OS=`uname -s | tr '[[A-Z]]' '[[a-z]]' | sed 's;/;;g'`
    if test -z "$OS"; then
        AC_MSG_WARN(OS unknown!)
    fi
    case $OS in
        aix)
            ;;
        hp-ux)
            OS=hpux`uname -r | sed 's/[[A-Z.0]]*\([[0-9]]*\).*/\1/'`
            ;;
        irix)
            OS=${OS}`uname -r | sed 's/\..*//'`
            ;;
        # I added the following case because the `tr' command above *seems* 
	# to fail on Irix 5. I can get it to run just fine from the shell, 
	# but not in the configure script built using this macro. jhrg 8/27/97
        IRIX)
            OS=irix`uname -r | sed 's/\..*//'`
	    ;;
        osf*)
            ;;
        sn*)
            OS=unicos
            ;;
        sunos)
            OS_MAJOR=`uname -r | sed 's/\..*//'`
            OS=$OS$OS_MAJOR
            ;;
        ultrix)
            case `uname -m` in
            VAX)
                OS=vax-ultrix
                ;;
            esac
            ;;
        *)
            # On at least one UNICOS system, 'uname -s' returned the
            # hostname (sigh).
            if uname -a | grep CRAY >/dev/null; then
                OS=unicos
            fi
            ;;
    esac

    # Adjust OS for CRAY MPP environment.
    #
    case "$OS" in
    unicos)

        case "$CC$TARGET$CFLAGS" in
        *cray-t3*)
            OS=unicos-mpp
            ;;
        esac
        ;;
    esac

    AC_SUBST(OS)

    AC_MSG_RESULT($OS)])


AC_DEFUN([DODS_MACHINE], [dnl
    AC_MSG_CHECKING(type of machine)

    if test -z "$MACHINE"; then
    MACHINE=`uname -m | tr '[[A-Z]]' '[[a-z]]'`
    case $OS in
        aix*)
            MACHINE=rs6000
            ;;
        hp*)
            MACHINE=hp`echo $MACHINE | sed 's%/.*%%'`
            ;;
        sunos*)
            case $MACHINE in
                sun4*)
                    MACHINE=sun4
                    ;;
            esac
            ;;
        irix*)
            case $MACHINE in
                ip2?)
                    MACHINE=sgi
                    ;;
            esac
            ;;
	ultrix*)
	    case $MACHINE in
		vax*)
		     case "$CC" in
        		/bin/cc*|cc*)
echo "changing C compiler to \`vcc' because \`cc' floating-point is broken"
            		CC=vcc
v            		;;
		     esac
		     ;;
	    esac
	    ;;

    esac
    fi

    AC_SUBST(MACHINE)
    AC_MSG_RESULT($MACHINE)])

AC_DEFUN([DODS_CHECK_SIZES], [dnl
    # Ignore the errors about AC_TRY_RUN missing an argument. jhrg 5/2/95

    AC_REQUIRE([AC_PROG_CC])

    AC_CHECK_SIZEOF(int)
    AC_CHECK_SIZEOF(long)
    AC_CHECK_SIZEOF(char)
    AC_CHECK_SIZEOF(double)
    AC_CHECK_SIZEOF(float)
   
    # check for C99 types, headers and functions
    AC_CHECK_HEADER([inttypes.h],[dap_inttypes_header=yes],,) 
    # DINT32 4
    AC_CHECK_SIZEOF([int32_t])
    # DUINT32 
    AC_CHECK_SIZEOF([uint32_t])
    # DINT16 short
    AC_CHECK_SIZEOF([int16_t])
    # DUINT16 unsigned short
    AC_CHECK_SIZEOF([uint16_t])
    # DBYTE 1
    AC_CHECK_SIZEOF([uint8_t])
    if test x"$dap_inttypes_header" = x'yes' -a $ac_cv_sizeof_int32_t -eq 4 -a $ac_cv_sizeof_int16_t -eq 2 -a $ac_cv_sizeof_uint8_t -eq 1 -a $ac_cv_sizeof_double -eq 8; then
        dap_use_c99_types=yes
    fi
    # Force dods to always be c99
#    dap_use_c99_types=yes
    AM_CONDITIONAL([USE_C99_TYPES],[test x"$dap_use_c99_types" = 'xyes'])

    # I've separated the typedefs from the config.h header because other
    # projects which use the DAP were getting conflicts with their includes,
    # or the includes of still other libraries, and config.h. The 
    # config.h header is now included only by .cc and .c files and headers
    # that need the typedefs use dods-datatypes.h. 
    # there are 2 possibilities for the definition of dods_int32, ..., 
    # types. First possibility is that the C99 types are used and 
    # dods-datatypes-static.h is copied. In that case the following 
    # definitions are not really usefull. In case the C99 types are 
    # not available, dods-datatypes-config.h.in is used to generate
    # dods-datatypes.h.
    # The code below makes dods-datatypes-config.h stand on its own. 
    # 8/2/2000 jhrg

    # DMH: Divide into two sets of tests: one for DODS and one for XDR
    if test x"$dap_use_c99_types" = 'xyes'; then
        DODS_INT32=int32_t
        DODS_UINT32=uint32_t
        DODS_INT16=int16_t
        DODS_UINT16=uint16_t
        DODS_BYTE=uint8_t
    else
        DODS_INT16=short
        DODS_UINT16="unsigned short"
	DODS_INT32=int
	DODS_UINT32="unsigned int"
	DODS_BYTE="unsigned char"
    fi
    DODS_FLOAT64=double
    DODS_FLOAT32=float

    # I'm using the three arg form of AC_DEFINE_UNQUOTED because autoheader
    # needs the third argument (although I don't quite get the specifics... 
    # 2/15/2001 jhrg
    AC_DEFINE_UNQUOTED(DINT32, $DODS_INT32, [int32])
    AC_DEFINE_UNQUOTED(DUINT32, $DODS_UINT32, [uint32])
    AC_DEFINE_UNQUOTED(DINT16, $DODS_INT16, [dint16])
    AC_DEFINE_UNQUOTED(DUINT16, $DODS_UINT16, [uint16])
    AC_DEFINE_UNQUOTED(DFLOAT64, $DODS_FLOAT64, [dfloat64])
    AC_DEFINE_UNQUOTED(DFLOAT32, $DODS_FLOAT32, [dfloat32])
    AC_DEFINE_UNQUOTED(DBYTE, $DODS_BYTE, [dbyte])

    # XDR INTEGER TYPES
    # Unfortunately, there is little commonality about xdr

    # First, we need to see if the xdr routines are in libc, librpc,
    # or librpcsvc or libnsl
    dap_xdrlib=
    AC_SEARCH_LIBS([xdr_void],[c rpc nsl rpcsvc],[
      dap_xdrlib=`echo $ac_res|sed -e 's/^-l//'`],[
      AC_MSG_WARN(Cannot locate library containing xdr functions.)])
    if test "$dap_xdrlib" = "none required" ; then dap_xdrlib=c; fi
    if test "$dap_xdrlib" != "c" ; then
       # Add to library list
       AC_CHECK_LIB($dap_xdrlib,xdr_void)
    fi
    # Now figure out what integer functions to use
    dap_xdrint=0
    AC_CHECK_LIB($dap_xdrlib,[xdr_uint32_t],[dap_xdrint=1],[
      AC_CHECK_LIB($dap_xdrlib,[xdr_u_int32_t],[dap_xdrint=2],[
        AC_CHECK_LIB($dap_xdrlib,[xdr_uint],[dap_xdrint=3],[
          AC_CHECK_LIB($dap_xdrlib,[xdr_u_int],[dap_xdrint=4],[])])])])
    case "$dap_xdrint" in
    1) # uint32_t
	XDR_INT32=xdr_int32_t
	XDR_UINT32=xdr_uint32_t
        XDR_INT16=xdr_int16_t
        XDR_UINT16=xdr_uint16_t
        ;;
    2) # u_int32_t
	XDR_INT32=xdr_int32_t
	XDR_UINT32=xdr_u_int32_t
        XDR_INT16=xdr_int16_t
        XDR_UINT16=xdr_u_int16_t
        ;;
    3) # uint
	XDR_INT32=xdr_int
	XDR_UINT32=xdr_uint
        XDR_INT16=xdr_short
        XDR_UINT16=xdr_ushort
        ;;
    4) # u_int
	XDR_INT32=xdr_int
	XDR_UINT32=xdr_u_int
        XDR_INT16=xdr_short
        XDR_UINT16=xdr_u_short
        ;;
    *)
	AC_MSG_ERROR(Cannot determine DODS XDR integer sizes)
        ;;
    esac
    XDR_FLOAT64=xdr_double
    XDR_FLOAT32=xdr_float

    AC_DEFINE_UNQUOTED([XDR_INT16], [$XDR_INT16], [xdr int16])
    AC_DEFINE_UNQUOTED([XDR_UINT16], [$XDR_UINT16], [xdr uint16])
    AC_DEFINE_UNQUOTED([XDR_INT32], [$XDR_INT32], [xdr int32])
    AC_DEFINE_UNQUOTED([XDR_UINT32], [$XDR_UINT32], [xdr uint32])
    AC_DEFINE_UNQUOTED([XDR_FLOAT64], [$XDR_FLOAT64], [xdr float64])
    AC_DEFINE_UNQUOTED([XDR_FLOAT32], [$XDR_FLOAT32], [xdr float32])])
