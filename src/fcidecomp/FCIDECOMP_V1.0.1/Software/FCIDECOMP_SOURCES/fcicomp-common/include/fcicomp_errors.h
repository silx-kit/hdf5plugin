// $Id: fcicomp_errors.h 778 2016-03-09 07:56:29Z delaunay $
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

 This file define the errors message of fcicomp.

 */

#ifndef FCICOMP_ERRORS
#define FCICOMP_ERRORS

#define ERR_TEST(e, msg) {fputs((msg), stderr); return (e);}

#define INVALID_NUMBER_ARGUMENTS 			  	"Invalid number of arguments !\n"
#define TOO_MANY_ARGUMENTS 				  	"Too many input arguments.\n"
#define MISSING_INPUT_ARGUMENTS 			  	"Missing input arguments.\n"
#define UNKNOWN_ARGUMENT 					  	"Unknown argument %s.\n"
#define UNEXPECTED_FILTER					 	"Unexpected filter!\n"
#define MEMORY_ALLOCATION_ERROR			 	"Memory allocation error!\n"
#define CANNOT_OPEN_FILE_W					 	"Cannot open file for writing!\n"
#define ERROR_WRITING_FILE					 	"Error writing file!\n"
#define ERROR_DURING_COMPRESSION			 	"Error during the compression!\n"
#define ERROR_DURING_DECOMPRESSION			 	"Error during the decompression!\n"
#define CANNOT_OPEN_FILE_R					 	"Cannot open file for reading!\n"
#define ERROR_READING_FILE					 	"Error reading file!\n"
#define JPEG_LS_FILTER_UNVAILABLE			 	"JPEG-LS encoding filter not available!\n"
#define ERROR_READING_JPEGLS_HEADER		 	"Error reading the JPEG-LS header!\n"
#define TRANSPARENT_FILTER_UNVAILABLE		 	"Transparent filter not available!\n"
#define TEST_NOT_DEFINED					 	"Test %s is not defined.\n"
#define CANNOT_READ_DATA					 	"Cannot read the image data file %s\n"


#endif
