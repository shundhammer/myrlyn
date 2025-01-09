/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2025 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include "Logger.h"
#include "Exception.h"
#include "MainWindow.h"
#include "MyrlynRepoManager.h"
#include "utf8.h"
#include "InitReposPage.h"


InitReposPage::InitReposPage( MyrlynRepoManager * repoManager,
                              QWidget *          parent )
    : QWidget( parent )
    , _repoManager( repoManager )
    , _ui( new Ui::InitReposPage )  // Use the Qt designer .ui form
{
    // logDebug() << "Creating InitReposPage" << Qt::endl;

    CHECK_NEW( _ui );
    _ui->setupUi( this ); // Actually create the widgets from the .ui form
    _ui->reposList->setSortingEnabled( false );

    reset();
    connectSignals();
    loadIcons();

    // logDebug() << "Creating InitReposPage done" << Qt::endl;
}


InitReposPage::~InitReposPage()
{
    // logDebug() << "Destroying InitReposPage" << Qt::endl;

    delete _ui;
}


void InitReposPage::connectSignals()
{
    CHECK_PTR( _repoManager );

    connect( _repoManager, SIGNAL( foundRepo( zypp::RepoInfo ) ),
             this,         SLOT  ( foundRepo( zypp::RepoInfo ) ) );

    connect( _repoManager, SIGNAL( refreshRepoStart( zypp::RepoInfo ) ),
             this,         SLOT  ( refreshRepoStart( zypp::RepoInfo ) ) );

    connect( _repoManager, SIGNAL( refreshRepoDone ( zypp::RepoInfo ) ),
             this,         SLOT  ( refreshRepoDone ( zypp::RepoInfo ) ) );
}


void InitReposPage::loadIcons()
{
    // Both download icons need to be double-wide (not just 22x22) to have
    // enough space for the checkmark for the "download done" icon,
    // and to keep the text aligned also for the "download ongoing" icon.
    //
    // This is also the difference between "download-ongoing.svg" and
    // "download.svg" (22x22).

    QSize iconSize( 40, 22 );

    _emptyIcon           = QPixmap( ":/empty-40x22" );
    _downloadOngoingIcon = QIcon( ":/download-ongoing" ).pixmap( iconSize );
    _downloadDoneIcon    = QIcon( ":/download-done"    ).pixmap( iconSize );
}


void InitReposPage::reset()
{
    _reposCount       = 0;
    _refreshDoneCount = 0;

    _ui->reposList->clear();
    _ui->progressBar->setValue( 0 );
    _ui->progressBar->setMaximum( 100 );

    MainWindow::processEvents();
}


void InitReposPage::foundRepo( const zypp::RepoInfo & repo )
{
    _ui->progressBar->setMaximum( ++_reposCount );
    QListWidgetItem * item = new QListWidgetItem( fromUTF8( repo.name() ) );
    CHECK_NEW( item );
    item->setIcon( _emptyIcon );
    _ui->reposList->addItem( item );

    MainWindow::processEvents();
}


void InitReposPage::refreshRepoStart( const zypp::RepoInfo & repo )
{
    // logDebug() << "Repo refresh start for " << repo.name() << Qt::endl;

    QListWidgetItem * item = setItemIcon( repo, _downloadOngoingIcon );

    if ( item )
    {
        _ui->reposList->setFocus();
        _ui->reposList->setCurrentItem( item );
    }

    MainWindow::processEvents();
}


void InitReposPage::refreshRepoDone ( const zypp::RepoInfo & repo )
{
    // logDebug() << "Repo refresh done for " << repo.name() << Qt::endl;

    _ui->progressBar->setValue( ++_refreshDoneCount );
    setItemIcon( repo, _downloadDoneIcon );

    MainWindow::processEvents();
}


QListWidgetItem *
InitReposPage::setItemIcon( const zypp::RepoInfo & repo,
                            const QPixmap &        icon )
{
    QListWidgetItem * item = findRepoItem( repo );

    if ( item )
        item->setIcon( icon );

    _ui->reposList->scrollToItem( item );

    return item;
}


QListWidgetItem *
InitReposPage::findRepoItem( const zypp::RepoInfo & repo )
{
    QString repoName = fromUTF8( repo.name() );
    QListWidgetItem * item = 0;

    for ( int i=0; i < _ui->reposList->count(); i++ )
    {
        item = _ui->reposList->item( i );

        if ( item->text() == repoName )
            return item;
    }

    logError() << "No item in repos list widget for \""
               << repoName << "\"" << Qt::endl;
    return 0;
}

