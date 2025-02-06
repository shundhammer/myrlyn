/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include <unistd.h>     // getuid()

#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QCloseEvent>

#include "Exception.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MyrlynRepoManager.h"
#include "MyrlynWorkflowSteps.h"
#include "PkgCommitPage.h"
#include "PkgTasks.h"
#include "SummaryPage.h"
#include "Workflow.h"
#include "YQPkgSelector.h"
#include "YQi18n.h"
#include "ZyppLogger.h"
#include "MyrlynApp.h"


MyrlynApp * MyrlynApp::_instance = 0;
MyrlynAppOptions    MyrlynApp::_optFlags( OptNone );


MyrlynApp::MyrlynApp( MyrlynAppOptions optFlags )
    : QObject()
    , _mainWin(0)
    , _workflow(0)
    , _pkgSel(0)
    , _pkgCommitPage(0)
    , _summaryPage(0)
    , _myrlynRepoManager(0)
    , _zyppLogger(0)
    , _pkgTasks(0)
{
    logDebug() << "Creating MyrlynApp" << Qt::endl;

    _instance = this;
    _optFlags = optFlags;

    if ( ! runningAsRoot() )
    {
        logInfo() << "Not running as root - enforcing read-only mode" << Qt::endl;
        _optFlags |= OptReadOnly;
    }

    logDebug() << "_optFlags: 0x" << Qt::hex << _optFlags << Qt::dec << Qt::endl;

    createMainWin(); // Create this early to get early visual feedback
    logDebug() << "Creating MyrlynApp done" << Qt::endl;
}


MyrlynApp::~MyrlynApp()
{
    logDebug() << "Destroying MyrlynApp..." << Qt::endl;

    if ( _pkgSel )
        delete _pkgSel;

    if ( _pkgCommitPage )
        delete _pkgCommitPage;

    if ( _summaryPage )
        delete _summaryPage;

    if ( _pkgTasks )
        delete _pkgTasks;

    if ( _workflow )
        delete _workflow;

    if ( _mainWin )
        delete _mainWin;

    if ( _myrlynRepoManager )
        delete _myrlynRepoManager;

    if ( _zyppLogger )
        delete _zyppLogger;

    _instance = 0;

    logDebug() << "Destroying MyrlynApp done" << Qt::endl;
}


void MyrlynApp::run()
{
    logDebug() << Qt::endl;

    if ( ! _workflow )
        createWorkflow();
    else
        _workflow->restart();

    // Control between main window pages and workflow steps is now entirely up
    // to the workflow object and the events passed back and forth between the
    // pages / workflow steps (each step has one page):
    //
    // They send signals to trigger slots like 'next()', 'back()', 'quit()',
    // 'restart()' (which goes back to the package selection).

    qApp->exec();  // Qt main event loop

    // Only 'quit()' or an exception terminate the event loop.
}


void MyrlynApp::createMainWin()
{
    logDebug() << Qt::endl;

    if ( _mainWin )
        return;

    logDebug() << "Creating the main window" << Qt::endl;

    _mainWin = new MainWindow();
    CHECK_NEW( _mainWin );

    setWindowTitle( _mainWin );
    _mainWin->installEventFilter( this );
    _mainWin->show();
}


void MyrlynApp::createWorkflow()
{
    if ( _workflow )
        return;

    logDebug() << "Creating the application workflow" << Qt::endl;

    WorkflowStepList steps;
    steps << new InitReposStep( this, "initRepos" ) // excluded from history
          << new PkgSelStep   ( this, "pkgSel"    )
          << new PkgCommitStep( this, "pkgCommit" ) // excluded from history
          << new SummaryStep  ( this, "summary"   );

    _workflow = new Workflow( steps );
    CHECK_PTR( _workflow );

    logDebug() << "Starting workflow" << Qt::endl;
    _workflow->start();
}


void MyrlynApp::setWindowTitle( QWidget * window )
{
    if ( window )
    {
        QString windowTitle( "Myrlyn" );

        if ( runningAsRoot() )
            windowTitle += _( " [root]" );

        if ( readOnlyMode() )
            windowTitle += _( " (read-only)" );

        window->setWindowTitle( windowTitle );
    }
}


YQPkgSelector *
MyrlynApp::pkgSel()
{
    if ( ! _pkgSel )
        createPkgSel(); // Created lazy because this takes a moment

    return _pkgSel;
}

void MyrlynApp::createPkgSel()
{
    if ( _pkgSel )
        return;

    QLabel busyPage( _( "Preparing the package selector..." ) );

    if ( _mainWin )
        _mainWin->splashPage( &busyPage );

    _pkgSel = new YQPkgSelector( 0 );
    CHECK_NEW( _pkgSel );

    connect( _pkgSel, SIGNAL( commit() ),
             this,    SLOT  ( next()   ) );

    connect( _pkgSel, SIGNAL( finished()   ),   // "accept" without changes
             this,    SLOT  ( skipCommit() ) );
}


PkgCommitPage *
MyrlynApp::pkgCommitPage()
{
    if ( ! _pkgCommitPage )
        createPkgCommitPage();

    return _pkgCommitPage;
}


void MyrlynApp::createPkgCommitPage()
{
    if ( _pkgCommitPage )
        return;

    _pkgCommitPage = new PkgCommitPage();
    CHECK_NEW( _pkgCommitPage );

    QObject::connect( _pkgCommitPage, SIGNAL( next() ),
                      this,          SLOT  ( next() ) );
}


SummaryPage *
MyrlynApp::summaryPage()
{
    if ( ! _summaryPage )
        createSummaryPage();

    return _summaryPage;
}


void MyrlynApp::createSummaryPage()
{
    if ( _summaryPage )
        return;

    _summaryPage = new SummaryPage();
    CHECK_NEW( _summaryPage );

    QObject::connect( _summaryPage, SIGNAL( back() ),
                      this,         SLOT  ( back() ) );

    QObject::connect( _summaryPage, SIGNAL( finish() ),
                      this,         SLOT  ( finish() ) );

}


MyrlynRepoManager *
MyrlynApp::repoManager()
{
    if ( ! _myrlynRepoManager )
        createRepoManager();

    return _myrlynRepoManager;
}


void MyrlynApp::createRepoManager()
{
    if ( _myrlynRepoManager )
        return;

    _myrlynRepoManager = new MyrlynRepoManager();
    CHECK_NEW( _myrlynRepoManager );
}


ZyppLogger *
MyrlynApp::zyppLogger()
{
    if ( ! _zyppLogger )
        createZyppLogger();

    return _zyppLogger;
}


void MyrlynApp::createZyppLogger()
{
    if ( _zyppLogger )
        return;

    _zyppLogger = new ZyppLogger();
    CHECK_NEW( _zyppLogger );
}


PkgTasks *
MyrlynApp::pkgTasks()
{
    if ( ! _pkgTasks )
        createPkgTasks();

    return _pkgTasks;
}


void MyrlynApp::createPkgTasks()
{
    if ( _pkgTasks )
        return;

    _pkgTasks = new PkgTasks();
    CHECK_NEW( _pkgTasks );
}


bool MyrlynApp::runningAsRoot()
{
    if ( isOptionSet( OptFakeRoot ) )
        return true;

    return geteuid() == 0;
}


/**
 * Catch some events from other QObjects to redirect them if needed:
 * For example WM_CLOSE triggers the same as "Cancel" in the package selector.
 **/
bool MyrlynApp::eventFilter( QObject * watchedObj, QEvent * event )
{
    if ( _mainWin && watchedObj == _mainWin && _pkgSel
	 && event && event->type() == QEvent::Close ) // WM_CLOSE (Alt-F4)
    {
        QWidget * currentPage = _mainWin->currentPage();

        if ( currentPage == _pkgSel )
        {
            logInfo() << "Caught WM_CLOSE for YQPkgSelector" << Qt::endl;
            _pkgSel->wmClose();  // _pkgSel handles asking for confirmation etc.

            return true;        // Event processing finished for this one
        }
        else if ( currentPage == _pkgCommitPage )
        {
            logInfo() << "Caught WM_CLOSE for PkgCommitPage" << Qt::endl;
            _pkgCommitPage->wmClose();

            return true;        // Event processing finished for this one
        }
        else
        {
            logInfo() << "Caught WM_CLOSE, but not for YQPkgSelector" << Qt::endl;
        }
    }

    return false; // Event processing not yet finished, continue down the chain
}


void MyrlynApp::next()
{
    CHECK_PTR( _workflow );

    if ( ! _workflow )
        return;

    logDebug() << "Current page: " << _workflow->currentStep()->id() << Qt::endl;

    if ( _workflow->currentStep()->id() == "pkgCommit" )
    {
        if ( ! _pkgCommitPage->showSummaryPage() )
        {
            logInfo() << "Skipping summary page -> quitting" << Qt::endl;
            quit();

            return;
        }
    }

    if ( _workflow->atLastStep() )
    {
        logDebug() << "This was the last step." << Qt::endl;
        finish();
    }
    else
    {
        logDebug() << "Next step in the workflow." << Qt::endl;
        _workflow->next();
    }
}


void MyrlynApp::finish()
{
    logDebug() << "Quitting the program." << Qt::endl;
    quit();
}


void MyrlynApp::back()
{
    logDebug() << Qt::endl;
    CHECK_PTR( _workflow );

    if ( ! _workflow )
        return;

    _workflow->back();
}


void MyrlynApp::restart()
{
    logDebug() << Qt::endl;
    CHECK_PTR( _workflow );

    if ( ! _workflow )
        return;

    _workflow->gotoStep( "pkgSel" );
}


void MyrlynApp::skipCommit()
{
    // The user finished the package selection with "Accept", but there was no
    // change: Skip the "commit" phase and go straight to the summary screen.

    logDebug() << Qt::endl;
    CHECK_PTR( _workflow );

    if ( ! _workflow )
        return;

    _workflow->gotoStep( "summary" );
}


void MyrlynApp::quit( bool askForConfirmation )
{
    logDebug() << Qt::endl;

    if ( askForConfirmation )
    {
	int result = QMessageBox::warning( _mainWin,
                                           "",  // Window title
                                           _( "Really quit?" ),
                                           _( "&Yes" ), _( "&No" ), "",
                                           0,   // defaultButtonNumber (from 0)
                                           1 ); // escapeButtonNumber
        if ( result != 0 ) // button #0 (Yes)
            return;
    }

    if ( _mainWin )
        _mainWin->hide();  // For instant user feedback

    qApp->quit();
}
