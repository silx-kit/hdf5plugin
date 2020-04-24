// $Id: fcicomp_jpegls.c 823 2017-09-05 14:30:55Z delaunay $
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
// VERSION:1.0.2:NCR:FCICOMP-12:04/09/2017:Remove the signal handler
// VERSION:1.0.1:NCR:FCICOMP-8:09/03/2016:Add the copyright notice in the header
//
// END-HISTORY
// =============================================================

#include <stdlib.h>
#include <string.h>

/* Includes from CharLS software */
#include "interface.h"

/* The "fcicomjpegls.h" include should appear after
 * the include of "interface.h" of CharLS software
 * otherwise this provoke a compilation error. Why ? */
#include "fcicomp_jpegls.h"
#include "fcicomp_jpegls_messages.h"

/* Include from fcicomp-common */
#include "fcicomp_log.h"

/** Maximum number of components in the images. */
#define MAX_COMPONENTS	4

///* Convert error codes return by charls to error code returned by the fcicomp-jpegls module */
int charlsToFjlsErrorCode(int charlsErr) {

	/* Initialize the output error code */
	int fcicompJlsErr = FJLS_NOERR;

	/* Convert error codes return by charls */
	switch (charlsErr) {
	case OK:
		/* No error */
		fcicompJlsErr = FJLS_NOERR;
		break;
	case InvalidJlsParameters:
		/* Parameter values are not a valid combination in JPEG-LS */
		fcicompJlsErr = FJLS_INVALID_JPEGLS_PARAMETERS;
		break;
	case ParameterValueNotSupported:
		/* Parameter values are not supported by CharLS */
		fcicompJlsErr = FJLS_UNSUPPORTED_JPEGLS_PARAMETERS;
		break;
	case UncompressedBufferTooSmall:
		/* Not enough memory allocated for the output of the JPEG-LS decode process */
		fcicompJlsErr = FJLS_UNCOMPRESSED_BUFER_TOO_SMALL;
		break;
	case CompressedBufferTooSmall:
		/* Not enough memory allocated for the output of the JPEG-LS encode process */
		fcicompJlsErr = FJLS_COMPRESSED_BUFER_TOO_SMALL;
		break;
	case InvalidCompressedData:
		/* The compressed bitstream is not decodable */
		fcicompJlsErr = FJLS_INVALID_COMPRESSED_DATA;
		break;
	case TooMuchCompressedData:
		/* Too much compressed data */
		fcicompJlsErr = FJLS_TOO_MUCH_COMPRESSED_DATA;
		break;
	case ImageTypeNotSupported:
		/* The image type used is not supported by CharLS */
		fcicompJlsErr = FJLS_IMAGE_TYPE_NOT_SUPPORTED;
		break;
	default:
		/* Default: Unknown CharLS error code */
		fcicompJlsErr = FJLS_UNKNOWN_ERROR;
		break;
	}

	/* Return the fcicomp-jpegls error code */
	return fcicompJlsErr;
}

///* Get the error messages corresponding to the input error code */
const char * getErrorMessage(int err) {

	/* Initialize the output message */
	char * msg = UNKNOWN_CHARLS_ERROR_CODE_MSG;

	/* Set the message string depending on the error */
	switch (err) {
	case InvalidJlsParameters:
		/* Parameter values are not a valid combination in JPEG-LS */
		msg = INVALID_JLS_PARAMETERS_MSG;
		break;
	case ParameterValueNotSupported:
		/* Parameter values are not supported by CharLS */
		msg = PARAMETER_VALUE_NOT_SUPPORTED_MSG;
		break;
	case UncompressedBufferTooSmall:
		/* Not enough memory allocated for the output of the JPEG-LS decode process */
		msg = UNCOMPRESSED_BUFFER_TOO_SMALL_MSG;
		break;
	case CompressedBufferTooSmall:
		/* Not enough memory allocated for the output of the JPEG-LS encode process */
		msg = COMPRESSED_BUFFER_TOO_SMALL_MSG;
		break;
	case InvalidCompressedData:
		/* The compressed bitstream is not decodable */
		msg = INVALID_COMPRESSED_DATA_MSG;
		break;
	case TooMuchCompressedData:
		/* Too much compressed data */
		msg = TOO_MUCH_COMPRESSED_DATA_MSG;
		break;
	case ImageTypeNotSupported:
		/* The image type used is not supported by CharLS */
		msg = IMAGE_TYPE_NOT_SUPPORTED_MSG;
		break;
	default:
		/* Default: Unknown CharLS error code */
		msg = UNKNOWN_CHARLS_ERROR_CODE_MSG;
		break;
	}

	return msg;
}

///* Compress an image in JPEG-LS. */
int jpeglsCompress(void *outBuf, size_t outBufSize, size_t *compressedSize, const void *inBuf, size_t inBufSize,
		int samples, int lines, jls_parameters_t jlsParams) {
	LOG(DEBUG_SEVERITY, ENTER_FUNCTION, __func__);

	/* Initialize the output value */
	int result = FJLS_NOERR;

	/* Initialize the charls parameters structure with zeros.
	 * JlsParameters is the structure defined in charls software. */
	struct JlsParameters charlsParams = { 0, 0, 0, 0, 0, 0, 0, 0, 0, { 0, 0, 0, 0, 0 }, /* JlsCustomParameters */
	{ 0, 0, 0, 0, 0, 0, NULL } /* JfifParameters */
	};

	/* Check some of the input JPEG-LS parameters */
	if (jlsParams.components > MAX_COMPONENTS) {
		result = FJLS_INVALID_JPEGLS_PARAMETERS;
		LOG(ERROR_SEVERITY, JPEGLS_COMPRESS_ERROR, INVALID_JLS_PARAMETERS_MSG);

	} else {

		/* Fill the charls parameters structure */

		/* Set the image dimension in the charls parameters structure */
		charlsParams.height = lines;
		charlsParams.width = samples;

		/* Copy the data from the jls_parameters_t structure
		 * to the JlsParameters structure (specific to charls) */

		/* Number of valid bits per sample to encode */
		charlsParams.bitspersample = jlsParams.bit_per_sample;
		/* Number of colour components */
		charlsParams.components = jlsParams.components;
		/* Interleave mode in the compressed stream */
		charlsParams.ilv = jlsParams.ilv;
		/* Difference bound for near-lossless coding */
		charlsParams.allowedlossyerror = jlsParams.near;

		/* Structure of JPEG-LS coding parameters */
		/* Maximum possible value for any image sample */
		charlsParams.custom.MAXVAL = jlsParams.preset.maxval;
		/* First quantization threshold value for the local gradients */
		charlsParams.custom.T1 = jlsParams.preset.t1;
		/* Second quantization threshold value for the local gradients  */
		charlsParams.custom.T2 = jlsParams.preset.t2;
		/* Third quantization threshold value for the local gradients  */
		charlsParams.custom.T3 = jlsParams.preset.t3;
		/* Value at which the counters A, B, and N are halved  */
		charlsParams.custom.RESET = jlsParams.preset.reset;

		/* Initialize the return value for the call to CharLS */
		int charlsResult = OK;

		/* Encode the data using CharLS software */
		LOG(DEBUG_SEVERITY, CALL_CHARLS_JPEGLS_ENCODE);
		LOG(DEBUG_SEVERITY, CHARLS_PARAMETERS, charlsParams.height, charlsParams.width,
				charlsParams.bitspersample, charlsParams.components, charlsParams.ilv, charlsParams.allowedlossyerror,
				charlsParams.custom.MAXVAL, charlsParams.custom.T1, charlsParams.custom.T2, charlsParams.custom.T3, charlsParams.custom.RESET);
		charlsResult = JpegLsEncode(outBuf, outBufSize, compressedSize, inBuf, inBufSize, &charlsParams);
		LOG(DEBUG_SEVERITY, EXIT_CHARLS_JPEGLS_ENCODE, charlsResult);

		/* Check the result of JpegLsEncode */
		if (charlsResult != OK) {
			LOG(ERROR_SEVERITY, JPEGLS_COMPRESS_ERROR, getErrorMessage(charlsResult));
			/* Set the compressed size to 0 in case an error has occurred */
			*compressedSize = 0;
			/* Convert charls error code to fcicomp-jpegls error code */
			result = charlsToFjlsErrorCode(charlsResult);
		}
	}

	/* Return the error code */
	LOG(DEBUG_SEVERITY, EXIT_FUNCTION, __func__, result);
	return result;
}

///* Get the JPEG-LS coding parameters for a compressed image. */
int jpeglsReadHeader(const void *inBuf, size_t inSize, int *samples, int *lines, jls_parameters_t * jlsParams) {
	LOG(DEBUG_SEVERITY, ENTER_FUNCTION, __func__);

	/* Initialize the output value */
	int result = FJLS_NOERR;

	/* Initialize the charls parameters structure with zeros.
	 * JlsParameters is the structure defined in charls software. */
	struct JlsParameters charlsParams = { 0, 0, 0, 0, 0, 0, 0, 0, 0, { 0, 0, 0, 0, 0 }, /* JlsCustomParameters */
	{ 0, 0, 0, 0, 0, 0, NULL } /* JfifParameters */
	};

	/* Initialize the return value for the call to CharLS */
	int charlsResult = OK;

	/* Read the JPEG-LS header using charls software */
	LOG(DEBUG_SEVERITY, CALL_CHARLS_JPEGLS_READHEADER);
	charlsResult = JpegLsReadHeader(inBuf, inSize, &charlsParams);
	LOG(DEBUG_SEVERITY, EXIT_CHARLS_JPEGLS_READHEADER, charlsResult);

	/* Check the result of JpegLsReadHeader */
	if (charlsResult == OK) {
		/* Get the image dimensions */
		*samples = charlsParams.width;
		*lines = charlsParams.height;

		/* The jlsParams pointer may be NULL.
		 * In this case, only the compressed image dimensions are returned. */
		if (jlsParams != NULL) {
			/* Copy the data from the JlsParameters structure (specific to charls)
			 * to the jls_parameters_t structure */

			/* Number of valid bits per sample to encode */
			jlsParams->bit_per_sample = charlsParams.bitspersample;
			/* Number of colour components */
			jlsParams->components = charlsParams.components;
			/* Interleave mode in the compressed stream */
			jlsParams->ilv = charlsParams.ilv;
			/* Difference bound for near-lossless coding */
			jlsParams->near = charlsParams.allowedlossyerror;

			/* Structure of JPEG-LS coding parameters */
			/* Maximum possible value for any image sample */
			jlsParams->preset.maxval = charlsParams.custom.MAXVAL;
			/* First quantization threshold value for the local gradients */
			jlsParams->preset.t1 = charlsParams.custom.T1;
			/* Second quantization threshold value for the local gradients  */
			jlsParams->preset.t2 = charlsParams.custom.T2;
			/* Third quantization threshold value for the local gradients  */
			jlsParams->preset.t3 = charlsParams.custom.T3;
			/* Value at which the counters A, B, and N are halved  */
			jlsParams->preset.reset = charlsParams.custom.RESET;
		}
	} else { /* charlsResult != OK */
		LOG(ERROR_SEVERITY, JPEGLS_READHEADER_ERROR, getErrorMessage(charlsResult));
		/* Convert charls error code to fcicomp-jpegls error code */
		result = charlsToFjlsErrorCode(charlsResult);
	}

	/* Return the error code */
	LOG(DEBUG_SEVERITY, EXIT_FUNCTION, __func__, result);
	return result;
}

///* Decompress a JPEG-LS image. */
int jpeglsDecompress(void *outBuf, size_t outSize, const void *inBuf, size_t inSize) {
	LOG(DEBUG_SEVERITY, ENTER_FUNCTION, __func__);

	/* Initialize the output value */
	int result = FJLS_NOERR;

	/* Initialize the return value for the call to CharLS */
	int charlsResult = OK;

	/* Uncompress the JPEG-LS image using charls software */
	LOG(DEBUG_SEVERITY, CALL_CHARLS_JPEGLS_DECODE);
	charlsResult = JpegLsDecode(outBuf, outSize, inBuf, inSize, NULL);
	LOG(DEBUG_SEVERITY, EXIT_CHARLS_JPEGLS_DECODE, charlsResult);

	/* Check the result of JpegLsDecode */
	if (charlsResult != OK) {
		LOG(ERROR_SEVERITY, JPEGLS_DECOMPRESS_ERROR, getErrorMessage(charlsResult));
		/* Convert charls error code to fcicomp-jpegls error code */
		result = charlsToFjlsErrorCode(charlsResult);
	}

	/* Return the error code */
	LOG(DEBUG_SEVERITY, EXIT_FUNCTION, __func__, result);
	return result;
}

