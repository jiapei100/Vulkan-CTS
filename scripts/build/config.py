# -*- coding: utf-8 -*-

#-------------------------------------------------------------------------
# drawElements Quality Program utilities
# --------------------------------------
#
# Copyright 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#-------------------------------------------------------------------------

import os
import sys
import copy
import platform
import multiprocessing

from common import which, DEQP_DIR

try:
	import _winreg
except:
	_winreg = None

class BuildConfig:
	def __init__ (self, buildDir, buildType, args, srcPath = DEQP_DIR):
		self.srcPath		= srcPath
		self.buildDir		= buildDir
		self.buildType		= buildType
		self.args			= copy.copy(args)

	def getSrcPath (self):
		return self.srcPath

	def getBuildDir (self):
		return self.buildDir

	def getBuildType (self):
		return self.buildType

	def getArgs (self):
		return self.args

class CMakeGenerator:
	def __init__ (self, name, isMultiConfig = False, extraBuildArgs = []):
		self.name			= name
		self.isMultiConfig	= isMultiConfig
		self.extraBuildArgs	= copy.copy(extraBuildArgs)

	def getName (self):
		return self.name

	def getGenerateArgs (self, buildType):
		args = ['-G', self.name]
		if not self.isMultiConfig:
			args.append('-DCMAKE_BUILD_TYPE=%s' % buildType)
		return args

	def getBuildArgs (self, buildType):
		args = []
		if self.isMultiConfig:
			args += ['--config', buildType]
		if len(self.extraBuildArgs) > 0:
			args += ['--'] + self.extraBuildArgs
		return args

	def getBinaryPath (self, buildType, basePath):
		return basePath

class UnixMakefileGenerator(CMakeGenerator):
	def __init__(self):
		CMakeGenerator.__init__(self, "Unix Makefiles", extraBuildArgs = ["-j%d" % multiprocessing.cpu_count()])

	def isAvailable (self):
		return which('make') != None

class NinjaGenerator(CMakeGenerator):
	def __init__(self):
		CMakeGenerator.__init__(self, "Ninja")

	def isAvailable (self):
		return which('ninja') != None

class VSProjectGenerator(CMakeGenerator):
	ARCH_32BIT	= 0
	ARCH_64BIT	= 1

	def __init__(self, version, arch):
		name = "Visual Studio %d" % version
		if arch == self.ARCH_64BIT:
			name += " Win64"

		CMakeGenerator.__init__(self, name, isMultiConfig = True, extraBuildArgs = ['/m'])
		self.version		= version
		self.arch			= arch

	def getBinaryPath (self, buildType, basePath):
		return os.path.join(os.path.dirname(basePath), buildType, os.path.basename(basePath) + ".exe")

	@staticmethod
	def getNativeArch ():
		arch = platform.machine().lower()

		if arch == 'x86':
			return VSProjectGenerator.ARCH_32BIT
		elif arch == 'amd64':
			return VSProjectGenerator.ARCH_64BIT
		else:
			raise Exception("Unhandled arch '%s'" % arch)

	@staticmethod
	def registryKeyAvailable (root, arch, name):
		try:
			key = _winreg.OpenKey(root, name, 0, _winreg.KEY_READ | arch)
			_winreg.CloseKey(key)
			return True
		except:
			return False

	def isAvailable (self):
		if sys.platform == 'win32' and _winreg != None:
			nativeArch = VSProjectGenerator.getNativeArch()
			if nativeArch == self.ARCH_32BIT and self.arch == self.ARCH_64BIT:
				return False

			arch = _winreg.KEY_WOW64_32KEY if nativeArch == self.ARCH_64BIT else 0
			keyMap = {
				10:		[(_winreg.HKEY_CLASSES_ROOT, "VisualStudio.DTE.10.0"), (_winreg.HKEY_LOCAL_MACHINE, "Software\\Microsoft\\VCExpress\\10.0")],
				11:		[(_winreg.HKEY_CLASSES_ROOT, "VisualStudio.DTE.11.0"), (_winreg.HKEY_LOCAL_MACHINE, "Software\\Microsoft\\VCExpress\\11.0")],
				12:		[(_winreg.HKEY_CLASSES_ROOT, "VisualStudio.DTE.12.0"), (_winreg.HKEY_LOCAL_MACHINE, "Software\\Microsoft\\VCExpress\\12.0")],
				14:		[(_winreg.HKEY_CLASSES_ROOT, "VisualStudio.DTE.14.0"), (_winreg.HKEY_LOCAL_MACHINE, "Software\\Microsoft\\VCExpress\\14.0")],
			}

			if not self.version in keyMap:
				raise Exception("Unsupported VS version %d" % self.version)

			keys = keyMap[self.version]
			for root, name in keys:
				if VSProjectGenerator.registryKeyAvailable(root, arch, name):
					return True
			return False
		else:
			return False

# Pre-defined generators

MAKEFILE_GENERATOR		= UnixMakefileGenerator()
NINJA_GENERATOR			= NinjaGenerator()
VS2010_X32_GENERATOR	= VSProjectGenerator(10, VSProjectGenerator.ARCH_32BIT)
VS2010_X64_GENERATOR	= VSProjectGenerator(10, VSProjectGenerator.ARCH_64BIT)
VS2012_X32_GENERATOR	= VSProjectGenerator(11, VSProjectGenerator.ARCH_32BIT)
VS2012_X64_GENERATOR	= VSProjectGenerator(11, VSProjectGenerator.ARCH_64BIT)
VS2013_X32_GENERATOR	= VSProjectGenerator(12, VSProjectGenerator.ARCH_32BIT)
VS2013_X64_GENERATOR	= VSProjectGenerator(12, VSProjectGenerator.ARCH_64BIT)
VS2015_X32_GENERATOR	= VSProjectGenerator(14, VSProjectGenerator.ARCH_32BIT)
VS2015_X64_GENERATOR	= VSProjectGenerator(14, VSProjectGenerator.ARCH_64BIT)

def selectFirstAvailableGenerator (generators):
	for generator in generators:
		if generator.isAvailable():
			return generator
	return None

ANY_VS_X32_GENERATOR	= selectFirstAvailableGenerator([
								VS2015_X32_GENERATOR,
								VS2013_X32_GENERATOR,
								VS2012_X32_GENERATOR,
								VS2010_X32_GENERATOR,
							])
ANY_VS_X64_GENERATOR	= selectFirstAvailableGenerator([
								VS2015_X64_GENERATOR,
								VS2013_X64_GENERATOR,
								VS2012_X64_GENERATOR,
								VS2010_X64_GENERATOR,
							])
ANY_UNIX_GENERATOR		= selectFirstAvailableGenerator([
								NINJA_GENERATOR,
								MAKEFILE_GENERATOR,
							])
ANY_GENERATOR			= selectFirstAvailableGenerator([
								VS2015_X64_GENERATOR,
								VS2015_X32_GENERATOR,
								VS2013_X64_GENERATOR,
								VS2012_X64_GENERATOR,
								VS2010_X64_GENERATOR,
								VS2013_X32_GENERATOR,
								VS2012_X32_GENERATOR,
								VS2010_X32_GENERATOR,
								NINJA_GENERATOR,
								MAKEFILE_GENERATOR,
							])
