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


#include <unistd.h>	// getpid()
#include <QString>

#include "Logger.h"
#include "utf8.h"
#include "ZyppLogger.h"


ZyppLogger::ZyppLogger()
    : _lineWriter   ( new ZyppLogLineWriter( this ) )
    , _lineFormatter( new ZyppLogLineFormatter() )
    , _zyppThreadLogger( Logger::lastLogDir(), "zypp.log" )
{
    logInfo() << "Installing the zypp logger" << endl;

    zypp::base::LogControl::instance().setLineFormater( _lineFormatter );
    zypp::base::LogControl::instance().setLineWriter  ( _lineWriter    );
}


ZyppLogger::~ZyppLogger()
{
    logInfo() << "Uninstalling the zypp logger" << endl;

    // Avoid error "QMutex: destroying locked mutex" in the (main) log:
    // The zypp thread might still be writing log messages.
    // Give it some time to complete that.

    bool locked = _mutex.tryLock( 1000 ); // millisec

    zypp::base::LogControl::instance().setLineFormater( 0 );
    zypp::base::LogControl::instance().setLineWriter  ( 0 );

    if ( locked )
        _mutex.unlock();
}


void ZyppLogger::logLine( const std::string & message )
{
    QMutexLocker locker( &_mutex );

    _zyppThreadLogger.logStream() << message << endl;
}




void ZyppLogLineWriter::writeOut( const std::string & formatted_msg )
{
    if ( ! formatted_msg.empty() && _zyppLogger )
    {
        _zyppLogger->logLine( formatted_msg );
    }
}


std::string
ZyppLogLineFormatter::format( const std::string &          log_group,
                              zypp::base::logger::LogLevel log_level,
                              const char *                 src_file,
                              const char *                 src_func,
                              int                          src_line,
                              const std::string &          message )
{
    if ( log_level <= zypp::base::logger::E_DBG )
        return std::string(); // Ignore log level Zypp E_DBG and lower

    QString severity;

    switch ( log_level )
    {
        case zypp::base::logger::E_DBG: severity = "<Debug>  ";   break;
        case zypp::base::logger::E_MIL: severity = "<Info>   ";   break;
        case zypp::base::logger::E_WAR: severity = "<WARNING>";   break;
        case zypp::base::logger::E_ERR: severity = "<ERROR>  ";   break;
        case zypp::base::logger::E_SEC: severity = "<Security>";  break;
        case zypp::base::logger::E_INT: severity = "<IntError>";  break;
        case zypp::base::logger::E_USR: severity = "<usr>    ";   break;
        default:                        severity = "<Verbose>";   break;
    }

    // Sample log lines:
    //   2024-12-06 18:24:59.016 [973] <Debug>   MainWindow.cc:89 showPage():  Showing first page
    //   2024-12-06 18:24:59.018 [973] <Debug>   [zypp-foo] foo_bar.cc:89 zyppify():    zyppifying...

    QString lineHeader( Logger::timeStamp() );

    lineHeader += QString( " [%1] " ).arg( (int) getpid() );
    lineHeader += severity + " ";

    QString logComponent = fromUTF8( log_group );

    if ( ! logComponent.startsWith( "zypp" ) )
        logComponent.prepend( "zypp " );

    lineHeader += QString( "[%1] " ).arg( logComponent.trimmed() );
    lineHeader += QString( src_file );

    if ( src_line > 0 )  // int
        lineHeader += QString( ":%1" ).arg( src_line );

    lineHeader += QString( " %1(): " ).arg( fromUTF8( src_func ) );

    std::string logLine( toUTF8( lineHeader ) );
    logLine += " " + message;


    return logLine;
}

