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

#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <boost/multiprecision/cpp_int.hpp> // arbitrary precision integer

#include <QStyle>
#include <QHeaderView>
#include <QEvent>
#include <QtGlobal>

#include "FSize.h"
#include "utf8.h"
#include "YQi18n.h"
#include "YQPkgDiskUsageList.h"
#include "YQPkgDiskUsageWarningDialog.h"

#include "Logger.h"


using std::set;


// Warning ranges for "disk space is running out" or "disk space overflow".
// The WARN value triggers a warning popup once ( ! ). The warning will not be
// displayed again until the value sinks below the PROXIMITY value and then
// increases again to the WARN value.
//
// See class YQPkgWarningRangeNotifier in file YQPkgDiskUsageList.h for details.

#define MIN_FREE_MB_WARN        400
#define MIN_FREE_MB_PROXIMITY   700

#define MIN_PERCENT_WARN        90
#define MIN_PERCENT_PROXIMITY   80

#define OVERFLOW_MB_WARN        0
#define OVERFLOW_MB_PROXIMITY   300


typedef zypp::DiskUsageCounter::MountPointSet           ZyppDuSet;
typedef zypp::DiskUsageCounter::MountPointSet::iterator ZyppDuSetIterator;



YQPkgDiskUsageList::YQPkgDiskUsageList( QWidget * parent,
                                        int       thresholdPercent )
    : QY2DiskUsageList( parent, true )
{
    _debug = false;

    ZyppDuSet diskUsage = zypp::getZYpp()->diskUsage();

    if ( diskUsage.empty() )
    {
        zypp::getZYpp()->setPartitions( zypp::DiskUsageCounter::detectMountPoints() );
        diskUsage = zypp::getZYpp()->diskUsage();
    }


    for ( ZyppDuSetIterator it = diskUsage.begin();
          it != diskUsage.end();
          ++it )
    {
        const ZyppPartitionDu & partitionDu = *it;

        if ( ! partitionDu.readonly )
        {
            YQPkgDiskUsageListItem * item = new YQPkgDiskUsageListItem( this, partitionDu );
            Q_CHECK_PTR( item );
            item->updateData();
            _items.insert( QString::fromUtf8(partitionDu.dir.c_str()), item );
        }
    }

    resizeColumnToContents( nameCol() );
    resizeColumnToContents( totalSizeCol() );
    resizeColumnToContents( freeSizeCol() );

    sortByColumn( percentageBarCol(), Qt::DescendingOrder );

    header()->setSectionResizeMode( nameCol(), QHeaderView::Stretch );
    header()->setSectionResizeMode( QHeaderView::Interactive );
}


void
YQPkgDiskUsageList::updateDiskUsage()
{
    runningOutWarning.clear();
    overflowWarning.clear();

    ZyppDuSet diskUsage = zypp::getZYpp()->diskUsage();

    for ( ZyppDuSetIterator it = diskUsage.begin();
          it != diskUsage.end();
          ++it )
    {
        const ZyppPartitionDu & partitionDu = *it;
        YQPkgDiskUsageListItem * item = _items[ QString::fromUtf8(partitionDu.dir.c_str()) ];

        if ( item )
            item->updateDuData( partitionDu );
        else
            logError() << "No entry for mount point " << partitionDu.dir << Qt::endl;
    }

    resizeColumnToContents( totalSizeCol() );
    postPendingWarnings();
}


void
YQPkgDiskUsageList::postPendingWarnings()
{
    if ( overflowWarning.needWarning() )
    {
        YQPkgDiskUsageWarningDialog::diskUsageWarning( _( "<b>Error:</b> Out of disk space!" ),
                                                       100, _( "&OK" ) );

        overflowWarning.warningPostedNotify();
        runningOutWarning.warningPostedNotify(); // Suppress this (now redundant) other warning
    }

    if ( runningOutWarning.needWarning() )
    {
        YQPkgDiskUsageWarningDialog::diskUsageWarning( _( "<b>Warning:</b> Disk space is running out!" ) ,
                                                       MIN_PERCENT_WARN, _( "&OK" ) );
        runningOutWarning.warningPostedNotify();
    }

    if ( overflowWarning.leavingProximity() )
        overflowWarning.clearHistory();

    if ( runningOutWarning.leavingProximity() )
        runningOutWarning.clearHistory();
}


QSize
YQPkgDiskUsageList::sizeHint() const
{
    QFontMetrics fontMetrics( font() );

    return QSize( fontMetrics.horizontalAdvance( "/var/usr/home 100% 100.32GB 100.3GB" ) + 50,  100 );
}


void
YQPkgDiskUsageList::keyPressEvent( QKeyEvent * event )
{

    if ( event )
    {
        Qt::KeyboardModifiers special_combo = ( Qt::ControlModifier| Qt::ShiftModifier | Qt::AltModifier );

        if ( ( event->modifiers() & special_combo ) == special_combo )
        {
            if ( event->key() == Qt::Key_Q )
            {
                _debug = ! _debug;
                logInfo() << "Debug mode: " << _debug << Qt::endl;
            }

        }

        if ( _debug && currentItem() )
        {
            YQPkgDiskUsageListItem * item = dynamic_cast<YQPkgDiskUsageListItem *> ( currentItem() );

            if ( item )
            {
                {
                    int percent = item->usedPercent();

                    switch ( event->key() )
                    {
                        case Qt::Key_1:     percent = 10;   break;
                        case Qt::Key_2:     percent = 20;   break;
                        case Qt::Key_3:     percent = 30;   break;
                        case Qt::Key_4:     percent = 40;   break;
                        case Qt::Key_5:     percent = 50;   break;
                        case Qt::Key_6:     percent = 60;   break;
                        case Qt::Key_7:     percent = 70;   break;
                        case Qt::Key_8:     percent = 80;   break;
                        case Qt::Key_9:     percent = 90;   break;
                        case Qt::Key_0:     percent = 100;  break;
                        case Qt::Key_Plus:  percent += 3;   break;
                        case Qt::Key_Minus: percent -= 3;   break;

                        case 'w':
                            // Only for testing, thus intentionally using no translations
                            YQPkgDiskUsageWarningDialog::diskUsageWarning( "<b>Warning:</b> Disk space is running out!",
                                                                           90, "&OK" );
                            break;

                        case 'f':
                            YQPkgDiskUsageWarningDialog::diskUsageWarning( "<b>Error:</b> Out of disk space!",
                                                                           100, "&Continue anyway", "&Cancel" );
                            break;
                    }

                    if ( percent < 0   )
                        percent = 0;

                    ZyppPartitionDu partitionDu( item->partitionDu() );

                    if ( percent != item->usedPercent() )
                    {
                        partitionDu.pkg_size = partitionDu.total_size / 100 * percent;

                        runningOutWarning.clear();
                        overflowWarning.clear();

                        item->updateDuData( partitionDu );
                        postPendingWarnings();
                    }
                }
            }
        }
    }

    QY2DiskUsageList::keyPressEvent( event );
}




YQPkgDiskUsageListItem::YQPkgDiskUsageListItem( YQPkgDiskUsageList *    parent,
                                                const ZyppPartitionDu & partitionDu )
    : QY2DiskUsageListItem( parent )
    , _partitionDu( partitionDu )
    , _pkgDiskUsageList( parent )
{
    // logVerbose() << "disk usage list entry for " << partitionDu.dir << Qt::endl;
}


FSize
YQPkgDiskUsageListItem::usedSize() const
{
    // the libzypp size is in KiB

    return FSize( _partitionDu.pkg_size, FSize::Unit::K );
}


FSize
YQPkgDiskUsageListItem::totalSize() const
{
    // the libzypp size is in KiB

    return FSize( _partitionDu.total_size, FSize::Unit::K );
}


QString
YQPkgDiskUsageListItem::name() const
{
    return fromUTF8( _partitionDu.dir.c_str() );
}


void
YQPkgDiskUsageListItem::updateDuData( const ZyppPartitionDu & fromData )
{
    _partitionDu = fromData;
    updateData();
    checkRemainingDiskSpace();
}


void
YQPkgDiskUsageListItem::checkRemainingDiskSpace()
{
    int percent = usedPercent();
    // free size in MiB
    boost::multiprecision::cpp_int free = freeSize().in_unit( FSize::Unit::M );

#if 0
    logDebug() << "Partition " << _partitionDu.dir
               << " free percent: " << percent << "%, "
               << " bfree: " << freeSize()
               << " (" << free << "MiB)"
               << Qt::endl;
#endif

    if ( percent > MIN_PERCENT_WARN )
    {
        // Modern hard disks can be huge, so a warning based on percentage only
        // can be misleading - check the absolute value, too.

        if ( free < MIN_FREE_MB_PROXIMITY )
            _pkgDiskUsageList->runningOutWarning.enterProximity();

        if ( free < MIN_FREE_MB_WARN )
            _pkgDiskUsageList->runningOutWarning.enterRange();
    }

    if ( free < MIN_FREE_MB_PROXIMITY )
    {
        if ( percent > MIN_PERCENT_PROXIMITY )
            _pkgDiskUsageList->runningOutWarning.enterProximity();
    }

    if ( free < OVERFLOW_MB_WARN )
        _pkgDiskUsageList->overflowWarning.enterRange();

    if ( free < OVERFLOW_MB_PROXIMITY )
        _pkgDiskUsageList->overflowWarning.enterProximity();
}






YQPkgWarningRangeNotifier::YQPkgWarningRangeNotifier()
{
    clearHistory();
}


void
YQPkgWarningRangeNotifier::clear()
{
    _inRange      = false;
    _hasBeenClose = _isClose;
    _isClose      = false;
}


void
YQPkgWarningRangeNotifier::clearHistory()
{
    clear();
    _hasBeenClose  = false;
    _warningPosted = false;
}


void
YQPkgWarningRangeNotifier::enterRange()
{
    _inRange = true;
    enterProximity();
}


void
YQPkgWarningRangeNotifier::enterProximity()
{
    _isClose      = true;
    _hasBeenClose = true;
}


void
YQPkgWarningRangeNotifier::warningPostedNotify()
{
    _warningPosted = true;
}


bool
YQPkgWarningRangeNotifier::inRange() const
{
    return _inRange;
}


bool
YQPkgWarningRangeNotifier::leavingProximity() const
{
    return ! _isClose && ! _hasBeenClose;
}


bool
YQPkgWarningRangeNotifier::needWarning() const
{
    return _inRange && ! _warningPosted;
}
