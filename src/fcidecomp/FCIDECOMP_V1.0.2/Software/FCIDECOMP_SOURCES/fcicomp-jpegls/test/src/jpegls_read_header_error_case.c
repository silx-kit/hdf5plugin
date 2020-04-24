// $Id: jpegls_read_header_error_case.c 778 2016-03-09 07:56:29Z delaunay $
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

#include <stdlib.h>
#include <stdio.h>

#include "fcicomp_jpegls.h"
#include "fcicomp_errors.h"
#include "fcicomp_options.h"

#define ERROR_MSG	"The expected error has not occurred!\n"

// Error case for reader JPEGLS header in an image
int jpeglsReadHeaderErrorCase(void) {

	// Read JPEG-LS header to get the image parameters
	int samples = 0;
	int lines = 0;
	int result = EXIT_SUCCESS;
	// Provoke an error: do not provide any input data
	if (jpeglsReadHeader(NULL, 0, &samples, &lines, NULL) != FJLS_INVALID_COMPRESSED_DATA) {
		// As we provoke an error, jpeglsReadHeader should return a positive error code
		fputs(ERROR_MSG, stderr);
		result = EXIT_FAILURE;
	}

	// The expected error has happened => exit with the success status
	return result;
}
