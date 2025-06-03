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


#include <algorithm>    // std::sort()

#include "Logger.h"
#include "Exception.h"
#include "utf8.h"
#include "YQZypp.h"
#include "PkgTasks.h"



bool PkgTask::matches( const QString &  name,
                       PkgTaskAction    action,
                       PkgTaskRequester requester ) const
{
    // Check action and requester.
    //
    // Using bitwise & to be able to use OR'ed values like
    // PkgAdd,    i.e.  PkgInstall | PkgUpdate, or
    // PkgReqAll, i.e.  PkgReqUser | PkgReqDep.

    bool match =
        ( this->_action    & action    ) &&
        ( this->_requester & requester );

    // Check the name unless empty.

    if ( match && ! name.isEmpty() )
        match = ( this->_name == name );

    return match;
}


bool PkgTask::matches( PkgTask * other ) const
{
    if ( ! other )
        return false;

    return matches( other->name(), other->action(), other->requester() );
}


bool PkgTask::matches( const PkgTask & other ) const
{
    return matches( other.name(), other.action(), other.requester() );
}


QString PkgTask::actionToString( PkgTaskAction action )
{
    if ( action & PkgInstall   )  return "PkgInstall";
    if ( action & PkgUpdate    )  return "PkgUpdate";
    if ( action & PkgRemove    )  return "PkgRemove";
    if ( action & PkgAdd       )  return "PkgAdd";
    if ( action & PkgAll       )  return "PkgAll";
    if ( action == PkgNoAction )  return "PkgNoAction";

    return QString( "0x%1" ).arg( (int) action, 16 );
}


//----------------------------------------------------------------------


LogStream & operator<<( LogStream & str, const PkgTask & task )
{
    str << "<" << task.actionToString()
        << " " << task.name()
        << ">";

    return str;
}


LogStream & operator<<( LogStream & str, PkgTask * task )
{
    if ( task )
    {
        str << "<" << task->actionToString()
            << " " << task->name()
            << ">";
    }
    else
    {
        str << "<NULL PkgTask>";
    }

    return str;
}


//----------------------------------------------------------------------


PkgTaskList::PkgTaskList( const QString & listName )
        : QList<PkgTask *>()
        , _name( listName )
{
}


PkgTaskList::~PkgTaskList()
{
}


PkgTask *
PkgTaskList::find( const QString &  name,
                   PkgTaskAction    action,
                   PkgTaskRequester requester ) const
{
    for ( PkgTask * task: *this )
    {
        if ( task && task->matches( name, action, requester ) )
            return task;
    }

    return 0;
}


PkgTask *
PkgTaskList::find( const PkgTask & filter ) const
{
    for ( PkgTask * task: *this )
    {
        if ( task && task->matches( filter ) )
            return task;
    }

    return 0;
}


PkgTask *
PkgTaskList::find( ZyppRes zyppRes ) const
{
    QString resName = fromUTF8( zyppRes->name() );

    for ( PkgTask * task: *this )
    {
        if ( task && task->name() == resName )
            return task;
    }

    return 0;
}


PkgTaskList
PkgTaskList::filtered( PkgTaskAction    filterAction,
                       PkgTaskRequester filterRequester )
{
    PkgTaskList result( QString( "filtered %1" ).arg( _name ) );

    for ( const PkgTask * task: *this )
    {
        if ( ( task->action()    & filterAction    ) &&
             ( task->requester() & filterRequester )    )
        {
            result.append( new PkgTask( *task ) );
        }
    }

    return result;
}


ByteCount
PkgTaskList::downloadSizeSum() const
{
    ByteCount sum(0);

    for ( const PkgTask * task: *this )
    {
        if ( ( task->action() & PkgAdd )    &&
             task->downloadSize()      > 0  &&
             task->downloadedPercent() > 0 )
        {
            sum += task->downloadSize() * ( task->downloadedPercent() / 100.0 );
        }
    }

    return sum;
}


ByteCount
PkgTaskList::installedSizeSum() const
{
    ByteCount sum(0);

    for ( const PkgTask * task: *this )
    {
        if ( task->installedSize() > 0 && task->completedPercent() > 0 )
            sum += task->installedSize() * ( task->completedPercent() / 100.0 );
    }

    return sum;
}


struct PkgTaskCompare
{
    bool operator() ( PkgTask * a, PkgTask * b )
        { return a->name() < b->name(); }
};


void PkgTaskList::sort()
{
    PkgTaskCompare compareFunctor;

    // We need a functor here; an PkgTask::operator<( PkgTask * other ) is
    // ignored, std::sort() just compares the pointer values (!) in that case.

    std::sort( begin(), end(), compareFunctor );
}


//----------------------------------------------------------------------


PkgTasks::PkgTasks()
    : _todo     ( "todo"      )
    , _downloads( "downloads" )
    , _doing    ( "doing"     )
    , _done     ( "done"      )
    , _failed   ( "failed"    )
{
    logDebug() << endl;
}


PkgTasks::~PkgTasks()
{
    clearAll();

    logDebug() << "Destroying PkgTasks done" << endl;
}


void PkgTasks::clearAll()
{
    nuke( _todo      );
    nuke( _downloads );
    nuke( _doing     );
    nuke( _done      );
    nuke( _failed    );
}


void PkgTasks::nuke( PkgTaskList & list )
{
    if ( ! list.isEmpty() )
    {
        logDebug() << "Nuking PkgTaskList \"" << list.name()
                   << "\"  size:" << list.size()
                   << endl;

        qDeleteAll( list );
        list.clear();
    }
}


void PkgTasks::moveTask( PkgTask *      task,
                         PkgTaskList &  fromList,
                         PkgTaskList &  toList )
{
    int index = fromList.indexOf( task );

    if ( index < 0 )
    {
        logError() << "Task " << task->name() << " not found in this list" << endl;
        return;
    }

    toList.append( fromList.takeAt( index ) );
}


void PkgTasks::initFromZypp()
{
    clearAll();

    for ( ZyppPoolIterator it = zyppPkgBegin();
	  it != zyppPkgEnd();
	  ++it )
    {
	ZyppSel selectable = *it;
        CHECK_PTR( selectable );;

        PkgTaskAction    action = PkgNoAction;
        PkgTaskRequester req    = PkgReqNone;

        switch ( selectable->status() )
        {
            case S_Install:     req = PkgReqUser; action = PkgInstall; break;
            case S_Update:      req = PkgReqUser; action = PkgUpdate;  break;
            case S_Del:         req = PkgReqUser; action = PkgRemove;  break;

            case S_AutoInstall: req = PkgReqDep;  action = PkgInstall; break;
            case S_AutoUpdate:  req = PkgReqDep;  action = PkgUpdate;  break;
            case S_AutoDel:     req = PkgReqDep;  action = PkgRemove;  break;

            case S_NoInst:
            case S_KeepInstalled:
            case S_Protected:
            case S_Taboo:
                action = PkgNoAction;
                break;

                // Intentionally omitting 'default' branch so the compiler can
                // catch unhandled enum states
        }

        if ( action != PkgNoAction )
        {
            QString   name = fromUTF8( selectable->name() );
            PkgTask * task = new PkgTask( name, action, req );
            CHECK_NEW( task );

            task->setCompletedPercent( 0 );

            if ( action & PkgAdd ) // PkgInstall | PkgUpdate
            {
                const ZyppObj candidate = selectable->candidateObj();

                if ( candidate )
                {
                    task->setInstalledSize( candidate->installSize() );
                    task->setDownloadSize ( candidate->downloadSize() );
                    task->setDownloadedPercent( 0 );
                }
            }
            else if ( action & PkgRemove )
            {
                task->setDownloadSize( 0.0 );

                const ZyppObj installed = selectable->installedObj();

                if ( installed )
                    task->setInstalledSize( installed->installSize() );
            }

            logInfo() << "New task " << task << endl;
            _todo.append( task );
        }
    }
}

