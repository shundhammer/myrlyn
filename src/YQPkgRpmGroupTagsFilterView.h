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


#ifndef YQPkgRpmGroupTagsFilterView_h
#define YQPkgRpmGroupTagsFilterView_h

#include "YQZypp.h"
#include <QTreeWidget>
#include <YRpmGroupsTree.h>

class YQPkgRpmGroupTag;


/**
 * RPM group tags filter view: Display the RPM group tags tree and emit
 * signals if any group tag is selected so a package list can be filled or
 * updated.
 **/
class YQPkgRpmGroupTagsFilterView : public QTreeWidget
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgRpmGroupTagsFilterView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgRpmGroupTagsFilterView();

    /**
     * Returns the currently selected item or 0 if there is none.
     **/
    YQPkgRpmGroupTag * selection() const;

    /**
     * Check if 'pkg' matches the selected RPM group.
     * Returns true if there is a match, false otherwise.
     **/
    bool check( ZyppSel selectable,
                ZyppPkg pkg );

    /**
     * Returns the (untranslated!) currently selected RPM group as string.
     * Special case: "*" is returned if "zzz_All" is selected.
     **/
    const std::string & selectedRpmGroup() const { return _selectedRpmGroup; }


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
     * Select a list entry (if there is any).
     * Usually this will be the first list entry, but don't rely on that - this
     * might change without notice. Emits signal currentItemChanged().
     **/
    void selectSomething();

    /**
     * Returns the internal RPM groups tree and fills it
     * if it doesn't exist yet.
     **/
    static YRpmGroupsTree * rpmGroupsTree();


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


protected slots:

    /**
     * Update _selectedRpmGroup and filter data
     **/
    void slotSelectionChanged( QTreeWidgetItem * newSelection );


protected:

    /**
     * Fill the internal RPM groups tree with RPM groups of all packages
     * currently in the pool
     **/
    static void fillRpmGroupsTree();

    /**
     * Recursively clone the RPM group tag tree for the QListView widget:
     * Make a deep copy of the tree starting at 'parentRpmGroup' and
     * 'parentClone'.
     **/
    void cloneTree( YStringTreeItem *   parentRpmGroup,
                    YQPkgRpmGroupTag *  parentClone = 0 );

    //
    // Data members
    //

    std::string _selectedRpmGroup;

    static YRpmGroupsTree * _rpmGroupsTree;
};



class YQPkgRpmGroupTag: public QTreeWidgetItem
{
public:

    /**
     * Constructor for toplevel RPM group tags
     **/
    YQPkgRpmGroupTag( YQPkgRpmGroupTagsFilterView * parentFilterView,
                      YStringTreeItem *             rpmGroup         );

    /**
     * Constructor for RPM group tags that have a parent
     **/
    YQPkgRpmGroupTag( YQPkgRpmGroupTagsFilterView * parentFilterView,
                      YQPkgRpmGroupTag *            parentGroupTag,
                      YStringTreeItem *             rpmGroup        );

    /**
     * Constructor for toplevel RPM group tags via STL string
     * (for special cases like "zzz All")
     **/
    YQPkgRpmGroupTag( YQPkgRpmGroupTagsFilterView * parentFilterView,
                      const QString &               rpmGroupName,
                      YStringTreeItem *             rpmGroup        );

    /**
     * Destructor
     **/
    virtual ~YQPkgRpmGroupTag();


    /**
     * Returns the parent filter view
     **/
    YQPkgRpmGroupTagsFilterView * filterView() const { return _filterView; }

    /**
     * Returns the original tree item
     **/
    const YStringTreeItem * rpmGroup() const { return _rpmGroup; }

    int depth() const { return _depth; }


private:

    // Data members

    YQPkgRpmGroupTagsFilterView * _filterView;
    YStringTreeItem *             _rpmGroup;
    int                           _depth;
};


#endif // ifndef YQPkgRpmGroupTagsFilterView_h
