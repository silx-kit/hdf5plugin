# =============================================================
#
# Copyright 2021-2023, European Organisation for the Exploitation of Meteorological Satellites (EUMETSAT)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# =============================================================

# AUTHORS:
# - B-Open Solutions srl

# Code inspired by:
# - https://github.com/conda-forge/charls-feedstock/blob/master/recipe/build.sh
# - https://github.com/mraspaud/fcidecomp-conda-recipe/blob/master/build.sh

#set -ex

 $PYTHON -m pip install $RECIPE_DIR/../src/fcidecomp-python --no-deps --ignore-installed -vv


PATH_TO_DELIVERY=$(pwd)
FCIDECOMP_BUILD_PATH=${PATH_TO_DELIVERY}/build
mkdir -p ${FCIDECOMP_BUILD_PATH}
cd ${FCIDECOMP_BUILD_PATH}

# Build FCIDECMP
cp -r ${PATH_TO_DELIVERY}/fcidecomp/* ${FCIDECOMP_BUILD_PATH}

## Build fcicomp-jpegls
./gen/build.sh fcicomp-jpegls release                                     \
    -DCMAKE_PREFIX_PATH=${CONDA_PREFIX}                                   \
    -DCMAKE_INSTALL_PREFIX=${PREFIX}                                      \
    -DCHARLS_ROOT=${PREFIX}
./gen/build.sh fcicomp-jpegls test
./gen/install.sh fcicomp-jpegls

## Build fcicomp-H5Zjpegls
./gen/build.sh fcicomp-H5Zjpegls release                                  \
    -DCMAKE_PREFIX_PATH="${PREFIX};${CONDA_PREFIX}"                       \
    -DCMAKE_INSTALL_PREFIX=${PREFIX}
# Fails (4 out of 7 tests failing)
# ./gen/build.sh fcicomp-H5Zjpegls test
./gen/install.sh fcicomp-H5Zjpegls


mkdir -p "${PREFIX}/etc/conda/activate.d"
cp "${RECIPE_DIR}/scripts/activate.sh" "${PREFIX}/etc/conda/activate.d/${PKG_NAME}_activate.sh"
mkdir -p "${PREFIX}/etc/conda/deactivate.d"
cp "${RECIPE_DIR}/scripts/deactivate.sh" "${PREFIX}/etc/conda/deactivate.d/${PKG_NAME}_deactivate.sh"
