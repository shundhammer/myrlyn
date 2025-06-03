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


#ifndef RepoTable_h
#define RepoTable_h

#include <string>

#include "MyrlynRepoManager.h"
#include "YQZypp.h"
#include "QY2ListView.h"

class RepoTableItem;


/**
 * Multi-column table widget to show libzypp repositories.
 *
 * This is the table used in the RepoEditor, not to confuse with the simple
 * one-column YQPkgRepoList used in the YQPkgRepoFilterView.
 **/
class RepoTable: public QY2ListView
{
    Q_OBJECT

public:

    /**
     * Table columns
     **/
    enum Col
    {
        NameCol,
        PrioCol,
        EnabledCol,
        AutoRefCol,
        ServiceCol,
        UrlCol
    };


    /**
     * Constructor
     **/
    RepoTable( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~RepoTable();

    /**
     * Return the zypp repo manager.
     **/
    RepoManager_Ptr repoManager() { return _repoManager; }

    /**
     * Populate with the repos from the MyrlynRepoManager.
     **/
    void populate();

    /**
     * Return the current repo item or 0 if there is none.
     **/
    RepoTableItem * currentRepoItem();

    /**
     * Don't let Qt designer get in the way of our desired columns.
     *
     * Don't override; those methods are not virtual in the base class.
     * But Qt designer calls the method of this class.
     **/
    void setHeaderItem( QTreeWidgetItem * headerItem );
    void setColumnCount( int ) {};

protected:

    RepoManager_Ptr _repoManager;
};


class RepoTableItem: public QY2ListViewItem
{
public:

    /**
     * Constructor
     **/
    RepoTableItem( RepoTable *          parentTable,
                   const ZyppRepoInfo & repoInfo );

    /**
     * Destructor
     **/
    virtual ~RepoTableItem();

    /**
     * Return the associated RepoInfo.
     **/
    const ZyppRepoInfo & repoInfo() const { return _repoInfo; }

    /**
     * Update the item from a new RepoInfo.
     **/
    void setRepoInfo( const ZyppRepoInfo & newRepoInfo );

    /**
     * Comparison function used for sorting the list.
     * Reimplemented from QY2ListViewItem / QTreeWidgetItem.
     **/
    virtual bool operator< ( const QTreeWidgetItem & other ) const override;

    /**
     * Set a column text
     **/
    void setText( int col, const std::string & txt );
    void setText( int col, const QString & txt );


protected:

    /**
     * Update the data for this item.
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual void updateData() override;

    /**
     * Return a "checkmark" icon.
     **/
    QPixmap checkmarkIcon();

    /**
     * Return an empty icon.
     **/
    QPixmap noIcon();


    // Data members

    RepoTable *  _parentTable;
    ZyppRepoInfo _repoInfo;
};


#endif // RepoTable_h
