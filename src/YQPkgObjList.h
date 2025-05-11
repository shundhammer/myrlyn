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


#ifndef YQPkgObjList_h
#define YQPkgObjList_h

#include <QColor>
#include <QPixmap>
#include <QRegularExpression>
#include <QMenu>
#include <QEvent>

#include <list>
#include <string>

#include <zypp/ResTraits.h>
#include <zypp/ui/Selectable.h>
#include <zypp/ui/Status.h>

#include "FSize.h"
#include "QY2ListView.h"
#include "YQZypp.h"


class YQPkgObjListItem;
class QAction;

using std::string;


/**
 * Abstract base class to display a list of zypp::ResObjects.
 * Handles most generic stuff like setting status etc.
 **/
class YQPkgObjList : public QY2ListView
{
    Q_OBJECT

protected:

    /**
     * Constructor. Does not add any QListView columns!
     **/
    YQPkgObjList( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgObjList();

    // avoiding warning about virtuals
    using QTreeWidget::currentItemChanged;


public:

    // Column numbers

    int iconCol()        const { return _iconCol;        }
    int statusCol()      const { return _statusCol;      }
    int nameCol()        const { return _nameCol;        }
    int summaryCol()     const { return _summaryCol;     }
    int sizeCol()        const { return _sizeCol;        }
    int versionCol()     const { return _versionCol;     }
    int instVersionCol() const { return _instVersionCol; }


    // Text colors

    QColor normalTextColor() { return _normalTextColor; }
    QColor blueTextColor()   { return _blueTextColor;   }
    QColor redTextColor()    { return _redTextColor;    }


    /**
     * Return whether or not items in this list are generally editable,
     * i.e. the user can change their status. Note that individual items can be
     * set to non-editable even if the list is generally editable.
     * Lists are editable by default.
     **/
    bool editable() const { return _editable; }

    /**
     * Set the list's editable status.
     **/
    void setEditable( bool editable = true ) { _editable = editable; }

    /**
     * Sets the currently selected item's status.
     * Automatically selects the next item if 'selectNextItem' is 'true'.
     **/
    void setCurrentStatus( ZyppStatus newStatus,
                           bool       selectNextItem = false,
                           bool       ifNewerOnly    = false );


    /**
     * Sets the status of all (toplevel) list items to 'newStatus', if possible.
     * Only one single statusChanged() signal is emitted.
     *
     * 'force' overrides sensible defaults like setting only zypp::ResObjects to
     * 'update' that really come with a newer version.
     **/
    void setAllItemStatus( ZyppStatus newStatus, bool force = false );

    /**
     * Add a submenu "All in this list..." to 'menu'.
     * Returns the newly created submenu.
     **/
    virtual QMenu * addAllInListSubMenu( QMenu * menu );

    /**
     * Returns the suitable icon for a zypp::ResObject status - the regular
     * icon if 'enabled' is 'true' or the insensitive icon if 'enabled' is
     * 'false.  'bySelection' is relevant only for auto-states: This uses the
     * icon for 'auto-by-selection" rather than the default auto-icon.
     **/
    virtual QPixmap statusIcon( ZyppStatus status,
                                bool       enabled     = true,
                                bool       bySelection = false );

    /**
     * Returns a short (one line) descriptive text for a zypp::ResObject status.
     **/
    virtual QString statusText( ZyppStatus status ) const;


    class ExcludeRule;
    typedef std::list<ExcludeRule *> ExcludeRuleList;


    /**
     * Add an exclude rule to this list.
     **/
    void addExcludeRule( YQPkgObjList::ExcludeRule * rule );

    /**
     * Apply all exclude rules of this list to all items,
     * including those that are currently excluded.
     **/
    void applyExcludeRules();

    /**
     * Apply all exclude rules of this list to one item.
     **/
    void applyExcludeRules( QTreeWidgetItem * );

    /**
     * Exclude or include an item, i.e. remove it from the visible items
     * and add it to the internal exclude list or vice versa.
     **/
    void exclude( YQPkgObjListItem * item, bool exclude );

    /**
     * Make the inherited QTreeWidget::itemFromIndex() method public
     **/
    QTreeWidgetItem * itemFromIndex ( const QModelIndex & index ) const
        { return QY2ListView::itemFromIndex(index); }


public slots:

    /**
     * Add a zypp::ResObject to the list. Connect a filter's filterMatch()
     * signal to this slot. Remember to connect filterStart() to clear()
     * (inherited from QListView).
     *
     * 'zyppObj' has to be one of the objects of 'selectable'. If it is 0,
     * selectable->theObject() will be used.
     *
     * Intentionally NOT named addItem() so the calling class cannot confuse
     * this method with overlaid methods of the same name that were simply
     * forgotten to implement!
     **/
    void addPkgObjItem( ZyppSel selectable,
                        ZyppObj zyppObj = 0 );

    /**
     * Add a purely passive list item that has a name and optional summary and
     * size.
     **/
    void addPassiveItem( const QString & name,
                         const QString & summary = QString(),
                         FSize           size    = -1 );

    /**
     * Dispatcher slot for mouse click: cycle status depending on column.
     **/
    virtual void pkgObjClicked( int               button,
                                QTreeWidgetItem * item,
                                int               col,
                                const QPoint &    pos );

    /**
     * Reimplemented from QY2ListView:
     * Emit currentItemChanged() signal after clearing the list.
     **/
    virtual void clear() override;

    /**
     * Reset all content: Update the status and content for each item
     **/
    virtual void resetContent();

    /**
     * Update the internal actions for the currently selected item ( if any ).
     * This only calls updateActions( YQPkgObjListItem * ) with the currently
     * selected item as argument, so there is normally no need to reimplement
     * this method, too, if the other one is reimplemented.
     **/
    virtual void updateActions( YQPkgObjListItem * item = 0);

    /**
     * Emit an updatePackages() signal.
     **/
    void sendUpdatePackages() { emit updatePackages(); }

    /**
     * Select the next item, i.e. move the selection one item further down the
     * list.
     **/
    void selectNextItem();

    /**
     * Emit a statusChanged() signal for the specified zypp::ResObject.
     **/
    void sendStatusChanged() { emit statusChanged(); }

    /**
     * Display a one-line message in the list.
     **/
    virtual void message( const QString & text );

    /**
     * Write statistics about excluded items to the log, if there are any.
     **/
    void logExcludeStatistics();

    /**
     * Set the keyboard focus to this list unless the focus is currently on a
     * similar widget that relies on heavy keyboard interaction, like another
     * list.
     **/
    void maybeSetFocus();


    // Direct access to some states for menu actions

    void setCurrentInstall()       { setCurrentStatus( S_Install        ); }
    void setCurrentDontInstall()   { setCurrentStatus( S_NoInst         ); }
    void setCurrentKeepInstalled() { setCurrentStatus( S_KeepInstalled  ); }
    void setCurrentDelete()        { setCurrentStatus( S_Del            ); }
    void setCurrentUpdate()        { setCurrentStatus( S_Update, false, true ); }
    void setCurrentUpdateForce()   { setCurrentStatus( S_Update         ); }
    void setCurrentTaboo()         { setCurrentStatus( S_Taboo          ); }
    void setCurrentProtected()     { setCurrentStatus( S_Protected      ); }

    void setListInstall()          { setAllItemStatus( S_Install        ); }
    void setListDontInstall()      { setAllItemStatus( S_NoInst         ); }
    void setListKeepInstalled()    { setAllItemStatus( S_KeepInstalled  ); }
    void setListDelete()           { setAllItemStatus( S_Del            ); }
    void setListUpdate()           { setAllItemStatus( S_Update         ); }
    void setListUpdateForce()      { setAllItemStatus( S_Update, true   ); }
    void setListTaboo()            { setAllItemStatus( S_Taboo          ); }
    void setListProtected()        { setAllItemStatus( S_Protected      ); }


protected slots:

    /**
     * Dispatcher slot for selection change - internal only.
     **/
    virtual void currentItemChangedInternal( QTreeWidgetItem * item );

    /**
     * slot that shows context menu when requested
     **/
    void slotCustomContextMenu(const QPoint& pos);


signals:

    /**
     * Emitted when a zypp::ui::Selectable is selected.
     * May be called with a null poiner if no zypp::ResObject is selected.
     **/
    void currentItemChanged( ZyppSel selectable );

    /**
     * Emitted when the status of a zypp::ResObject is changed.
     **/
    void statusChanged();

    /**
     * Emitted when it's time to update displayed package information,
     * e.g., package states.
     **/
    void updatePackages();


protected:

    /**
     * Initialize the internally used colors depending on the widget theme
     * (light or dark).
     **/
    void initColors();

    /**
     * Event handler for keyboard input.
     * Only very special keys are processed here.
     *
     * Reimplemented from QListView / QWidget.
     **/
    virtual void keyPressEvent( QKeyEvent * ev ) override;

    /**
     * Returns the context menu for items that are not installed.
     * Creates the menu upon the first call.
     **/
    virtual QMenu * installedContextMenu();

    /**
     * Returns the context menu for items that are installed.
     * Creates the menu upon the first call.
     **/
    virtual QMenu * notInstalledContextMenu();

    /**
     * Return 'true' if 'widget' should keep the keyboard focus, i.e. it's also
     * some kind of list widget that has internal navigation with the cursor
     * keys; like another list or tree widget.
     **/
    bool shouldKeepFocus( QWidget * widget ) const;

    /**
     * Create the context menu for items that are not installed.
     **/
    virtual void createNotInstalledContextMenu();

    /**
     * Create the context menu for installed items.
     **/
    virtual void createInstalledContextMenu();

    /**
     * Create the actions for the context menus.
     * Note: This is intentionally not virtual!
     **/
    void createActions();

    /**
     * Create an action based on a zypp::ResObject status - automatically
     * retrieve the corresponding status icons (both sensitive and insensitive)
     * and text.  'key' is only a descriptive text, no true accelerator.
     **/
    QAction * createAction( ZyppStatus      status,
                            const QString & key     = QString(),
                            bool            enabled = false );

    /**
     * Low-level: Create an action.
     * 'key' is only a descriptive text, no true accelerator.
     **/
    QAction * createAction( const QString & text,
                            const QPixmap & icon            = QPixmap(),
                            const QPixmap & insensitiveIcon = QPixmap(),
                            const QString & key             = QString(),
                            bool            enabled         = false );

    // Data members

    int     _iconCol;
    int     _statusCol;
    int     _nameCol;
    int     _summaryCol;
    int     _sizeCol;
    int     _versionCol;
    int     _instVersionCol;

    QColor  _normalTextColor;
    QColor  _redTextColor;
    QColor  _blueTextColor;

    bool    _editable;
    bool    _debug;
    int     _excludedItemsCount;

    QMenu * _installedContextMenu;
    QMenu * _notInstalledContextMenu;

    ExcludeRuleList _excludeRules;


public:

    QAction * actionSetCurrentInstall;
    QAction * actionSetCurrentDontInstall;
    QAction * actionSetCurrentKeepInstalled;
    QAction * actionSetCurrentDelete;
    QAction * actionSetCurrentUpdate;
    QAction * actionSetCurrentUpdateForce;
    QAction * actionSetCurrentTaboo;
    QAction * actionSetCurrentProtected;

    QAction * actionSetListInstall;
    QAction * actionSetListDontInstall;
    QAction * actionSetListKeepInstalled;
    QAction * actionSetListDelete;
    QAction * actionSetListUpdate;
    QAction * actionSetListUpdateForce;
    QAction * actionSetListTaboo;
    QAction * actionSetListProtected;
};



class YQPkgObjListItem: public QY2ListViewItem
{
public:

    /**
     * Constructor for root items: Creates a YQPkgObjList item that corresponds
     * to the ZYPP selectable that 'selectable' refers to. 'zyppObj' has to be
     * one object of 'selectable'.  If it is 0, selectable->theObject() will be
     * used.
     **/
    YQPkgObjListItem( YQPkgObjList * pkgObjList,
                      ZyppSel        selectable,
                      ZyppObj        zyppObj = 0 );

    /**
     * Constructor for root items: Creates a YQPkgObjList item that does not
     * correspond to a ZYPP selectable. You need to overload _AND RUN_ init
     * yourself then.
     **/
    YQPkgObjListItem( YQPkgObjList * pkgObjList );


protected:

    /**
     * Constructor for non-root items.
     **/
    YQPkgObjListItem( YQPkgObjList *    pkgObjList,
                      QY2ListViewItem * parent,
                      ZyppSel           selectable,
                      ZyppObj           zyppObj = 0 );

public:

    /**
     * Destructor
     **/
    virtual ~YQPkgObjListItem();

    /**
     * Returns the original selectable within the package manager backend.
     **/
    ZyppSel selectable() const { return _selectable; }

    /**
     * Returns the original object within the package manager backend.
     **/
    ZyppObj zyppObj() const { return _zyppObj; }

    /**
     * Return whether or not this items is editable, i.e. the user can change
     * its status. This requires the corresponding list to be editable, too.
     * Items are editable by default.
     **/
    bool editable() const { return _editable; }

    /**
     * Set this item's editable status.
     **/
    void setEditable( bool editable = true ) { _editable = editable; }

    /**
     * Returns the (binary RPM) package status
     **/
    virtual ZyppStatus status() const;

    /**
     * Returns 'true' if this selectable's status is set by a selection
     * (rather than by the user or by the dependency solver).
     **/
    virtual bool bySelection() const;

    /**
     * Set the (binary RPM) package status.
     *
     * If 'sendSignals' is 'true' (default), the parent list will be requested
     * to send update signals. List operations might want to use this for
     * optimizations to send the signals only once after all changes are done.
     **/
    virtual void setStatus( ZyppStatus newStatus, bool sendSignals = true );


    /**
     * Set a status icon according to the package's status.
     **/
    virtual void setStatusIcon();

    /**
     * Update this item's status.
     * Triggered by QY2ListView::updateAllItemStates().
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual void updateStatus() override;

    /**
     * Cycle the package status to the next valid value.
     **/
    virtual void cycleStatus();

    /**
     * Check if the candidate is newer than the installed version.
     **/
    bool candidateIsNewer() const { return _candidateIsNewer; }

    /**
     * Check if the installed version is newer than the candidate.
     **/
    bool installedIsNewer() const { return _installedIsNewer; }

    /**
     * Display this item's notify text (if there is any) that corresponds to
     * the specified status (S_Install, S_Del) in a pop-up window.
     **/
    void showNotifyTexts( ZyppStatus status );

    /**
     * Display a selectable's license agreement (if there is any) that
     * corresponds to its current status (S_Install, S_Update) in a pop-up
     * window.
     *
     * Returns 'true' if the user agreed to that license , 'false' otherwise.
     * The item's status may have changed to S_Taboo, S_Proteced or S_Del if
     * the user disagreed with the license.
     **/
    static bool showLicenseAgreement( ZyppSel sel );

    /**
     * Display this item's license agreement (if there is any) that corresponds
     * to its current status (S_Install, S_Update) in a pop-up window.
     **/
    bool showLicenseAgreement();

    /**
     * Comparison operator for sorting.
     *
     * Reimplemented from QY2ListViewItem.
     */
    virtual bool operator< ( const QTreeWidgetItem & other ) const override;

    /**
     * Calculate a numerical value to compare versions, based on version
     * relations:
     *
     * - Installed newer than candidate (red)
     * - Candidate newer than installed (blue) - worthwhile updating
     * - Installed
     * - Not installed, but candidate available
     **/
    int versionPoints() const;

    /**
     * Update this item's data completely.
     * Triggered by QY2ListView::updateAllItemData().
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual void updateData() override;

    /**
     * Returns a tool tip text for a specific column of this item.
     * 'column' is -1 if the mouse pointer is in the tree indentation area.
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual QString toolTip( int column ) override;

    /**
     * Returns 'true' if this item is excluded.
     **/
    bool isExcluded() const { return _excluded; }

    /**
     * Set this item's exclude flag.
     * Note that this is just a marker. It is the caller's responsibility
     * to add or remove it from exclude lists etc.
     **/
    void setExcluded( bool exclude = true );


    // Columns

    int statusCol()        const { return _pkgObjList->statusCol();        }
    int nameCol()          const { return _pkgObjList->nameCol();          }
    int summaryCol()       const { return _pkgObjList->summaryCol();       }
    int sizeCol()          const { return _pkgObjList->sizeCol();          }
    int versionCol()       const { return _pkgObjList->versionCol();       }
    int instVersionCol()   const { return _pkgObjList->instVersionCol();   }


    // Text colors

    QColor normalTextColor() { return _pkgObjList->normalTextColor(); }
    QColor blueTextColor()   { return _pkgObjList->blueTextColor();   }
    QColor redTextColor()    { return _pkgObjList->redTextColor();    }


protected:

    /**
     * Initialize internal data and set fields accordingly. Only works for items
     * presenting selectables - see YQPkgObjListItem
     **/
    void init();

    /**
     * Apply changes hook. This is called each time the user changes the status
     * of a list item manually (if the old status is different from the new
     * one). Insert code to propagate changes to other objects here, for
     * example to trigger a "small" solver run (Resolver::transactObjKind()
     * etc.).
     *
     * This default implementation does nothing.
     **/
    virtual void applyChanges() {}

    /**
     * Do a "small" solver run for all "resolvable collections", i.e., for
     * selections, patterns, languages, patches.
     **/
    void solveResolvableCollections();

    /**
     * Set a column text via STL string.
     * ( QListViewItem::setText() expects a QString! )
     **/
    void setText( int column, const string & text );

public:
    /**
     * Re-declare ordinary setText() method so the compiler doesn't get
     * confused which one to use.
     **/
    void setText( int column, const QString & text )
        { QTreeWidgetItem::setText( column, text ); }

protected:
    /**
     * Set a column text via Edition.
     **/
    void setText( int column, const zypp::Edition & edition );


    //
    // Data members
    //

    YQPkgObjList * _pkgObjList;
    ZyppSel        _selectable;
    ZyppObj        _zyppObj;
    bool           _editable:1;
    bool           _candidateIsNewer:1;
    bool           _installedIsNewer:1;
    bool           _excluded:1;
};



class YQPkgObjList::ExcludeRule
{
public:

    /**
     * Constructor: Creates a new exclude rule with a regular expression
     * to check against the text of the specified column of each list
     * entry.
     *
     * The parent YQPkgObjList will assume ownership of this exclude rule
     * and destroy it when the parent is destroyed.
     **/
    ExcludeRule( YQPkgObjList *             parent,
                 const QRegularExpression & regexp,
                 int                        column = 0 );


    // Intentionally omitting virtual destructor:
    // No allocated objects, no other virtual methods,
    // no need to have a vtable for each instance of this class.
    //
    // virtual ~ExcludeRule();

    /**
     * Enable or disable this exclude rule.
     * New exclude rules are enabled by default.
     **/
    void enable( bool enable = true );

    /**
     * Returns 'true' if this exclude rule is enabled,
     * 'false' otherwise.
     **/
    bool isEnabled() const { return _enabled; }

    /**
     * Change the regular expression after creation.
     **/
    void setRegexp( const QRegularExpression & regexp );

    /**
     * Returns the regexp.
     **/
    QRegularExpression regexp() const { return _regexp; };

    /**
     * Change the column number to check against after creation.
     **/
    void setColumn( int column = 0 );

    /**
     * Returns the column number.
     **/
    int column() const { return _column; }

    /**
     * Returns this exclude rule's parent YQPkgObjList.
     **/
    YQPkgObjList * parent() const { return _parent; }

    /**
     * Check a list item against this exclude rule.
     * Returns 'true' if the item matches this exclude rule,
     * i.e. if it should be excluded.
     **/
    bool match( QTreeWidgetItem * item );

private:

    YQPkgObjList *      _parent;
    QRegularExpression  _regexp;
    int                 _column;
    bool                _enabled;
};


#endif // ifndef YQPkgObjList_h
