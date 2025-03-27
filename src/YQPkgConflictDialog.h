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

    Textdomain "qt-pkg"
 */


#ifndef YQPkgConflictDialog_h
#define YQPkgConflictDialog_h


#include <QDialog>

class YQPkgConflictList;
class QMenu;


/**
 * Dialog that takes care of dependency checking and resolving
 * conflicts.
 **/
class YQPkgConflictDialog: public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor. Does not pop up a dialog yet.
     * Use solveAndShowConflicts().
     **/
    YQPkgConflictDialog( QWidget * parent );

    /**
     * Destructor.
     **/
    virtual ~YQPkgConflictDialog();

    /**
     * Return the first instance of this class or 0 if there is none.
     *
     * This is not a singleton class, but in normal applications exactly one
     * instance is created by a high-level object such as YQPkgSelector, and
     * that instance is kept until the package selection process is over.
     *
     * Use this to access high-level methods such as
     *   - solveAndShowConflicts()
     *   - verifySystem()
     *   - doPackageUpdate()
     *   - doDistUpgrade()
     **/
    static YQPkgConflictDialog * instance() { return _instance; }

    /**
     * Reimplemented from QWidget:
     * Reserve a reasonable amount of space.
     **/
    virtual QSize sizeHint() const override;

    /**
     * Reset all previously ignored dependency problems.
     **/
    static void resetIgnoredDependencyProblems();


public slots:

    /**
     * Run the package dependency solver for the current package set and open
     * the conflict dialog if there are any conflicts.
     * Returns only when the conflict dialog is closed.
     *
     * Returns QDialog::Accepted or QDialog::Rejected.
     **/
    int solveAndShowConflicts();

    /**
     * Run the package dependency solver for the currently installed system
     * plus the packages that are marked for installation (or update or...) and
     * open the conflict dialog if there are any conflicts.
     * Returns only when the conflict dialog is closed.
     *
     * Returns QDialog::Accepted or QDialog::Rejected.
     **/
    int verifySystem( bool showBusyPopup = false );

    /**
     * The same as verifySystem(), but with a busy popup.
     **/
    int verifySystemWithBusyPopup();

    /**
     * Update all installed packages that can be updated without a problem.
     * This is the counterpart to 'zypper up'.
     *
     * Returns QDialog::Accepted or QDialog::Rejected.
     **/
    int doPackageUpdate();

    /**
     * Do a distribution upgrade: Upgrade all installed packages that can be
     * updated without a problem, taking package splits etc. into account.
     *
     * This is the counterpart to 'zypper dup'.
     *
     * Returns QDialog::Accepted or QDialog::Rejected.
     **/
    int doDistUpgrade();

    /**
     * Mini-wizard to generate solver test case:
     *
     *   - Inform user what this is all about
     *   - Tell user where the files will go
     *   - Generate test case
     *   - Ask user if he would like to pack it all into y2logs.tgz
     *   - Prompt user for path for y2logs.tgz
     *   - Save y2logs.tgz
     **/
    void askCreateSolverTestCase();


signals:

    /**
     * Update package states - they may have changed.
     **/
    void updatePackages();


protected:

    /**
     * Initialize solving: Post "busy" popup etc.
     **/
    void prepareSolving();

    /**
     * Process the result of solving: Post conflict dialog, if neccessary.
     * 'success' is the return value of the preceding solver call.
     * Returns either QDialog::Accepted or QDialog::Rejected.
     **/
    int  processSolverResult( bool success );


    //
    // Data members
    //

    YQPkgConflictList * _conflictList;
    QMenu *             _expertMenu;

    static YQPkgConflictDialog * _instance;
    static int                   _resolverRunCount;
};



#endif // ifndef YQPkgConflictDialog_h
