#!/usr/bin/env python
#-*- coding:utf-8 -*-


import sys
import os
import platform
import re


env = Environment()
abs_path = os.getcwd()
print abs_path

headers = ['']

libs = ['']

abs_headers = []
for header in headers:
    abs_headers.append("%s/%s" % (abs_path,header))

abs_libs = []
for lib in libs:
    abs_libs.append("%s/%s" % (abs_path,lib))


print "headers and libs"
print abs_headers
print abs_libs

env["headers"] = abs_headers
env["libs"]    = abs_libs
env["MUX_DIR"] = abs_path 

Export('env')


SConscript(['./demo/SConscript'])
