
#include <stdio.h>
#include <ctype.h>

#include <X11/Intrinsic.h>

#include "xgrid_utility.h"

FILE * CheckOpen (String fileName, String access)
{
  FILE * result;
  char   msg[256];
  
  result = fopen(fileName, access);
  if (result == NULL) {
    sprintf(msg, "Opening file %s", fileName);
    XtError(msg);
    }
  return result;
}

void CheckReadLine (FILE *file, char line[], int maxLength)
{
  char * result;
  
  result = fgets(line, maxLength, file);
  if (result == NULL)
    XtError("Null returned by fgets");
}

void Trace (String message)
{
  fprintf(stderr, "%s\n", message);
  fflush(stderr);
}

