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


#ifndef YQPkgUpdatesFilterView_h
#define YQPkgUpdatesFilterView_h


#include <QWidget>
#include <QIcon>
#include "YQZypp.h"


// Generated with 'uic' from a Qt designer .ui form: updates-filter-view.ui
//
// Check out ../build/src/myrlyn_autogen/include/ui_updates-filter-view.h
// for the variable names of the widgets.

#include "ui_updates-filter-view.h"



/**
 * Filter view for packages that can be updated with push buttons
 * for "Package Update", "Dist Update", "Refresh List".
 **/
class YQPkgUpdatesFilterView : public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgUpdatesFilterView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgUpdatesFilterView();


    /**
     * Return the number of packages in the pool that have an update available,
     * i.e. that are installed and that have a candidate object that is newer
     * than the installed one.
     **/
    static int countUpdates();

    /**
     * Return 'true' if the specified selectable is installed and has a
     * candidate object that is newer than the installed one.
     **/
    static bool isUpdateAvailableFor( ZyppSel selectable );

    /**
     * Return the preferred size of this widget.
     *
     * Reimplemented from QWidget.
     **/
    virtual QSize sizeHint() const override;

    /**
     * Return the preferred size of this widget.
     *
     * Reimplemented from QWidget.
     **/
    virtual QSize minimumSizeHint() const override;


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
     * Do a package update ('zypper up' equivalent).
     **/
    void doPackageUpdate();

    /**
     * Do a distribution upgrade ('zypper dup' equivalent).
     **/
    void doDistUpgrade();

    /**
     * Refresh the package list.
     * This is just an alias for filter().
     **/
    void refreshList();


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

    void connectWidgets();

    /**
     * Mark all packages in the package list as "left over" (with a sad smiley
     * icon) that is not marked as "update" after a "package update" or "dist
     * upgrade".
     **/
    void markLeftovers();



    // Data members

    Ui::UpdatesFilterView * _ui;
    QIcon                   _leftoverPkgIcon;
    QIcon                   _updateOkIcon;
};



#endif // ifndef YQPkgUpdatesFilterView_h
