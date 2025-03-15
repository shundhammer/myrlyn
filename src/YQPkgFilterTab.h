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


#ifndef YQPkgFilterTab_h
#define YQPkgFilterTab_h


#include <memory>

#include <QAction>
#include <QHash>
#include <QKeySequence>
#include <QWidget>
#include <QTabWidget>

class YQPkgFilterTabPrivate;
class YQPkgFilterPage;
class YQPkgDiskUsageList;

typedef std::vector<YQPkgFilterPage *> YQPkgFilterPageVector;


/**
 * Widget for "tabbed browsing" in packages:
 *
 *               /------\/------\/------\
 *     [View v]  | Tab1 || Tab2 || Tab3 |                [logo]
 *     +-----------------+------------------------------------+
 *     |                 |                                    |
 *     | QStackedWidget: |    Right pane                      |
 *     |                 |                                    |
 *     | Filter pages    |    (application defined)           |
 *     |                 |                                    |
 *     |                 |                                    |
 *     |                 |                                    |
 *     .                 .                                    .
 *     .                 .                                    .
 *     .                 .                                    .
 *     |                 |                                    |
 *     +-----------------+------------------------------------+
 *
 * Each filter page corresponds to one tab and a number of widgets in a
 * QStackedWidget in the left filter pane. When tabs are switched, the
 * corresponding filter page is raised to the top of the widget stack.
 * The right pane, however, remains unchanged.
 *
 * Only a small numbers of filter pages is displayed as open tabs right
 * away. Each of the other filter pages is shown in a new tabs when the user
 * requests it via the pop-up menu on [View] button. Similarly, the tabs for all
 * but the last filter pages can be closed with the [Close] button.
 *
 * The left (filter page) and right panes are separated with a user-moveable
 * splitter.
 **/
class YQPkgFilterTab: public QTabWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     **/
    YQPkgFilterTab( QWidget * parent );

    /**
     * Destructor.
     **/
    virtual ~YQPkgFilterTab();

    /**
     * Add a page with a user-visible "pageLabel", a widget with the page
     * content, an internal name (or ID) and an optional hotkey.
     *
     * 'pageContent' will be reparented to a subwidget of this class.
     **/
    void addPage( const QString &      pageLabel,
                  QWidget *            pageContent,
                  const QString &      internalName,
                  const QKeySequence & hotkey = QKeySequence() );

    /**
     * Return the right pane.
     **/
    QWidget * rightPane() const;

    /**
     * Return the disk usage list widget or 0 if there is none.
     **/
    YQPkgDiskUsageList * diskUsageList() const;

    /**
     * Find a filter page by its content widget (the widget that was passed
     * to addPage() ).
     * Return 0 if there is no such page.
     **/
    YQPkgFilterPage * findPage( QWidget * pageContent ) const;

    /**
     * Find a filter page by its internal name.
     * Return 0 if there is no such page.
     **/
    YQPkgFilterPage * findPage( const QString & internalName ) const;

    /**
     * Find a filter page by its tab index.
     * Return 0 if there is no such page.
     **/
    YQPkgFilterPage * findPage( int tabIndex ) const;

    /**
     * Return the number of open tabs.
     **/
    int tabCount() const;

    /**
     * Return 'true' if widget 'pageContent' is the current page, 'false' if
     * not.
     **/
    bool isCurrentPage( QWidget * pageContent ) const;


    /**
     * Event filter to catch mouse right clicks on open tabs for the tab
     * context menu. Returns 'true' if the event is processed and consumed,
     * 'false' if processed should be continued by the watched object itself.
     *
     * Reimplemented from QObject.
     **/
    virtual bool eventFilter ( QObject * watchedObj, QEvent * event ) override;


signals:

    /**
     * Emitted when the current page changes.
     * NOT emitted initially for the very first page that is shown.
     **/
    void currentChanged( QWidget * newPageContent );


public slots:

    /**
     * Show a page. Create a tab for that page if it doesn't already exist.
     **/
    void showPage( QWidget * page );
    void showPage( const QString & internalName );
    void showPage( int tabIndex );

    /**
     * Set a new label for a page.
     * This can be used to add a numeric value like "Patches (17)".
     **/
    void setPageLabel( QWidget * pageContent, const QString & newLabel );

    /**
     * Reload the current page: Send a currentChange() signal with the current
     * page as argument so it will filter again.
     **/
    void reloadCurrentPage();

    /**
     * Load settings, including which tabs are to be opened and in which order.
     * Return 'true' if settings could be loaded, 'false' if not.
     *
     * Applications should call this after all pages have been added so the
     * open tabs can be restored the same way as the user left the program.
     * If tabCount() is still 0 afterwards, there were no settings, so it might
     * make sense to open a number of default pages.
     **/
    void readSettings();

    /**
     * Save the current settings, including which tabs are currently open and
     * in which order. This is implicitly done in the destructor.
     **/
    void writeSettings();

    /**
     * Close all currently open pages.
     **/
    void closeAllPages();


protected slots:

    /**
     * Show the page with the widget of this action's data().
     **/
    void showPage( QAction * action );

    /**
     * Move the current tab page one position to the left.
     **/
    void movePageLeft();

    /**
     * Move the current tab page one position to the right.
     **/
    void movePageRight();

    /**
     * Close the current tab page.
     **/
    void closePage();


protected:

    /**
     * Const access to the pages container
     **/
    const YQPkgFilterPageVector & constPages() const;

    /**
     * Read/write access to the pages container
     **/
    YQPkgFilterPageVector & pages();

    /**
     * Create and connec the actions for the tab context menu.
     **/
    void createActions();

    /**
     * Show a page.
     **/
    void showPage( YQPkgFilterPage * page );

    /**
     * Open the tab context menu for the tab at the specified position.
     * Return 'true' upon success (i.e., there is a tab at that position),
     * 'false' upon failure.
     **/
    bool postTabContextMenu( const QPoint & pos );

    /**
     * Swap two tabs and adjust their tab indices accordingly.
     **/
    void swapTabs( YQPkgFilterPage * page1, YQPkgFilterPage * page2 );


private:

    const std::unique_ptr<YQPkgFilterTabPrivate> _priv;
};



/**
 * Helper class for filter pages
 **/
class YQPkgFilterPage
{
public:

    YQPkgFilterPage( const QString &    pageLabel,
                     QWidget *          content,
                     const QString &    internalName )
        : content( content )
        , label( pageLabel )
        , id( internalName )
        , closeEnabled( true )
        , tabIndex( -1 )
        , action( 0 )
        {}

    virtual ~YQPkgFilterPage()
        {
            if ( action )
                delete action;
        }


    QWidget *   content;
    QString     label;          // user visible text
    QString     id;             // internal name
    bool        closeEnabled;
    int         tabIndex;       // index of the corresponding tab or -1 if none
    QAction *   action;
};


#endif // YQPkgFilterTab_h
