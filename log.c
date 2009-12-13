/* log.c - dispatch all log messages to the appropriate channels
 *
 * Copyright (C) 2003, 2009  Jochen Voss.
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
#include <stdlib.h>
#include <string.h>

#include "parallel.h"


struct logclient {
  struct logclient *next;
  write_line_fn  write_line;
  void *client_data;
  int  handle;
};

struct logline {
  struct logline *next;
  enum loglevel  level;
  char *message;
};

struct logdomain {
  struct logclient *clients;
  struct logline *backlog;
  int  backlog_flag;
  int  handle_gen;
};
static struct logdomain *log;

void
open_log(void)
{
  log = xnew(struct logdomain, 1);
  log->clients = NULL;
  log->backlog = NULL;
  log->backlog_flag = 1;
  log->handle_gen = 0;
}

static void
clear_backlog(void)
{
  struct logline *bptr;

  bptr = log->backlog;
  while (bptr) {
    struct logline *btmp = bptr->next;
    xfree(bptr->message);
    xfree(bptr);
    bptr = btmp;
  }
  log->backlog = NULL;
}

void
close_log(void)
{
  struct logclient *cptr;

  cptr = log->clients;
  while (cptr) {
    struct logclient *ctmp = cptr->next;
    xfree(cptr);
    cptr = ctmp;
  }
  clear_backlog();
  xfree(log);
  log = NULL;
}

void
log_up_and_running(void)
{
  log->backlog_flag = 0;
  clear_backlog();
}

int
log_add_client(write_line_fn write_line, void *client_data)
{
  struct logclient *client;
  struct logline *bptr;

  client = xnew(struct logclient, 1);
  client->next = log->clients;
  client->write_line = write_line;
  client->client_data = client_data;
  client->handle = log->handle_gen++;

  log->clients = client;

  bptr = log->backlog;
  while (bptr) {
    write_line(bptr->level, bptr->message, client_data);
    bptr = bptr->next;
  }

  return  client->handle;
}

void
log_remove_client(int handle)
{
  struct logclient **cpptr = &(log->clients);
  while (*cpptr) {
    if ((*cpptr)->handle == handle) {
      struct logclient *ctmp;
      ctmp = *cpptr;
      *cpptr = (*cpptr)->next;
      xfree(ctmp);
      return;
    }
    cpptr = &((*cpptr)->next);
  }
}

void
log_write_line(enum loglevel level, const char *message)
/* Write out the UTF-8 encoded MESSAGE to all clients.  */
{
  struct logclient *cptr;

  if (! log) {			/* emergency logging */
    fputs(message, stderr);
    fputc('\n', stderr);
    return;
  }

  if (log->backlog_flag) {
    struct logline *line;
    struct logline **lpptr = &(log->backlog);

    line = xnew(struct logline, 1);
    line->next = NULL;
    line->level = level;
    line->message = xstrdup(message);

    while (*lpptr)  lpptr = &((*lpptr)->next);
    *lpptr = line;
  }

  cptr = log->clients;
  while (cptr) {
    cptr->write_line(level, message, cptr->client_data);
    cptr = cptr->next;
  }
}
