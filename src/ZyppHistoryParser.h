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
    ZyppHistoryEvents::EventList parse();

protected:

    void parseLine( const QString & line );
    ZyppHistoryEvents::EventType parseEventType( const QString & str );

    ZyppHistoryEvents::Event * parseCommandEvent   ( const QStringList & fields );
    ZyppHistoryEvents::Event * parsePkgInstallEvent( const QStringList & fields );
    ZyppHistoryEvents::Event * parsePkgRemoveEvent ( const QStringList & fields );
    ZyppHistoryEvents::Event * parseRepoAddEvent   ( const QStringList & fields );
    ZyppHistoryEvents::Event * parseRepoRemoveEvent( const QStringList & fields );
    ZyppHistoryEvents::Event * parseRepoUrlEvent   ( const QStringList & fields );
    ZyppHistoryEvents::Event * parseRepoAliasEvent ( const QStringList & fields );
    ZyppHistoryEvents::Event * parsePatchEvent     ( const QStringList & fields );
    
    /**
     * Increase the parse error counter and throw an exception if it reaches a
     * maximum number of errors.
     **/
    void incErrCount();

    /**
     * Check if 'fields' has at least 'requiredCount' fields.
     * Return 'true' if ok. If not, increase the error count and return
     * 'false'. Throw an exception if there are too many parse errors.
     **/
    bool checkFieldsCount( const QStringList & fields, int requiredCount );

    void    finalizeLastCommand();
    QString prettyCommand( const QString & rawCommand );
    void    addEvent( ZyppHistoryEvents::Event * event );


    //
    // Data members
    //

    QString _fileName;
    int     _lineNo;
    int     _errCount;
    int     _eventCount;

    ZyppHistoryEvents::EventList      _events;
    ZyppHistoryEvents::CommandEvent * _lastCommand;
};


class ZyppHistoryParseException: public Exception
{
public:
    ZyppHistoryParseException( const QString & message ):
        Exception( "Parse error" )
        {}

    virtual ~ZyppHistoryParseException() throw()
        {}
};


#endif // ZyppHistoryParser_h
