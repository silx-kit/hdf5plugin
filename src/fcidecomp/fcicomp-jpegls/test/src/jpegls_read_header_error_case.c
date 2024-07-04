// =============================================================
//
// Copyright 2015-2023, European Organisation for the Exploitation of Meteorological Satellites (EUMETSAT)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// =============================================================

// AUTHORS:
// - THALES Services

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
