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


#ifndef QY2DiskUsageList_h
#define QY2DiskUsageList_h

#include <QColor>

#include "FSize.h"
#include "QY2ListView.h"


/**
 * Generic scrollable list of disk usage for any number of partitions.
 **/
class QY2DiskUsageList : public QY2ListView
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * Adds a standard set of list columns if 'addStdColumns' is
     *'true'. Otherwise the caller is responsible for adding any columns.
     **/
    QY2DiskUsageList( QWidget * parent, bool addStdColumns = true );

    /**
     * Destructor
     **/
    virtual ~QY2DiskUsageList();


    // Column numbers

    int nameCol()		const	{ return _nameCol;		}
    int percentageBarCol()	const	{ return _percentageBarCol;	}
    int freeSizeCol()		const	{ return _freeSizeCol;		}
    int totalSizeCol()		const	{ return _totalSizeCol;		}


    virtual void drawRow ( QPainter *                   painter,
                           const QStyleOptionViewItem & option,
                           const QModelIndex &          index ) const;

    // Reimplemented to make this public for use in the QItemDelegate
    QTreeWidgetItem * itemFromIndex ( const QModelIndex & index ) const
        { return QY2ListView::itemFromIndex(index); }

protected:

    int _nameCol;
    int _percentageBarCol;
    int _freeSizeCol;
    int _totalSizeCol;
};



/**
 * Abstract base class for one partition ( mount point ) to display in a
 * QY2DiskUsageList.
 *
 * This class contains pure virtuals, so it cannot be used directly.
 **/
class QY2DiskUsageListItem: public QY2ListViewItem
{
protected:
    /**
     * Constructor.
     *
     * Call updateData() after the constructor for the initial display
     * update. Unfortunately, this cannot be done automatically in the
     * constructor since it uses virtual methods that are not available yet at
     * this point.
     **/
    QY2DiskUsageListItem( QY2DiskUsageList * parent );

    /**
     * Destructor.
     **/
    virtual ~QY2DiskUsageListItem();


public:

    /**
     * The currently used size of this partition.
     *
     * Derived classes need to implement this method.
     **/

    virtual FSize usedSize() const = 0;

    /**
     * The total size of this partition.
     *
     * Derived classes need to implement this method.
     **/
    virtual FSize totalSize() const = 0;

    /**
     * The current free size of this partition.
     *
     * Derived classes can choose reimpmenent this if it is less expensive than
     * calculating this value each time from usedSize() and totalSize() which
     * is the default implementation.
     **/
    virtual FSize freeSize() const;

    /**
     * The currently used percentage ( 0..100 ) of this partition.
     *
     * Derived classes can choose reimpmenent this if it is less expensive than
     * calculating this value each time from usedSize() and totalSize() which
     * is the default implementation.
     **/
    virtual int usedPercent() const;

    /**
     * The name to display for this partition.
     * It makes most sense to use the mount point here ( but this is not a
     * requirement ). This is what will be displayed in the "Name" column.
     *
     * Derived classes need to implement this method.
     **/
    virtual QString name() const = 0;

    /**
     * The device name of this partition.
     *
     * Derived classes may choose to reimplement this method.
     * This default implementation returns an empty std::string.
     **/
    virtual QString deviceName() const { return ""; }


    /**
     * Update this item's status ( here: the numeric fields ).
     * Triggered by QY2ListView::updateAllItemStates().
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual void updateStatus() override;

    /**
     * Update this item's data completely.
     * Triggered by QY2ListView::updateAllItemData().
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual void updateData() override;

    /**
     * Re-declare ordinary setText() method so the compiler doesn't get
     * confused which one to use.
     **/
    void setText( int column, const QString & text )
	{ QTreeWidgetItem::setText( column, text ); }

    /**
     * Set a column text via FSize.
     **/
    void setText( int column, const FSize & size );

    /**
     * Comparison function used for sorting the list.
     * Reimplemented from QTreeWidgetItem.
     **/
    virtual bool operator< ( const QTreeWidgetItem & other ) const override;

    // Columns

    int nameCol()		const	{ return _diskUsageList->nameCol();		}
    int percentageBarCol()	const	{ return _diskUsageList->percentageBarCol();	}
    int freeSizeCol()		const	{ return _diskUsageList->freeSizeCol();		}
    int totalSizeCol()		const	{ return _diskUsageList->totalSizeCol();	}


protected:

    /**
     * ( Re- ) initialize fields - all displayed fields ( if 'allFields' is
     * 'true' ) or only the varying fields ( used, free, percentage ).
     **/
    void init( bool allFields );

    public:
    /**
     * Paint a percentage bar into a @ref QListViewItem cell.
     * 'width' is the width of the entire cell.
     * 'indent' is the number of pixels to indent the bar.
     *
     * Stolen from KDirStat::KDirTreeView with the author's permission.
     **/
    void paintPercentageBar( QPainter *		painter,
			     QStyleOptionViewItem option,
			     const QColor &	fillColor);

protected:


    //
    // Data members
    //

    QY2DiskUsageList * _diskUsageList;
};




#endif // ifndef QY2DiskUsageList_h
