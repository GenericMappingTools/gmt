/* $Id$
 *
 * Copyright (c) 2012-2013
 * by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
 * See LICENSE.TXT file for copying and redistribution conditions.
 */

/* Here are Windows implementation of POSIX shared library function.
 * Borrowed from http://www.refcode.net/2013/02/posix-dynamic-library-loading-calls-for.html
 * which says it is open source.
 */

#include "gmt_dev.h"

#if defined(_WIN32)
/**dlopen: opens a dll file*/
void *dlopen(const char *module_name, int mode)
{
	UINT err_code;
	HINSTANCE dll_handle;
  
	err_code = SetErrorMode(SEM_FAILCRITICALERRORS);
	dll_handle = LoadLibrary(module_name);
	if (!dll_handle) 
	{
		dll_handle = LoadLibraryEx(module_name, NULL, 0);
		if(!dll_handle)
			return (void *)dll_handle;
 	}

	/*clear the last error*/
	SetLastError(0); 
	return (void *)dll_handle;
}

/** dlclose: closes handle*/
int dlclose(void *handle)
{
	/*POSIX call returns zero for success, non-zero for failure*/
	return (!FreeLibrary(handle)); 
}

/** dlsym: get a symbol from dll*/
void *dlsym(void *handle, const char *name)
{
	return GetProcAddress(handle, name);
}

/**dlerror: reports last error occured*/
char *dlerror(void)
{
	int len, error_code;
	static char errstr[128];
        
	if((error_code = GetLastError()) == 0)
		return((char *)0);

	/*POSIX dlerror call needs to report no error(null) 
	  when it is called 2nd time consequently, so clear error*/
	SetLastError(0); 

	/*format the error string*/
	len = sprintf(errstr, "Error <%d>: ", error_code);
	len += FormatMessage( 
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
		LPTSTR) errstr + len,
		sizeof(errstr) - len,
		NULL 
		);
    
	/*replace \r\n*/
	if (len > 1 && errstr[len-2] == '\r' && errstr[len-1] == '\n') {
		if (len > 2 && errstr[len-3] == '.')
			len--;
	}

	return errstr;
}
#endif
