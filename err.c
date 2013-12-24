#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "err.h"

extern int sys_nerr;

void syserr(const char *fmt, ...)
{
	va_list fmt_args;

	fprintf(stderr, "ERROR: ");

	va_start(fmt_args, fmt);
	vfprintf(stderr, fmt, fmt_args);
	va_end (fmt_args);

	fprintf(stderr," (%d; %s)\n", errno, strerror(errno));

	exit(1);
}

void err(int b, const char *fmt, ...)
{
	va_list fmt_args;

	fprintf(stderr, "ERROR: ");

	va_start(fmt_args, fmt);
	vfprintf(stderr, fmt, fmt_args);
	va_end (fmt_args);

	fprintf(stderr," (%d; %s)\n", b, strerror(b));

	exit(1);
}

void fatal(const char *fmt, ...)
{
	va_list fmt_args;

	fprintf(stderr, "FATAL: ");

	va_start(fmt_args, fmt);
	vfprintf(stderr, fmt, fmt_args);
	va_end (fmt_args);

	fprintf(stderr," (%d; %s)\n", errno, strerror(errno));
}

void mfatal(int b, const char *fmt, ...)
{
	va_list fmt_args;

	fprintf(stderr, "FATAL: ");

	va_start(fmt_args, fmt);
	vfprintf(stderr, fmt, fmt_args);
	va_end (fmt_args);

	fprintf(stderr," (%d; %s)\n", b, strerror(b));
}


