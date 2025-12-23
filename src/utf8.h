/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

 */


#ifndef utf8_h
#define utf8_h

#include <qstring.h>
#include <string>
#include <iosfwd>


inline QString fromUTF8( const std::string & str )
{
    return QString::fromUtf8( str.c_str() );
}


inline std::string toUTF8( const QString & str )
{
    if ( str.isEmpty() )
	return std::string( "" );
    else
	return std::string( str.toUtf8().data() );
}


#endif // utf8_h
