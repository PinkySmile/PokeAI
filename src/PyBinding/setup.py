import pathlib
import os
from Cython.Build import cythonize
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext as build_ext_orig


class CMakeExtension(Extension):
	def __init__(self, name):
		# don't invoke the original build_ext for this special extension
		super().__init__(name, sources=[])


class build_ext(build_ext_orig):
	def run(self):
		cmake_exts = [ext for ext in self.extensions if isinstance(ext, CMakeExtension)]
		self.extensions = [ext for ext in self.extensions if not isinstance(ext, CMakeExtension)]
		for ext in cmake_exts:
			self.build_cmake(ext)
		extra_dirs = set()
		for ext in cmake_exts:
			extra_dirs.add(pathlib.Path(self.get_ext_fullpath(ext.name)).parent.absolute())
			extra_dirs.add(pathlib.Path(self.build_temp).absolute())
		for ext in self.extensions:
			ext.library_dirs += list(map(str, extra_dirs))
		super().run()

	def build_cmake(self, ext):
		cwd = pathlib.Path().absolute()

		# these dirs will be created in build_py, so if you don't have
		# any python sources to bundle, the dirs will be missing
		build_temp = pathlib.Path(self.build_temp).absolute()
		build_temp.mkdir(parents=True, exist_ok=True)
		extdir = pathlib.Path(self.get_ext_fullpath(ext.name)).parent
		extdir.mkdir(parents=True, exist_ok=True)

		config = 'Debug' if self.debug else 'Release'
		cmake_args = [
			'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + str(extdir.absolute()),
			'-DCMAKE_BUILD_TYPE=' + config
		]
		build_args = [
			'--target', ext.name,
			'--', '-j'
		]

		self.spawn(['cmake', '-S', str(cwd.parent.parent), '-B', str(build_temp)] + cmake_args)
		if not self.dry_run:
			self.spawn(['cmake', '--build', str(build_temp)] + build_args) # Troubleshooting: if fail then delete all possible temporary CMake files including "CMakeCache.txt" in top level dir.


cwd = os.getcwd()
include_dir = os.path.join(cwd, '..', '..', 'include')
source_dir = os.path.join(cwd, '..', '..', 'src')

incs = [include_dir, source_dir, '.']
libs = ['PokemonGen1Core', 'PokemonGen1Emulator']

extensions = [
	Extension("PokeBattle.Gen1.BattleHandler",      ['PokeBattle/Gen1/BattleHandler.pyx'],      include_dirs=incs, libraries=libs),
	Extension("PokeBattle.Gen1.Damage",             ['PokeBattle/Gen1/Damage.pyx'],             include_dirs=incs, libraries=libs),
	Extension("PokeBattle.Gen1.EmulatorGameHandle", ['PokeBattle/Gen1/EmulatorGameHandle.pyx'], include_dirs=incs, libraries=libs),
	Extension("PokeBattle.Gen1.Move",               ['PokeBattle/Gen1/Move.pyx'],               include_dirs=incs, libraries=libs),
	Extension("PokeBattle.Gen1.Pokemon",            ['PokeBattle/Gen1/Pokemon.pyx'],            include_dirs=incs, libraries=libs),
	Extension("PokeBattle.Gen1.RandomGenerator",    ['PokeBattle/Gen1/RandomGenerator.pyx'],    include_dirs=incs, libraries=libs),
	Extension("PokeBattle.Gen1.State",              ['PokeBattle/Gen1/State.pyx'],              include_dirs=incs, libraries=libs),
	Extension("PokeBattle.Gen1.StatsChange",        ['PokeBattle/Gen1/StatsChange.pyx'],        include_dirs=incs, libraries=libs),
	Extension("PokeBattle.Gen1.StatusChange",       ['PokeBattle/Gen1/StatusChange.pyx'],       include_dirs=incs, libraries=libs),
	Extension("PokeBattle.Gen1.Team",               ['PokeBattle/Gen1/Team.pyx'],               include_dirs=incs, libraries=libs),
	Extension("PokeBattle.Gen1.Type",               ['PokeBattle/Gen1/Type.pyx'],               include_dirs=incs, libraries=libs),
]
cython_ext = cythonize(extensions, build_dir="build")

setup(
	name='PokeBattle',
	version='1.0',
	packages=[
		'PokeBattle',
		'PokeBattle/Gen1'
	],
	url='https://github.com/PinkySmile/PokeAI',
	package_data={"PokeBattle/Gen1": ["pokeyellow.gbc"]},
	ext_modules=[CMakeExtension(lib) for lib in libs] + cython_ext,
	license='MIT',
	author='PinkySmile',
	author_email='Gegel85@laposte.net',
	description='Pokemon Generation 1 simulator, with emulator link cable support',
	cmdclass={ 'build_ext': build_ext }
)
