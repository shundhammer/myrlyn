# -*- mode: makefile -*-
#
# CMakeLists.txt for myrlyn/test/workflow-tester
#
# Building:
#
#   cd <project-root>
#   mkdir build
#   cd build
#   cmake -DBUILD_TEST=on -DBUILD_SRC=on ..
#   make
#
# or
#
#   cd test
#   make
#
# Start with
#
#   test/workflow-tester/workflow-tester

include( GNUInstallDirs )       # set CMAKE_INSTALL_INCLUDEDIR, ..._LIBDIR

#
# Qt-specific
#

set( TARGETBIN workflow-tester )

set( SOURCES
  workflow-tester.cc
  ../../src/Logger.cc
  ../../src/Exception.cc
  ../../src/Workflow.cc
  )

qt_add_executable( ${TARGETBIN}
  ${SOURCES}
)


#
# Compile options and definitions
#

# Workaround for boost::bind() complaining about deprecated _1 placeholder
# deep in the libzypp headers
target_compile_definitions( ${TARGETBIN} PUBLIC BOOST_BIND_GLOBAL_PLACEHOLDERS=1 )


# Special for this workflow-tester: Make the Workflow class more verbose
target_compile_definitions( ${TARGETBIN} PUBLIC VERBOSE_WORKFLOW=1 )


#
# Linking
#


# Libraries that are needed to build this executable
#
# If in doubt what is really needed, check with "ldd -u" which libs are unused.
target_link_libraries( workflow-tester
  PRIVATE
  Qt6::Core
  )
