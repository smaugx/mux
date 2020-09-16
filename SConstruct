#!/usr/bin/env python
#-*- coding:utf-8 -*-


import sys
import os
import platform
import re

env = Environment()
abs_path = os.getcwd()
print('workspace path:{0}'.format(abs_path))

sbuild_dir = 'sbuild'

headers = ['.', 'third-party/include']
libs = ['./third-party/lib']

abs_headers = []
abs_libs = []

for item in headers:
    abs_item = os.path.join(abs_path, item)
    abs_headers.append(abs_item)


for item in libs:
    abs_item = os.path.join(abs_path, item)
    abs_libs.append(abs_item)

build_dir = os.path.join(abs_path, sbuild_dir)
abs_libs.append(os.path.join(build_dir, 'lib'))

CCFLAGS = '-ggdb -std=c++11'

print('\nheaders path:')
print(abs_headers)
print('\n')

print('libs path:')
print(abs_libs)
print('\n')

print("begin load SConscript")

env["headers"] = abs_headers
env["libs"]    = abs_libs
env["MUX_DIR"] = abs_path 
env['ccflags'] = CCFLAGS
env['build_dir'] = build_dir

Export('env')

SConscript(['./mbase/SConscript'])
SConscript(['./message_handle/SConscript'])
SConscript(['./socket/SConscript'])
SConscript(['./transport/SConscript'])
SConscript(['./demo/bench/SConscript'])
SConscript(['./demo/echo/SConscript'])

print("\n All Done, Please Check {0}".format(env['build_dir']))
