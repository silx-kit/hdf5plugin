// $Id: fcicomp_log.h 778 2016-03-09 07:56:29Z delaunay $
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

/*! \file

 This file define the logging function of fcicomp.

 */

#ifndef FCICOMP_LOG_H_
#define FCICOMP_LOG_H_

/**@{*/
/** Define logging message severity by decreasing order of severity.*/
typedef enum {
	ERROR_SEVERITY = 0,
	WARNING_SEVERITY,
	NORMAL_SEVERITY,
	DEBUG_SEVERITY
} msg_severity_t;
/**@}*/


#ifdef LOGGING

/** Define the default logging level */
#define DEFAULT_LOGGING_LEVEL	ERROR_SEVERITY

/* Set the logging level to the default value if it has not been defined at
 * building time */
#ifndef LOGGING_LEVEL
/** Define the logging level */
#define LOGGING_LEVEL			DEFAULT_LOGGING_LEVEL
#endif /* LOGGING_LEVEL */

///**
// * @brief Print a logging message.
// *
// * Normal message are printed on the stdout. Warning and error message are
// * printed on the stderr.
// *
// * This function is heavily based on the function nc_log in netCDF-4 file error4.c.
// */
void fcicomp_log(msg_severity_t severity, const char *fmt, ...);

#define LOG(s, ...) 			(fcicomp_log ((s), __VA_ARGS__))

#else /* LOGGING */

/* This definition will be used unless LOGGING is defined. */
#define LOG(s, ...)

#endif /* LOGGING */

/* Handle errors by printing an error message and exiting with a non-zero status. */
#define ERR(e, msg) {LOG(ERROR_SEVERITY, (msg)); return (e);}

#endif /* FCICOMP_LOG_H_ */
