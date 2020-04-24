// $Id: fcicomp_jpegls_messages.h 823 2017-09-05 14:30:55Z delaunay $
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

/*! \file

 This file define the error messages printed by fcicomp_jpegls.

 */

#ifndef FCICOMP_JPEGLS_MESSAGES_H_
#define FCICOMP_JPEGLS_MESSAGES_H_

/* Error messages */
#define JPEGLS_COMPRESS_ERROR					"Error in jpeglsCompress: %s"
#define JPEGLS_READHEADER_ERROR					"Error in jpeglsReadHeader: %s"
#define JPEGLS_DECOMPRESS_ERROR					"Error in jpeglsDecompress: %s"
#define INVALID_JLS_PARAMETERS_MSG				"Parameter values are not a valid combination in JPEG-LS."
#define PARAMETER_VALUE_NOT_SUPPORTED_MSG		"Parameter values are not supported by CharLS."
#define UNCOMPRESSED_BUFFER_TOO_SMALL_MSG		"Not enough memory allocated for the output of the JPEG-LS decoding process."
#define COMPRESSED_BUFFER_TOO_SMALL_MSG			"Not enough memory allocated for the output of the JPEG-LS encoding process."
#define INVALID_COMPRESSED_DATA_MSG				"The compressed bit-stream cannot be decoded."
#define TOO_MUCH_COMPRESSED_DATA_MSG			"Too much compressed data."
#define IMAGE_TYPE_NOT_SUPPORTED_MSG			"The image type is not supported by CharLS."
#define UNKNOWN_CHARLS_ERROR_CODE_MSG			"Unknown CharLS error code."

/* Debug messages */
#define ENTER_FUNCTION							"-> Enter in %s()"
#define EXIT_FUNCTION							"<- Exit from %s() with code: %d"
#define CALL_CHARLS_JPEGLS_ENCODE				"-> Calling CharLS JpegLsEncode()"
#define EXIT_CHARLS_JPEGLS_ENCODE				"<- Exit from CharLS JpegLsEncode() with code: %d"
#define CALL_CHARLS_JPEGLS_READHEADER			"-> Calling CharLS JpegLsReadHeader()"
#define EXIT_CHARLS_JPEGLS_READHEADER			"<- Exit from CharLS JpegLsReadHeader() with code: %d"
#define CALL_CHARLS_JPEGLS_DECODE				"-> Calling CharLS JpegLsDecode()"
#define EXIT_CHARLS_JPEGLS_DECODE				"<- Exit from CharLS JpegLsDecode() with code: %d"

#define CHARLS_PARAMETERS						"CharLS parameters:\nheight:%d\nwidth:%d\nbitspersample:%d\ncomponents:%d\nilv:%d\nallowedlossyerror:%d\nMAXVAL:%d\nT1:%d\nT2:%d\nT3:%d\nRESET:%d"

#endif /* FCICOMP_JPEGLS_MESSAGES_H_ */
