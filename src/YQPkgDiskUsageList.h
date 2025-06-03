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


#ifndef YQPkgDiskUsageList_h
#define YQPkgDiskUsageList_h

#include <QKeyEvent>
#include <QMap>

#include <zypp/DiskUsageCounter.h>

#include "QY2DiskUsageList.h"


typedef zypp::DiskUsageCounter::MountPoint ZyppPartitionDu;
class YQPkgDiskUsageListItem;


/**
 * Helper class to manage warnings that are to be issued when a value enters a
 * predefined range, but repeated only when that value leaves a (wider)
 * "proximity" range and then re-enters the (narrower) "inner" range.
 *
 * Example: Disk space usage:
 *
 *      70%
 *      75%
 *      80%     [proximity range start]
 *      85%
 *      90%     [inner range start]
 *      95%
 *      100%
 *
 * A warning is to be posted when there is only 90% disk space left. After the
 * warning is displayed, there shall be no more warning until disk usage decreases
 * below 80% (the proximity range) and then later increases again to 90%.
 *
 * The net effect of all that is to avoid posting the warning over and over
 * again while the value changes back and forth around the boundary of the
 * ( inner ) warning range.
 **/
class YQPkgWarningRangeNotifier
{
public:

    /**
     * Constructor.
     **/
    YQPkgWarningRangeNotifier();

    /**
     * Notification that the inner range is entered.
     * The caller has to decide the criteria for that.
     **/
    void enterRange();

    /**
     * Notification that the proximity range is entered, i.e. that the value is
     * getting near the inner range.
     * 'enterRange()' automatically includes this, too.
     **/
    void enterProximity();

    /**
     * Notification that a warning has been posted.
     **/
    void warningPostedNotify();

    /**
     * Check if the value is in range, i.e. if anybody from the outside has
     * called 'enterRange()' since the last call to 'clear()'.
     **/
    bool inRange() const;

    /**
     * Check if a warning should be posted, i.e. if the value is currently in
     * range ( see 'inRange() ) and there has been no notification yet that a
     * warning has already been posted.
     **/
    bool needWarning() const;

    /**
     * Check if the value is leaving the proximity range.
     **/
    bool leavingProximity() const;

    /**
     * Clear the current values, i.e. prepare for a new round of checks
     **/
    void clear();

    /**
     * Clear everything, including all history values such as if a warning has
     * been posted.
     **/
    void clearHistory();


protected:

    bool _inRange;
    bool _isClose;
    bool _hasBeenClose;
    bool _warningPosted;
};





/**
 * List of disk usage of all attached partitions.
 **/
class YQPkgDiskUsageList : public QY2DiskUsageList
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * 'thresholdPercent' can be used to include only partitions with at least
     * this many percent used disk space in the list. This is useful for
     * warning dialogs ( only? ).
     **/
    YQPkgDiskUsageList( QWidget * parent,
                        int       thresholdPercent = 0 );

    /**
     * Destructor.
     **/
    virtual ~YQPkgDiskUsageList() {}

    /**
     * Suggest reasonable default size.
     *
     * Reimplemented from QListView.
     **/
    virtual QSize sizeHint() const override;

    /**
     * Warning range notifier about running out of disk space warning.
     **/
    YQPkgWarningRangeNotifier runningOutWarning;

    /**
     * Warning range notifier about disk space overflow warning.
     **/
    YQPkgWarningRangeNotifier overflowWarning;


public slots:

    /**
     * Update all statistical data in the list.
     **/
    void updateDiskUsage();

    /**
     * Post all pending disk space warnings based on the warning range
     * notifiers.
     **/
    void postPendingWarnings();


protected:

    /**
     * Event handler for keyboard input - for debugging and testing.
     * Changes the current item's percentage on the fly.
     *
     * Reimplemented from QListView / QWidget.
     **/
    virtual void keyPressEvent( QKeyEvent * ev ) override;


    // Data members

    QMap<QString, YQPkgDiskUsageListItem*> _items;
    bool                                   _debug;
};



class YQPkgDiskUsageListItem: public QY2DiskUsageListItem
{
public:

    /**
     * Constructor. Creates a YQPkgDiskUsageList item that corresponds to the
     * specified file system.
     **/
    YQPkgDiskUsageListItem( YQPkgDiskUsageList *    parent,
                            const ZyppPartitionDu & partitionDu );

    /**
     * Destructor.
     **/
    virtual ~YQPkgDiskUsageListItem() {}

    /**
     * Returns the corresponding disk usage data.
     **/
    ZyppPartitionDu partitionDu() const { return _partitionDu; }

    /**
     * Update the disk usage data.
     **/
    void updateDuData( const ZyppPartitionDu & fromData );

    /**
     * The currently used size of this partition.
     *
     * Reimplemented from QY2DiskUsageListItem.
     **/
    virtual FSize usedSize() const override;

    /**
     * The total size of this partition.
     *
     * Reimplemented from QY2DiskUsageListItem.
     **/
    virtual FSize totalSize() const override;

    /**
     * The name to display for this partition ( the mount point ).
     *
     * Reimplemented from QY2DiskUsageListItem.
     **/
    virtual QString name() const override;

    /**
     * The device name of this partition.
     *
     * Reimplemented from QY2DiskUsageListItem.
     **/
    virtual QString deviceName() const  override { return ""; }

    /**
     * Check the remaining disk space of this partition based on percentage and
     * absolute free MB. Notify the parent YQPkgDiskUsageList's warning ranges
     * accordingly.
     **/
    void checkRemainingDiskSpace();


protected:

    // Data members

    ZyppPartitionDu      _partitionDu;
    YQPkgDiskUsageList * _pkgDiskUsageList;
};


#endif // ifndef YQPkgDiskUsageList_h
