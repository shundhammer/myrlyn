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

#ifndef ZyppHistoryParser_h
#define ZyppHistoryParser_h

#include <QTextStream>

#include "ZyppHistory.h"
#include "Exception.h"


class ZyppHistoryParser
{
public:
    
    /**
     * Constructor
     **/
    ZyppHistoryParser( const QString & zyppHistoryFileName );

    /**
     * Destructor
     **/
    virtual ~ZyppHistoryParser();

    /**
     * Parse the zypp history file and return the list of history events in
     * that file.
     *
     * This may throw Exceptions:
     *   - FileException
     *   - ZyppHistoryParseException
     **/
    ZyppHistory::EventList parse();

protected:

    QString _fileName;
    int     _lineNo;
};


class ZyppHistoryParseException: public Exception
{
    ZyppHistoryParseException( const QString & message ):
        Exception( "Parse error" )
        {}

    virtual ~ZyppHistoryParseException() throw()
        {}
};


#endif // ZyppHistoryParser_h
