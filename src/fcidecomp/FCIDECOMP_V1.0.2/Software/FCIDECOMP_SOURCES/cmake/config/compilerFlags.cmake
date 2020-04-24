# $Id$
# =============================================================
# 
# PROJECT  : FCI_COMPRESSION 
# 
# AUTHOR   : THALES Services
# 
# Copyright 2015 EUMETSAT
# 
# =============================================================
# HISTORY :
# 
# VERSION:1.0.1:NCR:FCICOMP-8:09/03/2016:Add the copyright notice in the header
#
# END-HISTORY
# =============================================================


STRING( TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE )
IF("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
  SET(RELEASE ON)
ENDIF()

IF("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
  SET(DEBUG ON)
ENDIF()

#-----------------------------------------------------------------------------
# Compiler specific flags for GNU C
#-----------------------------------------------------------------------------

IF (CMAKE_COMPILER_IS_GNUCC)
  # Requirement [IDPF-I] ADD-08140 a) C ISO/IEC 9899
  SET (CMAKE_C_FLAGS "-std=iso9899:1999 -pedantic ${CMAKE_ANSI_CFLAGS} ${CMAKE_C_FLAGS}")
  
  # Shared libs settings
  IF (BUILD_SHARED_LIBS)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  ENDIF (BUILD_SHARED_LIBS) 
  
  # Note: CMake automatically adds the CMAKE_C_FLAGS_RELEASE=""-DNDEBUG
  # -O3" to the compiler option if CMAKE_BUILD_TYPE is RELEASE.
  
  IF (RELEASE)
    # Set the optimization flag
    SET (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
  ENDIF (RELEASE)
 
  # Note: CMake automatically adds the CMAKE_C_FLAGS_DEBUG to the
  # compiler option if CMAKE_BUILD_TYPE is DEBUG.

  IF (DEBUG)
    # Add compilation flags for the debug mode 
    SET (CMAKE_C_FLAGS_DEBUG "-DDEBUG -W -Wall ${CMAKE_C_FLAGS_DEBUG}")
  ENDIF (DEBUG)

ENDIF (CMAKE_COMPILER_IS_GNUCC)


#-----------------------------------------------------------------------------
# Compiler specific flags for GNU C++
#-----------------------------------------------------------------------------

IF (CMAKE_COMPILER_IS_GNUCXX)
  # Requirement [IDPF-I] ADD-08140 b) C++ ISO/IEC 14882
  SET (CMAKE_CXX_FLAGS "-std=c++03 -pedantic ${CMAKE_ANSI_CFLAGS} ${CMAKE_CXX_FLAGS}")
  
  # Shared libs settings
  IF (BUILD_SHARED_LIBS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  ENDIF (BUILD_SHARED_LIBS) 
  
  IF (RELEASE)
    # Set the optimization flag
    SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  ENDIF (RELEASE)

  IF (DEBUG)
    # Add compilation flags for the debug mode 
    SET (CMAKE_CXX_FLAGS_DEBUG "-DDEBUG  -W -Wall ${CMAKE_C_FLAGS_DEBUG}")
  ENDIF (DEBUG)

ENDIF (CMAKE_COMPILER_IS_GNUCXX)


