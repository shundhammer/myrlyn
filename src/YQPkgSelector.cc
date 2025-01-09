/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include <fstream>
#include <algorithm>
#include <boost/bind/bind.hpp>

#include <zypp/SysContent.h>
#include <zypp/base/String.h>
#include <zypp/base/Sysconfig.h>

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QSplitter>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

#include "Exception.h"
#include "Logger.h"
#include "QY2CursorHelper.h"
#include "QY2LayoutUtils.h"
#include "WindowSettings.h"
#include "MyrlynApp.h"
#include "YQPkgChangeLogView.h"
#include "YQPkgChangesDialog.h"
#include "YQPkgClassificationFilterView.h"
#include "YQPkgConflictDialog.h"
#include "YQPkgConflictList.h"
#include "YQPkgDependenciesView.h"
#include "YQPkgDescriptionView.h"
#include "YQPkgDiskUsageList.h"
#include "YQPkgDiskUsageWarningDialog.h"
#include "YQPkgFileListView.h"
#include "YQPkgFilterTab.h"
#include "YQPkgHistoryDialog.h"
#include "YQPkgLangList.h"
#include "YQPkgList.h"
#include "YQPkgPatchFilterView.h"
#include "YQPkgPatchList.h"
#include "YQPkgPatternList.h"
#include "YQPkgProductDialog.h"
#include "YQPkgRepoFilterView.h"
#include "YQPkgRepoList.h"
#include "YQPkgSearchFilterView.h"
#include "YQPkgServiceFilterView.h"
#include "YQPkgStatusFilterView.h"
#include "YQPkgTechnicalDetailsView.h"
#include "YQPkgTextDialog.h"
#include "YQPkgUpdatesFilterView.h"
#include "YQPkgVersionsView.h"
#include "YQZypp.h"
#include "YQi18n.h"
#include "utf8.h"

#include "YQPkgSelector.h"

#define USE_UPDATE_PROBLEM_FILTER_VIEW                  0
#define CHECK_DEPENDENCIES_ON_STARTUP                   0
#define DEPENDENCY_FEEDBACK_IF_OK                       1
#define AUTO_CHECK_DEPENDENCIES_DEFAULT                 true
#define ALWAYS_SHOW_PATCHES_VIEW_IF_PATCHES_AVAILABLE   0
#define GLOBAL_UPDATE_CONFIRMATION_THRESHOLD            20

#define PATH_TO_YAST_SYSCONFIG          "/etc/sysconfig/yast2"
#define OPTION_VERIFY                   "PKGMGR_VERIFY_SYSTEM"
#define OPTION_AUTO_CHECK               "PKGMGR_AUTO_CHECK"
#define OPTION_RECOMMENDED              "PKGMGR_RECOMMENDED"

#if USE_UPDATE_PROBLEM_FILTER_VIEW
#  include "YQPkgUpdateProblemFilterView.h"
#endif

using std::max;
using std::string;
using std::map;
using std::pair;


YQPkgSelector::YQPkgSelector( QWidget * parent,
                              long      modeFlags )
    : YQPkgSelectorBase( parent, modeFlags )
{
    _blockResolver               = true;
    _showChangesDialog           = true;
    _autoDependenciesAction      = 0;
    _detailsViews                = 0;
    _filters                     = 0;
    _langList                    = 0;
    _notifications               = 0;
    _pkgClassificationFilterView = 0;
    _patchFilterView             = 0;
    _patchList                   = 0;
    _patternList                 = 0;
    _pkgChangeLogView            = 0;
    _pkgDependenciesView         = 0;
    _pkgDescriptionView          = 0;
    _pkgFileListView             = 0;
    _pkgList                     = 0;
    _pkgTechnicalDetailsView     = 0;
    _pkgVersionsView             = 0;
    _repoFilterView              = 0;
    _serviceFilterView           = 0;
    _searchFilterView            = 0;
    _statusFilterView            = 0;
    _updateProblemFilterView     = 0;
    _updatesFilterView           = 0;
    _excludeDevelPkgs            = 0;
    _excludeDebugInfoPkgs        = 0;

    logDebug() << "Creating YQPkgSelector..." << Qt::endl;

    if ( onlineUpdateMode() )   logInfo() << "Online update mode" << Qt::endl;
    if ( updateMode() )         logInfo() << "Update mode" << Qt::endl;

    basicLayout();
    addMenus();         // Only after all widgets are created!
    readSettings();     // Only after menus are created!
    makeConnections();
    emit loadData();

    _filters->readSettings();
    bool pagesRestored = _filters->tabCount() > 0;

    if ( ! pagesRestored )
    {
        logDebug() << "No page configuration saved, using fallbacks" << Qt::endl;

        //
        // Add a number of default tabs in the desired order
        //

        if ( _searchFilterView  ) _filters->showPage( _searchFilterView  );
        if ( _updatesFilterView ) _filters->showPage( _updatesFilterView );
        if ( _repoFilterView    ) _filters->showPage( _repoFilterView    );
        if ( _patternList       ) _filters->showPage( _patternList       );
        if ( _statusFilterView  ) _filters->showPage( _statusFilterView  );

        // Select the active page
        if ( _searchFilterView  ) _filters->showPage( _searchFilterView  );
    }


    //
    // Move the desired tab to the foreground
    //

    if ( _patchFilterView && onlineUpdateMode() )
    {
        if ( _patchFilterView && _patchList )
        {
            _filters->showPage( _patchFilterView );
        }
    }
    else if ( _repoFilterView && repoMode() )
    {
        if ( YQPkgRepoList::countEnabledRepositories() > 1 )
        {
            _filters->showPage( _repoFilterView );
        }
    }
#if USE_UPDATE_PROBLEM_FILTER_VIEW
    else if ( _updateProblemFilterView )
    {
        _filters->showPage( _updateProblemFilterView );
    }
#endif
    else if ( searchMode() && _searchFilterView )
    {
        if ( _pkgClassificationFilterView && anyRetractedPkgInstalled() )
        {
            // Exceptional case: If the system has any retracted package
            // installed, switch to that filter view and show those packages.
            // This should happen only very, very rarely.

            logInfo() << "Found installed retracted packages; switching to that view" << Qt::endl;
            _filters->showPage( _pkgClassificationFilterView );
            _pkgClassificationFilterView->showPkgClass( YQPkgClassRetractedInstalled );

            // Also show a pop-up warning?
            //
            // This could become very annoying really quickly because you'll
            // get it with every start of the package selection as long as any
            // retracted package version is installed (which might be a
            // deliberate conscious decision by the user). It's also not easy
            // to add a "Don't show this again" checkbox in such a pop-up;
            // which retracted packages are installed might change between
            // program runs, and we'd have to inform the user when such a
            // change occurs.
        }
        else
        {
            // Normal case: Show the "Search" filter view.

            _filters->showPage( _searchFilterView );
            QTimer::singleShot( 0, _searchFilterView, SLOT( setFocus() ) );
        }
    }
    else if ( summaryMode() && _statusFilterView )
    {
        _filters->showPage( _statusFilterView );
        _pkgList->selectNextItem();
    }


    if ( _filters->diskUsageList() )
        _filters->diskUsageList()->updateDiskUsage();


    _blockResolver = false;

#if CHECK_DEPENDENCIES_ON_STARTUP

    if ( ! testMode() )
    {
        // Fire up the first dependency check in the main loop.
        // Don't do this right away - wait until all initializations are finished.
#if 0
        QTimer::singleShot( 0, this, SLOT( resolveDependencies() ) );
#else
        if ( _pkgConflictDialog && ! MyrlynApp::isOptionSet( OptNoVerify ) )
            QTimer::singleShot( 0, _pkgConflictDialog, SLOT( verifySystemWithBusyPopup() ) );
#endif
    }
#endif

    logDebug() << "YQPkgSelector init done" << Qt::endl;
}


YQPkgSelector::~YQPkgSelector()
{
    logDebug() << "Destroying YQPkgSelector..." << Qt::endl;

    writeSettings();

    logDebug() << "Destroying YQPkgSelector done." << Qt::endl;
}


void YQPkgSelector::basicLayout()
{
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout( layout );
    layout->setContentsMargins( 6,   // left
                                0,   // top
                                6,   // right
                                6 ); // bottom
    layout->setSpacing( 6 );
    layoutMenuBar( this );

    _filters = new YQPkgFilterTab( this );
    CHECK_NEW( _filters );

    layout->addWidget( _filters );
    createFilterViews();
    _filters->showPage( 0 );

    layoutRightPane( _filters->rightPane() );
}


// --- Filter views START ---


void YQPkgSelector::createFilterViews()
{
    // The order of creation is both the order in the "View" button's menu and
    // the order of tabs

    createUpdateProblemsFilterView(); // Only if problematic packages present
    createPatchFilterView();          // Patches - Online update or F2


    // Standard views - visible by default

    createSearchFilterView();         // Package search
    createUpdatesFilterView();        // Package update
    createRepoFilterView();
    createServiceFilterView();        // Only if services available
    createPatternsFilterView();

    // Not visible by default

    createPkgClassificationFilterView();
    createLanguagesFilterView();

    // This should be the last one
    createStatusFilterView();        // a.k.a. intallation summary
}


void YQPkgSelector::createSearchFilterView()
{
    _searchFilterView = new YQPkgSearchFilterView( this );
    CHECK_NEW( _searchFilterView );

    _filters->addPage( _( "S&earch" ), _searchFilterView, "search" );
}


void YQPkgSelector::createUpdatesFilterView()
{
    _updatesFilterView = new YQPkgUpdatesFilterView( this );
    CHECK_NEW( _updatesFilterView );

    _filters->addPage( _( "&Updates" ), _updatesFilterView, "updates" );
}


void YQPkgSelector::createUpdateProblemsFilterView()
{
#if USE_UPDATE_PROBLEM_FILTER_VIEW

    if ( updateMode() )
    {
        if ( YQPkgUpdateProblemFilterView::haveProblematicPackages()
             || testMode() )
        {
            _updateProblemFilterView = new YQPkgUpdateProblemFilterView( this );
            CHECK_NEW( _updateProblemFilterView );
            _filters->addPage( _( "&Update Problems" ), _updateProblemFilterView, "update_problems" );
        }
    }
#endif
}


void YQPkgSelector::createPatchFilterView()
{
    if ( onlineUpdateMode()
#if ALWAYS_SHOW_PATCHES_VIEW_IF_PATCHES_AVAILABLE
         || ! zyppPool().empty<zypp::Patch>()
#endif
         )
    {
        addPatchFilterView();
    }
}


void YQPkgSelector::createRepoFilterView()
{
    _repoFilterView = new YQPkgRepoFilterView( this );
    CHECK_NEW( _repoFilterView );

    _filters->addPage( _( "&Repositories" ), _repoFilterView, "repos" );

}


void YQPkgSelector::createServiceFilterView()
{
    if ( YQPkgServiceFilterView::any_service() ) // Only if a service is present
    {
        _serviceFilterView = new YQPkgServiceFilterView( this );
        CHECK_NEW( _serviceFilterView );

        // TRANSLATORS: Menu item
        _filters->addPage( _( "&Services" ), _serviceFilterView, "services" );
    }
}


void YQPkgSelector::createPatternsFilterView()
{
    if ( ! zyppPool().empty<zypp::Pattern>() || testMode() )
    {
        _patternList = new YQPkgPatternList( this );
        CHECK_NEW( _patternList );

        _filters->addPage( _( "P&atterns" ), _patternList, "patterns" );
    }
}


void YQPkgSelector::createPkgClassificationFilterView()
{
    _pkgClassificationFilterView = new YQPkgClassificationFilterView( this );
    CHECK_NEW( _pkgClassificationFilterView );
    _filters->addPage( _( "Package &Classification" ), _pkgClassificationFilterView, "package_classification" );
}


void YQPkgSelector::createLanguagesFilterView()
{
    _langList = new YQPkgLangList( this );
    CHECK_NEW( _langList );

    _filters->addPage( _( "&Languages" ), _langList, "languages" );
    _langList->setSizePolicy( QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored ) ); // hor/vert
}


void YQPkgSelector::createStatusFilterView()
{
    _statusFilterView = new YQPkgStatusFilterView( this );
    CHECK_NEW( _statusFilterView );
    _filters->addPage( _( "&Installation Summary" ), _statusFilterView, "inst_summary" );
}


// --- Filter views END ---


QWidget *
YQPkgSelector::layoutRightPane( QWidget * parent )
{
    QVBoxLayout * layout = new QVBoxLayout( parent );
    CHECK_NEW( layout );
    layout->setContentsMargins( 5,   // left
                                0,   // top
                                4,   // right
                                4 ); // bottom

    QSplitter * splitter = new QSplitter( Qt::Vertical, parent );
    CHECK_NEW( splitter );
    layout->addWidget( splitter );

    layoutPkgList( splitter );
    layoutDetailsViews( splitter );
    layoutButtons( parent );

    return parent;
}


void
YQPkgSelector::layoutPkgList( QWidget * parent )
{
    QWidget * pkgListPane = new QWidget( parent );
    CHECK_NEW( pkgListPane );

    QVBoxLayout * pkgListVBox = new QVBoxLayout( pkgListPane );
    CHECK_NEW( pkgListVBox );
    pkgListVBox->setContentsMargins( 0,   // left
                                     0,   // top
                                     0,   // right
                                     4 ); // bottom

    // Notifications

    layoutNotifications( pkgListPane );
    _notifications->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) ); // hor/vert
    pkgListVBox->addWidget( _notifications );


    // Package list

    _pkgList= new YQPkgList( pkgListPane );
    CHECK_NEW( _pkgList );
    pkgListVBox->addWidget( _pkgList );

    connect( _pkgList,  SIGNAL( statusChanged()           ),
             this,      SLOT  ( autoResolveDependencies() ) );
}


void
YQPkgSelector::layoutNotifications( QWidget * parent )
{
    // This is made visible when activating the repository filter

    _notifications = new QWidget( parent );
    QVBoxLayout * notificationsLayout = new QVBoxLayout( _notifications );
    notificationsLayout->setContentsMargins( 0, 5, 0, 5 ); // left / top / right / bottom

    _switchToRepoLabel = new QLabel( _notifications );
    _switchToRepoLabel->setTextFormat( Qt::RichText );
    _switchToRepoLabel->setWordWrap( true );
    _switchToRepoLabel->setVisible( false );

    _cancelSwitchingToRepoLabel = new QLabel( _notifications );
    _cancelSwitchingToRepoLabel->setTextFormat( Qt::RichText );
    _cancelSwitchingToRepoLabel->setWordWrap( true );
    _cancelSwitchingToRepoLabel->setVisible( false );

    notificationsLayout->addWidget( _switchToRepoLabel   );
    notificationsLayout->addWidget( _cancelSwitchingToRepoLabel );


    // If the user clicks on a link on the label, we have to check
    // which repository upgrade job to add or remove, for that
    // we will encode the links as repoupgradeadd://alias and
    // repoupgraderemove:://alias

    connect( _switchToRepoLabel,          SIGNAL( linkActivated( QString ) ),
             this,                        SLOT  ( switchToRepo ( QString ) ) );

    connect( _cancelSwitchingToRepoLabel, SIGNAL( linkActivated( QString ) ),
             this,                        SLOT  ( switchToRepo ( QString ) ) );

    updateSwitchRepoLabels();
}


void
YQPkgSelector::layoutDetailsViews( QWidget * parent )
{
    bool haveInstalledPkgs = YQPkgList::haveInstalledPkgs();

    QWidget * detailsPane = new QWidget( parent );
    CHECK_NEW( detailsPane );

    QVBoxLayout * detailsVBox = new QVBoxLayout( detailsPane );
    CHECK_NEW( detailsVBox );
    detailsVBox->setContentsMargins( 0,   // left
                                     4,   // top
                                     0,   // right
                                     0 ); // bottom

    _detailsViews = new QTabWidget( detailsPane );
    CHECK_NEW( _detailsViews );
    detailsVBox->addWidget( _detailsViews );

    //
    // Description
    //

    _pkgDescriptionView = new YQPkgDescriptionView( _detailsViews, confirmUnsupported() );
    CHECK_NEW( _pkgDescriptionView );

    _detailsViews->addTab( _pkgDescriptionView, _( "D&escription" ) );
    _detailsViews->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) ); // hor/vert

    connect( _pkgList,                  SIGNAL( currentItemChanged  ( ZyppSel ) ),
             _pkgDescriptionView,       SLOT  ( showDetailsIfVisible( ZyppSel ) ) );

    //
    // Technical details
    //

    _pkgTechnicalDetailsView = new YQPkgTechnicalDetailsView( _detailsViews );
    CHECK_NEW( _pkgTechnicalDetailsView );

    _detailsViews->addTab( _pkgTechnicalDetailsView, _( "&Technical Data" ) );

    connect( _pkgList,                 SIGNAL( currentItemChanged  ( ZyppSel ) ),
             _pkgTechnicalDetailsView, SLOT  ( showDetailsIfVisible( ZyppSel ) ) );


    //
    // Dependencies
    //

    _pkgDependenciesView = new YQPkgDependenciesView( _detailsViews );
    CHECK_NEW( _pkgDependenciesView );

    _detailsViews->addTab( _pkgDependenciesView, _( "Dependencies" ) );
    _detailsViews->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) ); // hor/vert

    connect( _pkgList,             SIGNAL( currentItemChanged  ( ZyppSel ) ),
             _pkgDependenciesView, SLOT  ( showDetailsIfVisible( ZyppSel ) ) );



    //
    // Versions
    //

    _pkgVersionsView = new YQPkgVersionsView( _detailsViews );
    CHECK_NEW( _pkgVersionsView );

    _detailsViews->addTab( _pkgVersionsView, _( "&Versions" ) );

    connect( _pkgList,          SIGNAL( currentItemChanged  ( ZyppSel ) ),
             _pkgVersionsView,  SLOT  ( showDetailsIfVisible( ZyppSel ) ) );

    connect( _pkgList,          SIGNAL( statusChanged() ),
             _pkgVersionsView,  SIGNAL( statusChanged() ) );


    //
    // File List
    //

    if ( haveInstalledPkgs )    // file list information is only available for installed pkgs
    {
        _pkgFileListView = new YQPkgFileListView( _detailsViews );
        CHECK_NEW( _pkgFileListView );

        _detailsViews->addTab( _pkgFileListView, _( "File List" ) );
        _detailsViews->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) ); // hor/vert

        connect( _pkgList,         SIGNAL( currentItemChanged  ( ZyppSel ) ),
                 _pkgFileListView, SLOT  ( showDetailsIfVisible( ZyppSel ) ) );
    }


    //
    // Change Log
    //

    if ( haveInstalledPkgs )    // change log information is only available for installed pkgs
    {
        _pkgChangeLogView = new YQPkgChangeLogView( _detailsViews );
        CHECK_NEW( _pkgChangeLogView );

        _detailsViews->addTab( _pkgChangeLogView, _( "Change Log" ) );
        _detailsViews->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) ); // hor/vert

        connect( _pkgList,          SIGNAL( currentItemChanged  ( ZyppSel ) ),
                 _pkgChangeLogView, SLOT  ( showDetailsIfVisible( ZyppSel ) ) );
    }
}


void
YQPkgSelector::layoutButtons( QWidget *parent )
{
    QWidget * button_box = new QWidget( parent );
    CHECK_NEW( button_box );
    parent->layout()->addWidget( button_box );

    QHBoxLayout * layout = new QHBoxLayout( button_box );
    CHECK_NEW( layout );

    button_box->setLayout( layout );
    layout->setContentsMargins( 2, 2, 2, 2 ); // left / right / top / bottom
    layout->addStretch();

    QPushButton * cancel_button = new QPushButton( _( "&Cancel" ), button_box );
    CHECK_NEW( cancel_button );
    layout->addWidget(cancel_button);

    cancel_button->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) ); // hor/vert

    connect( cancel_button, SIGNAL( clicked() ),
             this,          SLOT  ( reject()   ) );


    QPushButton * accept_button = new QPushButton( _( "&Accept" ), button_box );
    CHECK_NEW( accept_button );
    layout->addWidget(accept_button);
    accept_button->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) ); // hor/vert
    accept_button->setEnabled( ! MyrlynApp::readOnlyMode() );

    connect( accept_button, SIGNAL( clicked() ),
             this,          SLOT  ( accept()   ) );

    button_box->setFixedHeight( button_box->sizeHint().height() );
}


void
YQPkgSelector::layoutMenuBar( QWidget *parent )
{
    _menuBar = new QMenuBar( parent );
    CHECK_NEW( _menuBar );
    parent->layout()->addWidget(_menuBar);

    _fileMenu           = 0;
    _optionsMenu        = 0;
    _pkgMenu            = 0;
    _patchMenu          = 0;
    _extrasMenu         = 0;
    _configMenu         = 0;
    _dependencyMenu     = 0;
    _helpMenu           = 0;
}


void
YQPkgSelector::addMenus()
{
    //
    // File menu
    //

    _fileMenu = new QMenu( _menuBar );
    CHECK_NEW( _fileMenu );
    QAction * action = _menuBar->addMenu( _fileMenu );
    action->setText( _( "&File" ));
    _fileMenu->addSeparator();

    _fileMenu->addAction(          _( "E&xit -- Discard Changes" ), this, SLOT( reject() ) );
    action = _fileMenu->addAction( _( "&Quit -- Save Changes"    ), this, SLOT( accept() ) );
    action->setEnabled( ! MyrlynApp::readOnlyMode() );

    if ( _pkgList )
    {
        //
        // Package menu
        //

        _pkgMenu = new QMenu( _menuBar );
        CHECK_NEW( _pkgMenu );
        action = _menuBar->addMenu( _pkgMenu );
        action->setText(_( "&Package" ));

        _pkgMenu->addAction(_pkgList->actionSetCurrentInstall);
        _pkgMenu->addAction(_pkgList->actionSetCurrentDontInstall);
        _pkgMenu->addAction(_pkgList->actionSetCurrentKeepInstalled);
        _pkgMenu->addAction(_pkgList->actionSetCurrentDelete);
        _pkgMenu->addAction(_pkgList->actionSetCurrentUpdate);
        _pkgMenu->addAction(_pkgList->actionSetCurrentUpdateForce);
        _pkgMenu->addAction(_pkgList->actionSetCurrentTaboo);

        _pkgMenu->addSeparator();
        QMenu * submenu = _pkgList->addAllInListSubMenu( _pkgMenu );
        CHECK_NEW( submenu );

        //
        // Submenu for all packages
        //

        submenu = new QMenu( _pkgMenu );
        CHECK_NEW( submenu );

        // Translators: Unlike the "all in this list" submenu, this submenu
        // refers to all packages globally, not only to those that are
        // currently visible in the packages list.
        action = _pkgMenu->addMenu( submenu );
        action->setText(_( "All Packages" ));

        submenu->addAction( _( "Update if newer version available" ),
                            this, SLOT( globalUpdatePkg() ) );

        submenu->addAction( _( "Update unconditionally" ),
                            this, SLOT( globalUpdatePkgForce() ) );
    }


    if ( _patchList )
    {
        //
        // Patch menu
        //

        _patchMenu = new QMenu( _menuBar );
        CHECK_NEW( _patchMenu );
        action = _menuBar->addMenu( _patchMenu );
        action->setText(_( "&Patch" ));

        _patchMenu->addAction(_patchList->actionSetCurrentInstall);
        _patchMenu->addAction(_patchList->actionSetCurrentDontInstall);
        _patchMenu->addAction(_patchList->actionSetCurrentKeepInstalled);

#if ENABLE_DELETING_PATCHES
        _patchMenu->addAction(_patchList->actionSetCurrentDelete);
#endif
        _patchMenu->addAction(_patchList->actionSetCurrentUpdate);
        _patchMenu->addAction(_patchList->actionSetCurrentUpdateForce);
        _patchMenu->addAction(_patchList->actionSetCurrentTaboo);

        _patchMenu->addSeparator();
        _patchList->addAllInListSubMenu( _patchMenu );
    }


#ifdef FIXME_REPO_MGMT
    //
    // Configuration menu
    //

    if ( repoMgrEnabled() )
    {
        _configMenu = new QMenu( _menuBar );
        CHECK_NEW( _configMenu );
        action = _menuBar->addMenu( _configMenu );
        action->setText(_( "Confi&guration" ));
        _configMenu->addAction( _( "&Repositories..."  ), this, SLOT( repoManager() ), Qt::CTRL + Qt::Key_R );
        _configMenu->addAction( _( "&Online Update..." ), this, SLOT( onlineUpdateConfiguration() ), Qt::CTRL + Qt::Key_O );
    }
#endif


    //
    // Dependency menu
    //

    _dependencyMenu = new QMenu( _menuBar );
    CHECK_NEW( _dependencyMenu );
    action = _menuBar->addMenu( _dependencyMenu );
    action->setText(_( "&Dependencies" ));

    _dependencyMenu->addAction( _( "&Check Now" ), this, SLOT( manualResolvePackageDependencies() ) );

    _autoDependenciesAction = new QAction( _( "&Autocheck" ), this );
    _autoDependenciesAction->setCheckable( true );
    _dependencyMenu->addAction( _autoDependenciesAction );

    _installRecommendedAction = _dependencyMenu->addAction(
                                                           _("Install &Recommended Packages"),
                                                           this, SLOT (pkgInstallRecommendedChanged(bool)));
    _installRecommendedAction->setCheckable( true );


    //
    // View menu
    //

    _optionsMenu = new QMenu( _menuBar );
    CHECK_NEW( _optionsMenu );
    action = _menuBar->addMenu( _optionsMenu );
    // Translators: Menu for view options (Use a noun, not a verb!)
    action->setText(_( "&Options" ));

    // Translators: This is about packages ending in "-devel", so don't translate that "-devel"!
    _showDevelAction = _optionsMenu->addAction( _( "Show -de&vel Packages" ),
                                                this, SLOT( pkgExcludeDevelChanged( bool ) ), Qt::Key_F7 );
    _showDevelAction->setCheckable( true );

    _excludeDevelPkgs = new YQPkgObjList::ExcludeRule( _pkgList, QRegularExpression( ".*(\\d+bit)?-devel(-\\d+bit)?$" ), _pkgList->nameCol() );
    CHECK_NEW( _excludeDevelPkgs );
    _excludeDevelPkgs->enable( false );

    // Translators: This is about packages ending in "-debuginfo", so don't translate that "-debuginfo"!
    _showDebugAction = _optionsMenu->addAction( _( "Show -&debuginfo/-debugsource Packages" ),
                                                this, SLOT( pkgExcludeDebugChanged( bool ) ), Qt::Key_F8 );
    _showDebugAction->setCheckable(true);
    _excludeDebugInfoPkgs = new YQPkgObjList::ExcludeRule( _pkgList, QRegularExpression( ".*(-\\d+bit)?-(debuginfo|debugsource)(-32bit)?$" ), _pkgList->nameCol() );
    CHECK_NEW( _excludeDebugInfoPkgs );
    _excludeDebugInfoPkgs->enable( false );


#if 0
    _verifySystemModeAction = _optionsMenu->addAction( _( "&System Verification Mode" ),
                                                       this, SLOT( pkgVerifySytemModeChanged( bool ) ) );
    _verifySystemModeAction->setCheckable( true );
#endif

    // Widget styles can use the text information in the rendering for sections,
    // or can choose to ignore it and render sections like simple separators.
    _optionsMenu->addSection( _( "Options for this run only..." ) );

    _cleanDepsOnRemoveAction = _optionsMenu->addAction( _( "&Cleanup when deleting packages" ),
                                                        this, SLOT( pkgCleanDepsOnRemoveChanged( bool ) ) );
    _cleanDepsOnRemoveAction->setCheckable( true );

    _allowVendorChangeAction = _optionsMenu->addAction( _( "&Allow vendor change" ),
                                                        this, SLOT( pkgAllowVendorChangeChanged( bool ) ) );
    _allowVendorChangeAction->setCheckable( true );


    //
    // Extras menu
    //

    _extrasMenu = new QMenu( _menuBar );
    CHECK_NEW( _extrasMenu );
    action = _menuBar->addMenu( _extrasMenu );
    action->setText(_( "E&xtras" ));

    _extrasMenu->addAction( _( "Show &Products"         ), this, SLOT( showProducts() ) );
    _extrasMenu->addAction( _( "Show P&ackage Changes"  ), this, SLOT( showAutoPkgList() ), Qt::CTRL + Qt::Key_A );
    _extrasMenu->addAction( _( "Show &History"          ), this, SLOT( showHistory() ) );

    _extrasMenu->addSeparator();

    // Translators: This is about packages ending in "-devel", so don't translate that "-devel"!
    _extrasMenu->addAction( _( "Install All Matching -&devel Packages" ), this, SLOT( installDevelPkgs() ) );

    // Translators: This is about packages ending in "-debuginfo", so don't translate that "-debuginfo"!
    _extrasMenu->addAction( _( "Install All Matching -de&buginfo Packages" ), this, SLOT( installDebugInfoPkgs() ) );

    // Translators: This is about packages ending in "-debugsource", so don't translate that "-debugsource"!
    _extrasMenu->addAction( _( "Install All Matching -debug&source Packages" ), this, SLOT( installDebugSourcePkgs() ) );

    _extrasMenu->addAction( _( "Install All Matching &Recommended Packages" ),
                            this, SLOT( installRecommendedPkgs() ) );

    _extrasMenu->addSeparator();

    if ( _pkgConflictDialog )
    {
        QAction * action = _extrasMenu->addAction( _( "Generate Dependency Resolver &Test Case" ),
                                                   _pkgConflictDialog, SLOT( askCreateSolverTestCase() ) );
        action->setEnabled( MyrlynApp::runningAsRoot() );
    }

    if ( _actionResetIgnoredDependencyProblems )
        _extrasMenu->addAction(_actionResetIgnoredDependencyProblems);


    //
    // Help menu
    //

    _helpMenu = new QMenu( _menuBar );
    CHECK_NEW( _helpMenu );
    _menuBar->addSeparator();
    action = _menuBar->addMenu( _helpMenu );
    action->setText(_( "&Help" ));

    // Note: The help functions and their texts are moved out
    // to a separate source file YQPkgSelHelp.cc

    // Menu entry for help overview
    _helpMenu->addAction( _( "&Overview" ), this, SLOT( help()      ), Qt::Key_F1 );

    // Menu entry for help about used symbols ( icons )
    _helpMenu->addAction( _( "&Symbols" ), this, SLOT( symbolHelp() ), Qt::SHIFT + Qt::Key_F1 );

    // Menu entry for keyboard help
    _helpMenu->addAction( _( "&Keys" ), this, SLOT( keyboardHelp() ) );


    //
    // Program name and version in the top right corner
    //

    QString txt( "Myrlyn-" VERSION );
    _menuBar->setCornerWidget( new QLabel( txt ) );
}


void
YQPkgSelector::connectFilter( QWidget * filter,
                              QWidget * pkgList,
                              bool      hasUpdateSignal )
{
    if ( ! filter  )    return;
    if ( ! pkgList )    return;

    if ( _filters )
    {
        connect( _filters, SIGNAL( currentChanged( QWidget * ) ),
                 filter,   SLOT  ( showFilter    ( QWidget * ) ) );
    }

    connect( filter,    SIGNAL( filterStart()   ),
             pkgList,   SLOT  ( clear()         ) );

    connect( filter,    SIGNAL( filterStart()   ),
             this,      SLOT  ( busyCursor()            ) );

    connect( filter,    SIGNAL( filterMatch( ZyppSel, ZyppPkg ) ),
             pkgList,   SLOT  ( addPkgItem ( ZyppSel, ZyppPkg ) ) );

    connect( filter,    SIGNAL( filterFinished()       ),
             pkgList,   SLOT  ( resort() ) );

    connect( filter,    SIGNAL( filterFinished()  ),
             pkgList,   SLOT  ( selectSomething() ) );

    connect( filter,    SIGNAL( filterFinished()       ),
             pkgList,   SLOT  ( maybeSetFocus() ) );

    connect( filter,    SIGNAL( filterFinished()       ),
             this,      SLOT  ( normalCursor() ) );


    if ( hasUpdateSignal && _filters->diskUsageList() )
    {
        connect( filter,  SIGNAL( updatePackages()   ),
                 pkgList, SLOT  ( updateItemStates() ) );

        if ( _filters->diskUsageList() )
        {
            connect( filter,                    SIGNAL( updatePackages()  ),
                     _filters->diskUsageList(), SLOT  ( updateDiskUsage() ) );
        }
    }
}


void
YQPkgSelector::makeConnections()
{
#if USE_UPDATE_PROBLEM_FILTER_VIEW
    connectFilter( _updateProblemFilterView,     _pkgList, false );
#endif
    connectFilter( _searchFilterView,            _pkgList, false );
    connectFilter( _updatesFilterView,           _pkgList, false );
    connectFilter( _repoFilterView,              _pkgList, false );
    connectFilter( _serviceFilterView,           _pkgList, false );
    connectFilter( _patternList,                 _pkgList );
    connectFilter( _pkgClassificationFilterView, _pkgList, false );
    connectFilter( _langList,                    _pkgList );
    connectFilter( _statusFilterView,            _pkgList, false );

    if ( _searchFilterView && _pkgList )
    {
        connect( _searchFilterView,     SIGNAL( message( const QString & ) ),
                 _pkgList,              SLOT  ( message( const QString & ) ) );
    }

    if ( _repoFilterView && _pkgList )
    {
        connect( _repoFilterView,       SIGNAL( filterNearMatch  ( ZyppSel, ZyppPkg ) ),
                 _pkgList,              SLOT  ( addPkgItemDimmed ( ZyppSel, ZyppPkg ) ) );

    }

    if ( _serviceFilterView && _pkgList )
    {
        connect( _serviceFilterView,    SIGNAL( filterNearMatch  ( ZyppSel, ZyppPkg ) ),
                 _pkgList,              SLOT  ( addPkgItemDimmed ( ZyppSel, ZyppPkg ) ) );
    }


    if ( _langList )
    {

        connect( _langList, SIGNAL( statusChanged()           ),
                 this,      SLOT  ( autoResolveDependencies() ) );
    }

    if ( _pkgList && _filters->diskUsageList() )
    {

        connect( _pkgList,                      SIGNAL( statusChanged()   ),
                 _filters->diskUsageList(),     SLOT  ( updateDiskUsage() ) );
    }

    connectPatchList();
    connectPatternList();


    // Hide and show the upgrade label when tabs change, or when the user
    // selects repositories

    connect( _filters,        SIGNAL( currentChanged( QWidget * ) ),
             this,            SLOT  ( updateSwitchRepoLabels()    ) );

    if ( _repoFilterView )
    {
        connect( _repoFilterView, SIGNAL( filterStart()           ),
                 this,            SLOT  ( updateSwitchRepoLabels() ) );
    }


    //
    // Connect package conflict dialog
    //

    if ( _pkgConflictDialog )
    {
        if (_pkgList )
        {
            connect( _pkgConflictDialog,        SIGNAL( updatePackages()   ),
                     _pkgList,                  SLOT  ( updateItemStates() ) );
        }

        if ( _patternList )
        {
            connect( _pkgConflictDialog,        SIGNAL( updatePackages()   ),
                     _patternList,              SLOT  ( updateItemStates() ) );
        }


        if ( _filters->diskUsageList() )
        {
            connect( _pkgConflictDialog,        SIGNAL( updatePackages()   ),
                     _filters->diskUsageList(), SLOT  ( updateDiskUsage()  ) );
        }
    }


    //
    // Connect package versions view
    //

    if ( _pkgVersionsView && _pkgList )
    {
        connect( _pkgVersionsView,      SIGNAL( candidateChanged( ZyppObj ) ),
                 _pkgList,              SLOT  ( updateItemData()    ) );

        connect( _pkgVersionsView,      SIGNAL( statusChanged()  ),
                 _pkgList,              SLOT  ( updateItemData() ) );
    }


    //
    // Hotkey to enable "patches" filter view on the fly
    //

    QShortcut * accel = new QShortcut( Qt::Key_F2, this, SLOT( hotkeyInsertPatchFilterView() ) );
    CHECK_NEW( accel );


    //
    // Update actions just before opening menus
    //

    if ( _pkgMenu && _pkgList )
    {
        connect( _pkgMenu,      SIGNAL( aboutToShow()   ),
                 _pkgList,      SLOT  ( updateActions() ) );
    }

    if ( _patchMenu && _patchList )
    {
        connect( _patchMenu,    SIGNAL( aboutToShow()   ),
                 _patchList,    SLOT  ( updateActions() ) );
    }
}


void
YQPkgSelector::autoResolveDependencies()
{
    if ( _autoDependenciesAction && ! _autoDependenciesAction->isChecked() )
        return;

    resolveDependencies();
}


int
YQPkgSelector::manualResolvePackageDependencies()
{
    if ( ! _pkgConflictDialog )
    {
        logError() << "No package conflict dialog existing" << Qt::endl;
        return QDialog::Accepted;
    }

    busyCursor();
    int result = _pkgConflictDialog->solveAndShowConflicts();
    normalCursor();

#if DEPENDENCY_FEEDBACK_IF_OK

    if ( result == QDialog::Accepted )
    {
        QMessageBox::information( this, // parent
                                  "",   // window title
                                  _( "All package dependencies are OK." ) );
    }
#endif

    return result;
}


void
YQPkgSelector::addPatchFilterView()
{
    if ( ! _patchFilterView )
    {
        createPatchFilterView();
        _patchFilterView = new YQPkgPatchFilterView( this );
        CHECK_NEW( _patchFilterView );
        _filters->addPage( _( "P&atches" ), _patchFilterView, "patches" );

        _patchList = _patchFilterView->patchList();
        CHECK_PTR( _patchList );
    }
}


void
YQPkgSelector::hotkeyInsertPatchFilterView()
{
    if ( ! _patchFilterView )
    {
        logInfo() << "Activating patches filter view" << Qt::endl;

        addPatchFilterView();
        connectPatchList();

        _filters->showPage( _patchFilterView );
        _pkgList->clear();
        _patchList->filter();
    }
    else
    {
        _filters->showPage( _patchFilterView );
    }
}


void
YQPkgSelector::connectPatchList()
{
    if ( _pkgList && _patchList )
    {
        connectFilter( _patchList, _pkgList );

        connect( _patchList, SIGNAL( filterMatch   ( const QString &, const QString &, FSize ) ),
                 _pkgList,   SLOT  ( addPassiveItem( const QString &, const QString &, FSize ) ) );

        connect( _patchList,            SIGNAL( statusChanged()           ),
                 this,                  SLOT  ( autoResolveDependencies() ) );

        if ( _pkgConflictDialog )
        {
            connect( _pkgConflictDialog,SIGNAL( updatePackages()   ),
                     _patchList,        SLOT  ( updateItemStates() ) );
        }
    }
}


void
YQPkgSelector::connectPatternList()
{
    if ( ! _patternList )
        return;

    connect( _patternList, SIGNAL( statusChanged()           ),
             this,         SLOT  ( autoResolveDependencies() ) );

    if ( _pkgConflictDialog )
    {
        connect( _pkgConflictDialog, SIGNAL( updatePackages()   ),
                 _patternList,       SLOT  ( updateItemStates() ) );
    }
}


void
YQPkgSelector::globalUpdatePkg( bool force )
{
    if ( ! _pkgList )
        return;

    int count = _pkgList->globalSetPkgStatus( S_Update, force,
                                              true ); // countOnly
    logInfo() << count << " pkgs found for update" << Qt::endl;

    if ( count >= GLOBAL_UPDATE_CONFIRMATION_THRESHOLD )
    {
        if ( QMessageBox::question( this, "",   // caption
                                    // Translators: %1 is the number of affected packages
                                    _( "%1 packages will be updated" ).arg( count ),
                                    _( "&Continue" ), _( "C&ancel" ),
                                    0,          // defaultButtonNumber (from 0)
                                    1 )         // escapeButtonNumber
             == 1 )     // "Cancel"?
        {
            return;
        }
    }

    (void) _pkgList->globalSetPkgStatus( S_Update, force,
                                         false ); // countOnly

    if ( _statusFilterView )
    {
        _filters->showPage( _statusFilterView );
        _statusFilterView->writeSettings();
        _statusFilterView->clear();
        _statusFilterView->showTransactions();
    }
}


void
YQPkgSelector::updateSwitchRepoLabels()
{
    if ( ! _repoFilterView || ! _repoFilterView->isVisible() )
    {
        if ( _notifications )
            _notifications->hide();

        return;
    }

    _notifications->show();
    _switchToRepoLabel->setText("");
    _cancelSwitchingToRepoLabel->setText("");

    zypp::ResPool::repository_iterator it;

    for ( it  = zypp::getZYpp()->pool().knownRepositoriesBegin();
          it != zypp::getZYpp()->pool().knownRepositoriesEnd();
          ++it )
    {
        zypp::Repository repo(*it);

        // Add the option to upgrade to this repo packages if it is not the
        // system repository and there is no upgrade job in the solver for it
        // and the repo is the one selected right now

        if ( ! zypp::getZYpp()->resolver()->upgradingRepo( repo ) &&
             ! repo.isSystemRepo() &&
             _repoFilterView->selectedRepo() == repo )
        {
            QString html = _( "<p><a href=\"repoupgradeadd:///%1\">"
                              "Switch system packages</a> to the versions "
                              "in this repository (%2)</p>" )
                .arg( fromUTF8( repo.alias().c_str() ) )
                .arg( fromUTF8( repo.name().c_str()  ) );

            _switchToRepoLabel->setText( _switchToRepoLabel->text() + html );
        }
    }


    // We iterate twice to show first the repo upgrades that
    // can be cancelled, and then the repo that can be added

    for ( it  = zypp::getZYpp()->pool().knownRepositoriesBegin();
          it != zypp::getZYpp()->pool().knownRepositoriesEnd();
          ++it )
    {
        zypp::Repository repo( *it );

        // add the option to cancel the upgrade job against this repository
        // if there is a job for it

        if ( zypp::getZYpp()->resolver()->upgradingRepo(repo) )
        {
            QString html = _( "<p><a href=\"repoupgraderemove:///%1\">"
                              "Cancel switching</a> system packages to versions "
                              "in repository %2</p>" )
                .arg( fromUTF8( repo.alias().c_str() ) )
                .arg( fromUTF8( repo.name().c_str()  ) );

            _cancelSwitchingToRepoLabel->setText( _cancelSwitchingToRepoLabel->text() + html );
        }
    }

    _switchToRepoLabel->setVisible( ! _switchToRepoLabel->text().isEmpty() );
    _cancelSwitchingToRepoLabel->setVisible( ! _cancelSwitchingToRepoLabel->text().isEmpty() );

    _notifications->setVisible( _switchToRepoLabel->isVisible() ||
                                _cancelSwitchingToRepoLabel->isVisible() );
}


void
YQPkgSelector::switchToRepo( const QString & link )
{
    logDebug() << "link " << link << " clicked on label" << Qt::endl;

    QUrl url( link );
    if ( url.scheme() == "repoupgradeadd" )
    {
        logDebug() << "looking for repo " << url.path() << Qt::endl;

        std::string alias( url.path().remove( 0, 1 ).toStdString() );
        zypp::Repository repo( zypp::getZYpp()->pool().reposFind( alias ) );

        logDebug() << repo.name() << Qt::endl;

        if ( repo != zypp::Repository::noRepository )
        {
            zypp::getZYpp()->resolver()->addUpgradeRepo( repo );

            // Do not complain about vendor change when switching repos (bsc##1149391)
            zypp::getZYpp()->resolver()->dupSetAllowVendorChange( true );
        }
    }
    else if ( url.scheme() == "repoupgraderemove" )
    {
        std::string alias( url.path().remove( 0, 1 ).toStdString() );
        zypp::Repository repo( zypp::getZYpp()->pool().reposFind( alias ) );

        if (  repo != zypp::Repository::noRepository )
            zypp::getZYpp()->resolver()->removeUpgradeRepo( repo );
    }
    else
        logDebug() << "unknown link operation " << url.scheme() << Qt::endl;

    resolveDependencies();
}


void
YQPkgSelector::showProducts()
{
    YQPkgProductDialog::showProductDialog(this);
}


void
YQPkgSelector::showHistory()
{
    YQPkgHistoryDialog::showHistoryDialog(this);
}


void
YQPkgSelector::installDevelPkgs()
{
    installSubPkgs( "-devel" );
}


void
YQPkgSelector::installDebugInfoPkgs()
{
    installSubPkgs( "-debuginfo" );
}


void
YQPkgSelector::installDebugSourcePkgs()
{
    installSubPkgs( "-debugsource" );
}


void
YQPkgSelector::installRecommendedPkgs()
{
    zypp::getZYpp()->resolver()->setIgnoreAlreadyRecommended( false );
    resolveDependencies();

    if ( _filters && _statusFilterView )
    {
        _filters->showPage( _statusFilterView );
    }

    YQPkgChangesDialog::showChangesDialog( this,
                                           _( "Added Subpackages:" ),
                                           _( "&OK" ),
                                           QString(),                   // rejectButtonLabel
                                           YQPkgChangesDialog::FilterAutomatic,
                                           YQPkgChangesDialog::OptionNone );    // showIfEmpty
}


void
YQPkgSelector::pkgExcludeDebugChanged( bool on )
{
    if ( _optionsMenu && _pkgList )
    {
        if ( _excludeDebugInfoPkgs )
            _excludeDebugInfoPkgs->enable( ! on );

        _pkgList->applyExcludeRules();
    }
}


void
YQPkgSelector::pkgExcludeDevelChanged( bool on )
{
    if ( _optionsMenu && _pkgList )
    {
        if ( _excludeDevelPkgs )
            _excludeDevelPkgs->enable( ! on );

        _pkgList->applyExcludeRules();
    }
}


void
YQPkgSelector::pkgVerifySytemModeChanged( bool on )
{
    zypp::getZYpp()->resolver()->setSystemVerification( on );
}


void
YQPkgSelector::pkgInstallRecommendedChanged( bool on )
{
    zypp::getZYpp()->resolver()->setOnlyRequires( !on );
    resolveDependencies();
}


void
YQPkgSelector::pkgCleanDepsOnRemoveChanged( bool on )
{
    zypp::getZYpp()->resolver()->setCleandepsOnRemove( on );
    resolveDependencies();
}


void
YQPkgSelector::pkgAllowVendorChangeChanged( bool on )
{
    zypp::getZYpp()->resolver()->setAllowVendorChange( on );
    zypp::getZYpp()->resolver()->dupSetAllowVendorChange( on ); // bsc#1170521
    resolveDependencies();
}


void
YQPkgSelector::installSubPkgs( const QString & suffix )
{
    // Find all matching packages and put them into a QMap

    QMap<QString, ZyppSel> subPkgs;

    for ( ZyppPoolIterator it = zyppPkgBegin();
          it != zyppPkgEnd();
          ++it )
    {
        QString name = (*it)->name().c_str();

        if (  name.endsWith( suffix ) || name.endsWith( suffix + "-32bit" ) )
        {
            subPkgs[ name ] = *it;

            logDebug() << "Found subpackage: " << name << Qt::endl;
        }
    }


    // Now go through all packages and look if there is a corresponding subpackage in the QMap

    for ( ZyppPoolIterator it = zyppPkgBegin();
          it != zyppPkgEnd();
          ++it )
    {
        QString name = (*it)->name().c_str();

        if ( subPkgs.contains( name + suffix ) )
        {
            QString subPkgName( name + suffix );
            ZyppSel subPkg = subPkgs[ subPkgName ];

            switch ( (*it)->status() )
            {
                case S_AutoDel:
                case S_NoInst:
                case S_Protected:
                case S_Taboo:
                case S_Del:
                    // Don't install the subpackage
                    logInfo() << "Ignoring unwanted subpackage " << subPkgName << Qt::endl;
                    break;

                case S_AutoInstall:
                case S_Install:
                case S_KeepInstalled:

                    // Install the subpackage, but don't try to update it

                    if ( ! subPkg->installedObj() )
                    {
                        subPkg->setStatus( S_Install );
                        logInfo() << "Installing subpackage " << subPkgName << Qt::endl;
                    }
                    break;


                case S_Update:
                case S_AutoUpdate:

                    // Install or update the subpackage

                    if ( ! subPkg->installedObj() )
                    {
                        subPkg->setStatus( S_Install );
                        logInfo() << "Installing subpackage " << subPkgName << Qt::endl;
                    }
                    else
                    {
                        subPkg->setStatus( S_Update );
                        logInfo() << "Updating subpackage " << subPkgName << Qt::endl;
                    }
                    break;

                    // Intentionally omitting 'default' branch so the compiler can
                    // catch unhandled enum states
            }
        }
    }


    if ( _filters && _statusFilterView )
    {
        _filters->showPage( _statusFilterView );
    }

    YQPkgChangesDialog::showChangesDialog( this,
                                           _( "Added Subpackages:" ),
                                           QRegularExpression( ".*" + suffix + "$" ),
                                           _( "&OK" ),
                                           QString(),                   // rejectButtonLabel
                                           YQPkgChangesDialog::FilterAutomatic,
                                           YQPkgChangesDialog::OptionNone );    // showIfEmpty
}


bool
YQPkgSelector::anyRetractedPkgInstalled()
{
    logInfo() << "Checking for retracted installed packages..." << Qt::endl;

    for ( ZyppPoolIterator it = zyppPkgBegin(); it != zyppPkgEnd(); ++it )
    {
        if ( (*it)->hasRetractedInstalled() )
            return true;
    }

    logInfo() << "No retracted packages installed." << Qt::endl;

    return false;
}


void
YQPkgSelector::readSettings()
{
    QSettings settings;
    settings.beginGroup( "PackageSelector" );

    _showDevelAction->setChecked( settings.value( "showDevelPackages", false ).toBool());
    _showDebugAction->setChecked( settings.value( "showDebugPackages", false ).toBool());

    settings.endGroup();

    pkgExcludeDevelChanged( _showDevelAction->isChecked());
    pkgExcludeDebugChanged( _showDebugAction->isChecked());

    read_etc_sysconfig_yast();
}


void
YQPkgSelector::writeSettings()
{
    QSettings settings;
    settings.beginGroup( "PackageSelector" );

    settings.setValue( "showDevelPackages", _showDevelAction->isChecked() );
    settings.setValue( "showDebugPackages", _showDebugAction->isChecked() );

    settings.endGroup();

    write_etc_sysconfig_yast();
}



void
YQPkgSelector::read_etc_sysconfig_yast()
{
    // FIXME: It's not YaST anymore; it shouldn't rely on this file. Use the own settings instead.

    map<string, string> sysconfig = zypp::base::sysconfig::read( PATH_TO_YAST_SYSCONFIG );

    bool auto_check = AUTO_CHECK_DEPENDENCIES_DEFAULT;
    auto it = sysconfig.find( OPTION_AUTO_CHECK );

    if ( it != sysconfig.end() )
        auto_check = it->second == "yes";

    _autoDependenciesAction->setChecked(auto_check);

#if 0
    bool verify_system = zypp::getZYpp()->resolver()->systemVerification();
    it = sysconfig.find( OPTION_VERIFY );

    if ( it != sysconfig.end() )
        verify_system = it->second == "yes";

    _verifySystemModeAction->setChecked( verify_system );
    pkgVerifySytemModeChanged( verify_system );
#endif

    bool install_recommended = ! zypp::getZYpp()->resolver()->onlyRequires();
    it = sysconfig.find( OPTION_RECOMMENDED );

    if (it != sysconfig.end())
        install_recommended = it->second == "yes";

    _installRecommendedAction->setChecked( install_recommended );
    pkgInstallRecommendedChanged(install_recommended);

    bool allow_vendor_change = zypp::getZYpp()->resolver()->allowVendorChange();
    _allowVendorChangeAction->setChecked( allow_vendor_change );
    pkgAllowVendorChangeChanged( allow_vendor_change );

    bool clean_deps_on_remove = zypp::getZYpp()->resolver()->cleandepsOnRemove();
    _cleanDepsOnRemoveAction->setChecked( clean_deps_on_remove );
    pkgCleanDepsOnRemoveChanged( clean_deps_on_remove );
}


void
YQPkgSelector::write_etc_sysconfig_yast()
{
    // FIXME: It's not YaST anymore; it shouldn't write this file at all.
    // Write to the own settings instead.

    if ( ! geteuid() == 0 )
        return;

    try
    {
        zypp::base::sysconfig::writeStringVal( PATH_TO_YAST_SYSCONFIG,
                                               OPTION_AUTO_CHECK,
                                               ( _autoDependenciesAction->isChecked() ? "yes" : "no" ),
                                               "Automatic dependency checking");
#if 0
        zypp::base::sysconfig::writeStringVal( PATH_TO_YAST_SYSCONFIG,
                                               OPTION_VERIFY,
                                               ( _verifySystemModeAction->isChecked() ? "yes" : "no" ),
                                               "System verification mode");
#endif

        zypp::base::sysconfig::writeStringVal( PATH_TO_YAST_SYSCONFIG,
                                               OPTION_RECOMMENDED,
                                               ( _installRecommendedAction->isChecked() ? "yes" : "no" ),
                                               "Install recommended packages");
    }
    catch( const std::exception &e )
    {
        logError() << "Writing " << PATH_TO_YAST_SYSCONFIG << " failed" << Qt::endl;
    }
}

void YQPkgSelector::busyCursor()
{
    ::busyCursor();
}


void YQPkgSelector::normalCursor()
{
    ::normalCursor();
}
