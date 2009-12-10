/* options.c - Parse command line options.
 *
 * Copyright (C) 2003  Jochen Voss.
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

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "parallel.h"


/* local copy of the command line argument list */
static char **xargv;
static int  xargc, xargv_len;

static void **pointers;
static int  p_allocated;


static void
remove_arg (int idx)
{
  assert (idx>0 && idx<xargc);

  --xargc;
  memmove (xargv+idx, xargv+idx+1, (xargc-idx)*sizeof(char *));
}

static void
insert_args (int idx, int n)
{
  assert (idx>0 && idx<=xargc);

  if (xargc+n > xargv_len) {
    xargv_len = xargc+n;
    xargv = xrenew (char *, xargv, xargv_len);
  }
  memmove (xargv+idx+n, xargv+idx, (xargc-idx)*sizeof(char *));
  xargc += n;
}

static void *
vmalloc (size_t size)
{
  pointers = xrenew (void *, pointers, p_allocated+1);
  return  pointers[p_allocated++] = xmalloc (size);
}

static int
check_long_opt (const struct voption *options, const char *str)
{
  int  j;

  for (j=0; options[j].name != NULL; ++j) {
    const char *ptr1 = options[j].name;
    const char *ptr2 = str;
    while (*ptr1 != '\0' && *ptr1 == *ptr2)  ++ptr1, ++ptr2;
    if (*ptr1 != '\0')  continue;
    if (*ptr2 == '\0' || (options[j].has_arg && *ptr2 == '='))  return  j;
  }
  return  -1;
}

static int
check_short_opt (const struct voption *options, const char *str)
{
  int  j;

  for (j=0; options[j].name != NULL; ++j) {
    if (str[0] == options[j].short_name
	&& (str[1] == '\0' || options[j].has_arg)) {
      return  j;
    }
  }
  return  -1;
}

static const char *
get_long_argument (int i)
{
  const char *ptr = strchr (xargv[i], '=');

  if (ptr != NULL) {
    ++ptr;
  } else {
    if (i+1 >= xargc) {
      error ("long option \"%s\" lacks an argument", xargv[i]);
      return  NULL;
    }
    ptr = xargv[i+1];
    remove_arg (i+1);
  }
  return  ptr;
}

static const char *
get_short_argument (int i)
{
  const char *ptr = xargv[i];

  if (ptr[2] != '\0') {
    ptr += 2;
  } else {
    if (i+1 >= xargc) {
      error ("option \"%s\" lacks an argument", xargv[i]);
      return  NULL;
    }
    ptr = xargv[i+1];
    remove_arg (i+1);
  }
  return  ptr;
}

static int
split_test (const struct voption *options, char c)
{
  int  j;

  for (j=0; options[j].name!=NULL; ++j) {
    if (options[j].short_name == c)  return  !(options[j].has_arg);
  }
  return  0;
}

static void
split_option (int i)
{
  char *str = xargv[i];
  insert_args (i+1, 1);
  xargv[i] = vmalloc (3);
  xargv[i][0] = '-';
  xargv[i][1] = str[1];
  xargv[i][2] = '\0';

  xargv[i+1] = vmalloc (strlen(str));
  xargv[i+1][0] = '-';
  strcpy (xargv[i+1]+1, str+2);
}

/**********************************************************************
 * global functions
 */

void
open_options (int argc, char **argv)
{
  xargc = argc;
  xargv = xnew (char *, argc);
  memcpy (xargv, argv, xargc*sizeof (char *));
}

void
close_options (void)
{
  int  i;

  for (i=0; i<p_allocated; ++i)  xfree (pointers[i]);
  xfree (pointers);
  xfree (xargv);
}

int
options_get (const struct voption *options,
	     const char **argptr,
	     int flags)
/* Parse 'xargv' to get the next option.  Return the short option
 * letter, if one option is recognised and cannot be handled
 * internally.  Return -1 on end of options.  If an option lacks an
 * argument, an error message is output, '*argptr' is set to 'NULL',
 * and 0 is returned.  If an unknown option is encountered and the
 * 'V_KEEP_UNKNOWN' flag is not set, '*argptr' is set to the offending
 * argument string and 0 is returned.  */
{
  int  i, j;

  *argptr = NULL;

  i = 1;

  while (i<xargc) {
    const char *str = xargv[i];

    if (str[0] != '-' || strcmp (str, "-") == 0) {
      if (flags & V_MIXED) {
	++i;
	continue;
      }
      break;
    }
    if (strcmp (str, "--") == 0) {
      remove_arg (i);
      break;
    }

    j = -1;
    if (str[1] == '-') {
      j = check_long_opt (options, str+2);
      if (j >= 0 && options[j].has_arg)  *argptr = get_long_argument (i);
    } else if (flags & V_ONE_HYPHEN) {
      j = check_long_opt (options, str+1);
      if (j >= 0 && options[j].has_arg)  *argptr = get_long_argument (i);
    }
    if (j < 0) {
      j = check_short_opt (options, str+1);
      if (j >= 0 && options[j].has_arg)  *argptr = get_short_argument (i);
    }

    if (j < 0) {
      if (str[1] != '-'
	  && str[2] != '\0'
	  && ! (flags & V_NO_JOIN)
	  && (! V_ONE_HYPHEN || split_test (options, str[1])) ) {
	split_option (i);
	continue;
      }
      if (flags & V_KEEP_UNKNOWN) {
	++i;
	continue;
      } else {
	*argptr = str;
	remove_arg (i);
	return  0;
      }
    } else {
      remove_arg (i);
      if (options[j].flag_ptr) {
	*(options[j].flag_ptr) = 1;
	if (! options[j].has_arg)  continue;
      }
      if (options[j].has_arg && *argptr == NULL)  return  0;
      return  options[j].short_name;
    }
  }

  return  -1;
}

void
options_show (const struct voption *options)
/* Emit a list of options to stderr for use in the help text.  */
{
  int  i;

  fputs ("The following options are available:\n", stderr);
  for (i=0; options[i].name != NULL; ++i) {
    if (options[i].has_arg) {
      char  buffer [80];
      sprintf (buffer, "%s=%s", options[i].name, options[i].arg_name);
      fprintf (stderr, "  -%c, --%-16s %s\n",
	       options[i].short_name, buffer, options[i].help);
    } else {
      fprintf (stderr, "  -%c, --%-16s %s\n",
	       options[i].short_name, options[i].name, options[i].help);
    }
  }
}
