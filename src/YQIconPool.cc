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


#include <QGuiApplication>
#include <QIcon>
#include <QPalette>
#include <QPixmap>
#include <QSettings>
#include <QStyleHints>

#include "Logger.h"
#include "YQIconPool.h"


YQIconPool * YQIconPool::_instance          = 0;
bool         YQIconPool::_useThemeIcons     = true;
bool         YQIconPool::_verboseIconLoader = false;


YQIconPool * YQIconPool::instance()
{
    if ( ! _instance )
        _instance = new YQIconPool();

    return _instance;
}


// Just a shortcut
QIcon YQIconPool::ic( const QString & iconName )
{
    return YQIconPool::instance()->cachedIcon( iconName );
}


QIcon YQIconPool::pkgInstall()           { return ic( "package-install"           ); }
QIcon YQIconPool::pkgUpdate()            { return ic( "package-upgrade"           ); }
QIcon YQIconPool::pkgDel()               { return ic( "package-remove"            ); }

QIcon YQIconPool::pkgAutoInstall()       { return ic( "package-install-auto"      ); }
QIcon YQIconPool::pkgAutoUpdate()        { return ic( "package-upgrade-auto"      ); }
QIcon YQIconPool::pkgAutoDel()           { return ic( "package-remove-auto"       ); }

QIcon YQIconPool::pkgKeepInstalled()     { return ic( "package-installed-updated" ); }
QIcon YQIconPool::pkgNoInst()            { return ic( "package-available"         ); }
QIcon YQIconPool::pkgProtected()         { return ic( "package-installed-locked"  ); }
QIcon YQIconPool::pkgTaboo()             { return ic( "package-available-locked"  ); }

QIcon YQIconPool::arrowLeft()            { return ic( "arrow-left-symbolic"       ); }
QIcon YQIconPool::arrowRight()           { return ic( "arrow-right-symbolic"      ); }
QIcon YQIconPool::arrowDown()            { return ic( "pan-down-symbolic"         ); }
QIcon YQIconPool::checkmark()            { return ic( "checkmark-symbolic"        ); }
QIcon YQIconPool::pkgConflict()          { return ic( "emblem-warning"            ); }
QIcon YQIconPool::tabClose()             { return ic( "tab-close-symbolic"        ); }

QIcon YQIconPool::download()             { return ic( "download-symbolic"         ); }
QIcon YQIconPool::downloadDone()         { return ic( "checkmark-symbolic"        ); }
QIcon YQIconPool::downloadOngoing()      { return ic( "download-symbolic"         ); }
QIcon YQIconPool::downloadPlaceholder()  { return ic( "download-placeholder"      ); }
QIcon YQIconPool::emojiSad()             { return ic( "state-error-symbolic"      ); }
QIcon YQIconPool::emojiSick()            { return ic( "emoji-sick"                ); }
QIcon YQIconPool::hamburger()            { return ic( "application-menu-symbolic" ); }
QIcon YQIconPool::wrench()               { return ic( "configure-symbolic"        ); }


//-----------------------------------------------------------------------


YQIconPool::YQIconPool()
{
    readSettings();

    // Since the settings of this class are not accessible via GUI, make sure
    // to write them to the config file so the user can chang them there with a
    // plain text editor.
    writeSettings();
}


YQIconPool::~YQIconPool()
{
    // This is most likely never called, but we can't be sure.
    writeSettings();
}


bool YQIconPool::usingDarkTheme()
{
    static bool usingDarkTheme      = false;
    static bool checkedForDarkTheme = false;

    if ( ! checkedForDarkTheme )
    {
        usingDarkTheme = detectDarkWidgetTheme();
        checkedForDarkTheme = true;
    }

    return usingDarkTheme;
}


bool YQIconPool::detectDarkWidgetTheme()
{
    bool usingDarkTheme = false;

    switch ( QGuiApplication::styleHints()->colorScheme() )
    {
        case Qt::ColorScheme::Dark:
            usingDarkTheme = true;
            break;

        case Qt::ColorScheme::Light:
            usingDarkTheme = false;
            break;

        default:
            logDebug() << "Fallback detection method" << endl;
            QColor background = QGuiApplication::palette().color( QPalette::Active, QPalette::Base );
            usingDarkTheme = ( background.lightness() < 128 ); // 0 (black) .. 255 (white)
            break;
    }

    logInfo() << "Using a "
              << ( usingDarkTheme ? "dark" : "light" )
              << " widget theme" << endl;

    return usingDarkTheme;
}


void YQIconPool::readSettings()
{
    QSettings settings;
    settings.beginGroup( "Icons" );
    _useThemeIcons = settings.value( "useThemeIcons", true ).toBool();
    settings.endGroup();

    if ( _verboseIconLoader )
    {
        logInfo() << "_useThemeIcons: " << _useThemeIcons << endl;
        logInfo() << "Icon theme name: " << QIcon::themeName() << endl;
        logInfo() << "Icon theme search paths: " << QIcon::themeSearchPaths() << endl;
    }
}


void YQIconPool::writeSettings()
{
    QSettings settings;
    settings.beginGroup( "Icons" );
    settings.setValue( "useThemeIcons", _useThemeIcons );
    settings.endGroup();
}


QIcon
YQIconPool::cachedIcon( const QString & iconName )
{
    Q_INIT_RESOURCE( icons );

    QIcon icon = _iconCache[ iconName ];

    if ( icon.isNull() )
    {
        icon = loadIcon( iconName );

        if ( icon.isNull() )
        {
            // Create an icon for the cache to avoid more than one complaint
            // and to have a clearly visible error icon (a small red square)
            QPixmap errorPixmap( 12, 12 );
            errorPixmap.fill( Qt::red );
            icon = QIcon( errorPixmap );
        }
    }

    _iconCache.insert( iconName, icon );

    return icon;
}


QIcon
YQIconPool::loadIcon( const QString & iconName )
{
    QIcon icon;

    if ( _useThemeIcons && ! iconName.startsWith( "package-" ) )
        icon = loadThemeIcon( iconName );

    if ( icon.isNull() )
        icon = loadBuiltinIcon( iconName );

    if ( icon.isNull() )
        logError() << "Could not load icon " << iconName << endl;

    return icon;
}


QIcon
YQIconPool::loadThemeIcon( const QString & iconName )
{
    QIcon icon;

    if ( QIcon::hasThemeIcon( iconName ) )
    {
        if ( _verboseIconLoader )
            logDebug() << "Loading theme icon " << iconName << endl;

        icon = QIcon::fromTheme( iconName, QIcon( ":/" + iconName ) );
    }
    else //  ! hasThemeIcon
    {
        if ( _verboseIconLoader )
            logDebug() << "  No theme icon " << iconName << endl;
    }

    return icon;
}


QIcon
YQIconPool::loadBuiltinIcon( const QString & rawIconName )
{
    QString iconName = rawIconName;

    if ( usingDarkTheme() )
    {
        QString darkIconName = "dark-theme-" + rawIconName;

        if ( QFile::exists( ":/" + darkIconName ) )
            iconName = darkIconName;
    }

    if ( _verboseIconLoader )
        logDebug() << "Loading built-in icon " << iconName << endl;

    return QIcon( ":/" + iconName );
}
