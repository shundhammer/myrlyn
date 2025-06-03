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

 */


#include <QMessageBox>

#include "Exception.h"
#include "InitReposPage.h"
#include "Logger.h"
#include "MainWindow.h"
#include "PkgCommitPage.h"
#include "QY2CursorHelper.h"
#include "SummaryPage.h"
#include "YQPkgSelector.h"
#include "YQi18n.h"
#include "MyrlynApp.h"
#include "MyrlynRepoManager.h"
#include "MyrlynWorkflowSteps.h"

#if 0
#include "WizardPage.h"
// If needed, move the source files back here  from the attic/ subdirectory:
//
//   WizardPage.h
//   WizardPage.cc
//   wizard-page.ui
//
// and re-enable them in CMakeLists.txt
#endif


MyrlynWorkflowStep::MyrlynWorkflowStep( MyrlynApp * app,
                                            const QString &    id,
                                            const QString &    next )
    : WorkflowStep( id, next )
    , _app( app )
    , _page(0)
    , _doProcessEvents( false )
    , _doDeletePage( true )
{
    logDebug() << "Creating step " << _id << endl;
}


MyrlynWorkflowStep::~MyrlynWorkflowStep()
{
    logDebug() << "Destroying step " << _id << "..." << endl;

    if ( _doDeletePage && _page )
    {
        logDebug() << "  Deleting page of step " << _id << endl;
        delete _page;
    }

    logDebug() << "Destroying step " << _id << " done" << endl;
}


void MyrlynWorkflowStep::ensurePage()
{
    if ( _page )
        return;

    _page = page(); // First try: Get it without creating

    if ( _page )
        _doDeletePage = false; // We don't own this one
    else
    {
        // Second try: Create it

        _doDeletePage = true;  // We own it now (by default)
        _page = createPage();
    }

    if ( ! _page )      // Still no page?
    {
        logError() << "FATAL: Implement one of page() or createPage()" << endl;
        CHECK_PTR( _page );
    }

    _app->mainWin()->addPage( _page );
}


void MyrlynWorkflowStep::activate( bool goingForward )
{
    Q_UNUSED( goingForward );
    logDebug() << "Activating step " << _id << endl;

    ensurePage();
    _app->mainWin()->showPage( _page );

    if ( _doProcessEvents )
        MainWindow::processEvents();
}


void MyrlynWorkflowStep::nextPage( bool goingForward )
{
    // Automatically continue with the next step in the same direction

    if ( goingForward || _app->workflow()->historyEmpty() )
    {
        logDebug() << "_app->next()" << endl;
        _app->next();
    }
    else
    {
        logDebug() << "_app->back()" << endl;
        _app->back();
    }
}


//----------------------------------------------------------------------


void InitReposStep::activate( bool goingForward )
{
    MyrlynWorkflowStep::activate( goingForward ); // Show the page

    if ( goingForward )
        initRepos();  // Do the one-time work

    nextPage( goingForward );
}


QWidget * InitReposStep::createPage()
{
    _doProcessEvents = true; // This is a splash screen, so process events

    return new InitReposPage( _app->repoManager() );
}


void InitReposStep::initRepos()
{
    if ( _reposInitialized )
        return;

    logDebug() << "Initializing zypp..." << endl;

    MyrlynRepoManager * repoMan = _app->repoManager();
    CHECK_PTR( repoMan );
    busyCursor();

    try
    {
        repoMan->zyppConnect(); // This may throw
    }
    catch ( ... )
    {
        normalCursor();
        QString message = _( "Can't connect to the package manager!\n"
                             "It may be busy in another window.\n" );

        QMessageBox::warning( 0, // parent
                              _( "Error" ),
                              message );

        throw;  // Nothing else that we can do here
    }

    repoMan->initTarget();
    repoMan->attachRepos();
    normalCursor();

    logDebug() << "Initializing zypp done" << endl;
    _reposInitialized = true;
}


//----------------------------------------------------------------------


QWidget *
PkgSelStep::page()
{
    busyCursor();

    // This might take a few seconds
    QWidget * pg = _app->pkgSel();
    normalCursor();

    return pg;
}


void PkgSelStep::activate( bool goingForward )
{
    MyrlynWorkflowStep::activate( goingForward ); // Show the page

    if ( ! goingForward )
        _app->pkgSel()->reset(); // includes resetResolver()
}


//----------------------------------------------------------------------


QWidget * PkgCommitStep::page()
{
    _doProcessEvents = true;  // Make sure to display this page

    return _app->pkgCommitPage();
}


void PkgCommitStep::activate( bool goingForward )
{
    if ( goingForward )
    {
        MyrlynWorkflowStep::activate( goingForward ); // Show the page
        _app->pkgCommitPage()->reset();  // Reset the widgets on the page

        _app->pkgCommitPage()->commit(); // Do the package transactions
    }

    nextPage( goingForward );
}


//----------------------------------------------------------------------


QWidget * SummaryStep::page()
{
    return _app->summaryPage();
}


void SummaryStep::activate( bool goingForward )
{
    MyrlynWorkflowStep::activate( goingForward );

    _app->summaryPage()->updateSummary();
    _app->summaryPage()->startCountdown();
}


void SummaryStep::deactivate( bool goingForward )
{
    _app->summaryPage()->stopCountdown();
    MyrlynWorkflowStep::deactivate( goingForward );
}


//----------------------------------------------------------------------


#if 0

QWidget * WizardStep::createPage()
{
    WizardPage * wizardPage = new WizardPage( _id );
    CHECK_NEW( wizardPage );

    wizardPage->connect( _app );

    return wizardPage;
}

#endif
