/*
 * $Id$
 */

#define HAVE_SVN_VERSION @HAVE_SVN_VERSION@
#define SVN_VERSION_STRING "@SVN_VERSION@"

#define PACKAGE_VERSION "@GMT_PACKAGE_VERSION@"
#if HAVE_SVN_VERSION
#	define GMT_STRING "@GMT_PACKAGE_NAME@ @GMT_PACKAGE_VERSION@ (SVN)"
#	define MANDATE __DATE__
#else
#	define GMT_STRING "@GMT_PACKAGE_NAME@ @GMT_PACKAGE_VERSION@"
#	define MANDATE "@MANDATE@"
#endif

#ifdef __LP64__
#	define GMT_VER_64 " [64-bit]"
#else
#	if defined(WIN32) && defined(WINBITAGE)
#		if (WINBITAGE == 64)
#			define GMT_VER_64 " [Win64-bit]"
#		else
#			define GMT_VER_64 " [Win32-bit]"
#		endif
#	else
#		define GMT_VER_64 ""
#	endif
#endif

#ifdef GMT_COMPAT
#	define GMT_VER_COMPAT " [C4]"
#else
#	define GMT_VER_COMPAT ""
#endif

#define GMT_VERSION GMT_version()
#define GMT_MAJOR_VERSION @GMT_PACKAGE_VERSION_MAJOR@
#define GMT_MINOR_VERSION @GMT_PACKAGE_VERSION_MINOR@
#define GMT_RELEASE_VERSION @GMT_PACKAGE_VERSION_PATCH@
#define GMT_VERSION_YEAR @GMT_VERSION_YEAR@
#define GSHHS_VERSION "@GSHHS_VERSION@"

#ifdef _OPENMP	/* Open MP Parallelization is on */
#	define GMT_MP "[MP]"
#else
#	define GMT_MP ""
#endif

/* vim: set ft=c: */
