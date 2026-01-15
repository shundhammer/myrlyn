# -*- mode: makefile -*-
#
# CMakeLists.txt for myrlyn/src

include( ../VERSION.cmake )
include( GNUInstallDirs )

set( CMAKE_AUTORCC ON )

set( TARGETBIN myrlyn )

set( SOURCES

  main.cc
  BaseProduct.cc
  CommunityRepos.cc
  MyrlynApp.cc
  Translator.cc
  MyrlynWorkflowSteps.cc
  MyrlynRepoManager.cc
  BusyPopup.cc
  LicenseCache.cc
  Logger.cc
  LogStream.cc
  Exception.cc
  FSize.cc
  InitReposPage.cc
  KeyRingCallbacks.cc
  MainWindow.cc
  PkgCommitCallbacks.cc
  PkgCommitPage.cc
  PkgTasks.cc
  PkgTaskListWidget.cc
  PopupLogo.cc
  ProgressDialog.cc
  RepoConfigDialog.cc
  RepoEditDialog.cc
  RepoGpgKeyImportDialog.cc
  RepoTable.cc
  SearchFilter.cc
  SummaryPage.cc
  WindowSettings.cc
  Workflow.cc

  ZyppHistory.cc
  ZyppHistoryBrowser.cc
  ZyppHistoryEvents.cc
  ZyppHistoryFilterDialog.cc
  ZyppHistoryParser.cc
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
  YQPkgRpmGroupsFilterView.cc
  YQPkgSearchFilterView.cc
  YQPkgSecondaryFilterView.cc
  YQPkgServiceFilterView.cc
  YQPkgServiceList.cc
  YQPkgStatusFilterView.cc
  YQPkgTechnicalDetailsView.cc
  YQPkgTextDialog.cc
  YQPkgUpdatesFilterView.cc
  YQPkgVersionsView.cc

  YQIconPool.cc
  YRpmGroupsTree.cc
  YStringTree.cc

  QY2ComboTabWidget.cc
  QY2CursorHelper.cc
  QY2DiskUsageList.cc
  QY2IconLoader.cc
  QY2LayoutUtils.cc
  QY2ListView.cc
  )

# Needed only for the RPM groups view:
#
#   YQPkgRpmGroupsFilterView.h
#   YQPkgRpmGroupsFilterView.cc
#   YRpmGroupsTree.h
#   YRpmGroupsTree.cc
#   YStringTree.h
#   YStringTree.cc
#   TreeItem.h
#   YTransText.h


# Qt resource files
set( QRC_FILES
     icons.qrc     # Compiled-in resources: icons
     artwork.qrc ) # Compiled-in resources: artwork

#
# .ui widget forms generated with Qt designer (designer-qt6) or Qt Creator
#
# For each one, the Qt uic (User Interface Compiler) generates a file
# ui_formname.h at build time to include in a .cc file.
# This is done automatically with CMAKE_AUTOUIC above.
#
# See WizardPage.{h,cc} and wizard-page.ui (and the generated ui_wizard-page.h)
# for a very simple usage example.
#

set( UI_FILES
     about-dialog.ui
     init-repos-page.ui
     pkg-commit-page.ui
     progress-dialog.ui
     repo-config.ui
     repo-edit-add.ui
     repo-gpg-key-import-dialog.ui
     search-filter-view.ui
     search-filter-view.ui
     summary-page.ui
     updates-filter-view.ui
     zypp-history-browser.ui
     zypp-history-filter-dialog.ui
   )


# Putting it all together

qt_add_executable( ${TARGETBIN}
  ${SOURCES}
  ${UI_FILES}
  ${QRC_FILES}
)


#
# Compile options and definitions
#

# Make the version from ../../VERSION.cmake available as a #define
target_compile_definitions( ${TARGETBIN} PUBLIC VERSION="${VERSION}" )

if ( USE_RPM_GROUPS )
  message( STATUS "Using RPM groups" )
  target_compile_definitions( ${TARGETBIN} PUBLIC USE_RPM_GROUPS=1 )
else()
  message( STATUS "Not Using RPM groups" )
  target_compile_definitions( ${TARGETBIN} PUBLIC USE_RPM_GROUPS=0 )
endif()


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


# message( "prefix: ${CMAKE_INSTALL_PREFIX}" )

# Executable -> /usr/bin
install( TARGETS ${TARGETBIN} RUNTIME DESTINATION bin )

# Scripts -> /usr/bin
install( PROGRAMS
	 myrlyn-sudo
	 DESTINATION bin )

# .desktop files -> /usr/share/applications
install( FILES
	 myrlyn-sudo.desktop
	 myrlyn-user.desktop
	 DESTINATION share/applications )

# myrlyn-xdg-su.desktop moved to attic/


#
# Install Program icons (artwork)
#

# From:
#
#   artwork/Myrlyn-16x16.png
#   artwork/Myrlyn-22x22.png
#   artwork/Myrlyn-24x24.png
#   artwork/Myrlyn-32x32.png
#   artwork/Myrlyn-48x48.png
#   artwork/Myrlyn-72x72.png
#
# To:
#
#   /usr/share/icons/hicolor/16x16/apps/Myrlyn.png
#   /usr/share/icons/hicolor/22x22/apps/Myrlyn.png
#   /usr/share/icons/hicolor/24x24/apps/Myrlyn.png
#   /usr/share/icons/hicolor/32x32/apps/Myrlyn.png
#   /usr/share/icons/hicolor/48x48/apps/Myrlyn.png
#   /usr/share/icons/hicolor/72x72/apps/Myrlyn.png

set( ICONS_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/icons" )

foreach( resolution 16 22 24 32 48 72 )
  set( SRC_ICON  "artwork/Myrlyn-${resolution}x${resolution}.png" )
  set( DEST_ICON_DIR "${ICONS_INSTALL_DIR}/hicolor/${resolution}x${resolution}/apps" )
  install( FILES ${SRC_ICON} DESTINATION ${DEST_ICON_DIR} RENAME "Myrlyn.png" )
endforeach()
