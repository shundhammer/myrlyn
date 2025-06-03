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


#include <QApplication>
#include <QCloseEvent>
#include <QEventLoop>
#include <QScreen>
#include <QVBoxLayout>

#include "Exception.h"
#include "Logger.h"
#include "MyrlynApp.h"
#include "WindowSettings.h"
#include "MainWindow.h"


MainWindow * MainWindow::_instance = 0;


MainWindow::MainWindow( QWidget * parent )
    : QWidget( parent )
    , _layout(0)
{
    _instance = this;
    basicLayout();

    logDebug() << "Device pixel ratio: " << screen()->devicePixelRatio() << endl;
    logDebug() << "Logical  dpi: " << screen()->logicalDotsPerInch()  << endl;
    logDebug() << "Physical dpi: " << screen()->physicalDotsPerInch() << endl;
    logDebug() << "Default font: " << qApp->font().key() << endl;
    logDebug() << "Heading font: " << MyrlynApp::headingFont().key() << endl;

     // Set a reasonable initial size if there is nothing in the settings

    QSize size = screen()->availableSize();

    if ( size.width() > 1280 )
        size *= 0.8;

    resize( size );

    WindowSettings::read( this, "MainWindow" );
}


MainWindow::~MainWindow()
{
    WindowSettings::write( this, "MainWindow" );
    _instance = 0;
}


void MainWindow::basicLayout()
{
    _layout = new QVBoxLayout( this );
    CHECK_NEW( _layout );

    // Don't let margins accumulate: The UI designer generated forms have their
    // own, the manually created YQPackageManager doesn't need or want them.
    _layout->setContentsMargins( 0, 0, 0, 0 ); // left, right, top, bottom
    _layout->setAlignment( Qt::AlignCenter );
}


void MainWindow::addPage( QWidget * page, const QString & pageName )
{
    CHECK_PTR( page );

    if ( ! pageName.isEmpty() )
        page->setObjectName( pageName );

    _pages << QWidgetPointer( page );
}


void MainWindow::showPage( QWidget * page )
{
    _currentPage = page; // even if it's 0

    clearClientArea();

    if ( page )
    {
        if ( ! page->objectName().isEmpty() )
        {
            logDebug() << "Showing page " << page->objectName()
                       << " class " << page->metaObject()->className()
                       << endl;
        }
        else
        {
            logDebug() << "Showing class " << page->metaObject()->className() << endl;
        }

        _layout->addWidget( page );
        page->show();
    }
}


void MainWindow::clearClientArea()
{
    while ( _layout && _layout->count() > 0 )
    {
        QLayoutItem * layoutItem = _layout->takeAt( 0 );

        if ( layoutItem && layoutItem->widget() )
            layoutItem->widget()->hide();
    }
}


void MainWindow::showPage( const QString & pageName )
{
    QWidget * page = findPage( pageName );

    if ( page )
        showPage( page );
}


QWidget * MainWindow::findPage( const QString & pageName )
{
    if ( pageName.isEmpty() )
    {
        logError() << "Empty page name" << endl;
        return 0;
    }

    for ( const QWidgetPointer & page: _pages )
    {
        // Notice that 'page' might easily be 0 if it has been deleted in the
        // meantime from the outside; for example if it was a local variable
        // that went out of scope. See the splashPage() example in the header
        // file.

        if ( page && page->objectName() == pageName )
            return page;
    }

    logError() << "No page with name \"" << pageName << "\"" << endl;

    return 0;
}


void MainWindow::processEvents( int millisec )
{
    QEventLoop eventLoop;
    eventLoop.processEvents( QEventLoop::ExcludeUserInputEvents,
			     millisec );
}


void MainWindow::splashPage( QWidget * page )
{
    if ( page )
    {
        addPage( page );
        showPage( page );
        processEvents();
    }
}


void MainWindow::closeEvent( QCloseEvent * event )
{
    Q_UNUSED( event );

    logInfo() << "Caught WM_CLOSE" << endl;
    // event->ignore();
}
