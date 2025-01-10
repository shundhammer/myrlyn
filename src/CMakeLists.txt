# -*- mode: makefile -*-
#
# CMakeLists.txt for myrlyn/src

include( ../VERSION.cmake )
include( GNUInstallDirs )       # set CMAKE_INSTALL_INCLUDEDIR, ..._LIBDIR

set( TARGETBIN myrlyn )

set( SOURCES

  main.cc
  MyrlynApp.cc
  MyrlynWorkflowSteps.cc
  MyrlynRepoManager.cc
  BusyPopup.cc
  Logger.cc
  Exception.cc
  FSize.cc
  InitReposPage.cc
  MainWindow.cc
  PkgCommitCallbacks.cc
  PkgCommitPage.cc
  PkgTasks.cc
  PkgTaskListWidget.cc
  ProgressDialog.cc
  SearchFilter.cc
  SummaryPage.cc
  WindowSettings.cc
  Workflow.cc
  ZyppLogger.cc

  YQPkgSelector.cc
  YQPkgSelectorBase.cc
  YQPkgSelectorHelp.cc

  YQPkgChangeLogView.cc
  YQPkgChangesDialog.cc
  YQPkgClassificationFilterView.cc
  YQPkgConflictDialog.cc
  YQPkgConflictList.cc
  YQPkgDependenciesView.cc
  YQPkgDescriptionDialog.cc
  YQPkgDescriptionView.cc
  YQPkgDiskUsageList.cc
  YQPkgDiskUsageWarningDialog.cc
  YQPkgFileListView.cc
  YQPkgFilterTab.cc
  YQPkgFilters.cc
  YQPkgGenericDetailsView.cc
  YQPkgHistoryDialog.cc
  YQPkgLangList.cc
  YQPkgList.cc
  YQPkgObjList.cc
  YQPkgPatchFilterView.cc
  YQPkgPatchList.cc
  YQPkgPatternList.cc
  YQPkgProductDialog.cc
  YQPkgProductList.cc
  YQPkgRepoFilterView.cc
  YQPkgRepoList.cc
  YQPkgSearchFilterView.cc
  YQPkgSecondaryFilterView.cc
  YQPkgSelMapper.cc
  YQPkgServiceFilterView.cc
  YQPkgServiceList.cc
  YQPkgStatusFilterView.cc
  YQPkgTechnicalDetailsView.cc
  YQPkgTextDialog.cc
  YQPkgUpdatesFilterView.cc
  YQPkgVersionsView.cc

  YQIconPool.cc
  QY2ComboTabWidget.cc
  QY2CursorHelper.cc
  QY2DiskUsageList.cc
  QY2IconLoader.cc
  QY2LayoutUtils.cc
  QY2ListView.cc
  )

# Qt resource files
set( QRC_FILES
     icons.qrc     # Compiled-in resources: icons
     artwork.qrc ) # Compiled-in resources: artwork

#
# .ui widget forms generated with Qt designer (designer-qt5) or Qt Creator
#
# For each one, the Qt uic (User Interface Compiler) generates a file
# ui_formname.h at build time to include in a .cc file.
# This is done automatically with CMAKE_AUTOUIC above.
#
# See WizardPage.{h,cc} and wizard-page.ui (and the generated ui_wizard-page.h)
# for a very simple usage example.
#

set( UI_FILES
     init-repos-page.ui
     pkg-commit-page.ui
     progress-dialog.ui
     search-filter-view.ui
     search-filter-view.ui
     summary-page.ui
     updates-filter-view.ui
   )

# Putting it all together
qt_add_executable(${TARGETBIN}
  ${SOURCES}
  ${UI_FILES}
)
qt_add_resources(${TARGETBIN} "icons" FILES icons.qrc)


#
# Compile options and definitions
#

# Make the version from ../../VERSION.cmake available as a #define
target_compile_definitions( ${TARGETBIN} PUBLIC VERSION="${VERSION}" )


#
# For debugging
#

### Log filter views (left pane)
# target_compile_definitions( ${TARGETBIN} PUBLIC VERBOSE_FILTER_VIEWS=1 )

### Log details views (bottom right tabs)
# target_compile_definitions( ${TARGETBIN} PUBLIC VERBOSE_DETAILS_VIEWS=1 )

### Log initial list selection in QY2ListView (pkg, repo, pattern, lang lists)
# target_compile_definitions( ${TARGETBIN} PUBLIC VERBOSE_SELECT_SOMETHING=1 )


# Add this $TOPDIR/src directory as an include directory for headers generated
# into $(TOPDIR)/build/src/myrlyn_autogen from .ui files by uic.  This is needed
# when using custom widget classes in Qt Designer: They include the headers for
# those custom widget classes, but they are not in that build/.../..._autogen
# directory, but in src/ where they belong.
# Yet another problem to fix for CMake that wouldn't exist without CMake.

target_include_directories( ${TARGETBIN} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )

# Workaround for boost::bind() complaining about deprecated _1 placeholder
# deep in the libzypp headers
target_compile_definitions( ${TARGETBIN} PUBLIC BOOST_BIND_GLOBAL_PLACEHOLDERS=1 )

# Strip off the full path from __FILE__. See Logger.cc, Exception.cc.
target_compile_definitions( ${TARGETBIN} PUBLIC FIX_CMAKE_FILENAME_BUG=1 )


# Add more compile options to this target in addition to those
# added in the toplevel CMakeLists.txt.
#
# Notice that CMake will automatically add -fPIC etc. where needed,
# like for this shared lib.
### target_compile_options( ${TARGETBIN} PUBLIC "-Dfoo" )

# Show the complete compiler commands with all arguments:
#   make VERBOSE=1

# Add more compile options to an individual source file:
### set_source_files_properties( YUI.cc PROPERTIES COMPILE_OPTIONS "-Dfoo" )




#
# Linking
#


# Libraries that are needed to build this executable
#
# If in doubt what is really needed, check with "ldd -u" which libs are unused.
target_link_libraries( myrlyn
  PRIVATE
  zypp
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  )

# Notice that we don't link against Qt5::Svg, but we need it at runtime:
#
# It's a plugin for Qt and will be used to load SVGs (like our icons) if
# libQt5Svg is available. But we don't use it directly here, only via Qt
# classes like QPixmap and QIcon. Qt takes loads the SVG plugin as needed.



#
# Install
#

install(TARGETS ${TARGETBIN} RUNTIME DESTINATION bin)
