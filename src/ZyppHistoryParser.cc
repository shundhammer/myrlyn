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


#include "ZyppHistoryParser.h"
#include "Logger.h"
#include "Exception.h"


ZyppHistoryParser::ZyppHistoryParser( const QString & fileName ):
    _fileName( fileName ),
    _lineNo(0)
{
    // NOP
}


ZyppHistoryParser::~ZyppHistoryParser()
{
    // NOP
}


ZyppHistory::EventList
ZyppHistoryParser::parse()
{
    ZyppHistory::EventList events;

    return events;
}
