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


#ifndef YQPkgClassificationFilterView_h
#define YQPkgClassificationFilterView_h

#include "YQZypp.h"
#include <QTreeWidget>


typedef enum
{
    YQPkgClassNone,             // Not listed in the widget
    YQPkgClassSuggested,
    YQPkgClassRecommended,
    YQPkgClassOrphaned,
    YQPkgClassUnneeded,
    YQPkgClassMultiversion,
    YQPkgClassRetracted,
    YQPkgClassRetractedInstalled,
    YQPkgClassAll

} YQPkgClass;


/**
 * Filter view for package classes (categories) like suggested, recommended,
 * orphaned etc. packages. See enum YPkgClass.
 **/
class YQPkgClassificationFilterView : public QTreeWidget
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgClassificationFilterView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgClassificationFilterView();

    /**
     * Check if 'pkg' matches the selected package class and send a filterMatch
     * signal if it does.
     *
     * Returns 'true' if there is a match, 'false' otherwise.
     **/
    bool check( ZyppSel	selectable, ZyppPkg pkg );

    /**
     * Check if 'pkg' matches the selected package class.
     * Returns 'true' if there is a match, 'false' otherwise.
     **/
    bool checkMatch( ZyppSel selectable, ZyppPkg pkg );

    /**
     * Returns the currently selected YQPkgClass
     **/
    YQPkgClass selectedPkgClass() const;

    /**
     * Show the specified package class, i.e. select that filter.
     **/
    void showPkgClass( YQPkgClass pkgClass );


public slots:

    /**
     * Notification that a new filter is the one to be shown.
     **/
    void showFilter( QWidget * newFilter );

    /**
     * Filter according to the view's rules and current selection.
     * Emits those signals:
     *	  filterStart()
     *	  filterMatch() for each pkg that matches the filter
     *	  filterFinished()
     **/
    void filter();


signals:

    /**
     * Emitted when the filtering starts. Use this to clear package lists
     * etc. prior to adding new entries.
     **/
    void filterStart();

    /**
     * Emitted during filtering for each pkg that matches the filter.
     **/
    void filterMatch( ZyppSel selectable, ZyppPkg pkg );

    /**
     * Emitted when filtering is finished.
     **/
    void filterFinished();


protected slots:

    void slotSelectionChanged( QTreeWidgetItem * newSelection );


protected:

    void fillPkgClasses();

};


class YQPkgClassItem: public QTreeWidgetItem
{
public:

    YQPkgClassItem( QTreeWidget * parentView,
		    YQPkgClass    pkgClass );

    virtual ~YQPkgClassItem();

    YQPkgClass pkgClass() const { return _pkgClass; }

    virtual bool operator< ( const QTreeWidgetItem & otherListViewItem ) const override;

private:

    YQPkgClass _pkgClass;
};


#endif // ifndef YQPkgClassificationFilterView_h
