/*  ------------------------------------------------------
              __   _____  ____  _
              \ \ / / _ \|  _ \| | ____ _
               \ V / | | | |_) | |/ / _` |
                | || |_| |  __/|   < (_| |
                |_| \__\_\_|   |_|\_\__, |
                                    |___/
    ------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
    License:  GPL V2 - See file LICENSE for details.

 */


#include <iostream>	// cerr

#include <QApplication>
#include <QObject>

#include "Logger.h"
#include "MyrlynApp.h"


using std::cerr;

static const char * progName = "myrlyn";


void usage()
{
    cerr << "\n"
	 << "Usage: \n"
	 << "\n"
	 << "  " << progName << "\n"
	 << "\n"
	 << "  " << progName << " [<option>] [<option>...]\n"
	 << "\n"
	 << "Options:\n"
	 << "\n"
	 << "  -r | --read-only (default for non-root users)\n"
	 << "  -n | --dry-run\n"
	 << "  -d | --download-only\n"
         << "  -f | --no-repo-refresh\n"
         << "  -v | --force-service-view\n"
	 << "  -h | --help \n"
	 << "\n"
	 << "Debugging options:\n"
	 << "\n"
	 << "  --fake-root\n"
	 << "  --fake-commit\n"
	 << "  --fake-summary\n"
	 << "  --fake-translations  (\"xixoxixoxixo\" everywhere)\n"
         << "  --slow-repo-refresh\n"
	 << "\n"
	 << std::endl;

    exit( 1 );
}


/**
 * Extract a command line switch (a command line argument without any
 * additional parameter) from the command line and remove it from 'argList'.
 **/
bool commandLineOption( const QString & longName,
			const QString & shortName,
			QStringList   & argList )
{
    if ( argList.contains( longName  ) ||
	 ( ! shortName.isEmpty() && argList.contains( shortName ) ) )
    {
	argList.removeAll( longName  );

        if ( ! shortName.isEmpty() )
            argList.removeAll( shortName );

        logDebug() << "Found " << longName << endl;
	return true;
    }
    else
    {
        // logDebug() << "No " << longName << endl;
	return false;
    }
}


MyrlynAppOptions
parseCommandLineOptions( QStringList & argList )
{
    MyrlynAppOptions optFlags;

    if ( commandLineOption( "--read-only",          "-r", argList ) ) optFlags |= OptReadOnly;
    if ( commandLineOption( "--dry-run",            "-n", argList ) ) optFlags |= OptDryRun;
    if ( commandLineOption( "--download-only",      "-d", argList ) ) optFlags |= OptDownloadOnly;
    if ( commandLineOption( "--no-repo-refresh",    "-f", argList ) ) optFlags |= OptNoRepoRefresh;
    if ( commandLineOption( "--force-service-view", "-v", argList ) ) optFlags |= OptForceServiceView;
    if ( commandLineOption( "--fake-root",          "" ,  argList ) ) optFlags |= OptFakeRoot;
    if ( commandLineOption( "--fake-commit",        "" ,  argList ) ) optFlags |= OptFakeCommit;
    if ( commandLineOption( "--fake-summary",       "" ,  argList ) ) optFlags |= OptFakeSummary;
    if ( commandLineOption( "--fake-translations",  "" ,  argList ) ) optFlags |= OptFakeTranslations;
    if ( commandLineOption( "--slow-repo-refresh",  "" ,  argList ) ) optFlags |= OptSlowRepoRefresh;
    if ( commandLineOption( "--help",               "-h", argList ) ) usage(); // this will exit

    if ( ! argList.isEmpty() )
    {
        logError() << "FATAL: Bad command line args: " << argList.join( " " ) << endl;
        usage();
    }

    return optFlags;
}


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
    Logger logger( "/tmp/myrlyn-$USER", "myrlyn.log" );
    logVersion();

    // Set org/app name for QSettings
    QCoreApplication::setOrganizationName( "openSUSE" ); // ~/.cache/openSUSE
    QCoreApplication::setApplicationName ( "Myrlyn" );   // ~/.cache/openSUSE/Myrlyn.conf


    // Create the QApplication first because it might remove some Qt-specific
    // command line arguments already

    QApplication qtApp( argc, argv);

    QStringList argList = QCoreApplication::arguments();
    argList.removeFirst(); // Remove the program name
    MyrlynAppOptions optFlags = parseCommandLineOptions( argList );

    {
        // New scope to minimize the life time of this instance

        MyrlynApp app( optFlags );
        app.run();
    }

    logDebug() << "MyrlynApp finished." << endl;

    return 0;
}
