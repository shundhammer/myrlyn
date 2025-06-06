/*
 *   File name: Exception.cc
 *   Summary:	Exception classes for yqpkg
 *   License:	GPL V2 - See file LICENSE for details.
 *
 *   Author:	Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
 *              Donated by the QDirStat project
 */


#include <errno.h>

#include "Exception.h"


void Exception::setSrcLocation( const QString &srcFile,
				int	       srcLine,
				const QString &srcFunction ) const
{
    // This is why those member variables are 'mutable':
    // We need to be able to set the source location from RETHROW even after
    // the exception was caught as const reference.
    //
    // This is not 100% elegant, but it keeps in line with usual conventions -
    // conventions like "catch exception objects as const reference".

    _srcFile	 = srcFile;
    _srcLine	 = srcLine;
    _srcFunction = srcFunction;

    if ( _srcFile.contains( "/" ) )
    {
        // CMake just dumps the whole path wholesale to the compiler command
        // line which gcc merrily uses as __FILE__; which results in
        // abysmal-looking log lines.
        //
        // So let's cut off the path: Use only the last (No. -1) section
        // delimited with '/'.

        _srcFile = _srcFile.section( '/', -1 );
    }
}


QString IndexOutOfRangeException::errMsg( int	invalidIndex,
                                          int	validMin,
                                          int	validMax,
                                          const QString & prefix ) const
{
    QString msg = prefix;

    if ( msg.isEmpty() )
        msg = "Index out of range";

    msg += QString( ": %1 valid: %2..%3" ).arg( invalidIndex )
        .arg( validMin ).arg( validMax );

    return msg;
}
