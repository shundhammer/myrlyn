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


#ifndef YQPkgStatusFilterView_h
#define YQPkgStatusFilterView_h

#include <QWidget>
#include "YQZypp.h"


// Generated with 'uic' from a Qt designer .ui form: status-filter-view.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_status-filter-view.h
// for the variable names of the widgets.

#include "ui_status-filter-view.h"


/**
 * Filter view for packages by status
 **/
class YQPkgStatusFilterView : public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgStatusFilterView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgStatusFilterView();

    /**
     * Check if pkg matches the filter criteria.
     **/
    bool check( ZyppSel selectable,
                ZyppObj pkg );

    /**
     * Return 'true' if this view is showing any automatic package changes, so
     * showing an additional dialog for that is not needed.
     **/
    bool showingAutomaticChanges() const;


public slots:

    /**
     * Notification that a new filter is the one to be shown.
     **/
    void showFilter( QWidget * newFilter );

    /**
     * Filter according to the view's rules and current selection.
     * Emits those signals:
     *    filterStart()
     *    filterMatch() for each pkg that matches the filter
     *    filterFinished()
     **/
    void filter();

    /**
     * Reset all check boxes: Set them all to the default values.
     **/
    void clear();

    /**
     * Set up the check boxes so all pending transactions are displayed.
     **/
    void showTransactions();

    /**
     * Read settings from the config file.
     **/
    void readSettings();

    /**
     * Write settings to the config file.
     **/
    void writeSettings();


signals:

    /**
     * Emitted when the filtering starts. Use this to clear package lists
     * etc. prior to adding new entries.
     **/
    void filterStart();

    /**
     * Emitted during filtering for each pkg that matches the filter.
     **/
    void filterMatch( ZyppSel selectable,
                      ZyppPkg pkg );

    /**
     * Emitted when filtering is finished.
     **/
    void filterFinished();


protected:

    /**
     * Set up signal / slot connections.
     **/
    void connectWidgets();

    /**
     * Replace the icons from the compiled-in Qt resources from the .ui file
     * with icons from the desktop theme.
     **/
    void fixupIcons();



    // Data members

    Ui::StatusFilterView * _ui;
};



#endif // ifndef YQPkgStatusFilterView_h
