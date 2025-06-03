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


#ifndef YQPkgServiceList_h
#define YQPkgServiceList_h

#include <string>
#include "QY2ListView.h"
#include "YQZypp.h"


class YQPkgServiceListItem;

namespace zypp
{
    class RepoManager;
}

typedef std::string ZyppService;


/**
 * A widget to display a list of libzypp services.
 **/
class YQPkgServiceList : public QY2ListView
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgServiceList( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgServiceList();


public slots:

    /**
     * Filter according to the view's rules and current selection.
     * Emits those signals:
     *    filterStart()
     *    filterMatch() for each pkg that matches the filter
     *    filterFinished()
     **/
    void filter();

    /**
     * Add a service to the list.
     **/
    void addService( ZyppService service,
                     const zypp::RepoManager & repoManager );


public:

    // Column numbers

    int nameCol() const { return _nameCol; }

    /**
     * Return the currently selected item or 0 if there is none.
     **/
    YQPkgServiceListItem * selection() const;


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


protected slots:

    /**
     * Fill the list.
     **/
    void fillList();


private:

    int _nameCol;
};


class YQPkgServiceListItem: public QY2ListViewItem
{
public:

    /**
     * Constructor
     **/
    YQPkgServiceListItem( YQPkgServiceList *        parentList,
                          ZyppService               service,
                          const zypp::RepoManager & repoManager );

    /**
     * Destructor
     **/
    virtual ~YQPkgServiceListItem();

    /**
     * Returns the ZYPP service this item corresponds to (its alias)
     **/
    ZyppService zyppService() const { return _zyppService; }

    /**
     * Returns the ZYPP service name this item corresponds to
     **/
    std::string zyppServiceName() const { return _zyppServiceName; }

    /**
     * Returns the parent list
     **/
    const YQPkgServiceList * serviceList() const { return _serviceList; }

    /**
     * Returns the product on a source if it has one single product
     * or 0 if there are no or multiple products.
     **/
    static ZyppProduct singleProduct( ZyppService service );

    /**
     * Comparison operator for sorting.
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual bool operator<( const QTreeWidgetItem & other ) const override;

    /**
     * Return the column for the name.
     **/
    int nameCol() const { return _serviceList->nameCol(); }


protected:

    YQPkgServiceList * _serviceList;
    ZyppService        _zyppService;
    std::string        _zyppServiceName;
};


#endif // ifndef YQPkgServiceList_h
