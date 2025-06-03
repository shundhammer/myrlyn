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


#ifndef MainWindow_h
#define MainWindow_h

#include <QString>
#include <QWidget>
#include <QList>
#include <QPointer>


typedef QPointer<QWidget> QWidgetPointer;
typedef QList<QWidgetPointer> QWidgetPointerList;

class QVBoxLayout;


/**
 * Main window class that can hold multiple pages, of which one at a time is
 * shown.
 *
 * Create a MainWindow, add pages and show one of them.
 **/
class MainWindow: public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    MainWindow( QWidget * parent = 0 );

    /**
     * Destructor
     **/
    virtual ~MainWindow();

    /**
     * Return the instance of this class or 0 if there is none.
     *
     * This is not a singleton, but in each normal application there is no more
     * than one instance, and during the lifetime of any widgets inside the
     * MainWindow it is safe to assume that the MainWindow also exists.
     **/
    static MainWindow * instance() { return _instance; }

    /**
     * Add a page that can be switched to.
     *
     * If 'pageName' is non-empty, this will be set as the page's object name
     * with QObject::setObjectName(), so the page can be identified in the
     * logs, and you can use findPage() and showPage() with that page name.
     *
     * This class does not take ownership of the page. You can use the normal
     * QObject parent to delete a page when its parent is deleted.
     *
     * It is safe to delete a page from the outside without removing it: This
     * class uses smart pointers to keep track of the pages.
     **/
    void addPage( QWidget * page, const QString & pageName = QString() );

    /**
     * Show a previously added page.
     **/
    void showPage( QWidget * page );

    /**
     * Find a previously added page by its page name and show it.
     **/
    void showPage( const QString & pageName );

    /**
     * Find a previously added page by its name and return it (or 0 if not
     * found).
     **/
    QWidget * findPage( const QString & pageName );

    /**
     * Return the current page, i.e. the page that was last shown with
     * showPage(). Return 0 if there is no current page.
     **/
    QWidget * currentPage() const { return _currentPage; }

    /**
     * Process events for a short while to try to make sure that display
     * updates are being processed before some lengthy operations start.
     *
     * This is useful when showing a page that acts as a "splash screen" to
     * give immediate user feedback, even when the rest of the applicaton still
     * takes a while to load completely.
     *
     * Notice that you can use a simple QLabel as a page for this purpose;
     * this is very much like the BusyPopup class, but in the main window.
     **/
    static void processEvents( int millisec = 500 );


    /**
     * Add and show a splash screen page and process events to try to make sure
     * that it's actually displayed before lengthy operation are done.
     *
     * This is typically a QLabel created on the stack (not on the heap with
     * 'new') that exists in a code block just before the lengthy operations
     * start. Simply let it go out of scope.
     *
     * Example:
     *
     *   {
     *       QLabel message( _( "Computing world solution..." ) );
     *       myMainWindow->splashPage( &message );
     *       doComputations();
     *       computeSomeMore();
     *   }
     *
     **/
    void splashPage( QWidget * page );


protected:

    /**
     * Create the internal widgets and layouts of this main window.
     **/
    void basicLayout();

    /**
     * Clear the client area, i.e. remove the current page and hide it.
     **/
    void clearClientArea();

    /**
     * Event handler for WM_CLOSE (Alt-F4).
     *
     * Reimplemented from QWidget.
     **/
    virtual void closeEvent( QCloseEvent * event ) override;


    //
    // Data members
    //

    QWidgetPointerList  _pages;
    QWidgetPointer      _currentPage;
    QVBoxLayout *       _layout;

    static MainWindow * _instance;
};

#endif // MainWindow_h
