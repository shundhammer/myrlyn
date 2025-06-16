# -*- mode: makefile -*-
#
# CMakeLists.txt for myrlyn/src

include( GNUInstallDirs )

set( CMAKE_AUTOUIC ON )

set( TARGETBIN myrlyn-askpass )
set( SOURCES   myrlyn-askpass.cc )
set( UI_FILES  myrlyn-askpass.ui )


# Putting it all together

qt_add_executable( ${TARGETBIN}
  ${SOURCES}
  ${UI_FILES}
)


#
# Compile options and definitions
#

# Add this $TOPDIR/src directory as an include directory for headers generated
# into $(TOPDIR)/build/src/myrlyn-askpass_autogen from .ui files by uic.  This
# is needed when using custom widget classes in Qt Designer: They include the
# headers for those custom widget classes, but they are not in that
# build/.../..._autogen directory, but in aux/ where they belong.
# Yet another problem to fix for CMake that wouldn't exist without CMake.

target_include_directories( ${TARGETBIN} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )


# Add more compile options to this target in addition to those
# added in the toplevel CMakeLists.txt.
#
# Notice that CMake will automatically add -fPIC etc. where needed,
# like for this shared lib.
### target_compile_options( ${TARGETBIN} PUBLIC "-Dfoo" )

# Show the complete compiler commands with all arguments:
#   make VERBOSE=1


#
# Linking
#


# Libraries that are needed to build this executable
#
# If in doubt what is really needed, check with "ldd -u" which libs are unused.
target_link_libraries( myrlyn-askpass
  PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  )



#
# Install
#


# message( "prefix: ${CMAKE_INSTALL_PREFIX}" )

# Executable -> /usr/bin
install( TARGETS ${TARGETBIN} RUNTIME DESTINATION bin )

