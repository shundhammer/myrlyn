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

using namespace ZyppHistoryEvents;


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


EventList
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

    EventType eventType = parseEventType( fields.at( 1 ) );

    switch ( eventType )
    {
        case EventType::Command:     parseCommandEvent   ( fields ); break;
        case EventType::PkgInstall:  parsePkgInstallEvent( fields ); break;
        case EventType::PkgRemove:   parsePkgRemoveEvent ( fields ); break;
        case EventType::RepoAdd:     parseRepoAddEvent   ( fields ); break;
        case EventType::RepoRemove:  parseRepoRemoveEvent( fields ); break;
        case EventType::RepoUrl:     parseRepoUrlEvent   ( fields ); break;
        case EventType::RepoAlias:   parseRepoAliasEvent ( fields ); break;
        case EventType::Patch:       parsePatchEvent     ( fields ); break;

        case EventType::Unknown:
            break;
    }
}


EventType
ZyppHistoryParser::parseEventType( const QString & strOrig )
{
    QString str = strOrig.trimmed().toLower();

    if ( str == "command" ) return EventType::Command;
    if ( str == "install" ) return EventType::PkgInstall;
    if ( str == "remove"  ) return EventType::PkgRemove;
    if ( str == "radd"    ) return EventType::RepoAdd;
    if ( str == "rremove" ) return EventType::RepoRemove;
    if ( str == "rurl"    ) return EventType::RepoUrl;
    if ( str == "ralias"  ) return EventType::RepoAlias;
    if ( str == "patch"   ) return EventType::Patch;

    logError() << "Unknown zypp history event type \"" << str << "\""
               << " in line " << _lineNo << endl;

    return EventType::Unknown;
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
    CommandEvent * commandEvent = new CommandEvent;
    CHECK_NEW( commandEvent );

    commandEvent->timestamp = fields.at( 0 );
    commandEvent->eventType = EventType::Command;

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
        // Only append _lastCommand to _events if it has any child events to
        // prevent lots of empty commands apearing in the history; e.g. when a
        // user (like myself) often checks with Myrlyn if there are any
        // updates, or if trying a zypper dry-run that has no effect.

        if ( _lastCommand->hasChildEvents() )
        {
            _lastCommand->squeezeChildEvents();
            _events << _lastCommand;
        }
        else
        {
            logDebug() << "Deleting empty command " << _lastCommand->command << endl;
            delete _lastCommand;
        }

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
    // /usr/bin/ruby.ruby3.3 --encoding=utf-8 /usr/lib/YaST2/bin/y2start sw_single qt -name YaST2

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

    logDebug() << "result: " << result << endl;

    return result;
}


void ZyppHistoryParser::addEvent( Event * event )
{
    if ( _lastCommand )
        _lastCommand->addChildEvent( event );
    else
    {
        logWarning() << "No last command event for event in line " << _lineNo << endl;
        _events << event;
    }
}


void ZyppHistoryParser::parsePkgInstallEvent( const QStringList & fields )
{
    //      #0                #1       #2     #3      #4       #5              #6                    #7
    // 2024-09-13 18:20:50|install|qdirstat|1.9-1.3|x86_64|root@meteor|download.opensuse.org-oss|4138...527a0|

    if ( ! checkFieldsCount( fields, 7 ) )
        return;

    PkgEvent * event = new PkgEvent;
    CHECK_NEW( event );

    event->timestamp = fields.at( 0 );
    event->eventType = EventType::PkgInstall;
    event->pkgName   = fields.at( 2 );
    event->version   = fields.at( 3 );
    event->arch      = fields.at( 4 );
    event->repoAlias = fields.at( 6 );

    addEvent( event );
}


void ZyppHistoryParser::parsePkgRemoveEvent( const QStringList & fields )
{
    //      #0               #1       #2      #3       #4      #5
    // 2024-09-13 18:13:28|remove |drkonqi6|6.1.4-1.1|x86_64|root@meteor|

    if ( ! checkFieldsCount( fields, 4 ) )
        return;

    PkgEvent * event = new PkgEvent;
    CHECK_NEW( event );

    event->timestamp = fields.at( 0 );
    event->eventType = EventType::PkgRemove;
    event->pkgName   = fields.at( 2 );
    event->version   = fields.at( 3 );
    event->arch      = fields.at( 4 );

    addEvent( event );
}


void ZyppHistoryParser::parseRepoAddEvent( const QStringList & fields )
{
    //      #0              #1         #2                               #3
    // 2025-01-16 22:04:52|radd   |download.nvidia.com-tumbleweed|https://download.nvidia.com/opensuse/tumbleweed/|

    if ( ! checkFieldsCount( fields, 4 ) )
        return;

    RepoEvent * event = new RepoEvent;
    CHECK_NEW( event );

    event->timestamp = fields.at( 0 );
    event->eventType = EventType::RepoAdd;
    event->repoAlias = fields.at( 2 );
    event->url       = fields.at( 3 );

    addEvent( event );
}


void ZyppHistoryParser::parseRepoRemoveEvent( const QStringList & fields )
{
    //      #0               #1         #2
    // 2025-01-16 22:07:11|rremove|download.nvidia.com-tumbleweed|

    if ( ! checkFieldsCount( fields, 3 ) )
        return;

    RepoEvent * event = new RepoEvent;
    CHECK_NEW( event );

    event->timestamp = fields.at( 0 );
    event->eventType = EventType::RepoRemove;
    event->repoAlias = fields.at( 2 );

    addEvent( event );
}


void ZyppHistoryParser::parseRepoUrlEvent( const QStringList & fields )
{
    //      #0              #1          #2                                                       #3
    // 2025-02-07 13:06:26|rurl   |http://codecs.opensuse.org/openh264/openSUSE_Tumbleweed|https://codecs.opensuse.org/openh264/openSUSE_Tumbleweed|

    if ( ! checkFieldsCount( fields, 4 ) )
        return;

    RepoEvent * event = new RepoEvent;
    CHECK_NEW( event );

    event->timestamp = fields.at( 0 );
    event->eventType = EventType::RepoUrl;
    event->oldUrl    = fields.at( 2 );
    event->url       = fields.at( 3 );

    addEvent( event );
}


void ZyppHistoryParser::parseRepoAliasEvent( const QStringList & fields )
{
    //      #0               #1        #2                         #3
    // 2024-09-25 10:01:06|ralias |download.opensuse.org-oss_1|slowroll-update|

    if ( ! checkFieldsCount( fields, 4 ) )
        return;

    RepoEvent * event = new RepoEvent;
    CHECK_NEW( event );

    event->timestamp    = fields.at( 0 );
    event->eventType    = EventType::RepoUrl;
    event->oldRepoAlias = fields.at( 2 );
    event->repoAlias     = fields.at( 3 );

    addEvent( event );
}


void ZyppHistoryParser::parsePatchEvent( const QStringList & fields )
{
    //      #0              #1        #2            #3   #4         #5                #6        #7       #8     #9
    // 2026-01-07 16:24:32|patch  |openSUSE-2024-157|1|noarch|repo-backports-update|important|security|needed|applied|

    if ( ! checkFieldsCount( fields, 10 ) )
        return;

    PatchEvent * event = new PatchEvent;
    CHECK_NEW( event );

    event->timestamp  = fields.at( 0 );
    event->eventType  = EventType::Patch;
    event->patchName  = fields.at( 2 );
    event->version    = fields.at( 3 );
    event->arch       = fields.at( 4 );
    event->repoAlias  = fields.at( 5 );
    event->patchState = fields.at( 9 );

    addEvent( event );
}
