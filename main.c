/* main.c -
 *
 * Copyright (C) 2009  Jochen Voss.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "parallel.h"


/**********************************************************************
 * main program
 */

int
main(int argc, char **argv)
{
  int  error_flag = 0;
  int  help_flag = 0;
  long  nprocs = 0;
  char *cf_name = NULL;
  int  verbose_flag = 0;
  int  version_flag = 0;
  const char *optarg;
  struct voption options [] = {
    { "help", 'h', &help_flag, 0, NULL,
      "show this message" },
    { "nprocs", 'n', NULL, 1, "N",
      "maxmimal number of parallel processes" },
    { "commands", 'c', NULL, 1, "FNAME",
      "read commands from FNAME instead of from stdin" },
    { "verbose", 'v', &verbose_flag, 0, NULL,
      "emit messages to stdout" },
    { "version", 'V', &version_flag, 0, NULL,
      "show version information" },
    { NULL, '\0', NULL, 0, NULL, NULL }
  };
  struct cf *cf;

  open_options(argc, argv);
  do {
    int  c = options_get(options, &optarg, V_MIXED);
    if (c == -1)  break;

    switch (c) {
    case 'n':
      {
	char *tail;
	errno = 0;
	nprocs = strtol(optarg, &tail, 0);
	if (tail==optarg || *tail!=0 || errno || nprocs<1) {
	  error("error: invalid number of parallel processes \"%s\"", optarg);
	  error_flag = 1;
	}
      }
      break;
    case 'c':
      cf_name = xstrdup(optarg);
      break;
    case '\0':
      if (optarg)
	error("error: unknown option \"%s\"", optarg);
      error_flag = 1;
      break;
    default:
      fatal("error: internal error while parsing options");
    }
  } while (! error_flag);

  if (version_flag) {
    puts("parallel " VERSION);
    puts("Copyright(C) 2009 Jochen Voss <voss@seehuhn.de>");
    puts("\
Parallel comes with ABSOLUTELY NO WARRANTY, to the extent permitted by law.");
    puts("\
You may redistribute copies of Parallel under the terms of the GNU\n\
General Public License.  For more information about these matters, see\n\
the file named COPYING.");
    if (! error_flag)  exit(0);
  }

  if (error_flag || help_flag) {
    FILE *out = error_flag ? stderr : stdout;
    fprintf(out, "usage: %s [options]\n\n", argv[0]);
    options_show(options);
    fputs("\nPlease report bugs to Jochen Voss <voss@seehuhn.de>.\n", out);
    exit(error_flag);
  }
  close_options();

  if (nprocs == 0) {
    nprocs = sysconf(_SC_NPROCESSORS_CONF);
  }
  printf("** %ld parallel processes\n", nprocs);

  cf = new_cf(cf_name);
  if (! cf)
    fatal("error: cannot open command file \"%s\"", cf_name);

  int i;
  for (i=0; i<5; ++i) {
    puts(cf_next(cf));
  }

  if (cf_is_incomplete(cf)) {
    error("error: incomplete line at the end of command file (ignored)");
  }

  delete_cf(cf);
  xfree(cf_name);
  return 0;
}
