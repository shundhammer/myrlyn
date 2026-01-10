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


#ifndef ZyppHistoryBrowser_h
#define ZyppHistoryBrowser_h

#include <QDialog>
#include "ZyppHistoryEvents.h"

class QTreeWidgetItem;


// Generated with 'uic' from a Qt designer .ui form: zypp-history-browser.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_zypp-history-browser.h
// for the variable names of the widgets.

#include "ui_zypp-history-browser.h"


/**
 * Dialog to browser the zypp history.
 **/
class ZyppHistoryBrowser: public QDialog
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    ZyppHistoryBrowser( QWidget * parent = 0 );

    /**
     * Destructor
     **/
    virtual ~ZyppHistoryBrowser();


protected slots:

    /**
     * Notification that the user clicked on an item in the timeline
     * (navigation) tree.
     **/
    void timeLineClicked( QTreeWidgetItem * item );

    /**
     * Populate the events tree with the same content as before,
     * but possibly different settings.
     **/
    void rePopulateEventsTree();


protected:

    void connectWidgets();
    void populate();
    void populateTimeLineTree();

    /**
     * Populate the events tree with events that match the given date.
     **/
    void populateEventsTree( const QString & date );

    /**
     * Return the number of days in the given month (1..12) of the specified
     * year.
     **/
    int  daysInMonth( int year, int month ) const;

    /**
     * Return 'true' if any item in 'stringList' starts with 'searchText',
     * 'false' otherwise.
     **/
    bool anyItemstartsWith( const QString     & searchText,
                            const QStringList & stringList ) const;

    /**
     * Create a QTreeWidgetItem for the given zypp history event and add it as
     * a child item to 'parentItem' if specified, or as a toplevel item to the
     * event tree widget.
     *
     * Events that have child events (like CommandEvent) will call this
     * recursively.
     **/
    void addEventItem( ZyppHistoryEvents::Event * event,
                       QTreeWidgetItem          * parentItem = 0 );


    void fillCommandItem( QTreeWidgetItem * item, ZyppHistoryEvents::Event * event );
    void fillPkgItem    ( QTreeWidgetItem * item, ZyppHistoryEvents::Event * event );
    void fillRepoItem   ( QTreeWidgetItem * item, ZyppHistoryEvents::Event * event );
    void fillPatchItem  ( QTreeWidgetItem * item, ZyppHistoryEvents::Event * event );

    void readSettings();
    void writeSettings();


    // Data members

    Ui::ZyppHistoryBrowser * _ui;  // see ui_zypp-history-browser.h
};


#endif // ZyppHistoryBrowser_h
