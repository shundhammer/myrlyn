/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include <vector>
#include <algorithm> // std::swap()

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabBar>

#include "Exception.h"
#include "Logger.h"
#include "YQIconPool.h"
#include "YQPkgDiskUsageList.h"
#include "YQi18n.h"
#include "utf8.h"
#include "YQPkgFilterTab.h"


typedef std::vector<YQPkgFilterPage *> YQPkgFilterPageVector;


struct YQPkgFilterTabPrivate
{
    YQPkgFilterTabPrivate()
        : baseClassWidgetStack(0)
        , outerSplitter(0)
        , leftPaneSplitter(0)
        , filtersWidgetStack(0)
        , diskUsageList(0)
        , rightPane(0)
        , viewButton(0)
        , tabContextMenu(0)
        , tabContextMenuPage(0)
        {}

    QStackedWidget *      baseClassWidgetStack;
    QSplitter *           outerSplitter;
    QSplitter *           leftPaneSplitter;
    QStackedWidget *      filtersWidgetStack;
    YQPkgDiskUsageList *  diskUsageList;
    QWidget *             rightPane;
    QPushButton *         viewButton;
    QMenu *               tabContextMenu;
    QAction *             actionMovePageLeft;
    QAction *             actionMovePageRight;
    QAction *             actionClosePage;
    YQPkgFilterPage *     tabContextMenuPage;
    YQPkgFilterPageVector pages;
};




YQPkgFilterTab::YQPkgFilterTab( QWidget * parent )
    : QTabWidget( parent )
    , _priv( new YQPkgFilterTabPrivate() )
{
    CHECK_NEW( _priv );

    // Nasty hack: Find the base class's QStackedWidget in its widget tree so
    // we have a place to put our own widgets. Unfortunately, this is private
    // in the base class, but Qt lets us search the widget hierarchy by widget
    // type.

    _priv->baseClassWidgetStack = findChild<QStackedWidget*>();
    CHECK_PTR( _priv->baseClassWidgetStack );

    // Nasty hack: Disconnect the base class from signals from its tab bar.
    // We will handle that signal on our own.

    disconnect( tabBar(), SIGNAL( currentChanged( int ) ) );


    //
    // Splitter that divides this widget into a left and a right pane
    //

    _priv->outerSplitter = new QSplitter( Qt::Horizontal, this );
    CHECK_NEW( _priv->outerSplitter );

    _priv->outerSplitter->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                                      QSizePolicy::Expanding ) );
    _priv->baseClassWidgetStack->addWidget( _priv->outerSplitter );


    //
    // "View" button
    //

    QWidget * buttonBox = new QWidget( this );
    CHECK_NEW( buttonBox );
    setCornerWidget( buttonBox, Qt::TopRightCorner );
    buttonBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    QHBoxLayout * buttonBoxLayout = new QHBoxLayout( buttonBox );
    CHECK_NEW( buttonBoxLayout );
    buttonBox->setLayout( buttonBoxLayout );
    buttonBoxLayout->setContentsMargins( 0, 0, 0, 0 );

    // Translators: Button with pop-up menu to open a new page (very much like
    // in a web browser) with another package filter view or to switch to an
    // existing one if it's open already

    _priv->viewButton = new QPushButton( _( "&View" ), this );
    CHECK_NEW( _priv->viewButton );
    setCornerWidget( _priv->viewButton, Qt::TopLeftCorner );

    QMenu * menu = new QMenu( _priv->viewButton );
    CHECK_NEW( menu );
    _priv->viewButton->setMenu( menu );

    connect( menu, SIGNAL( triggered( QAction * ) ),
             this, SLOT  ( showPage ( QAction * ) ) );


    //
    // Splitter that divides the left pane into upper filters area and disk usage area
    //

    _priv->leftPaneSplitter = new QSplitter( Qt::Vertical, _priv->outerSplitter );
    CHECK_NEW( _priv->leftPaneSplitter );


    //
    // Left pane content
    //

    _priv->filtersWidgetStack = new QStackedWidget( _priv->leftPaneSplitter );
    CHECK_NEW( _priv->filtersWidgetStack );

    _priv->diskUsageList = new YQPkgDiskUsageList( _priv->leftPaneSplitter );
    CHECK_NEW( _priv->diskUsageList );

    {
        QSplitter * splitter = _priv->leftPaneSplitter;
        splitter->setStretchFactor( splitter->indexOf( _priv->filtersWidgetStack ), 1 );
        splitter->setStretchFactor( splitter->indexOf( _priv->diskUsageList      ), 1 );

        // FIXME: Don't always hide the disk usage list
        const int diskUsageListHeight = 0;
        QList<int> sizes;
        sizes << _priv->leftPaneSplitter->height();
        sizes << diskUsageListHeight;
        splitter->setSizes( sizes );
    }


    //
    // Right pane
    //

    _priv->rightPane = new QWidget( _priv->outerSplitter );
    CHECK_NEW( _priv->rightPane );


    //
    // Stretch factors for left and right pane
    //

    {
        QSplitter * sp = _priv->outerSplitter;
        sp->setStretchFactor( sp->indexOf( _priv->leftPaneSplitter ), 0 );
        sp->setStretchFactor( sp->indexOf( _priv->rightPane        ), 1 );
    }


    // Set up connections

    connect( tabBar(), SIGNAL( currentChanged( int ) ),
             this,     SLOT  ( showPage      ( int ) ) );

    tabBar()->installEventFilter( this ); // for tab context menu


    //
    // Cosmetics
    //

    // Inner margin between 3D borders and content
    _priv->baseClassWidgetStack->setContentsMargins(  8,   // left
                                                     11,   // top
                                                      8,   // right
                                                      4 ); // bottom

    // Margin around the left pane (the filter view)
    _priv->leftPaneSplitter->setContentsMargins  ( 0,    // left
                                                   0,    // top
                                                   0,    // right
                                                   0 );  // bottom


    // _priv->rightPane->setContentsMargins() is set when widgets are added to
    // the right pane. See YQPkgSelector::layoutRightPane().
}


YQPkgFilterTab::~YQPkgFilterTab()
{
    writeSettings();

    for ( YQPkgFilterPageVector::iterator it = _priv->pages.begin();
          it != _priv->pages.end();
          ++it )
    {
        delete (*it);
    }

    _priv->pages.clear();
}


QWidget *
YQPkgFilterTab::rightPane() const
{
    return _priv->rightPane;
}


YQPkgDiskUsageList *
YQPkgFilterTab::diskUsageList() const
{
    return _priv->diskUsageList;
}


void
YQPkgFilterTab::addPage( const QString & pageLabel,
                         QWidget *       pageContent,
                         const QString & internalName )
{
    YQPkgFilterPage * page = new YQPkgFilterPage( pageLabel,
                                                  pageContent,
                                                  internalName );
    CHECK_NEW( page );

    _priv->pages.push_back( page );
    _priv->filtersWidgetStack->addWidget( pageContent );


    if ( _priv->viewButton && _priv->viewButton->menu() )
    {
        QAction * action = new QAction( pageLabel, this );
        CHECK_NEW( action );
        action->setData( QVariant::fromValue( pageContent ) );

        _priv->viewButton->menu()->addAction( action );
    }
}


void
YQPkgFilterTab::showPage( QWidget * pageContent )
{
    YQPkgFilterPage * page = findPage( pageContent );
    CHECK_PTR( page );

    showPage( page );
}


void
YQPkgFilterTab::showPage( const QString & internalName )
{
    YQPkgFilterPage * page = findPage( internalName );

    if ( page )
        showPage( page );
    else
        logWarning() << "No page with ID \"" << internalName << "\"" << Qt::endl;
}


void
YQPkgFilterTab::showPage( int tabIndex )
{
    YQPkgFilterPage * page = findPage( tabIndex );

    if ( page )
        showPage( page );
}


void
YQPkgFilterTab::showPage( QAction * action )
{
    if ( ! action )
        return;

    QWidget * pageContent = action->data().value<QWidget *>();
    showPage( pageContent );
}


void
YQPkgFilterTab::showPage( YQPkgFilterPage * page )
{
    CHECK_PTR( page );
    QSignalBlocker sigBlocker( tabBar() );

    if ( page->tabIndex < 0 ) // No corresponding tab yet?
    {
        // Add a tab for that page
        page->tabIndex = tabBar()->addTab( page->label );
    }

    _priv->filtersWidgetStack->setCurrentWidget( page->content );
    tabBar()->setCurrentIndex( page->tabIndex );
    _priv->tabContextMenuPage = page;

    emit currentChanged( page->content );
}


void
YQPkgFilterTab::closeAllPages()
{
    while ( tabBar()->count() > 0 )
    {
        tabBar()->removeTab( 0 );
    }

    for ( YQPkgFilterPageVector::iterator it = _priv->pages.begin();
          it != _priv->pages.end();
          ++it )
    {
        (*it)->tabIndex = -1;
    }
}


void
YQPkgFilterTab::closeCurrentPage()
{
    if ( tabBar()->count() > 1 )
    {
        int currentIndex = tabBar()->currentIndex();
        YQPkgFilterPage * currentPage = findPage( currentIndex );

        if ( currentPage )
            currentPage->tabIndex = -1;

        tabBar()->removeTab( currentIndex );

        //
        // Adjust tab index of the active pages to the right of that page
        //

        for ( YQPkgFilterPageVector::iterator it = _priv->pages.begin();
              it != _priv->pages.end();
              ++it )
        {
            YQPkgFilterPage * page = *it;

            if ( page->tabIndex >= currentIndex )
                page->tabIndex--;
        }

        showPage( tabBar()->currentIndex() );
    }
}


YQPkgFilterPage *
YQPkgFilterTab::findPage( QWidget * pageContent )
{
    for ( YQPkgFilterPageVector::iterator it = _priv->pages.begin();
          it != _priv->pages.end();
          ++it )
    {
        if ( (*it)->content == pageContent )
            return *it;
    }

    return 0;
}


YQPkgFilterPage *
YQPkgFilterTab::findPage( const QString & internalName )
{
    for ( YQPkgFilterPageVector::iterator it = _priv->pages.begin();
          it != _priv->pages.end();
          ++it )
    {
        if ( (*it)->id == internalName )
            return *it;
    }

    return 0;
}


YQPkgFilterPage *
YQPkgFilterTab::findPage( int tabIndex )
{
    if ( tabIndex < 0 )
        return 0;

    for ( YQPkgFilterPageVector::iterator it = _priv->pages.begin();
          it != _priv->pages.end();
          ++it )
    {
        if ( (*it)->tabIndex == tabIndex )
            return *it;
    }

    return 0;
}


int
YQPkgFilterTab::tabCount() const
{
    return tabBar()->count();
}


bool
YQPkgFilterTab::eventFilter( QObject * watchedObj, QEvent * event )
{
    if ( watchedObj == tabBar() &&
         event && event->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *> (event);

        if ( mouseEvent && mouseEvent->button() == Qt::RightButton )
        {
            return postTabContextMenu( mouseEvent->pos() );
        }
    }

    return QTabWidget::eventFilter( watchedObj, event );
}


bool
YQPkgFilterTab::postTabContextMenu( const QPoint & pos )
{
    int tabIndex = tabBar()->tabAt( pos );

    if ( tabIndex >= 0 ) // -1 means "no tab at that position"
    {
        _priv->tabContextMenuPage = findPage( tabIndex );

        if ( _priv->tabContextMenuPage )
        {
            if ( ! _priv->tabContextMenu )
            {
                // On-demand menu creation

                _priv->tabContextMenu = new QMenu( this );
                CHECK_NEW( _priv->tabContextMenu );

                // bool reverseLayout = YUI::yApp()->reverseLayout();
                // FIXME
                bool reverseLayout = false;


                // Translators: Change this to "right" for Arabic and Hebrew
                _priv->actionMovePageLeft  = new QAction( reverseLayout ?
                                                          YQIconPool::arrowRight() : YQIconPool::arrowLeft(),
                                                          _( "Move page &left"  ), this );
                CHECK_NEW( _priv->actionMovePageLeft );

                connect( _priv->actionMovePageLeft, SIGNAL( triggered()           ),
                         this,                      SLOT  ( contextMovePageLeft() ) );


                // Translators: Change this to "left" for Arabic and Hebrew
                _priv->actionMovePageRight = new QAction(  reverseLayout ?
                                                           YQIconPool::arrowLeft() : YQIconPool::arrowRight(),
                                                           _( "Move page &right" ), this );
                CHECK_NEW( _priv->actionMovePageRight );

                connect( _priv->actionMovePageRight, SIGNAL( triggered()            ),
                         this,                       SLOT  ( contextMovePageRight() ) );


                _priv->actionClosePage = new QAction( YQIconPool::tabRemove(), _( "&Close page" ), this );
                CHECK_NEW( _priv->actionClosePage );

                connect( _priv->actionClosePage, SIGNAL( triggered()        ),
                         this,                   SLOT  ( contextClosePage() ) );


                _priv->tabContextMenu->addAction( _priv->actionMovePageLeft  );
                _priv->tabContextMenu->addAction( _priv->actionMovePageRight );
                _priv->tabContextMenu->addAction( _priv->actionClosePage     );
            }

            // Enable / disable actions

            _priv->actionMovePageLeft->setEnabled( tabIndex > 0 );
            _priv->actionMovePageRight->setEnabled( tabIndex < ( tabBar()->count() - 1 ) );
            _priv->actionClosePage->setEnabled( tabBar()->count() > 1 && _priv->tabContextMenuPage->closeEnabled );

            _priv->tabContextMenu->popup( tabBar()->mapToGlobal( pos ) );

            return true; // event consumed - no further processing
        }
    }

    return false; // no tab at that position
}


void
YQPkgFilterTab::contextMovePageLeft()
{
    if ( _priv->tabContextMenuPage )
    {
        int contextPageIndex = _priv->tabContextMenuPage->tabIndex;
        int otherPageIndex   = contextPageIndex-1;

        if ( otherPageIndex >= 0 )
        {
            swapTabs( _priv->tabContextMenuPage, findPage( otherPageIndex ) );
        }
    }
}


void
YQPkgFilterTab::contextMovePageRight()
{
    if ( _priv->tabContextMenuPage )
    {
        int contextPageIndex = _priv->tabContextMenuPage->tabIndex;
        int otherPageIndex   = contextPageIndex+1;

        if ( otherPageIndex < tabBar()->count() )
        {
            swapTabs( _priv->tabContextMenuPage, findPage( otherPageIndex ) );
        }
    }
}


void
YQPkgFilterTab::swapTabs( YQPkgFilterPage * page1, YQPkgFilterPage * page2 )
{
    if ( ! page1 or ! page2 )
        return;

    int oldCurrentIndex = tabBar()->currentIndex();
    std::swap( page1->tabIndex, page2->tabIndex );
    tabBar()->setTabText( page1->tabIndex, page1->label );
    tabBar()->setTabText( page2->tabIndex, page2->label );


    // If one of the two pages was the currently displayed one,
    // make sure the same page is still displayed.

    if ( oldCurrentIndex == page1->tabIndex )
    {
        QSignalBlocker sigBlocker( tabBar() );
        tabBar()->setCurrentIndex( page2->tabIndex );
    }
    else if ( oldCurrentIndex == page2->tabIndex )
    {
        QSignalBlocker sigBlocker( tabBar() );
        tabBar()->setCurrentIndex( page1->tabIndex );
    }
}


void
YQPkgFilterTab::contextClosePage()
{
    if ( _priv->tabContextMenuPage )
    {
        int pageIndex = _priv->tabContextMenuPage->tabIndex;
        _priv->tabContextMenuPage->tabIndex = -1;
        tabBar()->removeTab( pageIndex );


        //
        // Adjust tab index of the active pages to the right of that page
        //

        for ( YQPkgFilterPageVector::iterator it = _priv->pages.begin();
              it != _priv->pages.end();
              ++it )
        {
            YQPkgFilterPage * page = *it;

            if ( page->tabIndex >= pageIndex )
                page->tabIndex--;
        }

        showPage( tabBar()->currentIndex() ); // display the new current page
    }
}


void
YQPkgFilterTab::readSettings()
{
    closeAllPages();

    QSettings settings;
    settings.beginGroup( "Pages" );

    QStringList pages = settings.value( "OpenPages"   ).toStringList();
    QString current   = settings.value( "CurrentPage" ).toString();

    settings.endGroup();


    logDebug() << "Restoring pages " << pages << Qt::endl;
    logDebug() << "Current page:   " << current << Qt::endl;

    {
        // Prevent an event cascade as pages are added: Each one would cause
        // the package list to get filled, only to get cleared and filled again
        // by the next page.
        //
        // The signals are unblocked when the sigBlocker goes out of scope.

        QSignalBlocker sigBlocker( this );

        foreach ( QString id, pages )
        {
            YQPkgFilterPage * page = findPage( id );

            if ( page )
                showPage( page );
            else
                logWarning() << "No page with ID \"" << id << "\"" << Qt::endl;
        }
    }

    if ( ! current.isEmpty() )
    {
        YQPkgFilterPage * page = findPage( current );

        if ( page )
            showPage( page ); // We want this to emit signals to fill the pkg list
        else
            logWarning() << "Can't restore current page with ID \"" << current << "\"" << Qt::endl;
    }
}


void
YQPkgFilterTab::writeSettings()
{
    QStringList pages;

    for ( int i=0; i < tabBar()->count(); i++ )
    {
        YQPkgFilterPage * page = findPage( i );

        if ( page )
        {
            if ( page->id.isEmpty() )
                logWarning() << "No ID for tab page \"" << page->label << "\"" << Qt::endl;
            else
                pages << page->id;
        }
    }

    YQPkgFilterPage * currentPage = findPage( tabBar()->currentIndex() );


    QSettings settings;
    settings.beginGroup( "Pages" );

    settings.setValue( "OpenPages", pages );

    if ( currentPage )
        settings.setValue( "CurrentPage", currentPage->id );

    settings.endGroup();
}
