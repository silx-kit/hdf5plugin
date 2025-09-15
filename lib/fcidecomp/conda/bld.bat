:: =============================================================
::
:: Copyright 2021-2023, European Organisation for the Exploitation of Meteorological Satellites (EUMETSAT)
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::     http://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.
::
:: =============================================================

:: AUTHORS:
:: - B-Open Solutions srl

@echo ON
setlocal enabledelayedexpansion

set PATH_TO_DELIVERY=%cd%
set FCIDECOMP_BUILD_PATH=%PATH_TO_DELIVERY%\build
if not exist "%FCIDECOMP_BUILD_PATH%" mkdir "%FCIDECOMP_BUILD_PATH%"
cd %FCIDECOMP_BUILD_PATH%

if "%ARCH%"=="32" (
    rem Install CharLS
    cmake -LAH -G "Ninja"                                                     ^
        -DCMAKE_BUILD_TYPE="Release"                                          ^
        -DCMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX%                               ^
        -DCMAKE_PREFIX_PATH=%LIBRARY_PREFIX%                                  ^
        -DOPENCV_BIN_INSTALL_PATH=bin                                         ^
        -DOPENCV_LIB_INSTALL_PATH=lib                                         ^
        -DBUILD_SHARED_LIBS=1                                                 ^
        -DCHARLS_BUILD_TESTS=1                                                ^
        -DCHARLS_BUILD_SAMPLES=0                                              ^
        -DCHARLS_INSTALL=1                                                    ^
        ..

    cmake --build . --target install --config Release
    if errorlevel 1 exit 1
)

rem Build FCIDECMP
xcopy /E %PATH_TO_DELIVERY%\fcidecomp\* %FCIDECOMP_BUILD_PATH%

rem Build fcicomp-jpegls
call gen\build.bat fcicomp-jpegls release                                 ^
    -DCMAKE_BUILD_TYPE="Release"                                          ^
    -DCMAKE_PREFIX_PATH=%CONDA_PREFIX%;%LIBRARY_PREFIX%                                    ^
    -DCMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX%                               ^
    -DCHARLS_ROOT=%CONDA_PREFIX%                                          ^
    -DCMAKE_INCLUDE_PATH=%SRC_DIR%\src                                    ^
    -DBUILD_SHARED_LIBS=OFF                                               ^
    -DCHARLS_BUILT_DLL=1
if errorlevel 1 exit 1

cd %FCIDECOMP_BUILD_PATH%
call gen\build.bat fcicomp-jpegls test
if errorlevel 1 exit 1

cd %FCIDECOMP_BUILD_PATH%
call gen\install.bat fcicomp-jpegls
if errorlevel 1 exit 1

rem Build fcicomp-H5Zjpegls
cd %FCIDECOMP_BUILD_PATH%
call gen\build.bat fcicomp-H5Zjpegls release                              ^
    -DCMAKE_PREFIX_PATH=%CONDA_PREFIX%;%LIBRARY_PREFIX%                   ^
    -DCMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX%                               ^
    -DHDF5_USE_STATIC_LIBRARIES=1
if errorlevel 1 exit 1

:: Fails
:: cd %FCIDECOMP_BUILD_PATH%
:: call gen\build.bat fcicomp-H5Zjpegls test
::if errorlevel 1 exit 1

cd %FCIDECOMP_BUILD_PATH%
call gen\install.bat fcicomp-H5Zjpegls
if errorlevel 1 exit 1

cd %FCIDECOMP_BUILD_PATH%
call %PREFIX%\Scripts\pip install --no-deps --ignore-installed -vv %RECIPE_DIR%/../src/fcidecomp-python

if not exist %PREFIX%\etc\conda\activate.d mkdir %PREFIX%\etc\conda\activate.d
copy %RECIPE_DIR%\scripts\activate.bat %PREFIX%\etc\conda\activate.d\%PKG_NAME%_activate.bat
copy %RECIPE_DIR%\scripts\activate.ps1 %PREFIX%\etc\conda\activate.d\%PKG_NAME%_activate.ps1
copy %RECIPE_DIR%\scripts\activate.sh %PREFIX%\etc\conda\activate.d\%PKG_NAME%_activate.sh
if not exist %PREFIX%\etc\conda\deactivate.d mkdir %PREFIX%\etc\conda\deactivate.d
copy %RECIPE_DIR%\scripts\deactivate.bat %PREFIX%\etc\conda\deactivate.d\%PKG_NAME%_deactivate.bat
copy %RECIPE_DIR%\scripts\deactivate.ps1 %PREFIX%\etc\conda\deactivate.d\%PKG_NAME%_deactivate.ps1
copy %RECIPE_DIR%\scripts\deactivate.sh %PREFIX%\etc\conda\deactivate.d\%PKG_NAME%_deactivate.sh
