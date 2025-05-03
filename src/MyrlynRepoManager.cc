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
#include <clocale>              // std::setlocale()
#include <QElapsedTimer>
#include <QMessageBox>

#include <zypp/ZYppFactory.h>
#include <zypp/Locale.h>
#include <zypp/ZConfig.h>

#include "Exception.h"
#include "KeyRingCallbacks.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MyrlynApp.h"
#include "QY2CursorHelper.h"
#include "YQi18n.h"
#include "utf8.h"
#include "MyrlynRepoManager.h"


MyrlynRepoManager::MyrlynRepoManager()
{
    logDebug() << "Creating MyrlynRepoManager" << endl;
}


MyrlynRepoManager::~MyrlynRepoManager()
{
    logDebug() << "Destroying MyrlynRepoManager..." << endl;

    shutdownZypp();

    logDebug() << "Destroying MyrlynRepoManager done" << endl;
}


void MyrlynRepoManager::initTarget()
{
    logDebug() << "Creating the ZyppLogger" << endl;
    MyrlynApp::instance()->createZyppLogger();

    logDebug() << "Initializing zypp..." << endl;

    zyppPtr()->initializeTarget( "/", false );  // don't rebuild rpmdb
    zyppPtr()->target()->load(); // Load pkgs from the target (rpmdb)

    logDebug() << "Initializing zypp done" << endl;
}


void MyrlynRepoManager::shutdownZypp()
{
    logDebug() << "Shutting down zypp..." << endl;

    _repo_manager_ptr.reset();  // deletes the RepoManager
    _zypp_ptr.reset();          // deletes the ZYpp instance

    logDebug() << "Shutting down zypp done" << endl;
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
        logDebug() << "Creating RepoManager" << endl;
        _repo_manager_ptr.reset( new zypp::RepoManager() );
    }

    return _repo_manager_ptr;
}


void MyrlynRepoManager::zyppConnect( int attempts, int waitSeconds )
{
    (void) zyppConnectInternal( attempts, waitSeconds );
    initZyppLocale();
}


void MyrlynRepoManager::initZyppLocale()
{
    // Get the locale from the environment variables. See 'locale'.
    const char * locale = std::setlocale( LC_MESSAGES, 0 );

    logInfo() << "C locale: " << locale << endl;

    try
    {
        zypp::Locale zyppLocale = zypp::Locale( locale );
        zypp::ZConfig::instance().setTextLocale( zyppLocale );
    }
    catch (...)
    {
        logWarning() << "Can't set zypp locale " << locale << endl;
    }
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
	    logInfo() << "Initializing Zypp library..." << endl;
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
        logError() << "Caught zypp exception: " << ex.asString() << endl;

        if ( ! MyrlynApp::runningAsRealRoot() )
        {
            notifyUserToRunZypperDup();

            logInfo() << "Exiting." << endl;
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
        const ZyppRepoInfo & repo = *it;

        if ( repo.enabled() )
        {
            _repos.push_back( repo );

            logInfo() << "Found repo \"" << repo.name() << "\""
                      << " URL: " << repo.url().asString()
                      << endl;

            emit foundRepo( repo );
        }
        else
        {
            logInfo() << "Ignoring disabled repo \"" << repo.name() << "\""
                      << endl;
        }
    }
}


void MyrlynRepoManager::refreshRepos()
{
    if ( ! MyrlynApp::runningAsRealRoot() )
    {
        logWarning() << "Skipping repos refresh for non-root user" << endl;
        return;
    }

    if ( MyrlynApp::isOptionSet( OptNoRepoRefresh ) )
        return;

    KeyRingCallbacks keyRingCallbacks;
    QElapsedTimer    timer;
    _failedRepos.clear();

    for ( ZyppRepoInfo & repo: _repos )
    {
        try
        {
            timer.start();
            logInfo() << "Refreshing repo " << repo.name() << "..." << endl;
            emit refreshRepoStart( repo );

            repoManager()->refreshMetadata( repo, zypp::RepoManager::RefreshIfNeeded );
            repoManager()->buildCache     ( repo, zypp::RepoManager::BuildIfNeeded   );

            if ( MyrlynApp::isOptionSet( OptSlowRepoRefresh ) )
                sleep( 2 );

            logInfo() << "Refreshing repo " << repo.name()
                      << " done after " << timer.elapsed() / 1000.0 << " sec"
                      << endl;

            emit refreshRepoDone( repo );
        }
        catch ( const zypp::repo::RepoException & exception )
        {
            Q_UNUSED( exception );
            logWarning() << "CAUGHT zypp exception for repo " << repo.name() << endl;

            logInfo() << "Disabling repo " << repo.name() << endl;
            repo.setEnabled( false );
            _failedRepos.push_back( repo );

            emit refreshRepoError( repo );
        }
    }

    showFailedRepos();
}


void MyrlynRepoManager::loadRepos()
{
    for ( const ZyppRepoInfo & repo: _repos )
    {
        if ( repo.enabled() )
        {
            logDebug() << "Loading resolvables from " << repo.name() << endl;
            repoManager()->loadFromCache( repo );
        }
        else
        {
            logInfo() << "Skipping disabled repo " << repo.name() << endl;
        }
    }
}


void MyrlynRepoManager::notifyUserToRunZypperDup() const
{
    logInfo() << "Run 'sudo zypper refresh' and restart the program." << endl;

    QString message = _( "Error loading the repos. Run\n\n"
                         "    sudo zypper refresh\n\n"
                         "and restart the program." );
    std::cerr << toUTF8( message ) << std::endl;

    QMessageBox::warning( MainWindow::instance(), // parent
                          _( "Error" ),           // window title
                          message );
}


void MyrlynRepoManager::showFailedRepos() const
{
    if ( _failedRepos.empty() )
        return;

    QString msg;

    if ( _failedRepos.size() == 1 )
    {
        QString repoName = fromUTF8( _failedRepos.front().name() );

        msg = _( "Refreshing repository \"%1\" failed.\n\n"
                 "This repository is now disabled "
                 "for this program run." ).arg( repoName );
    }
    else
    {
        QString repoNameList;

        for ( const ZyppRepoInfo & repo: _failedRepos )
            repoNameList += QString( "  - %1\n" ).arg( fromUTF8( repo.name() ) );

        msg = _( "Refreshing failed for repositories\n\n"
                 "%1\n"
                 "Those repositories are now disabled "
                 "for this program run." ).arg( repoNameList );
    }

    normalCursor();
    QMessageBox::warning( MainWindow::instance(), // parent
                          _( "Warning" ),         // window title
                          msg );
}
