# /*##########################################################################
#
# Copyright (c) 2015-2024 European Synchrotron Radiation Facility
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# ###########################################################################*/


from typing import NamedTuple
import re

version = "5.0.0"


class _VersionInfo(NamedTuple):
    """Version information as a namedtuple"""

    major: int
    minor: int
    micro: int
    releaselevel: str = "final"
    serial: int = 0

    @classmethod
    def from_string(cls, version: str) -> "_VersionInfo":
        pattern = r"(?P<major>\d+)\.(?P<minor>\d+)\.(?P<micro>\d+)((?P<prerelease>a|b|rc)(?P<serial>\d+))?"
        match = re.fullmatch(pattern, version, re.ASCII)
        fields = {k: v for k, v in match.groupdict().items() if v is not None}
        # Remove prerelease and convert it to releaselevel
        prerelease = fields.pop("prerelease", None)
        releaselevel = {"a": "alpha", "b": "beta", "rc": "candidate", None: "final"}[
            prerelease
        ]
        version_fields = {k: int(v) for k, v in fields.items()}

        return cls(releaselevel=releaselevel, **version_fields)


version_info = _VersionInfo.from_string(version)
