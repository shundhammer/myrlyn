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


#ifndef QY2ComboTabWidget_h
#define QY2ComboTabWidget_h

#include <QString>
#include <QHash>
#include <QWidget>

class QComboBox;
class QLabel;
class QStackedWidget;
class QWidget;


/**
 * This widget is very much like a QTabWidget, but it uses a combo box above
 * the tab pages rather than a (scrolled) one-line row of tabs.
 **/
class QY2ComboTabWidget : public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor. 'label' is the user-visible label of the combo
     * box that is used to switch between the different tab pages.
     **/
    QY2ComboTabWidget( const QString &	label,
		       QWidget *	parent = 0 );

    /**
     * Destructor.
     **/
    virtual ~QY2ComboTabWidget();

    /**
     * Add a page. 'pageLabel' will be the user-visible combo box entry for
     * that page.
     **/
    void addPage( const QString & pageLabel, QWidget * page );

    /**
     * Declare a previously added page to be the one where the whole widget
     * stacck will be minimized (hidden) whenever that combo box entry is
     * selected.
     *
     * This will usually be an empty page; like the "All Packages" page in
     * YQPkgSecondaryFilter: it doesn't need all that screen space that is
     * reserved for the other secondary filters, so that whole widget tack can
     * be collapsed.
     **/
    void setMinimizePage( QWidget * page );


signals:

    /**
     * Emitted when the current page changes.
     * NOT emitted initially for the very first page that is shown.
     **/
    void currentChanged( QWidget * newCurrentPage );


public slots:

    /**
     * Show a page. Updates the combo box contents accordingly.
     * This is an expensive operation: All combo box items are searched for the
     * item that corresponds to this page.
     **/
    void showPage( QWidget * page );


protected slots:

    /**
     * Show a page identified by its index. Does NOT update the combo box
     * contents.
     **/
    void showPageIndex( int index );


protected:

    QComboBox		* _comboBox;
    QLabel		* _comboLabel;
    QStackedWidget 	* _widgetStack;
    QHash<int, QWidget *> _pages;
    QWidget            *  _minimizePage;
};


#endif // QY2ComboTabWidget_h
