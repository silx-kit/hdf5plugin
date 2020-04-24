// $Id: fcicomp_options.h 778 2016-03-09 07:56:29Z delaunay $
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

 This file define the options and separators for fcicomp.

 */


#ifndef FCICOMP_OPTIONS
#define FCICOMP_OPTIONS

//SEPARATOR
#define FCI_SEPARATOR				 "/"
#define FCI_UNDERSCORE				 "_"
#define FCI_SHARP					 '#'
#define FCI_COMMA					 ","
#define FCI_POINT					 "."
#define FCI_PLUS						 "+"
#define FCI_EQUAL						 "="

#define FCI_DOT_CHAR				 '.'
#define FCI_SLASH_CHAR				 '/'

//COMMAND OPTIONS
#define FCI_HELP						 "-h"
#define FCI_HELP_FULL				 "--help"
#define FCI_HELP_U					 "-u"
#define FCI_INPUT					 "-i"
#define FCI_OUTPUT					 "-o"
#define FCI_PREFIX					 "-p"
#define FCI_NETCDF_CONFIG_FILE		 "-r"
#define FCI_JPEGLS_CONFIG_FILE		 "-j"
#define FCI_WIDTH					 "-w"
#define FCI_HEIGHT					 "-h"


//EXTENSIONS
#define FCI_RAW_EXT					 ".raw"
#define FCI_HDR_EXT					 ".hdr"
#define FCI_NC_EXT					 ".nc"
//FILE OPTIONS
#define FCI_WRITE					 "wb"
#define FCI_READ					 "rb"
#define FCI_READ_ONLY				 "r"

//OTHERS
#define FCI_QF						 "_qf"
#define FCI_BSQ						 "bsq"
#define FCI_SKIP_LINE				 " \t\r\n"

//NETCDF
#define FCI_0						 '\0'

//NUMBER
#define FCI_ONE						1
#define FCI_TWO						2
#define FCI_THREE					3
#define FCI_FOUR					4
#define FCI_FIVE					5
#define FCI_SIX						6
#define FCI_SEVEN					7
#define FCI_EIGHT					8
#define FCI_NINE					9
#define FCI_12						12
#define FCI_13						13
#define FCI_14						14
#define FCI_15						15
#define FCI_16						16
#define FCI_19						19
#define FCI_22						22
#define FCI_32						32
#define FCI_64						64

#define FCI_BYTE					8

#endif
