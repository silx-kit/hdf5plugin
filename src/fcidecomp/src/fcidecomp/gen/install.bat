:: =============================================================
::
:: Copyright 2015-2023, European Organisation for the Exploitation of Meteorological Satellites (EUMETSAT)
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

@echo on
:: Get the module to build
SET MODULE=%1

:: If the FCICOMP_ROOT environment variable is not set, set the default one: the upper directory of this script
IF NOT DEFINED FCICOMP_COTS_ROOT (
    pushd %BASH_SOURCE%\..
    SET "FCICOMP_ROOT=%cd%"
    popd
)

:: Define the build directory
SET "BUILD_DIR=%FCICOMP_ROOT%\build\%MODULE%"

:: Move to the build directory
cd %BUILD_DIR%

echo "Installing %MODULE% ..."
:: Perform the install
CALL cmake --install . || goto :error

SET "INSTALL_MANIFEST=%BUILD_DIR%\install_manifest.txt"
SET "CMAKECACHE_FILE=%BUILD_DIR%\CMakeCache.txt"
echo manifest: "%INSTALL_MANIFEST%" , cmakecache: "%CMAKECACHE_FILE%"

IF NOT EXIST "%INSTALL_MANIFEST%" (
    echo Warning: The file install_manifest.txt has not been found in the building directory %BUILD_DIR%.
    goto :EOF
)
:: here exists the install manifest
IF NOT EXIST "%CMAKECACHE_FILE%" (
    echo Error: Cannot find file: %CMAKECACHE_FILE%.
	goto :error
)
:: here exists the cmake cache file
FOR /f "tokens=2 delims==" %%a IN ('find "CMAKE_INSTALL_PREFIX:PATH=" "%CMAKECACHE_FILE%"') DO @set cmakeinstallprefixraw=%%a

IF NOT DEFINED cmakeinstallprefixraw (
    echo Warning: Cannot copy the install_manifest.txt file to the install directory: Install directory is not known.
    goto :EOF
)
:: here cmakeinstallprefixraw is defined; it is a unix-style path
SET "cmakeinstallprefix=%cmakeinstallprefixraw:/=\%"
SET "DEST_DIR=%cmakeinstallprefix%\share\cmake"
SET "DEST_DIRraw=%cmakeinstallprefixraw%/share/cmake"
SET "DEST=%DEST_DIR%\%MODULE%_install_manifest.txt"
SET "DESTraw=%DEST_DIRraw%/%MODULE%_install_manifest.txt"
echo destdir: "%DEST_DIR%", dest: "%DEST%"
IF NOT EXIST %DEST_DIR% (
   echo Creating directory %DEST_DIR%
   mkdir %DEST_DIR%
)
echo.>>"%INSTALL_MANIFEST%"
echo %DESTraw%>>"%INSTALL_MANIFEST%"
echo -- Copying: %INSTALL_MANIFEST% to %DEST%
copy /y %INSTALL_MANIFEST% %DEST%

goto :EOF

:error
echo Failed with error #%errorlevel%.
exit 1
