from distutils.core import setup
from distutils.sysconfig import get_python_lib
from distutils.util import get_platform
import os, glob

pushstack = list()

def pushdir(dirname):
  global pushstack
  pushstack.append(os.getcwd())
  os.chdir(dirname)

def popdir():
  global pushstack
  os.chdir(pushstack.pop())

pushdir(os.path.dirname(os.path.abspath(__file__)))

dll = None
dll_names = ["../lib/wfmifc-mt-s.dll", "../lib/libwfmifc-mt.so"]
for name in dll_names:
    if os.access(name, os.R_OK):
        dll = name

setup(
    name='fwifc',
    version='1.26',
    description='Python interface to Riegl Full Wave data.',
    author='Roland Schwarz',
    author_email='rschwarz@riegl.co.at',
    maintainer='Riegl LMS GmbH',
    maintainer_email='support@riegl.co.at',
    url='http://www.riegl.com',
    py_modules=['fwifc'],
    data_files = [(get_python_lib(), [dll])],
    license = 'Riegl LMS GmbH'
    )

popdir()
