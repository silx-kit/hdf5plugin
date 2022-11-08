export SHELL = /bin/bash

ifeq ($(HDF5_HOME),)
    $(warning WARNING: HDF5_HOME not specified)
endif

ifeq ($(ZFP_HOME),)
    $(warning WARNING: ZFP_HOME not specified)
endif

# Construct version variable depending on what dir we're in
PWD_BASE = $(shell basename $$(pwd))
ifeq ($(PWD_BASE),src)
    H5Z_ZFP_BASE := .
else ifeq ($(PWD_BASE),test)
    H5Z_ZFP_BASE := ../src
else ifeq ($(PWD_BASE),H5Z-ZFP)
    H5Z_ZFP_BASE := ./src
endif

H5Z_ZFP_PLUGIN := $(H5Z_ZFP_BASE)/plugin
H5Z_ZFP_VERSINFO := $(shell grep '^\#define H5Z_FILTER_ZFP_VERSION_[MP]' $(H5Z_ZFP_BASE)/H5Zzfp_plugin.h | cut -d' ' -f3 | tr '\n' '.' | cut -d'.' -f-3 2>/dev/null)
ZFP_HAS_REVERSIBLE :=
ifneq ($(ZFP_HOME),)
    ZFP_HAS_REVERSIBLE := $(shell grep zfp_stream_set_reversible $(ZFP_HOME)/include/zfp.h 2>/dev/null)
endif

# Construct make-time knowledge of ZFP library version
ZFP_LIB_VERSION := $(shell grep '^\#define ZFP_VERSION_[MPT]' $(ZFP_HOME)/include/zfp/version.h 2>/dev/null | tr ' ' '\n' | grep '[0-9]' | tr -d '\n')
ifeq ($(ZFP_LIB_VERSION),)
    ZFP_LIB_VERSION := $(shell grep '^\#define ZFP_VERSION_[MRPT]' $(ZFP_HOME)/include/zfp.h 2>/dev/null | tr ' ' '\n' | grep '[0-9]' | tr -d '\n' 2>/dev/null)
endif
ifeq ($(ZFP_LIB_VERSION),)
    ZFP_LIB_VERSION := $(shell grep '^\#define ZFP_VERSION_[MRPT]' $(ZFP_HOME)/inc/zfp.h 2>/dev/null | tr ' ' '\n' | grep '[0-9]' | tr -d '\n' 2>/dev/null)
endif
ifeq ($(ZFP_LIB_VERSION),)
    $(warning WARNING: ZFP lib version not detected by make -- some tests may run)
endif

# Detect system type
PROCESSOR := $(shell uname -p | tr '[:upper:]' '[:lower:]')
OSNAME := $(shell uname -s | tr '[:upper:]' '[:lower:]')
OSTYPE := $(shell env | grep OSTYPE | cut -d'=' -f2- | tr '[:upper:]' '[:lower:]')
# LLNL specific enviornment variable
SYS_TYPE := $(shell env | grep SYS_TYPE | cut -d'=' -f2- | tr '[:upper:]' '[:lower:]')

# Common C compilers
HAS_GCC := $(shell basename $$(which gcc 2>/dev/null) 2>/dev/null)
HAS_CLANG := $(shell basename $$(which clang 2>/dev/null) 2>/dev/null)
HAS_ICC := $(shell basename $$(which icc 2>/dev/null) 2>/dev/null)
HAS_PGCC := $(shell basename $$(which pgcc 2>/dev/null) 2>/dev/null)
HAS_XLCR := $(shell basename $$(which xlc_r 2>/dev/null) 2>/dev/null)
HAS_BGXLCR := $(shell basename $$(which bgxlc_r 2>/dev/null) 2>/dev/null)

# Common Fortran compilers
HAS_GFORTRAN := $(shell basename $$(which gfortran 2>/dev/null) 2>/dev/null)
HAS_IFORT := $(shell basename $$(which ifort 2>/dev/null) 2>/dev/null)
HAS_XLFR := $(shell basename $$(which xlf_r 2>/dev/null) 2>/dev/null)
HAS_BGXLFR := $(shell basename $$(which bgxlf_r 2>/dev/null) 2>/dev/null)

# If compiler isn't set, lets try to pick it
ifeq ($(CC),)
    ifeq ($(OSNAME),darwin)
        ifneq ($(strip $(HAS_CLANG)),)
            CC = $(HAS_CLANG)
	else ifneq ($(strip $(HAS_GCC)),)
            CC = $(HAS_GCC)
        endif
    else ifneq ($(findstring ppc, $(PROCESSOR),),)
        ifneq ($(strip $(HAS_BGXLCR)),)
	    CC = $(HAS_BGXLCR)
        else ifneq ($(strip $(HAS_XLCR)),)
	    CC = $(HAS_XLCR)
        else ifneq ($(strip $(HAS_GCC)),)
	    CC = $(HAS_GCC)
        endif
    else
	ifneq ($(strip $(HAS_GCC)),)
            CC = $(HAS_GCC)
	else ifneq ($(strip $(HAS_ICC)),)
            CC = $(HAS_ICC)
	else ifneq ($(strip $(HAS_PGCC)),)
            CC = $(HAS_PGCC)
        endif
    endif
endif

# If we don't have a CC by now, error out
ifeq ($(CC),)
$(error $(CC))
endif

#
# Now, setup various flags based on compiler
#
ifneq ($(findstring gcc, $(CC)),)
    CFLAGS += -fPIC
    SOEXT ?= so
    SHFLAG ?= -shared
    PREPATH = -Wl,-rpath,
else ifneq ($(findstring clang, $(CC)),)
    SOEXT ?= dylib
    SHFLAG ?= -dynamiclib
    PREPATH = -L
else ifneq ($(findstring icc, $(CC)),)
    CFLAGS += -fpic
    SOEXT ?= so
    SHFLAG ?= -shared
    PREPATH = -Wl,-rpath,
else ifneq ($(findstring pgcc, $(CC)),)
    CFLAGS += -fpic
    SOEXT ?= so
    SHFLAG ?= -shared
    PREPATH = -Wl,-rpath,
else ifneq ($(findstring xlc_r, $(CC)),)
    CFLAGS += -qpic
    SOEXT ?= so
    SHFLAG ?= -qmkshrobj
    PREPATH = -Wl,-R,
else ifneq ($(findstring bgxlc_r, $(CC)),)
    CFLAGS += -qpic
    SOEXT ?= so
    SHFLAG ?= -qmkshrobj
    PREPATH = -Wl,-R,
endif

ifneq ($(findstring gfortran, $(FC)),)
    FCFLAGS += -fPIC
else ifneq ($(findstring ifort, $(FC)),)
    FCFLAGS += -fpic
else ifneq ($(findstring pgf90, $(FC)),)
    FCFLAGS += -fpic
else ifneq ($(findstring xlf_r, $(FC)),)
    FCFLAGS += -qpic
else ifneq ($(findstring bgxlf_r, $(FC)),)
    FCFLAGS += -qpic
else ifneq ($(findstring f77, $(FC)),)
# some makefile versions set FC=f77 if FC is not set
    FC =
endif

ifneq ($(wildcard $(ZFP_HOME)/include),)
ZFP_INC = $(ZFP_HOME)/include
else ifneq ($(wildcard $(ZFP_HOME)/inc),)
ZFP_INC = $(ZFP_HOME)/inc
endif
ifeq ($(wildcard $(ZFP_INC)/zfp.h),) # no header file
$(error "zfp.h not found")
endif

ifeq ($(wildcard $(ZFP_HOME)/lib),)
ZFP_LIB = $(ZFP_HOME)/lib64
else
ZFP_LIB = $(ZFP_HOME)/lib
endif

# Check if ZFP has CFP
ifeq ($(wildcard $(ZFP_LIB)/libcfp.*),) # no cfp lib file
  ZFP_HAS_CFP = 0
else
  ifeq ($(wildcard $(ZFP_INC)/zfp/array.h),) # no 1.0.0 header file
    ifeq ($(wildcard $(ZFP_INC)/cfparrays.h),) # no 0.5.5 header file
        ZFP_HAS_CFP = 0
    else
        ZFP_HAS_CFP = 1
    endif
  else
    ZFP_HAS_CFP = 1
  endif
endif

# Check if specified individually the HDF5 include directory,
# library directory and bin directory separated by commas, i.e. HDF5_HOME=INC,LIB,BIN
FOUND_LIST=$(shell echo "$(HDF5_HOME)" | grep -q "," && echo "true")
ifeq ("$(FOUND_LIST)","true")
  HDF5_INC = $(shell echo $(HDF5_HOME) | awk -F'[,]' '{print $$1}')
  HDF5_LIB = $(shell echo $(HDF5_HOME) | awk -F'[,]' '{print $$2}')
  HDF5_BIN = $(shell echo $(HDF5_HOME) | awk -F'[,]' '{print $$3}')
  MAKEVARS =
else
  HDF5_INC = $(HDF5_HOME)/include
  ifeq ($(wildcard $(HDF5_HOME)/lib),)
    HDF5_LIB = $(HDF5_HOME)/lib64
  else
    HDF5_LIB = $(HDF5_HOME)/lib
  endif
  HDF5_BIN = $(HDF5_HOME)/bin
  MAKEVARS = HDF5_HOME=$(HDF5_HOME)
endif

ifeq ($(PREFIX),)
    PREFIX := $(shell pwd)/install
endif
INSTALL ?= install

MAKEVARS += ZFP_HOME=$(ZFP_HOME)  PREFIX=$(PREFIX)

.SUFFIXES:
.SUFFIXES: .c .F90 .h .o .mod

%.o : %.c
	$(CC) $< -o $@ -c $(CFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)

%.o %.mod : %.F90
	$(FC) $< -o $@ -c $(FCFLAGS) -I$(H5Z_ZFP_BASE) -I$(ZFP_INC) -I$(HDF5_INC)
