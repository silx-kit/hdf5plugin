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

import os
from setuptools import setup, find_packages

with open("README.md") as readme_file:
    readme = readme_file.read()

install_requirements = ["h5py"]
setup_requirements = ["setuptools"]

try:
    with open("VERSION") as version_file:
        version = version_file.read().strip()
except FileNotFoundError:
    version = os.environ.get(
        "CI_COMMIT_TAG", os.popen("git describe --tags --abbrev=0").read()
        )
    if version == "":
        version = "0.0.0+"+os.environ.get("CI_COMMIT_SHORT_SHA")
    with open("VERSION", "w") as version_file:
        version_file.write(version)

setup(
    name='fcidecomp-python',
    version=version,
    description="FCIDECOMP Python plugin",
    long_description=readme,
    author="B-Open Solutions srl",
    author_email="software@bopen.eu",
    url="https://www.eumetsat.int",
    packages=find_packages(),
    include_package_data=True,
    install_requires=install_requirements,
    setup_requires=setup_requirements,
    license="Apache License 2.0'",
    zip_safe=True,
    keywords="",
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: Apache Software License",
        "Natural Language :: English",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
    ],
)
