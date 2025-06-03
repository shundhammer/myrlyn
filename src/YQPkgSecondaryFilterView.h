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


#ifndef YQPkgSecondaryFilterView_h
#define YQPkgSecondaryFilterView_h

#include "YQZypp.h"
#include <QWidget>

class QY2ComboTabWidget;
class YQPkgSearchFilterView;
class YQPkgStatusFilterView;


/**
 * Abstract base class for filter views containing a secondary filter
 */
class YQPkgSecondaryFilterView: public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor
     * Because of the circular dependencies you have to call the init()
     * method later with the new subclassed object.
     **/
    YQPkgSecondaryFilterView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgSecondaryFilterView();

    /**
     * Initialize the primary widget
     *
     * The primaryWidget is the primary widget to which the secondary views
     * will be added. It should be a subclass of YQPkgSecondaryFilterView.
     */
    void init( QWidget * primaryWidget );

signals:

    /**
     * Emitted when the filtering starts. Use this to clear package lists
     * etc. prior to adding new entries.
     **/
    void filterStart();

    /**
     * Emitted during filtering for each pkg that matches the filter
     * and the candidate package comes from the respective repository
     **/
    void filterMatch( ZyppSel selectable,
                      ZyppPkg pkg );

    /**
     * Emitted during filtering for each pkg that matches the filter
     * and the candidate package does not come from the respective repository
     **/
    void filterNearMatch( ZyppSel selectable,
                          ZyppPkg pkg );

    /**
     * Emitted when filtering is finished.
     **/
    void filterFinished();

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


protected slots:

    /**
     * Propagate a filter match from the primary filter
     * and appy any selected secondary filter(s) to it
     **/
    void primaryFilterMatch( ZyppSel selectable,
                             ZyppPkg pkg );

    /**
     * Propagate a filter near match from the primary filter
     * and appy any selected secondary filter(s) to it
     **/
    void primaryFilterNearMatch( ZyppSel selectable,
                                 ZyppPkg pkg );

protected:

    /**
     * Widget layout for the secondary filters
     **/
    QWidget * layoutSecondaryFilters( QWidget * parent,
                                      QWidget * primaryWidget );

    /**
     * Check if pkg matches the the currently selected secondary filter
     **/
    bool secondaryFilterMatch( ZyppSel selectable,
                               ZyppPkg pkg );

    /**
     * The actual filter method.
     *
     * Derived classes are required to implement this.
     **/
    virtual void primaryFilter() = 0;


    // Data members

    QY2ComboTabWidget *     _secondaryFilters;
    QWidget *               _allPackages;
    YQPkgSearchFilterView * _searchFilterView;
    YQPkgStatusFilterView * _statusFilterView;
};


#endif // ifndef YQPkgSecondaryFilterView_h
