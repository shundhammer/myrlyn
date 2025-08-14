/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

 */


#include <algorithm>

#include <zypp-core/parser/sysconfig.h>
#include <zypp/PoolItem.h>
#include <zypp/Repository.h>
#include <zypp/ResPool.h>
#include <zypp/Resolver.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/ui/Selectable.h>
#include <zypp/ui/Status.h>


#include <QAction>
#include <QApplication>
#include <QDialog>
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
#include "LicenseCache.h"
#include "Logger.h"
#include "QY2CursorHelper.h"
#include "MyrlynApp.h"
#include "RepoConfigDialog.h"
#include "YQPkgChangeLogView.h"
#include "YQPkgChangesDialog.h"
#include "YQPkgClassificationFilterView.h"
#include "YQPkgConflictDialog.h"
#include "YQPkgDependenciesView.h"
#include "YQPkgDescriptionView.h"
#include "YQPkgDiskUsageList.h"
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
#include "YQPkgRpmGroupsFilterView.h"
#include "YQPkgSearchFilterView.h"
#include "YQPkgServiceFilterView.h"
#include "YQPkgStatusFilterView.h"
#include "YQPkgTechnicalDetailsView.h"
#include "YQPkgUpdatesFilterView.h"
#include "YQPkgVersionsView.h"
#include "YQZypp.h"
#include "YQi18n.h"
#include "utf8.h"

#include "YQPkgSelector.h"

#define CHECK_DEPENDENCIES_ON_STARTUP          0
#define FORCE_SHOW_NEEDED_PATCHES              0
#define ENABLE_PATCH_MENU                      0
#define ENABLE_VERIFY_SYSTEM_MODE_ACTION       0
#define DEPENDENCY_FEEDBACK_IF_OK              1
#define GLOBAL_UPDATE_CONFIRMATION_THRESHOLD  20

// Users can override this in their ~/.config/openSUSE/Myrlyn.conf
// (/root/.config/openSUSE/Myrlyn.conf for root, of course)
#ifndef USE_RPM_GROUPS
#  define USE_RPM_GROUPS  1
#endif

using std::max;
using std::string;
using std::map;


YQPkgSelector * YQPkgSelector::_instance = 0;


YQPkgSelector::YQPkgSelector( QWidget * parent )
    : YQPkgSelectorBase( parent )
    , _pkgList(0)
    , _filters(0)
    , _searchFilterView(0)
    , _patchFilterView(0)
    , _updatesFilterView(0)
    , _repoFilterView(0)
    , _rpmGroupsFilterView(0)
    , _serviceFilterView(0)
    , _pkgClassificationFilterView(0)
    , _patternList(0)
    , _statusFilterView(0)
    , _langList(0)
    , _pkgVersionsView(0)
    , _notificationsArea(0)
    , _switchToRepoLabel(0)
    , _cancelSwitchingToRepoLabel(0)
    , _menuBar(0)
    , _pkgMenu(0)
    , _patchMenu(0)
    , _autoDependenciesAction(0)
    , _showDevelAction(0)
    , _showDebugAction(0)
    , _verifySystemModeAction(0)
    , _installRecommendedAction(0)
    , _cleanDepsOnRemoveAction(0)
    , _allowVendorChangeAction(0)
    , _excludeDevelPkgs(0)
    , _excludeDebugInfoPkgs(0)
    , _useRpmGroups( USE_RPM_GROUPS )
{
    _instance = this;

    // Inherited from YQPkgSelectorBase
    _blockResolver = true;

    logDebug() << "Creating YQPkgSelector..." << endl;

    readSettingsEarly();
    basicLayout();
    addMenus();         // Only after all widgets are created!
    readSettings();     // Only after menus are created!
    makeConnections();
    _filters->readSettings();

    if ( _filters->tabCount() == 0 )
    {
        logDebug() << "No page configuration saved, using fallbacks" << endl;
        showFallbackPages();
    }

    overrideInitialPage(); // Only for very important special cases!

    if ( _filters->diskUsageList() )
        _filters->diskUsageList()->updateDiskUsage();

    _blockResolver = false;
    firstSolverRun();

    logDebug() << "YQPkgSelector init done" << endl;
}


YQPkgSelector::~YQPkgSelector()
{
    logDebug() << "Destroying YQPkgSelector..." << endl;

    writeSettings();
    _instance = 0;

    logDebug() << "Destroying YQPkgSelector done." << endl;
}


void YQPkgSelector::showFallbackPages()
{
    {
        // New scope for the signal blocker:
        // Prevent a signal cascade for each page as it is shown.
        QSignalBlocker sigBlocker( _filters );

        if ( _searchFilterView  ) _filters->showPage( _searchFilterView  );
        if ( _patchFilterView   ) _filters->showPage( _patchFilterView   );
        if ( _updatesFilterView ) _filters->showPage( _updatesFilterView );
        if ( _repoFilterView    ) _filters->showPage( _repoFilterView    );
        if ( _serviceFilterView ) _filters->showPage( _serviceFilterView );
        if ( _patternList       ) _filters->showPage( _patternList       );
        if ( _statusFilterView  ) _filters->showPage( _statusFilterView  );
    }

    // Signals are no longer blocked; now trigger one for the first page

    _filters->showPage( 0 );
}


void YQPkgSelector::overrideInitialPage()
{
    // Override the initial page for very important special cases.
    //
    // Don't add any generic fallback here; that would kill the effect of
    // writing and reading the page configuration to and from the settings.

    if ( _pkgClassificationFilterView && anyRetractedPkgInstalled() )
    {
        // Exceptional case: If the system has any retracted package installed,
        // switch to that filter view and show those packages.  This should
        // happen only very, very rarely.

        logInfo() << "Found installed retracted packages; switching to that view" << endl;
        _filters->showPage( _pkgClassificationFilterView );
        _pkgClassificationFilterView->showPkgClass( YQPkgClassRetractedInstalled );

        // Also show a pop-up warning?
        //
        // No: This could become very annoying really quickly because you'll
        // get it with every start of the package selection as long as any
        // retracted package version is installed (which might be a deliberate
        // conscious decision by the user). It's also not easy to add a "Don't
        // show this again" checkbox in such a pop-up; which retracted packages
        // are installed might change between program runs, and we'd have to
        // inform the user when such a change occurs.
    }
#if FORCE_SHOW_NEEDED_PATCHES
    else if ( _patchFilterView && YQPkgPatchList::haveNeededPatches() )
    {
        _filters->showPage( _patchFilterView );
        _patchFilterView->patchList()->selectSomething();
    }
#endif
}


void YQPkgSelector::firstSolverRun()
{
#if CHECK_DEPENDENCIES_ON_STARTUP

    // Fire up the first dependency check in the main loop.
    // Don't do this right away - wait until all initializations are finished.

    if ( _pkgConflictDialog && ! MyrlynApp::isOptionSet( OptNoVerify ) )
        QTimer::singleShot( 0, _pkgConflictDialog, SLOT( verifySystemWithBusyPopup() ) );
#endif
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
    updatePageLabels();
    _filters->showPage( 0 );

    layoutRightPane( _filters->rightPane() );
}


// --- Filter views START ---


void YQPkgSelector::createFilterViews()
{
    // The order of creation is both the order in the "View" button's menu and
    // the order of tabs

    createSearchFilterView();         // Package search

    if ( _useRpmGroups )
        createRpmGroupsFilterView();  // RPM Groups

    createPatchFilterView();          // Patches - if patches available or F2
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

    _filters->addPage( _( "&Search" ), _searchFilterView,
                       "search", Qt::CTRL | Qt::SHIFT | Qt::Key_S );
}


void YQPkgSelector::createPatchFilterView( bool force )
{
    if ( YQPkgPatchList::haveAnyPatches() || force )
    {
        if ( ! _patchFilterView )
        {
            _patchFilterView = new YQPkgPatchFilterView( this );
            CHECK_NEW( _patchFilterView );

            _filters->addPage( _( "Patc&hes" ), _patchFilterView,
                               "patches", Qt::CTRL | Qt::SHIFT | Qt::Key_H  );
        }
    }
}


void YQPkgSelector::createUpdatesFilterView()
{
    _updatesFilterView = new YQPkgUpdatesFilterView( this );
    CHECK_NEW( _updatesFilterView );

    _filters->addPage( _( "&Updates" ), _updatesFilterView,
                       "updates", Qt::CTRL | Qt::SHIFT | Qt::Key_U );
}


void YQPkgSelector::createRepoFilterView()
{
    _repoFilterView = new YQPkgRepoFilterView( this );
    CHECK_NEW( _repoFilterView );

    _filters->addPage( _( "&Repositories" ), _repoFilterView,
                       "repos", Qt::CTRL | Qt::SHIFT | Qt::Key_R );
}


void YQPkgSelector::createRpmGroupsFilterView()
{
    _rpmGroupsFilterView = new YQPkgRpmGroupsFilterView( this );
    CHECK_NEW( _rpmGroupsFilterView );

    _filters->addPage( _( "RPM &Groups" ), _rpmGroupsFilterView,
                       "rpm-groups", Qt::CTRL | Qt::SHIFT | Qt::Key_G );
}


void YQPkgSelector::createServiceFilterView()
{
    if ( MyrlynApp::isOptionSet( OptForceServiceView ) ||
         YQPkgServiceFilterView::any_service() ) // Only if a service is present
    {
        _serviceFilterView = new YQPkgServiceFilterView( this );
        CHECK_NEW( _serviceFilterView );

        _filters->addPage( _( "Repository Index Ser&vices" ), _serviceFilterView,
                           "services", Qt::CTRL | Qt::SHIFT | Qt::Key_V );
    }
}


void YQPkgSelector::createPatternsFilterView()
{
    if ( ! zyppPool().empty<zypp::Pattern>() )
    {
        _patternList = new YQPkgPatternList( this );
        CHECK_NEW( _patternList );

        _filters->addPage( _( "Pa&tterns" ), _patternList,
                           "patterns", Qt::CTRL | Qt::SHIFT | Qt::Key_T );
    }
}


void YQPkgSelector::createPkgClassificationFilterView()
{
    _pkgClassificationFilterView = new YQPkgClassificationFilterView( this );
    CHECK_NEW( _pkgClassificationFilterView );

    _filters->addPage( _( "Package Classi&fication" ), _pkgClassificationFilterView,
                       "package_classification", Qt::CTRL | Qt::SHIFT | Qt::Key_F );
}


void YQPkgSelector::createLanguagesFilterView()
{
    _langList = new YQPkgLangList( this );
    CHECK_NEW( _langList );
    _langList->setSizePolicy( QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored ) ); // hor/vert

    _filters->addPage( _( "&Languages" ), _langList,
                       "languages", Qt::CTRL | Qt::SHIFT | Qt::Key_L );
}


void YQPkgSelector::createStatusFilterView()
{
    _statusFilterView = new YQPkgStatusFilterView( this );
    CHECK_NEW( _statusFilterView );

    _filters->addPage( _( "Installation Su&mmary" ), _statusFilterView,
                       "inst_summary", Qt::CTRL | Qt::SHIFT | Qt::Key_M );
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
    _notificationsArea->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) ); // hor/vert
    pkgListVBox->addWidget( _notificationsArea );


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

    _notificationsArea = new QWidget( parent );
    QVBoxLayout * notificationsLayout = new QVBoxLayout( _notificationsArea );
    notificationsLayout->setContentsMargins( 0, 5, 0, 5 ); // left / top / right / bottom

    _switchToRepoLabel = new QLabel( _notificationsArea );
    _switchToRepoLabel->setTextFormat( Qt::RichText );
    _switchToRepoLabel->setWordWrap( true );
    _switchToRepoLabel->setVisible( false );

    _cancelSwitchingToRepoLabel = new QLabel( _notificationsArea );
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

    QTabWidget * detailsViews = new QTabWidget( detailsPane );
    CHECK_NEW( detailsViews );

    detailsViews->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) ); // hor/vert
    detailsVBox->addWidget( detailsViews );


    //
    // Description
    //

    YQPkgDescriptionView * pkgDescriptionView = new YQPkgDescriptionView( detailsViews );
    CHECK_NEW( pkgDescriptionView );

    detailsViews->addTab( pkgDescriptionView, _( "Description" ) );

    connect( _pkgList,                  SIGNAL( currentItemChanged  ( ZyppSel ) ),
             pkgDescriptionView,        SLOT  ( showDetailsIfVisible( ZyppSel ) ) );

    //
    // Technical details
    //

    YQPkgTechnicalDetailsView * pkgTechnicalDetailsView = new YQPkgTechnicalDetailsView( detailsViews );
    CHECK_NEW( pkgTechnicalDetailsView );

    detailsViews->addTab( pkgTechnicalDetailsView, _( "Technical Data" ) );

    connect( _pkgList,                SIGNAL( currentItemChanged  ( ZyppSel ) ),
             pkgTechnicalDetailsView, SLOT  ( showDetailsIfVisible( ZyppSel ) ) );


    //
    // Dependencies
    //

    YQPkgDependenciesView * pkgDependenciesView = new YQPkgDependenciesView( detailsViews );
    CHECK_NEW( pkgDependenciesView );

    detailsViews->addTab( pkgDependenciesView, _( "Dependencies" ) );

    connect( _pkgList,            SIGNAL( currentItemChanged  ( ZyppSel ) ),
             pkgDependenciesView, SLOT  ( showDetailsIfVisible( ZyppSel ) ) );



    //
    // Versions
    //

    _pkgVersionsView = new YQPkgVersionsView( detailsViews );
    CHECK_NEW( _pkgVersionsView );

    detailsViews->addTab( _pkgVersionsView, _( "Versions" ) );

    connect( _pkgList,          SIGNAL( currentItemChanged  ( ZyppSel ) ),
             _pkgVersionsView,  SLOT  ( showDetailsIfVisible( ZyppSel ) ) );

    connect( _pkgList,          SIGNAL( statusChanged() ),
             _pkgVersionsView,  SIGNAL( statusChanged() ) );


    //
    // File List
    //

    if ( haveInstalledPkgs )    // file list information is only available for installed pkgs
    {
        YQPkgFileListView * pkgFileListView = new YQPkgFileListView( detailsViews );
        CHECK_NEW( pkgFileListView );

        detailsViews->addTab( pkgFileListView, _( "File List" ) );

        connect( _pkgList,        SIGNAL( currentItemChanged  ( ZyppSel ) ),
                 pkgFileListView, SLOT  ( showDetailsIfVisible( ZyppSel ) ) );
    }


    //
    // Change Log
    //

    if ( haveInstalledPkgs )    // change log information is only available for installed pkgs
    {
        YQPkgChangeLogView * pkgChangeLogView = new YQPkgChangeLogView( detailsViews );
        CHECK_NEW( pkgChangeLogView );

        detailsViews->addTab( pkgChangeLogView, _( "Change Log" ) );

        connect( _pkgList,         SIGNAL( currentItemChanged  ( ZyppSel ) ),
                 pkgChangeLogView, SLOT  ( showDetailsIfVisible( ZyppSel ) ) );
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

    _menuBar->setContentsMargins( 12, 20, 12, 20 ); // left / top / right / bottom
    parent->layout()->addWidget(_menuBar);

#if 0
    _pkgMenu        = 0;
    _patchMenu      = 0;
#endif
}


void
YQPkgSelector::addMenus()
{
    //
    // File menu
    //

    QMenu * fileMenu = new QMenu( _menuBar );
    CHECK_NEW( fileMenu );
    QAction * action = _menuBar->addMenu( fileMenu );
    action->setText( _( "&File" ));
    fileMenu->addSeparator();

    action = fileMenu->addAction( _( "&Accept Changes" ), Qt::CTRL | Qt::Key_A, this, SLOT( accept() ) );
    action->setEnabled( ! MyrlynApp::readOnlyMode() );
    fileMenu->addAction( _( "&Quit - Discard Changes" ), Qt::CTRL | Qt::Key_Q, this, SLOT( reject() ) );


    if ( _pkgList )
    {
        //
        // Package menu
        //

        _pkgMenu = new QMenu( _menuBar );
        CHECK_NEW( _pkgMenu );
        action = _menuBar->addMenu( _pkgMenu );
        action->setText(_( "&Package" ));

        _pkgMenu->addAction( _pkgList->actionSetCurrentInstall       );
        _pkgMenu->addAction( _pkgList->actionSetCurrentDontInstall   );
        _pkgMenu->addAction( _pkgList->actionSetCurrentKeepInstalled );
        _pkgMenu->addAction( _pkgList->actionSetCurrentDelete        );
        _pkgMenu->addAction( _pkgList->actionSetCurrentUpdate        );
        _pkgMenu->addAction( _pkgList->actionSetCurrentUpdateForce   );
        _pkgMenu->addAction( _pkgList->actionSetCurrentTaboo         );

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


#if ENABLE_PATCH_MENU

    if ( _patchFilterView )
    {
        //
        // Patch menu
        //

        YQPkgPatchList * patchList = _patchFilterView->patchList();
        CHECK_PTR( patchList );

        _patchMenu = new QMenu( _menuBar );
        CHECK_NEW( _patchMenu );
        action = _menuBar->addMenu( _patchMenu );
        action->setText(_( "Patch" ));

        _patchMenu->addAction( patchList->actionSetCurrentInstall       );
        _patchMenu->addAction( patchList->actionSetCurrentDontInstall   );
        _patchMenu->addAction( patchList->actionSetCurrentKeepInstalled );

#if ENABLE_DELETING_PATCHES
        _patchMenu->addAction( patchList->actionSetCurrentDelete );
#endif
        _patchMenu->addAction( patchList->actionSetCurrentUpdate      );
        _patchMenu->addAction( patchList->actionSetCurrentUpdateForce );
        _patchMenu->addAction( patchList->actionSetCurrentTaboo       );

        _patchMenu->addSeparator();
        patchList->addAllInListSubMenu( _patchMenu );
    }
#endif


    //
    // Dependency menu
    //

    QMenu * dependencyMenu = new QMenu( _menuBar );
    CHECK_NEW( dependencyMenu );
    action = _menuBar->addMenu( dependencyMenu );
    action->setText(_( "&Dependencies" ));

    dependencyMenu->addAction( _( "&Check Now" ), this, SLOT( manualResolvePackageDependencies() ) );

    _autoDependenciesAction = new QAction( _( "&Autocheck" ), this );
    _autoDependenciesAction->setCheckable( true );
    dependencyMenu->addAction( _autoDependenciesAction );

    _installRecommendedAction = dependencyMenu->addAction( _("Install &Recommended Packages"),
                                                           this, SLOT( pkgInstallRecommendedChanged( bool ) ) );
    _installRecommendedAction->setCheckable( true );


    //
    // View menu
    //

    QMenu * optionsMenu = new QMenu( _menuBar );
    CHECK_NEW( optionsMenu );
    action = _menuBar->addMenu( optionsMenu );
    // Translators: Menu for view options (Use a noun, not a verb!)
    action->setText(_( "&Options" ));

    // Translators: This is about packages ending in "-devel", so don't translate that "-devel"!
    _showDevelAction = optionsMenu->addAction( _( "Show -de&vel Packages" ),
                                                Qt::Key_F7,
                                                this, SLOT( pkgExcludeDevelChanged( bool ) ) );
    _showDevelAction->setCheckable( true );

    _excludeDevelPkgs = new YQPkgObjList::ExcludeRule( _pkgList, QRegularExpression( ".*(\\d+bit)?-devel(-\\d+bit)?$" ), _pkgList->nameCol() );
    CHECK_NEW( _excludeDevelPkgs );
    _excludeDevelPkgs->enable( false );

    // Translators: This is about packages ending in "-debuginfo", so don't translate that "-debuginfo"!
    _showDebugAction = optionsMenu->addAction( _( "Show -&debuginfo/-debugsource Packages" ),
                                                Qt::Key_F8,
                                                this, SLOT( pkgExcludeDebugChanged( bool ) ) );
    _showDebugAction->setCheckable(true);
    _excludeDebugInfoPkgs = new YQPkgObjList::ExcludeRule( _pkgList, QRegularExpression( ".*(-\\d+bit)?-(debuginfo|debugsource)(-32bit)?$" ), _pkgList->nameCol() );
    CHECK_NEW( _excludeDebugInfoPkgs );
    _excludeDebugInfoPkgs->enable( false );


#if ENABLE_VERIFY_SYSTEM_MODE_ACTION
    _verifySystemModeAction = optionsMenu->addAction( _( "&System Verification Mode" ),
                                                       this, SLOT( pkgVerifySytemModeChanged( bool ) ) );
    _verifySystemModeAction->setCheckable( true );
#endif

    // Widget styles can use the text information in the rendering for sections,
    // or can choose to ignore it and render sections like simple separators.
    optionsMenu->addSection( _( "Options for this run only" ) );

    _cleanDepsOnRemoveAction = optionsMenu->addAction( _( "&Cleanup when deleting packages" ),
                                                        this, SLOT( pkgCleanDepsOnRemoveChanged( bool ) ) );
    _cleanDepsOnRemoveAction->setCheckable( true );

    _allowVendorChangeAction = optionsMenu->addAction( _( "&Allow vendor change" ),
                                                        this, SLOT( pkgAllowVendorChangeChanged( bool ) ) );
    _allowVendorChangeAction->setCheckable( true );


    //
    // Extras menu
    //

    QMenu * extrasMenu = new QMenu( _menuBar );
    CHECK_NEW( extrasMenu );
    action = _menuBar->addMenu( extrasMenu );
    action->setText(_( "E&xtras" ));


#if 1
    extrasMenu->addAction( _( "C&onfigure Repositories..."  ),
                           Qt::CTRL | Qt::SHIFT | Qt::Key_O ,
                            this, SLOT( configRepos() ) );
    extrasMenu->addSeparator();
#endif

    extrasMenu->addAction( _( "Show &Products"         ), this, SLOT( showProducts()    ) );
    extrasMenu->addAction( _( "Show Package &Changes"  ), this, SLOT( showAutoPkgList() ) );
    extrasMenu->addAction( _( "Show &History"          ), this, SLOT( showHistory()     ) );

    extrasMenu->addSeparator();

    // Translators: This is about packages ending in "-devel", so don't translate that "-devel"!
    extrasMenu->addAction( _( "Install All Matching -devel Packages" ), this, SLOT( installDevelPkgs() ) );

    // Translators: This is about packages ending in "-debuginfo", so don't translate that "-debuginfo"!
    extrasMenu->addAction( _( "Install All Matching -debuginfo Packages" ), this, SLOT( installDebugInfoPkgs() ) );

    // Translators: This is about packages ending in "-debugsource", so don't translate that "-debugsource"!
    extrasMenu->addAction( _( "Install All Matching -debugsource Packages" ), this, SLOT( installDebugSourcePkgs() ) );

    extrasMenu->addAction( _( "Install All Matching Recommended Packages" ),
                            this, SLOT( installRecommendedPkgs() ) );

    if ( _pkgConflictDialog )
    {
        extrasMenu->addSeparator();
        extrasMenu->addAction( _( "Generate Dependency Resolver &Test Case" ),
                               _pkgConflictDialog, SLOT( askCreateSolverTestCase() ) );
    }


    //
    // Help menu
    //

    QMenu * helpMenu = new QMenu( _menuBar );
    CHECK_NEW( helpMenu );
    _menuBar->addSeparator();
    action = _menuBar->addMenu( helpMenu );
    action->setText(_( "H&elp" ));

    // Note: The help functions and their texts are moved out
    // to a separate source file YQPkgSelHelp.cc

    // Opening a hyperlink in the user's Internet browser is only supported
    // if not running via sudo or xdg-su
    bool canOpenInBrowser = ( geteuid() != 0 );

    if ( canOpenInBrowser )
        helpMenu->addAction( _( "&Overview" ), this, SLOT( help()    ) );

    helpMenu->addAction( _( "About &Myrlyn" ), this, SLOT( about()   ) );
    helpMenu->addAction( _( "About &Qt"     ), qApp, SLOT( aboutQt() ) );

    if ( canOpenInBrowser )
    {
        helpMenu->addSeparator();
        helpMenu->addAction( _( "Rep&ository Configuration" ), this, SLOT( helpRepoConfig() ) );
        helpMenu->addAction( _( "Root &Authentication"      ), this, SLOT( helpRootAuth()   ) );
    }



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
    connectFilter( _searchFilterView,            _pkgList, false );
    connectFilter( _updatesFilterView,           _pkgList, false );
    connectFilter( _repoFilterView,              _pkgList, false );
    connectFilter( _rpmGroupsFilterView,         _pkgList, false );
    connectFilter( _serviceFilterView,           _pkgList, false );
    connectFilter( _patternList,                 _pkgList );
    connectFilter( _pkgClassificationFilterView, _pkgList, false );
    connectFilter( _langList,                    _pkgList );
    connectFilter( _statusFilterView,            _pkgList, false );

    connectPatchFilterView();
    connectPatternList();

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



    // Hide and show the upgrade label when tabs change, or when the user
    // selects repositories

    connect( _filters, SIGNAL( currentChanged( QWidget * ) ),
             this,     SLOT  ( updateSwitchRepoLabels()    ) );

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

    QShortcut * accel = new QShortcut( Qt::Key_F2, this, SLOT( hotkeyAddPatchFilterView() ) );
    CHECK_NEW( accel );


    //
    // Update actions just before opening menus
    //

    if ( _pkgMenu && _pkgList )
    {
        connect( _pkgMenu,      SIGNAL( aboutToShow()   ),
                 _pkgList,      SLOT  ( updateActions() ) );
    }

    if ( _patchMenu && _patchFilterView )
    {
        connect( _patchMenu,                    SIGNAL( aboutToShow()   ),
                 _patchFilterView->patchList(), SLOT  ( updateActions() ) );
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
        logError() << "No package conflict dialog existing" << endl;
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
YQPkgSelector::hotkeyAddPatchFilterView()
{
    if ( ! _patchFilterView )
    {
        logInfo() << "Activating patches filter view" << endl;

        createPatchFilterView( true ); // force
        connectPatchFilterView();
    }

    _filters->showPage( _patchFilterView );
}


void
YQPkgSelector::connectPatchFilterView()
{
    if ( _pkgList && _patchFilterView )
    {
        YQPkgPatchList * patchList = _patchFilterView->patchList();
        CHECK_PTR( patchList );

        connectFilter( patchList, _pkgList );

        connect( patchList, SIGNAL( statusChanged()           ),
                 this,      SLOT  ( autoResolveDependencies() ) );

        if ( _pkgConflictDialog )
        {
            connect( _pkgConflictDialog, SIGNAL( updatePackages()   ),
                     patchList,          SLOT  ( updateItemStates() ) );
        }
    }

    if ( _filters && _patchFilterView )
    {
        connect( _filters,         SIGNAL( currentChanged( QWidget * ) ),
                 _patchFilterView, SLOT  ( showFilter    ( QWidget * ) ) );
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
YQPkgSelector::reset()
{
    logDebug() << "Reset" << endl;

    resetResolver();
    LicenseCache::confirmed()->clear();

    if ( _patchFilterView )
        _patchFilterView->reset();

    if ( _pkgList )
        _pkgList->clear();

    if ( _filters )
        _filters->reloadCurrentPage();

    updatePageLabels();

    // For all other connected QObjects that are not covered yet.
    // Don't connect this signal to the standard filter views;
    // they are already reset by reloadCurrentPage().

    emit resetNotify();
}


void
YQPkgSelector::configRepos()
{
    logDebug() << endl;

    if ( pendingChanges() && MyrlynApp::runningAsRoot() )
    {
        QMessageBox msgBox( this );

        msgBox.setText( _( "There are pending changes.\n"
                           "\n"
                           "If you continue to the repository configuration\n"
                           "and change anything there, you will have to\n"
                           "restart the program, and all package changes\n"
                           "will be lost.\n"
                           ) );
        msgBox.setIcon( QMessageBox::Question );
        QPushButton * continueButton = msgBox.addButton( _( "C&ontinue" ), QMessageBox::AcceptRole );
        QPushButton * cancelButton   = msgBox.addButton( _( "&Cancel"   ), QMessageBox::RejectRole );
        msgBox.setDefaultButton( cancelButton );
        msgBox.exec();

        if ( msgBox.clickedButton() != continueButton )
            return;
    }

    RepoConfigDialog dialog;
    dialog.exec();
}


void
YQPkgSelector::updatePageLabels()
{
    updatePageLabel( _( "Patc&hes" ), _patchFilterView,   YQPkgPatchList::countNeededPatches()   );
    updatePageLabel( _( "&Updates" ), _updatesFilterView, YQPkgUpdatesFilterView::countUpdates() );
}


void
YQPkgSelector::updatePageLabel( const QString & rawLabel, QWidget * page, int count )
{
    if ( page && _filters )
    {
        QString label = rawLabel;

        if ( count > 0 )
            label += QString( " (%1)" ).arg( count );

        _filters->setPageLabel( page, label );
    }
}


void
YQPkgSelector::globalUpdatePkg( bool force )
{
    if ( ! _pkgList )
        return;

    int count = _pkgList->globalSetPkgStatus( S_Update, force,
                                              true ); // countOnly
    logInfo() << count << " pkgs found for update" << endl;

    if ( count >= GLOBAL_UPDATE_CONFIRMATION_THRESHOLD )
    {
        QMessageBox msgBox( window() );
        msgBox.setText( _( "%1 packages will be updated" ).arg( count ) );
        msgBox.setIcon( QMessageBox::Question );
        msgBox.addButton( _( "C&ontinue" ), QMessageBox::AcceptRole );
        msgBox.addButton( QMessageBox::Cancel );
        msgBox.setDefaultButton( QMessageBox::Cancel );

        if ( msgBox.exec() == QMessageBox::Cancel )
            return;
    }

    (void) _pkgList->globalSetPkgStatus( S_Update, force,
                                         false ); // countOnly

    if ( _statusFilterView )
    {
        _filters->showPage( _statusFilterView );
        _statusFilterView->writeSettings();
        _statusFilterView->resetToDefaults();
    }
}


void
YQPkgSelector::updateSwitchRepoLabels()
{
    if ( ! _repoFilterView || ! _repoFilterView->isVisible() )
    {
        if ( _notificationsArea )
            _notificationsArea->hide();

        return;
    }

    _notificationsArea->show();
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

    _notificationsArea->setVisible( _switchToRepoLabel->isVisible() ||
                                    _cancelSwitchingToRepoLabel->isVisible() );
}


void
YQPkgSelector::switchToRepo( const QString & link )
{
    logDebug() << "link " << link << " clicked on label" << endl;

    QUrl url( link );
    if ( url.scheme() == "repoupgradeadd" )
    {
        logDebug() << "looking for repo " << url.path() << endl;

        std::string alias( url.path().remove( 0, 1 ).toStdString() );
        zypp::Repository repo( zypp::getZYpp()->pool().reposFind( alias ) );

        logDebug() << repo.name() << endl;

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
        logDebug() << "unknown link operation " << url.scheme() << endl;

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


bool
YQPkgSelector::showAutoChangesDialog()
{
    if ( _statusFilterView && _filters &&
         _filters->isCurrentPage( _statusFilterView ) &&
         _statusFilterView->showingAutomaticChanges() )
    {
        logDebug() << "Already showing automatic changes in the main windwow, "
                   << "skipping pop-up dialog" << endl;

        return false;
    }

    return true;
}


void
YQPkgSelector::pkgExcludeDebugChanged( bool on )
{
    if ( _excludeDebugInfoPkgs )
        _excludeDebugInfoPkgs->enable( ! on );

    if ( _pkgList )
        _pkgList->applyExcludeRules();
}


void
YQPkgSelector::pkgExcludeDevelChanged( bool on )
{
    if ( _excludeDevelPkgs )
        _excludeDevelPkgs->enable( ! on );

    if ( _pkgList )
        _pkgList->applyExcludeRules();
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

            logDebug() << "Found subpackage: " << name << endl;
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
                    logInfo() << "Ignoring unwanted subpackage " << subPkgName << endl;
                    break;

                case S_AutoInstall:
                case S_Install:
                case S_KeepInstalled:

                    // Install the subpackage, but don't try to update it

                    if ( ! subPkg->installedObj() )
                    {
                        subPkg->setStatus( S_Install );
                        logInfo() << "Installing subpackage " << subPkgName << endl;
                    }
                    break;


                case S_Update:
                case S_AutoUpdate:

                    // Install or update the subpackage

                    if ( ! subPkg->installedObj() )
                    {
                        subPkg->setStatus( S_Install );
                        logInfo() << "Installing subpackage " << subPkgName << endl;
                    }
                    else
                    {
                        subPkg->setStatus( S_Update );
                        logInfo() << "Updating subpackage " << subPkgName << endl;
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
    // logVerbose() << "Checking for retracted installed packages..." << endl;

    for ( ZyppPoolIterator it = zyppPkgBegin(); it != zyppPkgEnd(); ++it )
    {
        if ( (*it)->hasRetractedInstalled() )
        {
            logInfo() << "Found a retracted installed package" << endl;
            return true;
        }
    }

    logDebug() << "No retracted packages installed." << endl;

    return false;
}


void
YQPkgSelector::readSettingsEarly()
{
    // Before any widgets are created!

    QSettings settings;
    settings.beginGroup( "PackageSelector" );

    _useRpmGroups = settings.value( "useRpmGroups", (bool) USE_RPM_GROUPS ).toBool();

    settings.endGroup();
}


void
YQPkgSelector::readSettings()
{
    QSettings settings;
    settings.beginGroup( "PackageSelector" );

    _showDevelAction->setChecked( settings.value( "showDevelPackages", false ).toBool() );
    _showDebugAction->setChecked( settings.value( "showDebugPackages", false ).toBool() );

    settings.endGroup();

    pkgExcludeDevelChanged( _showDevelAction->isChecked());
    pkgExcludeDebugChanged( _showDebugAction->isChecked());

    readResolverSettings();
}


void
YQPkgSelector::writeSettings()
{
    QSettings settings;
    settings.beginGroup( "PackageSelector" );

    settings.setValue( "showDevelPackages", _showDevelAction->isChecked() );
    settings.setValue( "showDebugPackages", _showDebugAction->isChecked() );

#if 0
    // Disabled writing this default value until there is an agreement if
    // openSUSE is negatively affected by the RPM groups filter view; once the
    // users' config files contain this value, it's almost impossible to change
    // the default.
    settings.setValue( "useRpmGroups",      _useRpmGroups                 );
#endif

    settings.endGroup();

    writeResolverSettings();
}


void
YQPkgSelector::readResolverSettings()
{
    zypp::Resolver_Ptr resolver = zypp::getZYpp()->resolver();

    // Initialize fallbacks for most of the settings from the resolver which
    // may have read them from /etc/zypp/zypp*.conf

    bool autoCheckDependencies = true;
    bool installRecommended    = ! resolver->onlyRequires();
    bool verifySystem          = resolver->systemVerification();
    bool allowVendorChange     = resolver->allowVendorChange();
    bool cleanDepsOnRemove     = resolver->cleandepsOnRemove();


    QSettings settings;
    settings.beginGroup( "DependencyResolver" );

    autoCheckDependencies = settings.value( "autoCheckDependencies", autoCheckDependencies ).toBool();
    installRecommended    = settings.value( "installRecommended",    installRecommended    ).toBool();
    verifySystem          = settings.value( "verifySystem",          verifySystem          ).toBool();

#if 0
    // Intentionally not persistent
    allowVendorChange     = settings.value( "allowVendorChange",     allowVendorChange     ).toBool();
    cleanDepsOnRemove     = settings.value( "cleanDepsOnRemove",     cleanDepsOnRemove     ).toBool();
#endif

    settings.endGroup();


    // This is called in the constructor when the resolver is blocked anyway,
    // so the signal cascades that setting the actions' status may cause won't
    // have any effect: They might try to call the resolver, but that won't
    // happen since it's blocked.

    _autoDependenciesAction->setChecked  ( autoCheckDependencies );
    _installRecommendedAction->setChecked( installRecommended    );
    _allowVendorChangeAction->setChecked ( allowVendorChange     );
    _cleanDepsOnRemoveAction->setChecked ( cleanDepsOnRemove     );

    if ( _verifySystemModeAction )
        _verifySystemModeAction->setChecked( verifySystem );

    // No resolver call for autoCheckDependencies
    resolver->setSystemVerification  ( verifySystem         );
    resolver->setOnlyRequires        ( ! installRecommended );
    resolver->setAllowVendorChange   ( allowVendorChange    );
    resolver->dupSetAllowVendorChange( allowVendorChange    );
    resolver->setCleandepsOnRemove   ( cleanDepsOnRemove    );
}


void
YQPkgSelector::writeResolverSettings()
{
    QSettings settings;
    settings.beginGroup( "DependencyResolver" );

    settings.setValue( "autoCheckDependencies", _autoDependenciesAction->isChecked()   );
    settings.setValue( "installRecommended",    _installRecommendedAction->isChecked() );

    if ( _verifySystemModeAction )
        settings.setValue( "verifySystem",      _verifySystemModeAction->isChecked()   );

#if 0
    // Intentionally not persistent
    settings.setValue( "allowVendorChange",     _allowVendorChangeAction->isChecked()  );
    settings.setValue( "cleanDepsOnRemove",     _cleanDepsOnRemoveAction->isChecked()  );
#endif

    settings.endGroup();
}


void YQPkgSelector::busyCursor()
{
    ::busyCursor();
}


void YQPkgSelector::normalCursor()
{
    ::normalCursor();
}
