.PHONY:default
default:
	$(error Must specify target)

CMAKE_DEFINES:=

ifdef RELEASE_MODE
ifdef RELEASE_NAME
CMAKE_DEFINES:=$(CMAKE_DEFINES) -DRELEASE_NAME=$(RELEASE_NAME)
endif
endif

BUILD_FOLDER:=build

ifeq ($(OS),Windows_NT)

PYTHON3:=py -3

# https://github.com/muttleyxd/clang-tools-static-binaries/releases
CLANG_FORMAT:=bin/clang-format-11_windows-amd64.exe

CAT:=cmd /c type

NPROC:=$(NUMBER_OF_PROCESSORS)

include Makefile.windows.mak

else

# Is this how you're supposed to do it? Works for me on macOS, at
# least...
PYTHON3:=python3

# version number roulette.
CLANG_FORMAT:=clang-format

CAT:=cat

UNAME:=$(shell uname -s)

ifeq ($(UNAME),Darwin)
OS:=osx
NPROC:=$(shell sysctl -n hw.ncpu)
INSTALLER:=
CMAKE_DEFINES:=$(CMAKE_DEFINES) -DCMAKE_OSX_DEPLOYMENT_TARGET=10.9

include Makefile.unix.mak
include Makefile.osx.mak

endif

ifeq ($(UNAME),Linux)
OS:=linux
NPROC:=$(shell nproc)
INSTALLER:=1
CMAKE_DEFINES:=$(CMAKE_DEFINES) -DUSE_SYSTEM_SDL=1
include Makefile.unix.mak
endif

endif

SHELLCMD:=$(PYTHON3) ./submodules/shellcmd.py/shellcmd.py

##########################################################################
##########################################################################

.PHONY:rel
rel:
	$(PYTHON3) ./etc/release/release.py --make=$(MAKE)

##########################################################################
##########################################################################

.PHONY:rel_tests
rel_tests:
	$(PYTHON3) ./etc/b2_tests/rel_tests.py

##########################################################################
##########################################################################

.PHONY:clang-format
clang-format:
	$(SHELLCMD) cat experimental/clang-format.header.txt src/.clang-format experimental/clang-format.header.txt > experimental/.clang-format

	$(SHELLCMD) mkdir $(BUILD_FOLDER)
	$(PYTHON3) ./bin/make_clang-format_makefile.py -o "$(BUILD_FOLDER)/clang-format.mak" -e "$(CLANG_FORMAT)" src experimental
	$(MAKE) -f "$(BUILD_FOLDER)/clang-format.mak" -j $(NPROC)
