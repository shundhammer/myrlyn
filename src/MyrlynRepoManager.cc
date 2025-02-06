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


#include <unistd.h>             // sleep()
#include <iostream>             // cerr
#include <QElapsedTimer>
#include <QMessageBox>

#include <zypp/ZYppFactory.h>

#include "Exception.h"
#include "KeyRingCallbacks.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MyrlynApp.h"
#include "YQi18n.h"
#include "utf8.h"
#include "MyrlynRepoManager.h"


MyrlynRepoManager::MyrlynRepoManager()
{
    logDebug() << "Creating MyrlynRepoManager" << Qt::endl;
}


MyrlynRepoManager::~MyrlynRepoManager()
{
    logDebug() << "Destroying MyrlynRepoManager..." << Qt::endl;

    shutdownZypp();

    logDebug() << "Destroying MyrlynRepoManager done" << Qt::endl;
}


void MyrlynRepoManager::initTarget()
{
    logDebug() << "Creating the ZyppLogger" << Qt::endl;
    MyrlynApp::instance()->createZyppLogger();

    logDebug() << "Initializing zypp..." << Qt::endl;

    zyppPtr()->initializeTarget( "/", false );  // don't rebuild rpmdb
    zyppPtr()->target()->load(); // Load pkgs from the target (rpmdb)

    logDebug() << "Initializing zypp done" << Qt::endl;
}


void MyrlynRepoManager::shutdownZypp()
{
    logDebug() << "Shutting down zypp..." << Qt::endl;

    _repo_manager_ptr.reset();  // deletes the RepoManager
    _zypp_ptr.reset();          // deletes the ZYpp instance

    logDebug() << "Shutting down zypp done" << Qt::endl;
}


zypp::ZYpp::Ptr
MyrlynRepoManager::zyppPtr()
{
    if ( ! _zypp_ptr )
        _zypp_ptr = zyppConnectInternal();

    return _zypp_ptr;
}


RepoManager_Ptr
MyrlynRepoManager::repoManager()
{
    if ( ! _repo_manager_ptr )
    {
        logDebug() << "Creating RepoManager" << Qt::endl;
        _repo_manager_ptr.reset( new zypp::RepoManager() );
    }

    return _repo_manager_ptr;
}


void MyrlynRepoManager::zyppConnect( int attempts, int waitSeconds )
{
    (void) zyppConnectInternal( attempts, waitSeconds );
}


//
// Stolen from yast-pkg-bindings/src/PkgFunctions.cc
//

zypp::ZYpp::Ptr
MyrlynRepoManager::zyppConnectInternal( int attempts, int waitSeconds )
{
    while ( _zypp_ptr == NULL && attempts > 0 )
    {
	try
	{
	    logInfo() << "Initializing Zypp library..." << Qt::endl;
	    _zypp_ptr = zypp::getZYpp();

 	    // initialize solver flag, be compatible with zypper
	    _zypp_ptr->resolver()->setIgnoreAlreadyRecommended( true );

	    return _zypp_ptr;
	}
	catch ( const zypp::Exception & ex )
	{
	    if ( attempts == 1 )  // last attempt?
		ZYPP_RETHROW( ex );
	}

	attempts--;

	if ( _zypp_ptr == NULL && attempts > 0 )
	    sleep( waitSeconds );
    }

    if ( _zypp_ptr == NULL )
    {
	// Still not initialized; throw an exception.
	THROW( Exception( "Can't connect to the package manager" ) );
    }

    return _zypp_ptr;
}


void MyrlynRepoManager::attachRepos()
{
    // TO DO: check and load services (?)

    try
    {
        findEnabledRepos();
        refreshRepos();
        loadRepos();
    }
    catch ( const zypp::Exception & ex )
    {
        logError() << "Caught zypp exception: " << ex.asString() << Qt::endl;

        if ( ! MyrlynApp::runningAsRealRoot() )
        {
            notifyUserToRunZypperDup();

            logInfo() << "Exiting." << Qt::endl;
            exit( 1 );
        }

        throw;  // Nothing else that we can do here
    }
}


void MyrlynRepoManager::findEnabledRepos()
{
    for ( zypp::RepoManager::RepoConstIterator it = repoManager()->repoBegin();
          it != repoManager()->repoEnd();
          ++it )
    {
        ZyppRepoInfo repo = *it;

        if ( repo.enabled() )
        {
            _repos.push_back( repo );

            logInfo() << "Found repo \"" << repo.name() << "\""
                      << " URL: " << repo.url().asString()
                      << Qt::endl;

            emit foundRepo( repo );
        }
        else
        {
            logInfo() << "Ignoring disabled repo \"" << repo.name() << "\""
                      << Qt::endl;
        }
    }
}


void MyrlynRepoManager::refreshRepos()
{
    if ( ! MyrlynApp::runningAsRealRoot() )
    {
        logWarning() << "Skipping repos refresh for non-root user" << Qt::endl;
        return;
    }

    if ( MyrlynApp::isOptionSet( OptNoRepoRefresh ) )
        return;

    KeyRingCallbacks keyRingCallbacks;
    QElapsedTimer    timer;

    for ( ZyppRepoInfo & repo: _repos )
    {
        try
        {
            timer.start();
            logInfo() << "Refreshing repo " << repo.name() << "..." << Qt::endl;
            emit refreshRepoStart( repo );

            repoManager()->refreshMetadata( repo, zypp::RepoManager::RefreshIfNeeded );
            repoManager()->buildCache     ( repo, zypp::RepoManager::BuildIfNeeded   );

            if ( MyrlynApp::isOptionSet( OptSlowRepoRefresh ) )
                sleep( 2 );

            logInfo() << "Refreshing repo " << repo.name()
                      << " done after " << timer.elapsed() / 1000.0 << " sec"
                      << Qt::endl;

            emit refreshRepoDone( repo );
        }
        catch ( const zypp::repo::RepoException & exception )
        {
            Q_UNUSED( exception );
            logWarning() << "CAUGHT zypp exception for repo " << repo.name() << Qt::endl;

            logInfo() << "Disabling repo " << repo.name() << Qt::endl;
            repo.setEnabled( false );
        }
    }
}


void MyrlynRepoManager::loadRepos()
{
    for ( const ZyppRepoInfo & repo: _repos )
    {
        if ( repo.enabled() )
        {
            logDebug() << "Loading resolvables from " << repo.name() << Qt::endl;
            repoManager()->loadFromCache( repo );
        }
        else
        {
            logInfo() << "Skipping disabled repo " << repo.name() << Qt::endl;
        }
    }
}


void MyrlynRepoManager::notifyUserToRunZypperDup() const
{
    logInfo() << "Run 'sudo zypper refresh' and restart the program." << Qt::endl;

    QString message = _( "Error loading the repos. Run\n\n"
                         "    sudo zypper refresh\n\n"
                         "and restart the program." );
    std::cerr << toUTF8( message ) << std::endl;

    QMessageBox::warning( MainWindow::instance(), // parent
                          _( "Error" ),
                          message );
}

