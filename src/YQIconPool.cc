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


#include <QPixmap>
#include <QIcon>

#include "Logger.h"
#include "YQIconPool.h"



YQIconPool * YQIconPool::_instance = 0;


YQIconPool * YQIconPool::instance()
{
    if ( ! _instance )
        _instance = new YQIconPool();

    return _instance;
}


QPixmap YQIconPool::pkgTaboo()                  { return instance()->cachedIcon( "package-available-locked",  true  ); }
QPixmap YQIconPool::pkgDel()                    { return instance()->cachedIcon( "package-remove",            true  ); }
QPixmap YQIconPool::pkgUpdate()                 { return instance()->cachedIcon( "package-upgrade",           true  ); }
QPixmap YQIconPool::pkgInstall()                { return instance()->cachedIcon( "package-install",           true  ); }
QPixmap YQIconPool::pkgAutoInstall()            { return instance()->cachedIcon( "package-install-auto",      true  ); }
QPixmap YQIconPool::pkgAutoUpdate()             { return instance()->cachedIcon( "package-upgrade-auto",      true  ); }
QPixmap YQIconPool::pkgAutoDel()                { return instance()->cachedIcon( "package-remove-auto",       true  ); }
QPixmap YQIconPool::pkgKeepInstalled()          { return instance()->cachedIcon( "package-installed",         true  ); }
QPixmap YQIconPool::pkgNoInst()                 { return instance()->cachedIcon( "package-available",         true  ); }
QPixmap YQIconPool::pkgProtected()              { return instance()->cachedIcon( "package-installed-locked",  true  ); }

QPixmap YQIconPool::disabledPkgTaboo()          { return instance()->cachedIcon( "package-available-locked",  false ); }
QPixmap YQIconPool::disabledPkgDel()            { return instance()->cachedIcon( "package-remove",            false ); }
QPixmap YQIconPool::disabledPkgUpdate()         { return instance()->cachedIcon( "package-upgrade",           false ); }
QPixmap YQIconPool::disabledPkgInstall()        { return instance()->cachedIcon( "package-install",           false ); }
QPixmap YQIconPool::disabledPkgAutoInstall()    { return instance()->cachedIcon( "package-install-auto",      false ); }
QPixmap YQIconPool::disabledPkgAutoUpdate()     { return instance()->cachedIcon( "package-upgrade-auto",      false ); }
QPixmap YQIconPool::disabledPkgAutoDel()        { return instance()->cachedIcon( "package-remove-auto",       false ); }
QPixmap YQIconPool::disabledPkgKeepInstalled()  { return instance()->cachedIcon( "package-installed",         false ); }
QPixmap YQIconPool::disabledPkgNoInst()         { return instance()->cachedIcon( "package-available",         false ); }
QPixmap YQIconPool::disabledPkgProtected()      { return instance()->cachedIcon( "package-installed-locked",  false ); }

QPixmap YQIconPool::normalPkgConflict()         { return instance()->cachedIcon( "emblem-warning",            true  ); }

QPixmap YQIconPool::treePlus()                  { return instance()->cachedIcon( "list-add",                  true  ); }
QPixmap YQIconPool::treeMinus()                 { return instance()->cachedIcon( "list-remove",               true  ); }

QPixmap YQIconPool::warningSign()               { return instance()->cachedIcon( "emblem-warning",            true  ); }
QPixmap YQIconPool::pkgSatisfied()              { return instance()->cachedIcon( "package-supported",         true  ); }

QPixmap YQIconPool::tabRemove()                 { return instance()->cachedIcon( "tab-close",                 true  ); }
QPixmap YQIconPool::arrowLeft()                 { return instance()->cachedIcon( "arrow-left",                true  ); }
QPixmap YQIconPool::arrowRight()                { return instance()->cachedIcon( "arrow-right",               true  ); }
QPixmap YQIconPool::arrowDown()                 { return instance()->cachedIcon( "arrow-down",                true  ); }
QPixmap YQIconPool::checkmark()                 { return instance()->cachedIcon( "checkmark",                 true  ); }


YQIconPool::YQIconPool()
{
}


YQIconPool::~YQIconPool()
{
    // NOP
}



QPixmap
YQIconPool::cachedIcon( const QString & iconName, bool enabled )
{
    Q_INIT_RESOURCE( icons );
    QString cachedIconName  = iconName + ( enabled ? 'e' : 'd' );
    QPixmap iconPixmap      = _iconCache[ cachedIconName ];

    if ( iconPixmap.isNull() )
    {
        iconPixmap = loadIcon( iconName, enabled );

        if ( iconPixmap.isNull() )
        {
            // Create an icon for the cache to avoid more than one complaint
            // and to have a clearly visible error icon (a small red square)
            iconPixmap = QPixmap( 8, 8 );
            iconPixmap.fill( Qt::red );
        }
    }

    _iconCache.insert( cachedIconName, iconPixmap );

    return iconPixmap;
}


QPixmap
YQIconPool::loadIcon( const QString & iconName, bool enabled )
{
    QPixmap iconPixmap;

    if ( QIcon::hasThemeIcon( iconName ) )
    {
        // logVerbose() << "Loading theme icon " << iconName << endl;

        QIcon icon = QIcon::fromTheme( iconName, QIcon( ":/" + iconName ) );
        iconPixmap = icon.pixmap( QSize( 16, 16 ), enabled ? QIcon::Normal : QIcon::Disabled );
    }
    else
    {
        // logVerbose() << "Loading built-in icon " << iconName << endl;

        QIcon icon = QIcon( ":/" + iconName );
        iconPixmap = icon.pixmap( QSize( 16, 16 ), enabled ? QIcon::Normal : QIcon::Disabled );
    }

    if ( iconPixmap.isNull() )
        logError() << "Could not load icon " << iconName << endl;

    return iconPixmap;
}

