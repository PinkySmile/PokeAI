from setuptools import setup, Extension
import os
from Cython.Build import cythonize

cwd = os.getcwd()
include_dir = os.path.join(cwd, '..', '..', 'include')
source_dir = os.path.join(cwd, '..', '..', 'src')

os.environ['LDFLAGS'] = '-L' + cwd + '/../../cmake-build-debug'

extensions = [
	Extension("GameEngine", ['GameEngine/GameEngine.pyx'], include_dirs=[include_dir, source_dir, '.'], libraries=['PokemonGen1Core', 'PokemonGen1Emulator']),
]

setup(
	name='PyGen1',
	version='1.0',
	packages=['PyGen1'],
	url='',
	ext_modules=cythonize(extensions, build_dir="build"),
	license='MIT',
	author='PinkySmile',
	author_email='Gegel85@laposte.net',
	description='Pokemon Generation 1 simulator, with emulator link cable support'
)
