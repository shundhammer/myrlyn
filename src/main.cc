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


#include <iostream>	// cerr

#include <QApplication>
#include <QObject>

#include "Logger.h"
#include "MyrlynApp.h"
#include "Translator.h"
#include "ZyppHistory.h"
#include "utf8.h"


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
         << "  -z | --zypp-history </path/to/zypp/history>\n"
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
bool commandLineSwitch( const QString & longName,
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


/**
 * Extract a command line argument with an additional parameter from the
 * command line and remove both from 'argList'.
 *
 * Return the additional parameter if the argument was set, or an empty string
 * if not.
 **/
QString commandLineOptionWithArg( const QString & longName,
                                  const QString & shortName,
                                  QStringList   & argList )
{
    qsizetype pos = argList.indexOf( longName );

    if ( pos < 0 && ! shortName.isEmpty() )
        pos = argList.indexOf( shortName );

    QString result;

    if ( pos >= 0 )
    {
        logDebug() << "Found " << longName << endl;

        if ( pos + 1 >= argList.size() )
        {
            cerr << "\nERROR: Command line option " << toUTF8( longName )
                 << " requires an argument!" << std::endl;

            usage();   // this will exit
        }

        argList.removeAt( pos );  // remove longName or shortName
        result = argList.takeAt( pos );
    }

    return result;
}


MyrlynAppOptions
parseCommandLineOptions( QStringList & argList )
{
    MyrlynAppOptions optFlags;

    if ( commandLineSwitch( "--read-only",          "-r", argList ) ) optFlags |= OptReadOnly;
    if ( commandLineSwitch( "--dry-run",            "-n", argList ) ) optFlags |= OptDryRun;
    if ( commandLineSwitch( "--download-only",      "-d", argList ) ) optFlags |= OptDownloadOnly;
    if ( commandLineSwitch( "--no-repo-refresh",    "-f", argList ) ) optFlags |= OptNoRepoRefresh;
    if ( commandLineSwitch( "--force-service-view", "-v", argList ) ) optFlags |= OptForceServiceView;
    if ( commandLineSwitch( "--fake-root",          "",   argList ) ) optFlags |= OptFakeRoot;
    if ( commandLineSwitch( "--fake-commit",        "",   argList ) ) optFlags |= OptFakeCommit;
    if ( commandLineSwitch( "--fake-summary",       "",   argList ) ) optFlags |= OptFakeSummary;
    if ( commandLineSwitch( "--fake-translations",  "",   argList ) ) Translator::useFakeTranslations();
    if ( commandLineSwitch( "--slow-repo-refresh",  "",   argList ) ) optFlags |= OptSlowRepoRefresh;
    if ( commandLineSwitch( "--help",               "-h", argList ) ) usage(); // this will exit

    QString zyppHistFile = commandLineOptionWithArg( "--zypp-history", "-z", argList );

    if ( ! zyppHistFile.isEmpty() )
        ZyppHistory::setFileName( zyppHistFile );

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
    QCoreApplication::setOrganizationName( "openSUSE" ); // ~/.config/openSUSE
    QCoreApplication::setApplicationName ( "Myrlyn" );   // ~/.config/openSUSE/Myrlyn.conf


    // Create the QApplication first because it might remove some Qt-specific
    // command line arguments already

    QApplication qtApp( argc, argv );

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
