/* ttt3_arch_ms.h
 *
 * tttA_arch_ms.h - include file for ttt.h
 * Specially set for Windows
 *
 * Here lies anything that has to do with architecture-dependency
 * and things outside POSIX.
 *
 * Paul Wessel, Geoware, January 1, 2024.
 *
 */

/* Turn off some annoying "security" warnings in Visual Studio */

#pragma warning( disable : 4996 )

/* HAVE_<func> is set to 0 (FALSE) or 1 (TRUE) depending on whether or
 * not <func> is available on this system.  The default setting is 0:
 * none of these functions are available in the POSIX standard. */

#define HAVE_HYPOT 1
#define HAVE_RINT 0
#define HAVE_IRINT 0
#define HAVE_ISNANF 0
#define HAVE_ISNAND 0
#define HAVE_ISNAN 1
#define WORDS_BIGENDIAN 0

#ifdef _WIN64
#define BIT_SETTING 1
#else
#define BIT_SETTING 2
#endif
#define TTT_DIR_DEFAULT "C:/Geoware/share/ttt/data"
