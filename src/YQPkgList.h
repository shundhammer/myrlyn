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


#ifndef YQPkgList_h
#define YQPkgList_h


#include <QMenu>
#include <QResizeEvent>

#include <zypp/Package.h>

#include "YQPkgObjList.h"
#include "YQZypp.h"


class QObject;
class QWidget;


/**
 * Display a list of zypp::Package objects.
 **/
class YQPkgList : public YQPkgObjList
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgList( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgList();

    /**
     * Add a submenu "All in this list..." to 'menu'.
     * Returns the newly created submenu.
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual QMenu * addAllInListSubMenu( QMenu * menu ) override;

    /**
     * Returns 'true' if there are any installed packages.
     **/
    static bool haveInstalledPkgs();

    /**
     * Set the status of all packages in the pool to a new value.
     * This is not restricted to the current content of this package list.
     * All selectables in the ZYPP pool are affected.
     *
     * 'force' indicates if it should be done even if it is not very useful,
     * e.g., if packages should be updated even if there is no newer version.
     *
     * If 'countOnly' is 'true', the status is not actually changed, only the
     * number of packages that would be affected is return.
     *
     * Return value: The number of status changes
     **/
    int globalSetPkgStatus( ZyppStatus newStatus, bool force, bool countOnly );


    /**
     * Reimplemented from QListView / QWidget:
     * Reserve a reasonable amount of space.
     **/
    virtual QSize sizeHint() const override;


public slots:

    /**
     * Add a pkg to the list. Connect a filter's filterMatch() signal to this
     * slot. Remember to connect filterStart() to clear() (inherited from
     * QListView).
     **/
    void addPkgItem( ZyppSel selectable,
                     ZyppPkg zyppPkg );

    /**
     * Add a pkg to the list, but display it dimmed (grey text foreground
     * rather than normal black).
     **/
    void addPkgItemDimmed( ZyppSel selectable,
                           ZyppPkg zyppPkg );

    /**
     * Add a pkg to the list
     **/
    void addPkgItem( ZyppSel selectable,
                     ZyppPkg zyppPkg,
                     bool    dimmed );

    // No separate currentItemChanged( ZyppPkg ) signal:
    //
    // Use YQPkgObjList::currentItemChanged( ZyppObj ) instead and dynamic_cast
    // to ZyppPkg if required.  This saves duplicating a lot of code.

    /**
     * Clear the widget and reinitialize internal state.
     *
     * Reimplemented from QPkgObjList.
     **/
    virtual void clear() override;

    /**
     * Sort the tree widget again according to the column selected and
     * its current sort order.
     **/
    void resort();


protected slots:

    /**
     * Sort the packages by the specified column.
     *
     * This is a successor to the old 'sortByColumn( int )' slot
     * (Qt 3.x, 4.x, 5.x) that received an additional parameter in Qt 6.x.
     **/
    void sortPackages( int column );


protected:

    /**
     * Create the context menu for items that are not installed.
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual void createNotInstalledContextMenu() override;

    /**
     * Create the context menu for installed items.
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual void createInstalledContextMenu() override;

    /**
     * Resets the optimal column width values.
     * Needed for empty list.
     **/
    void resetBestColWidths();

    /**
     * Set and save optimal column widths depending on content only.
     *
     * There is currently no way to get the optimal widths without setting
     * them, so we have to do it.
     **/
    void updateBestColWidths( ZyppSel selectable,
                              ZyppPkg zyppPkg );

    /**
     * Optimizes the column widths depending on content and the available
     * horizontal space.
     **/
    void optimizeColumnWidths();

    /**
     * Handler for resize events.
     * Triggers column width optimization.
     **/
    void resizeEvent(QResizeEvent *event) override;


    //
    // Data members
    //

    int _bestStatusColWidth;
    int _bestNameColWidth;
    int _bestSummaryColWidth;
    int _bestVersionColWidth;
    int _bestInstVersionColWidth;
    int _bestSizeColWidth;
};



class YQPkgListItem: public YQPkgObjListItem
{
public:

    /**
     * Constructor. Creates a YQPkgList item that corresponds to the package
     * manager object that 'pkg' refers to.
     **/
    YQPkgListItem( YQPkgList *  pkgList,
                   ZyppSel      selectable,
                   ZyppPkg      zyppPkg );

    /**
     * Destructor
     **/
    virtual ~YQPkgListItem();

    /**
     * Returns the parent package list.
     **/
    YQPkgList * pkgList() { return _pkgList; }

    /**
     * Returns the original object within the package manager backend.
     **/
    ZyppPkg zyppPkg() const { return _zyppPkg; }

    /**
     * Update this item's data completely.
     * Triggered by QY2ListView::updateAllItemData().
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual void updateData() override;

    /**
     * Returns a tool tip text for a specific column of this item.
     * 'column' is -1 if the mouse pointer is in the tree indentation area.
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual QString toolTip( int column ) override;

    /**
     * Returns true if this package is to be displayed dimmed,
     * i.e. with grey text foreground rather than the normal black.
     **/
    bool isDimmed() const { return _dimmed; }

    /**
     * Set the 'dimmed' flag.
     **/
    void setDimmed( bool d = true ) { _dimmed = d; }


protected:

    YQPkgList * _pkgList;
    ZyppPkg     _zyppPkg;
    bool        _dimmed;
};


#endif // ifndef YQPkgList_h
