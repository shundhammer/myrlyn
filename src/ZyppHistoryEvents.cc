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


#include "ZyppHistoryEvents.h"
#include "Exception.h"

using namespace ZyppHistoryEvents;


QString Event::date() const
{
    // timestamp:  "2025-12-28 14:15:26" -> "2025-12-28"
    return timestamp.section( ' ', 0, 0 );
}



QString Event::time() const
{
    // timestamp:  "2025-12-28 14:15:26" -> "14:15:26"
    return timestamp.section( ' ', 1, 1 );
}



void ParentEvent::addChildEvent( Event * childEvent )
{
    if ( childEvent )
        _childEvents << childEvent;
}


ParentEvent::~ParentEvent()
{
    if ( _doDelete )
        qDeleteAll( _childEvents );
}


CommandEvent *
CommandEvent::shallowClone()
{
    CommandEvent * clone = new CommandEvent;
    CHECK_NEW( clone );

    clone->timestamp  = this->timestamp;
    clone->command    = this->command;
    clone->rawCommand = this->rawCommand;

    return clone;
}
