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


#ifndef YQPkgPatchList_h
#define YQPkgPatchList_h

#include <zypp/Patch.h>

#include <string>

#include "QY2ListView.h"
#include "YQPkgObjList.h"
#include "YQZypp.h"

class QMenu;
class QObject;
class QWidget;
class YQPkgPatchListItem;
class YQPkgPatchCategoryItem;


enum YQPkgPatchCategory // This is also the sort order
{
    YQPkgYaSTPatch,
    YQPkgSecurityPatch,
    YQPkgRecommendedPatch,
    YQPkgOptionalPatch,
    YQPkgFeaturePatch,
    YQPkgDocumentPatch,
};


/**
 * Display a list of zypp::Patch objects.
 **/
class YQPkgPatchList : public YQPkgObjList
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgPatchList( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgPatchList();


    enum FilterCriteria
    {
        RelevantPatches,                // needed (or broken)
        RelevantAndInstalledPatches,    // unneeded
        AllPatches                      // all
    };


    /**
     * Check if there are any patches at all.
     *
     * Some distros like openSUSE Tumbleweed or Slowroll don't provide any
     * patches; they are kept up to date with updated packages only.
     * OTOH openSUSE Leap or SLE do get patches.
     **/
    static bool haveAnyPatches();

    /**
     * Check if there are any needed patches in the pool, i.e. patches that are
     * relevant and not installed or satisfied yet.
     **/
    static bool haveNeededPatches();

    /**
     * Return the number of needed patches in the pool, i.e. patches that are
     * relevant and not installed or satisfied yet.
     **/
    static int countNeededPatches();


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
     * Add a patch to the list. Connect a filter's filterMatch() signal to
     * this slot. Remember to connect filterStart() to clear() (inherited from
     * QListView).
     **/
    void addPatchItem( ZyppSel   selectable,
                       ZyppPatch zyppPatch );

    /**
     * Fill the patch list according to filterCriteria().
     **/
    void fillList();

    /**
     * Display a one-line message in the list.
     * Reimplemented from YQPkgObjList.
     **/
    virtual void message( const QString & text ) override;

    /**
     * Select the first selectable list entry that is not a pattern category.
     *
     * Reimplemented from QY2ListView.
     **/
    virtual void selectSomething() override;


public:

    /**
     * Set the filter criteria for fillList().
     **/
    void setFilterCriteria( FilterCriteria filterCriteria );

    /**
     * Returns the current filter criteria.
     **/
    FilterCriteria filterCriteria() const { return _filterCriteria; }

    /**
     * Returns the currently selected item or 0 if there is none.
     **/
    YQPkgPatchListItem * selection() const;

    /**
     * Add a submenu "All in this list..." to 'menu'.
     * Returns the newly created submenu.
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual QMenu * addAllInListSubMenu( QMenu * menu ) override;


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
     * returns or creates a category item for a defined category
     */
    YQPkgPatchCategoryItem * category( YQPkgPatchCategory category );

    /**
     * Return 'true' if 'zyppPatch' is non-null and a needed patch, i.e. a
     * relevant patch that is not installed or satisfied yet.
     *
     * A patch is relevant if the packages that it consists of are installed,
     * but in older versions than the ones that the patch brings.
     **/
    static bool isNeeded( ZyppSel selectable, ZyppPatch zyppPatch );

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
     * Event handler for keyboard input.
     * Only very special keys are processed here.
     *
     * Reimplemented from YQPkgObjList / QWidget.
     **/
    virtual void keyPressEvent( QKeyEvent * ev ) override;


    // Data members

    FilterCriteria _filterCriteria;
    QMap<YQPkgPatchCategory, YQPkgPatchCategoryItem*> _categories;
};



class YQPkgPatchListItem: public YQPkgObjListItem
{
public:

    /**
     * Constructor. Creates a YQPkgPatchList item that corresponds to
     * zyppPatch.
     **/
    YQPkgPatchListItem( YQPkgPatchList * patchList,
                        ZyppSel          selectable,
                        ZyppPatch        zyppPatch );


    /**
     * Constructor. Creates a YQPkgPatchList item that corresponds to
     * zyppPatch.
     **/
    YQPkgPatchListItem( YQPkgPatchList *         patchList,
                        YQPkgPatchCategoryItem * parentCategory,
                        ZyppSel                  selectable,
                        ZyppPatch                zyppPatch );

    /**
     * Destructor
     **/
    virtual ~YQPkgPatchListItem();

    /**
     * Returns the original zyppPatch object.
     **/
    ZyppPatch zyppPatch() const { return _zyppPatch; }

    /**
     * Cycle the package status to the next valid value.
     * Reimplemented from YQPkgObjList.
     **/
    virtual void cycleStatus() override;

    /**
     * Returns a tool tip text for a specific column of this item.
     * 'column' is -1 if the mouse pointer is in the tree indentation area.
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual QString toolTip( int column ) override;

    /**
     * sorting function
     */
    virtual bool operator< ( const QTreeWidgetItem & other ) const override;

    // Columns

    int statusCol()     const   { return _patchList->statusCol();       }
    int summaryCol()    const   { return _patchList->summaryCol();      }

    /**
     * Returns the category of this patch (security, recommended, ...).
     **/
    YQPkgPatchCategory patchCategory() const { return _patchCategory; }

protected:
    void init();
    /**
     * Propagate status changes in this list to other lists:
     * Have the solver transact all patches.
     *
     * Reimplemented from YQPkgObjListItem.
     **/
    virtual void applyChanges() override;


    // Data members

    YQPkgPatchList *   _patchList;
    ZyppPatch          _zyppPatch;
    YQPkgPatchCategory _patchCategory;
};


class YQPkgPatchCategoryItem: public QY2ListViewItem
{
public:

    /**
     * Constructor
     **/
    YQPkgPatchCategoryItem( YQPkgPatchCategory category,
                            YQPkgPatchList *   patternList );

    /**
     * Destructor
     **/
    virtual ~YQPkgPatchCategoryItem();

    /**
     * Returns the first pattern. This should be the first in sort order.
     **/
    ZyppPatch firstPatch() const { return _firstPatch; }

    /**
     * Add a pattern to this category. This method sets firstPatch() if
     * necessary.
     **/
    void addPatch( ZyppPatch patch );

    /**
     * sorting function
     */
    virtual bool operator< ( const QTreeWidgetItem & other ) const;

    /**
     * Maps a string patch category to the corresponding enum.
     **/
    static YQPkgPatchCategory patchCategory( const QString      & category );
    static YQPkgPatchCategory patchCategory( const std::string  & category );

    /**
     * Converts a patch category to a user-readable (translated) string.
     **/
    static QString asString( YQPkgPatchCategory category );


    /**
     * Category (and order ) for this item
     */
    YQPkgPatchCategory category() const { return _category; }


protected:

    /**
     * Set a suitable tree open/close icon depending on this category's
     * open/close status.
     *
     * The default QListView plus/minus icons would require treeStepSize() to
     * be set >0 and rootItemDecorated( true ), but that would look very ugly
     * in this context, so the pattern categories paint their own tree open /
     * close icons.
     **/
    void setTreeIcon( void );


    //
    // Data members
    //

    YQPkgPatchCategory _category;
    YQPkgPatchList *   _patchList;
    ZyppPatch          _firstPatch;
};


#endif // ifndef YQPkgPatchList_h
