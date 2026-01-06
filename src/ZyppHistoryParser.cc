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


#include <QFile>
#include <QTextStream>

#include "ZyppHistoryParser.h"
#include "Logger.h"
#include "Exception.h"

#define MAX_ERR_COUNT 500


ZyppHistoryParser::ZyppHistoryParser( const QString & fileName ):
    _fileName( fileName ),
    _lineNo(0),
    _errCount(0)
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
    QFile file( _fileName );

    if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QString msg = QString( "Can't open %1" ).arg( _fileName );
        logError() << msg << endl;
        THROW( FileException( _fileName, msg ) );
    }

    _events.clear();
    _errCount = 0;
    _lineNo   = 0;

    QTextStream stream( &file );
    logInfo() << "Parsing zypp history file " << _fileName << endl;

    while ( ! stream.atEnd() )
    {
        QString line = stream.readLine().trimmed();
        _lineNo++;

        if ( ! line.isEmpty() && ! line.startsWith( "#" ) )
            parseLine( line );
    }

    return _events;
}


void ZyppHistoryParser::parseLine( const QString & line )
{
    // 2025-12-09 17:54:12|install|MozillaFirefox|145.0.2-1.1|x86_64|root@meteor|slowroll-oss|2ead...b676|
    //      0                 1          2            3         4         5         6            7
    QStringList fields = line.split( '|' );

    if ( ! checkFieldsCount( fields, 2 ) )
        return;

    ZyppHistory::EventType eventType = parseEventType( fields.at( 1 ) );

    switch ( eventType )
    {
        case ZyppHistory::CommandEvent:     parseCommandEvent   ( fields ); break;
        case ZyppHistory::PkgInstallEvent:  parsePkgInstallEvent( fields ); break;
        case ZyppHistory::PkgRemoveEvent:   parsePkgRemoveEvent ( fields ); break;
        case ZyppHistory::RepoAddEvent:     parseRepoAddEvent   ( fields ); break;
        case ZyppHistory::RepoRemoveEvent:  parseRepoRemoveEvent( fields ); break;
        case ZyppHistory::RepoUrlEvent:     parseRepoUrlEvent   ( fields ); break;
        case ZyppHistory::RepoAliasEvent:   parseRepoAliasEvent ( fields ); break;
        case ZyppHistory::PatchEvent:       parsePatchEvent     ( fields ); break;

        case ZyppHistory::UnknownEvent:
            break;
    }
}


ZyppHistory::EventType
ZyppHistoryParser::parseEventType( const QString & strOrig )
{
    QString str = strOrig.trimmed().toLower();

    if ( str == "command" ) return ZyppHistory::CommandEvent;
    if ( str == "install" ) return ZyppHistory::PkgInstallEvent;
    if ( str == "remove"  ) return ZyppHistory::PkgRemoveEvent;
    if ( str == "radd"    ) return ZyppHistory::RepoAddEvent;
    if ( str == "rremove" ) return ZyppHistory::RepoRemoveEvent;
    if ( str == "rurl"    ) return ZyppHistory::RepoUrlEvent;
    if ( str == "ralias"  ) return ZyppHistory::RepoAliasEvent;
    if ( str == "patch"   ) return ZyppHistory::PatchEvent;

    logError() << "Unknown zypp history event type \"" << str << "\""
               << " in line " << _lineNo << endl;

    return ZyppHistory::UnknownEvent;
}


bool ZyppHistoryParser::checkFieldsCount( const QStringList & fields, int requiredCount )
{
    if ( fields.size() >= requiredCount )
        return true;  // ok

    logError() << "Only " << fields.size() << " fields in line " << _lineNo
               << " but at least " << requiredCount << " required"
               << endl;

    incErrCount(); // this may throw an exception

    return false;  // error
}


void ZyppHistoryParser::incErrCount()
{
    if ( ++_errCount >= MAX_ERR_COUNT )
        THROW( ZyppHistoryParseException( "Too many parse errors - giving up" ) );
}


void ZyppHistoryParser::parseCommandEvent( const QStringList & fields )
{

}


void ZyppHistoryParser::parsePkgInstallEvent( const QStringList & fields )
{

}


void ZyppHistoryParser::parsePkgRemoveEvent( const QStringList & fields )
{

}


void ZyppHistoryParser::parseRepoAddEvent( const QStringList & fields )
{

}


void ZyppHistoryParser::parseRepoRemoveEvent( const QStringList & fields )
{

}


void ZyppHistoryParser::parseRepoUrlEvent( const QStringList & fields )
{

}


void ZyppHistoryParser::parseRepoAliasEvent( const QStringList & fields )
{

}


void ZyppHistoryParser::parsePatchEvent( const QStringList & fields )
{

}
