#!/usr/bin/env python
#-*- coding:utf-8 -*-


import sys
import os
import platform
import re

env = Environment()
abs_path = os.getcwd()
print('workspace path:{0}'.format(abs_path))

headers = ['.', 'transport', 'epoll', 'mbase', 'third-party/include', 'message_handle']
libs = ['./third-party/lib']

abs_headers = []
abs_libs = []

for item in headers:
    abs_item = os.path.join(abs_path, item)
    abs_headers.append(abs_item)


for item in libs:
    abs_item = os.path.join(abs_path, item)
    abs_libs.append(abs_item)

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
env['sbuild'] = os.path.join(abs_path, 'sbuild')

Export('env')


SConscript(['./demo/echo/SConscript'])
SConscript(['./demo/bench/SConscript'])

print("\n All Done, Please Check {0}".format(env['sbuild']))
