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


#include "Logger.h"
#include "Exception.h"
#include "PkgTaskListWidget.h"


void PkgTaskListWidget::addTaskItems( const PkgTaskList & taskList )
{
    for ( PkgTask * task: taskList )
        addTaskItem( task );
}


PkgTaskListWidgetItem *
PkgTaskListWidget::addTaskItem( PkgTask * task )
{
    PkgTaskListWidgetItem * item = new PkgTaskListWidgetItem( task, this );
    CHECK_NEW( item );

    if ( _autoScrollToLast )
        scrollToItem( item, QAbstractItemView::PositionAtBottom );

    return item;
}


PkgTaskListWidgetItem *
PkgTaskListWidget::findTaskItem( PkgTask * task ) const
{
    for ( int row=0; row < count(); ++row )
    {
        PkgTaskListWidgetItem * it =
            dynamic_cast<PkgTaskListWidgetItem *>( item( row ) );

        if ( it && it->task() == task )
            return it;
    }

    return 0;
}


void PkgTaskListWidget::removeTaskItem( PkgTask * task )
{
    PkgTaskListWidgetItem * item = findTaskItem( task );

    if ( item )
    {
        // This is actually the officially documented way to remove an item
        // from a QListWidget: Just delete it. It removes itself from its
        // parent QListView in its destructor. There is no call like
        // 'deleteItem()' or 'removeItem()'.

        delete item;
    }
}




PkgTaskListWidgetItem::PkgTaskListWidgetItem( PkgTask *           task,
                                              PkgTaskListWidget * parent )
    : QListWidgetItem( parent )
    , _task( task )
    , _serial( 0 )
{
    if ( parent )
        _serial = parent->nextSerial();

    QString txt;

    if ( _task->action() == PkgRemove )
        txt = "- ";

    txt += _task->name();
    setText( txt );

#if 0
    if ( parent )
    {
        logDebug() << "New PkgTask item for " << parent->objectName()
                   << ": " << txt << endl;
    }
#endif
}


bool PkgTaskListWidgetItem::operator<( const QListWidgetItem & otherListWidgetItem ) const
{
    const PkgTaskListWidgetItem * other =
        dynamic_cast<const PkgTaskListWidgetItem *>( &otherListWidgetItem );

    if ( other && sortByInsertionSequence() )
    {
        return ( this->serial() < other->serial() );
    }
    else
    {
        return QListWidgetItem::operator<( otherListWidgetItem );
    }
}



bool PkgTaskListWidgetItem::sortByInsertionSequence() const
{
    if ( ! listWidget() )
        return false;

    PkgTaskListWidget * parentPkgTaskListWidget =
        dynamic_cast<PkgTaskListWidget *> ( listWidget() );

    if ( parentPkgTaskListWidget )
	return parentPkgTaskListWidget->sortByInsertionSequence();
    else
        return false;
}

