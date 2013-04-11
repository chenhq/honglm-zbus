from distutils.core import setup 

'''
import sys
ext = 'so'
if sys.platform.startswith('win'):
    ext = 'pyd'
dylib = 'dylib/python%s%s/_pyzbus.%s'%(sys.version_info.major, sys.version_info.minor, ext)

'''
setup(name = 'pyzbus',
      version = '3.5.0',
      packages=['pyzbus'],
      package_data={'pyzbus':['*.*']})

