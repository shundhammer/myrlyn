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


#include <QAction>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QKeyEvent>

#include "Exception.h"
#include "Logger.h"
#include "PkgTasks.h"
#include "QY2CursorHelper.h"
#include "QY2LayoutUtils.h"
#include "YQPkgApplication.h"
#include "YQPkgChangesDialog.h"
#include "YQPkgConflictDialog.h"
#include "YQPkgDiskUsageList.h"
#include "YQPkgDiskUsageWarningDialog.h"
#include "YQPkgObjList.h"
#include "YQPkgTextDialog.h"
#include "YQi18n.h"

#include "YQPkgSelectorBase.h"

using std::string;


YQPkgSelectorBase::YQPkgSelectorBase( QWidget * parent,
                                      long      modeFlags )
    : QFrame( parent )
    , _modeFlags( modeFlags )
    , _blockResolver( true )
{
    _showChangesDialog          = false;
    _pkgConflictDialog          = 0;
    _diskUsageList              = 0;
    _pkgConflictDialog          = 0;

    // YQUI::setTextdomain( "qt-pkg" );
    // setFont( YQUI::yqApp()->currentFont() );

    _pkgConflictDialog = new YQPkgConflictDialog( this );
    Q_CHECK_PTR( _pkgConflictDialog );

#if 1
    // FIXME: Move this out to a separate function
    // and change those insane variable names to something reasonable.
    // This code is ugly.

    QString label = _( "Reset &Ignored Dependency Conflicts" );
    _actionResetIgnoredDependencyProblems = new QAction( label, this);
    Q_CHECK_PTR( _actionResetIgnoredDependencyProblems );

    _actionResetIgnoredDependencyProblems->setShortcut((QKeySequence) 0);

    connect( _actionResetIgnoredDependencyProblems, SIGNAL( triggered() ),
             this,                                  SLOT  ( resetIgnoredDependencyProblems() ) );
#endif

    zyppPool().saveState<zypp::Package  >();
    zyppPool().saveState<zypp::Pattern  >();
    zyppPool().saveState<zypp::Patch    >();

    _blockResolver = false;
    logInfo() << "PackageSelectorBase init done" << Qt::endl;
}


YQPkgSelectorBase::~YQPkgSelectorBase()
{
    logInfo() << "Destroying PackageSelectorBase" << Qt::endl;
}


void YQPkgSelectorBase::reset()
{
    logDebug() << "Sending resetNotify()" << Qt::endl;

    emit resetNotify();
}


int YQPkgSelectorBase::resolveDependencies()
{
    if ( _blockResolver )
        return QDialog::Rejected;

    if ( ! _pkgConflictDialog )
    {
<<<<<<< HEAD
        logError() << "No package conflict dialog existing" << endl;
        return QDialog::Rejected;
=======
        logError() << "No package conflict dialog existing" << Qt::endl;
        return QDialog::Accepted;
>>>>>>> 1561324 (Qualify endl by using Qt::endl)
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
        logError() << "No package conflict dialog existing" << Qt::endl;
        return QDialog::Accepted;
    }


    busyCursor();
    int result = _pkgConflictDialog->verifySystem();
    normalCursor();

    if ( result == QDialog::Accepted )
    {
        QMessageBox::information( this, "",
                                  _( "System dependencies verify OK." ),
                                  QMessageBox::Ok );
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
            logInfo() << "diffState() reports changed packages" << Qt::endl;

        if ( zyppPool().diffState<zypp::Pattern>() )
            logInfo() << "diffState() reports changed patterns" << Qt::endl;

        if ( zyppPool().diffState<zypp::Patch>() )
            logInfo() << "diffState() reports changed patches" << Qt::endl;
    }

    return changes;
}


void YQPkgSelectorBase::reject()
{
    bool changes = pendingChanges();
    bool confirm = false;

    if ( ! YQPkgApplication::runningAsRoot() && changes )
    {
        logInfo() << "Read-only mode (no root privileges) - abandoning changes" << Qt::endl;
        changes = false;
    }

    if ( changes )
    {
        int result =
            QMessageBox::warning( this, "",
                                  _( "Really abandon all changes and exit?" ),
                                  _( "&Yes" ), _( "&No" ), "",
                                  1,   // defaultButtonNumber (from 0)
                                  1 ); // escapeButtonNumber

        confirm = ( result == 0 );
    }

    if ( ! changes || confirm )
    {
        // Really reject

        zyppPool().restoreState<zypp::Package  >();
        zyppPool().restoreState<zypp::Pattern  >();
        zyppPool().restoreState<zypp::Patch    >();

        logInfo() << "Quitting the application" << Qt::endl;

        YQPkgApplication::instance()->quit();
    }
    else
    {
        logInfo() << "Returning to package selector" << Qt::endl;

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

    if ( _showChangesDialog )
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

    if ( confirmUnsupported() )
    {
        logInfo() << "Confirm unsupported packages enabled." << Qt::endl;
        // Show which packages are unsupported

        QString msg =
            "<p><b>"
            // Dialog header
            + _( "Unsupported Packages" )
            + "</b></p>"
            // Detailed explanation ( automatic word wrap! )
            + "<p>"
            + _( "Please realize that the following selected software is either unsupported or"
                 " requires an additional customer contract for support." )
            + "<p>";

        if ( YQPkgUnsupportedPackagesDialog::showUnsupportedPackagesDialog( this,
                                                                            msg,
                                                                            _( "C&ontinue" ), _( "&Cancel" ),
                                                                            YQPkgChangesDialog::FilterUser,
                                                                            YQPkgChangesDialog::OptionAutoAcceptIfEmpty )
             == QDialog::Rejected )
        {
            return;
        }
    }


    // Check disk usage
    if ( checkDiskUsage() == QDialog::Rejected )
        return;

    YQPkgApplication::instance()->pkgTasks()->initFromZypp();

    if ( ! YQPkgApplication::instance()->pkgTasks()->todo().isEmpty()
         || pendingChanges() )
    {
        logInfo() << "Closing PackageSelector with 'commit'" << Qt::endl;
        emit commit();
    }
    else
    {
        logInfo() << "No changes - closing PackageSelector with 'finished'" << Qt::endl;
        emit finished();
    }
}


bool YQPkgSelectorBase::showPendingLicenseAgreements()
{
    logInfo() << "Showing all pending license agreements" << Qt::endl;

    bool allConfirmed = true;

    if ( onlineUpdateMode() )
        allConfirmed = showPendingLicenseAgreements( zyppPatchesBegin(), zyppPatchesEnd() );

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
                        logInfo() << "Resolvable " << sel->name() << " has a license agreement" << Qt::endl;

                        if( ! sel->hasLicenceConfirmed() )
                        {
                            logDebug() << "Showing license agreement for resolvable " << sel->name() << Qt::endl;
                            allConfirmed = YQPkgObjListItem::showLicenseAgreement( sel ) && allConfirmed;
                        }
                        else
                        {
                            logInfo() << "Resolvable " << sel->name()
                                      << "'s  license is already confirmed" << Qt::endl;
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
    QMessageBox::information( this, "",
                              _( "Not implemented yet" ),
                              QMessageBox::Ok );
}


void YQPkgSelectorBase::resetIgnoredDependencyProblems()
{
    YQPkgConflictDialog::resetIgnoredDependencyProblems();
}


void YQPkgSelectorBase::closeEvent( QCloseEvent * event )
{
    logInfo() << "Caught WM_CLOSE" << Qt::endl;

    event->ignore();
    reject();
}


void YQPkgSelectorBase::keyPressEvent( QKeyEvent * event )
{
    if ( event )
    {
        Qt::KeyboardModifiers special_combo = ( Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier );

        if ( ( event->modifiers() & special_combo ) == special_combo )
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

