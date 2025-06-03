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


#ifndef YQPkgSelectorBase_h
#define YQPkgSelectorBase_h

#include <QFrame>

#include "YQZypp.h"


class QObject;
class QWidget;
class QAction;

class YQPkgConflictDialog;
class YQPkgDiskUsageList;


/**
 * Abstract base class for package selectors.
 **/
class YQPkgSelectorBase: public QFrame
{
    Q_OBJECT

protected:

    /**
     * Constructor
     *
     * Will initialize package and selection managers and create conflict
     * dialogs.
     **/
    YQPkgSelectorBase( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgSelectorBase();


public slots:

    /**
     * Reset the resolver, in particular modes like "upgrade mode" (after a
     * dist upgrade) or "update mode" after a package update).
     *
     * This is included in 'reset()'.
     **/
    void resetResolver();


    /**
     * Resolve dependencies (unconditionally) for all resolvables.
     *
     * Returns QDialog::Accepted or QDialog::Rejected.
     **/
    int resolveDependencies();

    /**
     * Verifies dependencies of the currently installed system.
     *
     * Returns QDialog::Accepted or QDialog::Rejected.
     **/
    int verifySystem();

    /**
     * Check for disk overflow and post a warning dialog if necessary.
     * The user can choose to override this warning.
     *
     * Returns QDialog::Accepted if no warning is necessary or if the user
     * wishes to override the warning, QDialog::Rejected otherwise.
     **/
    int checkDiskUsage();

    /**
     * Display a list of automatically selected packages
     * (excluding packages contained in any selections that are to be installed)
     **/
    void showAutoPkgList();

    /**
     * Close processing and abandon changes.
     * If there were changes, this will post an "Abandon all changes?" pop-up.
     **/
    void reject();

    /**
     * WM_CLOSE: ALT-F4 or the close window [x] icon in the window decoration.
     **/
    void wmClose() { reject(); }

    /**
     * Close processing and accept changes
     **/
    void accept();

    /**
     * Return 'true' if there are any pending changes,
     * i.e. package transactions that need to be committed.
     **/
    bool pendingChanges();

    /**
     * Inform user about a feature that is not implemented yet.
     * This should NEVER show up in the final version.
     **/
    void notImplemented();


signals:

    /**
     * Emitted when package resolving is started.
     * This can be used for some kind of "busy display".
     **/
    void resolvingStarted();

    /**
     * Emitted when package resolving is finished.
     **/
    void resolvingFinished();

    /**
     * Emitted when the user accepted and solved all pending dependency
     * problems: Commit the package transactions.
     **/
    void commit();

    /**
     * Emitted when the user accepted, and there is nothing to be done:
     * No package actions to commit.
     **/
    void finished();

    /**
     * Emitted when all data of connected widgets should be reset.
     *
     * Do NOT connect this to the filter views' filter() slot:
     * The YQPkgSelector already does that with
     * YQPkgFilteTab::reloadCurrentPage(). This resetNotify() signal is
     * intended for other widgets / QObjects that don't fit into this schema
     * like the YQPkgSelMapper.
     **/
    void resetNotify();


protected:

    /**
     * Return 'true' if the "Automatic Changes" dialog should be shown upon
     * accepting the changes.
     **/
    virtual bool showAutoChangesDialog() { return false; }

    /**
     * Show all license agreements the user has not confirmed yet
     * (for all packages that will be installed, and in YOU mode also for
     * patches).
     *
     * Returns 'true' if all licenses were confirmed, 'false' if one or more
     * licenses were not confirmed (in which case some packages might be set to
     * S_TABOO, which might require another resolver run).
     **/
    bool showPendingLicenseAgreements();

    /**
     * Show all license agreements in a resolvable range. To be used with
     * zyppPkgBegin() and zyppPkgEnd() or with zyppPatchesBegin() and
     * zyppPatchesEnd().
     **/
    bool showPendingLicenseAgreements( ZyppPoolIterator begin,
                                       ZyppPoolIterator end );

    /**
     * Event handler for keyboard input - for debugging and testing.
     *
     * Reimplemented from QWidget.
     **/
    virtual void keyPressEvent( QKeyEvent * event ) override;

    /**
     * Event handler for WM_CLOSE (Alt-F4).
     *
     * Reimplemented from QWidget.
     **/
    virtual void closeEvent( QCloseEvent * event ) override;


    // Data members

    bool                  _blockResolver;
    YQPkgConflictDialog * _pkgConflictDialog;
    YQPkgDiskUsageList *  _diskUsageList;
};


#endif // YQPkgSelectorBase_h
