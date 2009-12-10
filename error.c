/* error.c - handle jvterm error messages
 *
 * Copyright (C) 2001, 2003  Jochen Voss.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "parallel.h"


void
fatal (const char *format, ...)
/* Signal a fatal error and quit immediately.
 * 'format' should be a printf format string, starting with a
 * lower-case letter.  Both, the format and the arguments should be
 * encoded using the 'parent_CHARSET'.  */
{
  va_list  ap;
  char *msg;

  va_start (ap, format);
  vasprintf (&msg, format, ap);
  va_end (ap);
  log_write_line (log_ERROR, msg);
  free (msg);

  abort ();
}

void
error (const char *format, ...)
/* Emit an error message and continue without further consequence.
 * 'format' should be a printf format string, starting with a
 * lower-case letter.  Both, the format and the arguments should be
 * encoded using the 'parent_CHARSET'.  */
{
  va_list  ap;
  char *msg;

  va_start (ap, format);
  vasprintf (&msg, format, ap);
  va_end (ap);
  log_write_line (log_ERROR, msg);
  free (msg);
}

void
warning (const char *format, ...)
/* Emit a warning and continue without further consequence.
 * 'format' should be a printf format string, starting with a
 * lower-case letter.  Both, the format and the arguments should be
 * encoded using the 'parent_CHARSET'.  */
{
  va_list  ap;
  char *msg;

  va_start (ap, format);
  vasprintf (&msg, format, ap);
  va_end (ap);
  log_write_line (log_WARNING, msg);
  free (msg);
}

void
message (const char *format, ...)
/* Emit a message and continue without further consequence.
 * 'format' should be a printf format string, starting with a
 * lower-case letter.  Both, the format and the arguments should be
 * encoded using the 'parent_CHARSET'.  */
{
  va_list  ap;
  char *msg;

  va_start (ap, format);
  vasprintf (&msg, format, ap);
  va_end (ap);
  log_write_line (log_MESSAGE, msg);
  free (msg);
}