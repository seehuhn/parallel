/* cf.c - code to read the parallel command file
 *
 * Copyright (C) 2009  Jochen Voss.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include "parallel.h"


struct cf {
  FILE *fd;
  char *buffer;
  unsigned long allocated, used, pos;
  int has_error;
};

struct cf *
new_cf(const char *fname)
{
  FILE *fd;
  struct cf *res;

  if (fname)
    fd = fopen(fname, "r");
  else
    fd = stdin;
  if (! fd) {
    return NULL;
  }

  res = xnew(struct cf, 1);
  res->fd = fd;
  res->allocated = 512;
  res->buffer = xnew(char, res->allocated);
  res->used = 0;
  res->pos = 0;
  res->has_error = 0;
  return res;
}

void
delete_cf(struct cf *cf)
{
  int rc;

  if (cf->fd != stdin) {
    rc = fclose(cf->fd);
    if (rc) {
      warning("warning: error while closing command file (ignored)");
    }
  }
  xfree(cf->buffer);
  xfree(cf);
}

static void
cf_skip_whitespace(struct cf *cf)
{
  while (cf->pos < cf->used && isspace(cf->buffer[cf->pos]))
    ++cf->pos;
}

static char *
cf_extend(struct cf *cf)
/* Load more data in the buffer, until the next line is complete.  The
 * return value is a pointer to the newline character terminating the
 * line.  */
{
  unsigned long len;

  len = cf->used - cf->pos;
  memmove(cf->buffer, cf->buffer+cf->pos, len);
  cf->used = len;
  cf->pos = 0;

  for (;;) {
    char *eol;
    size_t rc;

    eol = memchr(cf->buffer+cf->pos, '\n', cf->used - cf->pos);
    if (eol)
      return eol;

    if (cf->has_error)
      return NULL;

    if (cf->used == cf->allocated) {
      cf->allocated *= 2;
      cf->buffer = xrenew(char, cf->buffer, cf->allocated);
    }

    clearerr(cf->fd);
    rc = fread(cf->buffer + cf->used, 1, cf->allocated - cf->used, cf->fd);

    if (rc > 0) {
      cf->used += rc;
      cf_skip_whitespace(cf);
      continue;
    }
    if (errno == EINTR) {
      clearerr(cf->fd);
      continue;
    }

    if (ferror(cf->fd)) {
      error("error: read from command file failed (%m)");
      cf->has_error = 1;
    }
    return NULL;
  }
}

const char *
cf_next(struct cf *cf)
{
  const char *base;
  char *eol;

  base = cf->buffer+cf->pos;
  eol = memchr(base, '\n', cf->used-cf->pos);
  if (! eol) {
    eol = cf_extend(cf);
    base = cf->buffer+cf->pos;
  }
  if (! eol)
    return NULL;
  assert(! isspace(*base));
  assert(*eol == '\n');

  cf->pos = (eol+1) - cf->buffer;
  cf_skip_whitespace(cf);

  while (eol > base && isspace(*(eol-1)))
    --eol;
  *eol = '\0';
  return base;
}

int
cf_is_incomplete(const struct cf *cf)
{
  if (! feof(cf->fd))
    return 0;			/* eof not reached */
  if (memchr(cf->buffer+cf->pos, '\n', cf->used-cf->pos))
    return 0;			/* at least one line available */
  if (cf->pos == cf->used)
    return 0;			/* all data processed */
  return 1;
}
