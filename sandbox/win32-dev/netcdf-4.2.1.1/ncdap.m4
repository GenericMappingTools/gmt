AC_DEFUN([NCDAP_XML2], [dnl
  # Did user must specify a location for the XML2 library?
  AC_MSG_CHECKING([whether a location for the XML2 library was specified])
  AC_ARG_WITH([xml2], [AS_HELP_STRING([--with-xml2=<directory>],
              [Specify location of XML2 library. XML2 is required for opendap. Configure will expect to find subdirs include and lib.])],
              [xml2dir=$with_xml2])
  if test -n "$with_xml2" ; then
    AC_MSG_RESULT([$with_xml2])
  else
    AC_MSG_RESULT([no])
  fi
  if test -n "${with_xml2}" ; then
    xml2src="with"
  else
    # See if we can locate xml2-config; if so, then use it for at least cflags
    if xml2-config --version >/dev/null 2>&1 ; then
      xml2dir="`xml2-config --prefix`"  
      xml2src="config"
      AC_MSG_NOTICE([Found -lxml2 using xml2-config])
    else
      # see if autoconf can find it
      AC_MSG_NOTICE([Checking for -lxml2])
      AC_CHECK_LIB([xml2],[xmlParseFile],[xml2src="checklib"],[xml2src=])
      if test "$xml2src" = checklib; then
        AC_MSG_NOTICE([Found -lxml2 using checklib])
      fi
    fi
  fi
  if test -z "$xml2src"; then
    AC_MSG_NOTICE([xml2 library not found; continuing])
  fi
  if test "$xml2src" = "with" -o "$xml2src" = "config"; then
    xml2_cflags="-I${xml2dir}/include/libxml2"
    xml2_libs="-L${xml2dir}/lib -lxml2"
  fi
  if test "$xml2src" = "checklib"; then
    xml2_libs="-lxml2"
  fi
  AC_SUBST([XML2_CFLAGS],[${xml2_cflags}])
  AC_SUBST([XML2_LIBS],[${xml2_libs}])
  AC_SUBST([XML2DIR],[${xml2dir}])
  found_xml2=$xml2src
])  
AC_DEFUN([NCDAP_CURL], [dnl
  # Did user must specify a location for the CURL library?
  AC_MSG_CHECKING([whether a location for the CURL library was specified])
  AC_ARG_WITH([curl], [AS_HELP_STRING([--with-curl=<directory>],
              [Specify location of CURL library. CURL is required for opendap. Configure will expect to find subdirs include and lib.])],
              [curldir=$with_curl])
  if test -n "$with_curl" ; then
    AC_MSG_RESULT([$with_curl])
  else
    AC_MSG_RESULT([no])
  fi
  if test -n "${with_curl}" ; then
    curlsrc="with"
  else
    # See if we can locate curl-config; if so, then use it for at least cflags
    if curl-config --version >/dev/null 2>&1 ; then
      curldir="`curl-config --prefix`"  
      curlsrc="config"
      AC_MSG_NOTICE([Found -lcurl using curl-config])
    else
      # see if autoconf can find it
      AC_MSG_NOTICE([Checking for -lcurl])
      AC_CHECK_LIB([curl],[curl_easy_setopt],[curlsrc="checklib"],[curlsrc=])
      if test "$curlsrc" = checklib; then
        AC_MSG_NOTICE([Checklib found -lcurl])
      fi
    fi
  fi
  if test -z "$curlsrc"; then
    AC_MSG_NOTICE([curl library not found; continuing])
  fi
  if test "$curlsrc" = "with" -o "$curlsrc" = "config"; then
    curl_cflags="-I${curldir}/include"
    curl_libs="-L${curldir}/lib -lcurl"
  fi
  if test "$curlsrc" = "checklib"; then
    curl_libs="-lcurl"
  fi
  AC_SUBST([CURL_CFLAGS],[${curl_cflags}])
  AC_SUBST([CURL_LIBS],[${curl_libs}])
  AC_SUBST([CURLDIR],[${curldir}])
  found_curl=$curlsrc
])  
AC_DEFUN([NCDAP_ZLIB], [dnl
  # Did user must specify a location for the ZLIB library?
  AC_MSG_CHECKING([whether a location for the ZLIB library was specified])
  AC_ARG_WITH([zlib], [AS_HELP_STRING([--with-zlib=<directory>],
              [Specify location of ZLIB library. ZLIB may be required for opendap. Configure will expect to find subdirs include and lib.])],
              [zlibdir=$with_zlib])
  if test -n "$with_zlib" ; then
    AC_MSG_RESULT([$with_zlib])
  else
    AC_MSG_RESULT([no])
  fi
  if test -n "${with_zlib}" ; then
    zlibsrc="with"
  else
    # see if autoconf can find it
    AC_CHECK_LIB([z],[zlibVersion],[zlibsrc="checklib"],[zlibsrc=])
  fi
  if test -z "$zlibsrc"; then
    AC_MSG_NOTICE([zlib library not found; assume optional])
  fi
  if test "$zlibsrc" = "with"; then
    zlib_cflags="-I${zlibdir}/include"
    zlib_libs="-L${zlibdir}/lib -lz"
  fi
  if test "$zlibsrc" = "checklib"; then
    zlib_libs="-lz"
  fi
  AC_SUBST([ZLIB_CFLAGS],[${zlib_cflags}])
  AC_SUBST([ZLIB_LIBS],[${zlib_libs}])
  AC_SUBST([ZLIBDIR],[${zlibdir}])
  found_zlib=$zlibsrc
])  
AC_DEFUN([NCDAP_SZLIB], [dnl
  # Did user must specify a location for the SZLIB library?
  AC_MSG_CHECKING([whether a location for the SZLIB library was specified])
  AC_ARG_WITH([szlib], [AS_HELP_STRING([--with-szlib=<directory>],
              [Specify location of SZLIB library. SZLIB may be required for opendap. Configure will expect to find subdirs include and lib.])],
              [szlibdir=$with_szlib])
  if test -n "$with_szlib" ; then
    AC_MSG_RESULT([$with_szlib])
  else
    AC_MSG_RESULT([no])
  fi
  if test -n "${with_szlib}" ; then
    szlibsrc="with"
  else
    # see if autoconf can find it
    AC_CHECK_LIB([sz],[SZ_Decompress],[szlibsrc="checklib"],[szlibsrc=])
  fi
  if test -z "$szlibsrc"; then
    AC_MSG_NOTICE([szlib library not found; assume optional])
  fi
  if test "$szlibsrc" = "with"; then
    szlib_cflags="-I${szlibdir}/include"
    szlib_libs="-L${szlibdir}/lib -lz"
  fi
  if test "$szlibsrc" = "checklib"; then
    szlib_libs="-lsz"
  fi
  AC_SUBST([SZLIB_CFLAGS],[${szlib_cflags}])
  AC_SUBST([SZLIB_LIBS],[${szlib_libs}])
  AC_SUBST([SZLIBDIR],[${szlibdir}])
  found_szlib=$szlibsrc
])  
AC_DEFUN([NCDAP_OCLIB], [dnl
  # Did user must specify a location for liboc?
  AC_MSG_CHECKING([whether a location for liboc was specified])
  AC_ARG_WITH([oclib], [AS_HELP_STRING([--with-oclib=<directory>],
              [Specify location of liboc. Configure will expect to find subdirs include and lib. This is required for opendap; if not specified, then the internal version will be used. ])],
              [oclibdir=$with_oclib])
  if test -n "$with_oclib" ; then
    AC_MSG_RESULT([$with_oclib])
  else
    AC_MSG_RESULT([no])
  fi
  if test -n "${with_oclib}" ; then
    oclibsrc="with"
  else
    oclibsrc=
  fi
  if test -z "$oclibsrc"; then
    AC_MSG_NOTICE([external oclib library not specified; using internal version])
  fi
  if test "$oclibsrc" = "with"; then
    oclib_cflags="-I${oclibdir}/include"
    oclib_libs="-L${oclibdir}/lib -loc"
  fi
  AC_SUBST([OCLIB_CFLAGS],[${oclib_cflags}])
  AC_SUBST([OCLIB_LIBS],[${oclib_libs}])
  found_oclib=$oclibsrc
])  
AC_DEFUN([NCDAP_XDR], [dnl
    # See if autoconf can find it; check libc, librpc, librpcsvc and libnsl
    dap_xdrlib=
    AC_SEARCH_LIBS([xdr_void],[c rpc nsl rpcsvc],
      [dap_xdrlib=$ac_res],[dapxdrlib=])
    if test -z "$dap_xdrlib" ; then
      AC_MSG_ERROR(Cannot locate library containing xdr functions.)
    else
      dap_xdrlib=`echo $dap_xdrlib | sed -e s/-l//g`
      if test "$dap_xdrlib" = "-lc" ; then
        AC_MSG_NOTICE(XDR Functions appear to be in libc.)	
      elif test "$dap_xdrlib" = "none required" ; then
        AC_MSG_NOTICE(XDR Functions appear to be in libc.)	
      else
        AC_MSG_NOTICE(XDR Functions appear to be in lib${dap_xdrlib}.)	
        # Add to library list
        AC_CHECK_LIB($dap_xdrlib,xdr_void)
      fi
    fi
])  
