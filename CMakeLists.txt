# -*- mode: makefile -*-
#
# CMakeLists.txt for Myrlyn
#
# Usage:
#
#   mkdir build
#   cd build
#   cmake ..
#
#   make
#   sudo make install
#
# Restart with a clean build environment:
#   rm -rf build
#
# Show the complete compiler commands with all arguments:
#   make VERBOSE=1
#

cmake_minimum_required( VERSION 3.17 )
project( myrlyn LANGUAGES CXX )

# Options usage:
#
#   cmake -DBUILD_TEST=off ..

option( BUILD_SRC         "Build in src/ subdirectory"                on  )
option( BUILD_TEST        "Build in test/ subdirectory"               off )
option( WERROR            "Treat all compiler warnings as errors"     on  )


#----------------------------------------------------------------------


# We use /usr as the default CMAKE_INSTALL_PREFIX, but it can be set on the
# cmake command line with
#
#   cmake -DCMAKE_INSTALL_PREFIX=/my/install/prefix ..
#
# or in the environment with
#
#   CMAKE_INSTALL_PREFIX=/usr/local  cmake ..

if ( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
  if ( DEFINED ENV{CMAKE_INSTALL_PREFIX} )
    set( INSTALL_PREFIX $ENV{CMAKE_INSTALL_PREFIX} )
  else()
    set( INSTALL_PREFIX /usr )
  endif()
  set( CMAKE_INSTALL_PREFIX "${INSTALL_PREFIX}" CACHE PATH "Install path prefix" FORCE)
endif()

message( "-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}" )

include( GNUInstallDirs )       # set CMAKE_INSTALL_INCLUDEDIR, ..._LIBDIR

set( CMAKE_INSTALL_MESSAGE LAZY ) # Suppress "up-to-date" messages during "make install"


# Initialize compiler flags for all targets in all subdirectories
add_compile_options( "-Wall" )
add_compile_options( "-Os" )    # Optimize for size (overrides CMake's -O3 in RELEASE builds)

if ( WERROR )
  add_compile_options( "-Werror" )
endif()

# libzypp and Qt 6 require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

qt_standard_project_setup()

#
# Descend into subdirectories
#

if ( BUILD_SRC )
  add_subdirectory( src )
endif()

if ( BUILD_TEST )
  add_subdirectory( test )
endif()

#
# Install
#

install( FILES README.md DESTINATION ${CMAKE_INSTALL_DOCDIR} )
install( FILES LICENSE   DESTINATION "${CMAKE_INSTALL_PREFIX}/share/licenses/myrlyn" )
