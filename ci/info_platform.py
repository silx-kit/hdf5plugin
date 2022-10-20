#!/usr/bin/env python
# coding: utf-8
"""Print information about python."""

__authors__ = ["Jérôme Kieffer"]
__date__ = "08/12/2015"
__license__ = "MIT"


import sys

print("Python %s bits" % (tuple.__itemsize__ * 8))
print("       maxsize: %s\t maxunicode: %s" % (sys.maxsize, sys.maxunicode))
print(sys.version)
print(" ")

from sysconfig import get_config_vars
print("Config: " + str(get_config_vars("CONFIG_ARGS")))
print("")
