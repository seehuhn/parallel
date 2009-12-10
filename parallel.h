/* parallel.h -
 *
 * Copyright (C) 2009  Jochen Voss.
 */

#ifndef FILE_PARALLEL_H_SEEN
#define FILE_PARALLEL_H_SEEN

#if __GNUC__ >= 3
#define  jv_pure  __attribute__ ((pure))
#define  jv_const  __attribute__ ((const))
#define  jv_noreturn  __attribute__ ((noreturn))
#define  jv_malloc  __attribute__ ((malloc))
#define  jv_printf  __attribute__ ((format (printf, 1, 2)))
#define  jv_must_check  __attribute__ ((warn_unused_result))
#define  jv_unused  __attribute__ ((unused))
#else
#define  jv_pure  /* no pure */
#define  jv_const  /* no const */
#define  jv_noreturn  /* no noreturn */
#define  jv_malloc  /* no malloc */
#define  jv_printf  /* no printf */
#define  jv_must_check  /* no warn_unused_result */
#define  jv_unused  /* no unused */
#endif


/* log.c */

/* log levels:
 *   log_MESSAGE = debugging and status information,
 *                 not normally presented to the user
 *                 (may be emitted via stdout)
 *   log_WARNING = warnings, only presented to the user in
 *                 verbose mode (may be emitted via stderr)
 *   log_ERROR   = errors which the user should know about
 *                 (may be emitted via stderr)
 */
enum loglevel { log_MESSAGE, log_WARNING, log_ERROR };
typedef  void  (*write_line_fn) (enum loglevel level, const char *message,
				 void *client_data);

extern  void  open_log (void);
extern  void  close_log (void);

extern  void  log_up_and_running (void);
extern  int  log_add_client (write_line_fn write_line, void *client_data);
extern  void  log_remove_client (int handle);

extern  void  log_write_line (enum loglevel level, const char *message);


/* error.c */

extern  void  fatal (const char *format, ...) jv_noreturn jv_printf;
extern  void  error (const char *format, ...) jv_printf;
extern  void  warning (const char *format, ...) jv_printf;
extern  void  message (const char *format, ...) jv_printf;


/* xmalloc.c */

extern  void *xmalloc (size_t size) jv_malloc;
#define xnew(T,N) ((T *)xmalloc((N)*sizeof(T)))
extern  void *xrealloc (void *ptr, size_t newsize);
#define xrenew(T,OLD,N) ((T *)xrealloc(OLD,(N)*sizeof(T)))
extern  void  xfree (void *ptr);
extern  char *xstrdup (const char *s) jv_malloc;


/* options.c */

struct voption {
  const char  *name;		/* long option name */
  char  short_name;		/* short option name */
  int *flag_ptr;		/* where is the flag */
  int  has_arg;			/* Is there an argument? */
  const char *arg_name;		/* name of the argument */
  const char *help;		/* "--help" line */
};

#define V_MIXED		1
#define V_ONE_HYPHEN	2
#define V_KEEP_UNKNOWN	4
#define V_NO_JOIN	8

extern  void  open_options (int argc, char **argv);
extern  void  close_options (void);
extern  int  options_get (const struct voption *options,
			  const char **argptr, int flags);
extern  void  options_show (const struct voption *options);

#endif /* FILE_PARALLEL_H_SEEN */
