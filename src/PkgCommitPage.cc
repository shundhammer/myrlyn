/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  YQPkg Package Selector
    Copyright (c) 2024 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include <unistd.h>             // usleep()
#include <stdlib.h>             // abs()

#include <zypp/target/TargetException.h>

#include <QSettings>
#include <QTimer>
#include <QMessageBox>

#include "BusyPopup.h"
#include "Exception.h"
#include "Logger.h"
#include "PkgTasks.h"
#include "PkgTaskListWidget.h"
#include "YQPkgApplication.h"
#include "YQZypp.h"
#include "YQi18n.h"
#include "utf8.h"
#include "PkgCommitCallbacks.h"
#include "PkgCommitPage.h"

#define VERBOSE_PROGRESS        0
#define VERBOSE_TRANSACT        1


PkgCommitPage * PkgCommitPage::_instance = 0;


PkgCommitPage::PkgCommitPage( QWidget * parent )
    : QWidget( parent )
    , _ui( new Ui::PkgCommitPage ) // Use the Qt designer .ui form (XML)
    , _pkgTasks( 0 )
    , _showingDetails( false )
    , _startedInstallingPkg( false )
{
    CHECK_PTR( _ui );
    _ui->setupUi( this ); // Actually create the widgets from the .ui form

    // See ui_pkg-commit-page.h in ../build/yqkg_autogen/include for the
    // widgets' variable names. They are taken from the widget names in the .ui
    // form (an XML file) that was generated with Qt designer, so choose them
    // carefully when using Qt designer.

    _ui->todoList->setSortByInsertionSequence( false );
    _ui->todoList->setAutoScrollToLast( false );

    readSettings();
    loadIcons();
    reset();
    connectWidgets();
    PkgCommitSignalForwarder::instance()->connectAll( this );

    _instance = this;
}


PkgCommitPage::~PkgCommitPage()
{
    writeSettings();
    delete _ui;
    // PkgCommitSignalForwarder::instance()->deleteLater();

    _instance = 0;
}


void PkgCommitPage::connectWidgets()
{
    connect( _ui->detailsButton, SIGNAL( clicked()       ),
             this,               SLOT  ( toggleDetails() ) );

    connect( _ui->cancelButton,  SIGNAL( clicked() ),
             this,               SLOT  ( cancelCommit()  ) );
}


void PkgCommitPage::commit()
{
    populateLists();
    initProgressData();
    _startedInstallingPkg = false;
    _ui->totalProgressBar->setValue( 0 );
    PkgCommitSignalForwarder::instance()->reset();

    if ( YQPkgApplication::isOptionSet( OptFakeCommit ) )
        fakeCommit();
    else
        realCommit();
}


void PkgCommitPage::populateLists()
{
    _ui->todoList->clear();
    _ui->downloadsList->clear();
    _ui->doingList->clear();
    _ui->doneList->clear();

    _ui->todoList->addTaskItems( pkgTasks()->todo() );

    // The other lists are almost certainly empty anyway at this point, but
    // let's make sure, not make assumptions; this will be over very quickly if
    // they are actually empty.

    _ui->doingList->addTaskItems( pkgTasks()->doing() );
    _ui->doneList->addTaskItems ( pkgTasks()->done()  );
}


PkgTasks * PkgCommitPage::pkgTasks()
{
    if ( ! _pkgTasks )
        _pkgTasks =  YQPkgApplication::instance()->pkgTasks();

    CHECK_PTR( _pkgTasks );

    return _pkgTasks;
}


void PkgCommitPage::fakeCommit()
{
    logInfo() << "Simulating package transactions" << Qt::endl;

    QListWidgetItem * item = _ui->todoList->count() > 0 ?
        _ui->todoList->item( 0 ) : 0;

    for ( int i=1; i <= 100; ++i )
    {
        if ( PkgCommitSignalForwarder::instance()->doAbort() )
            return;

        if ( i == 20 && item )
            item->setIcon( _downloadOngoingIcon );

        if ( i == 60 && item )
            item->setIcon( _downloadDoneIcon );

        usleep( 100 * 1000 ); // microseconds
        _ui->totalProgressBar->setValue( i );
        processEvents();
    }

    logInfo() << "Simulating transactions done" << Qt::endl;
}


void PkgCommitPage::realCommit()
{
    processEvents();

    // Create and install the callbacks.
    // They are uninstalled when the 'callbacks' variable goes out of scope.
    PkgCommitCallbacks callbacks;
    PkgCommitSignalForwarder::instance()->reset();

    try
    {
        logInfo() << "Starting package transactions" << Qt::endl;

        zypp::getZYpp()->commit( commitPolicy() );

        logInfo() << "Package transactions done" << Qt::endl;
    }
    catch ( const zypp::target::TargetAbortedException & ex )
    {
        logInfo() << "libzypp aborted as requested" << Qt::endl;
    }

}


zypp::ZYppCommitPolicy
PkgCommitPage::commitPolicy() const
{
    zypp::ZYppCommitPolicy policy;

    if ( YQPkgApplication::isOptionSet( OptDryRun ) )
    {
        logInfo() << "dry run" << Qt::endl;
        policy.dryRun( true );
    }

    if ( YQPkgApplication::isOptionSet( OptDownloadOnly ) )
    {
        logInfo() << "download only" << Qt::endl;
        policy.downloadMode( zypp::DownloadOnly );
    }

    policy.allowDowngrade( true );

    return policy;
}


bool PkgCommitPage::showSummaryPage() const
{
    return _ui->showSummaryPageCheckBox->isChecked();
}


void PkgCommitPage::reset()
{
    _ui->totalProgressBar->setValue( 0 );

    _ui->todoList->clear();
    _ui->downloadsList->clear();
    _ui->doingList->clear();
    _ui->doneList->clear();

    _ui->detailsFrame->setVisible( _showingDetails );
    updateDetailsButton();
}


void PkgCommitPage::loadIcons()
{
    // Both download icons need to be double-wide (not just 22x22) to have
    // enough space for the checkmark for the "download done" icon,
    // and to keep the text aligned also for the "download ongoing" icon.
    //
    // This is also the difference between "download-ongoing.svg" and
    // "download.svg" (22x22).

    QSize iconSize( 40, 22 );

    _downloadOngoingIcon = QIcon( ":/download-ongoing" ).pixmap( iconSize );
    _downloadDoneIcon    = QIcon( ":/download-done"    ).pixmap( iconSize );
}


void PkgCommitPage::toggleDetails()
{
    _showingDetails = ! _showingDetails;
    _ui->detailsFrame->setVisible( _showingDetails );
    updateDetailsButton();
}


void PkgCommitPage::updateDetailsButton()
{
    QString text = showingDetails() ?
        _( "Hide &Details" ) : _( "Show &Details" );

    _ui->detailsButton->setText( text );
}


void PkgCommitPage::cancelCommit()
{
    bool confirm = askForCancelCommitConfirmation();

    if ( confirm )
    {
        logInfo() << "Aborting commit. Notifying libzypp..." << Qt::endl;
        emit abortCommit();

        // Wait for libzypp to return from its commit() so it can shut down
        // properly.
    }
}


void PkgCommitPage::wmClose()
{
    bool confirm = askForCancelCommitConfirmation();

    if ( confirm )
    {
        logInfo() << "Aborting commit. Notifying libzypp..." << Qt::endl;
        emit abortCommit();

        // Give libzypp some time to shut down properly: Execute yqapp->quit()
        // only after returning to event loop after some time has passed.

        QTimer::singleShot( 500, // millisec
                            YQPkgApplication::instance(), SLOT( quit() ) );
    }
}


bool PkgCommitPage::askForCancelCommitConfirmation()
{
    // Not all users might know what "package transactions" means,
    // so let's be a bit clearer (albeit less precise)

    QString msg = _( "Really cancel the current package actions?" );

    int result = QMessageBox::warning( this, // parent widget
                                       "",   // Window title
                                       msg,
                                       _( "&Yes" ), _( "&No" ), "",
                                       0,   // defaultButtonNumber (from 0)
                                       1 ); // escapeButtonNumber

    return result == 0;  // button #0 (Yes)
}


void PkgCommitPage::readSettings()
{
    QSettings settings;
    settings.beginGroup( "PkgCommitPage" );

    _showingDetails      = settings.value( "showingDetails",  false ).toBool();
    bool showSummaryPage = settings.value( "showSummaryPage", true ).toBool();

    settings.endGroup();

    _ui->showSummaryPageCheckBox->setChecked( showSummaryPage );
}


void PkgCommitPage::writeSettings()
{
    QSettings settings;
    settings.beginGroup( "PkgCommitPage" );

    settings.setValue( "showingDetails",  _showingDetails );
    settings.setValue( "showSummaryPage", showSummaryPage() );

    settings.endGroup();
}


void PkgCommitPage::processEvents()
{
    QCoreApplication::processEvents( QEventLoop::AllEvents,
                                     500 ); //millisec
}


void PkgCommitPage::initProgressData()
{
    _totalDownloadSize      = 0;
    _totalInstalledSize     = 0;
    _totalTasksCount        = pkgTasks()->todo().size();

    _completedDownloadSize  = 0;
    _completedInstalledSize = 0;
    _completedTasksCount    = 0;

    foreach ( PkgTask * task, pkgTasks()->todo() )
    {
        if ( ( task->action() & PkgAdd ) && task->downloadSize() > 0 )
            _totalDownloadSize += task->downloadSize();

        if ( task->installedSize() > 0 )
            _totalInstalledSize += task->installedSize();
    }

    logDebug() << "total download size:  " << _totalDownloadSize.asString()  << Qt::endl;
    logDebug() << "total installed size: " << _totalInstalledSize.asString() << Qt::endl;
    logDebug() << "total tasks: "          << _totalTasksCount << Qt::endl;

    // Weights for different sub-tasks of downloading and installing packages:
    // There is a constant cost for doing anything with a package, no matter if
    // it's installing or removing it: The 'handling' of the package.
    //
    // Of course a large part of the cost is the download, and another is the
    // cost of actually installing or removing it, be it unpacking an RPM (for
    // installing a package) or removing it (removing every item of its file
    // list).

    _pkgDownloadWeight  = 0.60;
    _pkgActionWeight    = 0.30;
    _pkgFixedCostWeight = 0.10;

    logDebug() << "pkgDownloadWeight:  " << _pkgDownloadWeight  << Qt::endl;
    logDebug() << "pkgActionWeight:    " << _pkgActionWeight    << Qt::endl;
    logDebug() << "pkgFixedCostWeight: " << _pkgFixedCostWeight << Qt::endl;
}


int PkgCommitPage::currentProgressPercent()
{
    float downloadPercent  = 0.0;
    float installedPercent = 0.0;
    float tasksPercent     = 0.0;
    float percent          = 0.0;

    //
    // Download %
    //

    if ( _totalDownloadSize > 0 )
    {
        ByteCount downloadSize = _completedDownloadSize
            + pkgTasks()->downloads().downloadSizeSum();

        percent = ( 100.0 * downloadSize ) / _totalDownloadSize;
    }
    else // no download needed?
    {
        percent = 100.0; // download is 100% completed
    }

    downloadPercent = percent * _pkgDownloadWeight;

#if VERBOSE_PROGRESS

    logVerbose() << "Download  %: "  << downloadPercent
                 << "  weight: "     << _pkgDownloadWeight
                 << "  raw %: "      << percent
                 << Qt::endl;
#endif

    //
    // Installed / removed size %
    //

    if ( _totalInstalledSize > 0 )  // Prevent division by zero
    {
        ByteCount installedSize = _completedInstalledSize
            + pkgTasks()->doing().installedSizeSum();

        percent          = ( 100.0 * installedSize ) / _totalInstalledSize;
        installedPercent = percent * _pkgActionWeight;

#if VERBOSE_PROGRESS

        logVerbose() << "Installed %: " << installedPercent
                     << "  weight: "    << _pkgActionWeight
                     << "  raw %: "     << percent
                     << Qt::endl;
#endif
    }


    //
    // Number of tasks %
    //

    if ( _totalTasksCount > 0 )  // Prevent division by zero
    {
        percent      = ( 100.0 * _completedTasksCount ) / _totalTasksCount;
        tasksPercent = percent * _pkgFixedCostWeight;

#if VERBOSE_PROGRESS

        logVerbose() << "Tasks     %: " << tasksPercent
                     << "  weight: "    << _pkgFixedCostWeight
                     << "  raw %: "     << percent
                     << Qt::endl;
#endif
    }


    //
    // Total progress
    //

    float progress   = tasksPercent + downloadPercent + installedPercent;

#if VERBOSE_PROGRESS
    logVerbose() << "Progress: " << progress << "%" << Qt::endl;
#endif

    return qBound( 0, (int) ( progress + 0.5 ), 100 );
}


bool PkgCommitPage::updateTotalProgressBar()
{
    bool didUpdate   = false;
    int  oldProgress = _ui->totalProgressBar->value();
    int  progress    = currentProgressPercent();

    if ( progress >= 0 && progress > oldProgress )
    {
#if VERBOSE_PROGRESS
        logVerbose() << "Updating with " << progress << "%" << Qt::endl;
#endif
        _ui->totalProgressBar->setValue( progress );
        didUpdate = true;
    }

    return didUpdate;
}


//----------------------------------------------------------------------

//
// PkgCommitCallback slots
//

void PkgCommitPage::pkgDownloadStart( ZyppRes zyppRes )
{
    CHECK_PTR( zyppRes );
    PkgTask * task = pkgTasks()->todo().find( zyppRes );

    if ( ! task )
    {
        logError() << "Can't find task for " << zyppRes << " in todo" << Qt::endl;
        return;
    }

#if VERBOSE_TRANSACT
    logVerbose() << task << Qt::endl;
#endif

    // Move the task from the todo list to the downloads list

    PkgTasks::moveTask( task, pkgTasks()->todo(), pkgTasks()->downloads() );
    task->setDownloadedPercent( 0 ); // Just to make sure

    // Move the task from the todo list widget to the downloads list widget

    _ui->todoList->removeTaskItem( task );
    PkgTaskListWidgetItem * item = _ui->downloadsList->addTaskItem( task );
    item->setIcon( _downloadOngoingIcon );

    processEvents(); // Update the UI
}


void PkgCommitPage::pkgDownloadProgress( ZyppRes zyppRes, int percent )
{
    // Avoid unnecessary expensive progress updates:
    //
    // There is nothing to report for 0%, and pkgDownloadEnd() will take care
    // of 100%.

    if ( percent < 1 || percent > 99 )
        return;

    CHECK_PTR( zyppRes );
    PkgTask * task = pkgTasks()->downloads().find( zyppRes );

    if ( ! task )
    {
        logError() << "Can't find task for " << zyppRes << " in downloads" << Qt::endl;
        return;
    }

#if VERBOSE_PROGRESS
    logVerbose() << task << ": downloaded " << percent << "%" << Qt::endl;
#endif

    if ( percent != task->downloadedPercent() ) // only if there really was a change
    {
        task->setDownloadedPercent( percent );

        // Update the UI

        if ( updateTotalProgressBar() ) // This is somewhat expensive
            processEvents();
    }
}


void PkgCommitPage::pkgDownloadEnd( ZyppRes zyppRes )
{
    CHECK_PTR( zyppRes );
    PkgTask * task = pkgTasks()->downloads().find( zyppRes );

    if ( ! task )
    {
        logError() << "Can't find task for " << zyppRes << " in downloads" << Qt::endl;
        return;
    }

#if VERBOSE_TRANSACT
    logVerbose() << task << Qt::endl;
#endif

    task->setDownloadedPercent( 100 );
    PkgTaskListWidgetItem * item = _ui->downloadsList->findTaskItem( task );

    if ( item )
    {
        item->setIcon( _downloadDoneIcon );
        processEvents();
    }

    // Important: Not adding the download size to _completedDownloadSize just
    // yet, or it would be counted twice while the task is still in the doing
    // list. That has to wait until it is moved to the doing list.
}


void PkgCommitPage::pkgCachedNotify( ZyppRes zyppRes )
{
    CHECK_PTR( zyppRes );
    PkgTask * task = pkgTasks()->todo().find( zyppRes );

    if ( ! task )
    {
        logError() << "Can't find task for " << zyppRes << " in todo" << Qt::endl;
        return;
    }

#if VERBOSE_TRANSACT
    logVerbose() << task << Qt::endl;
#endif

    // Move the task from the todo list to the downloads list

    PkgTasks::moveTask( task, pkgTasks()->todo(), pkgTasks()->downloads() );
    task->setDownloadedPercent( 100 );

    // Move the task from the todo list widget to the downloads list widget

    _ui->todoList->removeTaskItem( task );
    PkgTaskListWidgetItem * item = _ui->downloadsList->addTaskItem( task );
    item->setIcon( _downloadDoneIcon );

    processEvents(); // Update the UI


    // Important: Not adding the download size to _completedDownloadSize just
    // yet, or it would be counted twice while the task is still in the doing
    // list. That has to wait until it is moved to the doing list.
}


void PkgCommitPage::pkgDownloadError( ZyppRes zyppRes, const QString & errorMsg )
{
    pkgActionError( zyppRes, errorMsg,
                    _( "Error while downloading package %1:" ),
                    __FUNCTION__ );
}


//----------------------------------------------------------------------


void PkgCommitPage::pkgInstallStart( ZyppRes zyppRes )
{
    // While packages are being downloaded, the list always scrolls to the
    // bottom, so the list appears to scroll like a text terminal as new output
    // appears.
    //
    // But when we are installing the first package, the downloads phase is
    // probably over, so scroll the downloads list widget to the top so the
    // user can now see the packages that are probably installed next and how
    // they move from the downloads list to the doing list.
    //
    // But only do that once: The user might choose to scroll the list
    // manually, and we don't want to fight the user's actions every few
    // seconds.

    if ( ! _startedInstallingPkg )
    {
        _startedInstallingPkg = true;
        _ui->downloadsList->scrollToTop();
    }

    pkgActionStart( zyppRes, PkgInstall, __FUNCTION__ );
}


void PkgCommitPage::pkgInstallProgress( ZyppRes zyppRes, int percent )
{
    pkgActionProgress( zyppRes, percent, PkgInstall, __FUNCTION__ );
}


void PkgCommitPage::pkgInstallEnd( ZyppRes zyppRes )
{
    pkgActionEnd( zyppRes, PkgInstall, __FUNCTION__ );
}


void PkgCommitPage::pkgInstallError( ZyppRes zyppRes, const QString & errorMsg )
{
    pkgActionError( zyppRes, errorMsg,
                    _( "Error installing package %1:" ),
                    __FUNCTION__ );
}


//----------------------------------------------------------------------


void PkgCommitPage::pkgRemoveStart( ZyppRes zyppRes )
{
    pkgActionStart( zyppRes, PkgRemove, __FUNCTION__ );
}


void PkgCommitPage::pkgRemoveProgress( ZyppRes zyppRes, int percent )
{
    pkgActionProgress( zyppRes, percent, PkgRemove, __FUNCTION__ );
}


void PkgCommitPage::pkgRemoveEnd( ZyppRes zyppRes )
{
    pkgActionEnd( zyppRes, PkgRemove, __FUNCTION__ );
}


void PkgCommitPage::pkgRemoveError( ZyppRes zyppRes, const QString & errorMsg )
{
    pkgActionError( zyppRes, errorMsg,
                    _( "Error installing package %1:" ),
                    __FUNCTION__ );
}


//----------------------------------------------------------------------


void PkgCommitPage::pkgActionStart( ZyppRes       zyppRes,
                                    PkgTaskAction action,
                                    const char *  caller )
{
    CHECK_PTR( zyppRes );
    PkgTask * task = 0;

    if ( action & PkgAdd ) // PkgInstall | PkgUpdate
    {
        task = pkgTasks()->downloads().find( zyppRes );

        if ( task )
        {
            // Move the task from the downloads list to the doing list
            PkgTasks::moveTask( task, pkgTasks()->downloads(), pkgTasks()->doing() );

            // Move the task from the downloads list widget to the doing list widget

            _ui->downloadsList->removeTaskItem( task );
            _ui->doingList->addTaskItem( task );

            // Update the bookkeeping sums.
            // We already know that the task was in the downloads list.

            if ( task->downloadSize() > 0 )
                _completedDownloadSize += task->downloadSize();
        }
    }

    if ( ! task ) // PkgRemove or no download needed
    {
        task = pkgTasks()->todo().find( zyppRes );

        if ( ! task )
        {
            logError() << caller << "(): "
                       << "Can't find task for " << zyppRes
                       << " in either downloads or todo" << Qt::endl;
            return;
        }

        // Move the task from the todo list to the doing list
        PkgTasks::moveTask( task, pkgTasks()->todo(), pkgTasks()->doing() );

        // Move the task from the todo list widget to the doing list widget

        _ui->todoList->removeTaskItem( task );
        _ui->doingList->addTaskItem( task );
    }

#if VERBOSE_TRANSACT
    logVerbose() << task << Qt::endl;
#endif

    task->setDownloadedPercent( 100 ); // The download is complete for sure
    task->setCompletedPercent( 0 );    // But the task itself isn't completed
    processEvents(); // Update the UI

    // No
    //
    //  _completedDownloadSize += task->downloadSize()
    //
    // here while the task is in the doing list, otherwise it would be
    // summed up twice!
}


void PkgCommitPage::pkgActionProgress( ZyppRes       zyppRes,
                                       int           percent,
                                       PkgTaskAction action,
                                       const char *  caller )
{
    Q_UNUSED( action );

    // Avoid an unreasonable number of expensive progress updates:
    //
    // Every 5% (i.e. 20 times per package) is more than enough, and libzypp
    // tends to send bogus 100% progress reports e.g. for PkgRemove.
    // pkgActionEnd() will take care of a finished package anyway, no need to
    // bother with 100% here.
    //
    // And since we have one single progress bar for the total progress,
    // reporting 0% for a package would not make any sense either.

    if ( percent % 5 != 0 || percent <= 0 || percent >= 100 )
        return;

    CHECK_PTR( zyppRes );
    PkgTask * task = pkgTasks()->doing().find( zyppRes );

    if ( ! task )
    {
        logError() << caller << "(): "
                   << "Can't find task for "
                   << zyppRes << " in doing" << Qt::endl;
        return;
    }

#if VERBOSE_PROGRESS
    logVerbose() << task << ": " << percent << "%" << Qt::endl;
#endif

    if ( percent != task->completedPercent() )
    {
        task->setCompletedPercent( percent );

        // Update the UI

        if ( updateTotalProgressBar() ) // This is somewhat expensive
            processEvents();
    }
}


void PkgCommitPage::pkgActionEnd( ZyppRes       zyppRes,
                                  PkgTaskAction action,
                                  const char *  caller )
{
    CHECK_PTR( zyppRes );

    PkgTask * task = pkgTasks()->doing().find( zyppRes );

    if ( ! task )
    {
        logError() << caller << "(): "
                   << "Can't find task for " << zyppRes
                   << " in doing" << Qt::endl;
        return;
    }

#if VERBOSE_TRANSACT
    logVerbose() << task << Qt::endl;
#endif


    // Move the task from the doing list to the done list

    PkgTasks::moveTask( task, pkgTasks()->doing(), pkgTasks()->done() );
    task->setDownloadedPercent( 100 );
    task->setCompletedPercent( 100 ); // Just to make sure


    // Move the task from the doing list widget to the done list widget

    _ui->doingList->removeTaskItem( task );
    _ui->doneList->addTaskItem( task );


    // Update the internal bookkeeping sums

    ++_completedTasksCount;

    if ( task->installedSize() > 0 )
        _completedInstalledSize += task->installedSize();

    // The task's download size has already be added to _completeDownloadSize
    // when the task was moved from the downloads list to the doing list.


    // Update the UI

    updateTotalProgressBar(); // This may or may not be needed
    processEvents();          // But this is needed for sure
}


void PkgCommitPage::pkgActionError( ZyppRes         zyppRes,
                                    const QString & errorMsg,
                                    const QString & msgHeader,
                                    const char *    caller     )
{
    CHECK_PTR( zyppRes );

    logError() << caller << "(): " << zyppRes << ": " << errorMsg << Qt::endl;

    QString msg;

    if ( msgHeader.contains( "%1" ) )
    {
        msg = msgHeader.arg( fromUTF8( zyppRes->name() ) );
        msg += "\n\n";
    }

    msg += errorMsg;

    int result = QMessageBox::warning( this, // parent widget
                                       "",   // Window title
                                       msg,
                                       _( "&Abort" ), _( "&Retry" ), _( "&Ignore" ),
                                       0,   // defaultButtonNumber (from 0)
                                       1 ); // escapeButtonNumber

    // Using the PkgCommitSignalForwarder::setReply() kludge since we can't
    // simply return a value from a Qt slot. But this only works with direct Qt
    // signal / slot connections: When the signal is emitted, the connected
    // slots are called immediately one after the other, and control flow only
    // returns to the emitter when the last one returns.
    //
    // That makes it safe to open the pop-up warning dialog and wait until the
    // user clicks on an answer button, and to set the reply before we return
    // here. None of this would work with multiple Qt threads.

    PkgCommitSignalForwarder::instance()->setReply( (ErrorReply) result );
}


//----------------------------------------------------------------------


QTextStream & operator<<( QTextStream & str, ZyppRes zyppRes )
{
    if ( zyppRes )
        str << fromUTF8( zyppRes->name() );
    else
        str << "<NULL ZyppRes>";

    return str;
}
