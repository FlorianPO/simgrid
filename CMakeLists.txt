cmake_minimum_required(VERSION 2.6)
### Need to set rc ccompiler before enable language
if(WIN32)
  SET(CMAKE_RC_COMPILER "windres")
endif()
project(SimGrid C)

enable_language(CXX)

INCLUDE(CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG(-fstack-cleaner HAVE_C_STACK_CLEANER)

if (APPLE) #MAC
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

if ((NOT DEFINED enable_smpi OR enable_smpi) AND NOT APPLE) # smpi is enabled by default
  # Call enable_language(Fortran) in order to load the build rules for
  # this language, needed by teshsuite/smpi/mpich-test/.  Use
  # CMAKE_FORCE_Fortran_COMPILER to bypass checks for a working
  # compiler (smpiff don't exist at configure time).
  include(CMakeForceCompiler)
  if(NOT COMMAND CMAKE_FORCE_Fortran_COMPILER)
    MACRO(CMAKE_FORCE_Fortran_COMPILER compiler id)
      SET(CMAKE_Fortran_COMPILER "${compiler}")
      SET(CMAKE_Fortran_COMPILER_ID_RUN TRUE)
      SET(CMAKE_Fortran_COMPILER_ID ${id})
      SET(CMAKE_Fortran_COMPILER_WORKS TRUE)
      SET(CMAKE_Fortran_COMPILER_FORCED TRUE)

      # Set old compiler id variables.
      IF("${CMAKE_Fortran_COMPILER_ID}" MATCHES "GNU")
        SET(CMAKE_COMPILER_IS_GNUG77 1)
      ENDIF("${CMAKE_Fortran_COMPILER_ID}" MATCHES "GNU")
    ENDMACRO(CMAKE_FORCE_Fortran_COMPILER)
  endif()
  CMAKE_FORCE_Fortran_COMPILER(smpiff smpiff)
  enable_language(Fortran OPTIONAL)
endif()

## Mapping version number -> version name
# 3.5.99 -> alpha1 (oops)
# 3.5.9{1,2} -> beta{1,2}
# 3.5.9{3,4,5} -> rc{1,2,3}
# 3.6.{0,1,2} -> release 3.6, 3.6.1, 3.6.2
# 3.7.{0,1} -> release 3.7, 3.7.1
# 3.8.{0,1} -> release 3.8, 3.8.1
# 3.9.0 -> release 3.9
# 3.9.90 -> release 3.10pre1
# 3.10.0 -> release 3.10
# 3.11.0 -> release 3.11
# 3.12.0 -> release 3.12

set(SIMGRID_VERSION_MAJOR "3")
set(SIMGRID_VERSION_MINOR "12")
set(SIMGRID_VERSION_PATCH "0")
set(SIMGRID_VERSION_EXTRA "-devel") # Extra words to add to version string (e.g. -rc1)

set(SIMGRID_VERSION_DATE  "2015") # Year for copyright information

if(${SIMGRID_VERSION_PATCH} EQUAL "0")
  set(release_version "${SIMGRID_VERSION_MAJOR}.${SIMGRID_VERSION_MINOR}")
else()
  set(release_version "${SIMGRID_VERSION_MAJOR}.${SIMGRID_VERSION_MINOR}.${SIMGRID_VERSION_PATCH}")
endif()

set(SIMGRID_VERSION_STRING
  "SimGrid version ${release_version}${SIMGRID_VERSION_EXTRA}\\nCopyright (c) 2004-${SIMGRID_VERSION_DATE}. The Simgrid Team.")

set(libsimgrid_version "${release_version}")
set(libsimgrid-java_version "${release_version}")
set(GCC_NEED_VERSION "4.0")
set(APPLE_NEED_GCC_VERSION "4.6")

### SET THE LIBRARY EXTENSION AND GCC VERSION
if(APPLE) #MAC
  set(LIB_EXE "dylib")
else()
  if(WIN32) #WINDOWS
    set(LIB_EXE "a")
    set(BIN_EXE ".exe")
  else() #UNIX
    set(LIB_EXE "so")
  endif()
endif()

if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
  exec_program("${CMAKE_C_COMPILER} --version" OUTPUT_VARIABLE "COMPILER_C_VERSION")
  exec_program("${CMAKE_CXX_COMPILER} --version" OUTPUT_VARIABLE "COMPILER_CXX_VERSION")
  string(REGEX MATCH "[0-9].[0-9].[0-9]" COMPILER_C_VERSION "${COMPILER_C_VERSION}")
  string(REGEX MATCH "[0-9].[0-9].[0-9]" COMPILER_CXX_VERSION "${COMPILER_CXX_VERSION}")

  string(REGEX MATCH "^[0-9].[0-9]" COMPILER_C_VERSION_MAJOR_MINOR "${COMPILER_C_VERSION}")
  string(REPLACE "${COMPILER_C_VERSION_MAJOR_MINOR}." "" COMPILER_C_VERSION_PATCH "${COMPILER_C_VERSION}")

  if(${GCC_NEED_VERSION} GREATER  COMPILER_C_VERSION_MAJOR_MINOR)
    message(FATAL_ERROR "Gcc must be to version ${GCC_NEED_VERSION} current version ${COMPILER_C_VERSION_MAJOR_MINOR}")
  endif()
  
  if(COMPILER_C_VERSION_MAJOR_MINOR STRGREATER "4.8")
    set (CMAKE_AR gcc-ar)
    set (CMAKE_RANLIB gcc-ranlib)
  endif()
  
endif()

exec_program("${CMAKE_LINKER} --version" OUTPUT_VARIABLE "LINKER_VERSION")
string(REGEX MATCH "[0-9].[0-9]*" LINKER_VERSION "${LINKER_VERSION}")

string(REGEX MATCH "cl.exe" VBC "${CMAKE_C_COMPILER}")
if(VBC)
  message(FATAL_ERROR "VB is not yet supported by Simgrid.")
endif()

### Find programs and paths
FIND_PROGRAM(GCOV_PATH gcov)
include(FindPerl)
if(NOT PERL_EXECUTABLE)
  message(FATAL_ERROR "-- SimGrid cannot be compiled without Perl installed -- sorry. Bailling out.")
endif()

### Set some variables for Cmake
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(INCLUDES
  ${CMAKE_HOME_DIRECTORY}
  ${CMAKE_HOME_DIRECTORY}/include
  ${CMAKE_HOME_DIRECTORY}/src
  ${CMAKE_HOME_DIRECTORY}/src/include
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/include
  ${CMAKE_BINARY_DIR}/src
  )

if(WIN32)
  set(INCLUDES ${INCLUDES} ${CMAKE_HOME_DIRECTORY}/include/xbt ${CMAKE_HOME_DIRECTORY}/src/xbt) #for win32_ucontext.[ch]
endif()
set(CMAKE_SOURCE_DIR ${PROJECT_SOURCE_DIRECTORY})

if(NOT CMAKE_CROSSCOMPILING AND EXISTS /usr/include/)
  set(INCLUDES ${INCLUDES} /usr/include/)
endif()

### Check 32bits or 64bits
INCLUDE (CheckTypeSize)
CHECK_TYPE_SIZE("void*" SIZEOF_VOIDSTAR)
IF(SIZEOF_VOIDSTAR EQUAL 4)
  SET(ARCH_32_BITS 1)
ELSE()
  SET(ARCH_32_BITS 0)
ENDIF()

if(WIN32)

  #Need env INCLUDE
  set(CMAKE_INCLUDE_WIN "${CMAKE_C_COMPILER}")
  set(CMAKE_LIB_WIN "${CMAKE_C_COMPILER}")
  string(REGEX REPLACE "/bin/gcc.*" "/include"  CMAKE_INCLUDE_WIN "${CMAKE_INCLUDE_WIN}")
  string(REGEX REPLACE "/bin/gcc.*" "/lib"  CMAKE_LIB_WIN "${CMAKE_LIB_WIN}")
  set(INCLUDES ${INCLUDES} ${CMAKE_INCLUDE_WIN})

  if(CMAKE_COMPILER_IS_GNUCC)
    set(__GNUC__ 1)
    exec_program("${CMAKE_C_COMPILER} --version" OUTPUT_VARIABLE "COMPILER_C_VERSION")
    string(REGEX MATCH "[0-9].[0-9].[0-9]" COMPILER_C_VERSION "${COMPILER_C_VERSION}")
    string(REGEX MATCH "^[0-9]" COMPILER_C_MAJOR_VERSION "${COMPILER_C_VERSION}")
    string(REGEX MATCH "^[0-9].[0-9]" COMPILER_C_MINOR_VERSION "${COMPILER_C_VERSION}")
    string(REGEX REPLACE "^${COMPILER_C_MAJOR_VERSION}." "" COMPILER_C_MINOR_VERSION "${COMPILER_C_MINOR_VERSION}")
    if(COMPILER_C_MAJOR_VERSION)
      # set(__GNUC__ ${COMPILER_C_MAJOR_VERSION})
    endif()
    if(COMPILER_C_MINOR_VERSION)
      # set(__GNUC_MINOR__ ${COMPILER_C_MINOR_VERSION})
    endif()
    set(MSVC 0)
    set(BORLAND 0)
  else()
    message(FATAL_ERROR "Please use MinGW to compile SimGrid!")
  endif()

  if(ARCH_32_BITS)      ### Arch 32bits
    set(_WIN32 1)
  else()        ### Arch 64bits
    set(_WIN64 1)
  endif()

  set(NSIS_WIN_VERSION $ENV{PROCESSOR_ARCHITEW6432})
  if(NSIS_WIN_VERSION MATCHES "")
    set(NSIS_WIN_VERSION $ENV{PROCESSOR_ARCHITECTURE})
  endif()
  if(${NSIS_WIN_VERSION})
    string(TOLOWER ${NSIS_WIN_VERSION} NSIS_WIN_VERSION)
  endif()

  set(_XBT_WIN32 1)

  message(STATUS "C_COMPILER                    ${CMAKE_C_COMPILER} ${COMPILER_C_VERSION}")
  message(STATUS "CXX_COMPILER                  ${CMAKE_CXX_COMPILER} ${COMPILER_CXX_VERSION}")
  message(STATUS "CMAKE_RC_COMPILER             ${CMAKE_RC_COMPILER}")
  message(STATUS "INCLUDE                       ${CMAKE_INCLUDE_WIN}")
  message(STATUS "LIB                           ${CMAKE_LIB_WIN}")
  message(STATUS "MAKE_PROGRAM                  ${CMAKE_MAKE_PROGRAM}")
  message(STATUS "CMAKE_BUILD_TOOL              ${CMAKE_BUILD_TOOL}")
  message(STATUS "LINKER                        ${CMAKE_LINKER}")
  message(STATUS "CMAKE_GENERATOR               ${CMAKE_GENERATOR}")
  message(STATUS "BORLAND                       ${BORLAND}")
  message(STATUS "VISUALC                       ${MSVC}")
  message(STATUS "GNUC                          ${CMAKE_COMPILER_IS_GNUCC}")

endif()

include_directories(${INCLUDES})

### Setup Options
include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/Option.cmake)

### Make the *.h files with *.h.in files
include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/CompleteInFiles.cmake)

### Define source packages for Libs
include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/DefinePackages.cmake)

### Build some Maintainer files
include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/MaintainerMode.cmake)
include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/UnitTesting.cmake)

### Setup gcc flags
include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/Flags.cmake)

### Make Libs
if(NOT WIN32)
  include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/MakeLib.cmake)
else()
  include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/MakeLibWin.cmake)
endif()

### Make Exes
include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/MakeExe.cmake)

### Make tests
if(enable_memcheck_xml)
  set(enable_memcheck true)
endif()

include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/AddTests.cmake)
include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/CTestConfig.cmake)

### Setup the distrib
include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/Distrib.cmake)

### Pipol compilation
include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/Pipol.cmake)

### Build the doc
if(NOT WIN32)
  include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/GenerateDoc.cmake)
else()
  include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/GenerateDocWin.cmake)
endif()

### Print ARGS
include(${CMAKE_HOME_DIRECTORY}/buildtools/Cmake/PrintArgs.cmake)

INCLUDE(Dart)
