// $Id: fcicomp_log.c 778 2016-03-09 07:56:29Z delaunay $
// =============================================================
//
// PROJECT : FCICOMP
//
// AUTHOR : THALES Services
//
// Copyright 2015 EUMETSAT
//
// =============================================================
// HISTORY :
//
// VERSION:1.0.1:NCR:FCICOMP-8:09/03/2016:Add the copyright notice in the header
//
// END-HISTORY
// =============================================================

#include <stdio.h>
#include <stdarg.h>
#include "fcicomp_log.h"

#ifdef LOGGING

// Define constants
#define FCI_ERROR_PREFIX	"ERROR: "
#define FCI_WARNING_PREFIX	"WARNING: "
#define FCI_TAB				"\t"
#define FCI_LINEFEED		"\n"


///* Print a logging message */
void
fcicomp_log(msg_severity_t severity, const char *fmt, ...)
{
	/* If the severity is greater than the LOGGING_LEVEL,
	 * do not print the message. */
	if (severity <= LOGGING_LEVEL) {

		/* Define the stream where the message is printed */
		struct _IO_FILE * stream = stdout;

		/* Select the stderr stream for errors,
		 * and stdout for other messages */
		if (severity == ERROR_SEVERITY) {
			stream = stderr;
		}

		/* Select the stderr stream for errors and warnings */
		unsigned int t = 0;
		switch(severity) {
			case ERROR_SEVERITY:
			/* Insert ERROR before the message */
			fprintf(stream, FCI_ERROR_PREFIX);
			break;
			case WARNING_SEVERITY:
			/* Insert WARNING before the message */
			fprintf(stream, FCI_WARNING_PREFIX);
			break;
			default:
			/* Insert many tabs before the message */
			for (t = WARNING_SEVERITY; t < severity; t++)
			fprintf(stream, FCI_TAB);
			break;
		}

		va_list argp;
		/* Print out the variable list of args with vprintf. */
		va_start(argp, fmt);
		vfprintf(stream, fmt, argp);
		va_end(argp);

		/* Put on a final linefeed. */
		fprintf(stream, FCI_LINEFEED);
		fflush(stream);
	}

}

#endif /* LOGGING */

/* Avoid the warning message "ISO C forbids an empty translation unit" by
 * defining something */
typedef int make_iso_compilers_happy;
