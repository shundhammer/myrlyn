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


#include <QtAlgorithms>  // qDeleteAll()

#include "ZyppHistory.h"
#include "ZyppHistoryParser.h"
#include "Logger.h"
#include "Exception.h"

#define DEFAULT_ZYPP_HISTORY "/var/log/zypp/history"

using namespace ZyppHistoryEvents;


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

    try
    {
        clear();
        ZyppHistoryParser parser( _fileName );
        _dirty  = false; // Even if this fails, don't try again
        _events = parser.parse();
    }
    catch ( const FileException & exception )
    {
        CAUGHT( exception );
        RETHROW( exception );
    }
    catch ( const ZyppHistoryParseException & exception )
    {
        CAUGHT( exception );
        RETHROW( exception );
    }

    return true;  // success
}


QStringList
ZyppHistory::uniqueDates()
{
    QStringList dates;
    QString lastDate;

    for ( Event * event: _events )
    {
        QString date = event->date();

        if ( date != lastDate )
        {
            dates << date;
            lastDate = date;
        }
    }

#if 0
    for ( const QString & date: dates )
        logDebug() << date << endl;
#endif

    return dates;
}


void ZyppHistory::clear()
{
    qDeleteAll( _events );
    _events.clear();
    _dirty = true;
}


void ZyppHistory::dropCache()
{
    logInfo() << "Dropping zypp history cache" << endl;
    instance()->clear();
}


void ZyppHistory::setFileName( const QString & fileName )
{
    _fileName = fileName;

    if ( fileName.isEmpty() )
        _fileName = DEFAULT_ZYPP_HISTORY;

    logInfo() << "Using zypp history file " << _fileName << endl;
}
