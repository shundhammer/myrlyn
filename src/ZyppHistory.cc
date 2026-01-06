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


#include "ZyppHistory.h"
#include "Logger.h"
#include "Exception.h"


#define DEFAULT_ZYPP_HISTORY "/var/log/zypp/history"



ZyppHistory * ZyppHistory::_instance = 0;
QString       ZyppHistory::_fileName( DEFAULT_ZYPP_HISTORY );


ZyppHistory::ZyppHistory():
    _dirty( true )
{

}


ZyppHistory::~ZyppHistory()
{
    clear();
}


ZyppHistory * ZyppHistory::instance()
{
    if ( ! _instance )
    {
        _instance = new ZyppHistory();
        CHECK_NEW( _instance );
    }

    return _instance;
}


bool ZyppHistory::read()
{
    if ( ! _dirty )
        return true;  // success

    bool success = false;

    // TO DO: Read and parse

    _dirty = false;

    return success;
}


void ZyppHistory::clear()
{
    qDeleteAll( _events );
    _events.clear();
    _dirty = true;
}


void ZyppHistory::dropCache()
{
    instance()->clear();
}


void ZyppHistory::setFileName( const QString & fileName )
{
    _fileName = fileName;

    if ( fileName.isEmpty() )
        _fileName = DEFAULT_ZYPP_HISTORY;

    logInfo() << "Using zypp history file " << _fileName << endl;
}




QString ZyppHistory::Event::date() const
{
    // timestamp:  "2025-12-28 14:15:26" -> "2025-12-28"
    return timestamp.section( ' ', 0 );
}



QString ZyppHistory::Event::time() const
{
    // timestamp:  "2025-12-28 14:15:26" -> "14:15:26"
    return timestamp.section( ' ', 1 );
}



void ZyppHistory::ParentEvent::addChild( Event * childEvent )
{
    if ( childEvent )
        _childEvents << childEvent;
}
