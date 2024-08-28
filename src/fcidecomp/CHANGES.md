# Changelog
All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.1.1]

### Added
- Added support for HDF5 (1.12 and 1.14)
- Added support for Python 3.10, 3.11, 3.12 (Linux only)

### Changed
- Improved IPR and license information
- Updated dependencies.

### Removed
- Remove support for Python 3.7, 3.8


## [2.0.1] - 2023-03-17

### Fixed
- Fix licenses and incomplete copyrights around.

## [2.0.0] - 2023-01-17

### Added
- Inventory items
- Restructured FCIDECOMP software source code, for compilation in Linux and Windows (32/64-bit) environments
- Python package, to allow for the use of the FCIDECOMP decompression filter within Python
- Conda package build system including both the bare FCIDECOMP software and the associated Python package, supporting 
  Linux and Windows (32/64-bit) environments and Python versions 3.7 to 3.9 (extremes included)
- Automatic tests based on `pytest`
- Performance tests based on `pytest` and `pytest-benchmark`
- Instructions to build and install the software from the source code (Ubuntu 20.04 and RockyLinux 8) 

### Changed
- Adopts Apache License Version 2.0 for FCIDECOMP source code
- Update CharLS library from 1.0 to 2.1.0

### Fixed
- Move inline macro functions to function to avoid critical code smells
