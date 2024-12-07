/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  YQPkg Package Selector
    Copyright (c) 2024 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

    Textdomain "qt-pkg"
 */


#include <unistd.h>	// getpid()
#include <iostream>     // std::cerr
#include <QString>

#include "Logger.h"
#include "Exception.h"
#include "utf8.h"
#include "ZyppLogger.h"


ZyppLogger::ZyppLogger()
    : _lineWriter   ( new ZyppLogLineWriter()    )
    , _lineFormatter( new ZyppLogLineFormatter() )
{
    if ( getenv( "YQPKG_NO_ZYPP_LOG" ) )
    {
        logInfo() << "env $YQPKG_NO_ZYPP_LOG is set - not installing a zypp logger." << endl;
        return;
    }

#if 0
    logInfo() << "Installing the zypp logger" << endl;

    zypp::base::LogControl::instance().setLineFormater( _lineFormatter );
    zypp::base::LogControl::instance().setLineWriter  ( _lineWriter    );
#endif
}


ZyppLogger::~ZyppLogger()
{
    logInfo() << "Uninstalling the zypp logger" << endl;
}



void ZyppLogLineWriter::writeOut( const std::string & formatted_msg )
{
    if ( ! formatted_msg.empty() )
    {
        Logger::defaultLogger()->logStream()
            << fromUTF8( formatted_msg ) << endl;
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

    QString file( src_file );  // const char *

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
