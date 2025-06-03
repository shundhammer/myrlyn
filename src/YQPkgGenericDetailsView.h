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


#ifndef YQPkgGenericDetailsView_h
#define YQPkgGenericDetailsView_h


#include <zypp-core/Date.h>

#include "YQZypp.h"
#include <QTextBrowser>


class QTabWidget;

using std::string;


/**
 * Abstract base class for details views. Handles generic stuff like HTML
 * formatting, Qt slots and display only if this view is visible at all: It may
 * be hidden if it's part of a QTabWidget.
 **/
class YQPkgGenericDetailsView : public QTextBrowser
{
    Q_OBJECT

protected:

    /**
     * Constructor.
     **/
    YQPkgGenericDetailsView( QWidget * parent );

    /**
     * Destructor.
     **/
    virtual ~YQPkgGenericDetailsView();


public:

    /**
     * Return the minimum size required for this widget.
     * Inherited from QWidget.
     **/
    virtual QSize minimumSizeHint() const;

    /**
     * Start the html tag and set the CSS style
     */
    static QString htmlStart();

    /**
     * Close the html tag.
     **/
    static QString htmlEnd();

    /**
     * Return a uniform heading in HTML format for the specified selectable:
     *
     * name and summary
     * or name, version and summary, if 'showVersion' is 'true'
     **/
    static QString htmlHeading( ZyppSel selectable, bool showVersion = false );

    /**
     * Escape characters special to HTML in a (plain text) string, such as:
     * '<'  ->  '&lt;'
     * '>'  ->  '&gt;'
     * '&'  ->  '&amp;'
     *
     * ...and return the escaped string.
     **/
    static QString htmlEscape( const QString & plainText );

    /**
     * Return a string containing a HTML table with 'contents'.
     **/
    static QString table( const QString & contents );

    /**
     * Return a string containing a HTML table row with 'contents'.
     **/
    static QString row( const QString & contents );

    /**
     * Return a string containing a HTML table cell with 'contents'.
     **/
    static QString cell( const QString &     contents );
    static QString cell( const std::string & contents );
    static QString cell( int                 contents );
    static QString cell( const zypp::Date &  date     );

    /**
     * Return a string containing a HTML table cell with 'contents'
     * for table headers.
     **/
    static QString hcell( QString contents );


public slots:

    /**
     * Show details for the specified package.
     * Delayed ( optimized ) display if this is embedded into a QTabWidget
     * parent: In this case, wait until this page becomes visible.
     **/
    void showDetailsIfVisible( ZyppSel selectable );

    // slot clear() inherited from QTextEdit

    /**
     * Show details for the specified package.
     * Reimplement this in derived classes.
     **/
    virtual void showDetails( ZyppSel selectable ) = 0;



protected slots:

    void reloadTab( int newCurrent );

    virtual void reload() { QTextBrowser::reload(); }


protected:

    QTabWidget * _parentTab;
    ZyppSel      _selectable;
};


#endif // ifndef YQPkgGenericDetailsView_h
