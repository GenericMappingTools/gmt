
#ifndef _UTILITY_
#define _UTILITY_

#ifndef FILE
#include <stdio.h>
#endif

#define Max(a, b) ((a)>(b)?(a):(b))
#define Min(a, b) ((a)<(b)?(a):(b))

#define RoundUp(n, unit) ((((n) + (unit) - 1) / (unit)) * (unit))
#define RoundDown(n, unit) (((n) / (unit)) * (unit))

extern FILE * CheckOpen (String fileName, String access);

extern void CheckReadLine (FILE * file, char line[], int maxLength);

extern void Trace (String message);

#ifndef __GNUC__
#define __attribute__(x)
#endif
	
#endif

