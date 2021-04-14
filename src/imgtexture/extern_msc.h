#ifndef _DECLSPEC_H_
#	define _DECLSPEC_H_

/*
 * When an application links to a DLL in Windows, the symbols that
 * are imported have to be identified as such.
 */

#	ifdef _WIN32
#		ifdef LIBRARY_EXPORTS
#			define LIBSPEC __declspec(dllexport)
#		else
#			define LIBSPEC __declspec(dllimport)
#		endif /* ifdef LIBRARY_EXPORTS */
#	else /* ifdef _WIN32 */
#		define LIBSPEC
#	endif /* ifdef _WIN32 */

/* By default, we use the standard "extern" declarations. */
#	define EXTERN_MSC extern LIBSPEC

#endif /* _DECLSPEC_H_ */
