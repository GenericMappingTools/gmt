#ifdef __cplusplus /* Basic C++ support */
extern "C" {
#endif

/* CMake definitions: This must be first! */
#include "gmt_config.h"

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

#include <stdio.h>

EXTERN_MSC int func1 ();
EXTERN_MSC int func2 ();

#ifdef __cplusplus /* Basic C++ support */
}
#endif

int func1 () {
	printf ("func1 in 2\n");
	return 1;
}

int func2 () {
	printf ("func2\n");
	return 1;
}
