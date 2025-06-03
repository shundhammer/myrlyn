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


#ifndef ZyppLogger_h
#define ZyppLogger_h


#include <memory>
#include <string>
#include <QMutex>

#include <zypp-core/base/Logger.h>
#include <zypp-core/base/LogControl.h>


class ZyppLogger;


/**
 * Class that writes a preformatted zypp log line.
 **/
struct ZyppLogLineWriter: public zypp::base::LogControl::LineWriter
{
    ZyppLogLineWriter( ZyppLogger * zyppLogger )
        : _zyppLogger( zyppLogger )
        {}

    virtual void writeOut( const std::string & formated_msg );

private:

    ZyppLogger * _zyppLogger;
};


/**
 * Class that formats a zypp log line.
 **/
struct ZyppLogLineFormatter: public zypp::base::LogControl::LineFormater
{
    virtual std::string
    format( const std::string &          log_group,
            zypp::base::logger::LogLevel log_level,
            const char *                 src_file,
            const char *                 src_func,
            int                          src_line,
            const std::string &          message );
};


/**
 * Class to redirect the Zypp log to the application's log for the life time of
 * this object.
 **/
class ZyppLogger
{
public:

    ZyppLogger();
    ~ZyppLogger();

    /**
     * Write a single line to the zypp log file, protected by a mutex.
     **/
    void logLine( const std::string & message );


private:

    zypp::shared_ptr<ZyppLogLineWriter>    _lineWriter;
    zypp::shared_ptr<ZyppLogLineFormatter> _lineFormatter;

    QMutex _mutex;
    Logger _zyppThreadLogger;
};

#endif // ZyppLogger_h
