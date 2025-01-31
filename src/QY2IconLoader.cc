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

    Textdomain "qt-pkg"
 */


#include <unistd.h>     // access() / R_OK

#include "Logger.h"
#include "QY2IconLoader.h"


QIcon QY2IconLoader::loadIcon( const QString & iconName )
{
    // Notice that QIcon::isNull() is not a useful check if loading an icon was
    // successful: It returns 'false' even if the icon just has a filename.
    //
    // There does not seem any reliable check if loading an icon was successful.

    if ( QIcon::hasThemeIcon( iconName ) )
        return loadThemeIcon( iconName );

    if ( iconName.startsWith( "/" ) )
        return loadIconFromPath( iconName );

    return loadBuiltInIcon( iconName );
}


QIcon QY2IconLoader::loadThemeIcon( const QString & iconName )
{
    // logVerbose() << "Using theme icon for " << iconName << Qt::endl;

    return QIcon::fromTheme( iconName );
}


QIcon QY2IconLoader::loadIconFromPath( const QString & iconName )
{
    if ( access( iconName.toUtf8(), R_OK ) != 0 )
    {
        logWarning() << "Can't open icon file " << iconName << Qt::endl;

        return QIcon();
    }

    // logVerbose() << "Loading icon from absolute path " << iconName << Qt::endl;

    return QIcon( iconName );
}


QIcon QY2IconLoader::loadBuiltInIcon( const QString & iconName )
{
    QString resource = ":/";

    QString iconPath = iconName.startsWith( resource ) ?
        iconName : resource + iconName;

    if ( QFile( iconPath ).exists() )
        // Using QFile because it can handle Qt resources and their aliases
    {
        // logVerbose() << "Using built-in icon " << iconName << Qt::endl;

        return QIcon( iconPath );
    }
    else
    {
        // logWarning() << "No built-in icon " << iconName << Qt::endl;

        return QIcon();
    }
}
