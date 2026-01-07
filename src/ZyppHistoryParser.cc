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
    _errCount(0),
    _lastCommand(0)
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
    _lastCommand = 0;
    _errCount    = 0;
    _lineNo      = 0;

    QTextStream stream( &file );
    logInfo() << "Parsing zypp history file " << _fileName << endl;

    while ( ! stream.atEnd() )
    {
        QString line = stream.readLine().trimmed();
        _lineNo++;

        if ( ! line.isEmpty() && ! line.startsWith( "#" ) )
            parseLine( line );
    }

    finalizeLastCommand();

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
        case ZyppHistory::EventType::Command:     parseCommandEvent   ( fields ); break;
        case ZyppHistory::EventType::PkgInstall:  parsePkgInstallEvent( fields ); break;
        case ZyppHistory::EventType::PkgRemove:   parsePkgRemoveEvent ( fields ); break;
        case ZyppHistory::EventType::RepoAdd:     parseRepoAddEvent   ( fields ); break;
        case ZyppHistory::EventType::RepoRemove:  parseRepoRemoveEvent( fields ); break;
        case ZyppHistory::EventType::RepoUrl:     parseRepoUrlEvent   ( fields ); break;
        case ZyppHistory::EventType::RepoAlias:   parseRepoAliasEvent ( fields ); break;
        case ZyppHistory::EventType::Patch:       parsePatchEvent     ( fields ); break;

        case ZyppHistory::EventType::Unknown:
            break;
    }
}


ZyppHistory::EventType
ZyppHistoryParser::parseEventType( const QString & strOrig )
{
    QString str = strOrig.trimmed().toLower();

    if ( str == "command" ) return ZyppHistory::EventType::Command;
    if ( str == "install" ) return ZyppHistory::EventType::PkgInstall;
    if ( str == "remove"  ) return ZyppHistory::EventType::PkgRemove;
    if ( str == "radd"    ) return ZyppHistory::EventType::RepoAdd;
    if ( str == "rremove" ) return ZyppHistory::EventType::RepoRemove;
    if ( str == "rurl"    ) return ZyppHistory::EventType::RepoUrl;
    if ( str == "ralias"  ) return ZyppHistory::EventType::RepoAlias;
    if ( str == "patch"   ) return ZyppHistory::EventType::Patch;

    logError() << "Unknown zypp history event type \"" << str << "\""
               << " in line " << _lineNo << endl;

    return ZyppHistory::EventType::Unknown;
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
    //        #0             #1        #2         #3
    // 2024-09-13 17:42:20|command|root@meteor|'zypper' 'in' 'xhost'|
    // 2024-09-16 13:55:34|command|root@meteor|'zypper' 'dup'|
    // 2024-09-13 18:13:28|command|root@meteor|'/usr/bin/ruby.ruby3.3' '--encoding=utf-8'
    //    '/usr/lib/YaST2/bin/y2start' 'sw_single' 'qt' '-name' 'YaST2' '-icon' 'yast'|
    // 2026-01-01 19:54:45|command|root@meteor|'/usr/bin/myrlyn'|

    if ( ! checkFieldsCount( fields, 4 ) )
        return;

    finalizeLastCommand();
    ZyppHistory::CommandEvent * commandEvent = new ZyppHistory::CommandEvent;
    CHECK_NEW( commandEvent );

    commandEvent->timestamp = fields.at( 0 );
    commandEvent->eventType = ZyppHistory::EventType::Command;

    QString command = fields.at( 1 ).simplified();
    command.remove( '\'' ); // Remove all single quotes: 'zypper' 'in' 'xhost'
    commandEvent->rawCommand = command;
    commandEvent->command    = prettyCommand( command );

    _lastCommand = commandEvent;

    // Intentionally not doing
    //
    //   _events << commandEvent;
    //
    // here and now: This is done in finalizeLastCommand(), but only if it has
    // child events.
}


void ZyppHistoryParser::finalizeLastCommand()
{
    if ( _lastCommand )
    {
        _lastCommand->squeezeChildEvents();

        // Only append _lastCommand to _events if it has any child events to
        // prevent lots of empty commands apearing in the history; e.g. when a
        // user (like myself) often checks with Myrlyn if there are any
        // updates, or if trying a zypper dry-run that has no effect.

        if ( _lastCommand->hasChildEvents() )
            _events << _lastCommand;

        _lastCommand = 0;
    }
}


QString ZyppHistoryParser::prettyCommand( const QString & rawCommandLine )
{
    // Sample input data:
    //
    // /usr/bin/myrlyn
    // zypper in xhost
    // zypper dup
    //
    // /usr/bin/ruby.ruby3.3 --encoding=utf-8 /usr/lib/YaST2/bin/y2start sw_single qt \
    //   -name YaST2 -icon yast

    QString commandLine = rawCommandLine.simplified();
    QString result      = commandLine;
    QString command     = commandLine.section( ' ', 0, 0 ); // first word only
    QString args        = commandLine.section( ' ', 1 );    // the rest

    command = command.section( '/', -1 ).toLower();  // basename only, no path

    logDebug() << "Found command \"" << command << "\"" << endl;

    if ( command.contains( "myrlyn" ) )
    {
        result = "Myrlyn";
    }
    else if ( command.contains( "ruby" ) && args.contains( "y2start" ) )
    {
        result = "YaST";

        if ( args.contains( " installation " ) )
            result += " installation";
        else if ( args.contains( " update " ) || args.contains( " upgrade ") )
            result += " upgrade";
        else if ( args.contains( " sw_single " ) )
            result += " sw_single";
    }
    else if ( command.contains( "zypper" ) )
    {
        result = "zypper";

        if ( args.size() < 25 )
            result += QString( " " ) + args;
        else
            result += QString( " " ) + args.left( 25 ) + "...";
    }
    else if ( command.toLower().contains( "packagekit" ) )
        result = command;

    return result;
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
