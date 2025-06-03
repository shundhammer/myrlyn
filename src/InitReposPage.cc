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


#include <zypp/RepoInfo.h>

#include "Logger.h"
#include "Exception.h"
#include "MainWindow.h"
#include "MyrlynApp.h"
#include "MyrlynRepoManager.h"
#include "utf8.h"
#include "YQZypp.h"
#include "InitReposPage.h"


InitReposPage::InitReposPage( MyrlynRepoManager * repoManager,
                              QWidget *           parent )
    : QWidget( parent )
    , _repoManager( repoManager )
    , _ui( new Ui::InitReposPage )  // Use the Qt designer .ui form
{
    // logDebug() << "Creating InitReposPage" << endl;

    CHECK_NEW( _ui );
    _ui->setupUi( this ); // Actually create the widgets from the .ui form
    MyrlynApp::setHeadingFont( _ui->headingLabel );
    _ui->reposList->setSortingEnabled( false );

    reset();
    connectSignals();
    loadIcons();

    // logDebug() << "Creating InitReposPage done" << endl;
}


InitReposPage::~InitReposPage()
{
    // logDebug() << "Destroying InitReposPage" << endl;

    delete _ui;
}


void InitReposPage::connectSignals()
{
    CHECK_PTR( _repoManager );

    connect( _repoManager, SIGNAL( foundRepo( ZyppRepoInfo ) ),
             this,         SLOT  ( foundRepo( ZyppRepoInfo ) ) );

    connect( _repoManager, SIGNAL( refreshRepoStart( ZyppRepoInfo ) ),
             this,         SLOT  ( refreshRepoStart( ZyppRepoInfo ) ) );

    connect( _repoManager, SIGNAL( refreshRepoDone ( ZyppRepoInfo ) ),
             this,         SLOT  ( refreshRepoDone ( ZyppRepoInfo ) ) );

    connect( _repoManager, SIGNAL( refreshRepoError( ZyppRepoInfo ) ),
             this,         SLOT  ( refreshRepoError( ZyppRepoInfo ) ) );
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
    _downloadErrorIcon   = QIcon( ":/emoji-sad"        ).pixmap( iconSize );
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


void InitReposPage::foundRepo( const ZyppRepoInfo & repo )
{
    _ui->progressBar->setMaximum( ++_reposCount );
    QListWidgetItem * item = new QListWidgetItem( fromUTF8( repo.name() ) );
    CHECK_NEW( item );
    item->setIcon( _emptyIcon );
    _ui->reposList->addItem( item );

    MainWindow::processEvents();
}


void InitReposPage::refreshRepoStart( const ZyppRepoInfo & repo )
{
    // logDebug() << "Repo refresh start for " << repo.name() << endl;

    QListWidgetItem * item = setItemIcon( repo, _downloadOngoingIcon );

    if ( item )
    {
        _ui->reposList->setFocus();
        _ui->reposList->setCurrentItem( item );
    }

    MainWindow::processEvents();
}


void InitReposPage::refreshRepoDone( const ZyppRepoInfo & repo )
{
    // logDebug() << "Repo refresh done for " << repo.name() << endl;

    _ui->progressBar->setValue( ++_refreshDoneCount );
    setItemIcon( repo, _downloadDoneIcon );

    MainWindow::processEvents();
}


void InitReposPage::refreshRepoError( const ZyppRepoInfo & repo )
{
    // logDebug() << "Repo refresh error for " << repo.name() << endl;

    _ui->progressBar->setValue( ++_refreshDoneCount );
    setItemIcon( repo, _downloadErrorIcon );

    MainWindow::processEvents();
}


QListWidgetItem *
InitReposPage::setItemIcon( const ZyppRepoInfo & repo,
                            const QPixmap &        icon )
{
    QListWidgetItem * item = findRepoItem( repo );

    if ( item )
        item->setIcon( icon );

    _ui->reposList->scrollToItem( item );

    return item;
}


QListWidgetItem *
InitReposPage::findRepoItem( const ZyppRepoInfo & repo )
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
               << repoName << "\"" << endl;
    return 0;
}

