/* test_common.c - utility functions for xdr tests
 * Copyright (c) 2009 Charles S. Wilson
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#include "test_common.h"
#include "test_data.h"

void
set_program_name (const char *name)
{
  program_name = base_name (name);
  if (!program_name || !*program_name)
    program_name = "<unknown program name>";
}

const char *
base_name (const char *name)
{
  const char *base;
#if defined (HAVE_DOS_BASED_FILE_SYSTEM)
  /* Skip over the disk name in MSDOS pathnames. */
  if (isalpha ((unsigned char) name[0]) && name[1] == ':')
    name += 2;
#endif

  for (base = name; *name; name++)
    if (IS_DIR_SEPARATOR (*name))
      base = name + 1;
  return base;
}

int
mkdir_p (const char *path, int mode)
{
  int len;
  char *new_path;
  int ret = 0;

  new_path = (char *) malloc (strlen(path) + 1);
  strcpy (new_path, path);

  len = strlen (new_path);
  while (len > 0 && IS_DIR_SEPARATOR(new_path[len-1]))
    {
      new_path[len-1] = 0;
      len--;
    }

#if defined(_MSC_VER) || defined(__MINGW32__)
  while (!_mkdir (new_path))
#else
  while (!mkdir (new_path, mode))
#endif
    {
      char *slash;
      int last_error = errno;
      if (last_error == EEXIST)
        break;
      if (last_error != ENOENT)
        {
          ret = -1;
          break;
        }
      slash = new_path + strlen (new_path);
      while (slash > new_path && !IS_DIR_SEPARATOR(*slash))
        slash--;
      if (slash == new_path)
        {
          ret = -1;
          break;
        }
      len = slash - new_path;
      new_path[len] = 0;
      if (!mkdir_p (new_path, mode))
        {
          ret = -1;
          break;
        }
      new_path[len] = '/';
    }
    free (new_path);
    return ret;
}

static void
puthex (long n, unsigned int digits, char *buf, unsigned int pos)
{
  if (digits > 1)
    puthex (n / 16, digits - 1, buf, pos);
  buf[pos + digits - 1] = "0123456789abcdef"[n % 16];
}

void
dumpmem (FILE * f, void *buf, unsigned int len, unsigned int offset)
{
  unsigned int i;
  unsigned int address;
  unsigned char *p;
  static const unsigned int MASK_LOWER = 0x0f;

  char line[80];

  if (offset >= len)
    return;

  address = offset;
  p = ((unsigned char *) buf) + offset;

  fputs("  Addr     0 1  2 3  4 5  6 7  8 9  A B  C D  E F 0 2 4 6 8 A C E \n",f);
  fputs("--------  ---- ---- ---- ---- ---- ---- ---- ---- ----------------\n",f);

  while (p < ((unsigned char *) buf) + len)
    {
      for (i = 0; i < 50; i++)
        line[i] = ' ';
      for (; i < 80; i++)
        line[i] = 0;
      if ((address & ~MASK_LOWER) != address)   /* address % 16 != 0 */
        {
          puthex ((address & ~MASK_LOWER), 8, line, 0);
          for (i = 0; i < (address & MASK_LOWER); i++)
            {
              line[10 + i * 2 + i / 2] = ' ';
              line[10 + i * 2 + i / 2 + 1] = ' ';
              line[50 + i] = ' ';
            }
          address = address & ~MASK_LOWER;
        }
      else
        {
          puthex (address, 8, line, 0);
          i = 0;
        }

      for (; i < 16; i++)
        {
          puthex (((long) *p) & 0x0ff, 2, line, 10 + i * 2 + i / 2);
          line[50 + i] = '.';
          if (isprint (*p))
            line[50 + i] = *p;
          if (++p >= (unsigned char *) buf + len)
            break;
        }
      fputs (line, f);
      fputs ("\n", f);
      address += 16;
    }
}

void
log_msg (log_opts * o, int level, const char *fmt, ...)
{
  va_list ap;
  if (o->level >= level)
    {
      va_start (ap, fmt);
      vfprintf (o->f, fmt, ap);
      va_end (ap);
    }
}

