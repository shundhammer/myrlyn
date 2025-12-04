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


#ifndef MyrlynApp_h
#define MyrlynApp_h

#include <unistd.h>     // geteuid()

#include <QObject>
#include <QFont>
#include <QFlags>


class MainWindow;
class PkgCommitPage;
class PkgTasks;
class QWidget;
class SummaryPage;
class Workflow;
class YQPkgSelector;
class MyrlynRepoManager;
class ZyppLogger;


enum MyrlynAppOption
{
    OptNone             = 0,
    OptReadOnly         = 0x01,
    OptDryRun           = 0x02,
    OptDownloadOnly     = 0x04,
    OptNoRepoRefresh    = 0x08,
    OptForceServiceView = 0x10,

    // For debugging

    OptFakeRoot         = 0x0100,
    OptFakeCommit       = 0x0200,
    OptFakeSummary      = 0x0400,
    OptFakeTranslations = 0x0800,  // "xixoxixoxixo" everywhere
    OptSlowRepoRefresh  = 0x1000
};

// See https://doc.qt.io/qt-5/qflags.html
//
// MyrlynAppOption  is just the enum,
// MyrlynAppOptions are several enum values OR'ed together.
Q_DECLARE_FLAGS( MyrlynAppOptions, MyrlynAppOption )
Q_DECLARE_OPERATORS_FOR_FLAGS( MyrlynAppOptions )


/**
 * Application class for Myrlyn.
 **/
class MyrlynApp: public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * 'optFlags' are flags OR'ed together.
     **/
    MyrlynApp( MyrlynAppOptions optFlags = OptNone );

    /**
     * Destructor
     **/
    virtual ~MyrlynApp();

    /**
     * Return the instance of this class or 0 if there is none.
     *
     * This is not a real singleton, but for the lifetime of this application
     * this instance will remain alive, i.e. for most other classes related to
     * this.
     **/
    static MyrlynApp * instance() { return _instance; }

    /**
     * Run the application. This also handles the Qt event loop.
     **/
    void run();

    /**
     * Return 'true' if this program is running with root privileges
     * or if OptFakeRoot (--fake-root) is set.
     *
     * This should be used for actions that semantically need root privileges,
     * but that can also be faked to some extent for demonstration or testing
     * purposes, just at the cost of not actually doing any changes
     * (i.e. losing changes). Do not use this for actions that will throw an
     * exception or cause another real error (not just a warning or error in
     * the log). For those actions, use runningAsRealRoot() instead.
     **/
    static bool runningAsRoot();

    /**
     * Return 'true' if this program is running with real root privileges.
     * This ignores OptFakeRoot.
     *
     * This should be used for actions that cannot be faked; actions that
     * really need root privileges and that will cause an exception or another
     * real error if attempted to do without them.
     **/
    static bool runningAsRealRoot() { return geteuid() == 0; }

    /**
     * Return 'true' if this program is running in read-only mode.
     **/
    static bool readOnlyMode() { return isOptionSet( OptReadOnly ); }

    /**
     * Return the whole option flags.
     **/
    static MyrlynAppOptions optFlags() { return _optFlags; }

    /**
     * Return 'true' if option 'opt' is set.
     **/
    static bool isOptionSet( MyrlynAppOption opt )
        { return _optFlags.testFlag( opt ); }


    //
    // Access to some important member variables
    //

    Workflow *   workflow() const { return _workflow; }
    MainWindow * mainWin()  const { return _mainWin; }

    /**
     * Return the package selector. Create it if it doesn't exist yet.
     *
     * Ownership remains with this class; do not delete it.
     **/
    YQPkgSelector * pkgSel();

    /**
     * Return the package committer. Create it if it doesn't exist yet.
     *
     * Ownership remains with this class; do not delete it.
     **/
    PkgCommitPage * pkgCommitPage();

    /**
     * Return the summary page. Create it if it doesn't exist yet.
     *
     * Ownership remains with this class; do not delete it.
     **/
    SummaryPage * summaryPage();

    /**
     * Return the package tasks object. Create it if it doesn't exist yet.
     *
     * Ownership remains with this class; do not delete it.
     **/
    PkgTasks * pkgTasks();

    /**
     * Return the MyrlynRepoManager. Create it if it doesn't exist yet.
     *
     * Ownership remains with this class; do not delete it.
     **/
    MyrlynRepoManager * repoManager();

    /**
     * Return the ZyppLogger. Create it if it doesn't exist yet.
     **/
    ZyppLogger * zyppLogger();

    /**
     * Create the ZyppLogger if it doesn't exist yet.
     **/
    void createZyppLogger();

    /**
     * Return a font that is suitable for headings:
     * A little big larger than the default font, and bold.
     **/
    static QFont headingFont();

    /**
     * Set the heading font as the font for 'widget'.
     **/
    static void setHeadingFont( QWidget * widget );


public slots:

    /**
     * Go to the next worflow step.
     * If there is no more step, quit the program.
     **/
    void next();

    /**
     * Go to the previous worflow step.
     **/
    void back();

    /**
     * Quit the program when the last workflow step was reached.
     **/
    void finish();

    /**
     * Restart the workflow: Go back to the package selection.
     **/
    void restart();

    /**
     * The user finished the package selection with "Accept", but there was no
     * change: Skip the "commit" phase and go traight to the summary screen.
     *
     * Alternatively this could simply quit the workflow.
     **/
    void skipCommit();

    /**
     * Quit the program.
     *
     * Ask for confirmation if 'askForConfirmation' is 'true'.
     **/
    void quit( bool askForConfirmation = false );


protected:

    /**
     * Log the Qt-related envionment variables
     **/
    void logQtEnv() const;

    // Create the various objects (and set up Qt connections if necessary)
    // if they don't exist yet.

    void createMainWin();
    void createWorkflow();
    void createPkgSel();
    void createPkgCommitPage();
    void createSummaryPage();
    void createPkgTasks();
    void createRepoManager();

    /**
     * Set the appropriate window title, depending if the applicaton is running
     * with or without root permissions.
     **/
    void setWindowTitle( QWidget * window );

    /**
     * Event filter to catch foreign events, e.g. the MainWindow WM_CLOSE.
     *
     * Reimplemented from QObject.
     **/
    virtual bool eventFilter( QObject * watchedObj, QEvent * event ) override;


    //
    // Data members
    //

    MainWindow *            _mainWin;
    Workflow *              _workflow;
    YQPkgSelector *         _pkgSel;
    PkgCommitPage *         _pkgCommitPage;
    SummaryPage *           _summaryPage;
    MyrlynRepoManager *     _myrlynRepoManager;
    ZyppLogger *            _zyppLogger;
    PkgTasks *              _pkgTasks;

    static MyrlynApp *      _instance;
    static MyrlynAppOptions _optFlags;
    static QFont            _headingFont;
    static bool             _headingFontInitialized;
};

#endif // MyrlynApp_h
