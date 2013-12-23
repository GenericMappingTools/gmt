/* $Id$
 *
 * Copyright (c) 2012-2013
 * by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
 * See LICENSE.TXT file for copying and redistribution conditions.
 */

/* Here are Windows implementations of standard POSIX shared
 * library function.  Borrowed from this website:
 * http://www.refcode.net/2013/02/posix-dynamic-library-loading-calls-for.html
 * which says it is open source.
 */

#include "gmt_lib.h"
#include "gmt_sharedlibs.h" 	/* Common shared libs structures */

#if defined(_WIN32)
void *dlopen (const char *module_name, int mode)
{	/* Opens a dll file*/
	UINT err_code;
	HINSTANCE dll_handle;
  
	err_code = SetErrorMode (SEM_FAILCRITICALERRORS);
	dll_handle = LoadLibrary (module_name);
	if (!dll_handle) 
	{
		dll_handle = LoadLibraryEx (module_name, NULL, 0);
		if (!dll_handle)
			return (void *)dll_handle;
 	}

	/* Clear the last error*/
	SetLastError (0); 
	return (void *)dll_handle;
}

int dlclose (void *handle)
{	/* Closes handle */
	/* POSIX call returns zero for success, non-zero for failure */
	return (!FreeLibrary (handle)); 
}

void *dlsym (void *handle, const char *name)
{	/* Get a symbol from dll */
	return GetProcAddress (handle, name);
}

char *dlerror (void)
{	/* Reports last error occured */
	int len, error_code;
	static char errstr[128];
        
	if ((error_code = GetLastError ()) == 0)
		return NULL;

	/* POSIX dlerror call needs to report no error (null) 
	   when it is called 2nd time consequently, so clear error */
	SetLastError (0); 

	/* Format the error string */
	len = sprintf (errstr, "Error <%d>: ", error_code);
	len += FormatMessage ( 
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
		(LPTSTR) errstr + len,
		sizeof(errstr) - len,
		NULL 
		);
    
	/* Replace \r\n */
	if (len > 1 && errstr[len-2] == '\r' && errstr[len-1] == '\n') {
		if (len > 2 && errstr[len-3] == '.')
			len--;
	}

	return errstr;
}

/* Extra convenience function for opening DLL of current process */
HINSTANCE GetMyModuleHandle() {
	/* http://stackoverflow.com/questions/846044/how-to-get-the-filename-of-a-dll */
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(GetMyModuleHandle, &mbi, sizeof(mbi));
	return (HINSTANCE) (mbi.AllocationBase);
}
void *dlopen_special(const char *name)
{	/* Opens the dll file of the current process.  This is how it is done
	 * under Windows, per http://en.wikipedia.org/wiki/Dynamic_loading */
	/*HMODULE this_process, this_process_again;
	GetModuleHandleEx (0, 0, &this_process);
	this_process_again = GetModuleHandle (NULL);
	return (this_process_again);*/
	HINSTANCE this_dll_process;
	this_dll_process = GetMyModuleHandle();
	return (this_dll_process);
}
#elif defined(__CYGWIN__)
	/* Cygwin behaves differently than most Unix and we must use regular dlopen with library name */
void *dlopen_special(const char *name)
{	/* Opens the shared library file of the current process under *nix.
	 * Just call dlopen with NULL and RTLD_LAZY */
	return (dlopen (name, RTLD_LAZY));
}
#else

/* Extra convenience function for opening shared library of current process */

void *dlopen_special(const char *name)
{	/* Opens the shared library file of the current process under *nix.
	 * Just call dlopen with NULL and RTLD_LAZY */
	return (dlopen (NULL, RTLD_LAZY));
}
#endif
