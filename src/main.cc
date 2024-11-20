/*
 *   File name: main.cpp
 *   Summary:	yqpkg main program
 *   License:	GPL V2 - See file LICENSE for details.
 *
 *   Author:	Stefan Hundhammer <shundhammer@suse.com>
 */


//  #include <iostream>	// cerr

#include <QApplication>
#include <QObject>
#include "YQPkgApplication.h"
#include "Logger.h"
#include "Exception.h"


// using std::cerr;
static const char * progName = "yqpkg";


void logVersion()
{
    // VERSION is imported from the toplevel VERSION.cmake file
    // via a compiler command line "-DVERSION=..."

    logInfo() << progName << "-" << VERSION
              << " built with Qt " << QT_VERSION_STR
              << endl;
}


int main( int argc, char *argv[] )
{
    Logger logger( "/tmp/yqpkg-$USER", "yqpkg.log" );
    logVersion();

    // Set org/app name for QSettings
    QCoreApplication::setOrganizationName( "openSUSE" ); // ~/.cache/openSUSE
    QCoreApplication::setApplicationName ( progName );   // ~/.cache/openSUSE/yqpkg

    QApplication     qtApp( argc, argv);

    {
        // New scope to limit the life time of this instance

        YQPkgApplication app;
        app.run();
    }

    logDebug() << "YQPkgApplication finished" << endl;

    return 0;
}
