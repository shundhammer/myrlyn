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


#include "QY2CursorHelper.h"
#include "YQi18n.h"

#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/Resolver.h>

#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QPushButton>
#include <QElapsedTimer>
#include <QPainter>
#include <QMessageBox>
#include <QPixmap>
#include <QBoxLayout>

#include "BusyPopup.h"
#include "Logger.h"
#include "MainWindow.h"
#include "QY2LayoutUtils.h"
#include "WindowSettings.h"
#include "YQPkgConflictList.h"
#include "YQPkgConflictDialog.h"


#define SPACING  6       // between subwidgets
#define MARGIN   4       // around the widget


YQPkgConflictDialog * YQPkgConflictDialog::_instance = 0;


YQPkgConflictDialog::YQPkgConflictDialog( QWidget * parent )
    : QDialog( parent ? parent : MainWindow::instance() )
{
    if ( ! _instance )
        _instance = this;

    // Set the dialog title.
    //
    // "Dependency conflict" is already used as the conflict list header just
    // some pixels below that, so don't use this twice. This dialog title may
    // or may not be visible, depending on whether or not there is a window
    // manager running (and configured to show any dialog titles).

    setWindowTitle( _( "Warning" ) );

    // Enable dialog resizing even without window manager
    setSizeGripEnabled( true );


    // Layout for the dialog (can't simply insert a QVbox)

    QVBoxLayout * layout = new QVBoxLayout();
    Q_CHECK_PTR( layout );

    setLayout(layout);
    layout->setContentsMargins( MARGIN, MARGIN, MARGIN, MARGIN );
    layout->setSpacing( SPACING );


    // Conflict list

    _conflictList = new YQPkgConflictList( this );
    Q_CHECK_PTR( _conflictList );

    layout->addWidget( _conflictList );
    layout->addSpacing( 2 );

    connect( _conflictList, SIGNAL( updatePackages() ),
             this,          SIGNAL( updatePackages() ) );


    // Button box

    QHBoxLayout * buttonBox = new QHBoxLayout();
    Q_CHECK_PTR( buttonBox );

    buttonBox->setSpacing( SPACING );
    buttonBox->setContentsMargins( MARGIN, MARGIN, MARGIN, MARGIN );
    layout->addLayout( buttonBox );
    buttonBox->addStretch();


    // "OK" button

    QPushButton * button = new QPushButton( _( "&OK -- Try Again" ), this);
    Q_CHECK_PTR( button );

    buttonBox->addWidget(button);
    button->setDefault( true );

    connect( button, SIGNAL( clicked()               ),
             this,   SLOT  ( solveAndShowConflicts() ) );


    // "Expert" menu button

    button = new QPushButton( _( "&Expert" ), this );
    Q_CHECK_PTR( button );

    buttonBox->addWidget(button);


    // "Expert" menu

    _expertMenu = new QMenu( button );
    Q_CHECK_PTR( _expertMenu );

    button->setMenu( _expertMenu );

    _expertMenu->addAction( _( "&Save This List to a File..." ),
                            _conflictList, SLOT( askSaveToFile() ) );


    // "Cancel" button

    button = new QPushButton( _( "&Cancel" ), this);
    Q_CHECK_PTR( button );

    buttonBox->addWidget( button );
    buttonBox->addStretch();

    connect( button, SIGNAL( clicked() ),
             this,   SLOT  ( reject()  ) );



    // Useful initial size if there is nothing in the settings yet
    resize( MainWindow::instance()->size() * 0.8 );

    WindowSettings::read( this, "PkgConflictDialog" );
}


YQPkgConflictDialog::~YQPkgConflictDialog()
{
    WindowSettings::write( this, "PkgConflictDialog" );

    if ( _instance == this )
        _instance = 0;
}


QSize
YQPkgConflictDialog::sizeHint() const
{
    return limitToScreenSize( this, 550, 450 );
}


int
YQPkgConflictDialog::solveAndShowConflicts()
{
    prepareSolving();
    logInfo() << "Resolving dependencies..." << Qt::endl;

    bool success = zypp::getZYpp()->resolver()->resolvePool();

    logDebug() << "Resolving dependencies done." << Qt::endl;

    return processSolverResult( success );
}


int
YQPkgConflictDialog::verifySystem( bool showBusyPopup )
{
    BusyPopup * busyPopup = 0;

    if ( showBusyPopup )
        busyPopup = new BusyPopup( _( "Verifying System Dependencies" ),
                                   MainWindow::instance() );

    prepareSolving();
    logInfo() << "Verifying all system dependencies..." << Qt::endl;

    bool success = zypp::getZYpp()->resolver()->verifySystem();

    logDebug() << "System dependencies verified." << Qt::endl;

    if ( busyPopup )
        delete busyPopup;

    return processSolverResult( success );
}


int
YQPkgConflictDialog::verifySystemWithBusyPopup()
{
    return verifySystem( true ); // showBusyPopup
}



int
YQPkgConflictDialog::doPackageUpdate()
{
    prepareSolving();
    logInfo() << "Starting a global package update ('zypper up' counterpart)..." << Qt::endl;

    bool success = true;
    zypp::getZYpp()->resolver()->doUpdate(); // No return value, assume success.

    logDebug() << "Package update done." << Qt::endl;

    return processSolverResult( success );
}


int
YQPkgConflictDialog::doDistUpgrade()
{
    prepareSolving();
    logInfo() << "Starting a dist upgrade ('zypper dup' counterpart)" << Qt::endl;

    bool success = zypp::getZYpp()->resolver()->doUpgrade();

    logDebug() << "Dist upgrade done." << Qt::endl;

    return processSolverResult( success );
}


void
YQPkgConflictDialog::prepareSolving()
{
    Q_CHECK_PTR( _conflictList );
    busyCursor();

    if ( isVisible() )
    {
        // This is not only the starting point for all the dependency solving
        // magic, it is also used internally when clicking the "OK - Try again"
        // button. Thus, before doing anything else, check if the conflict list
        // still contains anything, and if so, apply any conflict resolutions
        // the user selected - but only if this dialog is already visible.

        _conflictList->applyResolutions();
    }

    // Initialize for next round of solving.
    _conflictList->clear();
}


int
YQPkgConflictDialog::processSolverResult( bool success )
{
    // Package states may have changed: The solver may have set packages to
    // autoInstall or autoUpdate. Make those changes known.
    emit updatePackages();

    normalCursor();
    int result = QDialog::Accepted;

    if ( success )      // Solving went without any complaints?
    {
        result = QDialog::Accepted;

        if ( isVisible() )
            accept();   // Pop down the dialog.
    }
    else                // There were solving problems.
    {
        logDebug() << "Dependency conflict!" << Qt::endl;
        busyCursor();

        _conflictList->fill( zypp::getZYpp()->resolver()->problems() );
        normalCursor();

        if ( ! isVisible() )
        {
            // Pop up the dialog and run a local event loop.
            result = exec();
        }
    }

    return result;      // QDialog::Accepted or QDialog::Rejected
}


void
YQPkgConflictDialog::resetIgnoredDependencyProblems()
{
    zypp::getZYpp()->resolver()->undo();
}


void
YQPkgConflictDialog::askCreateSolverTestCase()
{
    QString testCaseDir = "/var/log/YaST2/solverTestcase";

    // Heading for popup dialog
    QString heading = QString( "<h2>%1</h2>" ).arg( _( "Create Dependency Resolver Test Case" ) );

    QString msg =
        _( "<p>Use this to generate extensive logs to help tracking down bugs in the dependency resolver. "
           "The logs will be stored in directory <br><tt>%1</tt></p>" ).arg( testCaseDir );

    int button_no = QMessageBox::information( 0,                        // parent
                                              _( "Solver Test Case" ),  // caption
                                              heading + msg,
                                              _( "C&ontinue" ),         // button #0
                                              _( "&Cancel" ) );         // button #1

    if ( button_no == 1 )  // Cancel
        return;

    logInfo() << "Generating solver test case START" << Qt::endl;

    bool success = zypp::getZYpp()->resolver()->createSolverTestcase( qPrintable( testCaseDir ) );

    logInfo() << "Generating solver test case END" << Qt::endl;

    if ( success )
    {
        msg = _( "<p>Dependency resolver test case written to <br><tt>%1</tt></p>" ).arg( testCaseDir );

        QMessageBox::information( MainWindow::instance(),  // parent
                                  _( "Success" ),          // caption
                                  msg );
    }
    else // no success
    {
        msg = _( "<p><b>Error</b> creating dependency resolver test case</p>"
                 "<p>Please check disk space and permissions for <tt>%1</tt></p>" ).arg( testCaseDir );

        QMessageBox::warning( MainWindow::instance(),  // parent
                              _( "Error" ),            // caption
                              msg );
    }
}
