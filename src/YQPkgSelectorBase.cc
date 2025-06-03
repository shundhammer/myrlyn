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


#include <QAction>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QKeyEvent>

#include "LicenseCache.h"
#include "Logger.h"
#include "PkgTasks.h"
#include "QY2CursorHelper.h"
#include "MyrlynApp.h"
#include "YQPkgChangesDialog.h"
#include "YQPkgConflictDialog.h"
#include "YQPkgDiskUsageList.h"
#include "YQPkgDiskUsageWarningDialog.h"
#include "YQPkgObjList.h"
#include "YQi18n.h"

#include "YQPkgSelectorBase.h"

using std::string;


YQPkgSelectorBase::YQPkgSelectorBase( QWidget * parent )
    : QFrame( parent )
    , _blockResolver( true )
{
    _pkgConflictDialog = 0;
    _diskUsageList     = 0;
    _pkgConflictDialog = 0;

    // YQUI::setTextdomain( "qt-pkg" );
    // setFont( YQUI::yqApp()->currentFont() );

    _pkgConflictDialog = new YQPkgConflictDialog( this );
    Q_CHECK_PTR( _pkgConflictDialog );

    zyppPool().saveState<zypp::Package>();
    zyppPool().saveState<zypp::Pattern>();
    zyppPool().saveState<zypp::Patch  >();

    _blockResolver = false;
    logInfo() << "PackageSelectorBase init done" << endl;
}


YQPkgSelectorBase::~YQPkgSelectorBase()
{
    logInfo() << "Destroying PackageSelectorBase" << endl;
}


void YQPkgSelectorBase::resetResolver()
{
    logInfo() << "Resetting resolver" << endl;

    zypp::getZYpp()->resolver()->setUpdateMode ( false ); // No package update  mode
    zypp::getZYpp()->resolver()->setUpgradeMode( false ); // No dist    upgrade mode
}


int YQPkgSelectorBase::resolveDependencies()
{
    if ( _blockResolver )
        return QDialog::Rejected;

    if ( ! _pkgConflictDialog )
    {
        logError() << "No package conflict dialog existing" << endl;
        return QDialog::Rejected;
    }


    busyCursor();
    emit resolvingStarted();

    int result = _pkgConflictDialog->solveAndShowConflicts();

    emit resolvingFinished();
    normalCursor();

    return result;
}


int YQPkgSelectorBase::verifySystem()
{
    if ( ! _pkgConflictDialog )
    {
        logError() << "No package conflict dialog existing" << endl;
        return QDialog::Accepted;
    }


    busyCursor();
    int result = _pkgConflictDialog->verifySystem();
    normalCursor();

    if ( result == QDialog::Accepted )
    {
        QMessageBox::information( this, // parent
                                  "",   // window title
                                  _( "System dependencies verify OK." ) );
    }

    return result;
}


int YQPkgSelectorBase::checkDiskUsage()
{
    if ( ! _diskUsageList )
    {
        return QDialog::Accepted;
    }

    if ( ! _diskUsageList->overflowWarning.inRange() )
        return QDialog::Accepted;

    QString msg =
        // Translators: RichText ( HTML-like ) format
        "<p><b>" + _( "Error: Out of disk space!" ) + "</b></p>"
        + _( "<p>"
             "You can choose to install anyway if you know what you are doing, "
             "but you risk getting a corrupted system that requires manual repairs. "
             "If you are not absolutely sure how to handle such a case, "
             "press <b>Cancel</b> now and deselect some packages."
             "</p>" );

    return YQPkgDiskUsageWarningDialog::diskUsageWarning( msg,
                                                          100, _( "C&ontinue Anyway" ), _( "&Cancel" ) );
}


void YQPkgSelectorBase::showAutoPkgList()
{
    resolveDependencies();

    // Show which packages are installed/deleted
    QString msg =
        // Detailed explanation ( automatic word wrap! )
        + "<p>"
        + _( "The following items will be changed:"
             " " )
        + "<p>";

    YQPkgChangesDialog::showChangesDialog( this,
                                           msg,
                                           _( "&OK" ),
                                           QString(),   // rejectButtonLabel
                                           YQPkgChangesDialog::FilterAutomatic,
                                           YQPkgChangesDialog::OptionNone );    // showIfEmpty
}


bool YQPkgSelectorBase::pendingChanges()
{
    bool changes =
        zyppPool().diffState<zypp::Package  >() ||
        zyppPool().diffState<zypp::Pattern  >() ||
        zyppPool().diffState<zypp::Patch    >();

    if ( changes )
    {
        if ( zyppPool().diffState<zypp::Package>() )
            logInfo() << "diffState() reports changed packages" << endl;

        if ( zyppPool().diffState<zypp::Pattern>() )
            logInfo() << "diffState() reports changed patterns" << endl;

        if ( zyppPool().diffState<zypp::Patch>() )
            logInfo() << "diffState() reports changed patches" << endl;
    }

    return changes;
}


void YQPkgSelectorBase::reject()
{
    bool changes = pendingChanges();
    bool confirm = false;

    if ( ! MyrlynApp::runningAsRoot() && changes )
    {
        logInfo() << "Read-only mode (no root privileges) - abandoning changes" << endl;
        changes = false;
    }

    if ( changes )
    {
        QMessageBox msgBox( window() );
        msgBox.setText( _( "Really abandon all changes and exit?" ) );
        msgBox.setIcon( QMessageBox::Warning );
        msgBox.addButton( QMessageBox::Yes );
        msgBox.addButton( QMessageBox::No  );
        msgBox.setDefaultButton( QMessageBox::No );
        msgBox.exec();

        confirm = ( msgBox.result() == QMessageBox::Yes );
    }

    if ( ! changes || confirm )
    {
        // Really reject

        zyppPool().restoreState<zypp::Package  >();
        zyppPool().restoreState<zypp::Pattern  >();
        zyppPool().restoreState<zypp::Patch    >();

        logInfo() << "Quitting the application" << endl;

        MyrlynApp::instance()->quit();
    }
    else
    {
        logInfo() << "Returning to package selector" << endl;

        // User changed his mind - don't reject
    }
}


void YQPkgSelectorBase::accept()
{
    bool confirmedAllLicenses;

    do
    {
        // Force final dependency resolving
        if ( resolveDependencies() == QDialog::Rejected )
            return;

        confirmedAllLicenses = showPendingLicenseAgreements();

    } while ( ! confirmedAllLicenses ); // Some packages will be set to S_TABOO - need another solver run

    if ( showAutoChangesDialog() )
    {
        // Show which packages are installed/deleted automatically
        QString msg =
            "<p><b>"
            // Dialog header
            + _( "Automatic Changes" )
            + "</b></p>"
            // Detailed explanation ( automatic word wrap! )
            + "<p>"
            + _( "In addition to your manual selections, the following packages"
                 " have been changed to resolve dependencies:" )
            + "<p>";

        if ( YQPkgChangesDialog::showChangesDialog( this,
                                                    msg,
                                                    _( "C&ontinue" ), _( "&Cancel" ),
                                                    YQPkgChangesDialog::FilterAutomatic,
                                                    YQPkgChangesDialog::OptionAutoAcceptIfEmpty )
             == QDialog::Rejected )
        {
            return;
        }
    }

    if ( checkDiskUsage() == QDialog::Rejected )
        return;

    MyrlynApp::instance()->pkgTasks()->initFromZypp();

    if ( ! MyrlynApp::instance()->pkgTasks()->todo().isEmpty()
         || pendingChanges() )
    {
        logInfo() << "Closing PackageSelector with 'commit'" << endl;
        emit commit();
    }
    else
    {
        logInfo() << "No changes - closing PackageSelector with 'finished'" << endl;
        emit finished();
    }
}


bool YQPkgSelectorBase::showPendingLicenseAgreements()
{
    logInfo() << "Showing all pending license agreements" << endl;

    bool allConfirmed = true;

#if 0
    allConfirmed = showPendingLicenseAgreements( zyppPatchesBegin(), zyppPatchesEnd() );
#endif

    allConfirmed = showPendingLicenseAgreements( zyppPkgBegin(), zyppPkgEnd() ) && allConfirmed;

    return allConfirmed;
}


bool YQPkgSelectorBase::showPendingLicenseAgreements( ZyppPoolIterator begin, ZyppPoolIterator end )
{
    bool allConfirmed = true;

    for ( ZyppPoolIterator it = begin; it != end; ++it )
    {
        ZyppSel sel = (*it);

        switch ( sel->status() )
        {
            case S_Install:
            case S_AutoInstall:
            case S_Update:
            case S_AutoUpdate:

                if ( sel->candidateObj() )
                {
                    string licenseText = sel->candidateObj()->licenseToConfirm();

                    if ( ! licenseText.empty() )
                    {
                        if (  sel->hasLicenceConfirmed() )
                        {
                            logInfo() << "Resolvable " << sel->name()
                                      << "'s  license is already confirmed" << endl;
                        }
                        else if ( LicenseCache::confirmed()->contains( licenseText ) )
                        {
                            logInfo() << "License verbatim confirmed before: " << sel->name() << endl;
                        }
                        else
                        {
                            logDebug() << "Showing license agreement for resolvable " << sel->name() << endl;
                            allConfirmed = YQPkgObjListItem::showLicenseAgreement( sel ) && allConfirmed;
                        }
                    }
                }
                break;

            default:
                break;
        }
    }

    return allConfirmed;
}


void YQPkgSelectorBase::notImplemented()
{
    QMessageBox::information( this, // parent
                              "",   // window title
                              _( "Not implemented yet" ) );
}


void YQPkgSelectorBase::closeEvent( QCloseEvent * event )
{
    logInfo() << "Caught WM_CLOSE" << endl;

    event->ignore();
    reject();
}


void YQPkgSelectorBase::keyPressEvent( QKeyEvent * event )
{
    if ( event )
    {
        Qt::KeyboardModifiers specialCombo = ( Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier );

        if ( ( event->modifiers() & specialCombo ) == specialCombo )
        {
            if ( event->key() == Qt::Key_A )
            {
                showAutoPkgList();
                event->accept();
                return;
            }
        }
    }

    QWidget::keyPressEvent( event );
}

