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


#ifndef PkgCommitPage_h
#define PkgCommitPage_h


#include <QStringList>
#include <QWidget>

#include <zypp/ZYppCommitPolicy.h>

#include "YQZypp.h"     // ZyppRes
#include "LogStream.h"  // operator<<()


// Generated with 'uic' from a Qt designer .ui form: pkg-commit.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_pkg-commit-page.h
// for the variable names of the widgets.

#include "ui_pkg-commit-page.h"


class ProgressDialog;
using zypp::ByteCount;


/**
 * Class for the "package commit" workflow step / page:
 *
 * Start the pending package transactions that have been collected in the
 * package selector; start installing / updating / removing packages.
 *
 * This is mostly a passive workflow step / page; it triggers the package
 * transactions in libzypp and displays the progress, mostly in a large
 * progress bar that visualizes the total progress of all the operations
 * combined, i.e. all the package downloads, installations, updates, removals,
 * and the pre- and post-installation (etc.) scripts.
 *
 * There is also one button that is active all the time to cancel / abort the
 * package transactions in emergency situations (when they are stuck or take a
 * very long time because of network problems).
 *
 * In a "detail" mode, this may also show more information about the packages
 * that are still to do and those that are done, and those that are in "doing",
 * i.e. that are currently actively being downloaded and installed / updated /
 * removed.
 *
 * libzypp may choose to do multiple operations in parallel, so more than one
 * package might be in "doing".
 *
 * This whole class relies heavily on libzypp callbacks reporting progress and
 * possible problems.
 **/
class PkgCommitPage: public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * This just creates the widget tree for the page; it doesn't start the
     * package transactions yet. Use commit() for that.
     **/
    PkgCommitPage( QWidget * parent = 0 );

    /**
     * Destructor.
     **/
    virtual ~PkgCommitPage();

    /**
     * Reset the widgets and the internal transaction status:
     * Set the progress back to 0 and clear any previous list content,
     **/
    void reset();

    /**
     * Start the package transactions.
     *
     * The passes control mostly to libzypp which has to call callbacks to
     * update the widgets reporting the progress. The callbacks will also do a
     * little Qt event processing (some milliseconds) to give the user a chance
     * to use the "Cancel" button or other interactive widgets.
     *
     * When this function returns (which will take a while), all the package
     * transactions should be done, or there was an unrecoverable error.
     **/
    void commit();

    /**
     * Return 'true' if the "Summary" page should be shown.
     **/
    bool showSummaryPage() const;

    /**
     * Return the UI (the widget tree) of this page.
     **/
    Ui::PkgCommitPage * ui() const { return _ui; }

    /**
     * Process the pending Qt events.
     *
     * This is necessary in callbacks to receive user events (e.g. button
     * clicks) and to update the display.
     *
     * DO NOT use MainWindow::processEvents() instead which might ignore user
     * input events (e.g. clicks on buttons).
     **/
    static void processEvents();

    /**
     * Return the instance of this class or 0 if there is none.
     *
     * This is not a singleton, but in each normal application there is no more
     * than one instance, and during the lifetime of any widgets inside the
     * MainWindow it is safe to assume that the MainWindow also exists.
     **/
    static PkgCommitPage * instance() { return _instance; }


public slots:

    /**
     * Cancel the package transactions.
     * This will ask for confirmation first.
     **/
    void cancelCommit();

    /**
     * WM_CLOSE: ALT-F4 or the close window [x] icon in the window decoration.
     * Cancel the package transactions and quit.
     * This will ask for confirmation first.
     **/
    void wmClose();


signals:

    /**
     * Emitted when the transactions are finished (successfully or with an
     * error), and the next workflow page should be displayed.
     *
     * Notice that there is intentionally no 'back()' counterpart.
     **/
    void next();

    /**
     * Emitted when the user clicks the "Cancel" button below the total
     * progress bar and confirms the "Really cancel" pop-up.
     **/
    void abortCommit();


public slots:

    //
    // PkgCommitCallback slots
    //

    void pkgDownloadStart    ( ZyppRes zyppRes );
    void pkgDownloadProgress ( ZyppRes zyppRes, int value );
    void pkgDownloadEnd      ( ZyppRes zyppRes );

    void pkgCachedNotify     ( ZyppRes zyppRes );
    void pkgDownloadError    ( ZyppRes zyppRes, const QString & msg );


    void pkgInstallStart     ( ZyppRes zyppRes );
    void pkgInstallProgress  ( ZyppRes zyppRes, int value );
    void pkgInstallEnd       ( ZyppRes zyppRes );
    void pkgInstallError     ( ZyppRes zyppRes, const QString & msg );

    void pkgRemoveStart      ( ZyppRes zyppRes );
    void pkgRemoveProgress   ( ZyppRes zyppRes, int value );
    void pkgRemoveEnd        ( ZyppRes zyppRes );
    void pkgRemoveError      ( ZyppRes zyppRes, const QString & msg );

    void fileConflictsCheckStart();
    void fileConflictsCheckProgress( int percent );
    void fileConflictsCheckResult  ( const QStringList & conflicts );


protected slots:

    /**
     * Switch displaying details on or off.
     **/
    void toggleDetails();


protected:

    /**
     * The real package commit: Tell libzypp to start downloading and
     * installing / updating / removing packages.
     *
     * See also the '--dry-run' and '--download-only' command line options.
     **/
    void realCommit();

    /**
     * A visual fake for committing packages: Just moving the progress bar
     * ahead, not actually installing packages.
     *
     * Use the '--fake-commit' command line option to trigger this rather than
     * the real commit.
     **/
    void fakeCommit();

    /**
     * Return a commit policy based on the app's options.
     **/
    zypp::ZYppCommitPolicy commitPolicy() const;

    /**
     * Fill the list widgets with content from the app's pkgTasks() lists.
     **/
    void populateLists();

    /**
     * Update the header labels of the task list widgets with the number of
     * items
     **/
    void updateListHeaders();

    /**
     * Update the header label of one task list widget with the number of its
     * items
     **/
    void updateListHeader( QLabel *        header,
                           const QString & rawMsg,
                           int             size );

    /**
     * Set up the internal Qt signal / slot connections.
     **/
    void connectWidgets();

    /**
     * Load the icons for this page.
     **/
    void loadIcons();

    /**
     * Ask the user for confirmation if the package transactions should really
     * be cancelled. Return 'true' if the user confirms, 'false' if not.
     **/
    bool askForCancelCommitConfirmation();

    /**
     * Return 'true' if details are currently shown, 'false' if not.
     **/
    bool showDetails() const { return _showDetails; }

    /**
     * Update the label of the details button between "Show Details" and "Hide
     * Details".
     **/
    void updateDetailsButton();

    /**
     * Read settings for this widget from the configuration file.
     **/
    void readSettings();

    /**
     * Write the settings of this widget to the configuration file.
     **/
    void writeSettings();

    /**
     * Get the package tasks from the application.
     **/
    PkgTasks * pkgTasks();

    /**
     * Initialize and calculate the values for the overall progress.
     **/
    void initProgressData();

    /**
     * Calculate the current progress percent based on the weighted progress
     * percent of number of completed tasks, completed download size, completed
     * install size.
     **/
    int currentProgressPercent();

    /**
     * Calculate the total progress and update the total progress bar if the
     * (integer) percent value is different from the old one.
     *
     * Return 'true' if there was a difference, 'false' if not. This return
     * value can be used to determine if actively processing events is needed
     * after this.
     **/
    bool updateTotalProgressBar();

    /**
     * Return the (non-modal!) file conflicts check progress dialog. Create it
     * if it doesn't exist yet.
     *
     * Not to confuse with the total progress bar of the commit page.
     **/
    ProgressDialog * fileConflictsProgressDialog();

    /**
     * Close the file conflicts check dialog and delete it.
     * Do nothing if it was never created.
     **/
    void closeFileConflictsProgressDialog();

    /**
     * The common part of pkgInstallStart() and pkgRemoveStart() /
     * ...progress(), ...End(), ...Error().
     *
     * 'action' is one of PkgInstall or PkgRemove,
     * 'caller' is the calling function (__FUNCTION__) for logging.
     **/
    void pkgActionStart   ( ZyppRes         zyppRes,
                            PkgTaskAction   action,
                            const char *    caller );

    void pkgActionProgress( ZyppRes         zyppRes,
                            int             percent,
                            PkgTaskAction   action,
                            const char *    caller );

    void pkgActionEnd     ( ZyppRes         zyppRes,
                            PkgTaskAction   action,
                            const char *    caller );

    void pkgActionError   ( ZyppRes         zyppRes,
                            const QString & errorMsg,
                            const QString & msgHeader,
                            const char *    caller );

    //
    // Data members
    //

    Ui::PkgCommitPage * _ui;
    PkgTasks *          _pkgTasks;
    bool                _showDetails;
    bool                _startedInstallingPkg;
    ProgressDialog *    _fileConflictsProgressDialog;

    ByteCount           _totalDownloadSize;
    ByteCount           _totalInstalledSize;
    int                 _totalTasksCount;

    ByteCount           _completedDownloadSize;
    ByteCount           _completedInstalledSize;
    int                 _completedTasksCount;

    float               _pkgFixedCostWeight; // 0.0 .. 1.0
    float               _pkgDownloadWeight;  // 0.0 .. 1.0
    float               _pkgActionWeight;    // 0.0 .. 1.0

    QPixmap             _downloadOngoingIcon;
    QPixmap             _downloadDoneIcon;

    static PkgCommitPage * _instance;
};


LogStream & operator<<( LogStream & str, ZyppRes zyppRes );



#endif  // PkgCommitPage_h

